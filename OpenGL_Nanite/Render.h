#pragma once
// Renderer.h
// Renderer ���װ�� OpenGL ����������ɫ������Ⱦ����

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

    // ��ʼ�� OpenGL ���󡢼������ݺ���ɫ��
    void Init();

    // ��Ⱦ����
    void Render();

    void CommonRender();
    // ������Դ
    void Cleanup();

    //�������
    void SetCamera(Camera* camera);

    //������Ļ��С
    void SetScreenSize(GLFWwindow* window ,int width, int height);

    //������Ⱦģʽ
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
    // ��ʼ����Ȼ���
    void InitDepthBuffer();

    void UpdateFPS();
    
    //��Ļ��С
    GLFWwindow* window;
    int screenWidth;
    int screenHeight;

    // ��ɫ������
    Shader* depthShader;        //�����Ⱦ��ɫ��
    Shader* computeShader;      // compute shader ���� LOD �л���ɼ����ж�
    Shader* renderShader;

    unsigned int ssboClusters;      // �洢 GPUCluster ����
    unsigned int ssboIndirectCmd;   // �洢��ӻ�����������
    unsigned int ssboCmdCounter;    // �洢ԭ�Ӽ�������ͳ�ƿɼ� Cluster ����
    unsigned int ssboTriCount;  //�洢����������
    unsigned int secondSsboIndirectCmd;
    unsigned int secondSsboCmdCounter;
    //�洢
    vector<Mesh*>meshes;
    vector<GPUCluster> gpuClusters;
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;

    //��Ⱦģʽ��0�������Σ� 1��Cluster ��2��ClusterGroup�� 3��LODlevel 4��������ͼ
    u32 viewMode;

    u32 renderMode;

    // �����
    Camera* camera;

    //�ڵ��޳�
    bool firstFrame;
    GLuint depthFBO, depthTexture;
    HZBManager* hzbManager;
    glm::mat4 prevView, prevProj;
    glm::mat4 currentView, currentProj;

    // ��ǰ Cluster ����
    unsigned int clusterCount;
    unsigned int triCount;

    //����
    GLuint textureArray;
    u32 numTextures;
    //֡��
    float lastTime = 0.0f;
    int frameCount = 0;
    float fps = 0.0f;
};

#endif // RENDERER_H
