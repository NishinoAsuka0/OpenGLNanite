#pragma once	// Ϊ�˱���ͬһ��ͷ�ļ���������include�����
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include "OpenGLwin.h"
using namespace std;

// ��ɫ��
class Shader {
private:
	GLuint vertexShader;	//������ɫ��
	GLuint fragShader;		//ƬԪ��ɫ��

public: 
	GLuint program;		//��ɫ������ID
	Shader(GLchar* vertexPath, GLchar* fragmentPath);
	~Shader();

	void Use();

};