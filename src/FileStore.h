#ifndef RRS_FILE_STORE_H
#define RRS_FILE_STORE_H

#include "AtomicFileReplace.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

// Reusable line-oriented storage for application data files.
// A caller-supplied validator defines what constitutes a valid record.
class FileStore {
public:
    using RecordValidator = std::function<bool(const std::string&)>;

    // A missing file is treated as an empty data store. An existing file that
    // cannot be read is reported as an error and does not alter the output.
    static bool loadLines(
        const std::string& filename,
        std::vector<std::string>& records,
        std::string& errorMessage) {

        errorMessage.clear();
        std::ifstream input(filename);

        if (!input.is_open()) {
            std::error_code filesystemError;
            const bool exists = std::filesystem::exists(filename, filesystemError);

            if (!filesystemError && !exists) {
                records.clear();
                return true;
            }

            errorMessage = "Unable to open data file for reading: " + filename;
            return false;
        }

        std::vector<std::string> loaded;
        std::string line;

        while (std::getline(input, line)) {
            if (!line.empty()) {
                loaded.push_back(line);
            }
        }

        if (input.bad()) {
            errorMessage = "An error occurred while reading data file: " + filename;
            return false;
        }

        records.swap(loaded);
        return true;
    }

    // Replaces the complete record set only after every record has passed
    // validation and the temporary file has been written successfully.
    static bool saveLines(
        const std::string& filename,
        const std::vector<std::string>& records,
        const RecordValidator& validator,
        std::string& errorMessage) {

        errorMessage.clear();

        if (!validator) {
            errorMessage = "A record validator is required before saving.";
            return false;
        }

        for (const std::string& record : records) {
            if (!validator(record)) {
                errorMessage = "Invalid record rejected; data file was not changed.";
                return false;
            }
        }

        const std::string temporaryFile = filename + ".tmp";
        std::ofstream output(temporaryFile, std::ios::out | std::ios::trunc);

        if (!output.is_open()) {
            errorMessage = "Unable to open temporary data file for writing: " + temporaryFile;
            return false;
        }

        for (const std::string& record : records) {
            output << record << '\n';
            if (!output) {
                output.close();
                std::remove(temporaryFile.c_str());
                errorMessage = "Failed while writing temporary data file.";
                return false;
            }
        }

        output.flush();
        const bool writeSucceeded = static_cast<bool>(output);
        output.close();

        if (!writeSucceeded || output.fail()) {
            std::remove(temporaryFile.c_str());
            errorMessage = "Unable to complete writing the data file.";
            return false;
        }

        if (!replaceFileAtomically(temporaryFile, filename)) {
            std::remove(temporaryFile.c_str());
            errorMessage = "Unable to safely replace data file: " + filename;
            return false;
        }

        return true;
    }

    // Adds a validated record without discarding records already on disk.
    static bool appendLine(
        const std::string& filename,
        const std::string& record,
        const RecordValidator& validator,
        std::string& errorMessage) {

        errorMessage.clear();

        if (!validator || !validator(record)) {
            errorMessage = "Invalid record rejected; data file was not changed.";
            return false;
        }

        std::vector<std::string> records;
        if (!loadLines(filename, records, errorMessage)) {
            return false;
        }

        records.push_back(record);
        return saveLines(filename, records, validator, errorMessage);
    }

    // Replaces one record by position while retaining every other record.
    static bool updateLine(
        const std::string& filename,
        std::size_t index,
        const std::string& replacement,
        const RecordValidator& validator,
        std::string& errorMessage) {

        errorMessage.clear();

        if (!validator || !validator(replacement)) {
            errorMessage = "Invalid replacement record; data file was not changed.";
            return false;
        }

        std::vector<std::string> records;
        if (!loadLines(filename, records, errorMessage)) {
            return false;
        }

        if (index >= records.size()) {
            errorMessage = "Record index is outside the available data.";
            return false;
        }

        records[index] = replacement;
        return saveLines(filename, records, validator, errorMessage);
    }
};

#endif
