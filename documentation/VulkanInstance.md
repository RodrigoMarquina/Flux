The vulkan instance initializes the vulkan loader which is how we communicate with the driver. It also loads extensions at the instance level and sets up validation layers to help us debug.

Instance-level extensions add functionality to the Vulkan instance itself — things that exist before you even have a device. Surface support (VK_KHR_surface) is an example — you need it to create a surface, which happens before you pick a physical device.

Device-level extensions add functionality to a specific logical device — things the GPU needs to support. The swapchain (VK_KHR_swapchain) is an example — it's the GPU that actually presents images.