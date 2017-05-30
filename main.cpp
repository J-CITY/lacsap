#include <iostream>
#include "parser.h"
#include "scanner.h"
//#include <direct.h>
#include <string>
#include <windows.h>

using namespace std;

int main(int argc, char* argv[])
{
    SetConsoleCP(866);
    SetConsoleOutputCP(866);

	std::string path = "C:\\Users\\333da\\Desktop\\in\\";
	for(int i = 0; i < 100; ++i) {
        std::string s;
        if(i < 10)
            s = path + "00" + std::to_string(i) + ".pas.out";
        else
            s = path + "0" + std::to_string(i) + ".pas.out";
		std::ofstream infile (s);
	}

	string key;
	string filename;
	int i = 1;
	while(i < argc) {
		key = argv[i];
		if(key == "-l" || key == "-lexer") {
			i++;
			filename = argv[i];
			break;
		}
		if(key == "-t" || key == "-test") {
			i++;
			filename = argv[i];
			break;
		}
		if(key == "-p" || key == "-parse") {
			i++;
			filename = argv[i];
			break;
		}
		if(key == "-a" || key == "-asm") {
			i++;
			filename = argv[i];
			break;
		}
	}
	if(key == "-l" || key == "-lexer") {
		Scanner scanner(filename);
		scanner.read();
		scanner.printLexems();
	} else if(key == "-p" || key == "-parse") {
		Parser parser(filename);
		parser.Parse();
		parser.print();
	} else if(key == "-a" || key == "-asm") {
		Parser parser(filename);
		parser.Parse();
		parser.printAsm();
	} else {
		Parser parser("0.txt");
		parser.Parse();
		parser.print();
		parser.printAsm();
	}

	return 0;
}
