#ifndef __OBJPOOL_H__
#define __OBJPOOL_H__

#include <stdint.h>
#include <string.h>
#include <new>
#include <assert.h>
#include <stdio.h>

/// object pool, prealloc max_alloc_count_ objects of type T
/// T MUST ALIGNED at 8 for performance
template<typename T>
class objpool 
{
public:
	objpool (size_t max_alloc_count)
	: allocated_obj_count_ (0), max_alloc_count_ (max_alloc_count)
	{
		static_assert (sizeof (T) % 8 == 0, "sizeof T % 8 != 0");
		/// more alloc 1 for reserve, for safe.
		size_t alloc_size = (max_alloc_count_ + 1) * sizeof (T);
		ptrunk_ = new char[alloc_size];
		memset (ptrunk_, 0, alloc_size);
		pobj_ = reinterpret_cast<T*> (ptrunk_);
		for (size_t i = 0; i < max_alloc_count_; ++i) {
			/// alloc in a trunk, free big trunk is ok.
			new (ptrunk_ + i * sizeof (T))T;
		}

		allocated_obj_count_ = 0;
	}

	~objpool ()
	{
		delete [] ptrunk_;
		ptrunk_ = NULL;
		pobj_ = NULL;
		allocated_obj_count_ = 0;
		max_alloc_count_ = 0;
	}

	T* alloc ()
	{
		if (allocated_obj_count_ < max_alloc_count_) {
			return &pobj_[allocated_obj_count_++];
		}

		return NULL;
	}

	bool empty ()
	{
		return allocated_obj_count_ >= max_alloc_count_;
	}

	size_t free_count ()
	{
		return max_alloc_count_ - allocated_obj_count_;
	}

	size_t allocated_count ()
	{
		return allocated_obj_count_;
	}

	void clear ()
	{
		allocated_obj_count_ = 0;
	}

	void dist_clear ()
	{
		size_t alloc_size = (max_alloc_count_ + 1) * sizeof (T);
		memset (ptrunk_, 0, alloc_size);
		allocated_obj_count_ = 0;
	}

private:
	char* ptrunk_;
	T* pobj_;
	size_t allocated_obj_count_;
	size_t max_alloc_count_;
};

#endif
