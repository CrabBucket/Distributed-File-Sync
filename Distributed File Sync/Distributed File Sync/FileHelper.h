#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>
#include <fstream>

namespace fs = std::filesystem;

//true if directory was created, false if error or directory already exists
bool createDirectory(const std::wstring& absPath);
//gets wstring filepath of all file in given directory including file of subdirectories and so on
std::vector<std::wstring> getFilepaths(const std::wstring& absPath);
//true if the two given paths are contain non-identical files
bool filesDiffer(const std::wstring& absPath1, const std::wstring& absPath2);

//get filesize in bytes
std::size_t filesize(const std::wstring& absPath);
//hash file into a 64 bit number
uint64_t getFileHash(const std::wstring& absPath);
