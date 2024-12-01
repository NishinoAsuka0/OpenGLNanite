#include "OpenGLwin.h"

void OpenGLWin::Window::InitWindow()
{
	glfwInit();
	//glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);

	window = glfwCreateWindow(winHeight, winWidth, "LearnOpenGL", nullptr, nullptr);

	if (window == nullptr)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return;
	}
	/*if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to create GLFW window" << endl;
		return ;
	}*/

}

void OpenGLWin::Window::MainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glfwPollEvents();
		glfwSwapBuffers(window);
	}
}

void OpenGLWin::Window::End()
{
	glfwTerminate();
}

void OpenGLWin::Window::SetWindowSize(int winHeight, int winWidth)
{
	this->winHeight = winHeight;
	this->winWidth = winWidth;
}

void OpenGLWin::Window::Run()
{
	this->InitWindow();
	this->MainLoop();
	this->End();
}

GLFWwindow* OpenGLWin::Window::GetWindow()
{
	return this->window;
}
