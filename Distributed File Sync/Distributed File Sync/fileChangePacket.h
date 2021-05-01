#pragma once
#include <string>
#include <SFML/Network.hpp>
#include <iostream>

enum class fileChangeType { Addition, Deletion, Edit };
struct fileChangeData {
	std::wstring filePath;
	uint64_t fileHash;
	fileChangeType change;

	bool operator==(const fileChangeData& other) {
		return filePath == other.filePath and fileHash == other.fileHash and change == other.change;
	}
};
sf::Packet& operator<<(sf::Packet&, fileChangeData&);
sf::Packet& operator<<(sf::Packet&, std::vector<fileChangeData>&);
sf::Packet& operator>>(sf::Packet&, fileChangeType&);
sf::Packet& operator>>(sf::Packet&, std::vector<fileChangeData>&);
sf::Packet& operator>>(sf::Packet&, fileChangeData&);