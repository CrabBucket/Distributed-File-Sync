#pragma once
#include "Server.h"
#include "Client.h"
#include "UdpConnection.h"
#include <SFML/Network.hpp>
#include <set>
#include <iostream>
#include <mutex>
#include <cstdint>

struct UdpMessage {
	sf::Packet* packet = nullptr;
	sf::IpAddress ip;
	unsigned short port;
};

class Node
{
private:
	Server tcpServer;
	Client tcpClient;
	UdpConnection udp;
	unsigned short port;
	std::set<sf::IpAddress> neighbors;

	//mutexed stuff
	std::queue<UdpMessage*> todoUdp;
	std::mutex queueMutex;
	std::set<uint64_t> fileHashes;
	std::mutex hashTableMutex;

	UdpMessage* tableManagerMessage = nullptr;
	bool needToSendTable = false;
	bool needToReceiveTable = false;
	bool needToReceiveCritiques = false;
	bool needToRequestFile = false;
	bool needToSendFile = false;
	std::mutex tableManagerMutex;

	//true if the message send is yourself
	bool isMyOwn(sf::IpAddress&);
	void disposeUdpMessage(UdpMessage*);
	sf::Uint8 getPacketID(sf::Packet&);
	//bool receiveWithTimeout(sf::UdpSocket& socket, sf::Time& time);

	void readResponseToArrival(UdpMessage*);
	void unknownPacket(UdpMessage*);


public:
	Node();
	~Node();
	bool listenUdp(unsigned short port);
	bool broadcast(sf::Packet& packet, unsigned short port);
	bool broadcast(sf::Packet& packet);
	//receive 1 udp message
	bool receiveUdp();
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

