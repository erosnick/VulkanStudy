#pragma once

#include <vulkan/vulkan_core.h>

class DebugUtil
{
public:
	DebugUtil() = default;
	DebugUtil(VkDevice inDevice)
	: device(inDevice)
	{}

	void setup(VkDevice inDevice)
	{
		device = inDevice;
	}

	void setObjectName(const uint64_t object, const std::string& name, VkObjectType type)
	{
#ifdef DEBUG
		VkDebugUtilsObjectNameInfoEXT s{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT, nullptr, type, object, name.c_str() };
		vkSetDebugUtilsObjectNameEXT(device, &s);
#endif // DEBUG
	}

	// clang-format off
	void setObjectName(VkBuffer object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_BUFFER); }
	void setObjectName(VkBufferView object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_BUFFER_VIEW); }
	void setObjectName(VkCommandBuffer object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_COMMAND_BUFFER); }
	void setObjectName(VkCommandPool object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_COMMAND_POOL); }
	void setObjectName(VkDescriptorPool object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DESCRIPTOR_POOL); }
	void setObjectName(VkDescriptorSet object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DESCRIPTOR_SET); }
	void setObjectName(VkDescriptorSetLayout object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT); }
	void setObjectName(VkDevice object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DEVICE); }
	void setObjectName(VkDeviceMemory object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DEVICE_MEMORY); }
	void setObjectName(VkFramebuffer object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_FRAMEBUFFER); }
	void setObjectName(VkImage object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_IMAGE); }
	void setObjectName(VkImageView object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_IMAGE_VIEW); }
	void setObjectName(VkPipeline object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_PIPELINE); }
	void setObjectName(VkPipelineLayout object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_PIPELINE_LAYOUT); }
	void setObjectName(VkQueryPool object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_QUERY_POOL); }
	void setObjectName(VkQueue object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_QUEUE); }
	void setObjectName(VkRenderPass object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_RENDER_PASS); }
	void setObjectName(VkSampler object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_SAMPLER); }
	void setObjectName(VkSemaphore object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_SEMAPHORE); }
	void setObjectName(VkShaderModule object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_SHADER_MODULE); }
	void setObjectName(VkSwapchainKHR object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_SWAPCHAIN_KHR); }

private:
	VkDevice device;
};