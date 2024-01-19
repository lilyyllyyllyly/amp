#include <stdlib.h>
#include <string.h>

#include <mpv/client.h>
#include "playlist.h"

#define DEFAULT_TITLE "unknown"

static playlist_entry* playlist;
static int size;

void set_playlist(mpv_node_list* new_list) {
	if (!new_list) return;

	if (playlist) free(playlist);
	size = new_list->num;
	playlist = malloc(size * sizeof(*playlist));

	for (int i = 0; i < size; ++i) { // going through each playlist entry
		playlist[i].title = DEFAULT_TITLE;
		playlist[i].current = 0;

		mpv_node song_node = new_list->values[i];
		if (song_node.format != MPV_FORMAT_NODE_MAP) continue;

		mpv_node_list song = *song_node.u.list;

		int is_title = 0, is_filename = 0;
		for (int j = 0; j < song.num; ++j) { // going through each kv pair in the song
			switch(song.values[j].format) {
				case MPV_FORMAT_STRING:
					if (is_title) continue; // already set song title and it's not filename

					is_title    = strcmp(song.keys[j], "title") == 0;
					is_filename = strcmp(song.keys[j], "filename") == 0;
					if (!is_title && !is_filename) continue;

					playlist[i].title = song.values[j].u.string;
				break;

				case MPV_FORMAT_FLAG:
					if (strcmp(song.keys[j], "current") == 0) playlist[i].current = song.values[j].u.flag;
				break;

				default: break;
			}
		}
	}
}

playlist_entry* get_playlist(int* ret_size) {
	*ret_size = size;
	return playlist;
}

