#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <vector>

typedef struct Framebuffer {
    unsigned int id;
    std::vector<unsigned int> colorTextures;
} Framebuffer;

class Framebuffers
{
    public:
        Framebuffers(int width, int height);
        Framebuffer mainFramebuffer;
        Framebuffer warpFrameBuffer;
        Framebuffer normalBlendFramebuffer;
        Framebuffer pingpongBuffers[2];
    private:
        void createMainFramebuffer(Framebuffer& buf, int numColorBuffers, int width, int height);
        void createNormalBlendingFramebuffer(int width, int height);
        void createPingpongFramebuffer(int width, int height);
};

#endif
