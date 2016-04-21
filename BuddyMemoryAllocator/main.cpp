// BuddyMemoryAllocator.cpp : Defines the entry point for the console application.
//

#include "BuddyMemoryAllocator.h"

int main()
{
    using namespace ZGE;
    BuddyMemoryAllocator myAllocator;

    void *mem = myAllocator.Malloc (2);

    myAllocator.Free (mem);
    return 0;
}

