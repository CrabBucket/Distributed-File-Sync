#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>
#include <fstream>

namespace fs = std::filesystem;

//true if directory was created, false if error or directory already exists
bool createDirectory(const std::string& absPath);
//gets string filepath of all file in given directory including file of subdirectories and so on
std::vector<std::string> getFilepaths(const std::string& absPath);
//true if the two given paths are contain non-identical files
bool filesDiffer(const std::string& absPath1, const std::string& absPath2);

//get filesize in bytes
std::size_t filesize(const std::string& absPath);
//hash file into a 64 bit number
uint64_t getFileHash(const std::string& absPath);
