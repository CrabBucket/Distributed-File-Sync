#include "fileChangePacket.h"

sf::Packet& operator>>(sf::Packet& packet, fileChangeType& changeType) {
	sf::Uint8 changeEnum;
	packet >> changeEnum;
	//convert extracted changeEnum from Uint8 to fileChangeType
	switch (changeEnum) {
	case 0:
		changeType = fileChangeType::Addition;
		break;
	case 1:
		changeType = fileChangeType::Deletion;
		break;
	case 2:
		changeType = fileChangeType::Edit;
		break;
	default:
		changeType = fileChangeType::Edit;
		std::cout << "Error Reading fileChangeType" << std::endl;
		break;
	}
	return packet;
}



sf::Packet& operator<<(sf::Packet& packet, fileChangeData& fileChange) {
	packet << fileChange.filePath;
	packet << fileChange.fileHash;
	packet << (sf::Uint8)fileChange.change;
	return packet;
}
sf::Packet& operator<<(sf::Packet& packet, std::vector<fileChangeData>& filesChanged) {
	sf::Uint8 pid = 6;
	packet << pid;
	packet << filesChanged.size(); // insert size of vector for extraction purposes
	for (auto fileChange : filesChanged) {
		packet << fileChange;
	}

	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, std::vector<fileChangeData>& fileChangeVector) {
	sf::Uint32 size;
	sf::Uint8 pid;
	packet >> pid;
	packet >> size; //extract size

	std::wstring filePath;
	uint64_t fileHash;
	fileChangeType changeType;
	//extract fileChangeDatas one by one inserting into target vector
	for (sf::Uint32 i = 0; i < size; ++i) {
		packet >> filePath;
		packet >> fileHash;
		packet >> changeType;
		fileChangeVector.push_back({ filePath,fileHash,changeType });
	}
	return packet;
}
sf::Packet& operator>>(sf::Packet& packet, fileChangeData& fileChange) {
	std::wstring filePath;
	uint64_t fileHash;
	fileChangeType changeType;
	packet >> filePath;
	packet >> fileHash;
	packet >> changeType;
	fileChange = { filePath, fileHash, changeType };
	return packet;
}