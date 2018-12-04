#include "model.h"
#include "lineobjload.h"

#include <string>
#include <set>
#include <stdio.h>
#include <memory>
#include <plog/Log.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;


Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices,
                vector<Texture> textures) :
    MeshRenderable(0 /* VAO set by setupMesh */, indices.size()) 
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    setupMesh();
}

void Mesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), 
            &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
            &indices[0], GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0);

    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
            (void*) offsetof(Vertex, normal));

    // texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
            (void*) offsetof(Vertex, texCoords));

    glBindVertexArray(0);
}

void Mesh::draw(Shader& shader)
{
    int i_diffuse = 0;
    int i_specular = 0;
    for (int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        string number;
        string name = textures[i].type;
        if (name == "diffuse") {
            number = to_string(i_diffuse);
            i_diffuse++;
        } else if (name == "specular") {
            number = to_string(i_specular);
            i_specular++;
        }
        shader.setInt((name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

LineMesh::LineMesh(vector<glm::vec3> vertices, vector<unsigned int> indices) :
    MeshRenderable(0 /* VAO set by setupMesh */, indices.size()) 
{
    this->vertices = vertices;
    this->indices = indices;
    setupMesh();
    stage = SHADER_LAMP;
}

void LineMesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), 
            &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
            &indices[0], GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*) 0);
}

void LineMesh::draw(Shader& shader)
{
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);
}

void Model::draw(Shader& shader)
{
    for (int i = 0; i < meshes.size(); i++) {
        meshes[i].draw(shader);
    }
}

void Model::loadModel(string path)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, 
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    if (!scene or scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE or !scene->mRootNode) {
        LOG_ERROR << importer.GetErrorString();
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}


void Model::processNode(aiNode *node, const aiScene *scene)
{
    // process ll the nodes meshes if any
    for (int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    // then do same thing recursively for any children
    for(int i =0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh * mesh, const aiScene *scene)
{
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    for (int i = 0;  i < mesh->mNumVertices; i++) {
        Vertex vertex;

        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        vertex.normal.x = mesh->mNormals[i].x;
        vertex.normal.y = mesh->mNormals[i].y;
        vertex.normal.z = mesh->mNormals[i].z;

        if (mesh->mTextureCoords[0]) {
            vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
            vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
        } else {
            vertex.texCoords = glm::vec2(0.0, 0.0);
        }

        vertices.push_back(vertex);
    }

    for (int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for(int j = 0; j < face.mNumIndices; j ++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    vector<Texture> diffuseMaps = loadMaterialTextures(material, 
            aiTextureType_DIFFUSE, "diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    vector<Texture> specularMaps = loadMaterialTextures(material, 
            aiTextureType_SPECULAR, "specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    return Mesh(vertices, indices, textures);
}

unsigned int loadTextureFromFile(string path, string directory, 
        bool genMipmaps)
{
    int width, height, nrChannels;
    string full_path = directory + "/" + path;
    unsigned char *data = stbi_load(full_path.c_str(), &width, &height, &nrChannels, 0); 
    if (not data) {
        LOG_ERROR << "Failed to load texture: " << full_path;
    }

	LOG_INFO << "Loading texture: " << full_path;

	// Create texture from image
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    if (nrChannels == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else if (nrChannels == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    if (genMipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return texture;
}

vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, 
        aiTextureType type, string typeName)
{
    vector<Texture> textures;
    for (int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        string texture_path = str.C_Str();
        if (texture_paths.count(texture_path)) {
            continue;
        }
        texture_paths.insert(texture_path);
        Texture texture;
        texture.id = loadTextureFromFile(texture_path, directory);
        texture.type = typeName;
        textures.push_back(texture);
    }
    return textures;
}

void LineModel::loadModel(string path)
{
    LineObj model = readLineObj(path);
    mesh = LineMesh(model.vertices, model.indices);
}

// TODO debug transform, remove
#include "timer.h"

void LineModel::draw(Shader& shader)
{
    mesh.draw(shader);
}


