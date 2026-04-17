#include "shader_module.h"

VkResult createShaderModule(VkDevice* logicalDevice, VkShaderModule* shaderModule, std::string path){
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if(file.fail()){
        return VK_ERROR_INITIALIZATION_FAILED;
    }
	size_t fileSize = file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
    file.close();

    VkShaderModuleCreateInfo shaderModuleCreateInfo {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = fileSize;
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    VkResult createShaderModuleResult = vkCreateShaderModule(*logicalDevice, &shaderModuleCreateInfo, nullptr, shaderModule);

    return createShaderModuleResult;
}

