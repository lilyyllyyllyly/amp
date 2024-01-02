#include <stdio.h>
#include <stdlib.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h> // Needed for glext.h
#include <GL/glext.h> // For framebuffer funtion pointer types, i think it's necessary? idk man opengl is crazy
#include <GLFW/glfw3.h>
#include <mpv/render_gl.h>

#define WIDTH  800
#define HEIGHT 600

int win_w, win_h; // Width and height of default framebuffer (whole window)

GLuint vid_fbo; // Video framebuffer object (where the mpv video is drawn)
GLuint vid_tex; // Texture attachment of video fbo
GLsizei vid_w, vid_h; // Width and height of video framebuffer

mpv_render_context* res;

void update_vid_size() {
	if (win_w > win_h) {
		// The window is horizontal, so the video takes as much vertical space as possible
		vid_h = win_h;
		vid_w = vid_h;
	} else {
		// The window is vertical, so the video takes as much horizontal space as possible
		vid_w = win_w;
		vid_h = vid_w;
	}
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
}

void draw(GLFWwindow* win) {
	glClear(GL_COLOR_BUFFER_BIT);

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

	// Drawing video framebuffer in window
  	  // - We're reading the contents of the video framebuffer and drawing to the default framebuffer (window)
	glBindFramebuffer(GL_READ_FRAMEBUFFER, vid_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glViewport(0, 0, win_w, win_h);

	  // - Getting source (video) rectangle that will be drawn
	GLsizei src_x = 0, src_y = 0;
	GLsizei src_w = vid_w, src_h = vid_h;

	  // - Getting destination (window) rectangle where the video will be drawn (positioned in the top right of the window)
	GLsizei dst_x = win_w-vid_w, dst_y = win_h-vid_h;
	GLsizei dst_w = vid_w, dst_h = vid_w;

	  // - Drawing
	glBlitFramebuffer(src_x, src_y, src_x+src_w, src_y+src_h,
			  dst_x, dst_y, dst_x+dst_w, dst_y+dst_h,
			  GL_COLOR_BUFFER_BIT, GL_LINEAR);
	//

	glfwSwapBuffers(win);
	glfwPollEvents();
}

