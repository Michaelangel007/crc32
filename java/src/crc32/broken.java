/*

"CRC32 Demystified"
https://github.com/Michaelangel007/crc32

Michaelangel007
Copyleft (C) 2017

*/

// Original Bret's Mulvey broken C# CRC32
/*
    public class CRC32 : HashFunction
    {
        uint[] tab;

        public CRC32()
        {
            Init(0x04c11db7);
        }

        public CRC32(uint poly)
        {
            Init(poly);
        }

        void Init(uint poly)
        {
            tab = new uint[256];
            for (uint i=0; i<256; i++)
            {
                uint t = i;
                for (int j=0; j<8; j++)
                    if ((t & 1) == 0)
                        t >>= 1;
                    else
                        t = (t >> 1) ^ poly;
                tab[i] = t;
            }
        }

        public override uint ComputeHash(byte[] data)
        {
            uint hash = 0xFFFFFFFF;
            foreach (byte b in data)
                hash = (hash << 8) 
                    ^ tab[b ^ (hash >> 24)];
            return ~hash;
        }
    }

Compile:

    javac -d . broken.java

Output:

    crc32/broken.class

To use:

    import crc32.broken;

    {
        crc32.broken broken = new crc32.broken();

        String text = "123456789";
        byte[] data = text.getBytes();
        int    crc1 = broken.ComputeHash( data );
    }
*/
package crc32;

public class broken
{
    int[] _crc;

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
            int crc = bite;

            for( int bit = 0; bit < 8; bit++ )
            {
                // if (crc & 1) // Work around retarded Java boolean cast
                if ((crc & 1) == 1)
                {
                    crc >>= 1;
                    crc  &= 0x7FFFFFFF; // Work around retarded pre Java 8 lacking unsigned int
                    crc  ^= POLY;
                }
                else
                {
                    crc >>= 1;
                    crc  &= 0x7FFFFFFF; // Work around retarded pre Java 8 lacking unsigned int
                }
            }

            _crc[ bite ] = crc;
        }
    }

// Public

    public broken()
    {
        init( 0x04C11DB7 );
    }

    public broken( int poly )
    {
        init( poly );
    }

    public void Dump()
    {
        dump( _crc );
    }

    public int ComputeHash(byte[] data)
    {
        int hash = 0xFFFFFFFF;
        for( byte bite: data )
            hash = _crc[ bite ^ ((hash >> 24) & 0xFF) ] ^ (hash << 8);
        return ~hash;
    }
}

