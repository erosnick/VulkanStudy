#pragma once

#include <vulkan/vulkan.h>

class Allocator
{
public:

	// Operator that allows an instance of this class to be used as a 
	// VkAllocationCallbacks structures
	inline operator VkAllocationCallbacks() const
	{
		VkAllocationCallbacks  result;
		result.pUserData = (void*)this;
		result.pfnAllocation = &Allocator::Allocation;
		result.pfnReallocation = &Allocator::Reallocation;
		result.pfnFree = &Allocator::Free;
		result.pfnInternalAllocation = &Allocator::InternalAllocationNotification;
		result.pfnInternalFree = &Allocator::InternalFreeNotification;
	}

private:

	// Declare the allocator callbacks as static member functions
	static void* VKAPI_CALL Allocation(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
	static void* VKAPI_CALL Reallocation(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
	static void VKAPI_CALL Free(void* pUserData, void* pMemory);

	static void VKAPI_CALL VKAPI_CALL InternalAllocationNotification(void* pUserData, size_t size, VkInternalAllocationType  allocationType, VkSystemAllocationScope allocationScope);
	static void VKAPI_CALL VKAPI_CALL InternalFreeNotification(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope);

	// Now declare the nonstatic member functions that will actually perform

	void* Allocation(size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
	void* Reallocation(void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
	void Free(void* pMemory);
};