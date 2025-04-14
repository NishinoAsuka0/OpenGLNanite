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
#include "HZBManager.h"

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

    void InitBuffers();
    void AddMesh(Mesh* mesh);

    void LoadTex(string dataName);
    void LoadClusters();
    void GenerateClusters(vector<PackedCluster>clusters, int texID);
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
    Shader* computeShader;      // compute shader 用于 LOD 切换与可见性判断
    Shader* renderShader;

    unsigned int ssboClusters;      // 存储 GPUCluster 数据
    unsigned int ssboIndirectCmd;   // 存储间接绘制命令数组
    unsigned int ssboCmdCounter;    // 存储原子计数器，统计可见 Cluster 数量
    unsigned int ssboTriCount;  //存储三角形数量
    unsigned int secondSsboIndirectCmd;
    unsigned int secondSsboCmdCounter;
    //存储
    vector<Mesh*>meshes;
    vector<GPUCluster> gpuClusters;
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;

    //渲染模式，0：三角形， 1：Cluster ，2：ClusterGroup， 3：LODlevel 4：纹理贴图
    u32 viewMode;

    u32 renderMode;

    // 摄像机
    Camera* camera;

    //遮挡剔除
    bool firstFrame;
    GLuint depthFBO, depthTexture;
    HZBManager* hzbManager;
    glm::mat4 prevView, prevProj;
    glm::mat4 currentView, currentProj;

    // 当前 Cluster 数量
    unsigned int clusterCount;
    unsigned int triCount;

    //纹理
    GLuint textureArray;
    u32 numTextures;
    //帧率
    float lastTime = 0.0f;
    int frameCount = 0;
    float fps = 0.0f;
};

#endif // RENDERER_H
