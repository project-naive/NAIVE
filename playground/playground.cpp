// playground.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>

/* read() moves the current position
int main()
{
	std::ifstream ifs;
	ifs.open("playground.cpp", std::ios::binary);
	if(!ifs) return 1;
	char buffer[10]{};
	std::cout << ifs.tellg();
	ifs.read(buffer,8);
	std::cout<<ifs.tellg()<<'\n'<<buffer;
	return 0;
}
*/

/* bitstream should start with low bits (little endian)
int main()
{
	uint64_t test=0xF0F0F0CC01;
	for (int i = 0; i < 64; ++i) {
		std::cout<<bool(test&(0x01Ui64<<i))<<' ';
	}
	std::cout<<'\n';
	char* str=(char*)&test;
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			std::cout << bool(str[i] & (0x01Ui8 << j)) << ' ';
		}
	}
}
*/

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
