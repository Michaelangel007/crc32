/*

"CRC32 Demystified"
https://github.com/Michaelangel007/crc32

Michaelangel007
Copyleft (C) 2017

*/

// Globals
    uint32_t aCRC32[ 256 ]; // Init Normal    form (Top-Bit) + forward polynomial
    uint32_t bCRC32[ 256 ]; // Init Reflected form (Low-Bit) + forward polynomial
    uint32_t cCRC32[ 256 ]; // Init Normal    form (Top-Bit) + reverse polynomial
    uint32_t dCRC32[ 256 ]; // Init Reflected form (Low-Bit) + reverse polynomial

    // Array of pointers to data
    const uint32_t *aData[] =
    {
         aCRC32
        ,bCRC32
        ,cCRC32
        ,dCRC32
    };
    const int nData = sizeof( aData ) / sizeof( uint32_t * );

    const char *aDesc[] =
    {
         "Normal    0x04C11DB7: &<<31 [  1] = poly     , [ 16] =     poly << 8 VALID "
        ,"Reflected 0x04C11DB7: &1 >> [  1] = rev. poly, [ 30] = rev.poly << 8 broken"
        ,"Normal    0xEDB88320: &<<31 [128] = rev. poly, [120] = rev.poly >> 8 broken"
        ,"Reflected 0xEDB88320: &1 >> [128] = poly     , [  8] =     poly >> 8 VALID "
    };
    const int nDesc = sizeof( aDesc ) / sizeof( char * );

    int ERROR_aData_size_not_equal_aDesc[ nData == nDesc ];

    // Table-Lookup CRC
    const Crc32Func_t aFunc[] =
    {
         crc32_000
        ,crc32_001
        ,crc32_010
        ,crc32_011
        ,crc32_100
        ,crc32_101
        ,crc32_110
        ,crc32_111
    };
    const int nFunc = sizeof( aFunc ) / sizeof( Crc32Func_t );

    uint32_t aPoly[] =
    {
         CRC32_FORWARD
        ,CRC32_FORWARD
        ,CRC32_REVERSE
        ,CRC32_REVERSE
    };

    // Formulaic CRC
    const Crc32Simple_t aSimple[] =
    {
         crc32_formula_normal
        ,crc32_formula_reflect
        ,crc32_formula_normal
        ,crc32_formula_reflect
    };

    const char *aStatus[] =
    {
         ""
        ,"invalid!"
        ,"invalid!"
        ,""
    };

    const char *aType[] =
    {
         "Normal   "
        ,"Reflected"
        ,"Normal   "
        ,"Reflected"
    };

// Utility

    // Dump 32-bit table of 256 entries
    // ========================================================================
    void dump( const char *header, const uint32_t *table )
    {
        printf( "%s\n", header );

        for (int i = 0; i < 256; i++ )
        {
            printf( "%08X, ", table[ i ] );
            if ((i % 8) == 7)
                printf( " // %3d [0x%02X .. 0x%02X]\n", i-7,i-7, i );
        }
        printf( "\n" );
    }


// ========================================================================
void common_init( const int bDumpTables = false )
{
    reverse_init();

    crc32_init_normal ( aCRC32, CRC32_FORWARD ); // Valid
    crc32_init_reflect( bCRC32, CRC32_FORWARD ); // Mismatched reflect form with forward polynomial
    crc32_init_normal ( cCRC32, CRC32_REVERSE ); // Mismatched normal  form with reverse polynomial
    crc32_init_reflect( dCRC32, CRC32_REVERSE ); // Valid

    if( bDumpTables )
        for( int iTable = 0; iTable < nDesc; iTable++ )
            dump( aDesc[ iTable ], aData[ iTable ] );
}

