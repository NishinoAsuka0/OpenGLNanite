#include<iostream>
#include"Engine.h"
using namespace std;



shared_ptr<Engine> Engine::instance = nullptr;
mutex Engine::i_mutex;//�����ʼ��
/* ��д������λ������ɫ */
/* ��д��������Ϣ */
GLfloat vertices_1[] = {
    // x��y��z ����				// color
    -0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 0.0f,	// red ��ɫ��
     0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,		1.0f, 0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,		1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,		1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,		0.0f, 1.0f, 0.0f,	// green ��ɫ��
     0.5f, -0.5f,  0.5f,		0.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,		0.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,		0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,		0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,		0.0f, 1.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,		0.0f, 0.0f, 1.0f,	// blue ��ɫ��
    -0.5f,  0.5f, -0.5f,		0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,		0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,		0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,		0.0f, 0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,		0.0f, 0.0f, 1.0f,

     0.5f,  0.5f,  0.5f,		1.0f, 1.0f, 0.0f,	// yellow ��ɫ��
     0.5f,  0.5f, -0.5f,		1.0f, 1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,		1.0f, 1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,		1.0f, 1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,		1.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,		1.0f, 1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 1.0f,	// purple ��ɫ��
     0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,		1.0f, 0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,		1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,		1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,		0.0f, 1.0f, 1.0f,	// cyan ��ɫ��
     0.5f,  0.5f, -0.5f,		0.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,		0.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,		0.0f, 1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,		0.0f, 1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,		0.0f, 1.0f, 1.0f,
};

/* �ĸ������������Ϣ������ */
GLuint indices_1[] =
{
	0, 1, 3,		// ���Ϊ 0��1��3 �Ķ�����ϳ�һ��������
	1, 2, 3			// ���Ϊ 1��2��3 �Ķ�����ϳ�һ��������
};

const GLint WIDTH = 1920, HEIGHT = 1080;		// ���ڵĳ��Ϳ�



int main(int argc, char** argv[])
{
	auto engine = Engine::GetInstance();
    engine->InitEngine();
	engine->SetWindowSize(WIDTH, HEIGHT);
	engine->Draw(vertices_1, sizeof(vertices_1));

	return 0;
}