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

#include "uithings.h"


using namespace std;
using namespace std::experimental::filesystem;

Font::Font(string fntFilename)
{
    path p = fntFilename;
    LOG_INFO << "Loading font from " << fntFilename;
    ifstream ifile(p, ios::binary);
    {
        cereal::PortableBinaryInputArchive iarchive(ifile);
        iarchive(info);
    }
    textureId = loadTextureFromFile(info.textureFilename, p.parent_path(), false);
    textQuad = createTextQuad();
}

UbfgInfo parseUbfg(std::string filename)
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
	regex namesizeRegex("(.*)\\s+(\\d+)pt");
	smatch m;

	for (string line; getline(file, line);) {
		if (regex_search(line, m, namesizeRegex)) {
            info.fontName = m[1];
            info.pointSize = stoi(m[2]);
        } else if (regex_search(line, m, texRegex)) {
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

shared_ptr<InstanceMeshRenderable> Font::createTextQuad()
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

    // Only need to create VAO once
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
    
    return shared_ptr<InstanceMeshRenderable>(new InstanceMeshRenderable(
            SHADER_TEXT,
            VAO, sizeof(indices) / sizeof(indices[0]),
            {
                INSTANCE_ATTRIB_MAT4,
                INSTANCE_ATTRIB_VEC3,
                INSTANCE_ATTRIB_VEC2,
                INSTANCE_ATTRIB_VEC2,
            }, {textureId}));
};

std::shared_ptr<Font> Fonts::title;
std::shared_ptr<Font> Fonts::regular;
std::shared_ptr<Font> Fonts::console;
std::shared_ptr<Font> Fonts::mono;
void Fonts::setup() {
    LOG_INFO << "Loading all fonts";
    title = shared_ptr<Font>(new Font("./res/fonts/conthrax.ubfgdat"));
    regular = shared_ptr<Font>(new Font("./res/fonts/gravity.ubfgdat"));
    console = shared_ptr<Font>(new Font("./res/fonts/console.ubfgdat"));
    mono = shared_ptr<Font>(new Font("./res/fonts/notosansmono.ubfgdat"));
}
volatile Fonts fonts; // need to instantiate fonts so setup gets run


struct CharInstanceData
{
    glm::mat4 model;
    glm::vec3 color;
    glm::vec2 TexCoordOffset;
    glm::vec2 charSize;
};

CharInfo iconInfo = {
	.codepoint = 0,
	.xpos = 0,
	.ypos = 0,
	.width = 50,
	.height = 50,
	.xoffset = 0,
	.yoffset = 0,
	.origw = 50,
	.origh = 50,
};

void Text::calcCharPositions()
{
    glm::vec2 curCharPos = {0, 0}; 
    float curMaxWidth = maxwidth / size;

    charPositions.clear();
    icons.clear();
    multiline.clear();

    float maxCharPos = 0;

    unsigned int lastChar = 0;
    bool breakLines = true;
    for (int i = 0; i < text.size(); i++) {
        unsigned int c = text[i];
        CharInfo charInfo; 

        // Identify icon strings
        if (c == '{') {
            string key;
            int j;
            bool isAny = false;
            string anyAmount;
            for (j = i + 1; j < text.size() and text[j] != '}'; j++) {
                if (text[j] == '|') {
                    isAny = true;
                    continue;
                }
                if (isAny) {
                    anyAmount.push_back(text[j]);
                } else {
                    key.push_back(text[j]);
                }

            }

            shared_ptr<Renderable> icon;
            if (isAny) {
                icon = createIcon(key, stoi(anyAmount));
            } else {
                icon = createIcon(key);
            }

            if (icon) {
                icons.push_back(icon);
                i = j;
                charInfo = iconInfo;
                c = '~';
            }
        } else {
            charInfo = font->info.characters[c];
        }
        
        // Reset xpos and go down for newline
        if (c == '\n') {
            curCharPos.x = 0;
            curCharPos.y -= 1;
            lastChar = 0;
        } else {
            float kerningDist = lastChar ? (float) font->info.kerningPairs[lastChar][c] / 50 : 0;
            lastChar = c;
            charPositions.push_back(curCharPos);
            multiline.push_back(c);
            curCharPos.x += charInfo.origw / 50 + kerningDist;

            if (curCharPos.x > maxCharPos) {
                maxCharPos = curCharPos.x;
            }
        }

        // If we exceed the width then go back to the last space
        // and drop a line
        // If there are no spaces then don't break lines
        if (breakLines and maxwidth and curCharPos.x >= curMaxWidth) {
            for (int j = i; j >= 0; j--) {
                c = text[j];
                charPositions.pop_back();
                multiline.pop_back();
                i = j;
                curCharPos.x = 0;
                lastChar = 0;
                // TODO other spaces?
                if (c == ' ') {
                    i = j;
                    curCharPos.y -= 1;
                    break;
                } else if (j == 0) {
                    breakLines = false;
                }
            }
        }
    }

    actualMaxWidth = maxCharPos;
}

void Text::queueDraw()
{
    CharInstanceData data;
    //TODO stop hardcoding 50pt text
    // Activate font texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font->textureId);
    data.color = color;


    // Actually render all of the characters
    glm::vec2 curCharPos = {0, 0}; 
    int curIcon = 0;
    for (int i = 0; i < charPositions.size(); i++) {
        unsigned int c = multiline[i];
        curCharPos = charPositions[i];
        std::shared_ptr<Renderable> icon(nullptr);

        CharInfo charInfo;
        // Identify icon strings
        if (c == '~') {
            charInfo = iconInfo;
            icon = icons[curIcon];
            curIcon++;
        } else {
            charInfo = font->info.characters[c];
        }

        data.TexCoordOffset = 
                {charInfo.xpos / font->info.textureWidth, 
                 charInfo.ypos / font->info.textureHeight};
        data.charSize = 
                {charInfo.width / font->info.textureWidth, 
                charInfo.height / font->info.textureHeight};

        glm::mat4 charModel = getModel();
        charModel = glm::scale(charModel, glm::vec3(size));
        charModel = glm::translate(charModel, {
                curCharPos.x + charInfo.xoffset / 50.0, 
                curCharPos.y - charInfo.yoffset / 50.0, 
                0});
        charModel = glm::scale(charModel, {(float) charInfo.width / 50.0, charInfo.height / 50.0, 1});
        data.model = charModel;
        
        if (not icon) {
            font->textQuad->addInstance(&data);
        } else {
            glm::mat4 iconModel = glm::translate(data.model, {0.5, -1, 0});
            dynamic_pointer_cast<has_model_mat>(icon)->setModel(iconModel);
        }
    }
    font->textQuad->queueDraw();

    for (auto i : icons) {
        i->queueDraw();
    }
}

float Text::calcTransformedMaxWidth(float rawWidth)
{
    glm::mat4 model = getModel();
    glm::vec4 zero = model * glm::vec4(0, 0, 0, 1);
    glm::vec4 textEnd = model * glm::vec4(rawWidth, 0, 0, 1);
    return  glm::length(textEnd - zero);
}

void Text::update(UpdateInfo& info)
{
    for (auto i : icons) {
        dynamic_pointer_cast<Object>(i)->update(info);
    }
}

float Text::calcWidth()
{
    if (maxwidth) {
        return maxwidth;
    } else {
        calcCharPositions();
        return actualMaxWidth * size;
    }
}
