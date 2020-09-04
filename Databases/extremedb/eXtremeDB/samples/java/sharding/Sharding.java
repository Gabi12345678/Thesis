import com.mcobject.extremedb.*;

import java.io.FileWriter;
import java.io.IOException;
import java.text.DateFormat;
import java.util.Date;
import java.util.Calendar;
import java.util.Random;


public class Sharding
{
    static int n_quotes = 2000;  // Quotes per trading day.
    static int nsymbols = 100;
    
    static String [] nodes = new String[]{"localhost:5000", "localhost:5001"};  
    static final int TRADING_DAY_LENGTH = 1000 * 60 * (6*30 + 30);
    static int mdays[] = new int[] {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    private static Random r;
    
    public static boolean is_working_day(Date dt)
    {
        Calendar c = Calendar.getInstance();
        c.setTime(dt);
        int dayOfWeek = c.get(Calendar.DAY_OF_WEEK);
        return !(dayOfWeek == 1 || dayOfWeek == 7);
    }

    public static long [] createRandTimes(long start, long length, int n_quotes)
    {
        long dt = 2*length/n_quotes;
        
        long [] ret = new long[n_quotes];
        for (int i = 0; i < n_quotes; i++) {
            ret[i] = (start*2 + i*dt + r.nextInt((int) dt))/2;
        }
        return ret;
    }
    
    public static void generate_daily_records(SqlRemoteConnection conn, Date dt, String [] symbols)
    {
        long int_start = dt.getTime();

        long [] times = createRandTimes(int_start, TRADING_DAY_LENGTH, n_quotes); 
        float[] bids = new float[n_quotes];
        float[] asks = new float[n_quotes];
        int  [] bidsizes = new int[n_quotes];
        int  [] asksizes = new int[n_quotes];

        DateFormat df = DateFormat.getDateInstance(DateFormat.SHORT);
        System.out.println(df.format(dt));
        
        for (int i = 0; i < n_quotes; i++) {
            bids[i] = (float) (r.nextFloat() * 1000.0);
            asks[i] = (float) (r.nextFloat() * 1000.0);
            bidsizes[i] = (int)r.nextInt(1000);
            asksizes[i] = (int)r.nextInt(1000);
        }
        
        for (int i = 0; i < symbols.length; i++) {
            conn.executeStatement("?:INSERT OR UPDATE INTO Quotes(Symbol, qTime, Bid, BidSize, Ask, AskSize) VALUES (?,?,?,?,?,?)",
                    symbols[i], times, bids, bidsizes, asks, asksizes);
        }
    }
    
    public static String [] createSymbols(int nsymbols)
    {
        char [] characters = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L'};

        String [] symbols = new String[nsymbols];
        int i = 0;
        for (char c1 : characters) {
            for (char c2 : characters) {
                for (char c3 : characters) {
                    String s = String.format("%s%s%s", c1,c2,c3);
                    symbols[i++] = s;
                    if (i >= nsymbols)
                        return symbols; 
                }
            }
        }
        return null;
    }
    
    public static void Create(SqlRemoteConnection conn)
    {
        // Create symbols
        String [] symbols = createSymbols(nsymbols);
                
        for (int mon = 0; mon < 12; mon ++) {
            for (int mday = 0; mday < mdays[mon]; mday++) {
                Calendar cal = Calendar.getInstance();
                cal.set(2014, mon, mday);
                Date dt = cal.getTime();
                if (is_working_day(dt)) {
                    generate_daily_records(conn, dt, symbols);
                }
            }
        }
    }
    
    public static void perform_hibid(SqlRemoteConnection conn)
    {
        Calendar cal = Calendar.getInstance();
        cal.set(2014,  1,  1,  9, 30);
        cal.set(2014, 12, 30, 16, 00);
        Date dstart = cal.getTime();
        Date dfinish = cal.getTime();
        long start = dstart.getTime();
        long finish = dfinish.getTime();
        
        System.out.println("Calculating high bids over symbols..." );
        SqlResultSet res = conn.executeQuery("SELECT Symbol, seq_ignore(seq_search(qTime, ?, ?)) as t, seq_max (Bid@t) FROM Quotes", start, finish);
        
        try {
            FileWriter fw = new FileWriter("hibid.csv");
            fw.write("SYMBOL, HIBID\n");
            for (SqlTuple row : res) {
                fw.write(String.format("%s, %f\n", row.get(0), row.get(2)));
            }
            fw.close();
        } catch (IOException e) { 
            e.printStackTrace();
        }
    }
    
    public static void perform_stats(SqlRemoteConnection conn)
    {
        Calendar cal = Calendar.getInstance();
        cal.set(2014, 05, 15, 15, 12);
        Date dstart = cal.getTime();
        long start = dstart.getTime();
        long finish = start * 60 * 1000;
        
        String stats_query = "SELECT Symbol, seq_ignore(seq_search(qTime, ?, ?)) as tq,\n" +
                "seq_last_real(Bid@tq),\n" +
                "seq_last_real(Ask@tq),\n" +
                "seq_avg(seq_sub(Bid@tq, Ask@tq)) as spread,\n" +
                "(seq_first_real(Bid@tq) +  seq_first_real(Ask@tq))/2,\n" +
                "(seq_last_real(Bid@tq) +  seq_last_real(Ask@tq))/2\n" +
                "    FROM Quotes";
                
        System.out.println("Calculating 1-hour statistics" );
        SqlResultSet res = conn.executeQuery(stats_query, start, finish);
        try {
            FileWriter fw = new FileWriter("stats.csv");
            fw.write("SYMBOL,LASTBID,LASTASK,SPREAD,FIRST_MID,LAST_MID\n");
            for (SqlTuple row : res) {
                fw.write(String.format("%s,%f,%f,%f,%f,%f\n", 
                        row.get(0),row.get(2),row.get(3),row.get("spread"),row.get(5),row.get(6)));
            }
            fw.close();
        } catch (IOException e) { 
            e.printStackTrace();
        }
    }
        
    public static void main(String[] args)
    {
        // Display program description
        System.out.println("Sample 'Sharding' demonstrates databae sharding using Java.\n");
        int config = Database.MCO_CFG_SQL_SUPPORT;

        // Get config parameters from the command line
        for (String s : args) {
            if ("debug".equals(s))  config |= Database.MCO_CFG_DEBUG_LIBRARY;
        }

        r = new Random();
        
        Database db = new Database(config);
        SqlRemoteConnection con = new SqlRemoteConnection(nodes);

        System.out.println("Creating database");
        
        con.executeStatement("create table Quotes (Symbol char(4) primary key, qTime sequence(longint asc), Bid sequence(real), Ask sequence(real), BidSize sequence(integer), AskSize sequence(integer))");
        Create(con);
        
        System.out.println("Database created. Running queries...");

        perform_hibid(con);
        perform_stats(con);
        
        con.disconnect();
        System.out.println("Disconnect from the database");
    }
}
