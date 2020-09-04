import com.mcobject.extremedb.*;

enum Color { 
    Red, 
    Green,
    Blue
};

class NestedStruct { 
    int    pk;
    String str;

    NestedStruct(int id) {
        pk = id;
        str = "nested.str " + id;
    }

    NestedStruct() {} // Default constructor is needed for eXtremeDB
    public String toString() { return "pk=" + pk + ", str=\"" + str + "\""; }
}

@Persistent(list=true, autoid=true) // Class will be stored in eXtremeDB database, declare list and autoid indexes
class Record 
{
    static final int CHARS_LEN = 16; // Length of char and nchar fields
    static final int ARRAY_LEN = 5; // length of arrays

// Simple types
    byte  i1; // signed<1> i1;
    short i2; // signed<2> i2;
    int   i4; // signed<4> i4;
    long  i8; // signed<8> i8;

    float  flt; // float flt;
    double dbl; // double dbl;

    boolean bool; // boolean bool;

    char u2; // unsigned<2> u2;

// String types
    String str; // string str;

    @Dimension(CHARS_LEN)
    String ch;   // char<CHARS_LEN> ch;

    @Encoding("UTF-16") // UTF-16 produce unicode string type
    String nstr; // nstring nstr;

    @Encoding("UTF-16")
    @Dimension(CHARS_LEN)
    String nch; // nchar<CHARS_LEN> nch

// Nested struct
    NestedStruct nested; // NestedStruct nested;

    @OptionalStruct
    NestedStruct opt_nested; // optional NestedStruct opt_nested;

// Arrays
    @Dimension(ARRAY_LEN)
    int arr_i4[]; // signed<4> arr_i4[ARRAY_LEN];

    @Dimension(ARRAY_LEN)
    byte arr_i1[]; // signed<1> arr_i1[ARRAY_LEN];

    @Dimension(ARRAY_LEN)
    boolean arr_bool[]; //bool arr_bool[ARRAY_LEN];

    @Dimension(ARRAY_LEN)
    String arr_str[]; // string arr_str[ARRAY_LEN];

    @Dimension(ARRAY_LEN)
    NestedStruct arr_nested[]; // NestedStruct arr_nested[ARRAY_LEN];

// Vectors
    int vec_i4[];    // vector<signed<4>> vec_i4;
    byte vec_i1[];   // vector<signed<1>> vec_i1;
    boolean vec_bool[];   // vector<bool> vec_bool;
    String vec_str[]; // vector<string> vec_str;
    NestedStruct vec_nested[]; // vector<NestedStruct> vec_nested;

// Blob
    @Blob
    byte b[]; // blob b;

// Enum
    Color color;

// References
    @References(Record.class)
    long r;    
}

public class Types 
{ 
    static final int PAGE_SIZE = 128;
    static final int DISK_PAGE_SIZE = 4096;
    static final int DISK_CACHE_SIZE = 4*1024*1024;
    static final int DATABASE_SIZE = 16*1024*1024;

    static final int N_RECORDS = 5;

    // Create & fill Java object 
    static Record createRecord(int i) {
        Record rec = new Record();
        int j;

        rec.i1 = (byte) i;
        rec.i2 = (short) i;
        rec.i4 = i;
        rec.i8 = i;

        rec.bool = (i % 2 == 0);

        rec.flt = i;
        rec.dbl = i;

        rec.u2 = (char)('A' + i);

        rec.str = "string " + i;
        rec.ch  = "chars " + i;
        rec.nstr = "nstring " + i;
        rec.nch  = "nchars " + i;

        rec.nested = new NestedStruct(i); // Can't be null
        rec.opt_nested = (i % 2 == 0) ? null : new NestedStruct(i); // OptionalStruct struct can be null

        rec.arr_i4     = new int[Record.ARRAY_LEN]; // eXtremeDB array fields can't be null, java array dimension must be the same as in @Dimension declaration
        rec.arr_i1     = new byte[Record.ARRAY_LEN];
        rec.arr_bool   = new boolean[Record.ARRAY_LEN];
        rec.arr_str    = new String[Record.ARRAY_LEN];
        rec.arr_nested = new NestedStruct[Record.ARRAY_LEN];
        for (j = 0; j < Record.ARRAY_LEN; ++j) {
            rec.arr_i1[j]  = (byte) (i + j);
            rec.arr_bool[j] = i == j;
            rec.arr_i4[j]  = i + j;
            rec.arr_str[j] = "arr_string " + i + " " + j;
            rec.arr_nested[j] = new NestedStruct(i + j);
        }

        rec.vec_i4     = (i % 2 == 0) ? null : new int[i]; // eXtremeDB vectors can be null 
        rec.vec_i1     = (i % 2 == 0) ? null : new byte[i];
        rec.vec_bool   = (i % 2 == 0) ? null : new boolean[i];
        rec.vec_str    = (i % 2 == 0) ? null : new String[i];
        rec.vec_nested = (i % 2 == 0) ? null : new NestedStruct[i];
        if (i % 2 != 0) {
            for (j = 0; j < i; ++j) {
                rec.vec_bool[j]  = i == j;
                rec.vec_i1[j]  = (byte) (i + j);
                rec.vec_i4[j]  = i + j;
                rec.vec_str[j] = "arr_string " + i + " " + j;
                rec.vec_nested[j] = new NestedStruct(i + j);
            }
        }

        rec.b = new byte[i * 100]; // Blob field can be null
        rec.color = Color.Green;

        return rec;
    }

    // Print out object
    static void printRecord(Record rec) {
        System.out.println("Record " + rec.i4);
        System.out.println("-------------------------");
        System.out.println("i1 = " + rec.i1);
        System.out.println("i2 = " + rec.i2);
        System.out.println("i4 = " + rec.i4);
        System.out.println("i8 = " + rec.i8);
        System.out.println("dbl = " + rec.dbl);
        System.out.println("flt = " + rec.flt);
        System.out.println("bool = " + rec.bool);
        System.out.println("u2 = " + rec.u2);
        System.out.println("str = \"" + rec.str + "\"");
        System.out.println("ch = \"" + rec.ch + "\"");
        System.out.println("nstr = \"" + rec.nstr + "\"");
        System.out.println("nch = \"" + rec.nch + "\"");
        
        System.out.println("nested = {" + rec.nested + "}");
        System.out.println("opt_nested = " + ((rec.opt_nested == null) ? "null" : ("{" + rec.nested + "}")));

        System.out.print("rec.arr_i1 = [");
        for (byte i1 : rec.arr_i1) System.out.print(" " + i1);
        System.out.println(" ]");

        System.out.print("rec.arr_bool = [");
        for (boolean b : rec.arr_bool) System.out.print(" " + b);
        System.out.println(" ]");

        System.out.print("rec.arr_i4 = [");
        for (int i4 : rec.arr_i4) System.out.print(" " + i4);
        System.out.println(" ]");

        System.out.print("rec.arr_str = [");
        for (String str : rec.arr_str) System.out.print(" \"" + str + "\"");
        System.out.println(" ]");

        System.out.print("rec.arr_nested = [");
        for (NestedStruct nest : rec.arr_nested) System.out.print(" {" + nest + "}");
        System.out.println(" ]");

        System.out.print("rec.vec_i1 = [");
        for (byte i1 : rec.vec_i1) System.out.print(" " + i1);
        System.out.println(" ]");

        System.out.print("rec.vec_bool = [");
        for (boolean b : rec.vec_bool) System.out.print(" " + b);
        System.out.println(" ]");

        System.out.print("rec.vec_i4 = [");
        for (int i4 : rec.vec_i4) System.out.print(" " + i4);
        System.out.println(" ]");

        System.out.print("rec.vec_str = [");
        for (String str : rec.vec_str) System.out.print(" \"" + str + "\"");
        System.out.println(" ]");

        System.out.print("rec.vec_nested = [");
        for (NestedStruct nest : rec.vec_nested) System.out.print(" {" + nest + "}");
        System.out.println(" ]");

        System.out.println("rec.b : Blob length = " + rec.b.length);

        System.out.println("rec.color=" + rec.color);
        System.out.println("\n\n");
    }

    public static void main(String[] args)
    { 
        Database db = new Database();
        Database.Parameters params = new Database.Parameters();
        params.memPageSize = PAGE_SIZE;
        params.classes = new Class[]{Record.class};       
        //
        // Get config parameters from the command line
        for (String s : args) {
	    if (s.startsWith("license:")) params.license_key = s.substring(8);
        }

        db.open("typesdb", params, DATABASE_SIZE);

        try {
            db.generateMcoFile("types.mco");
        } catch (java.io.IOException e) {
            System.out.println("Can't generate types.mco file");
        }
        Connection con = new Connection(db);

        ////////////////////////////////////////////////////// 
        // Insert data in the database
        ////////////////////////////////////////////////////// 
        con.startTransaction(Database.TransactionType.ReadWrite);
        for (int i = 1; i <= N_RECORDS; i++) { 
            con.insert( createRecord(i) );
        }            
        con.commitTransaction();

        ////////////////////////////////////////////////////// 
        // Read data from the database
        ////////////////////////////////////////////////////// 
        con.startTransaction(Database.TransactionType.ReadOnly);
        // Open cursor by autoid index
        Cursor<Record> cursor = new Cursor<Record>(con, Record.class);
        for (Record rec : cursor) {
            printRecord(rec);
        }
        cursor.close();
        con.commitTransaction();

        con.disconnect();
        db.close();
    }
}

