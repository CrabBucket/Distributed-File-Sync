//#include <iostream>
#include "FileHelper.h"
#include "Client.h"
#include "Server.h"
#include <iostream>

int main() {
	char i;
	std::cin >> i;
	if (i == 'c') {
		Client c;
		std::cout << c.connect("localhost", 23077);
		std::cout << c.send("message sent from client");
		std::cout << c.receive() << std::endl;
	}
	else {
		Server s;
		std::cout << s.listen(23077);
		std::cout << s.accept();
		std::cout << s.receive() << std::endl;
		std::cout << s.send("message returned");
	}
	std::cin >> i;
	//some test code that probably only works on my machine cus requires specific files

	/*
	std::cout << createDirectory("C:/Users/Tanner/Documents/BurgerKang") << std::endl;
	for(std::string s : getFilepaths("C:/Users/Tanner/Documents/Python")){
		std::cout << s << std::endl;
		std::cout << getFileHash(s) << std::endl;
	}

	std::cout << filesDiffer("C:/Users/Tanner/Documents/BurgerKang/test.txt", "C:/Users/Tanner/Documents/BurgerKang/test.txt") << std::endl;
	*/
	return 0;
}