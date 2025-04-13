#pragma once
#include <iostream>
#include <SOIL2/SOIL2.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "Types.h"
#include "VecMath.h"
#include <cmath>
#include <algorithm>
//#include "meshoptimier/src/meshoptimizer.h"
#include "metis.h"

using namespace std;

struct Vertex {
	vec3 pos;
	vec2 uv;

	bool operator==(Vertex& a) {
		return a.pos == pos;
	}
};

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
