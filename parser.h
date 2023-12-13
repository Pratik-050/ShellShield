#include <iostream>
#include <fstream>
#include <map>
#include <sstream>

using namespace std;
string trim(const string& str) {
  size_t first = str.find_first_not_of(" \t\n\r\f\v");
  if (first == string::npos) return "";
  size_t last = str.find_last_not_of(" \t\n\r\f\v");
  return str.substr(first, last - first + 1);
}

map<string, string> parseIni(const string& filename) {
  map<string, string> config;
  ifstream file(filename);
  //opening config file
  if (!file.is_open()) {
    cerr << "Error opening file: " << filename << endl;
    return {};
  }

  string line, section;
  while (getline(file, line)) {
    // Get section Name
    if (line.find('[') != string::npos && line.find(']') != string::npos) {
      section = line.substr(line.find('[') + 1, line.find(']') - line.find('[') - 1);
      continue; // Skip to next line for key-value pairs
    }

    // Extract key and value
    stringstream ss(line);
    string key, value;
    getline(ss, key, '=');
    getline(ss, value);

    // Trim spaces
    key = trim(key);
    value = trim(value);
  
    // Add data to map
    config[key] = value;
  }

  file.close();
  return config;
}




