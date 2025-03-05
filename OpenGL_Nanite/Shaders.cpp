#include "Shaders.h"

Shader::Shader(GLchar* computePath)
{
	type = 1;
	string computeCode;
	ifstream cShaderFile;
	cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		cShaderFile.open(computePath);
		std::stringstream cShaderStream;
		cShaderStream << cShaderFile.rdbuf();
		cShaderFile.close();
		computeCode = cShaderStream.str();
	}
	catch (std::ifstream::failure& e)
	{
		std::cerr << "Compute Shader 文件读取失败: " << e.what() << std::endl;
	}

	const char* cShaderCode = computeCode.c_str();
	int success;
	char infoLog[512];

	computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShader, 1, &cShaderCode, nullptr);
	glCompileShader(computeShader);
	glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(computeShader, 512, nullptr, infoLog);
		std::cerr << "Compute Shader 编译错误:\n" << infoLog << std::endl;
	}

	program = glCreateProgram();
	glAttachShader(program, computeShader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		std::cerr << "Compute Shader 程序链接错误:\n" << infoLog << std::endl;
	}
	glDeleteShader(computeShader);
}

Shader::Shader(GLchar* vertexPath, GLchar* fragmentPath)
{
	type = 0;
	//文件读取系列的变量定义
	string vertexCode;
	string fragmentCode;
	ifstream vertexFile;
	ifstream fragmentFile;

	//异常机制处理：保证ifstream对象可以抛出异常：
	vertexFile.exceptions(ifstream::badbit);
	fragmentFile.exceptions(ifstream::badbit);

	try
	{
		vertexFile.open(vertexPath);
		fragmentFile.open(fragmentPath);
		stringstream vShaderStream;
		stringstream fShaderStream;

		vShaderStream << vertexFile.rdbuf();
		fShaderStream << fragmentFile.rdbuf();

		vertexFile.close();
		fragmentFile.close();

		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (ifstream::failure e)
	{
		cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << endl;
	}

	const GLchar* vShaderCode = vertexCode.c_str();
	const GLchar* fShaderCode = fragmentCode.c_str();

	/* 顶点着色器 */	
	vertexShader = glCreateShader(GL_VERTEX_SHADER);		// 创建顶点着色器对象
	glShaderSource(vertexShader, 1, &vShaderCode, NULL);			// 将顶点着色器的内容传进来
	glCompileShader(vertexShader);								// 编译顶点着色器
	GLint flag;												// 用于判断编译是否成功
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &flag); // 获取编译状态
	if (!flag)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
	}

	/* 片元着色器 */
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);			// 创建片元着色器对象
	glShaderSource(fragShader, 1, &fShaderCode, NULL);		// 将片元着色器的内容传进来
	glCompileShader(fragShader);								// 编译顶点着色器
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &flag);		// 获取编译状态
	if (!flag)
	{
		glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
	}

	/* 着色器程序 */
	this->program = glCreateProgram();
	glAttachShader(this->program, vertexShader);
	glAttachShader(this->program, fragShader);
	glLinkProgram(program);

	if (!flag)
	{
		glGetProgramInfoLog(this->program, 512, NULL, infoLog);
		cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
	}
	// 删除着色器，它们已经链接到我们的程序中了，已经不再需要了
	glDeleteShader(vertexShader);
	glDeleteShader(fragShader);
}

Shader::~Shader()
{
	if (type == 0) {
		glDetachShader(this->program, this->vertexShader);
		glDetachShader(this->program, this->fragShader);
		glDeleteShader(this->vertexShader);
		glDeleteShader(this->fragShader);
		glDeleteProgram(this->program);
	}
	if (type == 1) {
		glDetachShader(this->program, this->computeShader);
		glDeleteShader(this->computeShader);
		glDeleteProgram(this->program);
	}
}

void Shader::Use()
{
	glUseProgram(this->program);
}


