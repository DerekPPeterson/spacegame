#include "text.h"

#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char **argv) {

    for (int i = 1; i < argc; i++) {
        string filename = argv[i];
        cerr << "Will process: " << filename << endl;
        UbfgInfo info = parseUbfg(filename);

        string outputFilename = filename;
        outputFilename.replace(outputFilename.size() - 3, 5, "ubfgdat");
        ofstream of(outputFilename, ios::binary);
        cerr << "Output filename: " << outputFilename << endl;

        {
            cereal::PortableBinaryOutputArchive oarchive(of);
            oarchive(info);
        }
    }
}
