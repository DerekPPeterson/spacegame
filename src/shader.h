#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include "nocopy.h"


enum CommonUniforms
{
    UNIFORM_MODEL,
    UNIFORM_VIEW,
    UNIFORM_PROJECTION
};

class Shader : public non_copyable
{
    public:
        unsigned int ID;
        
        Shader(const GLchar* vertexPath, const GLchar* fragmentPath);

        void use() const;

        void setBool(const std::string &name, bool value) const;
        void setInt(const std::string &name, int value) const;
        void setFloat(const std::string &name, float value) const;
        void set4Float(const std::string &name, float x, float y, float z, float w) const;
        void setMat4(const std::string &name, const glm::mat4 &mat) const;
        void setVec3(const std::string &name, const glm::vec3 &vec) const;
        void setVec2(const std::string &name, const glm::vec2 &vec) const;
        void setCommon(CommonUniforms which, glm::mat4 val) const;
    private:
        void saveCommonUniformLocations();
        std::unordered_map<CommonUniforms, unsigned int> locations;
};

#endif
