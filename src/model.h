#ifndef MESH_H
#define MESH_H

#include <vector>
#include <set>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/scene.h>

#include "shader.h"


struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
};

class Mesh
{
    public:
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
                std::vector<Texture> textures);
        void draw(const Shader& shader);

    private:
        unsigned int VAO, VBO, EBO;
        void setupMesh();
};

class Model
{
    public:
        Model() {};
        Model(const char *path)
        {
            loadModel(path);
        }
        void draw(const Shader& shader);

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
