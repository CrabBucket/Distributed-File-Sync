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
	bool send(std::string data);
	std::string receive(int buffer = 1024); 
};

