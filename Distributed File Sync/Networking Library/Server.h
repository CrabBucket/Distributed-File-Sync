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

	//read and return string of N characters from the given file handle
	std::string getNBytes(std::ifstream&, std::size_t startPosition, int n);

public:
	Server();
	~Server();

	//accept a client if they send a request, blocking function
	sf::IpAddress accept();
	//accept n clients with 10 second timeout, blocking function
	void accept(int n);
	//starts server
	bool listen(unsigned short port);
	//for sending basic strings
	bool send(const std::string& data, const sf::IpAddress&);
	//for sending packets
	bool send(sf::Packet&, const sf::IpAddress&);

	//for receiving basic strings
	std::string receiveString(const sf::IpAddress& source, int buffer = 1024);
	//for receiving packets
	bool receive(sf::Packet&, const sf::IpAddress& source);

	//echo's front packet in queue back to sender
	bool handle();

	//get length of todo queue
	int getTodoCount() const;

	//get vector of all known client Ips
	std::vector<sf::IpAddress> getClientIps();
};