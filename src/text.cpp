#include "text.h"

#include <plog/Log.h>

#include <regex>
#include <fstream>
#include <iostream>

using namespace std;

Font::Font(string path)
{
	parseUbfg(path);
}

UbfgInfo Font::parseUbfg(std::string filename)
{
	UbfgInfo info;
	ifstream file;
	file.open(filename);
	if (file.fail()) {
		LOG_ERROR << "Failed to open font file: " << filename;
		return info;
	}

	LOG_INFO << "Loading font from: " << filename;

	regex texRegex("textures:\\s+(\\S+)");
	regex charRegex("(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)");
	regex kerning("(\\S+)\\s+(\\S+)\\s+(\\S+)");
	smatch m;

	for (string line; getline(file, line);) {
		if (regex_search(line, m, texRegex)) {
			info.textureFilename = m[1];
		} else if (regex_search(line, m, charRegex)) {
			CharInfo character;
			character.codepoint = stoi(m[1]);
			character.xpos      = stof(m[2]);
			character.ypos      = stof(m[3]);
			character.width     = stof(m[4]);
			character.height    = stof(m[5]);
			character.xoffset   = stof(m[6]);
			character.yoffset   = stof(m[7]);
			character.origw     = stof(m[8]);
			character.origh     = stof(m[9]);

            info.characters[character.codepoint] = character;
		} else if (regex_search(line, m, kerning)) {
            info.kerningPairs[stoi(m[1])][stoi(m[2])] = stof(m[3]);
        }
	}
	return info;
}
