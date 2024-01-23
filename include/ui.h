#ifndef AMP_UI_H
#define AMP_UI_H

#include <GLFW/glfw3.h>
#include <mpv/render_gl.h>

void init_ui(GLFWwindow** win, mpv_handle* mpv);
void draw(GLFWwindow* win);

#endif

