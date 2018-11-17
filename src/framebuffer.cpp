#include "framebuffer.h"

#include <iostream>

#include "glad/glad.h"

Framebuffers::Framebuffers(int width, int height)
{
    createMainFramebuffer(width, height);
    createPingpongFramebuffer(width / 2, height / 2);
}

void Framebuffers::createMainFramebuffer(int width, int height)
{
    glGenFramebuffers(1, &mainFramebuffer.id);
    glBindFramebuffer(GL_FRAMEBUFFER, mainFramebuffer.id);
    
    // 2nd texture is for bloom
    mainFramebuffer.colorTextures.resize(2);
    for (int i = 0; i < 2; i++ ) {
        glGenTextures(2, &mainFramebuffer.colorTextures[i]);
        glBindTexture(GL_TEXTURE_2D, mainFramebuffer.colorTextures[i]);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, mainFramebuffer.colorTextures[i], 0);
    }

    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);  

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);  

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
};


void Framebuffers::createPingpongFramebuffer(int width, int height) {
    // ping pong buffer (for rendering bloom)
	for (unsigned int i = 0; i < 2; i++)
	{
        pingpongBuffers[i].colorTextures.resize(1);
        glGenFramebuffers(1, &pingpongBuffers[i].id);
        glGenTextures(2, &pingpongBuffers[i].colorTextures[0]);

		glBindFramebuffer(GL_FRAMEBUFFER, pingpongBuffers[i].id);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffers[i].colorTextures[0]);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffers[i].colorTextures[0], 0
		);
	}
}
