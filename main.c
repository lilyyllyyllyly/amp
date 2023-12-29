#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <mpv/client.h>
#include <GLFW/glfw3.h>

#define SEEK "5" // seconds to seek in song

#define WIDTH  800
#define HEIGHT 600

mpv_handle* mpv = NULL;
int wakeup = 0;

GLFWwindow* win = NULL;

void handle_interrupt(int signal) {
	puts("\nReceived interrupt signal, ending event loop...");
	mpv_wakeup(mpv);
	wakeup = 1;
}

void quit_mpv() {
	mpv_terminate_destroy(mpv);
	mpv = NULL;
}

void init_mpv() {
  	// Creating instance
	mpv = mpv_create();
	if (!mpv) {
		fprintf(stderr, "ERROR: failed to create mpv instance.\n");
		exit(EXIT_FAILURE);
	}
	atexit(quit_mpv);

	// Setting properties
	mpv_set_property_string(mpv, "video", "no");

	// Initializing
	int err = mpv_initialize(mpv);
	if (err) {
		fprintf(stderr, "ERROR: failed to initialize mpv instance. error code: %d\n", err);
		exit(EXIT_FAILURE);
	}
}

void pause_song() {
	const char* args[] = {"cycle", "pause"};
	mpv_command(mpv, args);
	printf("INFO: sent command \'%s %s\'\n", args[0], args[1]);
}

void next_song() {
	const char* args[] = {"playlist-next"};
	mpv_command(mpv, args);
	printf("INFO: sent command \'%s\'\n", args[0]);
}

void prev_song() {
	const char* args[] = {"playlist-prev"};
	mpv_command(mpv, args);
	printf("INFO: sent command \'%s\'\n", args[0]);
}

void seek_song(char* seek_secs) {
	const char* args[] = {"seek", seek_secs};
	mpv_command(mpv, args);
	printf("INFO: sent command \'%s %s\'\n", args[0], args[1]);
}

void update_mpv() {
	mpv_event* ev = mpv_wait_event(mpv, 0);
	if (ev->event_id == MPV_EVENT_NONE) return;

	printf("INFO: got event with id %d\n", ev->event_id);
	if (ev->error < 0) {
		fprintf(stderr, "WARNING: event with id %d had error with value %d./n", ev->event_id, ev->error);
		return;
	}
}

void handle_keyevent(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS) return;

	switch (key) {
		case GLFW_KEY_SPACE: pause_song(); break;
		default: break;
	}
}

void init_ui() {
	// Initializing glfw
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: failed to init GLFW.\n");
		exit(EXIT_FAILURE);
	}
	atexit(glfwTerminate);

	// Creating window
	win = glfwCreateWindow(WIDTH, HEIGHT, "amp - A Music Player", NULL, NULL);
	if (!win) {
		fprintf(stderr, "ERROR: failed to create window.");
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(win);

	// Keyboard key callback
	glfwSetKeyCallback(win, handle_keyevent);
}

void draw() {
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(win);
	glfwPollEvents();
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		fprintf(stderr, "ERROR: no music file given.\n");
		exit(EXIT_FAILURE);
	}

	init_mpv();
	init_ui();

	// Start playing music file
	const char* args[] = {"loadfile", argv[1], "append-play"};
	mpv_command(mpv, args);
	printf("INFO: sent command \'%s %s %s\'\n", args[0], args[1], args[2]);

	// Main loop
	signal(SIGINT, handle_interrupt); // Stop mpv event loop on SIGINT
	while (!wakeup && !glfwWindowShouldClose(win)) {
		update_mpv();
		draw();
	}

	puts("Quitting.");
	exit(EXIT_SUCCESS);
}

