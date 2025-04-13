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

    void AddMesh(Mesh* mesh);
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

    //�洢
    vector<Mesh*>meshes;


    //��Ⱦģʽ��0�������Σ� 1��Cluster ��2��ClusterGroup�� 3��LODlevel 4��������ͼ
    u32 viewMode;

    u32 renderMode;

    // �����
    Camera* camera;

    //�ڵ��޳�
    GLuint depthTextures[3];  // 0:N-2֡  1:N-1֡  2:��ǰ֡
    GLuint depthFBOs[3];      // ����FBO������ȸ���

    int currentDepthIndex = 2; // ��ǰд���depth index
    int prevDepthIndex = 1;    // ��һ֡index
    int prevPrevDepthIndex = 0; // ����֡index

    // ��ǰ Cluster ����
    unsigned int clusterCount;
    unsigned int triCount;

    //֡��
    float lastTime = 0.0f;
    int frameCount = 0;
    float fps = 0.0f;
};

#endif // RENDERER_H
