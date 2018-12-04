#ifndef TEXT_H
#define TEXT_H

#include "drawable.h"
#include "renderables.h"


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
	int textureWidth = 1024; // TODO dynamically set these
	int	textureHeight = 1024;
	std::string fontName;
};

class Font
{
    public:
        Font() {};
		Font(std::string path);
	private:
		UbfgInfo parseUbfg(std::string filename);
		UbfgInfo info;
		unsigned int textureId = 0;

		friend class Text;
};

class Text : public Renderable , public has_model_mat
{
	public:
		Text() {};
		Text(Font* font, std::string text, glm::vec3 color, 
                float maxwidth = 0, float size = 1) 
            : font(font), text(text), color(color), maxwidth(maxwidth),
              size(size)
				{stage = SHADER_TEXT; setup();};
		virtual void draw(Shader& shader) override;
		void setText(std::string text) {this->text = text;};
	private:
        float calcTransformedMaxWidth(float rawWidth);
		Font* font;
		std::string text;
        glm::vec3 color;
        float maxwidth;
        float size;
		static MeshRenderable textQuad;
		static void setup();
};

#endif
