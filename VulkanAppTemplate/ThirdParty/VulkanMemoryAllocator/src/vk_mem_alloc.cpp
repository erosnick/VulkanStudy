#include <volk/volk.h>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#define VMA_IMPLEMENTATION 1
#include <vk_mem_alloc.h>

void VmaDedicatedAllocationList::PrintAllocationList()
{
	if (!m_AllocationList.IsEmpty())
	{
		VmaMutexLockRead lock(m_Mutex, m_UseMutex);
		for (VmaAllocation alloc = m_AllocationList.Front();
			alloc != VMA_NULL; alloc = m_AllocationList.GetNext(alloc))
		{
			if (alloc->GetName() != nullptr)
			{
				printf("[Leaked]%s\n", alloc->GetName());
			}
		}
	}
}