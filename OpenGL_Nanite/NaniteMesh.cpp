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
	//将三角形分簇
	ClusterTriangles(vertices, indices, clusters);

	u32 levelOffset = 0;
	u32 mipLevel = 0;

	cout << "Begin building cluster DAG" << endl;

	while (true) {
		cout << "====Now Level is : " << mipLevel << "====" << endl;
		cout << "Cluster Nums: " << clusters.size() - levelOffset << endl;
		LogClusterSize(clusters.data(), levelOffset, clusters.size());

		u32 numLevelClusters = clusters.size() - levelOffset;
		if (numLevelClusters < 1) break;

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

		levelOffset = preClusterNum;
		mipLevel++;
	}

	numMipLevel = mipLevel + 1;

	cout << "End building cluster DAG" << endl;

	cout << "Total clusters: " << clusters.size() << endl;

	cout << "End building NaniteMesh" << endl;

}

NaniteMesh::NaniteMesh(vector<vec3> pos, vector<unsigned int> indices, vector<Texture> textures)
{
	this->vertices = pos;
	this->indices = indices;
	this->textures = textures;
	this->firstDraw = true;
	this->Build();
}