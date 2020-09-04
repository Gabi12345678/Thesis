/**
 * Class for manipulation of data represented as an integer in format YYYYMMDD, for example 20131231
 */
public class IntDate
{
    public static int construct(int day, int month, int year) { 
        return year*10000 + month*100 + day;
    }

    public static int day(int dmy) { 
        return dmy % 100;
    }

    public static int month(int dmy) { 
        return dmy/100%100;
    }
    
    public static int year(int dmy) { 
        return dmy / 10000;
    }
}

