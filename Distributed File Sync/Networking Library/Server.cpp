#include "Server.h"

Server::Server() {

}

Server::~Server() {

}

bool Server::accept() {
	if (listener.accept(client) != sf::Socket::Done) {
		return false;
	}
	return true;
}

bool Server::listen(unsigned short port) {
	if (listener.listen(port) != sf::Socket::Done) {
		return false;
	}
	return true;
}

bool Server::send(std::string data) {
	sf::Socket::Status status = client.send(data.c_str(), data.size());
	if (status != sf::Socket::Done) {
		return false;
	}
	return true;
}

std::string Server::receive(int buffer) { //default value of 1024
	std::size_t size;
	std::cout << buffer << std::endl;
	char* data = new char[buffer];
	if (client.receive(data, buffer, size) != sf::Socket::Done) {
		return "null";
	}
	std::string message(data, size);
	delete[] data;
	return message;
}
