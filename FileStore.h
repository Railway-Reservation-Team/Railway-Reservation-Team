
#ifndef FILE_STORE_H
#define FILE_STORE_H

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

class FileStore {
public:
    static bool saveLines(const std::string& filename,
                          const std::vector<std::string>& lines) {
        std::ofstream file(filename, std::ios::trunc);

        if (!file.is_open()) {
            return false;
        }

        for (const auto& line : lines) {
            file << line << '\n';

            if (!file) {
                return false;
            }
        }

        file.close();
        return !file.fail();
    }

    static bool loadLines(const std::string& filename,
                          std::vector<std::string>& lines) {
        std::ifstream file(filename);

        if (!file.is_open()) {
            return false;
        }

        std::vector<std::string> loaded;
        std::string line;

        while (std::getline(file, line)) {
            loaded.push_back(line);
        }

        if (file.bad()) {
            return false;
        }

        lines = loaded;
        return true;
    }
};

#endif
