#ifndef LINEOBJLOAD_H
#define LINEOBJLOAD_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct LineObj
{
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;

};

LineObj readLineObj(std::string path)
{
    LineObj model;
    
    std::ifstream file;
    file.open(path);


    for (std::string line; getline(file, line);) {
        std::string label;
        switch (line[0]) {
            case 'v':
                glm::vec3 v;
                std::stringstream(line) >> label >> v.x >> v.y >> v.z;
                model.vertices.push_back(v);
                break;
            case 'l':
                unsigned int i1, i2;
                std::stringstream(line) >> label >> i1 >> i2;
                model.indices.push_back(i1 - 1);
                model.indices.push_back(i2 - 1);
                break;
            default:
                ;
        }
    }

    return model;
}

#endif
