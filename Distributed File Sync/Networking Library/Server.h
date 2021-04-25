#pragma once
#include <iostream>
#include <string>
#include <SFML/Network.hpp>
#include <map>
#include <utility>
#include <queue>
#include <vector>
#include <fstream>

class Server
{
private:
	std::map<sf::IpAddress, sf::TcpSocket*> clients;
	std::queue<sf::Packet*> todo;
	sf::TcpListener listener;

	std::string getNBytes(std::ifstream&, std::size_t startPosition, int n);

public:
	Server();
	~Server();

	sf::IpAddress accept();
	void accept(int n);
	bool listen(unsigned short port);
	//for sending basic strings
	bool send(const std::string& data, const sf::IpAddress&);
	//for sending packets
	bool send(sf::Packet&, const sf::IpAddress&);

	//for receiving basic strings
	std::string receiveString(const sf::IpAddress& source, int buffer = 1024);
	//for receiving packets
	bool receive(sf::Packet&, const sf::IpAddress& source);

	bool handle();

	int getTodoCount() const;
	std::vector<sf::IpAddress> getClientIps();

	//void sendFile(std::ifstream&, std::vector<sf::IpAddress>);
};