#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <vector>
#include <memory>

#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "model.h"

class Drawable
{
    public:
        virtual void draw(Shader shader) {};
};

class Object: public Drawable 
{
    public:
        Object() {};
        virtual void draw(Shader Shader) {};
    protected:
        bool visible = true;
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
        virtual void draw(Shader shader) {};
        virtual void setUniforms(Shader shader, int i) {};

        static shared_ptr<Light> makeLight(LightType type, glm::vec3 position, glm::vec3 color);
        static vector<shared_ptr<Light>> getAllLights();

    private:
        static vector<shared_ptr<Light>> allLights;
};

class PointLight: public Light
{
    public:
        void draw(Shader shader);
        void setUniforms(Shader shader, int iPointLight);
        static void setup();
    
    protected:
        PointLight() {};
        PointLight(glm::vec3 position, glm::vec3 color);

        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 attenuation = {0, 0.5, 0.05};
        float size = 1;

        static Model sphere;

        friend class Light;
};

//std::vector<Light&> ALL_LIGHTS;


#endif
