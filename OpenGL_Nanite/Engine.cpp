#include "Engine.h"
#include<iostream>
#include<thread>
#include<direct.h>

vector<bool>keys(1024);
GLfloat lastX = 960;
GLfloat lastY = 520;
bool firstMouse = true;
Model* myModel;
u32 viewMode = 0;
u32 clusterCount = 0;
u32 triCount = 0;
void KeyCallBack(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
		cout << "按下了关闭键 esc = " << key << endl;
	}
	if (key == GLFW_KEY_J && action == GLFW_PRESS) {
		viewMode = (viewMode + 1) % 5;
		//myModel->AddMode();
		cout << "切换显示模式" << endl;
	}
	if (key >= 0 && key <= 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;	// true 代表按下了键
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

void MouseCallBack(GLFWwindow* window, double xPos, double yPos) {

	//if()

	if (firstMouse) {
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
		return;
	}

	float xOffset = xPos - lastX;
	float yOffset = lastY - yPos;

	lastX = xPos;
	lastY = yPos;

	//printf("%d\n", Engine::GetInstance()->GetCamera());
	Engine::GetInstance()->GetCamera()->ProcessMouseMovement(xOffset, yOffset);
}

void ScrollCallBack(GLFWwindow* window, double xOffset, double yOffset) {
	//printf("%d\n", Engine::GetInstance()->GetCamera());
	Engine::GetInstance()->GetCamera()->ProcessMouseScroll(yOffset);
}

shared_ptr<Engine> Engine::GetInstance()
{
	//双重锁模式
	if (instance == nullptr)
	{//先判断是否为空，如果为空则进入，不为空说明已经存在实例，直接返回
		//进入后加锁
		i_mutex.lock();//
		if (instance == nullptr)
		{//再判断一次，确保不会因为加锁期间多个线程同时进入
			instance = shared_ptr<Engine>(new Engine());
		}
		i_mutex.unlock();//解锁
	}
	return instance;
}

void Engine::InitEngine()
{
	this->deltaTime = 0.0f;
	this->lastTime = 0.0f;
	camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));
}


void Engine::Draw()
{
	this->window.InitWindow();
	int screenWidth, screenHeight;
	glfwGetFramebufferSize(this->window.GetWindow(), &screenWidth, &screenHeight);
	glfwMakeContextCurrent(this->window.GetWindow());
	glfwSetInputMode(this->window.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetKeyCallback(this->window.GetWindow(), KeyCallBack);
	glfwSetCursorPosCallback(this->window.GetWindow(), MouseCallBack);
	glfwSetScrollCallback(this->window.GetWindow(), ScrollCallBack);

	glewInit();

	Renderer render;
	
	render.SetRenderMode(renderMode);
	if (renderMode == 1) {
		string texName = "";
		int i = 0;
		while (this->modelPath[i] != '.') {
			texName += this->modelPath[i];
			i++;
		}
		vector<Vertex>verts;
		vector<u32>indices;
		myModel = new Model("Model/" + modelPath, false);
		texName += ".png";
		for (auto& mesh : myModel->GetMeshes())
		{
			mesh->LoadTex("Model/" + texName);
			render.AddMesh(mesh);
		}
		render.SetScreenSize(this->window.GetWindow(), screenWidth, screenHeight);
		render.SetCamera(camera);
		render.Init();
	}
	if (renderMode == 2) {
		string dataName = "";
		string texName;
		int i = 0;
		while (this->modelPath[i] != '.') {
			dataName += this->modelPath[i];
			i++;
		}
		texName = dataName + ".png";
		FILE* readFile = fopen(("Model/" + dataName + "/" + dataName + "_0.txt").c_str(), "rb");
		if (!readFile) {
			myModel = new Model("Model/" + modelPath, true);
			PackData(dataName);
		}
		ReadPackData(dataName);
		
		for (auto& mesh : meshes) {
			mesh->LoadTex("Model/" + texName);
			render.AddMesh(mesh);
		}

		render.SetScreenSize(this->window.GetWindow(), screenWidth, screenHeight);
		render.SetCamera(camera);
		render.SetClusterCount(clusterCount);
		render.SetTriCount(triCount);
		render.Init();
	}
	
	// draw loop 画图循环
	while (!glfwWindowShouldClose(this->window.GetWindow()))
	{
		/* 视口 + 时间 */
		GLfloat currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;
		glViewport(0, 0, screenWidth, screenHeight);
		glfwPollEvents();
		this->KeyController();

		if (renderMode == 1) {
			render.CommonRender();
		}
		if (renderMode == 2) {
			render.SetViewMode(viewMode);
			render.Render();
		}

		glfwSwapBuffers(this->window.GetWindow());
	}
	glfwTerminate();
}

void Engine::SetWindowSize(int winHeight, int winWidth)
{
	window.SetWindowSize(winHeight, winWidth);
}

void Engine::KeyController()
{

	//printf("%d\n", camera);
	if (keys[GLFW_KEY_W])		// 向前
		camera->ProcessKeyboard(FORWARD, deltaTime);

	if (keys[GLFW_KEY_S])		// 向后
		camera->ProcessKeyboard(BACKWARD, deltaTime);

	if (keys[GLFW_KEY_A])		// 向左
		camera->ProcessKeyboard(LEFT, deltaTime);

	if (keys[GLFW_KEY_D])		// 向右
		camera->ProcessKeyboard(RIGHT, deltaTime);

	if (keys[GLFW_KEY_SPACE])		// 向上
		camera->ProcessKeyboard(UPWARD, deltaTime);

	if (keys[GLFW_KEY_LEFT_SHIFT])		// 向下
		camera->ProcessKeyboard(DOWNWARD, deltaTime);
}

void Engine::PackData(string name) {
	
	int index = 0;

	_mkdir(("Model/" + name).c_str());
	for (auto mesh : myModel->GetMeshes()) {
		NaniteMesh* naniteMesh = static_cast<NaniteMesh*>(mesh);
		string fileName = "Model/" + name + "/" + name + "_" + to_string(index) + ".txt";
		FILE* file = fopen(fileName.c_str(), "wb");
		if (!file) {
			perror("文件打开失败");
			return;
		}
		size_t count = naniteMesh->clusters.size();
		fwrite(&count, sizeof(size_t), 1, file);

		for (auto c : naniteMesh->clusters) {
			vec4 lodBounds = vec4(c.lodBounds.center, c.lodBounds.radius);
			vec4 parentLodBounds = vec4(naniteMesh->clusterGroups[c.groupId].lodBounds.center, naniteMesh->clusterGroups[c.groupId].lodBounds.radius);
			fwrite(&lodBounds, sizeof(lodBounds), 1, file);
			fwrite(&parentLodBounds, sizeof(parentLodBounds), 1, file);
			fwrite(&c.lodError, sizeof(f32), 1, file);
			fwrite(&naniteMesh->clusterGroups[c.groupId].maxParentLodError, sizeof(f32), 1, file);
			fwrite(&c.mipLevel, sizeof(u32), 1, file);
			fwrite(&c.groupId, sizeof(u32), 1, file);

			//写入顶点
			size_t vSize = c.verts.size();
			fwrite(&vSize, sizeof(size_t), 1, file);
			fwrite(c.verts.data(), sizeof(Vertex), vSize, file);

			//写入索引
			size_t iSize = c.indexes.size();
			fwrite(&iSize, sizeof(size_t), 1, file);
			fwrite(c.indexes.data(), sizeof(u32), iSize, file);
		}

		fclose(file);
		index++;
	}
}
void Engine::SetRenderMode(int mode)
{
	renderMode = mode;
}
void Engine::LoadModelPath(string path)
{
	this->modelPath = path;
}

void Engine::ReadPackData(string name)
{
	int index = 0;
	string fileName = "Model/" + name + "/" + name + "_" + to_string(index) + ".txt";
	FILE* file = fopen(fileName.c_str(), "rb");
	meshes.clear();
	while (file) {
		NaniteMesh* mesh = new NaniteMesh();

		size_t numCluster;
		fread(&numCluster, sizeof(size_t), 1, file);
		readClusters.clear();
		readClusters = vector<PackedCluster>(numCluster);
		clusterCount += numCluster;

		for (auto& c : readClusters) {
			fread(&c.lodBounds, sizeof(vec4), 1, file);
			fread(&c.parentLodBounds, sizeof(vec4), 1, file);
			fread(&c.lodError, sizeof(f32), 1, file);
			fread(&c.maxParentLodError, sizeof(f32), 1, file);
			fread(&c.mipLevel, sizeof(u32), 1, file);
			fread(&c.groupId, sizeof(u32), 1, file);

			size_t vSize;
			fread(&vSize, sizeof(size_t), 1, file);
			c.verts.resize(vSize);
			fread(c.verts.data(), sizeof(Vertex), vSize, file);

			size_t iSize;
			fread(&iSize, sizeof(size_t), 1, file);
			c.indexes.resize(iSize);
			fread(c.indexes.data(), sizeof(u32), iSize, file);
			triCount += iSize / 3;
		}

		fclose(file);

		mesh->GenerateClusters(readClusters);
		meshes.push_back(mesh);

		index++;
		fileName = "Model/" + name + "/" + name + "_" + to_string(index) + ".txt";
		file = fopen(fileName.c_str(), "rb");
	}

}


Camera *Engine::GetCamera()
{
	return camera;
}
