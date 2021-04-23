#pragma once
#include "Server.h"
#include "Client.h"
#include "UdpConnection.h"
#include <SFML/Network.hpp>
#include <set>
#include <iostream>

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
	std::queue<UdpMessage> todoUdp;

	void disposeUdpMessage(UdpMessage&);
	//bool receiveWithTimeout(sf::UdpSocket& socket, sf::Time& time);

public:
	Node();
	~Node();
	bool listenUdp(unsigned short port);
	bool broadcast(sf::Packet& packet, unsigned short port);
	bool broadcast(sf::Packet& packet);
	//receive 1 udp message
	bool receiveUdp();
	void collectArrivalResponses();
	bool respondToArrival(sf::IpAddress);
	void logConnection(const sf::IpAddress&);
	//void updateNeighborSet(sf::Packet& packet);
	void startTcpServer(unsigned short port);
	void gatherClients();
	bool startClient(sf::IpAddress& ip, unsigned short port);
	//void sendFile(File* file);
	//void receiveFile();

	void printConnections();
};

