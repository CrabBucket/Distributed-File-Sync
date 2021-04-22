#pragma once
#include <iostream>
#include <string>
#include <SFML/Network.hpp>
#include <queue>

class Client
{
private:
	sf::TcpSocket socket;
	std::queue<sf::Packet*> todo;
public:
	Client();
	~Client();

	//times out after 30 seconds
	bool connect(std::string ip, unsigned short port);
	void connectAll(unsigned short port);

	//for sending basic strings
	bool sendString(std::string data);
	//for sending packets
	bool send(std::string);

	//for receiving basic strings
	std::string receiveString(int buffer = 1024);
	//for receiving packets
	bool receive();

	bool handle();

	int getTodoCount() const;
};

