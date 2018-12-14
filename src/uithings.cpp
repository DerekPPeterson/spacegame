#include "uithings.h"

using namespace std;


//shared_ptr<Model> WarpBeacon::model;
//
//void WarpBeacon::setup()
//{
//    model = shared_ptr<Model>(new Model("./res/models/warp_diamond/warp_diamond.obj"));
//}
//
//void WarpBeacon::draw(Shader& shader) 
//{
//    glm::mat4 modelMat(1.0f);
//    modelMat = glm::translate(modelMat, position);
//    modelMat = glm::rotate(modelMat, rotation, {0, 1, 0});
//    modelMat = glm::scale(modelMat, glm::vec3(size));
//    shader.setCommon(UNIFORM_MODEL, modelMat);
//    model->draw(shader);
//}
//
//void WarpBeacon::update(UpdateInfo& info) 
//{
//    rotation = info.curTime * 2 * 3.14 * 0.5;
//    position.y += 0.1 * sin(2 * 3.14 * 0.5 * info.curTime) * info.deltaTime;
//}
