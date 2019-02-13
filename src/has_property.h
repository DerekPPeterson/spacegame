#ifndef HAS_PROPERTY_H
#define HAS_PROPERTY_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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
        has_model_mat() : model(1.0f) {};
        glm::mat4 getModel() const {return model;};
        void setModel(glm::mat4 m) {model = m;};
    protected:
        glm::mat4 model;
};


#endif
