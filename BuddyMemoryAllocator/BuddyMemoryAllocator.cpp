#include <stack>

#include <vcruntime_string.h>
#include <math.h>
#include "BuddyMemoryAllocator.h"

namespace ZGE
{
    BuddyMemoryAllocator::BuddyMemoryAllocator ()
    {
        m_MemAlloc = new Byte [GetPowof2 (MaxDepth)];
        m_NodeTree = new NodeState [GetPowof2 (MaxDepth - GetExpof2 (AllocUnit) + 1)];
        m_TreeLength = GetPowof2 (MaxDepth - GetExpof2 (AllocUnit) + 1) - 1;

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

        if (GetPowof2 (MaxDepth) < allocSize)
        {
            // OVER LIMIT!
            assert (false);
            return nullptr;
        }

        unsigned int searchDepth = 0;
        if (allocSize <= AllocUnit)
        {
            searchDepth = MaxDepth;
        }
        else
        {
            unsigned int value = GetNextPowof2 (allocSize);
            searchDepth = MaxDepth - value;
        }

        //         auto GetAddrFromIndex = [ & ](unsigned int index) -> void *
        //         {
        //             void *beginAddr = m_MemAlloc;
        //             void *endAddr = m_MemAlloc + allocSize;
        //
        //             if ( index == 0 )
        //             {
        //                 return beginAddr;
        //             }
        //             else
        //             {
        //                 // true mean left, false mean left
        //                 std::stack<bool> nodePath;
        //                 unsigned int t = index;
        //                 while ( t != 0 )
        //                 {
        //                     if ( IsLeftNode ( t ) )
        //                     {
        //                         nodePath.push ( true );
        //                     }
        //                     else
        //                     {
        //                         nodePath.push ( false );
        //                     }
        //                     t = GetParentNodeIndex ( t );
        //                 }
        //
        //
        //                 while ( nodePath.empty () == false )
        //                 {
        //                     bool isLeft = nodePath.top ();
        //                     unsigned long beginAddrValue = ( long )( beginAddr );
        //                     unsigned long endAddrValue = ( long )( endAddr );
        //                     unsigned long size = endAddrValue - beginAddrValue;
        //
        //                     if ( isLeft )
        //                     {
        //                         endAddr = ( void * )( endAddrValue - size / 2 );
        //                     }
        //                     else
        //                     {
        //                         beginAddr = ( void * )( beginAddrValue + size / 2 );
        //                     }
        //                     nodePath.pop ();
        //                 }
        //                 return beginAddr;
        //
        //             }
        //         };

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

        unsigned int unitSizeofThisDepth = (unsigned int)(GetPowof2 (MaxDepth) / std::pow (2, searchDepth));
        void *retAddr = m_MemAlloc;
        long retAddrValue = (long)retAddr;
        retAddrValue += (searchIndex - beginIndex) * unitSizeofThisDepth;
        retAddr = (void *)(retAddrValue);
        return retAddr;
    }

    void BuddyMemoryAllocator::Free (void *ptr)
    {
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

            auto tB = beginAddrValue;
            auto tE = endAddrValue;
            auto tF = freeMemPtrValue;

            while (true)
            {
                if (tB == tF)
                {
                    break;
                }
                else
                {
                    if (tE - tB == AllocUnit)
                    {
                        if (tF != tB)
                        {
                            // This Ptr is not belongs to this Memory Area.
                            // It points to the Mid Part of AllocUnit.
                            assert (false);
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        ++beginSearchDepth;
                        auto tMid = (tB + tE) / 2;
                        if (tF < tMid)
                        {
                            // Go To Left Tree To search
                            beginSearchIndex = GetLeftNodeIndex (beginSearchIndex);
                            tE = tMid;
                        }
                        else
                        {
                            beginSearchIndex = GetRightNodeIndex (beginSearchIndex);
                            tB = tMid;
                        }
                    }
                }
            }

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