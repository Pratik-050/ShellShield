#include <iostream>
#include <fstream>
#include <map>
#include <sstream>

using namespace std;

/*parseIni function helps to  extract the key and values from config files return map<string,string>*/

map<string, string> parseIni(const string& filename) {
  map<string, string> config; 
  ifstream file(filename); //creates a input stream file and opens a file named as filename
  //opening config file
  if (!file.is_open()) {
    cerr << "Error opening file: " << filename << endl; //error
    return {};
  }

  string line, section;
  while (getline(file, line)) {
    // Get section Name
    if (line.find('[') != string::npos && line.find(']') != string::npos) {
      section = line.substr(line.find('[') + 1, line.find(']') - line.find('[') - 1); //finding section name
      continue; // Skip to next line for key-value pairs
    }

    // Extract key and value
    stringstream ss(line); //
    string key, value;
    getline(ss, key, '='); //to split the each line where "=" presents & before "=" -> value
    getline(ss, value); //after "=" -> value

    // Add data to map
    config[key] = value;
  }

  file.close(); //close the file
  return config;
}




