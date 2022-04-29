#include "storage/index/object_file/heap_file.h"

#include <vector>
#include <iostream>
#include <string>

using namespace std;

void print(std::vector<unsigned char> vec){
	for(unsigned char c: vec){
		std::cout << c;
	}
	std::cout << endl;
}

int main(int argc, char *argv[]) {
	const std::string& filename = argv[1];
	HeapFile* heap_file = new HeapFile(filename);
	int n;
	std::cin >> n;
	for(int quer = 0; quer < n; quer++) {
		std::string qtype;
		std::cin >> qtype;
		if(qtype[0] == 'w'){
			std::string str;
			std::cin >> str;
			std::vector<unsigned char> bytevec(str.begin(), str.end());
			uint64_t id = heap_file->write(bytevec);
			std::cout << id << endl;	
		}
		else if(qtype[0] == 'r') {
			uint64_t id;
			std::cin >> id;
			auto value = heap_file->read(id);
			print(*value);
		}
		else if(qtype[0] == 'f') {
			heap_file->flush();
		}
		else if(qtype[0] == 'x') {
			delete heap_file;
			heap_file = new HeapFile(filename);
		}
		else if(qtype[0] == 'd') {
			uint64_t id;
			std::cin >> id;
			heap_file->remove(id);
		}
		else{
			std::cout << "invalid query" << endl;
		}
	}
	
	return 0;
}
