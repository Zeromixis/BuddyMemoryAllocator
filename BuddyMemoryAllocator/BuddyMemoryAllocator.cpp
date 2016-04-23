#include <stack>

#include <vcruntime_string.h>
#include <math.h>
#include "BuddyMemoryAllocator.h"

namespace ZGE
{
    BuddyMemoryAllocator::BuddyMemoryAllocator ()
    {
        m_MemAlloc = new Byte [GetPowof2 (AllocSizeExp)];
        m_NodeTree = new NodeState [GetPowof2 (AllocSizeExp - GetExpof2 (AllocUnit) + 1)];
        m_TreeLength = GetPowof2 (AllocSizeExp - GetExpof2 (AllocUnit) + 1) - 1;

        MarkTree (0, NodeState::eNODE_UNUSE);
    }

    BuddyMemoryAllocator::~BuddyMemoryAllocator ()
    {
        delete m_MemAlloc;
        delete m_NodeTree;
    }

    void * BuddyMemoryAllocator::Malloc (unsigned int allocSize)
    {
        if (allocSize == 0)
        {
            assert (false);
            return nullptr;
        }

        if (GetPowof2 (AllocSizeExp) < allocSize)
        {
            // OVER LIMIT!
            assert (false);
            return nullptr;
        }

        unsigned int searchDepth = 0;
        if (allocSize <= AllocUnit)
        {
            searchDepth = AllocSizeExp;
        }
        else
        {
            unsigned int value = GetNextPowof2 (allocSize);
            searchDepth = AllocSizeExp - value;
        }

        // In this Depth, Search which node can be use
        unsigned int beginIndex = (1 << searchDepth) - 1;
        unsigned int endIndex = (1 << (searchDepth + 1)) - 2;

        unsigned int searchIndex = 0;

        for (searchIndex = beginIndex; searchIndex <= endIndex; ++searchIndex)
        {
            NodeState *nodeState = m_NodeTree + searchIndex;
            if (*nodeState == NodeState::eNODE_UNUSE)
            {
                break;
            }
            else
            {
                continue;
            }
        }

        if (searchIndex > endIndex)
        {
            // Can't Alloc.
            return nullptr;
        }
        else
        {
            // Make this node used, all children Occupied to parent, all parent occupied to children.

            // Children Part.
            MarkTree (searchIndex, NodeState::eNODE_OCCUPIED_TO_PARENT);

            // Root Part.
            m_NodeTree [searchIndex] = NodeState::eNODE_USED;

            // Parent Part.
            auto parentIndex = searchIndex;
            while (true)
            {
                if (parentIndex == 0)
                {
                    *(m_NodeTree + 0) = NodeState::eNODE_OCCUPIED_TO_CHILD;
                    break;
                }
                else
                {
                    parentIndex = GetParentNodeIndex (parentIndex);
                    *(m_NodeTree + parentIndex) = NodeState::eNODE_OCCUPIED_TO_CHILD;
                }
            }
        }

        unsigned int unitSizeofThisDepth = (unsigned int)(GetPowof2 (AllocSizeExp) / std::pow (2, searchDepth));
        void *retAddr = m_MemAlloc;
        long retAddrValue = (long)retAddr;
        retAddrValue += (searchIndex - beginIndex) * unitSizeofThisDepth;
        retAddr = (void *)(retAddrValue);
        return retAddr;
    }

    void BuddyMemoryAllocator::Free (void *ptr)
    {
        if (ptr == nullptr)
        {
            return;
        }

        unsigned long freeMemPtrValue = (unsigned long)(ptr);
        unsigned long beginAddrValue = (unsigned long)(m_MemAlloc);
        unsigned long endAddrValue = (unsigned long)(m_MemAlloc + AllocSize);

        // Test if this ptr is valid.
        if (freeMemPtrValue < beginAddrValue || freeMemPtrValue > endAddrValue)
        {
            // This Ptr is not belongs to this Memory Area.
            assert (false);
        }
        else
        {
            unsigned long offset = freeMemPtrValue - beginAddrValue;
            unsigned int beginSearchDepth = 0;
            unsigned int beginSearchIndex = 0;

            // The Following code used to find the search beginning level in the
            // tree by the offset

            // At first the offset must be divided exactly to the AllocUnit
            assert (offset % AllocUnit == 0);
            
            // Try to search up to find the highest Tree Level
            unsigned int exp = 0;
            unsigned int levelUnitSize = GetPowof2 (exp);
            for (exp = AllocUnitExp; exp <= AllocSizeExp; ++exp)
            {
                levelUnitSize = GetPowof2 (exp);
                if (offset % levelUnitSize == 0)
                {

                }
                else
                {
                    --exp;
                    break;
                }
            }

            levelUnitSize = GetPowof2 (exp);
            beginSearchDepth = TreeDepth - (exp - AllocUnitExp);
            auto levelBeginIndex = GetPowof2 (beginSearchDepth) - 1;
            beginSearchIndex = levelBeginIndex + offset / levelUnitSize;

//             auto tB = beginAddrValue;
//             auto tE = endAddrValue;
//             auto tP = freeMemPtrValue;

//             while (true)
//             {
//                 if (tB == tP)
//                 {
//                     break;
//                 }
//                 else
//                 {
//                     if (tE - tB == AllocUnit)
//                     {
//                         if (tP != tB)
//                         {
//                             // This Ptr is not belongs to this Memory Area.
//                             // It points to the Mid Part of AllocUnit.
//                             assert (false);
//                         }
//                         else
//                         {
//                             break;
//                         }
//                     }
//                     else
//                     {
//                         ++beginSearchDepth;
//                         auto tMid = (tB + tE) / 2;
//                         if (tP < tMid)
//                         {
//                             // Go To Left Tree To search
//                             beginSearchIndex = GetLeftNodeIndex (beginSearchIndex);
//                             tE = tMid;
//                         }
//                         else
//                         {
//                             beginSearchIndex = GetRightNodeIndex (beginSearchIndex);
//                             tB = tMid;
//                         }
//                     }
//                 }
//             }

            // Release
            if (IsLeaf (beginSearchIndex, m_TreeLength))
            {
                assert (m_NodeTree [beginSearchIndex] == NodeState::eNODE_USED);
                m_NodeTree [beginSearchIndex] = NodeState::eNODE_UNUSE;
            }
            else
            {
                if (m_NodeTree [beginSearchIndex] == NodeState::eNODE_USED)
                {
                    MarkTree (beginSearchIndex, NodeState::eNODE_UNUSE);
                }
                else if (m_NodeTree [beginSearchIndex] == NodeState::eNODE_OCCUPIED_TO_CHILD)
                {
                    // Go left Tree to find NodeState::eNODE_USED
                    while (true)
                    {
                        beginSearchIndex = GetLeftNodeIndex (beginSearchIndex);
                        if (IsLeaf (beginSearchIndex, m_TreeLength))
                        {
                            assert (m_NodeTree [beginSearchIndex] == NodeState::eNODE_USED);
                            break;
                        }
                        else
                        {
                            if (m_NodeTree [beginSearchIndex] == NodeState::eNODE_USED)
                            {
                                break;
                            }
                            assert (m_NodeTree [beginSearchIndex] == NodeState::eNODE_OCCUPIED_TO_CHILD);
                        }
                    }
                    MarkTree (beginSearchIndex, NodeState::eNODE_UNUSE);
                }
                else
                {
                    // Data Structure Error.
                    assert (false);
                }
            }

            // Try Combine.
            unsigned int nodeIndex = beginSearchIndex;

            while (true)
            {
                if (IsRootNode (nodeIndex))
                {
                    break;
                }
                else
                {
                    unsigned int siblingNodeIndex = GetSiblingNode (nodeIndex);
                    unsigned int parentIndex = GetParentNodeIndex (nodeIndex);
                    if (m_NodeTree [siblingNodeIndex] == NodeState::eNODE_UNUSE && m_NodeTree [parentIndex] == NodeState::eNODE_OCCUPIED_TO_CHILD)
                    {
                        m_NodeTree [parentIndex] = NodeState::eNODE_UNUSE;
                        nodeIndex = parentIndex;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }

    void BuddyMemoryAllocator::MarkTree (const unsigned int &rootIndex, const NodeState &nodeState)
    {
        if (IsLeaf (rootIndex, m_TreeLength))
        {
            m_NodeTree [rootIndex] = nodeState;
            return;
        }
        else
        {
            MarkTree (GetLeftNodeIndex (rootIndex), nodeState);
            MarkTree (GetRightNodeIndex (rootIndex), nodeState);
            m_NodeTree [rootIndex] = nodeState;
        }
    }
}