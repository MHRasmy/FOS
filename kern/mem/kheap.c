#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

uint32 pagesToSize[NUM_OF_KHEAP_PAGES];

uint32 get_page_index(uint32 virtual_address)
{
	return (virtual_address - (hLimit + PAGE_SIZE))/ PAGE_SIZE;
}

int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//TODO: [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator()
	//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
	//All pages in the given range should be allocated
	//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
	//Return:
	//	On success: 0
	//	Otherwise (if no memory OR initial size exceed the given limit): E_NO_MEM

	//Comment the following line(s) before start coding...
//	panic("not implemented yet");
	initSizeToAllocate = ROUNDUP(initSizeToAllocate,PAGE_SIZE);
	start = daStart;
	brk = daStart +  initSizeToAllocate;
	hLimit = daLimit;
	if (start + initSizeToAllocate > daLimit)
		return E_NO_MEM;
	int numOfPages = initSizeToAllocate / PAGE_SIZE;

	uint32 PtrPage = daStart;
	for (uint32 i = daStart; i< brk; i += PAGE_SIZE){
		struct FrameInfo* PtrFrameInfo = NULL;
		  uint32 ret_alloc = allocate_frame(&PtrFrameInfo);
		  if (ret_alloc == 0){
			  PtrFrameInfo->va = i;
			  map_frame(ptr_page_directory,PtrFrameInfo,i, PERM_PRESENT | PERM_WRITEABLE );
		  }
		  else
			  return E_NO_MEM;
	}
	initialize_dynamic_allocator(start,initSizeToAllocate);
	return 0;
}

void* sbrk(int increment)
{
	//TODO: [PROJECT'23.MS2 - #02] [1] KERNEL HEAP - sbrk()
	/* increment > 0: move the segment break of the kernel to increase the size of its heap,
	 * 				you should allocate pages and map them into the kernel virtual address space as necessary,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * increment = 0: just return the current position of the segment break
	 * increment < 0: move the segment break of the kernel to decrease the size of its heap,
	 * 				you should deallocate pages that no longer contain part of the heap as necessary.
	 * 				and returns the address of the new break (i.e. the end of the current heap space).
	 *
	 * NOTES:
	 * 	1) You should only have to allocate or deallocate pages if the segment break crosses a page boundary
	 * 	2) New segment break should be aligned on page-boundary to avoid "No Man's Land" problem
	 * 	3) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, kernel should panic(...)
	 */

	//MS2: COMMENT THIS LINE BEFORE START CODING====
//	return (void*)-1 ;
//	panic("not implemented yet");

	uint32 ret = brk;

	if (increment == 0)
		return (void*)ret;
	else if (increment > 0)
	{
		//check if already allocated
		uint32 *ptr_page_table = NULL;
		uint32 new_brk = ROUNDDOWN(brk + increment, PAGE_SIZE);
		struct FrameInfo* fi = get_frame_info(ptr_page_directory, new_brk, &ptr_page_table);
		if (fi != 0)
		{
			//updating the break
			brk = ROUNDUP(brk + increment, PAGE_SIZE);

			//returning the old break
			return (void*)(ret);
		}

		if (ROUNDUP(brk + increment, PAGE_SIZE) > hLimit)
			panic("Exceeded the hard limit");

		uint32 limit = ROUNDUP(brk + increment, PAGE_SIZE);

		for (uint32 va = ROUNDUP(brk, PAGE_SIZE); va < limit; va+=PAGE_SIZE)
		{
			//Allocate a frame
			struct FrameInfo *ptr_frame_info = NULL;
			int ret = allocate_frame(&ptr_frame_info);
			if (ret != 0)
				panic("NO free frames");
			ptr_frame_info->va = va;

			//Mapping the frame to the virtual address
			map_frame(ptr_page_directory, ptr_frame_info, va, PERM_PRESENT|PERM_WRITEABLE);
		}

		//updating the break
		brk = limit;
	}
	else if (increment < 0)
	{
		//checking if the decrement is within the same block -> update the break & return
		//if not, then unmap the crossed pages
		uint32 curr = ROUNDUP(brk,PAGE_SIZE);
		uint32 new = ROUNDUP(brk + increment, PAGE_SIZE);
		uint32 noPages = (curr - new)/PAGE_SIZE;
		if (noPages == 0)
			return (void*)(brk += increment);

		//the limit, here, is the new break
		uint32 limit = brk + increment;
		for (uint32 va = ROUNDUP(brk, PAGE_SIZE) - PAGE_SIZE; va >= limit; va -= PAGE_SIZE)
		{
			uint32 *ptr_page_table = NULL;
			struct FrameInfo* ptr_frame_info = get_frame_info(ptr_page_directory,va, &ptr_page_table);
			if (ptr_frame_info != NULL)
			{
				ptr_frame_info->va = 0;
				free_frame(ptr_frame_info);
				unmap_frame(ptr_page_directory, va);
			}
		}
		//updating the break and the return value
		ret = brk = limit;
	}

	return (void*)ret;
}


void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'23.MS2 - #03] [1] KERNEL HEAP - kmalloc()
	//refer to the project presentation and documentation for details
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	//change this "return" according to your answer
//	kpanic_into_prompt("kmalloc() is not implemented yet...!!");
//	return NULL;
	int pageCnt = ROUNDUP(size,PAGE_SIZE) / PAGE_SIZE;

	uint32 ret = -1;
	//block allocation
	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE ){
		return alloc_block_FF(size);
	}
	//page allocation
	else
	{
		bool found = 0;
		int foundPages = 0;

		for (uint32 va = hLimit + PAGE_SIZE; va < KERNEL_HEAP_MAX; va += PAGE_SIZE)
		{
			uint32 *ptr_page_table = NULL;
			struct FrameInfo* fi = get_frame_info(ptr_page_directory, va, &ptr_page_table);

			if (!fi)
			{
				if (ret == -1)
				{
					ret = va;
				}
				foundPages++;

				if (foundPages == pageCnt)
				{
					found = 1;
					break;
				}
			}
			else
			{
				foundPages = 0;
				ret = -1;
			}
		}

		if(found == 1)
		{
			uint32 maxPages = ret + (pageCnt * PAGE_SIZE);
			for(uint32 va = ret; va < maxPages; va += PAGE_SIZE) {
				struct FrameInfo* fi = NULL;
				uint32 ret_alloc = allocate_frame(&fi);
				fi->va = va;
				map_frame(ptr_page_directory, fi, va, PERM_PRESENT | PERM_WRITEABLE );
			}

			int idx = get_page_index(ret);
			pagesToSize[idx] = pageCnt;

			return (void*)(ret);
		}
	}
	return NULL;
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
//	panic("kfree() is not implemented yet...!!");
	uint32 va = (uint32) virtual_address;
	if (va < start || (va >= hLimit && va < hLimit + PAGE_SIZE) || va >= KERNEL_HEAP_MAX)
		panic("wrong address passed to kfree()");
	if (va < hLimit)
	{
		free_block(virtual_address);
		return;
	}
	uint32 *ptr_page_table = NULL ;
	uint32 idx = get_page_index(va);
	uint32 noPages = pagesToSize[idx];
	for (uint32 i = 0; i < noPages; ++i)
	{
		struct FrameInfo *ptr_Frame_Info = get_frame_info(ptr_page_directory, va, &ptr_page_table) ;
		if (ptr_Frame_Info == NULL)
			panic("wrong pages to free");
		ptr_Frame_Info->va = 0;
		free_frame(ptr_Frame_Info);
		unmap_frame(ptr_page_directory, va);
		va += PAGE_SIZE;
	}
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
//	panic("kheap_virtual_address() is not implemented yet...!!");

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

	struct FrameInfo* frame_info = to_frame_info(physical_address);
	if (frame_info->references == 0)
		return 0;

	uint32 va = (frame_info->va) & 0xFFFFF000;
	uint32 offset = (physical_address << 20) >> 20;
	va += offset;

	if (va < start || va > KERNEL_HEAP_MAX)
		return 0;

	return va;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #06] [1] KERNEL HEAP - kheap_physical_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
//	panic("kheap_physical_address() is not implemented yet...!!");

	//change this "return" according to your answer
	uint32 va = (uint32) virtual_address;
	if (va < start || (va >= hLimit && va < hLimit + PAGE_SIZE) || va >= KERNEL_HEAP_MAX)
		return 0;

	uint32 tableIdx = PTX(va);

	uint32 *ptr_page_table = NULL ;
	get_page_table(ptr_page_directory, va, &ptr_page_table) ;
	if (ptr_page_table) {
		uint32 pa = ptr_page_table[tableIdx];
		pa &= 0xFFFFF000;
		uint32 offset = (va << 20) >> 20;
		return pa + offset;
	}
	return 0;
}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc()
	// Write your code here, remove the panic and write your code
//	return NULL;
//	panic("krealloc() is not implemented yet...!!");
	if (virtual_address == NULL){
		return kmalloc(new_size);
	}

	if (new_size == 0) {
		kfree(virtual_address);
		return (void *) NULL;
	}

	//large size
	if (new_size > 134193152){
		return (void*)NULL;
	}

	uint32 va = (uint32) virtual_address;
	if (va < start || (va >= hLimit && va < hLimit + PAGE_SIZE) || va > KERNEL_HEAP_MAX)
	{
		return (void*)NULL;
	}

	if (va < hLimit)
	{
		return realloc_block_FF(virtual_address,new_size);
	}

	uint32 *ptr_page_table = NULL ;
	uint32 idx = get_page_index(va);
	uint32 noPages = pagesToSize[idx];
	uint32 currSize = noPages * PAGE_SIZE;

	uint32 ret = -1;

	uint32 *ptr_table = NULL;
	uint32 pageAdd;
	int noOfValidPages = 0;
	struct FrameInfo* ptr_frame_info;
	if(new_size > currSize)
	{

		int dif = new_size - currSize;
		uint32 neededPages = ROUNDUP(dif,PAGE_SIZE) / PAGE_SIZE;
		bool isValidNext = 0 ;
		pageAdd = va + currSize;
		while (pageAdd < KERNEL_HEAP_MAX)
		{
			if (noOfValidPages == neededPages){
				isValidNext = 1;
				break;
			}
			ptr_frame_info = get_frame_info(ptr_page_directory,pageAdd,&ptr_table);
			if (ptr_frame_info == NULL)
			{
				if (ret == -1)
					ret = pageAdd;
				noOfValidPages += 1;
			}
			else
			{

				break;
			}
			pageAdd += PAGE_SIZE;
		}
		if(isValidNext)
		{
			for(uint32 i=ret ; i< ret+(neededPages*PAGE_SIZE);i+=PAGE_SIZE){
				struct FrameInfo* ptr_frame_info = NULL;
				uint32 ret_alloc = allocate_frame(&ptr_frame_info);
				ptr_frame_info->va = i;
				map_frame(ptr_page_directory,ptr_frame_info,i, PERM_PRESENT | PERM_WRITEABLE );
			}

			int idx1 = get_page_index(va);
			pagesToSize[idx1]+=neededPages;

			return (void*)(va);
		}
		else
		{
			kfree(virtual_address);
			return kmalloc(new_size);
		}
	}
	else if (new_size < currSize)
	{
		uint32 endAdd = va + currSize;
		uint32 neededPages = ROUNDUP(new_size,PAGE_SIZE) / PAGE_SIZE;
		uint32 remainPages = noPages - neededPages;
		uint32 startAdd = va + (neededPages * PAGE_SIZE);

		while(remainPages--)
		{
			struct FrameInfo *ptr_Frame_Info = get_frame_info(ptr_page_directory, startAdd, &ptr_page_table) ;
			if (ptr_Frame_Info == NULL)
				panic("wrong pages to free");
			ptr_Frame_Info->va = 0;
			free_frame(ptr_Frame_Info);
			unmap_frame(ptr_page_directory, startAdd);
			startAdd += PAGE_SIZE;
		}
		return (void*)(va);
	}

	return NULL;
}
