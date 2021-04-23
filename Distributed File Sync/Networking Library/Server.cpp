#include "Server.h"

Server::Server() {

}

Server::~Server() {

}

bool Server::accept() {
	sf::TcpSocket* client = new sf::TcpSocket();
	if (listener.accept(*client) != sf::Socket::Done) {
		return false;
	}
	clients[client->getRemoteAddress()] = client;
	return true;
}

void Server::accept(int n) {
	sf::SocketSelector selector;
	selector.add(listener);
	for (int i = 0; i < n; i++) {
		if (selector.wait(sf::seconds(10.f)))
		{
			accept();
		}
		else {
			break;
		}
	}
}

bool Server::listen(unsigned short port) {
	if (listener.listen(port) != sf::Socket::Done) {
		return false;
	}
	return true;
}

//for sending basic strings
bool Server::send(const std::string& data, const sf::IpAddress& dest) {
	sf::Socket::Status status = clients[dest]->send(data.c_str(), data.size());
	if (status != sf::Socket::Done) {
		return false;
	}
	return true;
}
//for sending packets
bool Server::send(sf::Packet& packet, const sf::IpAddress& dest) {
	sf::Socket::Status status = clients[dest]->send(packet);
	if (status != sf::Socket::Done) {
		return false;
	}
	return true;
}

//for receiving basic strings
std::string Server::receiveString(const sf::IpAddress& source, int buffer) { //default value of 1024
	std::size_t size;
	std::cout << buffer << std::endl;
	char* data = new char[buffer];
	if (clients[source]->receive(data, buffer, size) != sf::Socket::Done) {
		return "null";
	}
	std::string message(data, size);
	delete[] data;
	return message;
}

//for receiving packets
bool Server::receive(const sf::IpAddress& source) {
	sf::Packet* packet = new sf::Packet();
	if (clients[source]->receive(*packet) != sf::Socket::Done) {
		return false;
	}
	todo.push(packet);
	std::cout << "packet received from " << source << std::endl;
	return true;
}

bool Server::handle() {
	if (todo.empty()) return false;

	sf::Packet* packet = todo.front();
	std::string ip;
	(*packet) >> ip;
	if (send(*packet, ip)) {
		std::cout << "sent massage to " << ip << std::endl;
		todo.pop();
		delete packet;
		return true;
	}
	return false;
}

int Server::getTodoCount() const {
	return todo.size();
}

std::vector<sf::IpAddress> Server::getClientIps() {
	std::vector<sf::IpAddress> ips;
	std::map<sf::IpAddress, sf::TcpSocket*>::iterator it = clients.begin();
	while (it != clients.end()) {
		ips.push_back(it->first);
		it++;
	}
	return ips;
}

std::string Server::getNBytes(std::ifstream& file, std::size_t startPosition, int n) {
	std::string buffer = "";
	char c;

	file.seekg(startPosition);
	for (int i = 0; i < n; i++) {
		if (file.get(c)) {
			buffer += c;
		}
		else {
			break;
		}
	}
	return buffer;
}