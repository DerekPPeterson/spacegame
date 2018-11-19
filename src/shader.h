#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


using std::string;

class Shader
{
    public:
        unsigned int ID;
        
        Shader(const GLchar* vertexPath, const GLchar* fragmentPath);

        void use();

        void setBool(const string &name, bool value) const;
        void setInt(const string &name, int value) const;
        void setFloat(const string &name, float value) const;
        void set4Float(const string &name, float x, float y, float z, float w) const;
        void setMat4(const std::string &name, const glm::mat4 &mat) const;
        void setVec3(const std::string &name, const glm::vec3 &vec) const;
        void setVec2(const std::string &name, const glm::vec2 &vec) const;
};

#endif
