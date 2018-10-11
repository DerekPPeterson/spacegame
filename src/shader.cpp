#include <exception>
#include <sstream>

#include "shader.h"

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
        std::cout << "ERROR: Failed to compile " << path << "\n" << 
            infoLog << std::endl;
        throw std::exception();
    }

    return shaderID;
}

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
        std::cout << "ERROR: shader linking failed\n" << infoLog << std::endl;
        throw std::exception();
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
}

void Shader::use()
{
    glUseProgram(ID);
}
