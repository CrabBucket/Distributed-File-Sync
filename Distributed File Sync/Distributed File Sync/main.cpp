#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
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
#include <iostream>
#include "DirectoryMonitor.h"
#include <fstream>
#include <mutex>

#pragma comment (lib, "Ws2_32.lib")

using namespace std;

int printMenu();

int test3();
void tcpFileShareTest();
int tableTest();
//void watchDirectoryTest();

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

int printMenu() {
	std::cout << "1) send message" << std::endl;
	std::cout << "2) receive message" << std::endl;
	std::cout << "3) get todo count" << std::endl;
	std::cout << "4) handle" << std::endl;
	std::cout << "5) accept" << std::endl;
	std::cout << "6) exit" << std::endl;
	int n;
	std::cin >> n;
	return n;
}

//int test3() {
//	Node n;
//	n.listenUdp(45773);
//	std::thread discoverer(discoverThreadFunction, std::ref(n));
//	std::thread handler(handlerThreadFunction, std::ref(n), std::ref(dirLock));
//	discoverer.join();
//	handler.join();
//	std::cout << "done" << std::endl;
//	getchar();
//	return 0;
//}

//int tableTest() {
//	Node n;
//	n.listenUdp(45773);
//	std::thread discoverer(discoverThreadFunction, std::ref(n));
//	std::thread handler(handlerThreadFunction, std::ref(n), std::ref(dirLock));
//	std::thread tableManager(tableManagerThreadFunction, std::ref(n));
//	discoverer.join();
//	handler.join();
//	tableManager.join();
//	std::cout << "done" << std::endl;
//	getchar();
//	return 0;
//}

//void tcpFileShareTest() {
//	char c;
//	std::cin >> c;
//	Node n;
//	if (c == 's') { //s to play the rold of file sender
//		std::ifstream file("test.txt");
//		n.startTcpServer(46012); //in practice, use a different port each time
//		std::cout << "server started" << std::endl;
//		n.sendFile(file);
//		file.close();
//	}
//	else { //anything else to be receiver
//		std::ofstream file("output.txt");
//		sf::IpAddress serverIp = "192.168.1.87";
//		n.startClient(serverIp, 46012);
//		n.receiveFile(file);
//		file.close();
//	}
//}

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

//some test code that probably only works on my machine cus requires specific files

/*
std::cout << createDirectory("C:/Users/Tanner/Documents/BurgerKang") << std::endl;
for(std::string s : getFilepaths("C:/Users/Tanner/Documents/Python")){
	std::cout << s << std::endl;
	std::cout << getFileHash(s) << std::endl;
}

std::cout << filesDiffer("C:/Users/Tanner/Documents/BurgerKang/test.txt", "C:/Users/Tanner/Documents/BurgerKang/test.txt") << std::endl;
*/