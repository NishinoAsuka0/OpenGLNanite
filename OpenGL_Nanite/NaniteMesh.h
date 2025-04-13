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
	bool firstFrame;
	Shader* computeShader;      // compute shader ���� LOD �л���ɼ����ж�

	unsigned int ssboClusters;      // �洢 GPUCluster ����
	unsigned int ssboIndirectCmd;   // �洢��ӻ�����������
	unsigned int ssboCmdCounter;    // �洢ԭ�Ӽ�������ͳ�ƿɼ� Cluster ����
	unsigned int ssboTriCount;  //�洢����������
	unsigned int secondSsboIndirectCmd;
	unsigned int secondSsboCmdCounter;

	unsigned int clusterCount;
	unsigned int firstDrawCount;
	unsigned int secondDrawCount;

	unsigned int mode;

	unsigned one, two, zero;
	//����
	glm::mat4 prevView, prevProj;
	glm::mat4 currentView, currentProj;

public:
	vector<Cluster> clusters;
	vector<ClusterGroup> clusterGroups;
	vector<GPUCluster> gpuClusters;
	u32 numMipLevel;
	NaniteMesh() { Init(); };
	NaniteMesh(vector<Vertex> pos, vector<unsigned int> indices = vector<unsigned int>(0), vector<Texture> textures = vector<Texture>(0));
	void GenerateClusters(vector<PackedCluster>& clusters);
	uvec2 Draw();
	void Init();
	void ComputePass(GLuint depthTex, u32 screenWidth, u32 screenHeight, int mode);
	void DrawDepth(Shader* depthShader);
	void SetInfo(glm::mat4 _transformMatrix,
		glm::mat4 _model,
		glm::mat4 _viewMatrix,
		glm::mat4 _projectionMatrix);
};
