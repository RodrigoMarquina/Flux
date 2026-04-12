#include "vulkan_instance.h"

VkResult createVulkanInstance(VkInstance* instance){
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Flux";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    
    const char* layers[] = {"VK_LAYER_KHRONOS_validation"};

    #ifdef NDEBUG
        createInfo.enabledLayerCount = 0; //Validation layers
    #else
        createInfo.enabledLayerCount = 1; //Validation layers

        createInfo.ppEnabledLayerNames = layers;
    #endif

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;


    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    VkResult result = vkCreateInstance(&createInfo, nullptr, instance);

    return result;
}