#pragma once
#include "Shaders.h"
#include "VecType.h"
#include <vector>

struct Vertex {
    vec3 position;
    vec3 color;
    vec2 texCoords;
};


struct Texture {
    unsigned int id;
    string type;
};

class Mesh {
protected:
    /*  网格数据  */
    vector<vec3> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    vector<vec3>color;
    vector<unsigned int> EBO, posVBO, colorVBO;

    u32 drawMode;
    virtual void SetUpMesh();
public:
    vector<unsigned int> VAO;
    /*  函数  */
    Mesh() { drawMode = 0; };
    Mesh(vector<vec3> pos, vector<vec3>color, vector<unsigned int> indices = vector<unsigned int>(0), vector<Texture> textures = vector<Texture>(0));

    vector<vec3>& GetVertices();

    vector<unsigned int>& GetIndices();

    vector<Texture>& GetTexture();

    virtual void Draw(Shader &shader);

    virtual void AddMode();
};