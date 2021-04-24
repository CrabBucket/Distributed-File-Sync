#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include "FileHelper.h"
#include <map>
#include <vector>

void RefreshDirectory(LPTSTR);
void RefreshTree(LPTSTR);
void WatchDirectory(LPTSTR);
void HandleDirectoryChange(LPTSTR);
std::map<std::wstring, uint64_t> CreateFileHashes(const std::wstring dirPath);
enum class fileChangeType { Addition, Deletion, Edit };
struct fileChangeData {
	std::wstring filePath;
	uint64_t fileHash;
	fileChangeType change;
	
};