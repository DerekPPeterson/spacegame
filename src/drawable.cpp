#include "drawable.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

DrawableFromVertexArray::DrawableFromVertexArray(
		const float* vertices, int nVertices, int vertexSize)
{
	this->nVertices = nVertices;
	this->vertexSize = vertexSize;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexSize * nVertices * sizeof(float), 
			vertices, GL_STATIC_DRAW);
}

void DrawableFromVertexArray::addAttribute(unsigned int size)
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(curAttribute, size, GL_FLOAT, GL_FALSE, 
			vertexSize * sizeof(float), (void*) (curOffset * sizeof(float)));
	glEnableVertexAttribArray(curAttribute);
	curOffset += size;
	curAttribute++;
}

std::unordered_map<int, int> DrawableInstance::indexCounts;
std::unordered_map<int, std::vector<glm::mat4>> DrawableInstance::modelMats;

DrawableInstance::DrawableInstance(unsigned int VAO, int indexCount, glm::mat4 model)
{
    this->VAO = VAO;
    this->model = model;

    // If we have't added the extra model attributes to this VAO do that
    indexCounts[VAO] = indexCount;

    // Update vertex attributes
    glBindVertexArray(VAO);
    for (int i = 0; i < 4; i ++) {
         glEnableVertexAttribArray(i + 3); 
         glVertexAttribPointer(i + 3, 4, GL_FLOAT, GL_FALSE, 
                 4 * sizeof(glm::vec4), (void*)(i * sizeof(glm::vec4)));
         glVertexAttribDivisor(i+3, 1);
    }
}

void DrawableInstance::queueDraw()
{
    modelMats[VAO].push_back(model);
}

void DrawableInstance::drawQueue()
{
    // foreach VAO
    for (auto pair : modelMats) {
        unsigned int VAO = pair.first;
        vector<glm::mat4> models = pair.second;

        // Create model matrix buffer
        unsigned int modelBuffer;
        glGenBuffers(1, &modelBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
        glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(glm::mat4),
                &models[0], GL_STATIC_DRAW);

        // Draw instances
        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, indexCounts[VAO], GL_UNSIGNED_INT, 
                0, models.size());
    }
}

vector<float> DrawableFromVertexArray::queue;

void DrawableFromVertexArray::queueDraw()
{
	queue.push_back(VAO);
	queue.push_back(nVertices);
}

void DrawableFromVertexArray::drawQueue()
{
	for (int i = 0; i < queue.size(); i += 2) {
		glBindVertexArray(queue[i]);
		glDrawArrays(GL_TRIANGLES, 0, queue[i+1]);
	}
    queue.clear();
}

void DrawableFromVertexArray::draw()
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, nVertices);
}

unordered_set<Object*> Object::objects;

Object::Object()
{
    objects.insert(this);
};

void Object::setVisible(bool visible)
{
    this->visible = visible;
}

bool Object::isVisible()
{
    return visible;
}

const std::unordered_set<Object*>& Object::getObjects()
{
    return objects;
}

std::vector<Drawable> Object::getDrawables()
{
    return vector<Drawable>();
}

// TODO pretty sure these lights will never get destroyed automatically
vector<shared_ptr<Light>> Light::allLights = {};

vector<shared_ptr<Light>> Light::getAllLights()
{
    return allLights;
}

shared_ptr<Light> Light::makeLight(LightType type, glm::vec3 position, glm::vec3 color)
{
    shared_ptr<Light> new_light;
    switch(type) {
        case LIGHT_POINT:
            new_light = shared_ptr<Light>(new PointLight(position, color));
    }
    
    allLights.push_back(new_light);

    return new_light;
}

void PointLight::setup()
{
    sphere = Model("./res/models/sphere.obj");
}

PointLight::PointLight(glm::vec3 position, glm::vec3 color)
{
    this->position = position;
    this->color = color;
}

void PointLight::setUniforms(const Shader& shader, int iPointLight)
{
    string arrayElem = "pointLights[" + to_string(iPointLight) + "]";

    shader.setVec3(arrayElem + ".position", position);
    shader.setVec3(arrayElem + ".color", color);
    shader.setVec3(arrayElem + ".attenuation", attenuation);
}

void PointLight::draw(const Shader& shader)
{
    shader.setVec3("color", color);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, glm::vec3(size, size, size));
    shader.setMat4("model", model);
    sphere.draw(shader);
}

Cube::Cube(glm::vec3 position)
{
    this->position = position;
}

void Cube::setup()
{
    model = Model("./res/models/cube.obj");
    isSetup = true;
} 
void Cube::draw(const Shader& shader)
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
    shader.setMat4("model", model);
    Cube::model.draw(shader);
}

Quad::Quad(glm::vec3 position)
{
    this->position = position;
}

void Quad::setup()
{
    if (not isSetup) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 4 * numVertices * sizeof(float), vertices, 
                GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 
                4 * sizeof(float), (void*) 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 
                4 * sizeof(float), (void*) (2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        isSetup = true;
    }
} 

void Quad::draw()
{
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
}

Model PointLight::sphere = Model();

Model Cube::model;
bool Cube::isSetup = false;

unsigned int Quad::VBO = -1;
unsigned int Quad::VAO = -1;
int Quad::numVertices = 6;
bool Quad::isSetup = false;
float Quad::vertices[] = {  
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
    };	


