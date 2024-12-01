#include "Engine.h"
#include<iostream>
#include<thread>

vector<bool>keys(1024);
GLfloat lastX = 960;
GLfloat lastY = 520;
bool firstMouse = true;

void KeyCallBack(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
		cout << "按下了关闭键 esc = " << key << endl;
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
	camera = new Camera(glm::vec3(1.0f, 1.0f, -5.0f), glm::vec3(-1.0f, -1.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}


void Engine::Draw(GLfloat vertex[], int vertexSize, GLuint indices[], int indiceSize)
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

	Shader myShader = Shader((GLchar*)"VertexShader.txt", (GLchar*)"FragmentShader.txt");	// 相对路径
	int width_1, height_1;
	unsigned char* image1 = SOIL_load_image("Texture1.png", &width_1, &height_1, 0, SOIL_LOAD_RGBA);
	int width_2, height_2;
	unsigned char* image2 = SOIL_load_image("Texture2.png", &width_2, &height_2, 0, SOIL_LOAD_RGBA);
	GLuint VAO, VBO;
	/* 设置顶点缓冲对象(VBO) + 设置顶点数组对象(VAO)  */
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexSize, vertex, GL_STATIC_DRAW);
	
	GLuint EBO;
	if (indiceSize != 0) {
		glGenBuffers(1, &EBO);						// 绑定 EBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);	// 使用 glBindBuffer 函数把新创建的索引缓冲对象绑定到 GL_ELEMENT_ARRAY_BUFFER目标上
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indiceSize, indices, GL_STATIC_DRAW); // GL_STATIC_DRAW：静态的画图(因为要频繁地读)
	}

	/* 设置链接顶点属性 */
	//带颜色，纹理。
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	//glEnableVertexAttribArray(0);	// 通道 0 打开
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	//glEnableVertexAttribArray(1);	// 通道 1 打开
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	//glEnableVertexAttribArray(2);	// 通道 2 打开

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);	// 通道 0 打开
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);	// 通道 1 打开

	///* 生成纹理 */
	//GLuint texture_1;
	//glGenTextures(1, &texture_1);
	//glBindTexture(GL_TEXTURE_2D, texture_1);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_1, height_1, 0, GL_RGBA, GL_UNSIGNED_BYTE, image1);			// 读取图片信息
	//glGenerateMipmap(GL_TEXTURE_2D);	// 多层渐进纹理
	//SOIL_free_image_data(image1);

	//GLuint texture_2;
	//glGenTextures(1, &texture_2);
	//glBindTexture(GL_TEXTURE_2D, texture_2);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_2, height_2, 0, GL_RGBA, GL_UNSIGNED_BYTE, image2);			// 读取图片信息
	//glGenerateMipmap(GL_TEXTURE_2D);	// 多层渐进纹理
	//SOIL_free_image_data(image2);

	///* 纹理环绕方式 */
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// S 坐标
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);	// T 坐标

	///* 纹理过滤 */
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	///* 纹理单元：下一节重点使用 */
	//myShader.Use();	// don't forget to activate/use the shader before setting uniforms!
	//glUniform1i(glGetUniformLocation(myShader.program, "ourTexture_1"), 0);
	//glUniform1i(glGetUniformLocation(myShader.program, "ourTexture_2"), 1);

	//int cnt = 0;				// 计数器
	//int flag = 1;				// 纹理切换标志
	//int N = 200;				// N 越大，位移/交替的速度都会变慢

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
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* 生成纹理 */
		//glActiveTexture(GL_TEXTURE0);			// 激活 0 号纹理单元。最多 16 个通道
		//glBindTexture(GL_TEXTURE_2D, texture_1);	// 绑定这个纹理到当前激活的纹理目标
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, texture_2);

		/* 每一次循环引起的反应 */
		//cnt = cnt + 1;		// 计数器加1
		//if (cnt >= 2 * N)
		//	cnt = 0;		// 设置重复循环
		//if (cnt == 0 || cnt == N)
		//	flag = -flag;	// 设置纹理循环交替标志

		///* 平移变换的实现 */
		//glm::mat4 firsrMatrix_1 = glm::mat4(1.0f); // 初始化为单位矩阵
		//glm::mat4 transform_1 = glm::translate(firsrMatrix_1, glm::vec3(0.0f, 1.0f - cnt * 1.0 / N, 0.0f));

		///* 交替变换的实现 */
		//int vertexColorLocation = glGetUniformLocation(myShader.program, "time");	// 找到 “time” 的索引
		//if (flag == 1)
		//	glUniform1f(vertexColorLocation, cnt * 1.0 / N);			// 从纹理1 逐渐交替到 纹理2
		//else
		//	glUniform1f(vertexColorLocation, 2.0 - cnt * 1.0 / N);		// 从纹理2 逐渐交替到 纹理1

		//// get matrix's uniform location and set matrix
		//unsigned int transformLoc = glGetUniformLocation(myShader.program, "transform_1");
		//glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform_1));

		/*  绘制图形 */
		myShader.Use();					// 图形渲染
		glBindVertexArray(VAO);									// 绑定 VAO
		if (indiceSize != 0) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);				// 绑定 EBO
		}
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);	// 画两个三角形 从第0个顶点开始 一共画6次
		for(int i = 0; i < 2; i++)
		{
			glm::mat4 transformMatrix = glm::mat4(1.0f);
			glm::mat4 viewMatrix = camera->GetViewMatrix();	// 求得观察矩阵
			if (i == 0)		// 大立方体
			{
				transformMatrix = glm::translate(transformMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
				float newSize = cos(currentTime) * 0.2f + 0.8f;
				transformMatrix = glm::scale(transformMatrix, glm::vec3(newSize, newSize, newSize));
			}
			else				// 小立方体
			{
				transformMatrix = glm::translate(transformMatrix, glm::vec3(0.0f, 1.0f, 0.0f));
				transformMatrix = glm::rotate(transformMatrix, currentTime, glm::vec3(0.2f, 1.0f, 0.0f));
				transformMatrix = glm::scale(transformMatrix, glm::vec3(0.15f, 0.15f, 0.15f));
			}

			glm::mat4 projectionMatrix = glm::perspective(camera->GetZoom(), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

			int transformLocation = glGetUniformLocation(myShader.program, "transformMatrix");
			glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transformMatrix));

			int projectionLocation = glGetUniformLocation(myShader.program, "projectionMatrix");
			glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

			int viewLocation = glGetUniformLocation(myShader.program, "viewMatrix");
			glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

			// 第一个参数在 vs.txt 中，第二个在主函数中
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindVertexArray(0);									// 解绑定 VAO
		if (indiceSize != 0) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);				// 解绑定 EBO
		}
		//glBindTexture(GL_TEXTURE_2D, 0);						// 解绑定 纹理


		glfwSwapBuffers(this->window.GetWindow());
	}


	glDeleteVertexArrays(1, &VAO);			// 释放资源	
	glDeleteBuffers(1, &VBO);
	glfwTerminate();
}

void Engine::SetWindowSize(int winHeight, int winWidth)
{
	window.SetWindowSize(winHeight, winWidth);
}


void Engine::KeyController()
{

	//printf("%d\n", camera);
	if (keys[GLFW_KEY_Q])		// 向前
		camera->ProcessKeyboard(FORWARD, deltaTime);

	if (keys[GLFW_KEY_E])		// 向后
		camera->ProcessKeyboard(BACKWARD, deltaTime);

	if (keys[GLFW_KEY_A])		// 向左
		camera->ProcessKeyboard(LEFT, deltaTime);

	if (keys[GLFW_KEY_D])		// 向右
		camera->ProcessKeyboard(RIGHT, deltaTime);

	if (keys[GLFW_KEY_W])		// 向上
		camera->ProcessKeyboard(UPWARD, deltaTime);

	if (keys[GLFW_KEY_S])		// 向下
		camera->ProcessKeyboard(DOWNWARD, deltaTime);
}

Camera *Engine::GetCamera()
{
	return camera;
}
