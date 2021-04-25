#pragma once
#include <string>;
#include <SFML/Network.hpp>
#include <iostream>
enum class fileChangeType { Addition, Deletion, Edit };
struct fileChangeData {
	std::wstring filePath;
	uint64_t fileHash;
	fileChangeType change;
};
sf::Packet& operator>>(sf::Packet&, std::vector<fileChangeData>&);
sf::Packet& operator>>(sf::Packet&, fileChangeType&);