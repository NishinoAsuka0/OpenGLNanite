#pragma once
#include "NaniteMesh.h"

class Model {
private:
	vector<Mesh*> meshes;
	vector<Texture> loadedTextures;
	string directory;

	void LoadModel(string path, bool useNanite);
	void ProcessNode(aiNode* node, const aiScene* scene, bool useNanite);
	Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene, bool useNanite);
	vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
public:
	Model() {};
	Model(string path, bool useNanite);
	vector<Mesh*>&GetMeshes();
	vector<Texture>&GetTextures();

};
