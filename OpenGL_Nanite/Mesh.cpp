#include "Mesh.h"

void Mesh::SetUpMesh()
{
    posVBO.push_back(0);
    colorVBO.push_back(0);
    EBO.push_back(0);
    VAO.push_back(0);
    color = vector<vec3>(0);
    for (int i = 0; i < vertices.size(); ++i) {
        color.push_back(vec3((f32)(rand() % 255) / 255.0f, (f32)(rand() % 255) / 255.0f, (f32)(rand() % 255) / 255.0f));
    }
    // create buffers/arrays
    glGenVertexArrays(1, &VAO[0]);
    glGenBuffers(1, &posVBO[0]);
    glGenBuffers(1, &colorVBO[0]);

    glBindVertexArray(VAO[0]);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, posVBO[0]);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(vec3), &this->vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, colorVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, this->color.size() * sizeof(vec3), &this->color[0], GL_STATIC_DRAW);
    if (indices.size() != 0) {
        glGenBuffers(1, &EBO[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), &this->indices[0], GL_STATIC_DRAW);
    }
    // set the vertex attribute pointers
    // vertex Positions
    glBindBuffer(GL_ARRAY_BUFFER, posVBO[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    // vertex color
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO[0]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    // vertex texture coords
    //glEnableVertexAttribArray(2);
    //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
}

Mesh::Mesh(vector<vec3> vertices, vector<vec3>color, vector<unsigned int> indices, vector<Texture> textures)
{
	this->vertices = vertices;
    this->color = color;
	this->indices = indices;
	this->textures = textures;
    
	SetUpMesh();
}

vector<vec3>& Mesh::GetVertices()
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

void Mesh::Draw(Shader& shader)
{
	// draw mesh
	glBindVertexArray(VAO[0]);
	if (indices.size() != 0) {
		glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(this->indices.size()), GL_UNSIGNED_INT, 0);
	}
	else {
		glDrawArrays(GL_TRIANGLES, 0, static_cast<unsigned int>(this->vertices.size()));
	}
	glBindVertexArray(0);
}

void Mesh::AddMode()
{
    drawMode = (drawMode + 1) % 3;
}
