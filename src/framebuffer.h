#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <vector>

struct Framebuffer {
    unsigned int id;
    std::vector<unsigned int> colorTextures;
};

class Framebuffers
{
    public:
        Framebuffers(int width, int height);
        Framebuffer mainFramebuffer;
        Framebuffer normalBlendFramebuffer;
        Framebuffer pingpongBuffers[2];
    private:
        void createMainFramebuffer(int width, int height);
        void createNormalBlendingFramebuffer(int width, int height);
        void createPingpongFramebuffer(int width, int height);
};

#endif
