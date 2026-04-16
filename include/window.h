#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>

GLFWwindow* createWindow(int width, int height, const char * title, GLFWmonitor* monitor);