#pragma once
#include "NaniteMesh.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

class Model {
private:
	vector<Mesh*> meshes;
	vector<Texture> loadedTextures;
	string directory;
	aiVector3D modelMin;
	aiVector3D modelMax;
	float scaleFactor;

	void LoadModel(string path, bool useNanite);
	void ProcessNode(aiNode* node, const aiScene* scene, bool useNanite);
	Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene, bool useNanite);
	vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);

	void ComputeBoundingBox(aiNode* node, const aiScene* scene);
public:
	Model() {};
	Model(string path, bool useNanite);
	vector<Mesh*>&GetMeshes();
	vector<Texture>&GetTextures();

};
