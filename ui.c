#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h> // Needed for glext.h
#include <GL/glext.h> // For framebuffer funtion pointer types, i think it's necessary? idk man opengl is crazy
#include <GLFW/glfw3.h>

#include <mpv/render_gl.h>
#include "drawtext.h"
#include "playlist.h"

#define WIDTH  800
#define HEIGHT 600

#define SIDEBAR_MIN_W 200
#define SIDEBAR_MIN_H 200

#define ENTRY_H 50 // height of each entry in the playlist

#define FONT_PATH "Renogare-Regular.otf" // temporary (hopefully)
#define FONT_SIZE 24 // temporary (hopefully)

// Colors (ideally these will be configurable in the future)
#define BACKGROUND_COLOR 0xFFFFFF
#define FOREGROUND_COLOR 0x808080
#define FGSELECTED_COLOR 0x506090
#define BGSELECTED_COLOR 0xCCCCFF
#define HEXCOLOR(clr) \
	(clr >> 2*8) & 0xFF, \
	(clr >> 1*8) & 0xFF, \
	(clr >> 0*8) & 0xFF

int win_w, win_h; // Width and height of default framebuffer (whole window)

GLuint vid_fbo; // Video framebuffer object (where the mpv video is drawn)
GLuint vid_tex; // Texture attachment of video fbo
GLsizei vid_w, vid_h; // Width and height of video framebuffer

mpv_render_context* res;

struct dtx_font* font;

void update_vid_size() {
	int max_space, free_space;

	if (win_w > win_h) {
		free_space = win_w - SIDEBAR_MIN_W;
		max_space = win_h;
	} else {
		free_space = win_h - SIDEBAR_MIN_H;
		max_space = win_w;
	}

	vid_w = free_space > max_space? max_space : free_space;
	vid_h = vid_w;

	// Using pixel coordinates rather than relative (libdrawtext needs it i think)
	glViewport(0, 0, win_w, win_h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, win_w, 0, win_h, 0, 1);
}

void handle_fbresize(GLFWwindow* window, int width, int height) {
	// Update default framebuffer size
	win_w = width;
	win_h = height;

	// Update video framebuffer size
	update_vid_size();
	
	glBindTexture(GL_TEXTURE_2D, vid_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, vid_w, vid_h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); // Updating texture size
}

void* get_proc_address(void* ctx, const char* name) {
	return glfwGetProcAddress(name);
}

void free_render() {
	mpv_render_context_free(res);
}

void free_video_fbo() {
	glDeleteFramebuffers(1, &vid_fbo);
}

void free_video_tex() {
	glDeleteTextures(1, &vid_tex);
}

void free_font() {
	dtx_close_font(font);
}

void create_video_fbo() {
	// Generating fbo
	glGenFramebuffers(1, &vid_fbo);
	atexit(free_video_fbo);

	// Binding fbo
	glBindFramebuffer(GL_FRAMEBUFFER, vid_fbo);

	// Creating texture attachment
	glGenTextures(1, &vid_tex);
	atexit(free_video_tex);

	glBindTexture(GL_TEXTURE_2D, vid_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, vid_w, vid_h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); // Since the data is null, we're only allocating memory
	
	// Attach the texture to the fbo as a color attachment
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, vid_tex, 0);
}

void create_mpv_render(mpv_handle* mpv) {
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

	// Getting width and height of the window framebuffer (and resize callback)
	glfwGetFramebufferSize(*win, &win_w, &win_h);
	glfwSetFramebufferSizeCallback(*win, handle_fbresize);

	// Setting size of video framebuffer
	update_vid_size();

	// Creating a framebuffer to be used for rendering the video
	create_video_fbo();

	// Create mpv render context
	create_mpv_render(mpv);

	// Setup dtx font
	if (!(font = dtx_open_font(FONT_PATH, FONT_SIZE))) {
		fprintf(stderr, "ERROR: failed to load font.");
		exit(EXIT_FAILURE);
	}
	dtx_use_font(font, FONT_SIZE);
	atexit(free_font);
}

void draw(GLFWwindow* win) {
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw mpv video to video framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, vid_fbo);
	glViewport(0, 0, vid_w, vid_h);

	mpv_opengl_fbo glfbo = {
		.fbo = vid_fbo,
		.w = vid_w,
		.h = vid_h,
		.internal_format = GL_RGB,
	};

	int flip_y = 1;

	mpv_render_param render_params[] = {
		(mpv_render_param){MPV_RENDER_PARAM_OPENGL_FBO, &glfbo},
		(mpv_render_param){MPV_RENDER_PARAM_FLIP_Y, &flip_y},
		(mpv_render_param){MPV_RENDER_PARAM_INVALID, NULL},
	};

	int err = mpv_render_context_render(res, render_params);
	if (err) fprintf(stderr, "ERROR: an error occured on mpv_render_context_render. error code %d\n", err);

	// Draw video framebuffer in window
  	  // - We're reading the contents of the video framebuffer and drawing to the default framebuffer (window)
	glBindFramebuffer(GL_READ_FRAMEBUFFER, vid_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glViewport(0, 0, win_w, win_h);

	  // - Getting source (video) rectangle that will be drawn
	GLsizei src_x1 = 0, src_y1 = 0;
	GLsizei src_x2 = vid_w, src_y2 = vid_h;

	  // - Getting destination (window) rectangle where the video will be drawn (centered on the available space)
	GLsizei dst_x1 = win_w - vid_w, dst_y1 = win_h - vid_h;
	GLsizei dst_x2 = win_w, dst_y2 = win_h;

	    // - Centering
	if (win_w > win_h) {
		dst_y1 /= 2;
		dst_y2 -= dst_y1;
	} else {
		dst_x1 /= 2;
		dst_x2 -= dst_x1;
	}
	  //

	  // - Drawing
	glBlitFramebuffer(src_x1, src_y1, src_x2, src_y2,
			  dst_x1, dst_y1, dst_x2, dst_y2,
			  GL_COLOR_BUFFER_BIT, GL_LINEAR);

	// Draw sidebar
	float bar_w_px = win_w-vid_w, bar_h_px = win_h-vid_h; // Sidebar size in pixels
	float bar_w = bar_w_px/win_w, bar_h = bar_h_px/win_h; // Sidebar size as percentage of window size
	
	bar_w = bar_w_px;
	bar_h = bar_h_px;

	glColor3ub(HEXCOLOR(BACKGROUND_COLOR));
	if (win_w > win_h) {
		glRectf(0, 0, bar_w, win_h);
	} else {
		glRectf(0, 0, win_w, bar_h);
	}

	// Draw playlist entries
	int entries;
	playlist_entry* playlist = get_playlist(&entries);

	  // - Moving to the top left of the bar
	if (win_w > win_h) {
		glTranslatef(0, win_h, 0);
	} else {
		glTranslatef(0, win_h - vid_h, 0);
	}

	  // - Drawing each entry
	for (int i = 0; i < entries; ++i) {
		// Setting text color
		if (playlist[i].current) glColor3ub(HEXCOLOR(FGSELECTED_COLOR));
		else glColor3ub(HEXCOLOR(FOREGROUND_COLOR));

		// Cutting off text if it goes past the end of the bar
		char* text = playlist[i].title;
		int chars = strlen(text);

		int max_w = win_w > win_h? win_w-vid_w : win_w;
		float txt_w = dtx_string_width(text);
		if (txt_w > max_w) chars = dtx_char_at_pt(text, max_w) - 1;

		// Moving down and drawing text
		glTranslatef(0, -ENTRY_H, 0);
		dtx_substring(text, 0, chars);
	}
	//

	glfwSwapBuffers(win);
	glfwPollEvents();
}

