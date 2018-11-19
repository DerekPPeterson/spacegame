#ifndef MESH_H
#define MESH_H

#include <vector>
#include <set>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/scene.h>

#include "shader.h"

using namespace std;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct Texture {
    unsigned int id;
    string type;
};

class Mesh
{
    public:
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        Mesh(vector<Vertex> vertices, vector<unsigned int> indices,
                vector<Texture> textures);
        void draw(Shader& shader);

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
        void draw(Shader shader);

    private:
        // Model Data
        vector<Mesh> meshes;
        string directory;
        set<string> texture_paths;

        //functions
        void loadModel(string path);
        void processNode(aiNode *node, const aiScene *scene);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        vector<Texture> loadMaterialTextures(aiMaterial *mat, 
                aiTextureType type, string typeName);
};


#endif
