#pragma once
#include <string>
#include <SFML/Network.hpp>
#include <iostream>

//enum for describing what type of change occured
enum class fileChangeType { Addition, Deletion, Edit };

//packages data relevant to file change into one object
struct fileChangeData {
	std::wstring filePath;
	uint64_t fileHash;
	fileChangeType change;

	//custom comparator
	bool operator==(const fileChangeData& other) {
		return filePath == other.filePath and fileHash == other.fileHash and change == other.change;
	}
};

//overloaded packet operators
sf::Packet& operator<<(sf::Packet&, fileChangeData&);
sf::Packet& operator<<(sf::Packet&, std::vector<fileChangeData>&);
sf::Packet& operator>>(sf::Packet&, fileChangeType&);
sf::Packet& operator>>(sf::Packet&, std::vector<fileChangeData>&);
sf::Packet& operator>>(sf::Packet&, fileChangeData&);