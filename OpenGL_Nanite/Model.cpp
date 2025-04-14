#include "Model.h"

void Model::ComputeBoundingBox(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
            aiVector3D v = mesh->mVertices[j];
            modelMin.x = std::min(modelMin.x, v.x);
            modelMin.y = std::min(modelMin.y, v.y);
            modelMin.z = std::min(modelMin.z, v.z);
            modelMax.x = std::max(modelMax.x, v.x);
            modelMax.y = std::max(modelMax.y, v.y);
            modelMax.z = std::max(modelMax.z, v.z);
        }
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        ComputeBoundingBox(node->mChildren[i], scene);
    }
}

void Model::LoadModel(string path, bool useNanite)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);
	if (!scene) {
		std::cerr << "Assimp加载失败：" << importer.GetErrorString() << std::endl;
		return;
	}

	if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
		std::cerr << "模型加载不完整：" << importer.GetErrorString() << std::endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));

    modelMin = aiVector3D(1e10f, 1e10f, 1e10f);
    modelMax = aiVector3D(-1e10f, -1e10f, -1e10f);
    ComputeBoundingBox(scene->mRootNode, scene);

    aiVector3D size = modelMax - modelMin;
    float maxSize = std::max({ size.x, size.y, size.z });
    scaleFactor = 1.0f / maxSize;

	ProcessNode(scene->mRootNode, scene, useNanite);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene, bool useNanite)
{
	// 处理节点所有的网格
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(mesh, scene, useNanite));
	}
	
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene, useNanite);
	}
}

Mesh* Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, bool useNanite)
{
    vector<Vertex> vertices;
    vector<vec3> color;
    vector<unsigned int> indices;
    vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        vec3 vector;

        //缩放
        aiVector3D adjusted = mesh->mVertices[i] * scaleFactor;

        vector.x = adjusted.x;
        vector.y = adjusted.y;
        vector.z = adjusted.z;
        vertex.pos = vector;

        if (mesh->mNormals) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
        }
        else {
            vector = vec3(0.0f, 1.0f, 0.0f);
        }
        color.push_back(vector);

        if (mesh->mTextureCoords[0]) // 网格是否有纹理坐标？
        {
            vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.uv = vec;
        }
        else
            vertex.uv = vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }

    // 处理索引
    for (unsigned int i = 0; i < mesh->mNumFaces * 3; i += 3)
    {
        assert(mesh->mFaces[i / 3].mNumIndices == 3);
        indices.push_back(mesh->mFaces[i / 3].mIndices[0]);
        indices.push_back(mesh->mFaces[i / 3].mIndices[1]);
        indices.push_back(mesh->mFaces[i / 3].mIndices[2]);
    }

    if (mesh->mMaterialIndex >= 0)
    {
        // 这里你的原代码暂时没处理材质，可以后续补
    }

    if (useNanite) {
        return new NaniteMesh(vertices, indices);
    }
    return new Mesh(vertices, color, indices, textures);
}

vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
	return vector<Texture>();
}

Model::Model(string path, bool useNanite)
{
	LoadModel(path, useNanite);
}

vector<Mesh*>& Model::GetMeshes()
{
	return this->meshes;
}

vector<Texture>& Model::GetTextures()
{
	return this->loadedTextures;
}
