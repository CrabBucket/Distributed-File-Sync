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
	std::set<sf::IpAddress> neighbors;
	std::wstring directory;

//mutexed stuff
	std::queue<UdpMessage*> todoUdp; //udp queue of unhandled packets
	std::mutex queueMutex;
	std::map<std::wstring, uint64_t> fileHashes; //table of file hashes
	std::mutex hashTableMutex;

	UdpMessage* tableManagerMessage = nullptr; //temporary pointer for carrying message to tableManagerDriver
	bool needToSendTable = false;
	bool receivedTable = false;
	bool needToReceiveCritiques = false;
	bool needToRequestFile = false;
	bool needToSendFile = false;
	std::mutex tableManagerMutex;

	std::vector<fileChangeData> requestQueue;
	std::mutex requestQueueMutex;

//helper functions
	//true if the message sent is your own
	bool isMyOwn(sf::IpAddress&);
	//safely delete UdpMessage object
	void disposeUdpMessage(UdpMessage*);
	//get pid without using it up
	sf::Uint8 getPacketID(sf::Packet&);
	//refreshes the local hash table, looks at shared folder
	void logNewFiles(std::wstring&);

	//debug functions
	void readResponseToArrival(UdpMessage*);
	void unknownPacket(UdpMessage*);


public:
	Node(std::wstring&);
	~Node();


//udp related
	bool listenUdp(unsigned short port); //initialize udp socket on selected port
	bool broadcast(sf::Packet& packet, unsigned short port); //broadcast to specific port
	bool broadcast(sf::Packet& packet); //broadcast to the same port listening for udp
	void collectUdpTraffic(sf::Time timeout = sf::Time::Zero); //catches udp traffic
	bool respondToArrival(sf::IpAddress); //acknowledge arrival of new node
	void logConnection(const sf::IpAddress&); //add ip to set of neighbors
	bool handleUdp(std::mutex&); //handle top UdpMessage in queue
	bool requestFileChange(fileChangeData&); //Attmept to add new request to requestQueue
	void dealWithHashTable(std::map<std::wstring, uint64_t>&, sf::IpAddress,bool);
	void requestFiles(std::vector<fileChangeData>, sf::IpAddress, std::mutex&);

//tcp related
	bool startClient(sf::IpAddress& ip, unsigned short port); //connect to tcp server
	void startTcpServer(unsigned short port); //initialize tcp server
	void gatherClients(); //accept any client connections = to size of neighbor table
	void sendFile(std::ifstream& file);
	void receiveFile(std::ofstream& file);
	bool negotiateTCPTransfer(unsigned short, fileChangeData);

//thread related
	//Drivers for threads
	void discoverDriver(); //discovers new nodes and udp traffic
	void handlerDriver(std::mutex&); //handle udp traffic from queue
	void tableManagerDriver(); //file hash table management


	//debug purposes
	void printConnections(); //print list of ip's of neighbors
};

