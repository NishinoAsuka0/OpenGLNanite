#pragma once	// 为了避免同一个头文件被包含（include）多次
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include "OpenGLwin.h"
using namespace std;

// 着色器
class Shader {
private:
	GLuint vertexShader;	//顶点着色器
	GLuint fragShader;		//片元着色器

public: 
	GLuint program;		//着色器程序ID
	Shader(GLchar* vertexPath, GLchar* fragmentPath);
	~Shader();

	void Use();

};