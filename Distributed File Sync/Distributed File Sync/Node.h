#pragma once
#include "Server.h"
#include "Client.h"
#include "UdpConnection.h"
#include <SFML/Network.hpp>
#include <set>

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
	std::set<sf::IpAddress> neighbors;
	std::queue<UdpMessage> todoUdp;

	void disposeUdpMessage(UdpMessage&);

public:
	Node();
	~Node();
	bool listenUdp(unsigned short port);
	bool broadcast(sf::Packet& packet, unsigned short port);
	bool receiveUdp();
	void logConnection(const sf::IpAddress&);
	void updateNeighborSet();
	void startTcpServer();
	void startClient();
	void sendFile();
	void receiveFile();
};

