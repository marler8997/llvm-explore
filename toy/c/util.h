#ifndef MORE_UTIL_H
#define MORE_UTIL_H

#include <stdio.h>

#define DeclareSizedArray(type,name)			\
  typedef struct {					\
    type* ptr;						\
    size_t length;					\
  } name;						\
  name name##_make(type* ptr, size_t length);		\

#define DefineSizedArray(type,name)			\
  name name##_make(type* ptr, size_t length)		\
  {							\
    name a;						\
    a.ptr = ptr;					\
    a.length = length;					\
    return a;						\
  }							\
  

#define DeclareAppender(type,name)					\
  typedef struct {							\
    type* ptr;								\
    size_t length;   /* current element count */			\
    size_t capacity; /* element capacity */				\
  } name;								\
  void name##_init(name* this, size_t capacity);			\
  void name##_free(name* this);						\
  void name##_clear(name* this);					\
  void name##_ensureCanAppend(name* this, size_t elementCount);		\
  void name##_append(name* this, type v);				\

#define DefineAppender(type,name)					\
  void name##_init(name* this, size_t capacity)				\
  {									\
    this->ptr = (type*)malloc(sizeof(type)*capacity);			\
    this->capacity = capacity;						\
    this->length = 0;							\
  }									\
  void name##_free(name* this)						\
  {									\
    free(this->ptr);							\
  }									\
  void name##_clear(name* this)						\
  {									\
    this->length = 0;							\
  }									\
  void name##_ensureCanAppend(name* this, size_t elementCount)		\
  {									\
    if(elementCount > this->capacity - this->length) {			\
      size_t newCapacity = this->length + elementCount;			\
      if(newCapacity < 2*this->capacity)				\
	newCapacity = 2*this->capacity;					\
									\
      this->ptr = (type*)realloc(this->ptr,				\
				    sizeof(type) * newCapacity);	\
      this->capacity = newCapacity;					\
    }									\
  }									\
  void name##_append(name* this, type v)				\
  {									\
    name##_ensureCanAppend(this, 1);					\
    this->ptr[this->length] = v;				        \
    this->length++;							\
  }									\

#endif
