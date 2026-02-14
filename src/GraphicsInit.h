#pragma once

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

inline glm::ivec2 mousePosition;
void mouseMoveCallback(GLFWwindow* window, double x, double y);

void glfwErrorCallback(int error, const char* description);

GLFWwindow* InitGraphics();
