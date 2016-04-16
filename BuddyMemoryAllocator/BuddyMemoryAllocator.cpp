#include <stack>

#include "BuddyMemoryAllocator.h"
#include <vcruntime_string.h>
#include <xtgmath.h>


namespace ZGE
{

    BuddyMemoryAllocator::BuddyMemoryAllocator ()
    {
        m_MemAlloc = new Byte[ GetPowof2 ( MaxDepth ) ];
        m_NodeTree = new NodeState[ GetPowof2 ( MaxDepth - GetExpof2 ( AllocUnit ) + 1 ) ];
        m_TreeLength = GetPowof2 ( MaxDepth - GetExpof2 ( AllocUnit ) + 1 );

        MarkTree ( 0, NodeState::eNODE_UNUSE );
    }

    BuddyMemoryAllocator::~BuddyMemoryAllocator ()
    {
        delete m_MemAlloc;
        delete m_NodeTree;
    }



    void * BuddyMemoryAllocator::Malloc ( unsigned int allocSize )
    {
        if ( allocSize == 0 )
        {
            assert ( false );
            return nullptr;
        }

        if ( GetPowof2 ( MaxDepth ) < allocSize )
        {
            // OVER LIMIT!
            assert ( false );
            return nullptr;
        }

        unsigned int searchDepth = 0;
        if ( allocSize <= AllocUnit )
        {
            searchDepth = MaxDepth;
        }
        else
        {
            unsigned int value = GetNextPowof2 ( allocSize );
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
        unsigned int beginIndex = ( 1 << searchDepth ) - 1;
        unsigned int endIndex = ( 1 << searchDepth + 1 ) - 2;

        unsigned int searchIndex = 0;

        for ( searchIndex = beginIndex; searchIndex <= endIndex; ++searchIndex )
        {
            NodeState *nodeState = m_NodeTree + searchIndex;
            if ( *nodeState == NodeState::eNODE_UNUSE )
            {
                break;
            }
            else
            {
                continue;
            }
        }

        if ( searchIndex > endIndex )
        {
            // Can't Alloc.
            return nullptr;
        }
        else
        {
            // Make this node used, all children Occupied to parent, all parent occupied to children.
            m_NodeTree[ searchIndex ] = NodeState::eNODE_USED;

            // Children Part.
            MarkTree ( searchIndex, NodeState::eNODE_OCCUPIED_TO_PARENT );

            // Parent Part.
            auto parentIndex = searchIndex;
            while ( true )
            {
                if ( parentIndex == 0 )
                {
                    *( m_NodeTree + 0 ) = NodeState::eNODE_OCCUPIED_TO_CHILD;
                    break;
                }
                else
                {
                    parentIndex = GetParentNodeIndex ( parentIndex );
                    *( m_NodeTree + parentIndex ) = NodeState::eNODE_OCCUPIED_TO_CHILD;
                }
            }
        }

        unsigned int unitSizeofThisDepth = allocSize / std::pow ( 2, searchDepth );
        void *retAddr = m_MemAlloc;
        long retAddrValue = ( long )retAddr;
        retAddrValue += ( searchIndex - beginIndex ) * unitSizeofThisDepth;
        retAddr = ( void * )( retAddrValue );
        return retAddr;
    }

    void BuddyMemoryAllocator::Free ( void *ptr )
    {

    }

    void BuddyMemoryAllocator::MarkTree ( const unsigned int &rootIndex, const NodeState &nodeState )
    {
        if ( IsLeaf ( rootIndex, m_TreeLength ) )
        {
            m_NodeTree[ rootIndex ] = nodeState;
            return;
        }
        else
        {
            MarkTree ( GetLeftNodeIndex ( rootIndex ), nodeState );
            MarkTree ( GetRightNodeIndex ( rootIndex ), nodeState );
        }
    }

}