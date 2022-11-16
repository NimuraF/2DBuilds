#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include "Binary.h"


int placeOnRandomPos(std::ofstream &file, int doje = -1) {

	int pos = 1 + rand() % 12;
	pos = pos * 12;
	while (pos == doje) {
		pos = 12 * rand() % 9;
	}
	std::cout << pos << std::endl;
	file.seekp(pos, std::ios::beg);
	std::string tmp = "Kursach.exe\0";
	file.write(tmp.c_str(), tmp.size());

	return pos;
}



void createBinaryFile() {

	srand(time(NULL));

	std::ofstream file("randomBinary.bin", std::ios_base::binary);

	std::string buf(12, 'h');

	for (int x = 0; x < 100; x++) {
		for (int y = 0; y < 12; y++) {
			buf[y] = (char)(59 + rand() % 25);
		}
		buf[12] = '\0';
		file.write(buf.c_str(), buf.size());
	}

	int rand_param = rand() % 100;

	if (rand_param < 86 && rand_param >= 76) {
		placeOnRandomPos(file);
	}
	else if (rand_param < 76) {
		int result = placeOnRandomPos(file);
		placeOnRandomPos(file, result);
	}
	

	file.close();
}

std::string readFromBinaryFile() {
	std::ifstream fileR("randomBinary.bin", std::ios::binary);

	std::string sout(11, 'h');

	std::string result = "NOT FOUND";

	while (!fileR.eof()) {
		for (int i = 0; i < 12; ++i) {
			fileR.read(&sout[i], sizeof(sout[0]));
		}
		if (sout == "Kursach.exe\0") {
			result = sout;
			return result;
		}
	}

	fileR.close();

	return result;
}