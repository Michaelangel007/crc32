/*

Compile:

    javac bret_crc32.java

Run:

    java bret_crc32

Reference

* http://stackoverflow.com/questions/13738343/import-class-file-in-java

*/
import crc32.broken;
import crc32.normal;
import crc32.reflect;

public class bret_crc32
{
    public static void main(String args[])
    {
        crc32.broken  broken  = new crc32.broken();
        crc32.normal  normal  = new crc32.normal();
        crc32.reflect reflect = new crc32.reflect();

        System.out.println( "Original Broken CRC32..." );
        broken.Dump();

        System.out.println( "Normal CRC32..." );
        normal.Dump();

        System.out.println( "Reflect CRC32..." );
        reflect.Dump();

        String text = "123456789";
        byte[] data = text.getBytes();
        int    crc1 = broken.ComputeHash( data );
        int    crc2 = normal .ComputeHash( data );
        int    crc3 = reflect.ComputeHash( data );

        System.out.println( "String = " + text );
        System.out.println( String.format( "Broken  crc32 = 0x%08X", crc1 ) );
        System.out.println( String.format( "Normal  crc32 = 0x%08X", crc2 ) );
        System.out.println( String.format( "Reflect crc32 = 0x%08X", crc3 ) );
    }
}

