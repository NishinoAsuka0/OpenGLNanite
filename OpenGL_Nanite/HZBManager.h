#pragma once
#include "Shaders.h"
#include <iostream>

class HZBManager {
public:
    void Initialize(int width, int height);
    void UpdateCurrentHZB(GLuint currentDepthTexture);
    void SwapHZB();

    GLuint GetPrevHZBTexture() const { return prevHZBTexture; }
    GLuint GetCurrentHZBTexture() const { return currentHZBTexture; }
    GLuint GetHZBLevels() const { return hzbLevels; }

private:
    GLuint prevHZBTexture = 0;
    GLuint currentHZBTexture = 0;
    GLuint hzbLevels = 0;
    Shader* hzbGenShader = nullptr; 
    u32 screenWidth;
    u32 screenHeight;
};
