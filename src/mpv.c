#include <stdio.h>
#include <stdlib.h>

#include <mpv/client.h>
#include "playlist.h"

mpv_node* playlist_node = NULL;

mpv_handle* mpv = NULL;
mpv_handle* get_mpv_handle() {
	return mpv;
}

void quit_mpv() {
	mpv_terminate_destroy(mpv);
	mpv = NULL;

	if (playlist_node) mpv_free_node_contents(playlist_node);
	set_playlist(NULL);
}

void init_mpv() {
  	// Creating instance
	mpv = mpv_create();
	if (!mpv) {
		fprintf(stderr, "ERROR: failed to create mpv instance.\n");
		exit(EXIT_FAILURE);
	}
	atexit(quit_mpv);

	// Initializing
	int err = mpv_initialize(mpv);
	if (err) {
		fprintf(stderr, "ERROR: failed to initialize mpv instance. error code: %d\n", err);
		exit(EXIT_FAILURE);
	}

	// Playlist stuff
	playlist_node = malloc(sizeof(mpv_node_list));
	mpv_observe_property(mpv, 0, "playlist", MPV_FORMAT_NONE); // Listening to changes in the playlist
}

void update_mpv() {
	mpv_event* ev = mpv_wait_event(mpv, 0);
	if (ev->event_id == MPV_EVENT_NONE) return;

	printf("INFO: got event with id %d\n", ev->event_id);
	if (ev->error < 0) {
		fprintf(stderr, "WARNING: event with id %d had error with value %d./n", ev->event_id, ev->error);
		return;
	}

	// Playlist changed (only property we're listening to is playlist)
	if (ev->event_id == MPV_EVENT_PROPERTY_CHANGE) {
		if (playlist_node) mpv_free_node_contents(playlist_node);

		mpv_get_property(mpv, "playlist", MPV_FORMAT_NODE, playlist_node);
		if (!playlist_node) return;

		if (playlist_node->format != MPV_FORMAT_NODE_ARRAY) return;
		set_playlist(playlist_node->u.list);
	}
}

// Playback control

void load_song(char* filename) {
	// Loading song
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

