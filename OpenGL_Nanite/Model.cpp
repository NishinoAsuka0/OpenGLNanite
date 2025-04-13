#include "Model.h"

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

	ProcessNode(scene->mRootNode, scene, useNanite);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene, bool useNanite)
{
	// 处理节点所有的网格（如果有的话）
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(mesh, scene, useNanite));
	}
	// 接下来对它的子节点重复这一过程
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene, useNanite);
	}
}

Mesh* Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, bool useNanite)
{
	vector<Vertex> vertices;
	vector<vec3> color;
	vector<unsigned int>indices;
	vector<Texture> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		// 处理顶点位置、法线和纹理坐标
		vec3 vector;

		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		//vertex.position = vector;

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
		//vertex.color = vector;

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
	for (unsigned int i = 0; i < mesh->mNumFaces*3; i+=3)
	{
		/*aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);*/
		assert(mesh->mFaces[i / 3].mNumIndices == 3);
		indices.push_back(mesh->mFaces[i / 3].mIndices[0]);
		indices.push_back(mesh->mFaces[i / 3].mIndices[1]);
		indices.push_back(mesh->mFaces[i / 3].mIndices[2]);
	}

	// 处理材质
	if (mesh->mMaterialIndex >= 0)
	{
		
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
