#include "framebuffer.h"

#include <iostream>
#include <plog/Log.h>

#include "glad/glad.h"

Framebuffers::Framebuffers(int width, int height)
{
    // main antialiased 
    createMainFramebuffer(mainFramebufferMultisampled, 1, width, height, true);

    createMainFramebuffer(mainFramebuffer, 1, width, height, false);
    createMainFramebuffer(warpFrameBuffer, 1, width, height, false);
    // Bind bright color texture to both frame buffers so bloom is applied to warp effects
    glBindFramebuffer(GL_FRAMEBUFFER, warpFrameBuffer.id);
    glBindTexture(GL_TEXTURE_2D, mainFramebuffer.colorTextures[1]);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mainFramebuffer.colorTextures[1], 0);
    createNormalBlendingFramebuffer(width, height);
    createPingpongFramebuffer(width / 2, height / 2);
}

void Framebuffers::createMainFramebuffer(Framebuffer& buf, int nColorBuffers, int width, int height, bool msaa)
{
    glGenFramebuffers(1, &buf.id);
    glBindFramebuffer(GL_FRAMEBUFFER, buf.id);
    
    // 2nd texture is for bloom
    buf.colorTextures.resize(nColorBuffers);

    std::vector<unsigned int> attachments;
    for (int i = 0; i < nColorBuffers; i++ ) {
        glGenTextures(1, &buf.colorTextures[i]);

        if ( msaa) {
            glEnable(GL_MULTISAMPLE);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, buf.colorTextures[i]);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F, width, height, GL_TRUE);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, buf.colorTextures[i], 0);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        } else {
            glBindTexture(GL_TEXTURE_2D, buf.colorTextures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, buf.colorTextures[i], 0);
        }
        attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
    }

    glDrawBuffers(2, attachments.data());  

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
    if(msaa) {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);  
    } else {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);  
    }
        

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LOG_ERROR << "Framebuffer is not complete!";
            throw std::runtime_error("Could not complete framebuffer");
    }

    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
};


void Framebuffers::createPingpongFramebuffer(int width, int height) {
    // ping pong buffer (for rendering bloom)
	for (unsigned int i = 0; i < 2; i++)
	{
        pingpongBuffers[i].colorTextures.resize(1);
        glGenFramebuffers(1, &pingpongBuffers[i].id);
        glGenTextures(1, &pingpongBuffers[i].colorTextures[0]);

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

void Framebuffers::createNormalBlendingFramebuffer(int width, int height) {
    // ping pong buffer (for rendering bloom)
        glGenFramebuffers(1, &normalBlendFramebuffer.id);
        normalBlendFramebuffer.colorTextures.resize(1);
        glGenTextures(1, &normalBlendFramebuffer.colorTextures[0]);

		glBindFramebuffer(GL_FRAMEBUFFER, normalBlendFramebuffer.id);
		glBindTexture(GL_TEXTURE_2D, normalBlendFramebuffer.colorTextures[0]);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normalBlendFramebuffer.colorTextures[0], 0
		);

        glEnablei(GL_BLEND, normalBlendFramebuffer.id);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);
}
