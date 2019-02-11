#ifndef TEXT_H
#define TEXT_H

#include "drawable.h"
#include "renderables.h"


#include <string>
#include <vector>
#include <map>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>

#define SERIALIZE(...) \
template<class Archive> \
void serialize(Archive & archive) \
{\
    archive(__VA_ARGS__); \
}

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

    SERIALIZE( codepoint, xpos, ypos, width, height, xoffset, yoffset, origw, origh)
};

struct UbfgInfo
{
	std::map<unsigned int, CharInfo> characters;
	std::map<unsigned int, std::map<unsigned int, float>> kerningPairs;
	std::string textureFilename;
	int textureWidth = 1024; // TODO dynamically set these
	int	textureHeight = 1024;
	std::string fontName;
    int pointSize;

    SERIALIZE(characters, kerningPairs, textureFilename, textureWidth, textureHeight, fontName, pointSize);
};

UbfgInfo parseUbfg(std::string filename);

class Font : public non_copyable
{
    public:
		Font(std::string path);

	private:
		UbfgInfo info;
		unsigned int textureId = 0;
        unsigned int pointSize = 0;
        std::string name;

        std::shared_ptr<InstanceMeshRenderable> createTextQuad();
        std::shared_ptr<InstanceMeshRenderable> textQuad;

		friend class Text;
};

/** Collection of fontst that can be used by text objects throughout applicaion
 */
class Fonts : public needs_setup<Fonts>
{
    public:
        static void setup();
        static std::shared_ptr<Font> title;
        static std::shared_ptr<Font> regular;
        static std::shared_ptr<Font> console;
        static std::shared_ptr<Font> mono;
};

class Text : public Renderable , public has_model_mat, public Object
{
	public:
		Text(std::shared_ptr<Font> font = Fonts::regular, 
                std::string text = "", glm::vec3 color = {1, 1, 1}, 
                float maxwidth = 0, float size = 0.8) 
            : Renderable(SHADER_TEXT), 
              font(font), text(text), color(color), maxwidth(maxwidth),
              size(size) {calcCharPositions();};
        void update(UpdateInfo& info) override;
        virtual void queueDraw() override;
		void setText(std::string text) {this->text = text; calcCharPositions();};
		void setColor(glm::vec3 color) {this->color = color;};
        float calcWidth();
	private:
        float calcTransformedMaxWidth(float rawWidth);
        void calcCharPositions();
        std::shared_ptr<Font> font;
		std::string text;
        std::string multiline;
        glm::vec3 color;
        float maxwidth;
        float actualMaxWidth;
        float size;
        std::vector<std::shared_ptr<Renderable>> icons;
        std::vector<glm::vec2> charPositions;
};

#endif
