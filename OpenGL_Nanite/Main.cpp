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

	cout << "========================================" << endl;
	cout << "1为普通渲染模式 2为Cluster渲染模式" << endl;
	cout << "========================================" << endl;
	cout << "请输入渲染模式：";

	unsigned int mode;
	cin >> mode;
	cout << endl;

	if (mode > 2 || mode < 1) {
		mode = 2; //如果不符合输入 默认为Cluster渲染
	}
	engine->SetRenderMode(mode);
	engine->LoadModelPath(path);
	engine->Draw();

	return 0;
}