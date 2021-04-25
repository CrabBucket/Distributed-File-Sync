#include "Node.h"


sf::Packet& operator<<(sf::Packet& packet, std::set<uint64_t>& fileHashTable);
sf::Packet& operator>>(sf::Packet& packet, std::set<uint64_t>& fileHashTable);

Node::Node() {}
Node::~Node() {}

bool Node::isMyOwn(sf::IpAddress& sender) {
	return sender.toString() == sf::IpAddress::getLocalAddress().toString();
}

void Node::disposeUdpMessage(UdpMessage* message) {
	if (message != nullptr) {
		if (message->packet != nullptr) {
			delete message->packet;
		}
		delete message;
	}
}

//packet must start with pid
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

void Node::collectUdpTraffic(sf::Time time) {
	sf::SocketSelector selector;
	selector.add(udp.socket);
	while (true) {
		//wait until traffic is received
		if (selector.wait(time)) {
			sf::Packet packet;
			sf::IpAddress sender;
			unsigned short senderPort;
			//gather packet
			if (udp.receive(packet, sender, senderPort)) {
				//ignore your own packets (thanks udp :P)
				if (!isMyOwn(sender)) {
					sf::Uint8 pid = getPacketID(packet);
					std::cout << "Packet received with pid " << (int)pid << std::endl;
					//arrival packet
					if (pid == 0) {
						std::string message;
						packet >> pid >> message;
						std::cout << "message contents: " << message << std::endl;
						logConnection(sender);
						std::cout << "responding to arrival: " << respondToArrival(sender) << std::endl;
					}
					//pid other than 0
					else {
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
			//should not get here
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

void Node::startTcpServer(unsigned short port) {
	tcpServer.listen(port);
}

void Node::gatherClients() {
	tcpServer.accept(neighbors.size());
}

bool Node::startClient(sf::IpAddress& ip, unsigned short port) {
	return tcpClient.connect(ip.toString(), port);
}

void Node::sendFile(std::ifstream& file) {
	sf::IpAddress clientIp = tcpServer.accept();
	//get file length
	file.seekg(0, file.end);
	unsigned int length = file.tellg();
	file.seekg(0, file.beg);
	unsigned int chunkSize = 1024;
	
	sf::Uint32 pos = 0;
	while (pos < length) {
		sf::Packet packet;
		sf::Uint8 pid = 100; //100 means sending file data
		packet << pid;
		packet << pos;
		char* buffer = new char[chunkSize];
		file.read(buffer, chunkSize);
		std::string contents(buffer, file.gcount());
		//std::cout << contents << std::endl << std::endl << std::endl;
		delete[] buffer;
		packet << contents;
		tcpServer.send(packet, clientIp);

		//gather response of what position the client is at
		sf::Packet response;
		if (tcpServer.receive(response, clientIp)) {
			sf::Uint32 clientPos;
			response >> clientPos;
			pos = clientPos; //update pos to the pos the client said he was at
		}
		else {
			std::cout << "Client Response NULLPTR freak out" << std::endl;
		}
	}
	//exit sequence
	std::cout << "sending finish notification packet" << std::endl;
	sf::Packet endPacket;
	sf::Uint8 pid = 101; //101 means done sending file data
	endPacket << pid;
	std::cout << "packet sent?: " << tcpServer.send(endPacket, clientIp) << std::endl;
	std::cout << tcpServer.receive(endPacket, clientIp) << std::endl;
}

void Node::receiveFile(std::ofstream& file) {
	sf::Uint8 pid;
	sf::Uint32 pos = 0, serverPos;
	std::string contents;

	while (true) {
		sf::Packet packet;
		std::cout << "receiving packet: ";
		sf::Socket::Status status = tcpClient.receive(packet);
		if (status == sf::Socket::Done) {
			std::cout << "successfully received" << std::endl;
			packet >> pid;
			std::cout << "Pid received: " << pid << std::endl;
			if (pid == 101) break; //101 is end of file
			packet >> serverPos >> contents;
			//std::cout << contents << std::endl;

			//if server is on the same page as the client
			if (pos == serverPos) {
				file << contents;
				pos = file.tellp();
				std::cout << "file written to, new pos: " << pos << std::endl;
			}

			//respond to server with your new position
			sf::Packet response;
			response << pos;
			if (tcpClient.send(response))
				std::cout << "response sent" << std::endl;
			else
				std::cout << "response NOT sent" << std::endl;
		}
		else if (status == sf::Socket::Disconnected) {
			std::cout << "Server disconnected" << std::endl;
		}
		else {
			std::cout << "Client Received NULLPTR freak out" << std::endl;
		}
	}
	//send empty packet to let the server know to disconnect
	sf::Packet endPacket;
	tcpClient.send(endPacket);
}

bool Node::handleUdp(std::mutex& dirLock) {
	//grab work from queue
	queueMutex.lock(); //lock
	if (todoUdp.empty()) {
		queueMutex.unlock();
		return false;
	}
	UdpMessage* message = todoUdp.front();
	todoUdp.pop();
	queueMutex.unlock(); //unlock

	//do work
	bool doDispose = true;
	sf::Uint8 pid = getPacketID(*(message->packet));
	switch (pid) {
		case 1: {
			readResponseToArrival(message); 
			break;
		}
		case 2: {
			tableManagerMutex.lock(); //lock
			receivedTable = true; 
			tableManagerMessage = message;
			tableManagerMutex.unlock(); //unlock
			doDispose = false;
			break;
		}
		case 3: {
			dirLock.lock();
			auto fileChangePacket = *(message->packet);
			sf::Uint8 pid; 
			fileChangePacket >> pid;
			std::vector<fileChangeData> fileChanges;
			fileChangePacket >> fileChanges;
			for

		}
		default: { //packet with unknown pid
			unknownPacket(message); 
			break;
		}
	}
	//safely discard UdpMessage object
	if(doDispose)
		disposeUdpMessage(message);
	return true;
}

void Node::discoverDriver() {
	//send out arrival announcement
	sf::Packet packet;
	std::string message = "arrival";
	sf::Uint8 pid = 0;
	packet << pid << message;
	broadcast(packet);
	//being collecting udp traffic
	collectUdpTraffic();
}

void Node::handlerDriver(std::mutex& dirLock) {
	while (true) {
		handleUdp();
	}
}

void Node::tableManagerDriver() {
	UdpMessage* message = nullptr;
	while (true) {
		tableManagerMutex.lock(); //lock
		//send table

		//receive table
		if (receivedTable) {
			receivedTable = false;
			message = tableManagerMessage;
			tableManagerMessage = nullptr;
			tableManagerMutex.unlock(); //unlock
			//unpack packet
			sf::Uint8 pid;
			*(message->packet) >> pid;
			std::cout << "received packet with pid " << (int)pid << " from " << message->ip << std::endl;
			std::set<uint64_t> table;
			*(message->packet) >> table;
			//CURRENTLY JSUT PRINTS TABLE CONTENTS TO CONSOLE
			for (uint64_t hash : table) {
				std::cout << hash << std::endl;
			}
		}

		//receive critiques

		//request file

		//send file

		else {
			tableManagerMutex.unlock(); //unlock
		}
		//delete UdpMessage object if it's not null
		if (message != nullptr) {
			disposeUdpMessage(message);
			message = nullptr;
		}
	}
}

void Node::printConnections() {
	for (sf::IpAddress ip : neighbors) {
		std::cout << ip << std::endl;
	}
}

void Node::readResponseToArrival(UdpMessage* message) {
	std::cout << "received packet with pid 1 from " << message->ip << std::endl;
}

void Node::unknownPacket(UdpMessage* message) {
	sf::Uint8 pid;
	*(message->packet) >> pid;
	std::cout << "received packet with pid " << (int)pid << " from " << message->ip << std::endl;
}

//custom packet operator for accepting hash tables
sf::Packet& operator<<(sf::Packet& packet, std::set<uint64_t>& fileHashTable) {
	sf::Uint16 size = fileHashTable.size();
	packet << size;
	for (sf::Uint64 hash : fileHashTable) {
		packet << hash;
	}
	return packet;
}

//custom packet operator for returning hash tables
sf::Packet& operator>>(sf::Packet& packet, std::set<uint64_t>& fileHashTable) {
	sf::Uint16 size;
	uint64_t hash;

	packet >> size;
	for (sf::Uint16 i = 0; i < size; i++) {
		packet >> hash;
		fileHashTable.insert(hash);
	}
	return packet;
}
