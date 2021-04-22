#include "UdpConnection.h"

UdpConnection::UdpConnection() {}
UdpConnection::~UdpConnection() {}

bool UdpConnection::bind(unsigned short port) {
	if (socket.bind(port) != sf::Socket::Done) {
		return false;
	}
	return true;
}

bool UdpConnection::send(sf::Packet& packet, sf::IpAddress recipient, unsigned short port) {
	if (socket.send(packet, recipient, port) != sf::Socket::Done)
	{
		return false;
	}
	return true;
}

bool UdpConnection::receive(sf::Packet& packet, sf::IpAddress& sender, unsigned short& port) {
	if (socket.receive(packet, sender, port) != sf::Socket::Done) {
		return false;
	}
	return true;
}