import com.mcobject.extremedb.*;

class Address 
{ 
    String country;
    String city;
    String street;

    public String toString() { 
        return country + " " + city + " " + street;
    }

    Address(String country, String city, String street) {
        this.country = country;
        this.city = city;
        this.street = street;
    }
}

@Indexes({@Index(name="byName", keys={@Key("lastName"), @Key("firstName")}, unique=true),
                 @Index(name="byAddress", 
                        keys={@Key("address.country"), @Key("address.city"), @Key("address.street")}, 
                        unique=false),
                 @Index(name="bySalary", keys={@Key(value="salary", descending=true)})})
class Employee 
{ 
    String  firstName;
    String  lastName;
    Address address;
    long    salary;

    public String toString() { 
        return firstName + " " + lastName + " Address=(" + address + "), salary=" + salary;
    }

    Employee(String firstName, String lastName, Address address, long salary) { 
        this.firstName = firstName;
        this.lastName = lastName;
        this.address = address;
        this.salary = salary;
    }
}

public class Compound 
{
    static final int PAGE_SIZE = 128;
    static final int DATABASE_SIZE = 16*1024*1024;

    public static void main(String[] args) throws Exception
    {   
        Database db = new Database();
        Database.Parameters params = new Database.Parameters();
        params.memPageSize = PAGE_SIZE;
        params.classes = new Class[]{Employee.class};        
        // Get config parameters from the command line
        for (String s : args) {
	    if (s.startsWith("license:")) params.license_key = s.substring(8);
        }

        System.out.println("Sample 'Compound' demonstrates definition and usage \nof compound (multifield) indices.\n");
        // Open and connect to database 
        db.open("compound-db", params, DATABASE_SIZE);
        db.generateMcoFile("compound-db.mco"); // generate database schema file

        Connection con = new Connection(db);
        // Fill database
        con.startTransaction(Database.TransactionType.ReadWrite);        
        con.insert(new Employee("John", "Smith", new Address("USA", "Newyork", "Green valley, 5"), 100000));
        con.insert(new Employee("Bob", "Norton", new Address("USA", "Boston", "Stonky road, 1"), 80000));
        con.insert(new Employee("Peter", "Dannon", new Address("USA", "Washington", "Outlook drive, 23"), 150000));
        con.commitTransaction();

        // List all objects in desired order
        con.startTransaction(Database.TransactionType.ReadOnly);        
        Cursor<Employee> cursor = new Cursor<Employee>(con, Employee.class, "bySalary");
        System.out.println("Employees sorted by salary:");
        for (Employee e : cursor) {
            System.out.println(e);            
        }
        System.out.println("------------------");
        cursor.close();
        System.out.println("Employees sorted by name:");
        cursor = new Cursor<Employee>(con, Employee.class, "byName");
        for (Employee e : cursor) {
            System.out.println(e);            
        }
        System.out.println("------------------");
        cursor.close();
        System.out.println("Employees sorted by address:");
        cursor = new Cursor<Employee>(con, Employee.class, "byAddress");
        for (Employee e : cursor) {
            System.out.println(e);            
        }
        System.out.println("------------------");
        cursor.close();
        con.commitTransaction();
        
        // Index search
        con.startTransaction(Database.TransactionType.ReadOnly);        
        cursor = new Cursor<Employee>(con, Employee.class, "byName");
        Employee e = cursor.find("Norton", "Bob");
        assert(e != null && e.lastName.equals("Norton"));
        cursor.close();
        
        cursor = new Cursor<Employee>(con, Employee.class, "byAddress");
        assert(cursor.search(Cursor.Operation.Equals, "USA", "Newyork", "Green valley, 5"));
        assert(cursor.next().lastName.equals("Smith"));
        cursor.close();
        con.commitTransaction();

        // Remove all data
        con.startTransaction(Database.TransactionType.ReadWrite);        
        con.removeAll(Employee.class);
        con.commitTransaction();
               
        // Disconnect and close database, stop runtime 
        con.disconnect();
        db.close();
        
         
    }
}
