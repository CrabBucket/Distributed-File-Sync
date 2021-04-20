#include "Client.h"

Client::Client() {

}

Client::~Client() {

}

bool Client::connect(std::string ip, unsigned short port) {
	sf::Socket::Status status = socket.connect(ip, port, sf::seconds(30));
	if (status != sf::Socket::Done) {
		return false;
	}
	return true;
}

bool Client::send(std::string data) {
	sf::Socket::Status status = socket.send(data.c_str(), data.size());
	if (status != sf::Socket::Done) {
		return false;
	}
	return true;
}

std::string Client::receive(int buffer) { //default value of 1024
	std::size_t size;
	char* data = new char[buffer];
	if (socket.receive(data, buffer, size) != sf::Socket::Done) {
		return "";
	}
	std::string message(data, size);
	delete[] data;
	return message;
}