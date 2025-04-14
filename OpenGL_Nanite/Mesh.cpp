#include "Mesh.h"

Mesh::Mesh(vector<Vertex> vertices, vector<vec3>color, vector<unsigned int> indices, vector<Texture> textures):VAO(0), VBO(0), EBO(0)
{
	this->vertices = vertices;
    this->color = color;
	this->indices = indices;
	this->textures = textures;

    triCount = indices.size() / 3;
    this->Init();
}

vector<Vertex>& Mesh::GetVertices()
{
    return this->vertices;
}

vector<unsigned int>& Mesh::GetIndices()
{
    return this->indices;
}

vector<Texture>& Mesh::GetTexture()
{
    return this->textures;
}

uvec2 Mesh::Draw()
{
    renderShader->Use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(renderShader->program, "ourTexture"), 0);

    int transformLocation = glGetUniformLocation(renderShader->program, "transformMatrix");
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));

    int projectionLocation = glGetUniformLocation(renderShader->program, "projectionMatrix");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    int viewLocation = glGetUniformLocation(renderShader->program, "viewMatrix");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    glBindVertexArray(VAO);

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    return uvec2{ (u32)indices.size() / 3, 0 };
}

void Mesh::Init()
{
    //初始化着色器
    renderShader = new Shader((GLchar*)"CommonVshader.txt", (GLchar*)"CommonFshader.txt");

    //初始化buffers
    //生成 VAO、VBO、EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    //VBO: 存储顶点位置
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);

    // EBO: 存储索引
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);

    // 顶点属性指针
    // 位置
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // 纹理坐标
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glBindVertexArray(0);
}

void Mesh::LoadTex(string texPath)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true); // OpenGL纹理坐标系需要翻转Y轴
    unsigned char* data = stbi_load(texPath.c_str(), &width, &height, &channels, 0);

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
}

void Mesh::SetInfo(glm::mat4 _transformMatrix, glm::mat4 _model, glm::mat4 _viewMatrix, glm::mat4 _projectionMatrix)
{
    transformMatrix = _transformMatrix;
    model = _model;
    viewMatrix = _viewMatrix;
    projectionMatrix = _projectionMatrix;
}
