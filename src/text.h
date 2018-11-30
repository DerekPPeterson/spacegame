#ifndef TEXT_H
#define TEXT_H

#include <string>
#include <vector>
#include <map>

struct CharInfo
{
	unsigned int codepoint;
	float xpos;
	float ypos;
	float width;
	float height;
	float xoffset;
	float yoffset;
	float origw;
	float origh;
};

struct UbfgInfo
{
	std::map<unsigned int, CharInfo> characters;
	std::map<unsigned int, std::map<unsigned int, float>> kerningPairs;
	std::string textureFilename;
	std::string fontName;
};

class Font
{
    public:
		Font(std::string path);
	private:
		UbfgInfo parseUbfg(std::string filename);
};

#endif
