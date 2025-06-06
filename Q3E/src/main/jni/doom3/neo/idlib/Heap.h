/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __HEAP_H__
#define __HEAP_H__

/*
===============================================================================

	Memory Management

	This is a replacement for the compiler heap code (i.e. "C" malloc() and
	free() calls). On average 2.5-3.0 times faster than MSVC malloc()/free().
	Worst case performance is 1.65 times faster and best case > 70 times.

===============================================================================
*/

#ifdef _RAVEN
// RAVEN BEGIN
// amccarthy:  tags for memory allocation tracking.  When updating this list please update the
// list of discriptions in Heap.cpp as well.
typedef enum {
	MA_NONE = 0,	
	
	MA_OPNEW,
	MA_DEFAULT,
	MA_LEXER,
	MA_PARSER,
	MA_AAS,
	MA_CLASS,
	MA_SCRIPT,
	MA_CM,
	MA_CVAR,
	MA_DECL,
	MA_FILESYS,
	MA_IMAGES,
	MA_MATERIAL,
	MA_MODEL,
	MA_FONT,
	MA_RENDER,
	MA_VERTEX,
	MA_SOUND,
	MA_WINDOW,
	MA_EVENT,
	MA_MATH,
	MA_ANIM,
	MA_DYNAMICBLOCK,
	MA_STRING,
	MA_GUI,
	MA_EFFECT,
	MA_ENTITY,
	MA_PHYSICS,
	MA_AI,
	MA_NETWORK,

	MA_DO_NOT_USE,		// neither of the two remaining enumerated values should be used (no use of MA_DO_NOT_USE prevents the second dword in a memory block from getting the value 0xFFFFFFFF)
	MA_MAX				// <- this enumerated value is a count and cannot exceed 32 (5 bits are used to encode tag within memory block with rvHeap.cpp)
} Mem_Alloc_Types_t;
// jnewquist: memory tag stack for new/delete
#if (defined(_DEBUG) || defined(_RV_MEM_SYS_SUPPORT)) && !defined(ENABLE_INTEL_SMP)
class MemScopedTag {
	byte mTag;
	MemScopedTag *mPrev;
	static MemScopedTag *mTop;
public:
	MemScopedTag( byte tag ) {
		mTag = tag;
		mPrev = mTop;
		mTop = this;
	}
	~MemScopedTag() {
		assert( mTop != NULL );
		mTop = mTop->mPrev;
	}
	void SetTag( byte tag ) {
		mTag = tag;
	}
	static byte GetTopTag( void ) {
		if ( mTop ) {
			return mTop->mTag;
		} else {
			return MA_OPNEW;
		}
	}
};
#define MemScopedTag_GetTopTag() MemScopedTag::GetTopTag()
#define MEM_SCOPED_TAG(var, tag) MemScopedTag var(tag)
#define MEM_SCOPED_TAG_SET(var, tag) var.SetTag(tag)
#else
#define MemScopedTag_GetTopTag() MA_OPNEW
#define MEM_SCOPED_TAG(var, tag)
#define MEM_SCOPED_TAG_SET(var, tag)
#endif
#endif

// RAVEN END

typedef struct {
	int		num;
	int		minSize;
	int		maxSize;
	int		totalSize;
} memoryStats_t;


void		Mem_Init(void);
void		Mem_Shutdown(void);
void		Mem_EnableLeakTest(const char *name);
void		Mem_ClearFrameStats(void);
void		Mem_GetFrameStats(memoryStats_t &allocs, memoryStats_t &frees);
void		Mem_GetStats(memoryStats_t &stats);
void		Mem_Dump_f(const class idCmdArgs &args);
void		Mem_DumpCompressed_f(const class idCmdArgs &args);
void		Mem_AllocDefragBlock(void);


#ifndef ID_DEBUG_MEMORY

void 		*Mem_Alloc(const int size);
void 		*Mem_ClearedAlloc(const int size);
void		Mem_Free(void *ptr);
char 		*Mem_CopyString(const char *in);
void 		*Mem_Alloc16(const int size);
void		Mem_Free16(void *ptr);
#ifdef _RAVEN // unused tag
ID_INLINE void 		*Mem_Alloc(const int size, byte tag) { (void)tag; return Mem_Alloc(size); }
ID_INLINE void 		*Mem_ClearedAlloc(const int size, byte tag) { (void)tag; return Mem_ClearedAlloc(size); }
ID_INLINE void 		*Mem_Alloc16(const int size, byte tag) { (void)tag; return Mem_Alloc16(size); }
#endif

#ifdef ID_REDIRECT_NEWDELETE

__inline void *operator new(size_t s)
{
	return Mem_Alloc(s);
}
__inline void operator delete(void *p)
{
	Mem_Free(p);
}
__inline void *operator new[](size_t s)
{
	return Mem_Alloc(s);
}
__inline void operator delete[](void *p)
{
	Mem_Free(p);
}

#endif

#else /* ID_DEBUG_MEMORY */

void 		*Mem_Alloc(const int size, const char *fileName, const int lineNumber);
void 		*Mem_ClearedAlloc(const int size, const char *fileName, const int lineNumber);
void		Mem_Free(void *ptr, const char *fileName, const int lineNumber);
char 		*Mem_CopyString(const char *in, const char *fileName, const int lineNumber);
void 		*Mem_Alloc16(const int size, const char *fileName, const int lineNumber);
void		Mem_Free16(void *ptr, const char *fileName, const int lineNumber);

#ifdef ID_REDIRECT_NEWDELETE

__inline void *operator new(size_t s, int t1, int t2, char *fileName, int lineNumber)
{
	return Mem_Alloc(s, fileName, lineNumber);
}
__inline void operator delete(void *p, int t1, int t2, char *fileName, int lineNumber)
{
	Mem_Free(p, fileName, lineNumber);
}
__inline void *operator new[](size_t s, int t1, int t2, char *fileName, int lineNumber)
{
	return Mem_Alloc(s, fileName, lineNumber);
}
__inline void operator delete[](void *p, int t1, int t2, char *fileName, int lineNumber)
{
	Mem_Free(p, fileName, lineNumber);
}
__inline void *operator new(size_t s)
{
	return Mem_Alloc(s, "", 0);
}
__inline void operator delete(void *p)
{
	Mem_Free(p, "", 0);
}
__inline void *operator new[](size_t s)
{
	return Mem_Alloc(s, "", 0);
}
__inline void operator delete[](void *p)
{
	Mem_Free(p, "", 0);
}

#define ID_DEBUG_NEW						new( 0, 0, __FILE__, __LINE__ )
#undef new
#define new									ID_DEBUG_NEW

#endif

#define		Mem_Alloc( size )				Mem_Alloc( size, __FILE__, __LINE__ )
#define		Mem_ClearedAlloc( size )		Mem_ClearedAlloc( size, __FILE__, __LINE__ )
#define		Mem_Free( ptr )					Mem_Free( ptr, __FILE__, __LINE__ )
#define		Mem_CopyString( s )				Mem_CopyString( s, __FILE__, __LINE__ )
#define		Mem_Alloc16( size )				Mem_Alloc16( size, __FILE__, __LINE__ )
#define		Mem_Free16( ptr )				Mem_Free16( ptr, __FILE__, __LINE__ )

#endif /* ID_DEBUG_MEMORY */


// allocate SIZE bytes, aligned to 16 bytes - possibly on the stack (like _alloca16())
// if it's too big (> ID_MAX_ALLOCA_SIZE, 1MB), it gets allocated on the Heap instead.
// ON_STACK should be a bool and will be set to true if it was allocated on the stack
// and false if it was allocated on the heap.
// if ON_STACK is false, you must free this with Mem_FreeA() or Mem_Free16()!
// (just pass your ON_STACK bool to Mem_FreeA() and it will do the right thing)
#define Mem_MallocA( SIZE, ON_STACK ) \
	( (SIZE) < ID_MAX_ALLOCA_SIZE ? ( ON_STACK=true, _alloca16(SIZE) ) : ( ON_STACK=false, Mem_Alloc16(SIZE) ) )

// free memory allocated with Mem_MallocA()
ID_INLINE void Mem_FreeA( void* ptr, bool onStack )
{
    if( !onStack )
    {
        Mem_Free16( ptr );
    }
}


/*
===============================================================================

	Block based allocator for fixed size objects.

	All objects of the 'type' are properly constructed.
	However, the constructor is not called for re-used objects.

===============================================================================
*/

#ifdef _RAVEN
template<class type, int blockSize, byte memoryTag = MA_NONE>
#else
template<class type, int blockSize>
#endif
class idBlockAlloc
{
	public:
		idBlockAlloc(void);
		~idBlockAlloc(void);

		void					Shutdown(void);

		type 					*Alloc(void);
		void					Free(type *element);

		int						GetTotalCount(void) const {
			return total;
		}
		int						GetAllocCount(void) const {
			return active;
		}
		int						GetFreeCount(void) const {
			return total - active;
		}

	private:
		typedef struct element_s {
			type				t;
			struct element_s 	*next;
		} element_t;
		typedef struct block_s {
			element_t			elements[blockSize];
			struct block_s 	*next;
		} block_t;

		block_t 				*blocks;
		element_t 				*free;
		int						total;
		int						active;
};

#ifdef _RAVEN
template<class type, int blockSize, byte memoryTag>
idBlockAlloc<type,blockSize, memoryTag>::idBlockAlloc(void)
#else
template<class type, int blockSize>
idBlockAlloc<type,blockSize>::idBlockAlloc(void)
#endif
{
	blocks = NULL;
	free = NULL;
	total = active = 0;
}

#ifdef _RAVEN
template<class type, int blockSize, byte memoryTag>
idBlockAlloc<type,blockSize, memoryTag>::~idBlockAlloc(void)
#else
template<class type, int blockSize>
idBlockAlloc<type,blockSize>::~idBlockAlloc(void)
#endif
{
	Shutdown();
}

#ifdef _RAVEN
template<class type, int blockSize, byte memoryTag>
type *idBlockAlloc<type,blockSize, memoryTag>::Alloc(void)
#else
template<class type, int blockSize>
type *idBlockAlloc<type,blockSize>::Alloc(void)
#endif
{
	if (!free) {
		block_t *block = new block_t;
		block->next = blocks;
		blocks = block;

		for (int i = 0; i < blockSize; i++) {
			block->elements[i].next = free;
			free = &block->elements[i];
		}

		total += blockSize;
	}

	active++;
	element_t *element = free;
	free = free->next;
	element->next = NULL;
	return &element->t;
}

#ifdef _RAVEN
template<class type, int blockSize, byte memoryTag>
void idBlockAlloc<type,blockSize, memoryTag>::Free(type *t)
#else
template<class type, int blockSize>
void idBlockAlloc<type,blockSize>::Free(type *t)
#endif
{
	element_t *element = (element_t *) t;
	element->next = free;
	free = element;
	active--;
}

#ifdef _RAVEN
template<class type, int blockSize, byte memoryTag>
void idBlockAlloc<type,blockSize, memoryTag>::Shutdown(void)
#else
template<class type, int blockSize>
void idBlockAlloc<type,blockSize>::Shutdown(void)
#endif
{
	while (blocks) {
		block_t *block = blocks;
		blocks = blocks->next;
		delete block;
	}

	blocks = NULL;
	free = NULL;
	total = active = 0;
}

/*
==============================================================================

	Dynamic allocator, simple wrapper for normal allocations which can
	be interchanged with idDynamicBlockAlloc.

	No constructor is called for the 'type'.
	Allocated blocks are always 16 byte aligned.

==============================================================================
*/

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag = MA_NONE>
#else
template<class type, int baseBlockSize, int minBlockSize>
#endif
class idDynamicAlloc
{
	public:
		idDynamicAlloc(void);
		~idDynamicAlloc(void);

		void							Init(void);
		void							Shutdown(void);
		void							SetFixedBlocks(int numBlocks) {}
		void							SetLockMemory(bool lock) {}
		void							FreeEmptyBaseBlocks(void) {}

		type 							*Alloc(const int num);
		type 							*Resize(type *ptr, const int num);
		void							Free(type *ptr);
		const char 					*CheckMemory(const type *ptr) const;

		int								GetNumBaseBlocks(void) const {
			return 0;
		}
		int								GetBaseBlockMemory(void) const {
			return 0;
		}
		int								GetNumUsedBlocks(void) const {
			return numUsedBlocks;
		}
		int								GetUsedBlockMemory(void) const {
			return usedBlockMemory;
		}
		int								GetNumFreeBlocks(void) const {
			return 0;
		}
		int								GetFreeBlockMemory(void) const {
			return 0;
		}
		int								GetNumEmptyBaseBlocks(void) const {
			return 0;
		}

	private:
		int								numUsedBlocks;			// number of used blocks
		int								usedBlockMemory;		// total memory in used blocks

		int								numAllocs;
		int								numResizes;
		int								numFrees;

		void							Clear(void);
};

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
idDynamicAlloc<type, baseBlockSize, minBlockSize, memoryTag>::idDynamicAlloc(void)
#else
template<class type, int baseBlockSize, int minBlockSize>
idDynamicAlloc<type, baseBlockSize, minBlockSize>::idDynamicAlloc(void)
#endif
{
	Clear();
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
idDynamicAlloc<type, baseBlockSize, minBlockSize, memoryTag>::~idDynamicAlloc(void)
#else
template<class type, int baseBlockSize, int minBlockSize>
idDynamicAlloc<type, baseBlockSize, minBlockSize>::~idDynamicAlloc(void)
#endif
{
	Shutdown();
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
void idDynamicAlloc<type, baseBlockSize, minBlockSize, memoryTag>::Init(void)
#else
template<class type, int baseBlockSize, int minBlockSize>
void idDynamicAlloc<type, baseBlockSize, minBlockSize>::Init(void)
#endif
{
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
void idDynamicAlloc<type, baseBlockSize, minBlockSize, memoryTag>::Shutdown(void)
#else
template<class type, int baseBlockSize, int minBlockSize>
void idDynamicAlloc<type, baseBlockSize, minBlockSize>::Shutdown(void)
#endif
{
	Clear();
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
type *idDynamicAlloc<type, baseBlockSize, minBlockSize, memoryTag>::Alloc(const int num)
#else
template<class type, int baseBlockSize, int minBlockSize>
type *idDynamicAlloc<type, baseBlockSize, minBlockSize>::Alloc(const int num)
#endif
{
	numAllocs++;

	if (num <= 0) {
		return NULL;
	}

	numUsedBlocks++;
	usedBlockMemory += num * sizeof(type);
	return Mem_Alloc16(num * sizeof(type));
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
type *idDynamicAlloc<type, baseBlockSize, minBlockSize, memoryTag>::Resize(type *ptr, const int num)
#else
template<class type, int baseBlockSize, int minBlockSize>
type *idDynamicAlloc<type, baseBlockSize, minBlockSize>::Resize(type *ptr, const int num)
#endif
{

	numResizes++;

	if (ptr == NULL) {
		return Alloc(num);
	}

	if (num <= 0) {
		Free(ptr);
		return NULL;
	}

	assert(0);
	return ptr;
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
void idDynamicAlloc<type, baseBlockSize, minBlockSize, memoryTag>::Free(type *ptr)
#else
template<class type, int baseBlockSize, int minBlockSize>
void idDynamicAlloc<type, baseBlockSize, minBlockSize>::Free(type *ptr)
#endif
{
	numFrees++;

	if (ptr == NULL) {
		return;
	}

	Mem_Free16(ptr);
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
const char *idDynamicAlloc<type, baseBlockSize, minBlockSize, memoryTag>::CheckMemory(const type *ptr) const
#else
template<class type, int baseBlockSize, int minBlockSize>
const char *idDynamicAlloc<type, baseBlockSize, minBlockSize>::CheckMemory(const type *ptr) const
#endif
{
	return NULL;
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
void idDynamicAlloc<type, baseBlockSize, minBlockSize, memoryTag>::Clear(void)
#else
template<class type, int baseBlockSize, int minBlockSize>
void idDynamicAlloc<type, baseBlockSize, minBlockSize>::Clear(void)
#endif
{
	numUsedBlocks = 0;
	usedBlockMemory = 0;
	numAllocs = 0;
	numResizes = 0;
	numFrees = 0;
}


/*
==============================================================================

	Fast dynamic block allocator.

	No constructor is called for the 'type'.
	Allocated blocks are always 16 byte aligned.

==============================================================================
*/

#include "containers/BTree.h"

//#define DYNAMIC_BLOCK_ALLOC_CHECK

template<class type>
class idDynamicBlock
{
	public:
		type 							*GetMemory(void) const {
			return (type *)(((byte *) this) + sizeof(idDynamicBlock<type>));
		}
		int								GetSize(void) const {
			return abs(size);
		}
		void							SetSize(int s, bool isBaseBlock) {
			size = isBaseBlock ? -s : s;
		}
		bool							IsBaseBlock(void) const {
			return (size < 0);
		}

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
		int								id[3];
		void 							*allocator;
#endif

		int								size;					// size in bytes of the block
		idDynamicBlock<type> *			prev;					// previous memory block
		idDynamicBlock<type> *			next;					// next memory block
		idBTreeNode<idDynamicBlock<type>,int> *node;			// node in the B-Tree with free blocks
};

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag = MA_NONE>
#else
template<class type, int baseBlockSize, int minBlockSize>
#endif
class idDynamicBlockAlloc
{
	public:
		idDynamicBlockAlloc(void);
		~idDynamicBlockAlloc(void);

		void							Init(void);
		void							Shutdown(void);
		void							SetFixedBlocks(int numBlocks);
		void							SetLockMemory(bool lock);
		void							FreeEmptyBaseBlocks(void);

		type 							*Alloc(const int num);
		type 							*Resize(type *ptr, const int num);
		void							Free(type *ptr);
		const char 					*CheckMemory(const type *ptr) const;

		int								GetNumBaseBlocks(void) const {
			return numBaseBlocks;
		}
		int								GetBaseBlockMemory(void) const {
			return baseBlockMemory;
		}
		int								GetNumUsedBlocks(void) const {
			return numUsedBlocks;
		}
		int								GetUsedBlockMemory(void) const {
			return usedBlockMemory;
		}
		int								GetNumFreeBlocks(void) const {
			return numFreeBlocks;
		}
		int								GetFreeBlockMemory(void) const {
			return freeBlockMemory;
		}
		int								GetNumEmptyBaseBlocks(void) const;

	private:
		idDynamicBlock<type> *			firstBlock;				// first block in list in order of increasing address
		idDynamicBlock<type> *			lastBlock;				// last block in list in order of increasing address
		idBTree<idDynamicBlock<type>,int,4>freeTree;			// B-Tree with free memory blocks
		bool							allowAllocs;			// allow base block allocations
		bool							lockMemory;				// lock memory so it cannot get swapped out

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
		int								blockId[3];
#endif

		int								numBaseBlocks;			// number of base blocks
		int								baseBlockMemory;		// total memory in base blocks
		int								numUsedBlocks;			// number of used blocks
		int								usedBlockMemory;		// total memory in used blocks
		int								numFreeBlocks;			// number of free blocks
		int								freeBlockMemory;		// total memory in free blocks

		int								numAllocs;
		int								numResizes;
		int								numFrees;

		void							Clear(void);
		idDynamicBlock<type> *			AllocInternal(const int num);
		idDynamicBlock<type> *			ResizeInternal(idDynamicBlock<type> *block, const int num);
		void							FreeInternal(idDynamicBlock<type> *block);
		void							LinkFreeInternal(idDynamicBlock<type> *block);
		void							UnlinkFreeInternal(idDynamicBlock<type> *block);
		void							CheckMemory(void) const;
};

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::idDynamicBlockAlloc(void)
#else
template<class type, int baseBlockSize, int minBlockSize>
idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::idDynamicBlockAlloc(void)
#endif
{
	Clear();
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::~idDynamicBlockAlloc(void)
#else
template<class type, int baseBlockSize, int minBlockSize>
idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::~idDynamicBlockAlloc(void)
#endif
{
	Shutdown();
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::Init(void)
#else
template<class type, int baseBlockSize, int minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::Init(void)
#endif
{
	freeTree.Init();
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::Shutdown(void)
#else
template<class type, int baseBlockSize, int minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::Shutdown(void)
#endif
{
	idDynamicBlock<type> *block;

	for (block = firstBlock; block != NULL; block = block->next) {
		if (block->node == NULL) {
			FreeInternal(block);
		}
	}

	for (block = firstBlock; block != NULL; block = firstBlock) {
		firstBlock = block->next;
		assert(block->IsBaseBlock());

		if (lockMemory) {
			idLib::sys->UnlockMemory(block, block->GetSize() + (int)sizeof(idDynamicBlock<type>));
		}

		Mem_Free16(block);
	}

	freeTree.Shutdown();

	Clear();
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::SetFixedBlocks(int numBlocks)
#else
template<class type, int baseBlockSize, int minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::SetFixedBlocks(int numBlocks)
#endif
{
	idDynamicBlock<type> *block;

	for (int i = numBaseBlocks; i < numBlocks; i++) {
		block = (idDynamicBlock<type> *) Mem_Alloc16(baseBlockSize);

		if (lockMemory) {
			idLib::sys->LockMemory(block, baseBlockSize);
		}

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
		memcpy(block->id, blockId, sizeof(block->id));
		block->allocator = (void *)this;
#endif
		block->SetSize(baseBlockSize - (int)sizeof(idDynamicBlock<type>), true);
		block->next = NULL;
		block->prev = lastBlock;

		if (lastBlock) {
			lastBlock->next = block;
		} else {
			firstBlock = block;
		}

		lastBlock = block;
		block->node = NULL;

		FreeInternal(block);

		numBaseBlocks++;
		baseBlockMemory += baseBlockSize;
	}

	allowAllocs = false;
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::SetLockMemory(bool lock)
#else
template<class type, int baseBlockSize, int minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::SetLockMemory(bool lock)
#endif
{
	lockMemory = lock;
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::FreeEmptyBaseBlocks(void)
#else
template<class type, int baseBlockSize, int minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::FreeEmptyBaseBlocks(void)
#endif
{
	idDynamicBlock<type> *block, *next;

	for (block = firstBlock; block != NULL; block = next) {
		next = block->next;

		if (block->IsBaseBlock() && block->node != NULL && (next == NULL || next->IsBaseBlock())) {
			UnlinkFreeInternal(block);

			if (block->prev) {
				block->prev->next = block->next;
			} else {
				firstBlock = block->next;
			}

			if (block->next) {
				block->next->prev = block->prev;
			} else {
				lastBlock = block->prev;
			}

			if (lockMemory) {
				idLib::sys->UnlockMemory(block, block->GetSize() + (int)sizeof(idDynamicBlock<type>));
			}

			numBaseBlocks--;
			baseBlockMemory -= block->GetSize() + (int)sizeof(idDynamicBlock<type>);
			Mem_Free16(block);
		}
	}

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	CheckMemory();
#endif
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
int idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::GetNumEmptyBaseBlocks(void) const
#else
template<class type, int baseBlockSize, int minBlockSize>
int idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::GetNumEmptyBaseBlocks(void) const
#endif
{
	int numEmptyBaseBlocks;
	idDynamicBlock<type> *block;

	numEmptyBaseBlocks = 0;

	for (block = firstBlock; block != NULL; block = block->next) {
		if (block->IsBaseBlock() && block->node != NULL && (block->next == NULL || block->next->IsBaseBlock())) {
			numEmptyBaseBlocks++;
		}
	}

	return numEmptyBaseBlocks;
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
type *idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::Alloc(const int num)
#else
template<class type, int baseBlockSize, int minBlockSize>
type *idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::Alloc(const int num)
#endif
{
	idDynamicBlock<type> *block;

	numAllocs++;

	if (num <= 0) {
		return NULL;
	}

	block = AllocInternal(num);

	if (block == NULL) {
		return NULL;
	}

	block = ResizeInternal(block, num);

	if (block == NULL) {
		return NULL;
	}

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	CheckMemory();
#endif

	numUsedBlocks++;
	usedBlockMemory += block->GetSize();

	return block->GetMemory();
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
type *idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::Resize(type *ptr, const int num)
#else
template<class type, int baseBlockSize, int minBlockSize>
type *idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::Resize(type *ptr, const int num)
#endif
{

	numResizes++;

	if (ptr == NULL) {
		return Alloc(num);
	}

	if (num <= 0) {
		Free(ptr);
		return NULL;
	}

	idDynamicBlock<type> *block = (idDynamicBlock<type> *)(((byte *) ptr) - (int)sizeof(idDynamicBlock<type>));

	usedBlockMemory -= block->GetSize();

	block = ResizeInternal(block, num);

	if (block == NULL) {
		return NULL;
	}

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	CheckMemory();
#endif

	usedBlockMemory += block->GetSize();

	return block->GetMemory();
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::Free(type *ptr)
#else
template<class type, int baseBlockSize, int minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::Free(type *ptr)
#endif
{

	numFrees++;

	if (ptr == NULL) {
		return;
	}

	idDynamicBlock<type> *block = (idDynamicBlock<type> *)(((byte *) ptr) - (int)sizeof(idDynamicBlock<type>));

	numUsedBlocks--;
	usedBlockMemory -= block->GetSize();

	FreeInternal(block);

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	CheckMemory();
#endif
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
const char *idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::CheckMemory(const type *ptr) const
#else
template<class type, int baseBlockSize, int minBlockSize>
const char *idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::CheckMemory(const type *ptr) const
#endif
{
	idDynamicBlock<type> *block;

	if (ptr == NULL) {
		return NULL;
	}

	block = (idDynamicBlock<type> *)(((byte *) ptr) - (int)sizeof(idDynamicBlock<type>));

	if (block->node != NULL) {
		return "memory has been freed";
	}

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK

	if (block->id[0] != 0x11111111 || block->id[1] != 0x22222222 || block->id[2] != 0x33333333) {
		return "memory has invalid id";
	}

	if (block->allocator != (void *)this) {
		return "memory was allocated with different allocator";
	}

#endif

	/* base blocks can be larger than baseBlockSize which can cause this code to fail
	idDynamicBlock<type> *base;
	for ( base = firstBlock; base != NULL; base = base->next ) {
		if ( base->IsBaseBlock() ) {
			if ( ((int)block) >= ((int)base) && ((int)block) < ((int)base) + baseBlockSize ) {
				break;
			}
		}
	}
	if ( base == NULL ) {
		return "no base block found for memory";
	}
	*/

	return NULL;
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::Clear(void)
#else
template<class type, int baseBlockSize, int minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::Clear(void)
#endif
{
	firstBlock = lastBlock = NULL;
	allowAllocs = true;
	lockMemory = false;
	numBaseBlocks = 0;
	baseBlockMemory = 0;
	numUsedBlocks = 0;
	usedBlockMemory = 0;
	numFreeBlocks = 0;
	freeBlockMemory = 0;
	numAllocs = 0;
	numResizes = 0;
	numFrees = 0;

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	blockId[0] = 0x11111111;
	blockId[1] = 0x22222222;
	blockId[2] = 0x33333333;
#endif
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
idDynamicBlock<type> *idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::AllocInternal(const int num)
#else
template<class type, int baseBlockSize, int minBlockSize>
idDynamicBlock<type> *idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::AllocInternal(const int num)
#endif
{
	idDynamicBlock<type> *block;
	int alignedBytes = (num * sizeof(type) + 15) & ~15;

	block = freeTree.FindSmallestLargerEqual(alignedBytes);

	if (block != NULL) {
		UnlinkFreeInternal(block);
	} else if (allowAllocs) {
		int allocSize = Max(baseBlockSize, alignedBytes + (int)sizeof(idDynamicBlock<type>));
		block = (idDynamicBlock<type> *) Mem_Alloc16(allocSize);

		if (lockMemory) {
			idLib::sys->LockMemory(block, baseBlockSize);
		}

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
		memcpy(block->id, blockId, sizeof(block->id));
		block->allocator = (void *)this;
#endif
		block->SetSize(allocSize - (int)sizeof(idDynamicBlock<type>), true);
		block->next = NULL;
		block->prev = lastBlock;

		if (lastBlock) {
			lastBlock->next = block;
		} else {
			firstBlock = block;
		}

		lastBlock = block;
		block->node = NULL;

		numBaseBlocks++;
		baseBlockMemory += allocSize;
	}

	return block;
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
idDynamicBlock<type> *idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::ResizeInternal(idDynamicBlock<type> *block, const int num)
#else
template<class type, int baseBlockSize, int minBlockSize>
idDynamicBlock<type> *idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::ResizeInternal(idDynamicBlock<type> *block, const int num)
#endif
{
	int alignedBytes = (num * sizeof(type) + 15) & ~15;

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	assert(block->id[0] == 0x11111111 && block->id[1] == 0x22222222 && block->id[2] == 0x33333333 && block->allocator == (void *)this);
#endif

	// if the new size is larger
	if (alignedBytes > block->GetSize()) {

		idDynamicBlock<type> *nextBlock = block->next;

		// try to annexate the next block if it's free
		if (nextBlock && !nextBlock->IsBaseBlock() && nextBlock->node != NULL &&
		    block->GetSize() + (int)sizeof(idDynamicBlock<type>) + nextBlock->GetSize() >= alignedBytes) {

			UnlinkFreeInternal(nextBlock);
			block->SetSize(block->GetSize() + (int)sizeof(idDynamicBlock<type>) + nextBlock->GetSize(), block->IsBaseBlock());
			block->next = nextBlock->next;

			if (nextBlock->next) {
				nextBlock->next->prev = block;
			} else {
				lastBlock = block;
			}
		} else {
			// allocate a new block and copy
			idDynamicBlock<type> *oldBlock = block;
			block = AllocInternal(num);

			if (block == NULL) {
				return NULL;
			}

			memcpy(block->GetMemory(), oldBlock->GetMemory(), oldBlock->GetSize());
			FreeInternal(oldBlock);
		}
	}

	// if the unused space at the end of this block is large enough to hold a block with at least one element
	if (block->GetSize() - alignedBytes - (int)sizeof(idDynamicBlock<type>) < Max(minBlockSize, (int)sizeof(type))) {
		return block;
	}

	idDynamicBlock<type> *newBlock;

	newBlock = (idDynamicBlock<type> *)(((byte *) block) + (int)sizeof(idDynamicBlock<type>) + alignedBytes);
#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	memcpy(newBlock->id, blockId, sizeof(newBlock->id));
	newBlock->allocator = (void *)this;
#endif
	newBlock->SetSize(block->GetSize() - alignedBytes - (int)sizeof(idDynamicBlock<type>), false);
	newBlock->next = block->next;
	newBlock->prev = block;

	if (newBlock->next) {
		newBlock->next->prev = newBlock;
	} else {
		lastBlock = newBlock;
	}

	newBlock->node = NULL;
	block->next = newBlock;
	block->SetSize(alignedBytes, block->IsBaseBlock());

	FreeInternal(newBlock);

	return block;
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::FreeInternal(idDynamicBlock<type> *block)
#else
template<class type, int baseBlockSize, int minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::FreeInternal(idDynamicBlock<type> *block)
#endif
{

	assert(block->node == NULL);

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	assert(block->id[0] == 0x11111111 && block->id[1] == 0x22222222 && block->id[2] == 0x33333333 && block->allocator == (void *)this);
#endif

	// try to merge with a next free block
	idDynamicBlock<type> *nextBlock = block->next;

	if (nextBlock && !nextBlock->IsBaseBlock() && nextBlock->node != NULL) {
		UnlinkFreeInternal(nextBlock);
		block->SetSize(block->GetSize() + (int)sizeof(idDynamicBlock<type>) + nextBlock->GetSize(), block->IsBaseBlock());
		block->next = nextBlock->next;

		if (nextBlock->next) {
			nextBlock->next->prev = block;
		} else {
			lastBlock = block;
		}
	}

	// try to merge with a previous free block
	idDynamicBlock<type> *prevBlock = block->prev;

	if (prevBlock && !block->IsBaseBlock() && prevBlock->node != NULL) {
		UnlinkFreeInternal(prevBlock);
		prevBlock->SetSize(prevBlock->GetSize() + (int)sizeof(idDynamicBlock<type>) + block->GetSize(), prevBlock->IsBaseBlock());
		prevBlock->next = block->next;

		if (block->next) {
			block->next->prev = prevBlock;
		} else {
			lastBlock = prevBlock;
		}

		LinkFreeInternal(prevBlock);
	} else {
		LinkFreeInternal(block);
	}
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
ID_INLINE void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::LinkFreeInternal(idDynamicBlock<type> *block)
#else
template<class type, int baseBlockSize, int minBlockSize>
ID_INLINE void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::LinkFreeInternal(idDynamicBlock<type> *block)
#endif
{
	block->node = freeTree.Add(block, block->GetSize());
	numFreeBlocks++;
	freeBlockMemory += block->GetSize();
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
ID_INLINE void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::UnlinkFreeInternal(idDynamicBlock<type> *block)
#else
template<class type, int baseBlockSize, int minBlockSize>
ID_INLINE void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::UnlinkFreeInternal(idDynamicBlock<type> *block)
#endif
{
	freeTree.Remove(block->node);
	block->node = NULL;
	numFreeBlocks--;
	freeBlockMemory -= block->GetSize();
}

#ifdef _RAVEN
template<class type, int baseBlockSize, int minBlockSize, byte memoryTag>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize, memoryTag>::CheckMemory(void) const
#else
template<class type, int baseBlockSize, int minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::CheckMemory(void) const
#endif
{
	idDynamicBlock<type> *block;

	for (block = firstBlock; block != NULL; block = block->next) {
		// make sure the block is properly linked
		if (block->prev == NULL) {
			assert(firstBlock == block);
		} else {
			assert(block->prev->next == block);
		}

		if (block->next == NULL) {
			assert(lastBlock == block);
		} else {
			assert(block->next->prev == block);
		}
	}
}

#endif /* !__HEAP_H__ */
