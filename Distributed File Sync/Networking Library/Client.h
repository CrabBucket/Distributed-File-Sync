#pragma once
#include <iostream>
#include <SFML/Network.hpp>

class Client
{
private:
	sf::TcpSocket socket;
public:
	void test() {
		std::cout << "Hello world" << std::endl;
	}
};

