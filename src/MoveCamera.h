#pragma once

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <utility/Camera.h>

void MoveCamera(RenderingUtilities::Camera& camera, GLFWwindow* window, float dt, const glm::ivec2& mousePositionWRTViewport, const glm::ivec2& viewportSize, bool mouseOverViewport);
