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

class Renderer
{
public:
    Renderer();
    ~Renderer();

    // 初始化 OpenGL 对象、加载数据和着色器
    void Init();
    // 渲染场景
    void Render();
    // 清理资源
    void Cleanup();

    //深度纹理生成
    /*void InitDephtBuffer();
    void RenderDepthPass();*/


    //传入相机
    void SetCamera(Camera* camera);

    //传入屏幕大小
    void SetScreenSize(GLFWwindow* window ,int width, int height);

    //设置渲染模式
    void SetViewMode(u32 viewMode);

    void SetClusterCount(u32 clusterCount);
    void SetTriCount(u32 triCount);
    void GenerateClusters(vector<PackedCluster>&clusters);
private:
    // 初始化顶点/索引和 SSBO 缓冲区
    void InitBuffers();
    // 初始化 compute shader
    void InitComputeShader();
    // 初始化用于绘制 Cluster 的 shader 程序
    void InitRenderShader();

    void UpdateFPS();
    
    //屏幕大小
    GLFWwindow* window;
    int screenWidth;
    int screenHeight;

    // OpenGL 对象
    unsigned int VAO, VBO, EBO;
    unsigned int ssboClusters;      // 存储 GPUCluster 数据
    unsigned int ssboIndirectCmd;   // 存储间接绘制命令数组
    unsigned int ssboCmdCounter;    // 存储原子计数器，统计可见 Cluster 数量
    unsigned int ssboTriCount;  //存储三角形数量

    // 着色器对象
    Shader* renderShader;       // 用于渲染的着色器
    Shader* computeShader;      // compute shader 用于 LOD 切换与可见性判断

    // 存储 CPU 端的 Cluster 数据
    vector<GPUCluster> gpuClusters;
    // 存储顶点和索引数据
    vector<vec3> vertices;
    vector<u32> indices;

    //渲染模式，0：三角形， 1：Cluster ，2：ClusterGroup， 3：LODlevel
    u32 viewMode;

    // 摄像机
    Camera* camera;

    //遮挡剔除
    GLuint depthFBO;
    GLuint depthTexture;

    // 当前 Cluster 数量
    unsigned int clusterCount;
    unsigned int triCount;

    //帧率
    float lastTime = 0.0f;
    int frameCount = 0;
    float fps = 0.0f;
};

#endif // RENDERER_H
