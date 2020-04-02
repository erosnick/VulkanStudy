#include "Allocator.h"
#include <malloc.h>

void* VKAPI_CALL Allocator::Allocation(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return static_cast<Allocator*>(pUserData)->Allocation(size, alignment, allocationScope);
}

void* VKAPI_CALL Allocator::Reallocation(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return static_cast<Allocator*>(pUserData)->Reallocation(pOriginal, size, alignment, allocationScope);
}

void VKAPI_CALL Allocator::Free(void* pUserData, void* pMemory)
{
	static_cast<Allocator*>(pUserData)->Free(pMemory);
}

void VKAPI_CALL VKAPI_CALL Allocator::InternalAllocationNotification(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{

}

void VKAPI_CALL VKAPI_CALL Allocator::InternalFreeNotification(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{

}

void* Allocator::Allocation(size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return _aligned_malloc(size, alignment);
}

void* Allocator::Reallocation(void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return _aligned_realloc(pOriginal, size, alignment);
}

void Allocator::Free(void* pMemory)
{
	_aligned_free(pMemory);
}
