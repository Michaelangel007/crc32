/*

"CRC32 Demystified"
https://github.com/Michaelangel007/crc32

Michaelangel007
Copyleft (C) 2017

*/
#define USE_OMP 1

// Includes
    #include "common.cpp"

// BEGIN OMP
#if USE_OMP
    #include <omp.h>
#endif
    #include "util_threads.h"
// END OMP


void SearchLen1()
{
    printf( "Searching len 1...\n" );
    for( int bytes1 = 0; bytes1 <= 0xFF; bytes1++ )
    {
        unsigned char data1[1];
        *((uint8_t *)data1) = bytes1;

        unsigned int result1 = crc32_forward( 1, data1 );
        if (result1 == 0) printf( "\n[%08X] Found zero LEN 1!\n", bytes1 );
    }
}

void SearchLen2()
{
    printf( "Searching len 2...\n" );
    for (int bytes2 = 0; bytes2 <= 0xFFFF; bytes2++)
    {
        unsigned char data2[2];
        *((uint16_t *)data2) = bytes2;

        unsigned result2 = crc32_forward( 2, data2 );
        if (result2 == 0) printf( "\n[%08X] Found zero LEN 2!\n", bytes2 );
    }
}

void SearchLen3()
{
    printf( "Searching len 3...\n" );
    for (int bytes3 = 0; bytes3 <= 0xFFFFFF; bytes3++)
    {
        unsigned char data3[4];
        *((uint32_t *)data3) = bytes3;

        unsigned result2 = crc32_forward( 3, data3 );
        if (result2 == 0) printf( "\n[%08X] Found zero LEN 3!\n", bytes3 );
    }
}

void SearchLen4()
{
    int total = 0;

    printf( "Searching len 4...\n" );
#pragma omp parallel for
    for( int iPage = 0; iPage <= 0xFF; iPage++ )
    {
#if USE_OMP
        const int iTid = omp_get_thread_num(); // Get Thread Index: 0 .. nCores-1
#else
        const int iTid = 0;
#endif
        if (iTid == 0)
            printf( "." );
        // printf( " %02X_000000: \n", iPage & 0xFF );

        uint32_t       Haystack = ((uint32_t)iPage) << 24;
        unsigned char *pNeedle  = (unsigned char*) &Haystack;
        unsigned int result4;

        for( int byte = 0; byte <= 0xFFFFFF; byte++ )
        {
            // Which poly???  B69B50B9

            // 6DD90A9D
            result4 = crc32_reverse( 4, pNeedle ); if (result4 == 0) 
            {
                printf( "[%08X] Found zero!\n", Haystack );
#pragma omp atomic
                total++;
            }
            Haystack++;
        }
    }
    printf( "Found %d of len 4\n", total );
}

void SearchLen5()
{
    int total = 0;
    printf( "Searching len 5...\n" );
#pragma omp parallel for
    for( int iPage = 0; iPage <= 0xFF; iPage++ )
    {
#if USE_OMP
        const int iTid = omp_get_thread_num(); // Get Thread Index: 0 .. nCores-1
#else
        const int iTid = 0;
#endif
//        if (iTid == 0)
            printf( "." );

        uint64_t       Haystack = ((uint64_t)iPage) << 32;
        unsigned char *pNeedle = (unsigned char*) &Haystack;
        unsigned int result5;

        for( int bytes5 = 0; bytes5 <= 0xFFFFFFFF; bytes5++ )
        {
            *((uint32_t*)(&pNeedle[1])) = bytes5;
            result5 = crc32_reverse( 5, pNeedle ); if (result5 == 0)
            {
                printf( "[%08llX] Found zero!\n", Haystack );
#pragma omp atomic
                total++;
            }
            Haystack++;
        }
    }
    printf( "Found %d of len 5\n", total );
}

void SetString(size_t length, unsigned char* data, uint64_t key)
{
    for (int byte = 0; byte < length; byte++)
    {
        data[ byte ] = key & 0xFF;
        key >>= 8;
    }
}

// ========================================================================
int main(int nArg, char *aArg[])
{
#if USE_OMP
    gnThreadsMaximum = omp_get_num_procs();
    if( gnThreadsMaximum > MAX_THREADS )
        gnThreadsMaximum = MAX_THREADS;
#endif // USE_OMP

    int   iArg = 0;

    if( nArg > 1 )
    {
        while( iArg < nArg )
        {
            char *pArg = aArg[ iArg + 1 ];
            if(  !pArg )
                break;

            if( pArg[0] == '-' )
            {
                iArg++;
                pArg++; // point to 1st char in option
// BEGIN OMP
                if( *pArg == 'j' )
                {
                    int i = atoi( pArg+1 ); 
                    if( i > 0 )
                        gnThreadsActive = i;
                    if( gnThreadsActive > MAX_THREADS )
                        gnThreadsActive = MAX_THREADS;
                }
// END OMP
                else
                    printf( "Unrecognized option: %c\n", *pArg ); 
            }
            else
                break;
        }
    }

#if USE_OMP
    if(!gnThreadsActive) // user didn't specify how many threads to use, default to all of them
        gnThreadsActive = gnThreadsMaximum;
    else
        omp_set_num_threads( gnThreadsActive );
#endif // USE_OMP
    printf( "Using: %u / %u threads\n", gnThreadsActive, gnThreadsMaximum );

    common_init( false );
    printf( "Searching for CRC32() zero hash...\n" );

    SearchLen1();
    SearchLen2();
    SearchLen3();
    SearchLen4();
    SearchLen5();

    printf( "Done.\n" );

    return 0;
}

