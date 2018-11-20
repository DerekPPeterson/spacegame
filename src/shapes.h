/* shapes.h
 * Defines commonly used drawables such as quads, cubes and spheres
 */

#ifndef SHAPES_H
#define SHAPES_H

#include "drawable.h"

namespace shapes {

void setup();

// Quad to use for rendering framebuffers to
extern DrawableFromVertexArray quad2DTexCoords;
extern DrawableFromVertexArray quad3DXYTexCoord;


};


#endif
