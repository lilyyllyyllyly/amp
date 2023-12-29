#include <stdio.h>
#include <stdlib.h>
#include <mpv/client.h>

mpv_handle* mpv = NULL;

mpv_handle* get_mpv_handle() {
	return mpv;
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

void update_mpv() {
	mpv_event* ev = mpv_wait_event(mpv, 0);
	if (ev->event_id == MPV_EVENT_NONE) return;

	printf("INFO: got event with id %d\n", ev->event_id);
	if (ev->error < 0) {
		fprintf(stderr, "WARNING: event with id %d had error with value %d./n", ev->event_id, ev->error);
		return;
	}
}

// Playback control

void load_song(char* filename) {
	const char* args[] = {"loadfile", filename, "append-play"};
	mpv_command(mpv, args);
	printf("INFO: sent command \'%s %s %s\'\n", args[0], args[1], args[2]);
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

