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
		cout << "�����˹رռ� esc = " << key << endl;
	}
	if (key >= 0 && key <= 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;	// true ���������˼�
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
	//˫����ģʽ
	if (instance == nullptr)
	{//���ж��Ƿ�Ϊ�գ����Ϊ������룬��Ϊ��˵���Ѿ�����ʵ����ֱ�ӷ���
		//��������
		i_mutex.lock();//
		if (instance == nullptr)
		{//���ж�һ�Σ�ȷ��������Ϊ�����ڼ����߳�ͬʱ����
			instance = shared_ptr<Engine>(new Engine());
		}
		i_mutex.unlock();//����
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

	/* ������Ȳ��� */
	glEnable(GL_DEPTH_TEST);

	Shader myShader = Shader((GLchar*)"VertexShader.txt", (GLchar*)"FragmentShader.txt");	// ���·��
	int width_1, height_1;
	unsigned char* image1 = SOIL_load_image("Texture1.png", &width_1, &height_1, 0, SOIL_LOAD_RGBA);
	int width_2, height_2;
	unsigned char* image2 = SOIL_load_image("Texture2.png", &width_2, &height_2, 0, SOIL_LOAD_RGBA);
	GLuint VAO, VBO;
	/* ���ö��㻺�����(VBO) + ���ö����������(VAO)  */
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexSize, vertex, GL_STATIC_DRAW);
	
	GLuint EBO;
	if (indiceSize != 0) {
		glGenBuffers(1, &EBO);						// �� EBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);	// ʹ�� glBindBuffer �������´����������������󶨵� GL_ELEMENT_ARRAY_BUFFERĿ����
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indiceSize, indices, GL_STATIC_DRAW); // GL_STATIC_DRAW����̬�Ļ�ͼ(��ΪҪƵ���ض�)
	}

	/* �������Ӷ������� */
	//����ɫ��������
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	//glEnableVertexAttribArray(0);	// ͨ�� 0 ��
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	//glEnableVertexAttribArray(1);	// ͨ�� 1 ��
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	//glEnableVertexAttribArray(2);	// ͨ�� 2 ��

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);	// ͨ�� 0 ��
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);	// ͨ�� 1 ��

	///* �������� */
	//GLuint texture_1;
	//glGenTextures(1, &texture_1);
	//glBindTexture(GL_TEXTURE_2D, texture_1);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_1, height_1, 0, GL_RGBA, GL_UNSIGNED_BYTE, image1);			// ��ȡͼƬ��Ϣ
	//glGenerateMipmap(GL_TEXTURE_2D);	// ��㽥������
	//SOIL_free_image_data(image1);

	//GLuint texture_2;
	//glGenTextures(1, &texture_2);
	//glBindTexture(GL_TEXTURE_2D, texture_2);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_2, height_2, 0, GL_RGBA, GL_UNSIGNED_BYTE, image2);			// ��ȡͼƬ��Ϣ
	//glGenerateMipmap(GL_TEXTURE_2D);	// ��㽥������
	//SOIL_free_image_data(image2);

	///* �������Ʒ�ʽ */
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// S ����
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);	// T ����

	///* �������� */
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	///* ������Ԫ����һ���ص�ʹ�� */
	//myShader.Use();	// don't forget to activate/use the shader before setting uniforms!
	//glUniform1i(glGetUniformLocation(myShader.program, "ourTexture_1"), 0);
	//glUniform1i(glGetUniformLocation(myShader.program, "ourTexture_2"), 1);

	//int cnt = 0;				// ������
	//int flag = 1;				// �����л���־
	//int N = 200;				// N Խ��λ��/������ٶȶ������

	// draw loop ��ͼѭ��
	while (!glfwWindowShouldClose(this->window.GetWindow()))
	{
		/* �ӿ� + ʱ�� */
		GLfloat currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;
		glViewport(0, 0, screenWidth, screenHeight);
		glfwPollEvents();
		this->KeyController();

		/* ��Ⱦ + �����ɫ���� */
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* �������� */
		//glActiveTexture(GL_TEXTURE0);			// ���� 0 ��������Ԫ����� 16 ��ͨ��
		//glBindTexture(GL_TEXTURE_2D, texture_1);	// �������������ǰ���������Ŀ��
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, texture_2);

		/* ÿһ��ѭ������ķ�Ӧ */
		//cnt = cnt + 1;		// ��������1
		//if (cnt >= 2 * N)
		//	cnt = 0;		// �����ظ�ѭ��
		//if (cnt == 0 || cnt == N)
		//	flag = -flag;	// ��������ѭ�������־

		///* ƽ�Ʊ任��ʵ�� */
		//glm::mat4 firsrMatrix_1 = glm::mat4(1.0f); // ��ʼ��Ϊ��λ����
		//glm::mat4 transform_1 = glm::translate(firsrMatrix_1, glm::vec3(0.0f, 1.0f - cnt * 1.0 / N, 0.0f));

		///* ����任��ʵ�� */
		//int vertexColorLocation = glGetUniformLocation(myShader.program, "time");	// �ҵ� ��time�� ������
		//if (flag == 1)
		//	glUniform1f(vertexColorLocation, cnt * 1.0 / N);			// ������1 �𽥽��浽 ����2
		//else
		//	glUniform1f(vertexColorLocation, 2.0 - cnt * 1.0 / N);		// ������2 �𽥽��浽 ����1

		//// get matrix's uniform location and set matrix
		//unsigned int transformLoc = glGetUniformLocation(myShader.program, "transform_1");
		//glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform_1));

		/*  ����ͼ�� */
		myShader.Use();					// ͼ����Ⱦ
		glBindVertexArray(VAO);									// �� VAO
		if (indiceSize != 0) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);				// �� EBO
		}
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);	// ������������ �ӵ�0�����㿪ʼ һ����6��
		for(int i = 0; i < 2; i++)
		{
			glm::mat4 transformMatrix = glm::mat4(1.0f);
			glm::mat4 viewMatrix = camera->GetViewMatrix();	// ��ù۲����
			if (i == 0)		// ��������
			{
				transformMatrix = glm::translate(transformMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
				float newSize = cos(currentTime) * 0.2f + 0.8f;
				transformMatrix = glm::scale(transformMatrix, glm::vec3(newSize, newSize, newSize));
			}
			else				// С������
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

			// ��һ�������� vs.txt �У��ڶ�������������
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindVertexArray(0);									// ��� VAO
		if (indiceSize != 0) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);				// ��� EBO
		}
		//glBindTexture(GL_TEXTURE_2D, 0);						// ��� ����


		glfwSwapBuffers(this->window.GetWindow());
	}


	glDeleteVertexArrays(1, &VAO);			// �ͷ���Դ	
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
	if (keys[GLFW_KEY_Q])		// ��ǰ
		camera->ProcessKeyboard(FORWARD, deltaTime);

	if (keys[GLFW_KEY_E])		// ���
		camera->ProcessKeyboard(BACKWARD, deltaTime);

	if (keys[GLFW_KEY_A])		// ����
		camera->ProcessKeyboard(LEFT, deltaTime);

	if (keys[GLFW_KEY_D])		// ����
		camera->ProcessKeyboard(RIGHT, deltaTime);

	if (keys[GLFW_KEY_W])		// ����
		camera->ProcessKeyboard(UPWARD, deltaTime);

	if (keys[GLFW_KEY_S])		// ����
		camera->ProcessKeyboard(DOWNWARD, deltaTime);
}

Camera *Engine::GetCamera()
{
	return camera;
}