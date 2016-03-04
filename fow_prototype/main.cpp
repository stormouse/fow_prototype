#define DEBUG
#include <glew/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "GLShader.h"
#include <iostream>
#include <exception>
#include <list>
using namespace std;

#pragma region GameEntity
class GameEntity
{
private:
	unsigned int _entityId;
	unsigned int _modelId;
	float _x, _y;
};
#pragma endregion

#pragma region GameWorld
class GameWorld
{
private:
	glm::vec3 _mainCameraPos;
};
#pragma endregion

#pragma region GraphicEngine
class GraphicEngine
{
public:
	GraphicEngine();
private:
	GLFWwindow* _gameWindow;
	void init();
	void update();
};

GraphicEngine::GraphicEngine()
{
	try
	{
		init();
		while (!glfwWindowShouldClose(_gameWindow))
		{
			update();
		}
	}
	catch (exception e)
	{
		cout << e.what() << endl;
	}
}

void GraphicEngine::init()
{
	if (!glfwInit())
	{
		throw new exception("glfw init failed");
		return;
	}
	_gameWindow = glfwCreateWindow(800, 600, "fow prototype", NULL, NULL);
	if (!_gameWindow)
	{
		throw new exception("create window failed");
	}
	glfwMakeContextCurrent(_gameWindow);
	
	glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_DEPTH_TEST);
}

void GraphicEngine::update()
{
	
}

#pragma endregion

// import useful types
typedef glm::vec2 Point;
typedef glm::vec2 Vector2;
typedef glm::ivec2 Vector2i;
typedef glm::vec3 Vector3;

// init game environment
const int WIDTH = 32, HEIGHT = 32;
const int MAX_FOW_ENTITY_COUNT = 30;
const float tileW = 2.0f / WIDTH, tileH = 2.0f / HEIGHT;
GLFWwindow* window;
double mouseX, mouseY;
Vector2i windowResolution = Vector2(640, 640);
bool mouseReleased = true;
GLuint gridVAO;
GLint gridColorLoc;

#define EMPTY 0
#define ENTITY 1
#define VIEWPOINT 2
float fowMap[WIDTH][HEIGHT];
int entityMap[WIDTH][HEIGHT];
list<Point> fowEntities;
Vector2i viewPoint;
float sight = 12.0f;

void updateFOW();
void getCursorPos(double *x, double *y);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void rightClickHandler(double, double);
void leftClickHandler(double, double);
Vector2i view2MapTransform(double, double);
Vector2 map2ViewCenterTransform(int, int);
void toggleEntity(int, int);
void setViewPoint(int, int);
bool intersect(Vector2 a, Vector2 b, Vector2 c, Vector2 d);
void drawWorld();
void init();

Vector2i view2MapTransform(double vx, double vy)
{
	Vector2i mapVec;
	mapVec.x = vx / windowResolution.x * WIDTH;
	mapVec.y = HEIGHT - (vy / windowResolution.y)*HEIGHT;
	return mapVec;
}

Vector2 map2ViewCenterTransform(int gx, int gy)
{
	return Vector2((0.5f + gx)*tileW - 1, (0.5f + gy)*tileH - 1);
}

void toggleEntity(int x, int y)
{
	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
		return;

	Vector2i p = Vector2i(x, y);
	if (entityMap[x][y] > 0)
	{
		fowEntities.remove(p);
		entityMap[x][y] = EMPTY;
	}
	else
	{
		fowEntities.push_back(p);
		entityMap[x][y] = ENTITY;
	}
}

void setViewPoint(int x, int y)
{
	entityMap[x][y] = VIEWPOINT;
	viewPoint.x = x;
	viewPoint.y = y;
}

float crossprod(Vector2 a, Vector2 b)
{
	return a.x*b.y - a.y*b.x;
}

bool intersect(Vector2 a, Vector2 b, Vector2 c, Vector2 d)
{
	Vector2 AB = b - a,
		BC = c - b,
		BD = d - b,
		CD = d - c,
		DA = a - d,
		DB = b - d;

	if (crossprod(AB, BC) * crossprod(AB, BD) <= 0 && crossprod(CD, DA) * crossprod(CD, DB) <= 0)
		return true;

	return false;
}

void getCursorPos(double *x, double *y)
{
	glfwGetCursorPos(window, x, y);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	getCursorPos(&mouseX, &mouseY);

	if (mouseReleased)
	{
		if (GLFW_MOUSE_BUTTON_RIGHT == button && GLFW_PRESS == action)
		{
			rightClickHandler(mouseX, mouseY);
		}
		else if (GLFW_MOUSE_BUTTON_LEFT == button && GLFW_PRESS == action)
		{
			leftClickHandler(mouseX, mouseY);
		}
		mouseReleased = false;
	}
	else
	{
		if (GLFW_RELEASE == action)
		{
			mouseReleased = true;
		}
	}
}

void rightClickHandler(double mouseX, double mouseY)
{
	Vector2i tile = view2MapTransform(mouseX, mouseY);
	toggleEntity(tile.x, tile.y);
	updateFOW();
}

void leftClickHandler(double mouseX, double mouseY)
{
	Vector2i tile = view2MapTransform(mouseX, mouseY);
	if (entityMap[tile.x][tile.y] == EMPTY)
	{
		if (viewPoint.x>=0 && viewPoint.x<WIDTH &&
			viewPoint.y>=0 && viewPoint.y<HEIGHT &&
			entityMap[viewPoint.x][viewPoint.y] == VIEWPOINT)
		{
			entityMap[viewPoint.x][viewPoint.y] = EMPTY;
		}
		setViewPoint(tile.x, tile.y);
		updateFOW();
	}
}

void updateFOW()
{
	int si = int(glm::ceil(sight));
	memset(fowMap, 0.0f, sizeof(fowMap));
	for (int ix = viewPoint.x - si; ix < viewPoint.x + si; ix++)
	{
		if (ix < 0)
		{
			ix = 0;
			continue;
		}
		else if (ix >= WIDTH)break;

		for (int iy = viewPoint.y - si; iy < viewPoint.y + si; iy++)
		{
			if (iy < 0)
			{
				iy = 0;
				continue;
			}
			else if (iy >= HEIGHT)break;

			fowMap[ix][iy] = 1.0f;

			Vector2 p = Vector2(ix, iy);
			Vector2 viewPointf = Vector2(viewPoint.x, viewPoint.y);
			Vector2 p_vec = p - viewPointf;
			float p_dist = glm::distance<float>(p, viewPointf);
			
			for (Vector2i e : fowEntities)
			{
				Vector2 ef = Vector2(e.x, e.y);
				float e_dist = glm::distance<float>(ef, viewPointf);
				// see whether entity is farther from viewpoint than this grid
				if (e_dist > p_dist)
					continue;
				
				// see whether LOS crosses horizontal diagonal
				Vector2 e_vec = ef - viewPointf;
				Vector2 dia11 = Vector2(ef.x - 0.5, ef.y - 0.5),
					dia12 = Vector2(ef.x + 0.5, ef.y + 0.5),
					dia21 = Vector2(ef.x - 0.5, ef.y + 0.5),
					dia22 = Vector2(ef.x + 0.5, ef.y - 0.5);

				// select a more perpendicular one
				Vector2 dia1_vec = dia12 - dia11;
				Vector2 dia2_vec = dia22 - dia21;
				Vector2 p_vec_mag = p_vec / p_dist;
				float cos1 = glm::dot<float>(dia1_vec, p_vec_mag),
					cos2 = glm::dot<float>(dia2_vec, p_vec_mag);

				bool blocked = false;
				if (glm::abs(cos1) < glm::abs(cos2))
					blocked = intersect(dia11, dia12, p, viewPointf);
				else
					blocked = intersect(dia21, dia22, p, viewPointf);

				// brighten
				if (blocked)
					fowMap[ix][iy] = 0.0f;
			}
		}
	}
}

glm::vec3 hidden_col = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 reveal_col = glm::vec3(0.2f, 0.8f, 0.3f);
glm::vec3 viewpoint_col = glm::vec3(0.8f, 0.2f, 0.2f);
glm::vec3 entity_col = glm::vec3(0.6f, 0.6f, 0.6f);
GLuint fow_shader_program;

GLuint vbo;
GLuint vao;

void drawGrid(glm::vec3 color, int gx, int gy)
{
	glUniform3f(gridColorLoc, color.r, color.g, color.b);
	glDrawArrays(GL_QUADS, gy * 4 * WIDTH + gx * 4, 4);
}

void drawWorld()
{
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	//float grid_col[WIDTH*HEIGHT * 3];
	for (int ix = 0; ix < WIDTH; ix++)
	{
		for (int iy = 0; iy < HEIGHT; iy++)
		{
			glm::vec3 col = glm::mix(glm::vec3(hidden_col), glm::vec3(reveal_col), fowMap[ix][iy]);
			if (entityMap[ix][iy] != EMPTY)
			{
				switch (entityMap[ix][iy])
				{
				case ENTITY:
					col = entity_col;
					break;
				case VIEWPOINT:
					col = viewpoint_col;
					break;
				}
			}
			drawGrid(col, ix, iy);
			/*int index = iy*WIDTH + ix * 3;
			grid_col[index + 0] = col.r;
			grid_col[index + 1] = col.g;
			grid_col[index + 2] = col.b;*/
		}
	}
}

void init()
{
	fow_shader_program = LoadShader("grid_vertex.glsl", "grid_fragment.glsl");

	glm::mat4x4 viewMat = glm::ortho<float>(0.0f, windowResolution.x, 0.0, windowResolution.y, 0.1f, 100.0f);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);


	float vertices[WIDTH * HEIGHT * 12];
	for (int ix = 0; ix < WIDTH; ix++)
	{
		for (int iy = 0; iy < HEIGHT; iy++)
		{
			glm::vec2 c = map2ViewCenterTransform(ix, iy);
			int index = iy * WIDTH * 12 + ix * 12;
			vertices[index + 0] = c.x - 0.5*tileW;
			vertices[index + 1] = c.y - 0.5*tileH;
			vertices[index + 2] = 0.0f;

			vertices[index + 3] = c.x + 0.5*tileW;
			vertices[index + 4] = c.y - 0.5*tileH;
			vertices[index + 5] = 0.0f;

			vertices[index + 6] = c.x + 0.5*tileW;
			vertices[index + 7] = c.y + 0.5*tileH;
			vertices[index + 8] = 0.0f;

			vertices[index + 9] = c.x - 0.5*tileW;
			vertices[index + 10] = c.y + 0.5*tileH;
			vertices[index + 11] = 0.0f;
		}
	}

	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	gridVAO = vao;

	gridColorLoc = glGetUniformLocation(fow_shader_program, "fowColor");
}

void update()
{
	getCursorPos(&mouseX, &mouseY);
	glBindVertexArray(gridVAO);
	drawWorld();
	glBindVertexArray(0);
}

int main()
{
	if (!glfwInit())
		return -1;
	window = glfwCreateWindow(windowResolution.x, windowResolution.y, "fow prototype", NULL, NULL);
	if (!window)
		return 1;
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	glewInit();
	glViewport(0, 0, windowResolution.x, windowResolution.y);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("Version: %s\n", version);

	init();
	Vector2 center = Vector2(0, 0);

	//GLint gridColorLoc = glGetUniformLocation(fow_shader_program, "grid_col");

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glUseProgram(fow_shader_program);
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);
		
		update();
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	return 0;
}