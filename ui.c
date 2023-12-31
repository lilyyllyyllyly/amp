#include <stdio.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>
#include <mpv/render_gl.h>

#define WIDTH  800
#define HEIGHT 600

mpv_render_context* res;
int win_w, win_h;

void handle_fbresize(GLFWwindow* window, int width, int height) {
	win_w = width;
	win_h = height;

	glViewport(0, 0, win_w, win_h); // Resizing viewport
}

void* get_proc_address(void* ctx, const char* name) {
	return glfwGetProcAddress(name);
}

void free_render() {
	mpv_render_context_free(res);
}

void init_ui(GLFWwindow** win, mpv_handle* mpv) {
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

	// Getting width and height (and resize callback)
	glfwGetFramebufferSize(*win, &win_w, &win_h);
	glfwSetFramebufferSizeCallback(*win, handle_fbresize);

	// Create mpv render context
	mpv_opengl_init_params glinit = {
		get_proc_address,
		NULL,
	};
	
	mpv_render_param render_params[] = {
		(mpv_render_param){MPV_RENDER_PARAM_API_TYPE, MPV_RENDER_API_TYPE_OPENGL},
		(mpv_render_param){MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &glinit},
		(mpv_render_param){MPV_RENDER_PARAM_INVALID, NULL},
	};

	int err = mpv_render_context_create(&res, mpv, render_params);
	if (err) {
		fprintf(stderr, "ERROR: failed to create mpv render context. error code %d\n", err);
		exit(EXIT_FAILURE);
	}
	atexit(free_render);
}

void draw(GLFWwindow* win) {
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw mpv video
	// TODO

	glfwSwapBuffers(win);
	glfwPollEvents();
}

