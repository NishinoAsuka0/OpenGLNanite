#pragma once
#include<iostream>
#include"openGLwin.h"
#include"Shaders.h"
#include"Camera.h"
#include<mutex>
using namespace std;


class Engine {
private:
	OpenGLWin::Window window;
	Engine() {};
	Engine(const Engine& copy) = delete;
	const Engine& operator=(const Engine& copy) = delete;
	static shared_ptr<Engine> instance;
	static mutex i_mutex;//Ëø
	GLfloat deltaTime;
	GLfloat lastTime;
	Camera* camera;
public:
	static shared_ptr<Engine> GetInstance();
	void InitEngine();
	void Draw(GLfloat vertex[], int vertexSize, GLuint indices[] = nullptr, int indiceSize = 0);
	~Engine() {};
	void SetWindowSize(int winHeight, int winWidth);
	void KeyController();
	//static void MouseCallBack(GLFWwindow* window, double xPos, double yPos);
	Camera *GetCamera();
};

