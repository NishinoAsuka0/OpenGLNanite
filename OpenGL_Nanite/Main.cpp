#include<iostream>
#include"Engine.h"
#include"Model.h"
using namespace std;


shared_ptr<Engine> Engine::instance = nullptr;
mutex Engine::i_mutex;//类外初始化

const GLint WIDTH = 1920, HEIGHT = 1080;		// 窗口的长和宽


int main(int argc, char** argv[])
{

	auto engine = Engine::GetInstance();
    engine->InitEngine();
	engine->SetWindowSize(WIDTH, HEIGHT);

	//加载模型
	//Model model(path);
	//engine->LoadMesh(mesh);
	cout << "========================================" << endl;
	cout << "请将模型文件夹放在目录后，输入模型文件名" << endl;
	cout << "========================================" << endl;
	cout << "请输入模型文件名称：";
	string path;
	cin >> path;
	cout << endl;
	engine->LoadModelPath(path);
	engine->Draw();

	return 0;
}