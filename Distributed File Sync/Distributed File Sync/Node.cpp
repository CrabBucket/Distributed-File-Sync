#include "Node.h"

sf::Packet& operator<<(sf::Packet& packet, std::map<std::wstring, uint64_t>& fileHashTable);
sf::Packet& operator>>(sf::Packet& packet, std::map<std::wstring, uint64_t>& fileHashTable);

Node::Node(std::wstring& folderPath) {
	directory = folderPath;
	for (std::wstring path : getFilepaths(folderPath)) {
		fileHashes[getRelativeToDocuments(path)] = getFileHash(path);
	}
}

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
	std::cout << "Packet ID identified: " << (int)ptr[0] << std::endl;
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
						std::map<std::wstring, uint64_t> foreignHashes;
						packet >> pid >> foreignHashes;
						logConnection(sender);
						std::cout << "responding to arrival: " << respondToArrival(sender) << std::endl;
						dealWithHashTable(foreignHashes, sender,true);
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
	packet << pid << fileHashes;
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
	if (dirLock.try_lock()) {
		if (!fileChangeBuf.empty()) {
			//update local file hash table
			for (fileChangeData& changeData : fileChangeBuf) {
				if (changeData.change != fileChangeType::Deletion) {
					fileHashes[getRelativeToDocuments(changeData.filePath)] = changeData.fileHash;
				}
				else {
					auto it = fileHashes.find(getRelativeToDocuments(changeData.filePath));
					if (it != fileHashes.end()) {
						fileHashes.erase(it);
					}
				}
			}
			//broadcast changes to other nodes
			sf::Packet dirChangesPacket;
			broadcast(dirChangesPacket << fileChangeBuf);
			//clear the buffer
			fileChangeBuf.clear();
		}
		dirLock.unlock();
	}
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
			tableManagerMutex.lock(); //lock
			receivedTable = true;
			tableManagerMessage = message;
			tableManagerMutex.unlock(); //unlock
			doDispose = false;
			break;
		}
		case 2: {
			//unused, used to be for receiving tables, see case 1
			break;
		}
		case 5: {
			auto packet = *(message->packet);
			sf::Uint8 packetPID;
			packet >> packetPID;
			bool abandon;
			fileChangeData fileChange;
			packet >> fileChange;
			unsigned short tcpNegotiationPort;
			packet >> tcpNegotiationPort;
			abandon = !std::filesystem::exists(getDocumentsPath() + fileChange.filePath);
			std::cout << "should I abdoned: " << abandon << std::endl;
			sf::Packet tcpDetails;

			tcpDetails << abandon;

			tcpDetails << fileChange;
			
			unsigned short tcpPort = 45016;

			tcpDetails << (sf::Uint16)tcpPort;
			
			std::cout << "ip: " << message->ip << " " << "port: " << tcpNegotiationPort << std::endl;
			udp.send(tcpDetails, message->ip, tcpNegotiationPort);
			if (abandon) {
				break;
			}
			std::ifstream file(getDocumentsPath() + fileChange.filePath);
			std::cout << "about to start server" << std::endl;
			this->startTcpServer(tcpPort);
			std::cout << "server started" << std::endl;
			this->sendFile(file);
			std::cout << "file sent" <<  std::endl;
			file.close();
			break;

		}
		case 6: {
			//dirLock.lock();
			auto fileChangePacket = *(message->packet);
			std::vector<fileChangeData> fileChanges;
			fileChangePacket >> fileChanges;
			requestFiles(fileChanges, message->ip, dirLock);
			std::cout << "for loop reltionship ENDED" << std::endl;
			break;
		}
		default: { //packet with unknown pid
			unknownPacket(message);
			break;
		}
	}
	//safely discard UdpMessage object
	if (doDispose)
		disposeUdpMessage(message);
	return true;
}

bool Node::negotiateTCPTransfer(unsigned short tcpNegotiationPort,fileChangeData fileChange) {
	std::cout << "tcpnegotiaiiion port: " << tcpNegotiationPort << std::endl;
	UdpConnection tcpNegotiationCon;
	tcpNegotiationCon.bind(tcpNegotiationPort);
	std::cout << "tcpNegotiationCon successfully binded" << std::endl;
	sf::SocketSelector selector;
	selector.add(tcpNegotiationCon.socket);
	if (selector.wait(sf::Time::Zero)) {
		std::cout << "caught some traffic" << std::endl;
		sf::Packet packet;
		sf::IpAddress sender;
		unsigned short senderPort;
		unsigned short tcpPort;
		//gather packet
		std::cout << "about to wait to receive" << std::endl;
		if (tcpNegotiationCon.receive(packet, sender, senderPort)) {
			std::cout << "negotiation received" << std::endl;
			bool abandon;
			fileChangeData fileChange;
			packet >> abandon;
			packet >> fileChange;
			packet >> tcpPort;
			if (abandon) {
				std::cout << "abandoning" << std::endl;
				return false;
			}
			std::wcout << "acquiring dirs for: " << getDocumentsPath() + fileChange.filePath << std::endl;
			acquireDirectories(getDocumentsPath() + fileChange.filePath);
			std::ofstream file(getDocumentsPath() + fileChange.filePath);
			std::cout << "about to start client" << std::endl;
			this->startClient(sender, tcpPort);
			std::cout << "client started" << std::endl;
			this->receiveFile(file);
			std::cout << "file received" << std::endl;
			file.close();


		}
	}
	return true;
}			

bool Node::requestFileChange(fileChangeData& changeData) {
	for (const fileChangeData& fcd : requestQueue) {
		if (changeData == fcd)
			return false; //reuest has already been queued up
	}
	requestQueue.push_back(changeData);
	return true;
}

void Node::discoverDriver() {
	//send out arrival announcement
	sf::Packet packet;
	//std::string message = "arrival";
	sf::Uint8 pid = 0;
	//packet << pid << message;
	packet << pid << fileHashes;
	broadcast(packet);
	//being collecting udp traffic
	collectUdpTraffic();
}

void Node::handlerDriver(std::mutex& dirLock) {
	while (true) {
		handleUdp(dirLock);
	}
}

void Node::tableManagerDriver() {
	UdpMessage* message = nullptr;
	while (true) {
		tableManagerMutex.lock(); //lock
		//send table
		if (needToSendTable) {

		}
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
			std::map<std::wstring, uint64_t> table;
			*(message->packet) >> table;
			dealWithHashTable(table, message->ip,false);
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
	sf::Uint8 pid = getPacketID(*(message->packet));
	std::cout << "received packet with pid " << (int)pid << " from " << message->ip << std::endl;
}

void Node::unknownPacket(UdpMessage* message) {
	sf::Uint8 pid;
	*(message->packet) >> pid;
	std::cout << "received packet with pid " << (int)pid << " from " << message->ip << std::endl;
}


void Node::requestFiles(std::vector<fileChangeData> fileChanges, sf::IpAddress server, std::mutex& dirLock) {
	for (auto fileChange : fileChanges) {
		std::cout << "there" << std::endl;
		dirLock.lock();
		sf::Packet packet;

		switch (fileChange.change) {
		case fileChangeType::Edit:
			//We need to negotiate a tcp file transfer with another client to get this file.

		case fileChangeType::Addition:
			std::wcout << L"received file path" << fileChange.filePath << std::endl;
			std::wcout << L"my file hash" << fileChange.fileHash << std::endl;
			std::wcout << L"received file hash" << getFileHash(getDocumentsPath() + fileChange.filePath) << std::endl;
			if ((std::filesystem::exists(getDocumentsPath() + fileChange.filePath) && (fileChange.fileHash == getFileHash(getDocumentsPath() + fileChange.filePath)))) {
				dirLock.unlock();
				std::cout << "about to continue" << std::endl;
				continue;
			}
			packet << (sf::Uint8)5;
			packet << fileChange;
			packet << (sf::Uint16)25565;
			//We need to negotiate a tcp file transfer with anotehr client to get this.
			std::cout << "about to send packet for case 5" << std::endl;
			udp.send(packet, server, port);
			negotiateTCPTransfer(25565, fileChange);
			dirLock.unlock();
			break;
		case fileChangeType::Deletion:
			//We check if the file exists and if it does we delete it.
			deleteFile(getDocumentsPath() + fileChange.filePath);
			dirLock.unlock();
			break;

		}
	}
}

void Node::dealWithHashTable(std::map<std::wstring, uint64_t>& table, sf::IpAddress sender, bool ignoreEdits) {
	//CURRENTLY JSUT PRINTS TABLE CONTENTS TO CONSOLE
	std::cout << "Received hash table" << std::endl;
	for (std::pair<std::wstring, uint64_t> entry : table) {
		std::wcout << entry.first << L" " << entry.second << std::endl;
	}
	auto dirChanges = getDirectoryChanges(directory.data(), table);
	printChanges(dirChanges);
	for (auto i = 0; i < dirChanges.size(); ++i) {
		switch (dirChanges[i].change) {
		case fileChangeType::Addition:
			break;
		case fileChangeType::Edit:
			if (ignoreEdits) {
				dirChanges.erase(dirChanges.begin() + i);
			}
			break;
		case fileChangeType::Deletion:
			dirChanges.erase(dirChanges.begin() + i);
			break;
		
		}
	}
	requestFiles(dirChanges)

}

//custom packet operator for accepting hash tables
sf::Packet& operator<<(sf::Packet& packet, std::map<std::wstring, uint64_t>& fileHashTable) {
	sf::Uint16 size = fileHashTable.size();
	packet << size;
	for (std::pair<std::wstring, uint64_t> entry : fileHashTable) {
		packet << entry.first << (sf::Uint64)entry.second;
	}
	return packet;
}

//custom packet operator for returning hash tables
sf::Packet& operator>>(sf::Packet& packet, std::map<std::wstring, uint64_t>& fileHashTable) {
	sf::Uint16 size;
	std::wstring filepath;
	uint64_t hash;

	packet >> size;
	for (sf::Uint16 i = 0; i < size; i++) {
		packet >> filepath >> hash;
		fileHashTable[filepath] = hash;
	}
	return packet;
}