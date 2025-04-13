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

NaniteMesh::NaniteMesh(vector<Vertex> pos, vector<unsigned int> indices, vector<Texture> textures):ssboClusters(0), ssboIndirectCmd(0), ssboCmdCounter(0), ssboTriCount(0),
clusterCount(0)
{
	this->vertices = pos;
	this->indices = indices;
	this->textures = textures;
	this->firstFrame = true;
	this->Build();
}

void NaniteMesh::GenerateClusters(vector<PackedCluster>& clusters)
{
	// 创建 SSBO 用于存储间接绘制命令数组
	glGenBuffers(1, &ssboIndirectCmd);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIndirectCmd);

	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawElementsIndirectCommand) * clusters.size(), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboIndirectCmd);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &secondSsboIndirectCmd);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, secondSsboIndirectCmd);

	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawElementsIndirectCommand) * clusters.size(), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, secondSsboIndirectCmd);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	vertices.clear();
	indices.clear();
	gpuClusters.clear();

	for (auto c : clusters) {
		GPUCluster gc = {};
		gc.groupId = c.groupId;
		gc.lodBounds = c.lodBounds;
		gc.parentLodBounds = c.parentLodBounds;
		gc.lodError = c.lodError;
		gc.maxParentLodError = c.maxParentLodError;
		gc.mipLevel = c.mipLevel;
		gc.indexOffset = indices.size();
		gc.vertOffset = vertices.size();
		gc.indexCount = c.indexes.size();
		gc.visible = 0;
		//cout << vertices.size() << endl;
		vertices.insert(vertices.end(), c.verts.begin(), c.verts.end());
		//cout << indices.size() << endl;
		indices.insert(indices.end(), c.indexes.begin(), c.indexes.end());
		//cout << gpuClusters.size() << endl;
		gpuClusters.push_back(gc);
	}


	// 上传顶点和索引数据到 GPU
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), indices.data(), GL_DYNAMIC_DRAW);


	// 上传 gpuClusters 数据到 ssboClusters
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboClusters);
	glBufferData(GL_SHADER_STORAGE_BUFFER, gpuClusters.size() * sizeof(GPUCluster), gpuClusters.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboClusters); // 绑定到 binding=0
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


}

uvec2 NaniteMesh::Draw()
{
	renderShader->Use();
	if (mode == 1) {

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(glGetUniformLocation(renderShader->program, "ourTexture"), 0);

		glUniform1ui(glGetUniformLocation(renderShader->program, "viewMode"), viewMode);

		int transformLocation = glGetUniformLocation(renderShader->program, "transformMatrix");
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));

		int projectionLocation = glGetUniformLocation(renderShader->program, "projectionMatrix");
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		int viewLocation = glGetUniformLocation(renderShader->program, "viewMatrix");
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

		glBindVertexArray(VAO);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ssboIndirectCmd);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCmdCounter);
		unsigned int* firstCounterPtr = (unsigned int*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), GL_MAP_READ_BIT);
		firstDrawCount = *firstCounterPtr;
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, firstDrawCount, 0);
		glBindVertexArray(0);
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(glGetUniformLocation(renderShader->program, "ourTexture"), 0);

		glUniform1ui(glGetUniformLocation(renderShader->program, "viewMode"), viewMode);

		int transformLocation = glGetUniformLocation(renderShader->program, "transformMatrix");
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));

		int projectionLocation = glGetUniformLocation(renderShader->program, "projectionMatrix");
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		int viewLocation = glGetUniformLocation(renderShader->program, "viewMatrix");
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

		glBindVertexArray(VAO);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, secondSsboIndirectCmd);

		// 读取第二次补充的绘制命令数量
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, secondSsboCmdCounter);
		unsigned int* counterPtr = (unsigned int*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), GL_MAP_READ_BIT);
		secondDrawCount = *counterPtr;
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		//读取三角形数量
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboTriCount);
		counterPtr = (unsigned int*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), GL_MAP_READ_BIT);
		triCount = *counterPtr;
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, secondDrawCount, 0);
		glBindVertexArray(0);

	}
	//std::cout << "Draw -> mode: " << mode << " firstDrawCount: " << firstDrawCount << " secondDrawCount: " << secondDrawCount << std::endl;
	return uvec2{ triCount, firstDrawCount + secondDrawCount };
}

void NaniteMesh::Init()
{
	//初始化着色器
	renderShader = new Shader((GLchar*)"VertexShader.txt", (GLchar*)"FragmentShader.txt");

	computeShader = new Shader((GLchar*)"ComputeVisibility.comp");

	//初始化buffers
	//生成 VAO、VBO、EBO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	//VBO: 存储顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), nullptr, GL_DYNAMIC_DRAW);

	// EBO: 存储索引
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * indices.size(), nullptr, GL_DYNAMIC_DRAW);

	// 顶点属性指针
	// 位置
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	// 纹理坐标
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glBindVertexArray(0);

	// 创建 SSBO 用于存储 GPUCluster 数据
	glGenBuffers(1, &ssboClusters);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboClusters);

	glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	// 创建 SSBO 用于存储命令计数（原子计数器）
	glGenBuffers(1, &ssboCmdCounter);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCmdCounter);
	unsigned int zero = 0;
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), &zero, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboCmdCounter);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &secondSsboCmdCounter);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, secondSsboCmdCounter);
	unsigned int two = 0;
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), &two, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, secondSsboCmdCounter);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	//创建SSBO 存储三角形数量
	glGenBuffers(1, &ssboTriCount);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboTriCount);
	unsigned int one = 0;
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), &one, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssboTriCount);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void NaniteMesh::ComputePass(GLuint depthTex, u32 screenWidth, u32 screenHeight, int mode)
{
	if (firstFrame) {
		prevView = viewMatrix;
		prevProj = projectionMatrix;
	}
	else {
		prevView = currentView;
		prevProj = currentProj;
	}
	currentProj = projectionMatrix;
	currentView = viewMatrix;

	computeShader->Use();
	this->mode = mode;
	//摄像机视锥
	glm::vec4 planes[6];
	glm::mat4 viewProjMatrix = projectionMatrix * viewMatrix;
	for (int i = 0; i < 6; i++) {
		int sign = (i % 2 == 0) ? 1 : -1;
		planes[i] = glm::vec4(
			viewProjMatrix[0][3] + sign * viewProjMatrix[0][i / 2],
			viewProjMatrix[1][3] + sign * viewProjMatrix[1][i / 2],
			viewProjMatrix[2][3] + sign * viewProjMatrix[2][i / 2],
			viewProjMatrix[3][3] + sign * viewProjMatrix[3][i / 2]
		);
		planes[i] /= glm::length(glm::vec3(planes[i]));
	}

	glm::vec2 screenSize = { screenWidth, screenHeight };
	glUniform1i(glGetUniformLocation(computeShader->program, "isFirstFrame"), firstFrame ? 1 : 0);
	glUniform1i(glGetUniformLocation(computeShader->program, "occludeMode"), mode);
	glUniformMatrix4fv(glGetUniformLocation(computeShader->program, "uPrevViewMat"), 1, GL_FALSE, glm::value_ptr(prevView));
	glUniformMatrix4fv(glGetUniformLocation(computeShader->program, "uPrevProjMat"), 1, GL_FALSE, glm::value_ptr(prevProj));
	glUniformMatrix4fv(glGetUniformLocation(computeShader->program, "uViewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(glGetUniformLocation(computeShader->program, "uProjMat"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniform2fv(glGetUniformLocation(computeShader->program, "uScreenSize"), 1, glm::value_ptr(screenSize));
	glUniform4fv(glGetUniformLocation(computeShader->program, "frustumPlanes"), 6, glm::value_ptr(planes[0]));

	if (mode == 1) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTex);
		triCount = 0;
		clusterCount = 0;

		glUniform1i(glGetUniformLocation(computeShader->program, "prevDepthTexture"), 0);

		// 重置命令计数器为 0
		zero = 0;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCmdCounter);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &zero);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		// 重置三角形计数
		one = 0;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboTriCount);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &one);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	else if (mode == 2) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthTex);
		glUniform1i(glGetUniformLocation(computeShader->program, "curDepthTexture"), 1);

		// 重置命令计数器为 0
		two = 0;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, secondSsboCmdCounter);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &two);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	glDispatchCompute(((gpuClusters.size() + 255) / 256), 1, 1);

	glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}

void NaniteMesh::DrawDepth(Shader* depthShader)
{
	if (mode == 1) {
		glUniform1ui(glGetUniformLocation(depthShader->program, "viewMode"), viewMode);

		int transformLocation = glGetUniformLocation(depthShader->program, "transformMatrix");
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));

		int projectionLocation = glGetUniformLocation(depthShader->program, "projectionMatrix");
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		int viewLocation = glGetUniformLocation(depthShader->program, "viewMatrix");
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

		glBindVertexArray(VAO);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ssboIndirectCmd);
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, firstDrawCount, 0);
	}
	else {
		glUniform1ui(glGetUniformLocation(depthShader->program, "viewMode"), viewMode);

		int transformLocation = glGetUniformLocation(depthShader->program, "transformMatrix");
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));

		int projectionLocation = glGetUniformLocation(depthShader->program, "projectionMatrix");
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		int viewLocation = glGetUniformLocation(depthShader->program, "viewMatrix");
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

		glBindVertexArray(VAO);
		//绘制最终的深度纹理
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ssboIndirectCmd);
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, firstDrawCount, 0);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, secondSsboIndirectCmd);
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, secondDrawCount, 0);

		firstFrame = false;
	}
}

void NaniteMesh::SetInfo(glm::mat4 _transformMatrix, glm::mat4 _model, glm::mat4 _viewMatrix, glm::mat4 _projectionMatrix)
{
	transformMatrix = _transformMatrix;
	model = _model;
	viewMatrix = _viewMatrix;
	projectionMatrix = _projectionMatrix;

	if (firstFrame) {
		prevView = viewMatrix;
		prevProj = projectionMatrix;
	}

	currentView = viewMatrix;
	currentProj = projectionMatrix;
}


