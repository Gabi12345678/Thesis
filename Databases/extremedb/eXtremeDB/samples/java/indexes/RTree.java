import java.util.*;
import com.mcobject.extremedb.*;

@Persistent(list=true) // Store class in eXtremeDB database, declare list index
class Rect
{
    @Dimension(4)
    @Indexable(type=Database.IndexType.RTree) // Declare rtree index on "square" field
    public short[] square;
    
    Rect(int lx, int ly, int rx, int ry) {
        square = new short[4];
        square[0] = (short) lx;
        square[1] = (short) ly;
        square[2] = (short) rx;
        square[3] = (short) ry;
    }
    
    Rect() {}; // Default constructor needed for eXtremeDB
    
    public String toString() { return "(" + square[0] + "," + square[1] + ") - (" + square[2] + "," + square[3] + ")"; }
}

public class RTree
{
    static final int PAGE_SIZE = 128;
    static final int DATABASE_SIZE = 16*1024*1024;

    static final int NRECTS = 100000;
    static final int SHOW_FIRST = 5;
    static final int MCO_BOUND = 32760;

    // Print out objects in direct or reverse order 
    static int iterateRects(Cursor<Rect> cursor, boolean reverse_order) {
        int i = 0;
        
        for (Rect rect = (reverse_order) ? cursor.last() : cursor.first(); 
             rect != null; 
             rect = (reverse_order) ? cursor.prev() : cursor.next()) 
        {
            if (i++ < SHOW_FIRST) {
                System.out.println("\t" + i + "." + rect);
            }
        }

        if (i > SHOW_FIRST) {
            System.out.println("\t...");
        }
        return i;
    }


    public static void main(String[] args) throws Exception
    { 
        int i;
        Database db = new Database();
        Database.Parameters params = new Database.Parameters();
        Cursor<Rect> cursor;
        Rect rect1 = new Rect( 25, 25, 50, 35 );
        Rect rect2 = new Rect(  5, 45, 60, 65 );
        Rect rect3 = new Rect( 20, 30, 85, 50 );
        Rect rect4 = new Rect( 10, 55, 45, 60 );
        Rect point = new Rect( 10, 10, 10, 10 );

        params.memPageSize = PAGE_SIZE;
        params.classes = new Class[]{Rect.class};        
        // Get config parameters from the command line
        for (String s : args) {
	    if (s.startsWith("license:")) params.license_key = s.substring(8);
        }

        System.out.println("Sample 'RTree' demonstrates eXtremeDB RTree index operations.\n");

        db.open("rtree-db", params, DATABASE_SIZE);
        db.generateMcoFile("rtree-db.mco");  // Generate database schema file
        Connection con = new Connection(db);

        con.startTransaction(Database.TransactionType.ReadWrite);        
      
        // Insert rects 1 and 2
        con.insert(rect1);
        con.insert(rect2);

        // Insert rect 3 temporarily, then delete it after exact match found
        con.insert(rect3);
      
        con.commitTransaction();


        con.startTransaction(Database.TransactionType.ReadWrite);        
        // Create cursor
        cursor = new Cursor<Rect> (con, Rect.class, "square");
        System.out.println("\n\tIterate through cursor with no search condition : ");
        i = iterateRects(cursor, false);
        System.out.println("\tFound " + i + " total rects");

        // Look up the sample rect and delete it
        System.out.println("\n\n\tSearch(Operation.Equals, '" + rect3 + "');");
        if (cursor.search(Cursor.Operation.Equals, rect3.square)) {
            i = iterateRects(cursor, false);
            System.out.println("\tFound " + i + " rect(s)");
            if (i > 0) {
                System.out.println("\n\tDelete the found rect...");
                cursor.first();
                cursor.remove();
            }
        } else {
            System.out.println("\tUnable to find any matching rect");
        }

        // Look up rects that overlap ...
        System.out.println("\n\n\tSearch(Operation.Overlaps, '" + rect3 + "');");

        if (cursor.search(Cursor.Operation.Overlaps, rect3.square)) {
            i = iterateRects(cursor, false);
            System.out.println("\tFound " + i + " rects");
        } else {
            System.out.println("\n\tCouldn't find any suitable rect");
        }

        // Look up rects that contain a specified rect ...
        System.out.println("\n\n\tSearch(Operation.Contains, '" + rect4 + "');");

        if (cursor.search(Cursor.Operation.Contains, rect4.square)) {
            i = iterateRects(cursor, false);
            System.out.println("\tFound " + i + " rects");
        } else {
            System.out.println("\n\tCouldn't find any suitable rect");
        }

        // Order all rects by distance from a specified point (rect with 0 height and width) ...
        System.out.println("\n\n\tSearch(Operation.Neighbourhood, '" + point + "');");

        if (cursor.search(Cursor.Operation.Neighbourhood, point.square)) {
            i = iterateRects(cursor, false);
            System.out.println("\tFound " + i + " rects");

            System.out.println("\tIn reverse order :");
            cursor.last();
            i = iterateRects(cursor, true);
            System.out.println("\tFound " + i + " rects");
        } else {
            System.out.println("\n\tCouldn't find any suitable rect");
        }

        // Delete all objects
        System.out.println("\n\tDelete all rects\n");
        con.removeAll(Rect.class);
        cursor.close();
        con.commitTransaction();
        
        con.disconnect();
        db.close();
    }
}
