#include <inc/lib.h>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
int pagesToSizeUser[NUM_OF_UHEAP_PAGES] =  {0};

uint32 get_page_index(uint32 virtual_address)
{
	uint32 hardLimit = sys_get_hard_limit();
	return (virtual_address - (hardLimit + PAGE_SIZE))/ PAGE_SIZE;
}

int FirstTimeFlag = 1;
void InitializeUHeap()
{
	if(FirstTimeFlag)
	{
#if UHP_USE_BUDDY
		initialize_buddy();
		cprintf("BUDDY SYSTEM IS INITIALIZED\n");
#endif
		FirstTimeFlag = 0;
	}
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'23.MS2 - #09] [2] USER HEAP - malloc() [User Side]
	// Write your code here, remove the panic and write your code
//	panic("malloc() is not implemented yet...!!");
//	return NULL;
	uint32 hLimit = sys_get_hard_limit();

	uint32 ret = -1;
	int noOfPages = ROUNDUP(size,PAGE_SIZE) / PAGE_SIZE;
	uint32 pageAdd = hLimit + PAGE_SIZE;

	//large size
	if (size > (NUM_OF_UHEAP_PAGES * PAGE_SIZE)){
		return NULL;
	}

	//block allocation
	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE ){
		return alloc_block_FF(size);
	}

	//page allocation
	else
	{
		bool isValidPages = 0;
		int idx = get_page_index(pageAdd);

		while (pageAdd < USER_HEAP_MAX)
		{

			idx = get_page_index(pageAdd);
			int  ptr_frame_info = pagesToSizeUser[idx];

			if (ptr_frame_info == 0)
			{
				if(pageAdd + (PAGE_SIZE*noOfPages) <= USER_HEAP_MAX)
				{
					isValidPages = 1 ;
					ret = pageAdd;
					pagesToSizeUser[idx] = noOfPages;
					break;

				}
				else
				{
					break;
				}
			}
			pageAdd += (PAGE_SIZE*ptr_frame_info);
		}
		if(isValidPages)
		{
			sys_allocate_user_mem(pageAdd,size);
			return (void*)(ret);
		}
	}
	return NULL;
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and	sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy

}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #11] [2] USER HEAP - free() [User Side]
	// Write your code here, remove the panic and write your code
//	panic("free() is not implemented yet...!!");

	//Getting the hard limit
	uint32 hardLimit = sys_get_hard_limit();

	//Checking for wrong virtual address
	uint32 va = (uint32) virtual_address;
	if (va < USER_HEAP_START || (va >= hardLimit && va < hardLimit + PAGE_SIZE) || va >= USER_HEAP_MAX)
		panic("wrong address passed to free()");
	if (va < hardLimit)
	{
		free_block(virtual_address);
		return;
	}

	uint32 idx = get_page_index(va);
	uint32 noPages = pagesToSizeUser[idx];
	uint32 curSize = noPages * PAGE_SIZE;
	pagesToSizeUser[idx] = 0;
	sys_free_user_mem(va,curSize);
}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	panic("smalloc() is not implemented yet...!!");
	return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");
	return NULL;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================

	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
