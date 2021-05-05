#include <stdio.h>
#include "FileHelper.h"
#include "Client.h"
#include "Server.h"
#include "Node.h"
#include <iostream>
#include <SFML/Network.hpp>
#include <string>
#include <vector>
#include <thread>
#include "DirectoryMonitor.h"
#include <mutex>

#include <fstream>


using namespace std;

void discoverThreadFunction(Node&);
void handlerThreadFunction(Node&);
void tableManagerThreadFunction(Node&);
void directoryWatcherThreadFunction(std::wstring&, std::mutex&);

std::mutex dirLock;

int main() {
	/*std::ifstream file("test.txt", ios::in | ios::binary);
	int chunkSize = 4096;
	char* buffer = new char[chunkSize];
	file.read(buffer, chunkSize);
	std::string contents(buffer, file.gcount());
	//delete[] buffer;
	std::cout << contents << std::endl;
	std::cout << file.gcount() << " " << file.tellg() << std::endl;
	file.close();
	std::ofstream ofile("test1.txt", ios::out | ios::binary);
	ofile.write(buffer, file.gcount());
	std::cout << ofile.tellp() << std::endl;
	ofile.close();
	delete[] buffer;*/


	std::wstring directory = getDocumentsPath() + L"\\File Sync Shared Folder";
	createDirectory(directory);
	Node n(directory);
	n.listenUdp(45773);
	n.setDirLock(dirLock);
	std::thread discoverer(discoverThreadFunction, std::ref(n));
	std::thread handler(handlerThreadFunction, std::ref(n));
	std::thread tableManager(tableManagerThreadFunction, std::ref(n));
	std::thread directoryWatcher(directoryWatcherThreadFunction, std::ref(directory), std::ref(dirLock));
	discoverer.join();
	handler.join();
	tableManager.join();
	directoryWatcher.join();
	std::cout << "done" << std::endl;
	getchar();
}

void discoverThreadFunction(Node& n) {
	n.discoverDriver();
}

void handlerThreadFunction(Node& n) {
	n.handlerDriver();
}

void tableManagerThreadFunction(Node& n) {
	n.tableManagerDriver();
}

void directoryWatcherThreadFunction(std::wstring& directory, std::mutex& dirLockMutex) {
	WatchDirectory(directory.data(), dirLockMutex);
}