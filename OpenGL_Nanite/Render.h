#pragma once
// Renderer.h
// Renderer 类封装了 OpenGL 缓冲区、着色器和渲染流程

#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include "OpenGLwin.h"
#include "Camera.h"
#include "Cluster.h"
#include "NaniteMesh.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "stb_image.h"

class Renderer
{
public:
    Renderer();
    ~Renderer();

    // 初始化 OpenGL 对象、加载数据和着色器
    void Init();
    // 渲染场景
    void Render();

    void CommonRender();
    // 清理资源
    void Cleanup();

    //传入相机
    void SetCamera(Camera* camera);

    //传入屏幕大小
    void SetScreenSize(GLFWwindow* window ,int width, int height);

    //设置渲染模式
    void SetViewMode(u32 viewMode);

    void SetRenderMode(u32 mode);

    void SetClusterCount(u32 clusterCount);
    void SetTriCount(u32 triCount);

    void AddMesh(Mesh* mesh);
private:
    // 初始化深度缓冲
    void InitDepthBuffer();

    void UpdateFPS();
    
    //屏幕大小
    GLFWwindow* window;
    int screenWidth;
    int screenHeight;

    // 着色器对象
    Shader* depthShader;        //深度渲染着色器

    //存储
    vector<Mesh*>meshes;


    //渲染模式，0：三角形， 1：Cluster ，2：ClusterGroup， 3：LODlevel 4：纹理贴图
    u32 viewMode;

    u32 renderMode;

    // 摄像机
    Camera* camera;

    //遮挡剔除
    GLuint depthTextures[3];  // 0:N-2帧  1:N-1帧  2:当前帧
    GLuint depthFBOs[3];      // 三个FBO配套深度附件

    int currentDepthIndex = 2; // 当前写入的depth index
    int prevDepthIndex = 1;    // 上一帧index
    int prevPrevDepthIndex = 0; // 上上帧index

    // 当前 Cluster 数量
    unsigned int clusterCount;
    unsigned int triCount;

    //帧率
    float lastTime = 0.0f;
    int frameCount = 0;
    float fps = 0.0f;
};

#endif // RENDERER_H
