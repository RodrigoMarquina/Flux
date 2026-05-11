The vulkan surface is the interface between the vulkan instance and the os. Its how we communicate with the os and through the surface we display the images to the window.

We use the glfwCreateWindowSurface() function which creates the vulkan surface for a specific window and connects the instance, the surface and the window.

GLFW handles the platform-specific surface creation under the hood. On Linux it might create an X11 or Wayland surface, on Windows a Win32 surface. 