#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

const string M_DATA = "M_DATA";
const string M_END = "M_END";

using Counter = unordered_map<string, int>;

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

int main() {
  ifstream file("../counter.py");
  char tmp[102] = {0};
  do {
    file.read(tmp, 100);
    cout << file.gcount() << "  "<< file.tellg() <<  endl;
  } while (file);
  file.clear();
  file.seekg(3700);
  cout << file.tellg() << endl;
  file.read(tmp, 65);
  cout << tmp << endl;
}