#include "Node.h"

Node::Node() {}
Node::~Node() {}

void Node::disposeUdpMessage(UdpMessage& message) {
	delete message.packet;
}

bool Node::listenUdp(unsigned short port) {
	this->port = port;
	return udp.bind(port);
}

bool Node::broadcast(sf::Packet& packet, unsigned short port) {
	return udp.send(packet, sf::IpAddress::Broadcast, port);
}

bool Node::broadcast(sf::Packet& packet) {
	return udp.send(packet, sf::IpAddress::Broadcast, port);
}

bool Node::receiveUdp() {
	UdpMessage message;
	message.packet = new sf::Packet();
	return udp.receive(*(message.packet), message.ip, message.port);
}

void Node::collectArrivalResponses() {
	sf::SocketSelector selector;
	selector.add(udp.socket);
	while (true) {
		if (selector.wait(sf::seconds(10.f))) {
			sf::Packet packet;
			sf::IpAddress sender;
			unsigned short senderPort;
			if (udp.receive(packet, sender, senderPort)) {
				std::string message;
				sf::Uint8 pid;
				packet >> pid >> message;
				std::cout << "Packet received with pid " << pid << std::endl;
				if (pid == 0) {
					std::cout << "message contents: " << message << std::endl;
					logConnection(sender);
				}
			}
		}
		else {
			break;
		}
	}
}

void Node::logConnection(const sf::IpAddress& neighbor) {
	neighbors.insert(neighbor);
}

//void Node::updateNeighborSet(sf::Packet& packet) {
//	std::set<sf::IpAddress> alive;
//	for (sf::IpAddress neighbor : neighbors) {
//		udp.send(packet, neighbor, port);
//	}
//
//}
//
//bool receiveWithTimeout(sf::UdpSocket& socket, sf::Time& timeout) {
//	sf::SocketSelector selector;
//	selector.add(socket);
//	if (selector.wait(timeout)) {
//		if (socket.receive() != sf::Socket::Done) {
//			return false;
//		}
//		return true;
//	}
//	else {
//		return false;
//	}
//}

void Node::startTcpServer(unsigned short port) {
	tcpServer.listen(port);
}

void Node::gatherClients() {
	tcpServer.accept(neighbors.size());
}

bool Node::startClient(sf::IpAddress& ip, unsigned short port) {
	return tcpClient.connect(ip.toString(), port);
}

//void Node::sendFile(File* file) {
//	std::map<sf::IpAddress, int> positions;
//	//initialize every as having nothing from the new file
//	for (sf::IpAddress ip : neighbors) {
//		positions[ip] = 0;
//	}
//
//	tcpServer.sendFile(file, positions);
//}
//
//void receiveFile();


void Node::printConnections() {
	for (sf::IpAddress ip : neighbors) {
		std::cout << ip << std::endl;
	}
}