#include "fileChangePacket.h"

sf::Packet& operator>>(sf::Packet& packet, fileChangeType& changeType) {
	sf::Uint8 changeEnum;
	packet >> changeEnum;
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



sf::Packet& operator>>(sf::Packet& packet, std::vector<fileChangeData>& fileChangeVector) {
	sf::Uint32 size;
	packet >> size;

	std::wstring filePath;
	uint64_t fileHash;
	fileChangeType changeType;
	for (sf::Uint32 i = 0; i < size; ++i) {
		packet >> filePath;
		packet >> fileHash;
		packet >> changeType;
		fileChangeVector.push_back({ filePath,fileHash,changeType });
	}
	return packet;
}