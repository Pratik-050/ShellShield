#include <iostream>
#include <fstream>
#include <sstream>
#include <map>



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
        } else {
            std::cerr << "Key not found: " << key << " in section: " << section << std::endl;
        }
    } else {
        std::cerr << "Section not found: " << section << std::endl;
    }

    return defaultValue;
}


private:
    void processLine(const std::string& line, std::string& currentSection) {
    // std::cout << "Processing line: " << line << std::endl;

    std::istringstream iss(line);
    std::string token;
    iss >> std::ws;  // Skip leading whitespaces

    if (!token.empty() && token[0] == ';') {
        // Comment, ignore the whole line
    } else if (line.find('[') != std::string::npos && line.find(']') != std::string::npos) {
        // Section
        currentSection = line.substr(line.find('[') + 1, line.find(']') - line.find('[') - 1);
    } else {
        // Key and value
        size_t equalsPos = line.find('=');
        if (equalsPos != std::string::npos) {
            std::string key = line.substr(0, equalsPos);
            std::string value = line.substr(equalsPos + 1);

            // Trim leading and trailing whitespaces from the key and value
            size_t firstNonSpaceKey = key.find_first_not_of(" \t");
            size_t lastNonSpaceKey = key.find_last_not_of(" \t");

            size_t firstNonSpaceValue = value.find_first_not_of(" \t");
            size_t lastNonSpaceValue = value.find_last_not_of(" \t");

            if (firstNonSpaceKey != std::string::npos && lastNonSpaceKey != std::string::npos &&
                firstNonSpaceValue != std::string::npos && lastNonSpaceValue != std::string::npos) {
                key = key.substr(firstNonSpaceKey, lastNonSpaceKey - firstNonSpaceKey + 1);
                value = value.substr(firstNonSpaceValue, lastNonSpaceValue - firstNonSpaceValue + 1);
                data[currentSection][key] = value;
            }
        }
    }
}




    std::map<std::string, std::map<std::string, std::string>> data;
};


// Use case of parser: you can use it to see how the parser is working
// int main() {
//     INIParser parser;
//     if (parser.load("container_config.ini")) {
//         std::string value = parser.getValue("container", "max_processes", "4");
//         std::string value2 = parser.getValue("container", "custom_root", "4");
//         std::cout << "Value: " << value << std::endl;
//         std::cout << "Value2: " << value2 << std::endl;

//     }

//     return 0;
// }
