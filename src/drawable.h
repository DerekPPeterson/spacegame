#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <vector>
#include <memory>
#include <unordered_set>

#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "model.h"
#include "nocopy.h"

class Selectable
{
    public:
        bool checkSetHover(const glm::mat4& view, const glm::mat4& projection, 
                float x, float y, int screenWidth, int screenHeight);
    protected:
        bool isHovered = false;
        bool isSelected = true;
        float targetRadius = 50;
        glm::vec3 position;
};

struct UpdateInfo
{
    float deltaTime;
    float curTime;
};

class Object : public non_copyable
{
    public:
        //virtual void draw(Shader Shader) const {};
        virtual void update(UpdateInfo& info) {};
        bool visible = true;

    private:

        static std::unordered_set<Object*> objects;
};

class Cube: public Object
{
    public:
        Cube() {};
        Cube(glm::vec3 position);
        void draw(Shader shader);
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
        void draw(Shader shader) {draw();};
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
        virtual void setUniforms(Shader shader, int i) {};
        virtual void setColor(glm::vec3 color);

        static std::shared_ptr<Light> makeLight(LightType type, glm::vec3 position, glm::vec3 color);
        static std::vector<std::shared_ptr<Light>> getAllLights();

        glm::vec3 color;

    private:
        static std::vector<std::shared_ptr<Light>> allLights;
        
};

class PointLight: public Light , public Renderable
{
    public:
        void draw(Shader& shader) override;
        void setUniforms(Shader shader, int iPointLight) override;
        static void setup();
    
    protected:
        PointLight() {};
        PointLight(glm::vec3 position, glm::vec3 color);

        glm::vec3 position;
        glm::vec3 attenuation = {0, 0.5, 0.1};
        float size = 0.3;

        static Model sphere;

        friend class Light;
};

//std::vector<Light&> ALL_LIGHTS;


#endif
