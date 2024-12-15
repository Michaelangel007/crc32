#if _WIN32
    #define _CRT_SECURE_NO_WARNINGS 1
#endif

#include "common.cpp"
#include <ctype.h> // isspace()

const uint32_t CRC32Family[32]
{
    //                 Polynomial reversed?
    //                  Reflected?
    //                   Shift Right?
    //                    Data Reversed?
    //                     CRC Reversed?
    //     Hash    Id  PRSDC
     0xFC891918 //  0 [00000] 0x04C11DB7
    ,0x1898913F //  1 [00001]
    ,0x649C2FD3 //  2 [00010]
    ,0xCBF43926 //  3 [00011]
    ,0x7EAD5C77 //  4 [00100]
    ,0xEE3AB57E //  5 [00101]
    ,0x0D0B7023 //  6 [00110]
    ,0xC40ED0B0 //  7 [00111]

    ,0xC9A0B7E5 //  8 [01000]
    ,0xA7ED0593 //  9 [01001]
    ,0x9D594C04 // 10 [01010]
    ,0x20329AB9 // 11 [01011]
    ,0xFC4F2BE9 // 12 [01100]
    ,0x97D4F23F // 13 [01101]
    ,0xFDEFB72E // 14 [01110]
    ,0x74EDF7BF // 15 [01111]
   
    ,0x74EDF7BF // 16 [01000] 0xEDB88320
    ,0xFDEFB72E // 17 [01001]
    ,0x97D4F23F // 18 [01010]
    ,0xFC4F2BE9 // 19 [01011]
    ,0x20329AB9 // 20 [01100]
    ,0x9D594C04 // 21 [01101]
    ,0xA7ED0593 // 22 [01110]
    ,0xC9A0B7E5 // 23 [01111]

    ,0xC40ED0B0 // 24 [11000]
    ,0x0D0B7023 // 25 [11001]
    ,0xEE3AB57E // 26 [11010]
    ,0x7EAD5C77 // 27 [11011]
    ,0xCBF43926 // 28 [11100]
    ,0x649C2FD3 // 29 [11101]
    ,0x1898913F // 30 [11110]
    ,0xFC891918 // 31 [11111]
    //                 0 = 0x04C11DB7
    //                 1 = 0xEDB88320
    //                  0 = Normal
    //                  1 = Reflected
    //                   0 = Left
    //                   1 = Right
};

int CRC32Id(const uint32_t poly, const uint32_t hash)
{
    const int nHash = (int) (sizeof(CRC32Family) / sizeof(CRC32Family[0]));

    for( int iHash = 0; iHash < nHash; ++iHash )
    {
        // The last 16 hashes are reflected from the first 16
        // thus we need to check if we have the first polynomial as well.
        // The 4 forms have the polynomial stored in aPoly
        if ((CRC32Family[iHash] == hash) && (poly == aPoly[iHash/8]))
            return iHash;
    }
    return -1;
}

void CRC32Analysis(const uint32_t crc32id)
{
    if (crc32id < 0)
    {
        printf( "Not a known CRC32 algorithm.\n" );
        return;
    }

    int isNormal  = (crc32id >> 4) & 1;
    int isReflect = (crc32id >> 3) & 1;
    int isShiftR  = (crc32id >> 2) & 1;
    int isRevData = (crc32id >> 1) & 1;
    int isRevCRC  = (crc32id >> 0) & 1;

    const int       iFunc = (crc32id >> 3) & 3;
    const uint32_t *pData = aData[ iFunc ];
    const char     *pDesc = aDesc[ iFunc ];

    const char   *text   = CRC32_CHECK_TXT;
    const size_t  length = strlen( text );

    const char *aNoYes[2] = { "No ", "Yes" };

    uint32_t crc = aFunc[ iFunc ]( pData, length, text ); 

    printf( "Poly: %08X   Reflect: %s  Normal: %s  Shift: %s, Rev. Data: %d, Rev. CRC: %d, 0x%08X\n"
        , aPoly[ iFunc ]
        , aNoYes[ isReflect ]
        , aNoYes[ isNormal ]
        , isShiftR
          ? "Right"
          : "Left "
        , isRevData
        , isRevCRC
        , crc
    );
}

int main(int nArg, char *aArg[])
{
    char buffer[16];

    uint32_t poly = (nArg > 1) ? strtoul( aArg[1], 0, 16 ) : 0;
    uint32_t hash = (nArg > 2) ? strtoul( aArg[2], 0, 16 ) : 0;
    int      id;

    common_init();

    if( !poly )
    {
        printf( "Using your favorite crc32()\n" );
        printf( "1. Enter the 32-bit Polynomial (in hex) used:\n" );
        if (fgets(buffer, sizeof(buffer), stdin))
            sscanf( buffer, "%X", &poly );
    }

    if (!hash)
    {
        printf( "2. Enter the 32-bit Hash value (in hex) it generates for the string: %s\n", CRC32_CHECK_TXT );
        if (fgets(buffer, sizeof(buffer), stdin))
            sscanf(buffer, "%X", &hash);
    }

    id = CRC32Id( poly, hash );
    printf( "CRCid: %d\n", id);
    CRC32Analysis( id );

    return 0;
}
