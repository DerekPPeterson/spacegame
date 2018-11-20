#include "shapes.h"

namespace shapes {

DrawableFromVertexArray quad2DTexCoords;
DrawableFromVertexArray quad3DXYTexCoord;

float quad2DTexCoordvertices[] = {  
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
    };	

float quad3DXYTexCoordvertices[] = {  
    // positions         // texCoords
    -1.0f,  1.0f,  0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f, 0.0f,
     1.0f, -1.0f,  0.0f, 1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 0.0f, 1.0f,
     1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
     1.0f,  1.0f,  0.0f, 1.0f, 1.0f
    };	

void setup() {
    // Only run setup once
    static bool isSetup = false;
    if (isSetup) return;

    quad2DTexCoords = DrawableFromVertexArray(quad2DTexCoordvertices, 6, 4);
    quad2DTexCoords.addAttribute(2);
    quad2DTexCoords.addAttribute(2);

    quad3DXYTexCoord = DrawableFromVertexArray(quad3DXYTexCoordvertices, 6, 5);
    quad3DXYTexCoord.addAttribute(3);
    quad3DXYTexCoord.addAttribute(2);
};

};
