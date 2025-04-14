#include "NaniteMesh.h"

void LogClusterSize(Cluster* clusters, u32 begin, u32 end) {
	f32 maxsz = 0, minsz = 100000, avgsz = 0;
	for (u32 i = begin; i < end; i++) {
		auto& cluster = clusters[i];
		//assert(cluster.verts.size() < 256);
		f32 sz = cluster.indexes.size() / 3.0;
		if (sz > maxsz) maxsz = sz;
		if (sz < minsz) minsz = sz;
		avgsz += sz;
	}
	avgsz /= end - begin;
	cout << "cluster size: max = " << maxsz << " min = " << minsz << " avg = " << avgsz << endl;
}

void LogGroupSize(ClusterGroup* groups, u32 begin, u32 end) {
	f32 maxsz = 0, minsz = 100000, avgsz = 0;
	for (u32 i = begin; i < end; i++) {
		f32 sz = groups[i].clusters.size();
		if (sz > maxsz) maxsz = sz;
		if (sz < minsz) minsz = sz;
		avgsz += sz;
	}
	avgsz /= end - begin;
	cout << "group size: max = " << maxsz << " min = " << minsz << " avg = " << avgsz << endl;
}

void NaniteMesh::Build()
{
	cout << "Begin building NaniteMesh" << endl;

	cout << "Fix up Mesh: " << endl;
	cout << "Mesh verts: " << vertices.size() 
		 << " Mesh triangles: " << indices.size() / 3 << endl;

	/*FILE* file = fopen("Model/test.txt", "rb");
	
	size_t psize;
	size_t isize;
	fread(&psize, sizeof(size_t), 1, file);
	vector<vec3>pos(psize);
	for (auto& p : pos) {
		fread(&p.x, sizeof(float), 1, file);
		fread(&p.y, sizeof(float), 1, file);
		fread(&p.z, sizeof(float), 1, file);
		p.x = p.x / 500.0f;
		p.y = p.y / 500.0f;
		p.z = p.z / 500.0f;
	}
	fread(&isize, sizeof(size_t), 1, file);
	vector<u32>idx(isize);
	for (auto& p : idx) {
		fread(&p, sizeof(unsigned int), 1, file);
	}*/
	//将三角形分簇
	
	ClusterTriangles(vertices, indices, clusters);

	u32 levelOffset = 0;
	u32 mipLevel = 0;
	u32 preNumLevelClusters = 0;

	cout << "Begin building cluster DAG" << endl;

	while (true) {
		cout << "====Now Level is : " << mipLevel << "====" << endl;
		cout << "Cluster Nums: " << clusters.size() - levelOffset << endl;
		LogClusterSize(clusters.data(), levelOffset, clusters.size());

		u32 numLevelClusters = clusters.size() - levelOffset;
		if (numLevelClusters == preNumLevelClusters) break;

		u32 preClusterNum = clusters.size();
		u32 preGroupNum = clusterGroups.size();

		//给Cluster分组
		GroupCluster(
			clusters,
			levelOffset,
			numLevelClusters,
			clusterGroups,
			mipLevel
		);

		cout << "Group Num: " << clusterGroups.size() - preGroupNum << endl;
		LogGroupSize(clusterGroups.data(), preGroupNum, clusterGroups.size());
		//合并Group内的Cluster并构造DAG
		cout << "building cluster DAG: " << endl;
		if (numLevelClusters <= 1) break;
		for (u32 i = preGroupNum; i < clusterGroups.size(); ++i) {
			BuildParentClusters(clusterGroups[i], clusters);
		}
		preNumLevelClusters = numLevelClusters;
		levelOffset = preClusterNum;
		mipLevel++;
	}

	numMipLevel = mipLevel + 1;

	cout << "End building cluster DAG" << endl;

	cout << "Total clusters: " << clusters.size() << endl;

	cout << "End building NaniteMesh" << endl;

}

NaniteMesh::NaniteMesh(vector<Vertex> pos, vector<unsigned int> indices, vector<Texture> textures)
{
	this->vertices = pos;
	this->indices = indices;
	this->textures = textures;
	this->Build();
}
