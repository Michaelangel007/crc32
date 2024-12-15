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

    if( bDumpTables )
        for( int iTable = 0; iTable < nDesc; iTable++ )
            dump( aDesc[ iTable ], aData[ iTable ] );
}
