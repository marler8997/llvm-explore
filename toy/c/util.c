#include "util.h"

#ifdef TEST_MORE_UTIL

DeclareAppender(char,CharAppender);
DeclareAppender(int,IntAppender);

void printCharAppender(const char* prefix, CharAppender* a)
{
  printf("%s(length=%d,capcity=%d,ptr=%p) '%.*s'\n", prefix,
	 a->length, a->capacity, a->ptr, a->length, a->ptr);
}
void printIntAppender(const char* prefix, IntAppender* a)
{
  size_t i;
  printf("%s(length=%d,capcity=%d,ptr=%p) [", prefix,
	 a->length, a->capacity, a->ptr);
  for(i = 0; i < a->length; i++) {
    printf("%s%d", (i==0)?"":",", a->ptr[i]);
  }
  printf("]\n");
}

int main(int args, char* argv[])
{
  CharAppender a;

  CharAppender_init(&a, 1);
  printCharAppender("init()     :", &a);

  CharAppender_append(&a, 'a');
  printCharAppender("append('a'):", &a);

  CharAppender_append(&a, 'b');
  printCharAppender("append('b'):", &a);

  CharAppender_append(&a, 'c');
  printCharAppender("append('c'):", &a);

  CharAppender_clear(&a);
  printCharAppender("clar()     :", &a);

  CharAppender_append(&a, 'a');
  printCharAppender("append('a'):", &a);

  CharAppender_append(&a, 'b');
  printCharAppender("append('b'):", &a);

  {
    IntAppender ia;

    printf("IntAppender test\n");
    printf("-----------------------------\n");

    IntAppender_init(&ia, 1);
    printIntAppender("init()     :", &ia);

    IntAppender_append(&ia, 9);
    printIntAppender("append(9)  :", &ia);

    IntAppender_append(&ia, 255);
    printIntAppender("append(255):", &ia);

    IntAppender_append(&ia, 43);
    printIntAppender("append(43) :", &ia);
    
  }


  return 0;
}

#endif
