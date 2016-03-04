#include <glew\glew.h>
#include <GLFW\glfw3.h>

#include <stdio.h>
#include <time.h>

#include "FOWManager.h"

int fake_main(void)
{
	if (!glfwInit())
	{
		return -1;
	}

	GLFWwindow* window = glfwCreateWindow(640, 480, "Hello Triangle", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);

	// start GLEW extention handler
	glewExperimental = GL_TRUE;
	glewInit();

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("Version: %s\n", version);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	float points[] =
	{
		0.0f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, 0.0f, 0.0f
	};

	// use vertex buffer object(VBO)
	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);

	// use vertex attribute object(VAO)
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// init vertex shader
	const char* vertex_shader =
		"#version 400\n"
		"in vec3 vp;"
		"void main(){"
		"	gl_Position = vec4(vp, 1.0);"
		"}";

	// init fragment shader
	const char* fragment_shader =
		"#version 400\n"
		"uniform float time;"
		"uniform sampler2D mapTex;"
		"out vec4 frag_color;"
		"void main(){"
		"	vec3 color = texture(mapTex, glTexCoord.st);"
		"	frag_color = vec4(color, 1.0);"
		"}";
	
	// compile shaders
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, NULL);
	glCompileShader(fs);

	// combine shaders into a single, executable GPU shader programme
	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, fs);
	glAttachShader(shader_program, vs);
	glLinkProgram(shader_program);

	// offer time property
	clock_t tm;
	GLint timeShaderHandle = glGetUniformLocation(shader_program, "time");

	// -------------------------------- game program -------------------------------- //
	const int mapWidth = 64, mapHeight = 64;
	FOWManager fowManager(mapWidth, mapHeight);
	Entity
		e1{ 5, 3, 8.0f, 8.0f },
		e2{ 13, 24, 12.0f, 12.0f },
		e3{ 40, 27, 11.0f, 20.0f };
	fowManager.AddEntity(e1);
	fowManager.AddEntity(e2);
	fowManager.AddEntity(e3);
	fowManager.SetSight(8);

	fowManager.Update();
	// ----------------------------- game program ends --------------------------------- //

	// gen array texture
	GLuint texture = 0;
	GLsizei layerCount = 1;
	GLsizei mipLevelCount = 1;

	GLubyte texels[mapWidth * mapHeight * 4];
	for (int i = 0; i < mapHeight; i++)
	{
		for (int j = 0; j < mapWidth; j++)
		{
			texels[i*mapHeight + j*mapWidth] = (GLubyte)(fowManager.Map()[i][j].r*255);
			texels[i*mapHeight + j*mapWidth + 1] = (GLubyte)(fowManager.Map()[i][j].g * 255);
			texels[i*mapHeight + j*mapWidth + 2] = 0;
			texels[i*mapHeight + j*mapWidth + 3] = 255;
		}
	}

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mapWidth, mapHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)texels);
	//glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevelCount, GL_RGBA8, mapWidth, mapHeight, layerCount);
	//glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, mapWidth, mapHeight, layerCount, GL_RGBA, GL_UNSIGNED_BYTE, texels);

	GLint mapTextureHandle = glGetUniformLocation(shader_program, "mapTex");
	//glUniform3uiv(mapTextureHandle, mapWidth*mapHeight, texels);
	glUniform1i(mapTextureHandle, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, texture);

	while (!glfwWindowShouldClose(window))
	{
		tm = clock();
		float t = ((float)(tm % 100))/100.0f;
		glUniform1f(timeShaderHandle, t);
		

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shader_program);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glfwPollEvents();	// update other events like input handling
		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;
}