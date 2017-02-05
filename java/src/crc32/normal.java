/*

"CRC32 Demystified"
https://github.com/Michaelangel007/crc32

Michaelangel007
Copyleft (C) 2017

*/

package crc32;

public class normal
{
    int[] _crc;
    int[] _reverse8;

// Private

    static void dump( int[] array )
    {
        for( int bite = 0; bite < 256; bite++ )
        {
            if( (bite % 8) == 0 )
                System.out.print( "    " );

            System.out.print( String.format( "%08X, ", array[ bite ] ) );

            if( (bite % 8) == 7 )
                System.out.print( String.format( " // %3d [ 0x%02X ]\n", bite - 7, bite - 7 ) );
        }

        System.out.println();
    }

    void init( int POLY )
    {
        _crc = new int[ 256 ];

        for( int bite = 0; bite < 256; bite++ )
        {
            int crc = bite << 24;

            for( int bit = 0; bit < 8; bit++ )
            {
                // optimized: if (crc & (1 << 31))
                if (crc < 0)
                {
                    crc <<= 1;
                    crc  ^= POLY;
                }
                else
                {
                    crc <<= 1;
                }
            }

            _crc[ bite ] = crc;
        }

        _reverse8 = new int[ 256 ];
        for( int bite = 0; bite <= 255; bite++ )
            _reverse8[ bite ] = (reflect32( bite ) >> 24) & 0xFF;

        dump( _reverse8 );
    }

    int reflect32( int x )
    {
        int bits = 0;
        int mask = x;

        for( int i = 0; i < 32; i++ )
        {
            bits <<= 1;
            // if (mask & 1) // Work around retarded Java boolean cast
            if ((mask & 1) == 1)
                bits |= 1;
            mask >>= 1;
        }

        return bits;
    }

// Public

    public normal()
    {
        init( 0x04C11DB7 );
    }

    public normal( int POLY )
    {
        init( POLY );
    }

    public void Dump()
    {
        dump( _crc );
    }

    public int ComputeHash(byte[] data)
    {
        int hash = 0xFFFFFFFF;

        for( byte bite: data )
            hash = _crc[ (_reverse8[ bite ] ^ (hash >> 24)) & 0xFF ] ^ (hash << 8);

        return reflect32( ~hash );
    }
}

