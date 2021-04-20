#pragma once
#include <iostream>
#include <string>
#include <SFML/Network.hpp>

//enum class ConnectionStatus { Connected, Failed, TimedOut};

class Server
{
private:
	sf::TcpListener listener;
	sf::TcpSocket client;
public:
	Server();
	~Server();

	bool accept();
	bool listen(unsigned short port);
	bool send(std::string data);
	std::string receive(int buffer = 1024);
};