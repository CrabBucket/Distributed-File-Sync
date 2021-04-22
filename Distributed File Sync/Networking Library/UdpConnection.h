#pragma once
#include <SFML/Network.hpp>

class UdpConnection
{
private:
	sf::UdpSocket socket;
public:
	UdpConnection();
	~UdpConnection();

	bool bind(unsigned short port);
	bool send(sf::Packet& packet, sf::IpAddress recipient, unsigned short port);
	bool receive(sf::Packet& packet, sf::IpAddress& sender, unsigned short& port);

};

