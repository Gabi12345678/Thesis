import com.mcobject.extremedb.*;

class _X
{
    @Indexable(type=Database.IndexType.BTree, unique=false)
    int id;
    String str;
}

public class IncBackup
{
    static final int PAGE_SIZE = 128;
    static final int DATABASE_SIZE = 16*1024*1024;
    static final int MAX_RECORDS = 1000;

    public static void main(String[] args)
    {
        Cursor<_X> cursor;
        Database db = new Database();
        Database.Parameters params = new Database.Parameters();
        params.memPageSize = PAGE_SIZE;
        params.classes = new Class[]{_X.class};
        params.backupMapSize = DATABASE_SIZE;
        params.backupMapFile = "BackupMap";
        params.backupMinPages = 10;
        params.backupMaxPasses = 10;
        params.mode |= Database.DB_INCREMENTAL_BACKUP;

        // Open and connect to database
        db.open("IncBackup", params, DATABASE_SIZE);
        Connection con = new Connection(db);

        String fileName = "backup.bak";
        String label = "label", label2 = "label2", label3 = "label3";

        int i;
        _X x = new _X();
        for (i = 0; i < MAX_RECORDS; i++) //insert 1000 records
        {
            con.startTransaction(Database.TransactionType.ReadWrite);
            x.id = i;
            x.str = "ARecord";
            con.insert(x);
            con.commitTransaction();
        }
        System.out.println("1000 Records inserted into in-memory database");

        //create full backup
        con.createBackup(fileName, label, Database.BackupType.Auto);
        System.out.println("1st full backup");

        _X a = new _X();
        for (i = MAX_RECORDS; i < (MAX_RECORDS + 10); i++) //insert 10 more records
        {
            con.startTransaction(Database.TransactionType.ReadWrite);
            a.id = i;
            a.str = "ARecord";
            con.insert(a);
            con.commitTransaction();
        }
        System.out.println("10 more records inserted into in-memory database");

        //create 1st incremental backup
        con.createBackup(fileName, label2, Database.BackupType.Incremental);
        System.out.println("1st Incremental backup");
        _X p = new _X();
        for (i = (MAX_RECORDS + 10); i < ( MAX_RECORDS + 20); i++) //insert 10 more records
        {
            con.startTransaction(Database.TransactionType.ReadWrite);
            p.id = i;
            p.str = "ARecord";
            con.insert(p);
            con.commitTransaction();
        }
        System.out.println("10 more records inserted into in-memory database");

        //create 2nd incremental backup
        con.createBackup(fileName, label3, Database.BackupType.Incremental);
        System.out.println("2nd Incremental backup");

        // Disconnect and close database, stop runtime
        con.disconnect();
        db.close();

        Database db2 = new Database();
        Database.Parameters params2 = new Database.Parameters();
        params2.memPageSize = PAGE_SIZE;
        params2.classes = new Class[]{_X.class};
        params2.backupMapSize = DATABASE_SIZE;
        params2.backupMapFile = "BackupMap";
        params2.backupMinPages = 10;
        params2.backupMaxPasses = 10;
        params2.mode |= Database.DB_INCREMENTAL_BACKUP;

        // Open and connect to database
        db2.open("IncBackup", params2, DATABASE_SIZE);
        Connection con2 = new Connection(db2);

        //restore
        con2.restoreBackup(fileName, label2);

        System.out.println("\nRecords of 1st incremental backup:");
        con2.startTransaction(Database.TransactionType.ReadOnly);
        cursor = new Cursor<_X>(con2,_X.class, "id");
        for (_X o : cursor)
        {
            System.out.println("Id: " + o.id + " Value: " + o.str);
        }
        cursor.close();
        con2.rollbackTransaction();

        // Disconnect and close database, stop runtime
        con2.disconnect();
        db2.close();

        // Delete backup file to avoid error at next start
        java.io.File file = new java.io.File(fileName);
        file.delete();
    }
}
