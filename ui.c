#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

#define WIDTH  800
#define HEIGHT 600

void init_ui(GLFWwindow** win) {
	// Initializing glfw
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: failed to init GLFW.\n");
		exit(EXIT_FAILURE);
	}
	atexit(glfwTerminate);

	// Creating window
	*win = glfwCreateWindow(WIDTH, HEIGHT, "amp - A Music Player", NULL, NULL);
	if (!*win) {
		fprintf(stderr, "ERROR: failed to create window.");
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(*win);
}

void draw(GLFWwindow* win) {
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(win);
	glfwPollEvents();
}

