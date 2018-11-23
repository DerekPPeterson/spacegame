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

class Model : public Renderable
{
    public:
        Model() {};
        Model(const char *path)
        {
            loadModel(path);
        }
        void draw(Shader& shader) override;

    private:
        // Model Data
        std::vector<Mesh> meshes;
        std::string directory;
        std::set<std::string> texture_paths;

        //functions
        void loadModel(std::string path);
        void processNode(aiNode *node, const aiScene *scene);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial *mat, 
                aiTextureType type, std::string typeName);
};


#endif
