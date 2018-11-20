#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "model.h"
#define JUST_RENDER_STAGES
#include "render.h"
#include "nocopy.h"

class Drawable
{
    public:
		Drawable() {};
        virtual void queueDraw() {};
        virtual void draw() 
        {
            queueDraw();
            drawQueue();
        };
        virtual void setShader(const Shader *shader) {
            this->shader = shader;
        }
        virtual void setRenderStage(RenderStage stage) {
            this->stage = stage;
        }
        RenderStage stage = RENDER_NONE;

        static void drawQueue() {};
        void (*drawQueuePtr)() = drawQueue;;
    private:
        const Shader *shader = NULL;

};

class DrawableFromVertexArray : public Drawable
{
    public:
		DrawableFromVertexArray() {};
        DrawableFromVertexArray(const float* vertices, int nVertices, int vertexSize);
        void addAttribute(unsigned int size);

        virtual void queueDraw() override;
        virtual void draw() override;

        static void drawQueue();
        void (*drawQueuePtr)() = drawQueue;
    private:
        int nVertices, vertexSize;
		int curAttribute = 0;
		int curOffset = 0;
        unsigned int VAO, VBO;

		static std::vector<float> queue;
};

class DrawableInstance : public Drawable
{
    public:
        DrawableInstance() {};
        DrawableInstance(unsigned int VAO, int indexCount, glm::mat4 model);
        virtual void queueDraw() override;

        static void drawQueue();
        void (*drawQueuePtr)() = drawQueue;
    private:
        unsigned int VAO;
        glm::mat4 model;

        static std::unordered_map<int, int> indexCounts;
        static std::unordered_map<int, std::vector<glm::mat4>> modelMats;
};

class Object : public non_copyable
{
    public:
        Object();
        virtual void draw(const Shader& shader) const {};
        void setVisible(bool visible);
        bool isVisible();
        virtual std::vector<Drawable> getDrawables();

        static const std::unordered_set<Object*>& getObjects();

    private:
        bool visible = true;
        static std::unordered_set<Object*> objects;
};

class Cube: public Object
{
    public:
        Cube() {};
        Cube(glm::vec3 position);
        void draw(const Shader& shader);
        static void setup();

    private:
        glm::vec3 position;

        static Model model;
        static bool isSetup;
};

class Quad: public Object
{
    public:
        Quad() {};
        Quad(glm::vec3 position);
        void draw();
        void draw(const Shader& shader) {draw();};
        static void setup();

    private:
        glm::vec3 position;

        static float vertices[];
        static int numVertices;
        static unsigned int VBO, VAO;
        static bool isSetup;
};

enum LightType {LIGHT_POINT};

class Light
{
    public:
        virtual void draw(const Shader& shader) {};
        virtual void setUniforms(const Shader& shader, int i) {};

        static std::shared_ptr<Light> makeLight(LightType type, glm::vec3 position, glm::vec3 color);
        static std::vector<std::shared_ptr<Light>> getAllLights();

    private:
        static std::vector<std::shared_ptr<Light>> allLights;
};

class PointLight: public Light
{
    public:
        void draw(const Shader& shader);
        void setUniforms(const Shader& shader, int iPointLight);
        static void setup();
    
    protected:
        PointLight() {};
        PointLight(glm::vec3 position, glm::vec3 color);

        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 attenuation = {0, 0.5, 0.1};
        float size = 0.3;

        static Model sphere;

        friend class Light;
};

//std::vector<Light&> ALL_LIGHTS;


#endif
