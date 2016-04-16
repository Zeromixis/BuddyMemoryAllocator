#pragma once

#include <assert.h>

namespace ZGE
{
    static constexpr unsigned int GetPowof2 ( const unsigned int &e )
    {
        return 1 << e;
    }

    static unsigned int GetExpof2 ( unsigned int value )
    {
        unsigned int e = 0;
        do
        {
            ++e;
            value = value >> 1;
            
        } while ( value > 0 );
        return e - 1;
    }

    static unsigned int GetNextPowof2 ( const unsigned int &value )
    {
        assert ( value > 0 );
        unsigned int exp = 0;
        while ( true )
        {
            if ( 1 << exp >= value )
            {
                return exp;
            }
            else
            {
                ++exp;
            }
        }
    }

    static constexpr unsigned int GetLeftNodeIndex ( const unsigned int &parentIndex )
    {
        return parentIndex * 2 + 1;
    }

    static constexpr unsigned int GetRightNodeIndex ( const unsigned int &parentIndex )
    {
        return parentIndex * 2 + 2;
    }

    static constexpr unsigned int GetParentNodeIndex ( const unsigned int &childIndex )
    {
        return ( childIndex - 1 ) / 2;
    }

    static constexpr bool IsLeaf ( const unsigned int &childIndex, const unsigned int &MaxIndex )
    {
        return GetLeftNodeIndex ( childIndex ) > MaxIndex;
    }

    static constexpr bool IsLeftNode ( const unsigned int &childIndex )
    {
        return childIndex % 2 == 1;
    }

    static constexpr bool IsRightNode ( const unsigned int &childIndex )
    {
        return childIndex % 2 == 0;
    }

    static constexpr bool IsRootNode ( const unsigned int &childIndex )
    {
        return childIndex == 0;
    }

    static unsigned int GetNodeDepth ( const unsigned int &nodeIndex )
    {
        if ( nodeIndex == 0 )
        {
            return 0;
        }
        else
        {
            unsigned int tIndex = nodeIndex;
            if ( IsLeftNode ( tIndex ) )
            {
                ++tIndex;
            }

            unsigned int depth = 0;
            while ( nodeIndex != 1 )
            {
                ++depth;
                tIndex >>= 1;
            }
            return depth;
        }
    }
}