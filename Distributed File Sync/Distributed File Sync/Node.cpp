#include "Node.h"

Node::Node() {}
Node::~Node() {}

void Node::disposeUdpMessage(UdpMessage& message) {
	delete message.packet;
}

bool Node::listenUdp(unsigned short port) {
	return udp.bind(port);
}

bool Node::broadcast(sf::Packet& packet, unsigned short port) {
	return udp.send(packet, sf::IpAddress::Broadcast, port);
}

bool Node::receiveUdp() {
	UdpMessage message;
	message.packet = new sf::Packet();
	return udp.receive(*(message.packet), message.ip, message.port);
}

void logConnection(const sf::IpAddress&);
void updateNeighborSet();
void startTcpServer();
void startClient();
void sendFile();
void receiveFile();