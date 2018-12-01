#include "text.h"

#include <plog/Log.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <regex>
#include <fstream>
#include <iostream>
#include <experimental/filesystem>

using namespace std;
using namespace std::experimental::filesystem;

Font::Font(string fntFilename)
{
    path p = fntFilename;
	info = parseUbfg(fntFilename);
    textureId = loadTextureFromFile(info.textureFilename, p.parent_path());
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

MeshRenderable createTextQuad()
{
    float vertices[] = {
        // positions         // texCoords
         0.0f,  -1.0f,  0.0f, 0.0f, 1.0f,
         1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
         0.0f,  0.0f,  0.0f, 0.0f, 0.0f,

         0.0f,  -1.0f,  0.0f, 0.0f, 1.0f,
         1.0f,  -1.0f,  0.0f, 1.0f, 1.0f,
         1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
        };

    unsigned int indices[] = {0, 1, 2, 3, 4, 5};

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*) 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 
            (void*) (sizeof(float) * 3));
    
    return MeshRenderable(VAO, sizeof(indices) / sizeof(indices[0]));
};

MeshRenderable Text::textQuad;

void Text::setup()
{
    static bool isSetup = false;
    if (not isSetup) {
        textQuad = createTextQuad();
    }
}

void Text::draw(Shader& shader)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font->textureId);

    float curTextPos = 0;

    unsigned int lastChar = 0;
    for (auto c : text) {
        CharInfo charInfo = font->info.characters[c];
        shader.setVec3("color", color);
        shader.setVec2("TexCoordOffset", 
                {charInfo.xpos / font->info.textureWidth, 
                 charInfo.ypos / font->info.textureHeight});
        shader.setVec2("charSize", 
                {charInfo.width / font->info.textureWidth, 
                charInfo.height / font->info.textureHeight});

        glm::mat4 charModel = getModel();
        float kerningDist = lastChar ? (float) font->info.kerningPairs[lastChar][c] / 50 : 0;
        charModel = glm::translate(charModel, {curTextPos + charInfo.xoffset / 50, -charInfo.yoffset / 50.0, 0});
        charModel = glm::scale(charModel, {(float) charInfo.width / 50.0, charInfo.height / 50.0, 1});

        shader.setCommon(UNIFORM_MODEL, charModel);
        textQuad.draw(shader);

        curTextPos += charInfo.origw / 50 + kerningDist;
        lastChar = c;
    }
}
