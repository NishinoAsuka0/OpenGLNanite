#pragma once
#include <vector>
#include "Cluster.h"
#include "Partitioner.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "co/fs.h"
#include "Mesh.h"

typedef struct Cluster Cluster;
typedef struct ClusterGroup ClusterGroup;
typedef struct GPUCluster GPUCluster;
typedef struct PackedCluster PackedCluster;
typedef struct Texture Texture;
typedef class Mesh Mesh;

class NaniteMesh:public Mesh {
private:
	void Build();

public:
	vector<Cluster> clusters;
	vector<ClusterGroup> clusterGroups;
	u32 numMipLevel;
	NaniteMesh(vector<Vertex> pos, vector<unsigned int> indices = vector<unsigned int>(0), vector<Texture> textures = vector<Texture>(0));
};
