#ifndef CARDS_H
#define CARDS_H

#include "drawable.h"
#include "renderables.h"
#include "model.h"


class Card : public Renderable , public Object, public Selectable{
    public:
        Card();
        virtual void draw(Shader& shader) override;
        virtual void update(UpdateInfo& info) override;
        static void setup();
    private:
        static LineModel cardModel;
        glm::vec3 color = {0.5, 5, 5.5};
        float size = 0.2;
};

#endif
