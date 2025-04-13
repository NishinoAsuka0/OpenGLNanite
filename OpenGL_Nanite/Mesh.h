#pragma once
#include "Shaders.h"
#include "VecType.h"
#include "stb_image.h"
#include <vector>


struct Texture {
    unsigned int id;
    string type;
};

class Mesh {

protected:
    /*  网格数据  */
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    vector<vec3>color;
    unsigned int VAO, VBO, EBO;
    Shader* renderShader;       // 用于渲染的着色器
    GLuint textureID;
    u32 viewMode;
    unsigned int triCount;

    //转换矩阵
    glm::mat4 transformMatrix;
    glm::mat4 model;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
public:
    /*  函数  */
    Mesh() { viewMode = 0; };
    Mesh(vector<Vertex> pos, vector<vec3>color, vector<unsigned int> indices = vector<unsigned int>(0), vector<Texture> textures = vector<Texture>(0));

    vector<Vertex>& GetVertices();

    vector<unsigned int>& GetIndices();

    vector<Texture>& GetTexture();

    void SetViewMode(u32 mode) { this->viewMode = mode; };

    virtual uvec2 Draw();
    virtual void Init();
    virtual void LoadTex(string texPath);
    virtual void SetInfo(glm::mat4 _transformMatrix,
        glm::mat4 _model,
        glm::mat4 _viewMatrix,
        glm::mat4 _projectionMatrix);
};