#pragma once
#include <vector>
#include "Cluster.h"
#include "Partitioner.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "co/fs.h"

typedef struct Cluster Cluster;
typedef struct ClusterGroup ClusterGroup;

class NaniteMesh:public Mesh {
private:
	void Build();
	void SetUpMesh();
	bool firstDraw;
public:
	vector<Cluster> clusters;
	vector<ClusterGroup> clusterGroups;
	u32 numMipLevel;
	NaniteMesh(vector<vec3> pos, vector<unsigned int> indices = vector<unsigned int>(0), vector<Texture> textures = vector<Texture>(0));
	void Draw(Shader& shader);
	void AddMode();
};
