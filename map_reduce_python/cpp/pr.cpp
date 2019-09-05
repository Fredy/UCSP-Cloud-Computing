#include "tbb/concurrent_queue.h"
#include <iostream>
#include <regex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include <fstream>
#include <algorithm>
#include <streambuf>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/regex.hpp>

using namespace std;

const size_t CHUNK_SIZE = 4096 * (1 << 13);

const string M_DATA = "M_DATA";
const string M_END = "M_END";

template <class T> using Message = pair<string, T>;

template <class T>
using MessageQueue = tbb::concurrent_bounded_queue<Message<T>>;

using Counter = unordered_map<string, int>;

struct ChunkData {
  string data;
  size_t size;
};

template <typename T> void process_one(MessageQueue<T> &input_queue) {
  while (true) {
    Message<T> message;
    input_queue.pop(message);
    cout << message.second << endl;
  }
}

template <typename T> void process_two(MessageQueue<T> &output_queue) {
  while (true) {
    output_queue.push({"A", "mensaje"});
    this_thread::sleep_for(3s);
  }
}

Counter _count_words(const string &s) {
  const string rgx_str = "\\s+";
  Counter counter;

  regex rgx(rgx_str);

  sregex_token_iterator iter(s.begin(), s.end(), rgx, -1);
  sregex_token_iterator end;

  while (iter != end) {
    counter[*iter]++;
    iter++;
  }

  return counter;
}

Counter _count_words_2(const string &s) {
  Counter counter;

  vector<string> strings;
  boost::algorithm::split_regex(strings, s, boost::regex("\\s+"));

  for (const auto& str: strings) {
    counter[str]++;
  }

  return counter;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout <<  "Please include the file name as argument." << endl;
    return -1;
  }

  ifstream file(argv[1]);
  string str((std::istreambuf_iterator<char>(file)),
                 std::istreambuf_iterator<char>());

  str = str.substr(0, 32000000);
  cout << str.size() << endl;
  auto data = _count_words_2(str);

  for( auto& i : data) {
    cout << i.first << " " << i.second << endl;
  }
}