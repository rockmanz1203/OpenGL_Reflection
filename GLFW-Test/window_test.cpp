#include <stdio.h>
#include <stdlib.h>
// Include GLEW
#include "include\GL\glew.h"

// Include GLFW
#include "include\GLFW\glfw3.h"
GLFWwindow* window; //建立一個windows

					// Include GLM
#include "glm\glm.hpp"
using namespace glm;


int main(void) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);


	return 0;


}