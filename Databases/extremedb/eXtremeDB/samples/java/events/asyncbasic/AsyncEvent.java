import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.io.Console;

import com.mcobject.extremedb.*;

public class AsyncEvent
{
    static final int PAGE_SIZE = 128;
    static final int DATABASE_SIZE = 16*1024*1024;

    static final int NOBJECTS = 5;

    static final int WAIT_TIME = 100;

    private static int obj_count = 0;

    static String     license_key;
    /* Creates a new object triggering the <new_object> event */
    public static long newObject(Connection con)
    {      
        con.startTransaction(Database.TransactionType.ReadWrite);

        MyClass obj = new MyClass();
        obj.key = obj_count++;
        obj.value = obj.key;
        long autoid = con.insert(obj);
        
        con.commitTransaction();
        return autoid;
    }

    public static void updateObject(Connection con, long autoid)
    {
        con.startTransaction(Database.TransactionType.ReadWrite);

        // By default tries to use autoid
        Cursor<MyClass> cursor = new Cursor<MyClass>(con, MyClass.class); 

        MyClass obj = cursor.find(autoid);
        
        obj.value++;
        cursor.update();
        con.commitTransaction();
    }

    public static void deleteObject(Connection con, long autoid)
    {
        con.startTransaction(Database.TransactionType.ReadWrite);
        Cursor<MyClass> cursor = new Cursor<MyClass>(con, MyClass.class);

        MyClass obj = cursor.find(autoid);
        obj.value++;
        cursor.remove();
        con.commitTransaction();
    }

    public static void main(String[] args) 
    { 
        AsyncEvent program = new AsyncEvent();
        program.perform();
        //
        // Get config parameters from the command line
        for (String s : args) {
	    if (s.startsWith("license:")) license_key = s.substring(8);
        }
    }

    void perform() {
        Database db = new Database();
        Database.Parameters parameters = new Database.Parameters();

        parameters.memPageSize = PAGE_SIZE;
        parameters.classes = new Class[]{MyClass.class};        
        parameters.license_key = license_key;

        System.out.println("Sample 'AsyncEvent' demonstrates asynchronous event operations for Java.\n");

        // Open and connect to database 
        db.open("AsyncEventDb", parameters, DATABASE_SIZE);
        try {
            // Generate database schema file
            db.generateMcoFile("AsyncEventDb.mco");

            Connection con = new Connection(db);

            StartHandlers(db);

            for (int i = 0; i < NOBJECTS; i++) {
                Thread.sleep(WAIT_TIME);

                long last_id;
                last_id = newObject(con);
                updateObject(con, last_id);
                deleteObject(con, last_id);
            }

            exit = true;

            Thread.sleep(WAIT_TIME);

            System.out.println("Releasing events ...");
            /* Release all the database events. This will cause the appropriate
            wait function to return MCO_S_EVENT_RELEASED */
            con.releaseAllEvents();

            System.out.println("Stopping threads ...");
            /* Wait until all the threads are finished */
            stopThreads();
           
            // Disconnect and close database, stop runtime
            con.disconnect();
            db.close();
            System.out.println("Finished. Press enter to continue...");
            if (System.console() != null) {
                System.console().readLine();
            }
        } catch (IOException e) {
            System.out.println("Error generating mco file");
            e.printStackTrace();
        } catch (InterruptedException e) {
            System.out.println("Error generating mco file");
            e.printStackTrace();
        }
    }

    private static boolean exit = false;

    private class EventThread extends Thread
    {
        public Database db;
        public String event_name;

        public EventThread(Database db, String EventName)
        {
            this.db = db;
            this.event_name = EventName;
        }

        public void run()
        {
            Connection con = new Connection(db);
            System.out.println("Starting thread for event "+ event_name);
            try
            {
                while (!exit)
                {
                    con.waitEvent(event_name);
                    System.out.println("Event " + event_name);
                }
            }
            catch (DatabaseError x)
            {
                if (x.errorCode >= 50)
                {
                    // Errors
                    System.out.println("ERROR: Thread for event " + 
                            event_name + " finished with error " + x.errorCode + 
                            ": " + x.getMessage());
                }
                else
                {
                    System.out.println("Thread for event " + event_name + 
                            " finished with code " + x.errorCode);
                }
            }
            con.disconnect();
            System.out.println("Exiting thread for event " + event_name);
        }
    }
    
    private static List<Thread> threads = new ArrayList<Thread>();

    private void StartHandlers(Database db)
    {
        String[] events = new String[] { 
            "NewEvent", 
            "FieldUpdateEvent", 
            "DeleteEvent", 
            "DeleteAllEvent", 
            "CheckPointEvent", 
            "UpdateEvent"
        };

        for (int i = 0; i < events.length; i++)
        {
            String evt = events[i];
            EventThread t = new EventThread(db, evt);
            t.start();

            threads.add(t);
        }
    }

    /* Wait until all the threads are finished */
    private static void stopThreads() throws InterruptedException
    {
        for (int i =0; i < threads.size(); i++)
        {
            Thread t = threads.get(i);
            t.join();
        }
    }
}
