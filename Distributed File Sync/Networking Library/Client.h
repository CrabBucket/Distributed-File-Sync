#pragma once
#include <iostream>
#include <string>
#include <SFML/Network.hpp>

//enum class ConnectionStatus { Connected, Failed, TimedOut};

class Client
{
private:
	sf::TcpSocket socket;
public:
	Client();
	~Client();

	bool connect(std::string ip, unsigned short port);

	//for sending basic strings
	bool sendString(std::string data);
	//for sending packets
	bool send(std::string);

	//for receiving basic strings
	std::string receiveString(int buffer = 1024);
	//for receiving packets
	std::string receive();
};

