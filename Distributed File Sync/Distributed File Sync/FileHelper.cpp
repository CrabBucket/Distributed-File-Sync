#include "FileHelper.h"

bool mycreateDirectory(const std::string& name) {
	std::filesystem::path p = name;
	p.make_preferred();
	if (std::filesystem::create_directory(p)) {
		return true;
	}
	else {
		std::cout << "Error creating directory: " << name << std::endl;
		return false;
	}
}

void hello() {
	std::cout << "hello" << std::endl;
}