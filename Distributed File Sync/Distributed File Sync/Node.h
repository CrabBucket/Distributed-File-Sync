#pragma once
#include "Server.h"
#include "Client.h"
#include "UdpConnection.h"
#include "DirectoryMonitor.h"
#include <SFML/Network.hpp>
#include <set>
#include <iostream>
#include <mutex>
#include <cstdint>
#include "fileChangePacket.h"

//Packaged udp info into one object
struct UdpMessage {
	sf::Packet* packet = nullptr; //pointer to packet
	sf::IpAddress ip; //sender of packet
	unsigned short port; //port the data was received on
};

class Node
{
private:
	Server tcpServer; //tcp server socket
	Client tcpClient; //tcp client socket
	UdpConnection udp; //udp socket
	unsigned short port; //udp port
	std::wstring directory; //path of sync'd directory
	

//mutexed stuff
	std::queue<UdpMessage*> todoUdp; //udp queue of unhandled packets
	std::mutex queueMutex;
	std::map<std::wstring, uint64_t> fileHashes; //table of file hashes
	std::mutex* dirLock;

	UdpMessage* tableManagerMessage = nullptr; //temporary pointer for carrying message to tableManagerDriver
	bool receivedTable = false;
	std::mutex tableManagerMutex;


//helper functions
	//true if the message sent is your own
	bool isMyOwn(sf::IpAddress&);
	//safely delete UdpMessage object
	void disposeUdpMessage(UdpMessage*);
	//get pid without using it up
	sf::Uint8 getPacketID(sf::Packet&);

	//debug functions
	void readResponseToArrival(UdpMessage*);
	void unknownPacket(UdpMessage*);


public:
	Node(std::wstring&);
	~Node();

	void setDirLock(std::mutex&);  //set the dirLock member variable

//udp related
	bool listenUdp(unsigned short port); //initialize udp socket on selected port
	bool broadcast(sf::Packet& packet, unsigned short port); //broadcast to specific port
	bool broadcast(sf::Packet& packet); //broadcast to the same port listening for udp
	void collectUdpTraffic(sf::Time timeout = sf::Time::Zero); //catches udp traffic
	bool respondToArrival(sf::IpAddress); //acknowledge arrival of new node
	bool handleUdp(); //handle top UdpMessage in queue
	void dealWithHashTable(std::map<std::wstring, uint64_t>&, sf::IpAddress,bool); //process foreign hashtable
	void requestFiles(std::vector<fileChangeData>, sf::IpAddress);

//tcp related
	bool startClient(sf::IpAddress& ip, unsigned short port); //connect to tcp server
	void startTcpServer(unsigned short port); //initialize tcp server
	void sendFile(std::ifstream& file); //send file over tcp
	void receiveFile(std::ofstream& file); //receive file over tcp
	bool negotiateTCPTransfer(unsigned short, fileChangeData, sf::Packet&, sf::IpAddress&);

//thread related
	//Drivers for threads
	void discoverDriver(); //discovers new nodes and udp traffic
	void handlerDriver(); //handle udp traffic from queue
	void tableManagerDriver(); //file hash table management
};

