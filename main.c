#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <mpv/client.h>

#define NO_TIMEOUT -1

mpv_handle* mpv = NULL;
int wakeup = 0;

void handle_interrupt(int signal) {
	puts("\nReceived interrupt signal, ending event loop...");
	mpv_wakeup(mpv);
	wakeup = 1;
}

void quit_mpv() {
	mpv_terminate_destroy(mpv);
	mpv = NULL;
}

int main(int argc, char* argv[]) {
	signal(SIGINT, handle_interrupt);

	if (argc < 2) {
		fprintf(stderr, "ERROR: no music file given.\n");
		exit(EXIT_FAILURE);
	}

	// Initializing mpv
  	  // - Creating instance
	mpv = mpv_create();
	if (!mpv) {
		fprintf(stderr, "ERROR: failed to create mpv instance.\n");
		exit(EXIT_FAILURE);
	}
	atexit(quit_mpv);

	  // - Setting properties
	mpv_set_property_string(mpv, "video", "no");

	  // - Initializing
	int err = mpv_initialize(mpv);
	if (err) {
		fprintf(stderr, "ERROR: failed to initialize mpv instance. error code: %d\n", err);
		exit(EXIT_FAILURE);
	}

	// Start playing music file
	const char* args[] = {"loadfile", argv[1], "append-play"};
	mpv_command_async(mpv, 0, args);
	printf("INFO: sent command %s %s %s\n", args[0], args[1], args[2]);

	// Event loop
	mpv_event* ev;
	while (!wakeup) {
		ev = mpv_wait_event(mpv, NO_TIMEOUT);
		printf("INFO: got event with id %d\n", ev->event_id);
		if (ev->error < 0) {
			fprintf(stderr, "WARNING: event with id %d had error with value %d./n", ev->event_id, ev->error);
			continue;
		}
	}

	puts("Quitting.");
	exit(EXIT_SUCCESS);
}

