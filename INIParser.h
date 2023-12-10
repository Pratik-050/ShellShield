#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

// Use case of parser
// INIParser parser;
//     if (parser.load("container_config.ini")) {
//         std::string value = parser.getValue("section", "key", "default_value");
//         std::cout << "Value: " << value << std::endl;
//     }

// Simple INI file parser
class INIParser {
public:
    bool load(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return false;
        }

        std::string line;
        std::string currentSection;
        while (std::getline(file, line)) {
            processLine(line, currentSection);
        }

        return true;
    }

    std::string getValue(const std::string& section, const std::string& key, const std::string& defaultValue = "") const {
        auto sectionIter = data.find(section);
        if (sectionIter != data.end()) {
            auto keyIter = sectionIter->second.find(key);
            if (keyIter != sectionIter->second.end()) {
                return keyIter->second;
            }
        }

        return defaultValue;
    }

private:
    void processLine(const std::string& line, std::string& currentSection) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (!token.empty()) {
            if (token[0] == ';') {
                // Comment, ignore the whole line
            } else if (token[0] == '[') {
                // Section
                currentSection = token.substr(1, token.find(']') - 1);
            } else {
                // Key and value
                size_t equalsPos = token.find('=');
                if (equalsPos != std::string::npos) {
                    std::string key = token.substr(0, equalsPos);
                    std::string value = token.substr(equalsPos + 1);
                    data[currentSection][key] = value;
                }
            }
        }
    }

    std::map<std::string, std::map<std::string, std::string>> data;
};

// int main() {
//     

//     return 0;
// }
