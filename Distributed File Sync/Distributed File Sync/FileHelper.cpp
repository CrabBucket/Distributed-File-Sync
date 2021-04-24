#include "FileHelper.h"




bool createDirectory(const std::wstring& absPath) {
	fs::path p = absPath;
	p.make_preferred();
	if (fs::create_directory(p)) {
		return true;
	}
	else {
		#ifdef DEBUG
		std::cout << "Directory already exists or error creating directory: " << absPath << std::endl;
		#endif
		return false;
	}
}

std::vector<std::wstring> getFilepaths(const std::wstring& absPath) {
	std::vector<std::wstring> filepaths;
	for (fs::directory_entry entry : fs::directory_iterator(absPath)){
		fs::path p = entry.path();
		p.make_preferred();
		if (fs::is_directory(p)) {
			for (std::wstring filepath : getFilepaths(p.wstring())) {
				filepaths.push_back(filepath);
			}
		}
		else {
			filepaths.push_back(p.wstring());
		}
	}
	return filepaths;
}

bool filesDiffer(const std::wstring& absPath1, const std::wstring& absPath2) {
	return getFileHash(absPath1) != getFileHash(absPath2);
}

std::size_t filesize(const std::wstring& absPath) {
	std::ifstream file;
	file.open(absPath);
	std::size_t length = 0;
	if (file) {
		// get length of file:
		file.seekg(0, file.end);
		length = file.tellg();
	}
	file.close();

	return length;
}

//hash is a takes into account absolute path, file size, and file contents
uint64_t getFileHash(const std::wstring& absPath) {
	std::vector<uint64_t> multiplands{2,7,17,29,41,53,67,79,97};
	uint64_t modBase = UINT64_MAX - 1000;
	int i = 0;

	//hash file length
	uint64_t hash = filesize(absPath);

	//hash path
	for(char c : absPath)
		hash = (hash + c) % modBase;

	//hash contents
	std::ifstream file;
	file.open(absPath);

	if (file) {
		char c;
		file.get(c);
		while (file.good()) {
			hash = (hash + (int)c * i) % modBase;
			file.get(c);
			i = (i == multiplands.size()) ? 0 : i + 1;
		}
	}

	file.close();

	return hash;
}