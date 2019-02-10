#include <exception>
#include <sstream>

#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace std;

string read_whole_file(const char *path)
{
    std::stringstream buf;
    std::ifstream fs;
    fs.open(path);
    buf << fs.rdbuf();
    string contents = buf.str();
    fs.close();
    return buf.str();
}

unsigned int compile_shader_from_filename(GLenum type, const char * path)
{
    string source = read_whole_file(path);
    const char * source_c_str = source.c_str();

    int success;
    char infoLog[512];

    unsigned int shaderID;
    shaderID = glCreateShader(type);
    glShaderSource(shaderID, 1, &source_c_str, NULL);
    glCompileShader(shaderID);

    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
            infoLog;
        throw std::runtime_error(infoLog);
    }

    return shaderID;
}

unordered_map<CommonUniforms, string> commonUniformNames = {
    {UNIFORM_MODEL, "model"},
    {UNIFORM_VIEW, "view"},
    {UNIFORM_PROJECTION, "projection"},
    {UNIFORM_COLOR, "color"},
    {UNIFORM_USE_VIEW, "useView"},
};

void Shader::saveCommonUniformLocations()
{
    locations.resize(commonUniformNames.size());
    for (auto pair : commonUniformNames) {
        locations[pair.first] = glGetUniformLocation(ID, pair.second.c_str());
    }
};

unsigned int link_shaders(unsigned int vertexShader, unsigned int fragmentShader)
{
    int success;
    char infoLog[512];

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // At this point shader program should be ready to use, just checking
    // success and cleaning up afterwards

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        throw std::runtime_error(infoLog);
    }
    // Shaders are not needed after they are linked
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

Shader::Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
{
    unsigned int vertexShader = compile_shader_from_filename(GL_VERTEX_SHADER, vertexPath);
    unsigned int fragmentShader = compile_shader_from_filename(GL_FRAGMENT_SHADER, fragmentPath);
    
    ID = link_shaders(vertexShader, fragmentShader);
    saveCommonUniformLocations();
}

void Shader::use()
{
    glUseProgram(ID);
}

void Shader::setBool(const std::string &name, bool value) const
{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}
void Shader::setInt(const std::string &name, int value) const
{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}
void Shader::setFloat(const std::string &name, float value) const
{ 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
} 
void Shader::set4Float(const std::string &name, float x, float y, float z, float w) const
{ 
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
} 
void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void Shader::setVec3(const std::string &name, const glm::vec3 &vec) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), vec[0], vec[1], vec[2]);
}
void Shader::setVec2(const std::string &name, const glm::vec2 &vec) const
{
	glUniform2f(glGetUniformLocation(ID, name.c_str()), vec[0], vec[1]);
}
void Shader::setCommon(CommonUniforms which, glm::mat4 val) const
{
	glUniformMatrix4fv((int) locations.at(which), 1, GL_FALSE, &val[0][0]);
}
void Shader::setCommon(CommonUniforms which, glm::vec3 val) const
{
	glUniform3f((int) locations.at(which), val[0], val[1], val[2]);
}
void Shader::setCommon(CommonUniforms which, bool val) const
{
	glUniform1i((int) locations.at(which), val);
}

