#pragma once
#include "Server.h"
#include "Client.h"
#include "UdpConnection.h"
#include <SFML/Network.hpp>
#include <set>
#include <iostream>
#include <mutex>
#include <cstdint>

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

//mutexed stuff
	std::queue<UdpMessage*> todoUdp; //udp queue of unhandled packets
	std::mutex queueMutex;
	std::set<uint64_t> fileHashes; //table of file hashes
	std::mutex hashTableMutex;

	UdpMessage* tableManagerMessage = nullptr; //temporary pointer for carrying message to tableManagerDriver
	bool needToSendTable = false;
	bool needToReceiveTable = false;
	bool needToReceiveCritiques = false;
	bool needToRequestFile = false;
	bool needToSendFile = false;
	std::mutex tableManagerMutex;

	//true if the message sent is your own
	bool isMyOwn(sf::IpAddress&);
	//safely delete UdpMessage object (cannot be null)
	void disposeUdpMessage(UdpMessage*);
	//get pid without using it up
	sf::Uint8 getPacketID(sf::Packet&);

	//debug functions
	void readResponseToArrival(UdpMessage*);
	void unknownPacket(UdpMessage*);


public:
	Node();
	~Node();

//udp related
	bool listenUdp(unsigned short port);
	bool broadcast(sf::Packet& packet, unsigned short port);
	bool broadcast(sf::Packet& packet);
	void collectArrivalResponses(sf::Time timeout = sf::Time::Zero);
	bool respondToArrival(sf::IpAddress);
	void logConnection(const sf::IpAddress&);
	//void updateNeighborSet(sf::Packet& packet);
	void startTcpServer(unsigned short port);
	void gatherClients();
	bool startClient(sf::IpAddress& ip, unsigned short port);
	//void sendFile(File* file);
	//void receiveFile();
	bool handleUdp();

	//Drivers for threads
	void discoverDriver();
	void handlerDriver();
	void tableManagerDriver();

	void printConnections();
	void simulateDirectoryChange();

};

