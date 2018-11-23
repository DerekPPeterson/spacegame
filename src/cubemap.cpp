#include "cubemap.h"

#include <glad/glad.h>

#include <stb_image.h>
#include <string>
#include <vector>
#include <iostream>

#include "drawable.h"

using namespace std;

Cube skyboxCube(glm::vec3(0, 0, 0));

void SkyboxRenderable::draw() 
{
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glCullFace(GL_FRONT);

    //TODO fix this
    //shader.use();
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
    glActiveTexture(GL_TEXTURE0);
    //skyboxCube.draw(shader);

    glCullFace(GL_BACK);
    glDepthMask(GL_TRUE);
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 
                    GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}  

void Skybox::setupSkybox(string path)
{
	vector<std::string> faces = {
		path + "/right.png",
		path + "/left.png",
		path + "/top.png",
		path + "/bottom.png",
		path + "/front.png",
		path + "/back.png"
	};
	textureId = loadCubemap(faces);  
    skyboxCube.setup();
}

void Skybox::draw(Shader shader) 
{
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glCullFace(GL_FRONT);

    shader.use();
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
    glActiveTexture(GL_TEXTURE0);
    skyboxCube.draw(shader);

    glCullFace(GL_BACK);
    glDepthMask(GL_TRUE);
}

