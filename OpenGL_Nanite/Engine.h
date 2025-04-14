#pragma once
#include<iostream>
#include "openGLwin.h"
#include "Shaders.h"
#include "Camera.h"
#include "Render.h"
#include "Model.h"
#include "co/fs.h"
#include<mutex>
using namespace std;

class Engine {
private:
	OpenGLWin::Window window;
	Engine() {};
	Engine(const Engine& copy) = delete;
	const Engine& operator=(const Engine& copy) = delete;
	vector<NaniteMesh*>meshes;
	string modelPath;
	static shared_ptr<Engine> instance;
	static mutex i_mutex;//Ëø
	int renderMode;
	GLfloat deltaTime;
	GLfloat lastTime;
	Renderer render;
	Camera* camera;
	vector<Cluster>clusters;
	vector<ClusterGroup>clusterGroups;
	vector<PackedCluster>readClusters;

public:
	static shared_ptr<Engine> GetInstance();
	void InitEngine();
	void Draw();
	~Engine() {};
	void SetWindowSize(int winHeight, int winWidth);
	void KeyController();
	void LoadModelPath(string path);
	void ReadPackData(string name);
	void PackData(string name);
	void SetRenderMode(int mode);
	//static void MouseCallBack(GLFWwindow* window, double xPos, double yPos);
	Camera *GetCamera();
};

