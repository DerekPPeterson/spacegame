#ifndef MESH_H
#define MESH_H

#include <vector>
#include <set>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/scene.h>

#include "shader.h"
#include "renderables.h"
#include "has_property.h"

unsigned int loadTextureFromFile(std::string path, std::string directory, bool genMipmaps=true);

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
};

class Mesh : virtual public MeshRenderable
{
    public:
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
                std::vector<Texture> textures);
        void draw(Shader& shader) override;

    private:
        unsigned int VBO, EBO;
        void setupMesh();
};

// TODO probably have to create a new Renderable class for this
class LineMesh : virtual public MeshRenderable
{
    public:
        std::vector<glm::vec3> vertices;
        std::vector<unsigned int> indices;

        LineMesh(std::vector<glm::vec3> vertices, 
                std::vector<unsigned int> indices);
        ~LineMesh();
        void draw(Shader& shader) override;

    private:
        unsigned int VBO, EBO;
        void setupMesh();
};

// TODO this will have to change into an instance renderable at some point
class Model : public Renderable
{
    public:
        virtual ~Model() {};
        // The model itself does not get drawn, just the meshes that are part
        // of it, which is why we use SHADER_NONE here
        Model(const char *path) : Renderable(SHADER_NONE)
        {
            loadModel(path);
        }
        virtual void draw(Shader& shader) override;

    private:
        // Model Data
        std::vector<Mesh> meshes;
        std::string directory;

        //functions
        void loadModel(std::string path);
        void processNode(aiNode *node, const aiScene *scene);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial *mat, 
                aiTextureType type, std::string typeName);
};

class LineModel : public Renderable
{
    public:
        LineModel(const char *path) 
            : Renderable(SHADER_NONE), mesh(loadLineMesh(path)) {};
        void queueDraw() override {mesh.queueDraw();};
        void draw(Shader& shader) override {mesh.draw(shader);};

    private:
        // Model Data
        LineMesh mesh;

        //functions
        LineMesh loadLineMesh(std::string path);
};


#endif
