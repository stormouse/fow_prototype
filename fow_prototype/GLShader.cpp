#include "GLShader.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

std::string readFile(const char *filePath)
{
	std::string content;
	std::ifstream fileStream(filePath, std::ios::in);

	if (!fileStream.is_open())
	{
		std::cerr << "File error " << filePath << std::endl;
		return "";
	}

	std::string line = "";
	while (!fileStream.eof())
	{
		std::getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();
	return content;
}

GLuint LoadShader(const char* vertex_path, const char* fragment_path)
{
	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	std::string vertstr_const = readFile(vertex_path);
	std::string fragstr_const = readFile(fragment_path);
	char *vertstr = new char[vertstr_const.length()];
	char *fragstr = new char[fragstr_const.length()];
	strcpy(vertstr, vertstr_const.c_str());
	strcpy(fragstr, fragstr_const.c_str());

	glShaderSource(vertShader, 1, &vertstr, NULL);
	glShaderSource(fragShader, 1, &fragstr, NULL);

	GLint result = GL_FALSE;
	int logLength;
	glCompileShader(vertShader);
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
	std::vector<GLchar> vertShaderError((logLength > 1) ? logLength : 1);
	glGetShaderInfoLog(vertShader, logLength, NULL, &vertShaderError[0]);
	std::cout << &vertShaderError[0] << std::endl;
	
	result = GL_FALSE;
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
	std::vector<GLchar> fragShaderError((logLength > 1) ? logLength : 1);
	glGetShaderInfoLog(fragShader, logLength, NULL, &fragShaderError[0]);
	std::cout << &fragShaderError[0] << std::endl;

	GLuint program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &result);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
	std::vector<char> programError((logLength > 1) ? logLength : 1);
	glGetProgramInfoLog(program, logLength, NULL, &programError[0]);
	std::cout << &programError[0] << std::endl;

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return program;
}