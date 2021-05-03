//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include <winsock2.h>
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

//#pragma comment (lib, "Ws2_32.lib")

using namespace std;

void discoverThreadFunction(Node&);
void handlerThreadFunction(Node&);
void tableManagerThreadFunction(Node&);
void directoryWatcherThreadFunction(std::wstring&, std::mutex&);

std::mutex dirLock;

int main() {
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