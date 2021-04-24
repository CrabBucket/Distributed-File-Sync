#include "Node.h"

Node::Node() {}
Node::~Node() {}

bool Node::isMyOwn(sf::IpAddress& sender) {
	return sender.toString() == sf::IpAddress::getLocalAddress().toString();
}

void Node::disposeUdpMessage(UdpMessage* message) {
	delete message->packet;
	delete message;
}

sf::Uint8 Node::getPacketID(sf::Packet& packet) {
	char* ptr = (char*)packet.getData();
	std::cout << "Packet ID identified: " << (int) ptr[0] << std::endl;
	return ptr[0];
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
	if (udp.receive(*(message.packet), message.ip, message.port)) {
		if (isMyOwn(message.ip)) {
			return receiveUdp();
		}
		return true;
	}
	else {
		return false;
	}
}

void Node::collectArrivalResponses(sf::Time time) {
	sf::SocketSelector selector;
	selector.add(udp.socket);
	while (true) {
		if (selector.wait(time)) {
			sf::Packet packet;
			sf::IpAddress sender;
			unsigned short senderPort;
			if (udp.receive(packet, sender, senderPort)) {
				if (!isMyOwn(sender)) {
					sf::Uint8 pid = getPacketID(packet);
					std::cout << "Packet received with pid " << (int)pid << std::endl;
					if (pid == 0) {
						std::string message;
						packet >> pid >> message;
						std::cout << "message contents: " << message << std::endl;
						logConnection(sender);
						std::cout << "responding to arrival: " << respondToArrival(sender) << std::endl;
					}
					else { //pid other than 0
						logConnection(sender);
						UdpMessage* udpMessage = new UdpMessage();
						udpMessage->ip = sender;
						udpMessage->packet = new sf::Packet(packet);
						udpMessage->port = port;
						queueMutex.lock();
						todoUdp.push(udpMessage);
						queueMutex.unlock();
					}
				}
			}
		}
		else {
			std::cout << "Selector decided to not wait" << std::endl;
			break;
		}
	}
}

bool Node::respondToArrival(sf::IpAddress recipient) {
	sf::Packet packet;
	sf::Uint8 pid = 1;
	packet << pid;
	return udp.send(packet, recipient, port);
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

bool Node::handleUdp() {
	queueMutex.lock();
	if (todoUdp.empty()) {
		queueMutex.unlock();
		return false;
	}
	UdpMessage* message = todoUdp.front();
	todoUdp.pop();
	queueMutex.unlock();

	sf::Uint8 pid;
	*(message->packet) >> pid;
	std::cout << "received packet with pid " << (int)pid << " from " << message->ip << std::endl;
	disposeUdpMessage(message);
	return true;
}

void Node::discoverDriver() {
	sf::Packet packet;
	std::string message = "arrival";
	sf::Uint8 pid = 0;
	packet << pid << message;
	broadcast(packet);
	collectArrivalResponses();
}

void Node::handlerDriver() {
	while (true) {
		handleUdp();
	}
}

void Node::printConnections() {
	for (sf::IpAddress ip : neighbors) {
		std::cout << ip << std::endl;
	}
}