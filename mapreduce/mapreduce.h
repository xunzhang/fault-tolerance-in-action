#ifndef MAPRETUCE_H
#define MAPRETUCE_H

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <unordered_map>

#include "util.h"

namespace mapreduce {

using std::string;
using std::vector;
using std::unordered_map;

class MapReduce {
 public:
  MapReduce(string _file,
            int nmap, int nreduce, string master = "") 
      : file(_file),
        nMap(nmap),
        nReduce(nreduce),
        MasterAddress(master) {}

  void Split() {
    std::ifstream fin(file);
    if(!fin) {
      throw std::runtime_error("open file error in Split.\n");
    }
    std::ifstream f(file);
    long sz = f.tellg();
    long chunkSz = sz / static_cast<long>(nMap);
    auto splitFile = [&] (long start, long end, int indx) {
      std::ofstream os;
      string name = MapName(indx);
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
  void DoMap(int indx, mapreduce::Map<K, V> && Map) {
    string name = MapName(indx);
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
    vector<std::ofstream> fsout;
    for(int k = 0; k < nReduce; ++k) {
      std::ofstream fout(ReduceName(indx, k));
      fsout.push_back(fout);
    }
    for(auto & res : mapRes) {
      for(auto & kv : res) {
        auto key = std::get<0>(kv);
        auto value = std::get<1>(kv);
        auto findx = ihash(key);
        fsout[findx] << std::to_string(key) 
            << ":" << std::to_string(value) << '\n';
      }
    }
    
    for(auto & fout : fsout) {
      fout.close();
    }
    fin.close();
  }

  template <class K, class V>
  void DoReduce(int JobNumber, mapreduce::Reduce<K, V> && Reduce) {
    for(int k = 0; k < nMap; ++k) {
      auto name = ReduceName(k, JobNumber);
      std::cout << "DoReduce: read " << name << std::endl;
      std::ifstream f(name);
      if(!f) {
        throw std::runtime_error("open file error in DoReduce.\n");
      }
      // TODO
      f.close();
    }
  }

  void Merge() {
  }

 private:
  inline string MapName(int indx) {
    return "mrtmp." + file + "-" + std::to_string(indx);
  }

  inline string ReduceName(int mindx, int rindx) {
    return MapName(mindx) + "-" + std::to_string(rindx);
  }

  template <class K>
  inline std::size_t ihash(K key) {
    return  std::hash<K>()(key);
  }

 private:
  int nMap;
  int nReduce;
  string file;
  string MasterAddress;
}; // class MapReduce

template <class K, class V>
void RunSingle(string file, int nMap, int nReduce,
               mapreduce::Map<K, V> && Map, mapreduce::Reduce<K, V> && Reduce) {
  MapReduce mr(file, nMap, nReduce);
  mr.Split();
  for(size_t i = 0; i < nMap; ++i) {
    mr.DoMap<K, V>(i, nReduce, Map);
  }
  for(size_t i = 0; i < nReduce; ++i) {
    mr.DoReduce<K, V>(i, nMap, Reduce);
  }
  mr.Merge();
}

} // namespace mapreduce

#endif
