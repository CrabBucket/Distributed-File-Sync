#include "Node.h"

sf::Packet& operator<<(sf::Packet& packet, std::map<std::wstring, uint64_t>& fileHashTable);
sf::Packet& operator>>(sf::Packet& packet, std::map<std::wstring, uint64_t>& fileHashTable);

Node::Node(std::wstring& folderPath) {
	directory = folderPath;
	for (std::wstring path : getFilepaths(folderPath)) {
		fileHashes[getRelativeTo(path, directory)] = getFileHash(path);
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

void Node::setDirLock(std::mutex& dirLock) {
	this->dirLock = &dirLock;
}

//packet must start with pid
sf::Uint8 Node::getPacketID(sf::Packet& packet) {
	char* ptr = (char*)packet.getData();
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
					//arrival packet
					if (pid == 0) {
						std::map<std::wstring, uint64_t> foreignHashes;
						packet >> pid >> foreignHashes;
						respondToArrival(sender);
						dealWithHashTable(foreignHashes, sender,true);
					}
					//pid other than 0
					else {
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

void Node::startTcpServer(unsigned short port) {
	tcpServer.listen(port);
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
		char* buffer = new char[chunkSize];
		file.read(buffer, chunkSize);
		std::string contents(buffer, file.gcount());
		delete[] buffer;
		packet << pos << contents.size() << contents;
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
	sf::Packet endPacket;
	sf::Uint8 pid = 101; //101 means done sending file data
	endPacket << pid;
}

void Node::receiveFile(std::ofstream& file) {
	sf::Uint8 pid;
	sf::Uint32 pos = 0, serverPos, length;
	std::string contents;

	while (true) {
		sf::Packet packet;
		sf::Socket::Status status = tcpClient.receive(packet);
		if (status == sf::Socket::Done) {
			packet >> pid;
			if (pid == 101) break; //101 is end of file
			packet >> serverPos >> length >> contents;

			//if server is on the same page as the client
			if (pos == serverPos) {
				//file << contents;
				file.write(contents.data(), length);
				pos = file.tellp();
				std::cout << "file written to, new pos: " << pos << std::endl;
			}

			//respond to server with your new position
			sf::Packet response;
			response << pos;
			tcpClient.send(response);
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

bool Node::handleUdp() {
	if (dirLock->try_lock()) {
		if (!fileChangeBuf.empty()) {
			//update local file hash table
			for (fileChangeData& changeData : fileChangeBuf) {
				if (changeData.change != fileChangeType::Deletion) {
					fileHashes[changeData.filePath] = changeData.fileHash;
				}
				else {
					auto it = fileHashes.find(changeData.filePath);
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
		dirLock->unlock();
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

			//We read the packet and get the nessecary information out (file change and tcpNegotiationPort)
			auto packet = *(message->packet);
			sf::Uint8 packetPID;
			packet >> packetPID;
			bool abandon;
			fileChangeData fileChange;
			packet >> fileChange;
			unsigned short tcpNegotiationPort;
			packet >> tcpNegotiationPort;

			abandon = !std::filesystem::exists(directory + L'\\' + fileChange.filePath);
			
			//Creating a packet with the TCP connection details to be sent to the client requesting the file.
			sf::Packet tcpDetails;

			tcpDetails << abandon;

			tcpDetails << fileChange;
			
			unsigned short tcpPort = 45016;

			tcpDetails << (sf::Uint16)tcpPort;

			//We send the packet to the client on the tcpNegotiationPort
			udp.send(tcpDetails, message->ip, tcpNegotiationPort);
			//If we do not have the file we do not proceed with creating the TCP server.
			if (abandon) {
				break;
			}
			std::ifstream file(directory + L'\\' + fileChange.filePath, std::ios::in | std::ios::binary);
			//We start the tcp server send the file and then close the filestream.
			this->startTcpServer(tcpPort);
			this->sendFile(file);
			std::cout << "file sent" <<  std::endl;
			file.close();
			break;

		}
			  //CASE 6: 
		case 6: {  //Case 6 occurs when another node on the network has an update in the directory, they broadcast a packet with PID 6.
			//dirLock.lock();
			//We get the fileChanges that need to be synced in the directory.
			auto fileChangePacket = *(message->packet);
			std::vector<fileChangeData> fileChanges;
			fileChangePacket >> fileChanges;
			//We request the files from the node who notifed us of the change.
			requestFiles(fileChanges, message->ip);
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

//Negotiate TCPTransfer is a function that communicates with another node to setup a tcp connection between two nodes
bool Node::negotiateTCPTransfer(unsigned short tcpNegotiationPort,fileChangeData fileChange, sf::Packet& packetP, sf::IpAddress& server) {

	//We create udp connection which we bind to the negotiation port.
	UdpConnection tcpNegotiationCon;
	tcpNegotiationCon.bind(tcpNegotiationPort);

	//We send the other node the port information that this node is about to start listening on.
	udp.send(packetP, server, port);

	sf::Packet packet;
	sf::IpAddress sender;
	unsigned short senderPort;
	unsigned short tcpPort;
	//gather packet
	//We listen for a node on the correct port and determine how to proceed with the TCP file transfer
	if (tcpNegotiationCon.receive(packet, sender, senderPort)) {
		bool abandon;
		fileChangeData fileChange;
		packet >> abandon;
		packet >> fileChange;
		packet >> tcpPort;
		//If the server has decided to not send the file our client just stops trying to get the file and lets itself get out of sync.
		if (abandon) {
			return false;
		}
		acquireDirectories(directory + L'\\' + fileChange.filePath);
		std::ofstream file(directory + L'\\' + fileChange.filePath, std::ios::out | std::ios::binary);

		//We start the TCP client and receieve the requested file over the TCP connection.
		this->startClient(sender, tcpPort);
		this->receiveFile(file);
		std::cout << "file received" << std::endl;
		file.close();


	}
	return true;
}			

void Node::discoverDriver() {
	//send out arrival announcement
	sf::Packet packet;
	sf::Uint8 pid = 0;
	packet << pid << fileHashes;
	broadcast(packet);
	//being collecting udp traffic
	collectUdpTraffic();
}

void Node::handlerDriver() {
	while (true) {
		handleUdp();
	}
}

void Node::tableManagerDriver() {
	UdpMessage* message = nullptr;
	while (true) {
		tableManagerMutex.lock(); //lock
		//receive table
		if (receivedTable) {
			//flip boolean back to false
			receivedTable = false;
			message = tableManagerMessage;
			tableManagerMessage = nullptr;
			tableManagerMutex.unlock(); //unlock
			//unpack packet
			sf::Uint8 pid;
			*(message->packet) >> pid;
			std::map<std::wstring, uint64_t> table;
			*(message->packet) >> table;
			//process the received hash table
			dealWithHashTable(table, message->ip,false);
		}
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

void Node::readResponseToArrival(UdpMessage* message) {
	sf::Uint8 pid = getPacketID(*(message->packet));
}

void Node::unknownPacket(UdpMessage* message) {
	sf::Uint8 pid;
	*(message->packet) >> pid;
	std::cout << "received packet with pid " << (int)pid << " from " << message->ip << std::endl;
}


void Node::requestFiles(std::vector<fileChangeData> fileChanges, sf::IpAddress server) {
	for (auto fileChange : fileChanges) {
		dirLock->lock();
		sf::Packet packet;

		switch (fileChange.change) {
		case fileChangeType::Edit:
			//We need to negotiate a tcp file transfer with another client to get this file.

		case fileChangeType::Addition:
			//std::wcout << L"received file path" << fileChange.filePath << std::endl;
			//std::wcout << L"my file hash" << fileChange.fileHash << std::endl;
			//std::wcout << L"received file hash" << getFileHash(directory + L'\\' + fileChange.filePath) << std::endl;
			if ((std::filesystem::exists(directory + L'\\' + fileChange.filePath) && (fileChange.fileHash == getFileHash(directory + L'\\' + fileChange.filePath)))) {
				dirLock->unlock();
				continue;
			}
			packet << (sf::Uint8)5;
			packet << fileChange;
			packet << (sf::Uint16)25565;
			//We need to negotiate a tcp file transfer with anotehr client to get this.
			negotiateTCPTransfer(25565, fileChange, packet, server);
			dirLock->unlock();
			break;
		case fileChangeType::Deletion:
			//We check if the file exists and if it does we delete it.
			deleteFile(directory + L'\\' + fileChange.filePath);
			dirLock->unlock();
			break;

		}
	}
}

void Node::dealWithHashTable(std::map<std::wstring, uint64_t>& table, sf::IpAddress sender, bool ignoreEdits) {
	for (std::pair<std::wstring, uint64_t> entry : table) {
		std::wcout << entry.first << L" " << entry.second << std::endl;
	}
	auto dirChanges = getDirectoryChanges(fileHashes, table);
	
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
	requestFiles(dirChanges,sender);

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