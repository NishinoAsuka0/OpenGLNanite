#pragma once
#include<iostream>
#include<SOIL2/SOIL2.h>
#define GLEW_STATIC
#include<GL/glew.h>
#include<glut.h>
#include<GLFW/glfw3.h>
#include<glm.hpp>
#include<gtc/matrix_transform.hpp>
#include<gtc/type_ptr.hpp>
#include<vector>

using namespace std;

namespace OpenGLWin {
	class Window {
	private:
		int winHeight;
		int winWidth;
		GLFWwindow* window;
		void MainLoop();
		void End();
	public:
		Window():winHeight(0), winWidth(0), window(nullptr) {};
		Window(int _winHeight, int _winWidth) :winHeight(_winHeight), winWidth(_winWidth), window(nullptr) {};
		void SetWindowSize(int winHeight, int winWidth);
		void Run();
		void InitWindow();
		GLFWwindow* GetWindow();
	};
}
