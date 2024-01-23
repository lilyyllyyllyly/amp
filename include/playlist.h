#ifndef AMP_PLAYLIST_H
#define AMP_PLAYLIST_H

#include <mpv/client.h>

typedef struct playlist_entry {
	char* title;
	int current;
} playlist_entry;

void set_playlist(mpv_node_list* new_list);
playlist_entry* get_playlist(int* ret_size);

#endif

