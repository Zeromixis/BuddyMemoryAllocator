#pragma once

#include "Util.h"

namespace ZGE
{
    class BuddyMemoryAllocator
    {
        enum class NodeState
        {
            eNODE_UNUSE = 0,
            eNODE_USED,
            eNODE_OCCUPIED_TO_CHILD,
            eNODE_OCCUPIED_TO_PARENT,
        };

        static constexpr unsigned int AllocSizeExp = 3;

        static constexpr unsigned int AllocSize = GetPowof2 (AllocSizeExp);

        static constexpr unsigned int AllocUnitExp = 0;

        static constexpr unsigned int AllocUnit = GetPowof2 (AllocUnitExp);

        static constexpr unsigned int TreeDepth = AllocSizeExp;

    public:
        typedef unsigned char Byte;

        BuddyMemoryAllocator ();

        ~BuddyMemoryAllocator ();

        void *Malloc (unsigned int size);

        void Free (void *ptr);

    private:
        void MarkTree (const unsigned int &rootIndex, const NodeState &nodeState);

    private:
        Byte *m_MemAlloc;
        NodeState *m_NodeTree;

        unsigned int m_TreeLength;
    };
}