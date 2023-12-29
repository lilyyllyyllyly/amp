#ifndef AMP_MPV_H
#define AMP_MPV_H

#include <mpv/client.h>

mpv_handle* get_mpv_handle();

void init_mpv();
void update_mpv();

// Playback Control
void load_song(char* filename);
void pause_song();
void next_song();
void prev_song();
void seek_song(char* seek_secs);

#endif

