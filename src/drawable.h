#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <vector>
#include <memory>
#include <unordered_set>

#include <glm/gtc/type_ptr.hpp>
#include <plog/Log.h>

#include "shader.h"
#include "model.h"
#include "nocopy.h"
#include "camera.h"
#include "input.h"

/* This is black magic to statically initilize the list of classes that need
 * to run their setup functions
 */
std::vector<void(*)()>& _getSetupFuncs();
void _addSetupFunc(void (*func)());
void runAllSetups();

class _setup_adder
{
    public:
        _setup_adder(void (*func)())
        {
            _addSetupFunc(func);
        };
};

template <class T>
class needs_setup : public non_copyable
{
    protected:
        static _setup_adder a;
        needs_setup() {(void)a;};
};
template <typename T>
_setup_adder needs_setup<T>::a = _setup_adder(T::setup);
/* End black magic */


glm::vec2 calcScreenSpaceCoords(glm::vec3 position, 
        glm::mat4 projection, glm::mat4 view,
        int screenWidth, int screenHeight);

glm::vec3 calcWorldSpaceCoords(glm::vec2 screenCoords, float worldDepth,
        glm::mat4 projection, glm::mat4 view,
        int screenWidth, int screenHeight);

class has_position
{
    public:
        glm::vec3 getPos() const {return position;};
        void setPos(glm::vec3 pos) {this->position = pos;};
    protected:
        glm::vec3 position = glm::vec3(1.0f);
};

class has_model_mat
{
    public:
        glm::mat4 getModel() const {return model;};
        void setModel(glm::mat4 m) {model = m;};
    protected:
        glm::mat4 model = glm::mat4(1.0f);
};

struct UpdateInfo
{
    float deltaTime;
    float curTime;
    Camera* camera;
    glm::mat4 projection;
    MouseInfo mouse;
    int screenWidth;
    int screenHeight;
};

class Selectable : public virtual has_position, public virtual has_model_mat
{
    protected:
        virtual void onClick();
        bool checkSetHoverCircle(UpdateInfo info, bool screenSpace = false);
        bool checkSetHoverQuad(UpdateInfo info, bool screenSpace = false);
        void updateClickState(UpdateInfo info);
        bool isHovered = false;
        bool isSelected = false;
        bool wasClickedOn = false;
        float targetRadius = 50; // For circle func
        std::vector<glm::vec3> quadVertices; // for quad test
        static Selectable* beingHovered; // Points to current selectable being hovered
};

class Dragable : public Selectable
{
    protected:
        void checkSetDrag(UpdateInfo info, bool screenSpace = false);
        bool dragging = false;
        glm::vec3 dragDisplacement;
        glm::vec3 lastDragPos;
        static Dragable* beingDragged; // Points to current dragable being dragged
};


class Object : public non_copyable
{
    public:
        //virtual void draw(Shader Shader) const {};
        virtual ~Object() {};
        virtual void update(UpdateInfo& info) {};
        virtual bool isVisible() {return visible;}
        bool visible = false;
    private:
        static void setup();

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

        static std::shared_ptr<Model> model;
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
        virtual ~Light() {};
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
        virtual ~PointLight() {};
        void draw(Shader& shader) override;
        void setUniforms(Shader shader, int iPointLight) override;
        static void setup();
    
    protected:
        PointLight(glm::vec3 position, glm::vec3 color);

        glm::vec3 position;
        glm::vec3 attenuation = {0, 0.5, 0.1};
        float size = 0.3;

        static std::shared_ptr<Model> sphere;

        friend class Light;
};

//std::vector<Light&> ALL_LIGHTS;


#endif
