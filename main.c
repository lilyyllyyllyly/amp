#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "mpv.h"
#include "ui.h"

#define SEEK "5" // seconds to seek in song

int wakeup = 0; // When true, mpv event loop will quit
GLFWwindow* win = NULL;

void handle_interrupt(int signal) {
	puts("\nReceived interrupt signal, ending event loop...");
	mpv_wakeup(get_mpv_handle());
	wakeup = 1;
}

void handle_keyevent(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS) return;

	switch (key) {
		case GLFW_KEY_SPACE: pause_song(); break;
		case GLFW_KEY_LEFT:
			if (mods & GLFW_MOD_CONTROL) {
				prev_song();
			} else {
				seek_song("-"SEEK);
			}
		break;
		case GLFW_KEY_RIGHT:
			if (mods & GLFW_MOD_CONTROL) {
				next_song();
			} else {
				seek_song(SEEK);
			}
		break;
		default: break;
	}
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		fprintf(stderr, "ERROR: no music file given.\n");
		exit(EXIT_FAILURE);
	}

	init_mpv();
	init_ui(&win);
	glfwSetKeyCallback(win, handle_keyevent);

	// Start playing music file
	load_song(argv[1]);

	// Main loop
	signal(SIGINT, handle_interrupt); // Stop mpv event loop on SIGINT
	while (!wakeup && !glfwWindowShouldClose(win)) {
		update_mpv();
		draw(win);
	}

	puts("Quitting.");
	exit(EXIT_SUCCESS);
}

