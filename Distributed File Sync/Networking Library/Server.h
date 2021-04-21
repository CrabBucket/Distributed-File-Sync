#pragma once
#include <iostream>
#include <string>
#include <SFML/Network.hpp>
#include <map>
#include <utility>
#include <queue>
#include <vector>

class Server
{
private:
	sf::TcpListener listener;
	std::map<sf::IpAddress, sf::TcpSocket*> clients;
	std::queue<sf::Packet*> todo;
public:
	Server();
	~Server();

	bool accept();
	bool listen(unsigned short port);
	//for sending basic strings
	bool send(const std::string& data, const sf::IpAddress&);
	//for sending packets
	bool send(sf::Packet&, const sf::IpAddress&);

	//for receiving basic strings
	std::string receiveString(const sf::IpAddress& source, int buffer = 1024);
	//for receiving packets
	bool receive(const sf::IpAddress& source);

	bool handle();

	int getTodoCount() const;
	std::vector<sf::IpAddress> getClientIps();
};