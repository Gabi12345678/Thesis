#ifndef __BASEDEF_H__
#define __BASEDEF_H__

#include "allocator.h"
#include "exceptions.h"


#ifdef WIN32
#if _MSC_VER >= 1600
/* VS2010 and higher has stdint.h definition file */
#include <stdint.h>
#else
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif
#endif

namespace McoSql
{

	class MCOSQL_API DynamicObject
    {
      public:
        void* operator new(size_t size, Allocator* allocator) { 
            return allocator->allocate(size);
        }
    
        void* operator new(size_t fixedSize, size_t varyingPart, Allocator* allocator)
        {
            return allocator->allocate(fixedSize + varyingPart);
        }
    
        void operator delete(void*) {}
        void operator delete(void* ptr, Allocator*) {}
        void operator delete(void* ptr, size_t size, Allocator* allocator) {}
    
        virtual void destroy(Allocator* allocator) = 0; 

        virtual ~DynamicObject() {}
    };
    
    #define DELETE_OBJ(allocator,obj) if ((obj) != NULL) (obj)->destroy(allocator)
    #define DESTROY(CLASS) void destroy(McoSql::Allocator* allocator) { this->~CLASS(); allocator->free(this, sizeof(CLASS)); }

/**
 * Resource which can be released using release() method. Actually this class just contains reference to allocator, allowing to destruct it without specifying allocator
 */
    class MCOSQL_API Resource : public DynamicObject
    {
    public:
        Allocator* const allocator;

        Resource(Allocator* _allocator) : allocator(_allocator) {}

        void release() {
            destroy(allocator);
        }
    };

    class Finally
    {
    private:
        Resource* resource;
    public:
        Finally(Resource* res): resource(res) {}

        void defuse() { 
            resource = NULL;
        }

        void replace(Resource* res) { 
            resource = res;
        }

        ~Finally() { 
            if (resource) { 
                resource->release();
            }
        }
    };


    /**
     * Iterator interface
     * This is a one-way iterator without rewind.
     * When iterator is created, current position is before first record.
     * Invoke <code>Iterator.next()</code> to get first record
     */
    template<class T>
    class Iterator : public Resource
    {
    public:
        /**
         * Check if more elements are available.
         * @return <code>true</code> if more elements are available
         * @deteriorated this method is obsolete now and is provided onl for ResultSet cursor. 
         * Instead you should check for value returned by next() method.
         */
        virtual bool hasNext() {
            MCO_THROW InvalidOperation("Iterator.hasNext is not implemented");
        }
        virtual T* next() = 0;                              
    
        Iterator(Allocator* allocator) : Resource(allocator) {}
    };

    template<class T>
    class Vector : public DynamicObject
    {
    public:
        size_t const length;
        T* items[MCO_VAR_SIZE_ARRAY];

        T*& at(size_t index) { 
            if (index > length) { 
                MCO_THROW IndexOutOfBounds(index, length);
            }
            return items[index];
        }

        static Vector* create(Allocator* allocator, size_t size, T** data) 
        {
            return new (size*sizeof(T*), allocator) Vector(size, data);
        }

        static Vector* create(Allocator* allocator, size_t size) 
        {
            return new (size*sizeof(T*), allocator) Vector(size);
        }

        virtual void destroy(Allocator* allocator) 
        {
            for (size_t i = 0; i < length; i++) { 
                DELETE_OBJ(allocator, items[i]); 
            }
            allocator->free(this, sizeof(*this) + length*sizeof(T*));
        }
        
        void copy(Vector* src, size_t offs = 0) 
        { 
            size_t n = length - offs < src->length ? length - offs : src->length;
            for (size_t i = 0; i < n; i++) { 
                items[offs + i] = src->items[i];
            }
        }

        void move(Vector* src, size_t offs = 0) 
        { 
            size_t n = length - offs < src->length ? length - offs : src->length;
            for (size_t i = 0; i < n; i++) { 
                items[offs + i] = src->items[i];
                src->items[i] = NULL; // prevent deallocation
            }
        }

        Vector* remove(Allocator* allocator, size_t pos) 
        {
            size_t i, n = length-1;
            Vector* newVec = create(allocator, n);
            for (i = 0; i < pos; i++)  {
                newVec->items[i] = items[i];
                items[i] = NULL;
            }
            while (++i <= n) { 
                newVec->items[i] = items[i-1];
                items[i-1] = NULL;
            }
            DELETE_OBJ(allocator, this);
            return newVec;
        }

        Vector* resize(Allocator* allocator, size_t newSize) 
        {
            Vector* newVec = create(allocator, newSize);
            newVec->clear();
            newVec->move(this);
            DELETE_OBJ(allocator, this);
            return newVec;
        }

        Vector* clone(Allocator* allocator) 
        {
            Vector* copy = create(allocator, length);
            copy->copy(this);
            return copy;
        }

        

        void clear() 
        { 
            memset(items, 0, length*sizeof(T*));
        }

        Vector(size_t size) : length(size)  {}

        Vector(size_t size, T** data) : length(size) 
        {
            for (size_t i = 0; i < length; i++) { 
                items[i] = data[i];
            }
        }
    };
}
#endif
