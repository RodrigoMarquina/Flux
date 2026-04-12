#include "window.h"

GLFWwindow* createWindow(int width, int height, const char * title, GLFWmonitor* monitor){


    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(width, height, title, monitor, NULL);

    return window;
}


