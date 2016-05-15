#include <vector>
#include <utility>
#include <string>
#include <numeric>

#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>

#include <google/gflags.h>

#include "mapreduce.h"
#include "util.h"

std::vector<std::string> parser(const std::string & line) {
  std::vector<std::string> wl, rl;
  boost::algorithm::split_regex(wl, line, boost::regex("[^-a-zA-Z0-9_]"));
  for(size_t i = 0; i < wl.size(); ++i) {
    if(wl[i] != "") {
      rl.push_back(wl[i]);
    }
  }
  return rl;
}

std::vector<std::pair<std::string, int>> mapper(const std::string & v) {
  std::vector<std::pair<std::string, int>> result;
  auto lines = mapreduce::strSplit(v, '\n');
  for(auto & line : lines) {
    auto words = parser(line);
    for(auto & word : words) {
      result.push_back(std::make_pair(word, 1));
    }
  }
  return result;
}

int reducer(const std::vector<int> & v) {
  return std::accumulate(v.begin(), v.end(), 0);
}

DEFINE_string(file, "", "input data\n");
DEFINE_string(method, "sequential", "sequential | master | worker\n");
DEFINE_int32(nmap, 3, "number of mappers\n");
DEFINE_int32(nreduce, 2, "number of reducers\n");

int main(int argc, char *argv[])
{
  google::SetUsageMessage("[options]\n\t--method\n\t--nmap\n\t--nreduce\n\t--file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);
  if(FLAGS_method == "sequential") {
    mapreduce::RunSingle<std::string, int>(FLAGS_file, FLAGS_nmap, FLAGS_nreduce, mapper, reducer);
  } else if(FLAGS_method == "master") {
    
  } else if(FLAGS_method == "worker") {
  
  } else {
  
  }
}
