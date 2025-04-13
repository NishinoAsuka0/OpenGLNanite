#include<iostream>
#include"Engine.h"
#include"Model.h"
using namespace std;


shared_ptr<Engine> Engine::instance = nullptr;
mutex Engine::i_mutex;//�����ʼ��

const GLint WIDTH = 1920, HEIGHT = 1080;		// ���ڵĳ��Ϳ�


int main(int argc, char** argv[])
{

	auto engine = Engine::GetInstance();
    engine->InitEngine();
	engine->SetWindowSize(WIDTH, HEIGHT);

	//����ģ��
	//Model model(path);
	//engine->LoadMesh(mesh);
	cout << "========================================" << endl;
	cout << "�뽫ģ���ļ��з���Ŀ¼������ģ���ļ���" << endl;
	cout << "========================================" << endl;
	cout << "������ģ���ļ����ƣ�";
	string path;
	cin >> path;
	cout << endl;

	cout << "========================================" << endl;
	cout << "1Ϊ��ͨ��Ⱦģʽ 2ΪCluster��Ⱦģʽ" << endl;
	cout << "========================================" << endl;
	cout << "��������Ⱦģʽ��";

	unsigned int mode;
	cin >> mode;
	cout << endl;

	if (mode > 2 || mode < 1) {
		mode = 2; //������������� Ĭ��ΪCluster��Ⱦ
	}
	engine->SetRenderMode(mode);
	engine->LoadModelPath(path);
	engine->Draw();

	return 0;
}