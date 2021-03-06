#ifndef MAPRETUCE_H
#define MAPRETUCE_H

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <utility>
#include <algorithm>
#include <unordered_map>
#include <cstddef>
#include <thread>
#include <mutex>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "util.h"
#include "chan.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using std::string;
using std::vector;
using std::unordered_map;
using std::thread;
using cpp::channel;
using std::mutex;
using boost::shared_ptr;
 
namespace mapreduce {

string MapName(const string & file, int indx) {
  string fileSuffix = mapreduce::strSplit(file, '/').back();
  return "mrtmp." + fileSuffix + "-" + std::to_string(indx);
}

string ReduceName(const string & file, int mindx, int rindx) {
  return MapName(file, mindx) + "-" + std::to_string(rindx);
}

string MergeName(const string & file, int rindx) {
  string fileSuffix = mapreduce::strSplit(file, '/').back();
  return "mrtmp." + fileSuffix + "-res-" + std::to_string(rindx);
}

template <class K>
inline std::size_t ihash(K key) {
  return std::hash<K>()(key);
}

template <class K, class V>
void DoMap(int indx, const string & file, int nReduce, mapreduce::Map<K, V> & Map) {
  string name = MapName(file, indx);
  std::ifstream fin(name);
  if(!fin) {
    throw std::runtime_error("open file error in DoMap.\n");
  }
  std::stringstream strStream;
  strStream << fin.rdbuf();
  string text = strStream.str();
  std::cout << "DoMap: read split " << name << std::endl;
  auto mapRes = Map(text);

  // no compression here, naive serialization
  vector<std::ofstream> fstreams;
  for(int k = 0; k < nReduce; ++k) {
    fstreams.emplace_back(std::ofstream{ReduceName(file, indx, k)});
  }
  for(auto & kv : mapRes) {
    auto key = std::get<0>(kv);
    auto value = std::get<1>(kv);
    size_t findx = ihash(key) % nReduce;
    fstreams[findx] << key << ':' << value << '\n';
  }

  for(auto & fout : fstreams) {
    fout.close();
  }
  fin.close();
}

template <class K, class V>
void DoReduce(int JobNumber, const string & file, int nMap, mapreduce::Reduce<V> & Reduce) {
  unordered_map<K, vector<V>> kvs;
  for(int k = 0; k < nMap; ++k) {
    auto name = ReduceName(file, k, JobNumber);
    std::cout << "DoReduce: read " << name << std::endl;
    std::ifstream f(name);
    if(!f) {
      throw std::runtime_error("open file error in DoReduce.\n");
    }
    string line;
    while(std::getline(f, line)) {
      auto kv = mapreduce::strSplit(line, ':');
      K key;
      V val;
      std::istringstream is(kv[0]), iss(kv[1]);
      is >> key; iss >> val;
      kvs[key].push_back(val);
    }
    f.close();
  }
  vector<K> keyList;
  for(auto & kv : kvs) {
    keyList.push_back(kv.first);
  }
  auto cmp_lambda = [] (K a, K b) {
    return a < b;
  };
  std::sort(keyList.begin(), keyList.end(), cmp_lambda);
  auto rname = MergeName(file, JobNumber);
  std::ofstream os;
  os.open(MergeName(file, JobNumber));
  for(auto key : keyList) {
    V res = Reduce(kvs[key]);
    os << key << ":" << res << '\n';
  }
  os.close();
}

class MapReduce {
 public:
  MapReduce(string _file, int nmap, int nreduce) 
      : file(_file),
        nMap(nmap),
        nReduce(nreduce) {
    fileSuffix = mapreduce::strSplit(_file, '/').back();
  }
  
  MapReduce(string _file,
            int nmap, int nreduce, string master)
      : file(_file),
        nMap(nmap),
        nReduce(nreduce),
        MasterAddr(master) {
    fileSuffix = mapreduce::strSplit(_file, '/').back();
    StartRegisterServer();
  }

  ~MapReduce() {
    if(registerServerThrd.size() == 1) {
      for(auto & thrd : registerServerThrd) {
        thrd.join();
      }
      delete registerServerPtr;
    }
  }

  void Split() {
    std::ifstream fin(file);
    if(!fin) {
      throw std::runtime_error("open file error in Split.\n");
    }
    std::ifstream f(file, std::ios::ate);
    long sz = f.tellg();
    long chunkSz = sz / static_cast<long>(nMap);
    auto splitFile = [&] (long start, long end, int indx) {
      std::ofstream os;
      string name = MapName(file, indx);
      std::cout << "Split " << name << std::endl;
      os.open(name);
      auto offset = start;
      if(offset != 0) {
        fin.seekg(offset - 1);
        string tmp;
        std::getline(fin, tmp);
        offset += tmp.size();
      }
      while(offset < end) {
        string buff;
        std::getline(fin, buff);
        offset += buff.size() + 1;
        os << buff << '\n';
      }
      os.close();
    }; // splitFile
    for(int k = 0; k < nMap; ++k) {
      long s = static_cast<long>(k) * chunkSz;
      long e = s;
      if(k == nMap - 1) {
        e = sz; 
      } else {
        e = (static_cast<long>(k) + 1) * chunkSz;
      }
      splitFile(s, e, k);
    } 
    f.close();
  }

  template <class K, class V>
  void Merge() {
    unordered_map<string, string> kvs;
    for(int k = 0; k < nReduce; ++k) {
      auto name = MergeName(file, k);
      std::ifstream fin(name);
      if(!fin) {
        throw std::runtime_error("open file error in Merge.\n");
      }
      std::cout << "Merge: read " << name << std::endl;
      string line;
      while(std::getline(fin, line)) {
        auto kv = mapreduce::strSplit(line, ':'); 
        kvs[kv[0]] = kv[1];
      }
      fin.close();
    }
    vector<string> keyList;
    for(auto & kv : kvs) {
      keyList.push_back(kv.first);
    }
    std::sort(keyList.begin(), keyList.end());
    std::ofstream os;
    string resultFile = "mrtmp." + fileSuffix;
    os.open(resultFile);
    for(auto & key : keyList) {
      os << key << ":" << kvs[key] << '\n';
    }
    std::cout << "Result generated in: " << resultFile << std::endl;
    os.close();
  }

 public:
  // Run jobs in parallel, assuming a shared file system such as nfs
  template <class K, class V>
  void Run() {
    std::cout << "Run mapreduce job " << MasterAddr << " " << file << std::endl;
    Split();
    stats = RunMaster();
    Merge<K, V>();
    // CleanupRegister();
    std::cout << MasterAddr << ": Mapreduce done" << std::endl;
    // mr.DoneChannel <- true
  }

 private:
  void StartRegisterServer() {
    auto register_handler = [&] () {
      int port = std::stoi(mapreduce::strSplit(masterAddr, ':').back());
      shared_ptr<MapReduceServiceHandler> handler(new MapReduceServiceHandler(registerChan));
      shared_ptr<TProcessor> processor(new MapReduceServiceProcessor(handler))
      shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
      shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
      shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
      registerServerPtr = new TSimpleServer(processor, serverTransport, transportFactory, protocolFactory);
      registerServerPtr->serve();
    };
    registerServerThrd.push_back(thread(register_handler));
  }

  // master logic
  vector<int> RunMaster() {
    // hand out map jobs
    auto map_handler = [&] (int id) {
      while(1) {
        string worker;
        //DoRpc(DoMap, worker, id, file, nReduce, Mapper);
        return;
      }
    };
    for(int i = 0; i < nMap; ++i) {
      workers.push_back(thread(map_handler, i));
    }
    // wait until all done
    for(auto & worker : workers) worker.join();
    std::cout << "All map jobs done!" << std::endl;

    auto reduce_handler = [&] (int id) {
    };
    // hand out reduce jobs
    for(int i = 0; i < nReduce; ++i) {
      workers.push_back(thread(reduce_handler, i));
    }
    // wait until all done
    for(auto & worker : workers) worker.join();
    std::cout << "All reduce jobs done!" << std::endl;

    // kill workers
  }

  // master logic
  vector<int> KillWorkers() {}

 private:
  string file;
  int nMap;
  int nReduce;
  string MasterAddr;
  string fileSuffix;
  vector<int> stats;
 
 private: // for parallel use
  TSimpleServer *registerServerPtr;
  vector<thread> registerServerThrd;
  vector<thread> workersThrds;
  unordered_map<string, string> workers;
  channel<string> registerChan;
  channel<string> freeChan;
}; // class MapReduce

template <class K, class V>
void RunSingle(string file, int nMap, int nReduce,
               mapreduce::Map<K, V> && Map, mapreduce::Reduce<V> && Reduce) {
  MapReduce mr(file, nMap, nReduce);
  mr.Split();
  for(size_t i = 0; i < nMap; ++i) {
    DoMap(i, file, nReduce, Map);
  }
  for(size_t i = 0; i < nReduce; ++i) {
    DoReduce<K, V>(i, file, nMap, Reduce);
  }
  mr.Merge<K, V>();
}

} // namespace mapreduce

#endif
