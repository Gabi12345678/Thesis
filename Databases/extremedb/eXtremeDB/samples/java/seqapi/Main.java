import com.mcobject.extremedb.*;

public class Main
{
    static final int N_QUOTES = 1000;
    static final int N_SYMBOLS = 10;

    public static void main(String[] args) 
    { 
        Database db = Create.database(args);
        Connection con = new Connection(db);

        con.setCommitPolicy(Database.CommitPolicy.NoSync);

        Create.randomData(con, N_SYMBOLS, N_QUOTES);

        Query.iterate(con);
        Query.interval(con);
        Query.apply(con);
        Query.filter(con);
        Query.correlation(con);
        Query.grandAggregate(con);
        Query.windowAggregate(con);
        Query.gridAggregate(con);
        Query.groupByAggregate(con);
        Query.hashAggregate(con);
        Query.sort(con);
        Query.top(con);
        Query.VWAP(con);

        con.disconnect();
        db.close();
    }
}