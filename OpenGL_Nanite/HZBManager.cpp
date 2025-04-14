#include "HZBManager.h"


void HZBManager::Initialize(int width, int height)
{
    int maxSize = std::max(width, height);
    hzbLevels = static_cast<GLuint>(std::floor(std::log2(maxSize))) + 1;

    for (int i = 0; i < 2; ++i) {
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexStorage2D(GL_TEXTURE_2D, hzbLevels, GL_R32F, width, height);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        //在初始化第一帧时，给prevHZBTexture赋一个全1的深度
        if (i == 0) {
            GLfloat maxDepth = 1.0f;  // 设置全为1
            glClearTexImage(tex, 0, GL_RED, GL_FLOAT, &maxDepth); 
            prevHZBTexture = tex;
        }
        else {
            currentHZBTexture = tex;
        }
    }

    hzbGenShader = new Shader((GLchar*)"HZBGen.comp");
}

void HZBManager::UpdateCurrentHZB(GLuint currentDepthTexture)
{
    glCopyImageSubData(
        currentDepthTexture, GL_TEXTURE_2D, 0, 0, 0, 0,
        currentHZBTexture, GL_TEXTURE_2D, 0, 0, 0, 0,
        screenWidth, screenHeight, 1
    );

    hzbGenShader->Use();
    for (GLuint i = 1; i < hzbLevels; ++i) {
        GLuint srcMip = i - 1;
        GLuint dstMip = i;
        u32 minSize = 1;
        int mipWidth = std::max(minSize, screenWidth >> dstMip);
        int mipHeight = std::max(minSize, screenHeight >> dstMip);

        glBindImageTexture(1, currentHZBTexture, dstMip, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentHZBTexture);
        glUniform1i(glGetUniformLocation(hzbGenShader->program, "srcMip"), srcMip);

        glDispatchCompute((mipWidth + 7) / 8, (mipHeight + 7) / 8, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    }
}

void HZBManager::SwapHZB()
{
    std::swap(prevHZBTexture, currentHZBTexture);
}

