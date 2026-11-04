// risv.cpp : Defines the entry point for the application.
//

#include "risv/memory.h"
#include "risv.h"
#define FILE_PATH "C:/Users/Philipp/source/repos/risv/risv/binaryFile.bin"
using namespace std;
#include <filesystem>


void write_file(std::fstream& fs ,int Number) {
	fs.write(reinterpret_cast<const char*>(&Number), sizeof Number);
}
void write_file(std::fstream& fs, string binStr){
	for (char ch : binStr) {
		fs.write(reinterpret_cast<const char*>(&ch), sizeof ch);
	}
	
}
void read_file(std::fstream& fs) {
	fs.seekg(0, std::ios::end);
	int size = fs.tellg();
	fs.seekg(0, std::ios::beg);
	char* buffer = new char[size];
	fs.read(buffer , size);
    for (int i = 0; i < size; i++)
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (int)(unsigned char)buffer[i] << ' ';
    std::cout << std::dec << '\n';


}
int main(){
	std::fstream fs(FILE_PATH, std::ios::out | std::ios::binary | std::ios::app | std::ios::in);
	cout << "Hello CMake." << endl;
	std::cout << std::filesystem::current_path() << '\n';
	Memory mem(16);

	write_file(fs, 5);
	write_file(fs, "hello");
	read_file(fs);
	fs.close();
	return 0;
}
