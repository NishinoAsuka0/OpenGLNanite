#include "NaniteMesh.h"

u32 AsUint(f32 x) {
	return *((u32*)&x);
};

void PackingVirtualMesh(const NaniteMesh& vm, vector<u32>& packedData) {
	// vector<u32> packed_data;
	packedData.clear();

	packedData.push_back(vm.clusters.size()); //num cluster
	packedData.push_back(0);
	packedData.push_back(0);
	packedData.push_back(0);
	for (auto& cluster : vm.clusters) {
		packedData.push_back(cluster.verts.size()); //num vert
		packedData.push_back(0); //v data ofs
		packedData.push_back(cluster.indexes.size() / 3); //num tri
		packedData.push_back(0); //t data ofs

		packedData.push_back(AsUint(cluster.lodBounds.center.x)); //lod bounds
		packedData.push_back(AsUint(cluster.lodBounds.center.y));
		packedData.push_back(AsUint(cluster.lodBounds.center.z));
		packedData.push_back(AsUint(cluster.lodBounds.radius));

		Sphere parentLodBounds = vm.clusterGroups[cluster.groupId].lodBounds;
		f32 maxParentLodError = vm.clusterGroups[cluster.groupId].maxParentLodError;
		packedData.push_back(AsUint(parentLodBounds.center.x)); //parent lod bounds
		packedData.push_back(AsUint(parentLodBounds.center.y));
		packedData.push_back(AsUint(parentLodBounds.center.z));
		packedData.push_back(AsUint(parentLodBounds.radius));

		packedData.push_back(AsUint(cluster.lodError));
		packedData.push_back(AsUint(maxParentLodError));
		packedData.push_back(cluster.groupId);
		packedData.push_back(cluster.mipLevel);
	}
	u32 i = 0;
	for (auto& cluster : vm.clusters) {
		u32 ofs = 4 + 16 * i;
		packedData[ofs + 1] = packedData.size();
		for (vec3 p : cluster.verts) {
			packedData.push_back(AsUint(p.x));
			packedData.push_back(AsUint(p.y));
			packedData.push_back(AsUint(p.z));
		}

		packedData[ofs + 3] = packedData.size();
		for (u32 i = 0; i < cluster.indexes.size() / 3; i++) { //tri data
			u32 i0 = cluster.indexes[i * 3];
			u32 i1 = cluster.indexes[i * 3 + 1];
			u32 i2 = cluster.indexes[i * 3 + 2];
			//assert(i0 < 256 && i1 < 256 && i2 < 256);

			u32 packedTri = (i0 | (i1 << 8) | (i2 << 16));
			packedData.push_back(packedTri);
		}
		i++;
	}
}


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

	/*auto& vertex = mesh->GetVertices();
	auto& indices = mesh->GetIndices();*/

	cout << "Fix up Mesh: " << endl;
	cout << "Old Mesh verts: " << vertices.size() 
		 << " Mesh triangles: " << indices.size() / 3 << endl;
	//使用简化器去除重复点和三角形

	/*MeshSimplifier simplifier(vertices.data(), vertices.size(), indices.data(), indices.size());
	simplifier.Simplify(indices.size());
	vertices.resize(simplifier.RemainingNumVert());
	indices.resize(simplifier.RemainingNumTri() * 3);*/

	cout << "New Mesh verts: " << vertices.size()
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

	this->SetUpMesh();
	//vector<u32> packed_data;
	//PackingVirtualMesh(*this, packed_data);
	//FILE* file = fopen("packed_data.txt", "wb");
	//if (file == NULL) {
	//	perror("fopen failed");  // 打印错误信息
	//}
	//fwrite(packed_data.data(), sizeof(u32), packed_data.size(), file);
	//fclose(file);
}

void NaniteMesh::SetUpMesh()
{
	srand(time(NULL));  // 初始化随机数生成器
	int i = 0;
	if (drawMode == 0 || drawMode == 1) {
		for (auto& cluster : clusters) {
			if (drawMode == 0) {
				cluster.color = vector<vec3>(0);
				for (u32 i = 0; i < cluster.verts.size(); ++i) {
					cluster.color.push_back(vec3((f32)(rand() % 255) / 255.0f, (f32)(rand() % 255) / 255.0f, (f32)(rand() % 255) / 255.0f));
				}
			}
			else if (drawMode == 1) {
				cluster.color = vector<vec3>(cluster.verts.size(), vec3((f32)(rand() % 255) / 255.0f, (f32)(rand() % 255) / 255.0f, (f32)(rand() % 255) / 255.0f));
			}
			if (firstDraw) {
				VAO.push_back(0);
				EBO.push_back(0);
				posVBO.push_back(0);
				colorVBO.push_back(0);
				glGenVertexArrays(1, &VAO[i]);
				glGenBuffers(1, &posVBO[i]);
				glGenBuffers(1, &colorVBO[i]);
			}

			glBindVertexArray(VAO[i]);
			// load data into vertex buffers
			glBindBuffer(GL_ARRAY_BUFFER, posVBO[i]);
			// A great thing about structs is that their memory layout is sequential for all its items.
			// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
			// again translates to 3/2 floats which translates to a byte array.
			glBufferData(GL_ARRAY_BUFFER, cluster.verts.size() * sizeof(vec3), &cluster.verts[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, colorVBO[i]);
			glBufferData(GL_ARRAY_BUFFER, cluster.color.size() * sizeof(vec3), &cluster.color[0], GL_STATIC_DRAW);

			if (indices.size() != 0) {
				glGenBuffers(1, &EBO[i]);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, cluster.indexes.size() * sizeof(unsigned int), &cluster.indexes[0], GL_STATIC_DRAW);
			}
			// set the vertex attribute pointers
			// vertex Positions
			glBindBuffer(GL_ARRAY_BUFFER, posVBO[i]);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
			// vertex color
			glBindBuffer(GL_ARRAY_BUFFER, colorVBO[i]);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
			i++;
		}
	}
	if (drawMode == 2) {
		for (auto& cgroup : clusterGroups) {
			vec3 groupColor = vec3((f32)(rand() % 255) / 255.0f, (f32)(rand() % 255) / 255.0f, (f32)(rand() % 255) / 255.0f);
			for (auto clusterId : cgroup.clusters) {
				auto& cluster = clusters[clusterId];
				cluster.color = vector<vec3>(cluster.verts.size(), groupColor);

				glBindVertexArray(VAO[i]);
				// load data into vertex buffers
				glBindBuffer(GL_ARRAY_BUFFER, posVBO[i]);
				// A great thing about structs is that their memory layout is sequential for all its items.
				// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
				// again translates to 3/2 floats which translates to a byte array.
				glBufferData(GL_ARRAY_BUFFER, cluster.verts.size() * sizeof(vec3), &cluster.verts[0], GL_STATIC_DRAW);

				glBindBuffer(GL_ARRAY_BUFFER, colorVBO[i]);
				glBufferData(GL_ARRAY_BUFFER, cluster.color.size() * sizeof(vec3), &cluster.color[0], GL_STATIC_DRAW);

				if (indices.size() != 0) {
					glGenBuffers(1, &EBO[i]);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, cluster.indexes.size() * sizeof(unsigned int), &cluster.indexes[0], GL_STATIC_DRAW);
				}
				// set the vertex attribute pointers
				// vertex Positions
				glBindBuffer(GL_ARRAY_BUFFER, posVBO[i]);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
				// vertex color
				glBindBuffer(GL_ARRAY_BUFFER, colorVBO[i]);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
				i++;
			}
		}
	}
}

NaniteMesh::NaniteMesh(vector<vec3> pos, vector<unsigned int> indices, vector<Texture> textures)
{
	this->vertices = pos;
	this->indices = indices;
	this->textures = textures;
	this->firstDraw = true;
	this->Build();
}

void NaniteMesh::Draw(Shader& shader)
{
	int i = 0;
	firstDraw = false;
	for (auto cluster : clusters) {
		glBindVertexArray(VAO[i]);
		if (cluster.indexes.size() != 0) {
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(cluster.indexes.size()), GL_UNSIGNED_INT, 0);
		}
		else {
			glDrawArrays(GL_TRIANGLES, 0, static_cast<unsigned int>(cluster.verts.size()));
		}
		glBindVertexArray(0);
		i++;
	}
}

void NaniteMesh::AddMode()
{
	drawMode = (drawMode + 1) % 3;
	cout << "Now Mode is " << drawMode << endl;
	this->SetUpMesh();
}
