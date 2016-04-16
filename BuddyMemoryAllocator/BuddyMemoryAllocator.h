#pragma once

#include "Util.h"

namespace ZGE
{
    class BuddyMemory
    {
    public:


    private:
        unsigned int m_TreeIndex;
    };

    class BuddyMemoryAllocator
    {

        enum class NodeState
        {
            eNODE_UNUSE = 0,
            eNODE_USED,
            eNODE_OCCUPIED_TO_CHILD,
            eNODE_OCCUPIED_TO_PARENT,
        };

        // 1024 Byte Alloc Memory
        static constexpr int MaxDepth = 10;

        // 1024
        static constexpr int AllocSize = GetPowof2 ( MaxDepth );

        // 4 Byte Per Unit, so it has 1024 / 4 = 256 Units
        static constexpr int AllocUnit = 4;

    public:
        typedef unsigned char Byte;

        BuddyMemoryAllocator ();

        ~BuddyMemoryAllocator ();

        void *Malloc ( unsigned int size );

        void Free ( void *ptr );

    private:
        void MarkTree ( const unsigned int &rootIndex, const NodeState &nodeState );

    private:
        Byte *m_MemAlloc;
        NodeState *m_NodeTree;

        unsigned int m_TreeLength;

    };
}