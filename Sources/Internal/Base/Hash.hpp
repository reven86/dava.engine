#if !defined __HASH_HPP__
#define __HASH_HPP__
//==============================================================================
//
//  quasi compile-time hash
//  it's 'quasi' since you can't use its value in switch() or as template-argument

    #include "BaseTypes.h"
    using DAVA::uint32;

// NOTE: 'str' expected to be immutable const static string
#define L_HASH(str)     StringHash(str).hash
#define L_HASH_INLINE inline

//------------------------------------------------------------------------------

L_HASH_INLINE uint32
HashValue_N( const char* key, unsigned length )
{
    #define _Hash_Mix( a, b, c )    \
    a -= b; a -= c; a ^= (c>>13);   \
    b -= c; b -= a; b ^= (a<<8);    \
    c -= a; c -= b; c ^= (b>>13);   \
    a -= b; a -= c; a ^= (c>>12);   \
    b -= c; b -= a; b ^= (a<<16);   \
    c -= a; c -= b; c ^= (b>>5);    \
    a -= b; a -= c; a ^= (c>>3);    \
    b -= c; b -= a; b ^= (a<<10);   \
    c -= a; c -= b; c ^= (b>>15);   \


    // set up the internal state

    register uint32 len = length;
    register uint32 a   = 0x9E3779B9;   // the golden ratio, an arbitrary value
    register uint32 b   = 0x9E3779B9;   // the golden ratio, an arbitrary value
    register uint32 c   = 0;            // the previous hash value


    // handle most of the key
    
    while( len >= 12 )
    {
        a += (key[0] +((uint32)key[1]<<8) +((uint32)key[2]<<16) +((uint32)key[3]<<24));
        b += (key[4] +((uint32)key[5]<<8) +((uint32)key[6]<<16) +((uint32)key[7]<<24));
        c += (key[8] +((uint32)key[9]<<8) +((uint32)key[10]<<16)+((uint32)key[11]<<24));

        _Hash_Mix( a, b, c );

        key += 12; len -= 12;
    }


    // handle the last 11 bytes
    
    c += length;

    //lint -e{616} // control flows into case/default
    switch( len ) 
    {
        // all the case statements fall through
        
        case 11: c += ((uint32)key[10]<<24);
        case 10: c += ((uint32)key[9]<<16);
        case 9 : c += ((uint32)key[8]<<8);
        // the first byte of c is reserved for the length
        case 8 : b += ((uint32)key[7]<<24);
        case 7 : b += ((uint32)key[6]<<16);
        case 6 : b += ((uint32)key[5]<<8);
        case 5 : b += key[4];
        case 4 : a += ((uint32)key[3]<<24);
        case 3 : a += ((uint32)key[2]<<16);
        case 2 : a += ((uint32)key[1]<<8);
        case 1 : a += key[0];
    //  case 0: nothing left to add
    }

    _Hash_Mix( a, b, c );

    return c;
    #undef _Hash_Mix
}


//------------------------------------------------------------------------------

class 
StringHash
{ 
public:

    uint32  hash;

    L_HASH_INLINE   StringHash( const char (&str)[1] )  : hash(HashValue_N(str,1-1))    {}
    L_HASH_INLINE   StringHash( const char (&str)[2] )  : hash(HashValue_N(str,2-1))    {}
    L_HASH_INLINE   StringHash( const char (&str)[3] )  : hash(HashValue_N(str,3-1))    {}
    L_HASH_INLINE   StringHash( const char (&str)[4] )  : hash(HashValue_N(str,4-1))    {}
    L_HASH_INLINE   StringHash( const char (&str)[5] )  : hash(HashValue_N(str,5-1))    {}
    L_HASH_INLINE   StringHash( const char (&str)[6] )  : hash(HashValue_N(str,6-1))    {}
    L_HASH_INLINE   StringHash( const char (&str)[7] )  : hash(HashValue_N(str,7-1))    {}
    L_HASH_INLINE   StringHash( const char (&str)[8] )  : hash(HashValue_N(str,8-1))    {}
    L_HASH_INLINE   StringHash( const char (&str)[9] )  : hash(HashValue_N(str,9-1))    {}
    L_HASH_INLINE   StringHash( const char (&str)[10] ) : hash(HashValue_N(str,10-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[11] ) : hash(HashValue_N(str,11-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[12] ) : hash(HashValue_N(str,12-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[13] ) : hash(HashValue_N(str,13-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[14] ) : hash(HashValue_N(str,14-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[15] ) : hash(HashValue_N(str,15-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[16] ) : hash(HashValue_N(str,16-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[17] ) : hash(HashValue_N(str,17-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[18] ) : hash(HashValue_N(str,18-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[19] ) : hash(HashValue_N(str,19-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[20] ) : hash(HashValue_N(str,20-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[21] ) : hash(HashValue_N(str,21-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[22] ) : hash(HashValue_N(str,22-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[23] ) : hash(HashValue_N(str,23-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[24] ) : hash(HashValue_N(str,24-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[25] ) : hash(HashValue_N(str,25-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[26] ) : hash(HashValue_N(str,26-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[27] ) : hash(HashValue_N(str,27-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[28] ) : hash(HashValue_N(str,28-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[29] ) : hash(HashValue_N(str,29-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[30] ) : hash(HashValue_N(str,30-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[31] ) : hash(HashValue_N(str,31-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[32] ) : hash(HashValue_N(str,32-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[33] ) : hash(HashValue_N(str,33-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[34] ) : hash(HashValue_N(str,34-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[35] ) : hash(HashValue_N(str,35-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[36] ) : hash(HashValue_N(str,36-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[37] ) : hash(HashValue_N(str,37-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[38] ) : hash(HashValue_N(str,38-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[39] ) : hash(HashValue_N(str,39-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[40] ) : hash(HashValue_N(str,40-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[41] ) : hash(HashValue_N(str,41-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[42] ) : hash(HashValue_N(str,42-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[43] ) : hash(HashValue_N(str,43-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[44] ) : hash(HashValue_N(str,44-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[45] ) : hash(HashValue_N(str,45-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[46] ) : hash(HashValue_N(str,46-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[47] ) : hash(HashValue_N(str,47-1))   {}
    L_HASH_INLINE   StringHash( const char (&str)[48] ) : hash(HashValue_N(str,48-1))   {}
};

//#endif // L_PLATFORM != L_PLATOFRM_PS3


//==============================================================================
#endif // __HASH_HPP__

