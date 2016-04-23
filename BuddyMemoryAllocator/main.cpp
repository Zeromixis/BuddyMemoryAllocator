// BuddyMemoryAllocator.cpp : Defines the entry point for the console application.
//

#include "BuddyMemoryAllocator.h"

int main()
{
    using namespace ZGE;
    BuddyMemoryAllocator myAllocator;

    void *mem = myAllocator.Malloc (2);

    void *mem2 = myAllocator.Malloc (3);

    void *mem3 = myAllocator.Malloc (4);

    myAllocator.Free (mem);
    myAllocator.Free (mem2);
    int i = 0;
    return 0;
}

