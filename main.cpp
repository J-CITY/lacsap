#include <iostream>
#include "scanner.h"
#include <direct.h>
#include <string>

using namespace std;

int main(int argc, char* argv[])
{
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
    }


    string path = argv[0];
    path.resize(path.size() - 10);
    cout << path + filename << endl;

    Scanner scanner(path + filename);

    scanner.read();

    if(key == "-l" || key == "-lexer") {
        scanner.printLexems2();
    } else if(key == "-t" || key == "-test") {
        scanner.test();
    }

    return 0;
}
