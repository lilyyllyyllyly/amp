#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <mpv/client.h>

mpv_handle* mpv = NULL;

void handle_interrupt(int signal) {
	printf("Exiting from signal %d\n", signal);
	exit(EXIT_SUCCESS);
}

void quit_mpv() {
	mpv_terminate_destroy(mpv);
}

int main(int argc, char* argv[]) {
	signal(SIGINT, handle_interrupt);

	if (argc < 2) {
		fprintf(stderr, "ERROR: no music file given.\n");
		exit(EXIT_FAILURE);
	}

	// Initializing mpv
	mpv = mpv_create();
	if (!mpv) {
		fprintf(stderr, "ERROR: failed to create mpv instance.\n");
		exit(EXIT_FAILURE);
	}
	atexit(quit_mpv);

	int err = mpv_initialize(mpv);
	if (err) {
		fprintf(stderr, "ERROR: failed to initialize mpv instance. error code: %d\n", err);
		exit(EXIT_FAILURE);
	}

	// Playing music file
	const char* command_args[] = {"loadfile", "append-play", argv[1]};
	mpv_command(mpv, command_args);

	while (1);

	exit(EXIT_SUCCESS);
}

