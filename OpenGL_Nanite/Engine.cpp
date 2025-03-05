#include "Engine.h"
#include<iostream>
#include<thread>
#include "Model.h"

vector<bool>keys(1024);
GLfloat lastX = 960;
GLfloat lastY = 520;
bool firstMouse = true;
Model* myModel;
u32 viewMode = 0;

void KeyCallBack(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
		cout << "按下了关闭键 esc = " << key << endl;
	}
	if (key == GLFW_KEY_J && action == GLFW_PRESS) {
		viewMode = (viewMode + 1) % 4;
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

	/* 开启深度测试 */
	glEnable(GL_DEPTH_TEST);

	//Shader myShader = Shader((GLchar*)"VertexShader.txt", (GLchar*)"FragmentShader.txt");	// 相对路径
	//int width_1, height_1;
	//unsigned char* image1 = SOIL_load_image("Texture1.png", &width_1, &height_1, 0, SOIL_LOAD_RGBA);
	//int width_2, height_2;
	//unsigned char* image2 = SOIL_load_image("Texture2.png", &width_2, &height_2, 0, SOIL_LOAD_RGBA);
	//
	Renderer render;
	/*vector<Cluster>clusters;
	vector<ClusterGroup>clusterGroups;
	int index = 0;
	for (auto mesh : myModel->GetMeshes()) {
		for (auto c : ((NaniteMesh*)mesh)->clusters) {
			c.groupId += index;
			clusters.push_back(c);
		}
		index += clusterGroups.size();
		clusterGroups.insert(clusterGroups.end(), ((NaniteMesh*)myModel->GetMeshes()[0])->clusterGroups.begin(), ((NaniteMesh*)myModel->GetMeshes()[0])->clusterGroups.end());
	}*/
	string dataName = "";
	int i = 0;
	while (this->modelPath[i] != '.') {
		dataName += this->modelPath[i];
		i++;
	}
	dataName += ".txt";
	FILE* readFile = fopen(dataName.c_str(), "rb");
	if (!readFile) {
		myModel = new Model(modelPath, true);
		PackData(dataName);
	}
	ReadPackData(dataName);
	render.SetScreenSize(this->window.GetWindow(), screenWidth, screenHeight);
	render.SetCamera(camera);
	render.SetClusterCount(readClusters.size());
	render.Init();
	render.GenerateClusters(readClusters);
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

		/* 渲染 + 清除颜色缓冲 */
		/*glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/

		render.SetViewMode(viewMode);
		render.Render();
		/*  绘制图形 */
		//myShader.Use();					// 图形渲染
		//
		//glm::mat4 transformMatrix = glm::mat4(1.0f);
		//glm::mat4 model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		//glm::mat4 viewMatrix = camera->GetViewMatrix();	// 求得观察矩阵

		//glm::mat4 projectionMatrix = glm::perspective(camera->GetZoom(), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

		//int transformLocation = glGetUniformLocation(myShader.program, "transformMatrix");
		//glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));

		//int projectionLocation = glGetUniformLocation(myShader.program, "projectionMatrix");
		//glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		//int viewLocation = glGetUniformLocation(myShader.program, "viewMatrix");
		//glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

		//myModel->Draw(myShader);

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
	FILE* file = fopen(name.c_str(), "wb");
	if (!file) {
		perror("文件打开失败");
		return;
	}
	
	int index = 0;
	for (auto mesh : myModel->GetMeshes()) {
		for (auto c : ((NaniteMesh*)mesh)->clusters) {
			c.groupId += index;
			clusters.push_back(c);
		}
		index += clusterGroups.size();
		clusterGroups.insert(clusterGroups.end(), ((NaniteMesh*)myModel->GetMeshes()[0])->clusterGroups.begin(), ((NaniteMesh*)myModel->GetMeshes()[0])->clusterGroups.end());
	}

	size_t count = clusters.size();
	fwrite(&count, sizeof(size_t), 1, file);
	
	for (auto c : clusters) {
		vec4 lodBounds = vec4(c.lodBounds.center, c.lodBounds.radius);
		vec4 parentLodBounds = vec4(clusterGroups[c.groupId].lodBounds.center, clusterGroups[c.groupId].lodBounds.radius);
		fwrite(&lodBounds, sizeof(lodBounds), 1, file);
		fwrite(&parentLodBounds, sizeof(parentLodBounds), 1, file);
		fwrite(&c.lodError, sizeof(f32), 1, file);
		fwrite(&clusterGroups[c.groupId].maxParentLodError, sizeof(f32), 1, file);
		fwrite(&c.mipLevel, sizeof(u32), 1, file);
		fwrite(&c.groupId, sizeof(u32), 1, file);

		//写入顶点
		size_t vSize = c.verts.size();
		fwrite(&vSize, sizeof(size_t), 1, file);
		fwrite(c.verts.data(), sizeof(vec3), vSize, file);

		//写入索引
		size_t iSize = c.indexes.size();
		fwrite(&iSize, sizeof(size_t), 1, file);
		fwrite(c.indexes.data(), sizeof(u32), iSize, file);
	}

	fclose(file);
}
void Engine::LoadModelPath(string path)
{
	this->modelPath = "Model/" + path;
}

void Engine::ReadPackData(string name)
{
	FILE* file = fopen(name.c_str(), "rb");
	if (!file) {
		perror("文件打开失败");
		return;
	}

	size_t numCluster;
	fread(&numCluster, sizeof(size_t), 1, file);

	readClusters = vector<PackedCluster>(numCluster);

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
		fread(c.verts.data(), sizeof(vec3), vSize, file);

		size_t iSize;
		fread(&iSize, sizeof(size_t), 1, file);
		c.indexes.resize(iSize);
		fread(c.indexes.data(), sizeof(u32), iSize, file);
	}

	fclose(file);
}


Camera *Engine::GetCamera()
{
	return camera;
}
