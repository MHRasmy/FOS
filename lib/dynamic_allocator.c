/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"

struct MemBlock_LIST blk_list;

#define FREE 1
#define NOT_FREE 0
uint32 MAX_HEAP_SIZE = 2*(1024*1024);
bool is_initialized = 0;
//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
uint32 get_block_size(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->size ;
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
int8 is_free_block(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->is_free ;
}

//===========================================
// 3) ALLOCATE BLOCK BASED ON GIVEN STRATEGY:
//===========================================
void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockMetaData* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", blk->size, blk->is_free) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//=========================================
	//DON'T CHANGE THESE LINES=================
	if (initSizeOfAllocatedSpace == 0)
		return ;
	is_initialized = 1;
	//=========================================
	//=========================================

	//TODO: [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator()
//	panic("initialize_dynamic_allocator is not implemented yet");
	//Initializing the block list
	LIST_INIT(&blk_list);

	//Initializing the meta data block at the given address
	struct BlockMetaData* blk = ((struct BlockMetaData *)daStart);

	//Allocating the gived size
	blk->size = initSizeOfAllocatedSpace;

	//Marking the block free
	blk->is_free = FREE;

	//Adding the block to the meta data list
	LIST_INSERT_TAIL(&blk_list, blk);

	//Getting the max heap size for alloc_bf
	MAX_HEAP_SIZE = initSizeOfAllocatedSpace;
}

//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size)
{
	//TODO: [PROJECT'23.MS1 - #6] [3] DYNAMIC ALLOCATOR - alloc_block_FF()
//	panic("alloc_block_FF is not implemented yet");

	//Handling wrong size
	if (size <= 0)
		return NULL;

	if (!is_initialized)
	{
		uint32 required_size = size + sizeOfMetaData();
		uint32 da_start = (uint32)sbrk(required_size);
		//get new break since it's page aligned! thus, the size can be more than the required one
		uint32 da_break = (uint32)sbrk(0);
		initialize_dynamic_allocator(da_start, da_break - da_start);
	}


	//Iterating to find the block
	struct BlockMetaData* blk ;
	LIST_FOREACH(blk, &blk_list)
	{
		//Checking the available size to be greater than or equal the required size
		uint32 found = (blk->size - sizeOfMetaData());
		if (blk->is_free == FREE && found >= size)
		{
			//Calculating the remaining free size
			uint32 freedSize = blk->size - size - sizeOfMetaData();

			//Adjusting the new allocated size and marking it to be allocated
			blk->size = size + sizeOfMetaData();
			blk->is_free = NOT_FREE;

			//The remaining size should be at least greater than or equal to the sizeOfMetaData
			//to avoid overwriting the next data block
			if (freedSize >= sizeOfMetaData())
			{
				//Allocating the free block to be in the next address
				struct BlockMetaData* freedBlk = (struct BlockMetaData*) ((uint32)(blk + 1) + size);

				//Adjusting its size and marking it to be free
				freedBlk->size = freedSize;
				freedBlk->is_free = FREE;

				//Inserting it after the allocated block
				LIST_INSERT_AFTER(&blk_list, blk, freedBlk);
			}
			else
				blk->size += freedSize;
			return (struct BlockMetaData*)(blk + 1);
		}
	}

	//Adjusting the system break if there is not data block available
	uint32 oldBrk = (uint32) sbrk(size + (uint32)sizeOfMetaData());
	if (oldBrk != -1)
	{
//		cprintf("\noldBrk: %d\n", oldBrk);
//		struct BlockMetaData* tail = LIST_LAST(&blk_list);
		//Adding it to the new address with its size and marking it to be allocated
//		struct BlockMetaData* allocatedBlk = (tail->is_free == NOT_FREE) ? (struct BlockMetaData*) oldBrk : tail;
		struct BlockMetaData* allocatedBlk = (struct BlockMetaData*) oldBrk;
		uint32 allocatedSize = (uint32)sbrk(0) - (uint32) oldBrk;

		uint32 freedSize = allocatedSize - size - sizeOfMetaData();
		allocatedBlk->size = size + sizeOfMetaData();
		allocatedBlk->is_free = NOT_FREE;
		//Inserting it at the end of the list
		LIST_INSERT_TAIL(&blk_list, allocatedBlk);

		//Allocating the free block to be in the next address
		struct BlockMetaData* freedBlk = (struct BlockMetaData*) ((uint32)(allocatedBlk + 1) + size);

		//Adjusting its size and marking it to be free
		freedBlk->size = freedSize;
		freedBlk->is_free = FREE;

		//Inserting it after the allocated block
		LIST_INSERT_AFTER(&blk_list, allocatedBlk, freedBlk);

		return (void*)(allocatedBlk + 1);
	}
	return NULL;
}
//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT'23.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF()
//	panic("alloc_block_BF is not implemented yet");
	//Handling wrong size
	if (size <= 0)
		return NULL;

	//Initialize the minimum size to the maximum allocated space
	uint32 min_size = MAX_HEAP_SIZE + 1;
	//Iterating to find the block
	struct BlockMetaData* blk = NULL, *idx = NULL;
	LIST_FOREACH(idx, &blk_list)
	{
		//Checking the available size to be greater than or equal the required size
		uint32 found = (idx->size - sizeOfMetaData());
		if (idx->is_free == NOT_FREE || found < size)
			continue;

		//Saving the minimum fit block
		if (found < min_size)
		{
			min_size = found;
			blk = idx;
		}
	}

	if (blk != NULL)
	{
		//Calculating the remaining free size
		uint32 freedSize = blk->size - size - sizeOfMetaData();

		//Adjusting the new allocated size and marking it to be allocated
		blk->size = size + sizeOfMetaData();
		blk->is_free = NOT_FREE;

		//The remaining size should be at least greater than or equal to the sizeOfMetaData
		//to avoid overwriting the next data block
		if (freedSize >= sizeOfMetaData())
		{
			//Allocating the free block to be in the next address
			struct BlockMetaData* freedBlk = (struct BlockMetaData*) ((uint32)(blk + 1) + size);

			//Adjusting its size and marking it to be free
			freedBlk->size = freedSize;
			freedBlk->is_free = FREE;

			//Inserting it after the allocated block
			LIST_INSERT_AFTER(&blk_list, blk, freedBlk);
		}
		return (struct BlockMetaData*)(blk + 1);
	}

	//Adjusting the system break if there is not data block available
	uint32 oldBrk = (uint32) sbrk(size + (uint32)sizeOfMetaData());
	if (oldBrk != -1)
	{
		//Adding it to the new address with its size and marking it to be allocated
		struct BlockMetaData* allocatedBlk = (struct BlockMetaData*) oldBrk;
		allocatedBlk->size = size + sizeOfMetaData();
		allocatedBlk->is_free = NOT_FREE;

		//Inserting it at the end of the list
		LIST_INSERT_TAIL(&blk_list, allocatedBlk);
		return (void*)(allocatedBlk + 1);
	}
	return NULL;
}

//=========================================
// [6] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}

//===================================================
// [8] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
	//TODO: [PROJECT'23.MS1 - #7] [3] DYNAMIC ALLOCATOR - free_block()
//	panic("free_block is not implemented yet");
	if (((struct BlockMetaData *)va) == NULL)
		return;
	struct BlockMetaData* blk = ((struct BlockMetaData *)va - 1);
	if (blk->is_free == FREE)
		return;
	blk->is_free = FREE;

	/** HANDLING THE NEXT BLOCK **/
	//getting the next block in the list
	struct BlockMetaData* nxt = LIST_NEXT(blk);

	//validating the next block
	if (nxt != NULL && nxt->is_free == FREE)
	{
		//getting the size of the next block
		uint32 nxt_size = nxt->size;

		//marking the next meta data to be 0
		nxt->size = 0;
		nxt->is_free = 0;

		//removing the next block
		LIST_REMOVE(&blk_list, nxt);

		//extending the current block size
		blk->size += nxt_size;
	}
	/** HANDLING THE PREVIOUS BLOCK **/
	//getting the previous block in the list
	struct BlockMetaData* prev = LIST_PREV(blk);
	//validating the previous block
	if (prev != NULL && prev->is_free == FREE)
	{
		//getting the size of the previous block
		uint32 curr_size = blk->size;

		//marking the current meta data to be 0
		blk->size = 0;
		blk->is_free = 0;

		//removing the current block
		LIST_REMOVE(&blk_list, blk);

		//extending the current block size
		prev->size += curr_size;
	}
}

//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	 //TODO: [PROJECT'23.MS1 - #8] [3] DYNAMIC ALLOCATOR - realloc_block_FF()
	  //panic("realloc_block_FF is not implemented yet");
	  struct BlockMetaData *curBlk = ((struct BlockMetaData *)va - 1);
	    if(va != NULL && new_size == 0){
	      free_block(va);
	      return NULL;
	    }
	    else if(va == NULL && new_size != 0){
	      return alloc_block_FF(new_size);
	    }
	    else if(va == NULL && new_size == 0){
	      return NULL;
	    }


	    uint32 normalSize = curBlk->size - sizeOfMetaData();

	    if(new_size > curBlk->size){
	      //cprintf("new_size > curBlk->size 1 /n ");
	      struct BlockMetaData* nxtBlk = (struct BlockMetaData*) (curBlk->prev_next_info.le_next);
	      if(nxtBlk != NULL && nxtBlk->is_free == 1 && nxtBlk->size + curBlk->size >= new_size)
	      {

	        if((nxtBlk->size + curBlk->size) == new_size){
	          curBlk->prev_next_info.le_next = nxtBlk->prev_next_info.le_next;
	          nxtBlk->prev_next_info.le_next->prev_next_info.le_prev = curBlk;
	          nxtBlk->is_free = 0;
	          nxtBlk->size = 0;
	          curBlk->size = new_size + sizeOfMetaData();
	          return (struct BlockMetaData*) curBlk + 1;
	        }
	        else{
	          nxtBlk->is_free = 0;
	          nxtBlk->size = 0 ;
	          nxtBlk =(struct BlockMetaData*)curBlk + curBlk->size;
	          uint32 freedSize = new_size - curBlk->size;
	          curBlk->size = new_size + sizeOfMetaData();
	          nxtBlk->size = freedSize;
	          nxtBlk->is_free = 1;
	          return (struct BlockMetaData*)(curBlk + 1);
	        }
	      }
	      else if(nxtBlk != NULL && nxtBlk->is_free == 0)
	      {
	        free_block(va);
	        alloc_block_FF(new_size);
	      }
	      }
	    else if(new_size < curBlk->size)
	    {
	      uint32 freedSize = curBlk->size - new_size - sizeOfMetaData();
	      curBlk->size = new_size + sizeOfMetaData();
	      curBlk->is_free = 0;
	      struct BlockMetaData* freedBlk = (struct BlockMetaData*)(struct BlockMetaData*) ((uint32)(curBlk + 1) + new_size);
	      freedBlk->size = freedSize;
	      freedBlk->is_free =1;
	      LIST_INSERT_AFTER(&blk_list, curBlk, freedBlk);
	      return (struct BlockMetaData*) curBlk + 1;
	    }
	    return NULL;

}
