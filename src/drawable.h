#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <vector>

#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

class Drawable
{
    public:
        virtual void draw() {};
};

class Object: public Drawable 
{
    public:
        Object() {};
        virtual void draw() {};
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

        static float vertices[];
        static int numVertices;
        static unsigned int VBO, VAO;
        static bool isSetup;
};

enum LightType {LIGHT_NONE, LIGHT_POINT};

class Light
{
    public:
        LightType type;
        Light();
};

class PointLight: public Light
{
    public:
        PointLight(glm::vec3 position, glm::vec3 color);
        void draw(Shader shader);
        void setUniforms(Shader shader, int iPointLight);
    
    private:
        Cube cube;
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 attenuation = {0, 0, 1};
};

//std::vector<Light&> ALL_LIGHTS;


#endif
