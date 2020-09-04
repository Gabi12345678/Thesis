#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

#include "config.h"
#include "exceptions.h"

// Malloc error checking level
// 0: no checking (no overhead)
// 1: check that size specified in free is correct (additional 8 bytes per object overhead)
// 2: detect out-of-bounds access: additional 8k per block overhead + mprotect cost
#ifndef MCO_SQL_DEBUG_ALLOCATOR
#define MCO_SQL_DEBUG_ALLOCATOR 0
#endif


// Check that allocator is accessed by single thread.
#ifndef MCO_CFG_CHECK_ALLOCATOR_USAGE
#define MCO_CFG_CHECK_ALLOCATOR_USAGE MCO_SQL_DEBUG_ALLOCATOR
#endif

#if MCO_SQL_DEBUG_ALLOCATOR >= 2
#include <unistd.h>
#include <sys/mman.h>
#endif

#if MCO_SQL_DEBUG_ALLOCATOR >= 1 || MCO_CFG_CHECK_ALLOCATOR_USAGE
#include "mcosmp.h"
#endif

namespace McoSql
{
	class Mutex;

    class MCOSQL_API SystemAllocator
    {
    public:
        virtual void* allocate(size_t size) = 0;
        virtual void free(void* ptr) = 0;

        virtual size_t allocated() {
            return 0;
        }

        SystemAllocator() {
            implementation = this;
        }

        static SystemAllocator* implementation;
    };

	class MCOSQL_API Allocator
    {
		struct MCOSQL_API Block
        {
            Block* next;
            Block* prev;

            void link(Block* block) {
                block->next = next;
                block->prev = this;
                next = next->prev = block;
            }

            void unlink() {
                next->prev = prev;
                prev->next = next;
            }

            void prune() {
                next = prev = this;
            }

            bool isEmpty() {
                return next == this;
            }

            Block* pop() {
                Block* block = next;
                block->unlink();
                return block;
            }

            void concat(Block& list)  {
                list.prev->next = this;
                prev->next = list.next;
                list.next->prev = prev;
                prev = list.prev;
                list.prune();
            }

            void free(SystemAllocator* sysAlloc) {
                Block *curr, *succ;
                for (curr = next; curr != this; curr = succ) {
                    #if MCO_SQL_DEBUG_ALLOCATOR >= 2
                    size_t size = *(size_t*)(curr+1);
                    if (size <= maxBlockObjectSize) {
                        size = blockSize;
                    }
                    size_t pageSize = getpagesize();
                    mprotect((void*)(((size_t)(curr+1) + size + pageSize - 1) & ~(pageSize-1)), pageSize, PROT_READ|PROT_WRITE);
                    #endif
                    succ = curr->next;
                    sysAlloc->free(curr);
                }
                prune();
            }

            Block() {
                prune();
            }
        };

        enum {
            nChains = 128,
            blockSize = 64*1024,
            minObjectSizeLog = 4,
            maxObjectSizeLog = 30,

            minObjectSize = 1 << minObjectSizeLog,
            maxBlockObjectSize = minObjectSize * nChains,

            maxReservedSize = 64*1024*1024
        };

        struct DebugHeader {
            size_t size;
            size_t reserved; // preserve 16-byte alignment at 64-bit platform
        };

        SystemAllocator* sysAlloc;
        Block* chains[nChains];
        Block* blocks[nChains];
        size_t used[nChains];
        Block  blobs;
        Block  freeBlocks;
        Block  usedBlocks;
        Block* freeList;
        Allocator* parent;

        void init()
        {
            for (size_t i = 0; i < nChains; i++) {
                chains[i] = NULL;
                blocks[i] = NULL;
                used[i] = blockSize;
            }
            allocated = 0;
            max_allocated = 0;
            reserved = 0;
            blob_reserved = 0;
            nSmallObjects = 0;
            nLargeObjects = 0;
        }

        void updateMaxAllocated(size_t fromChild)
        {
            if (allocated + fromChild > max_allocated) {
                max_allocated = allocated + fromChild;
                if (parent) parent->updateMaxAllocated(max_allocated);
            }
        }


    public:
        size_t allocated;
        size_t max_allocated;
        size_t reserved;
        size_t blob_reserved;
        size_t nSmallObjects;
        size_t nLargeObjects;
#if MCO_CFG_CHECK_ALLOCATOR_USAGE
        mco_process_t owner;
#endif

        void setSystemAllocator(SystemAllocator* alloc)
        {
            sysAlloc = alloc;
        }

		virtual bool isThreadSafe()
		{
			return false;
		}

        virtual void* allocate(size_t size);
        virtual void free(void* ptr, size_t size);
		virtual void clean();
        virtual void reset();

        #if MCO_CFG_CHECK_ALLOCATOR_USAGE
        mco_process_t grab() {
            mco_process_t original = owner;
            owner = mco_get_thread_id();
            return original;
        }
        void release(mco_process_t original) {
            owner = original;
        }
        #endif


        Allocator(Allocator* super = NULL, SystemAllocator* alloc = SystemAllocator::implementation)
        : sysAlloc(alloc), parent(super)
        {
            freeList = parent ? &parent->freeBlocks : &freeBlocks;
            #if MCO_CFG_CHECK_ALLOCATOR_USAGE
            owner = mco_get_thread_id();
            #endif
            init();
        }

        virtual ~Allocator()
        {
            clean();
            freeBlocks.free(sysAlloc);
        }
    };

    class AllocatorContext
    {
#if MCO_CFG_CHECK_ALLOCATOR_USAGE
      private:
        Allocator* allocator;
	    mco_process_t owner;
      public:
        AllocatorContext(Allocator* alloc) : allocator(alloc), owner(alloc->grab()) {}
        ~AllocatorContext() {
            allocator->release(owner);
        }
#else
      public:
        AllocatorContext(Allocator*) {}
#endif
    };

	class MCOSQL_API ThreadSafeAllocator : public Allocator
	{
		Mutex *mutex;

	  public:
        static ThreadSafeAllocator global;

		ThreadSafeAllocator(Allocator* super = NULL, SystemAllocator* alloc = SystemAllocator::implementation);
		~ThreadSafeAllocator();

		virtual bool isThreadSafe()
		{
			return true;
		}

        virtual void* allocate(size_t size);
		virtual void  free(void* ptr, size_t size);
	};


    class MCOSQL_API StdAllocator : public SystemAllocator
    {
      public:
        typedef void* (*malloc_t)(size_t size);
        typedef void  (*free_t)(void* ptr);

        static StdAllocator instance;

        malloc_t malloc_func;
        free_t free_func;

        void* allocate(size_t size)
        {
            return malloc_func(size);
        }

        void free(void* ptr)
        {
            free_func(ptr);
        }


        StdAllocator(malloc_t m = &::malloc, free_t f = &::free) : malloc_func(m), free_func(f) {}
    };
}
#endif
