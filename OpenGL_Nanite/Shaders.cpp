#include "Shaders.h"

Shader::Shader(GLchar* vertexPath, GLchar* fragmentPath)
{
	//�ļ���ȡϵ�еı�������
	string vertexCode;
	string fragmentCode;
	ifstream vertexFile;
	ifstream fragmentFile;

	//�쳣���ƴ�������֤ifstream��������׳��쳣��
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

	/* ������ɫ�� */	
	vertexShader = glCreateShader(GL_VERTEX_SHADER);		// ����������ɫ������
	glShaderSource(vertexShader, 1, &vShaderCode, NULL);			// ��������ɫ�������ݴ�����
	glCompileShader(vertexShader);								// ���붥����ɫ��
	GLint flag;												// �����жϱ����Ƿ�ɹ�
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &flag); // ��ȡ����״̬
	if (!flag)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
	}

	/* ƬԪ��ɫ�� */
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);			// ����ƬԪ��ɫ������
	glShaderSource(fragShader, 1, &fShaderCode, NULL);		// ��ƬԪ��ɫ�������ݴ�����
	glCompileShader(fragShader);								// ���붥����ɫ��
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &flag);		// ��ȡ����״̬
	if (!flag)
	{
		glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
	}

	/* ��ɫ������ */
	this->program = glCreateProgram();
	glAttachShader(this->program, vertexShader);
	glAttachShader(this->program, fragShader);
	glLinkProgram(program);

	if (!flag)
	{
		glGetProgramInfoLog(this->program, 512, NULL, infoLog);
		cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
	}
	// ɾ����ɫ���������Ѿ����ӵ����ǵĳ������ˣ��Ѿ�������Ҫ��
	glDeleteShader(vertexShader);
	glDeleteShader(fragShader);
}

Shader::~Shader()
{
	glDetachShader(this->program, this->vertexShader);
	glDetachShader(this->program, this->fragShader);
	glDeleteShader(this->vertexShader);
	glDeleteShader(this->fragShader);
	glDeleteProgram(this->program);
}

void Shader::Use()
{
	glUseProgram(this->program);
}