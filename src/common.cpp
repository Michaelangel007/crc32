// Includes

    #include "common.h"

// Vars

    uint32_t REVERSE_BITS[ 256 ];

// ========================================================================
void common_init( const int bDumpTables = false )
{
    CRC32_Init();
    ReverseBits_Init();

    crc32_init_normal ( aCRC32, POLY_FORWARD ); // Valid
    crc32_init_reflect( bCRC32, POLY_FORWARD ); // Mismatched reflect form with forward polynomial
    crc32_init_normal ( cCRC32, POLY_REVERSE ); // Mismatched normal  form with reverse polynomial
    crc32_init_reflect( dCRC32, POLY_REVERSE ); // Valid

    if (aCRC32[ 1] != (POLY_FORWARD << 0)) printf( "ERROR: Failed to initialized CRC32 table\n" );
    if (aCRC32[16] != (POLY_FORWARD << 4)) printf( "ERROR: Failed to initialized CRC32 table\n" );
    if (dCRC32[ 8] != (POLY_REVERSE >> 4)) printf( "ERROR: Failed to initialized CRC32 table\n" );
    if (dCRC32[ 1] ==                   0) printf( "ERROR: Failed to initialized CRC32 table\n" );

    if (bDumpTables)
    {
        for( int iTable = 0; iTable < nDesc; iTable++ )
            dump( aDesc[ iTable ], aData[ iTable ] );

        printf( "aCRC32 = { 0x%08X, 0x%08X, ..., 0x%08X }\n", aCRC32[0], aCRC32[1], aCRC32[255] );
        printf( "bCRC32 = { 0x%08X, 0x%08X, ..., 0x%08X }\n", bCRC32[0], bCRC32[1], bCRC32[255] );
        printf( "cCRC32 = { 0x%08X, 0x%08X, ..., 0x%08X }\n", cCRC32[0], cCRC32[1], cCRC32[255] );
        printf( "dCRC32 = { 0x%08X, 0x%08X, ..., 0x%08X }\n", dCRC32[0], dCRC32[1], dCRC32[255] );
        printf( "\n" );
    }
}
