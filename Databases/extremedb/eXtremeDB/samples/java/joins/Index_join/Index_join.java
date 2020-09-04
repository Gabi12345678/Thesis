import com.mcobject.extremedb.*;

// Class for Department data 
class Department_Data
{
    String code;
    String name;
    int dept_no;

    Department_Data(String code, String name, int dept_no)
    { 
        this.code = code;
        this.name = name;
        this.dept_no = dept_no;
    }
}

// Class for associating Employees to Departments
class  Employee_Department
{
    String name;
    String deptCode;
 
    Employee_Department(String name, String deptCode)
    { 
        this.name = name;
        this.deptCode = deptCode;
    }
}

@Persistent()    
class Department 
{
    @Indexable(type=Database.IndexType.BTree, unique=true) // Declare unique tree index by "code" field
    String code;
    String name;
    @Indexable(type=Database.IndexType.BTree, unique=true) // Declare unique Tree index by "dept_no" field
    int dept_no;
}

@Persistent()
@Index(name="byDept_EmployeeName", keys={@Key("dept_no"), @Key("name")}, unique=true)   
class Employee 
{ 
    @Indexable(type=Database.IndexType.BTree, unique=true) // Declare unique tree index by "name" field
    String name;
    int dept_no;
}

public class Index_join
{
    static final int PAGE_SIZE = 128;
    static final int DATABASE_SIZE = 16*1024*1024;

    public static void main(String[] args) throws Exception
    {     
        final int N_DEPARTMENTS = 4;
        Department_Data[] DD = new Department_Data[N_DEPARTMENTS];
        DD[0] = new Department_Data("Acct", "Accounting", 101);
        DD[1] = new Department_Data("Eng", "Engineering", 102);
        DD[2] = new Department_Data("CS", "Customer Service", 103);
        DD[3] = new Department_Data("TS", "Technical Support", 104);

        // Define Employees and associate them with Departments
        final int N_EMPLOYEES = 12;
        Employee_Department[] ED = new Employee_Department[N_EMPLOYEES];
        ED[0] = new Employee_Department("John", "Acct");
        ED[1] = new Employee_Department("Samuel", "Acct");
        ED[2] = new Employee_Department("Thomas", "Acct");
        ED[3] = new Employee_Department("David", "Eng");
        ED[4] = new Employee_Department("James", "Eng");
        ED[5] = new Employee_Department("Robert", "Eng");
        ED[6] = new Employee_Department("William", "CS");
        ED[7] = new Employee_Department("Kevin", "CS");
        ED[8] = new Employee_Department("Alex", "CS");
        ED[9] = new Employee_Department("Daniel", "CS");
        ED[10] = new Employee_Department("Diego", "CS");
        ED[11] = new Employee_Department("Brandon", "TS");
        
        
        Database db = new Database();
        Database.Parameters params = new Database.Parameters();
        params.memPageSize = PAGE_SIZE;
        params.classes = new Class[]{Department.class, Employee.class};        
        // Get config parameters from the command line 
        System.out.println("Sample 'index_join' demonstrates using an index to join two classes.\n");
        // Open and connect to database 
        db.open("index_join_db", params, DATABASE_SIZE);
        db.generateMcoFile("index_join_db.mco"); // Generate database schema file
        Connection con = new Connection(db);
        
        // Create and insert Department objects
        System.out.println("\nCreate Departments:\n");
        con.startTransaction(Database.TransactionType.ReadWrite);        
        for (short i = 0; i < N_DEPARTMENTS; i++)
        {
            Department dept = new Department();
            dept.name = DD[i].name;
            dept.code = DD[i].code;
            dept.dept_no = DD[i].dept_no;
            con.insert(dept); 
            System.out.println("Code " + dept.code + ", Name " + dept.name + ", Dept_no = " + dept.dept_no);  
        }
        con.commitTransaction();
        
        System.out.println("\nCreate employees and join each to a department:\n");     
        con.startTransaction(Database.TransactionType.ReadWrite);
        // Find Department by code; extract dept_no; create Employee and assign name, dept_no
        Cursor<Department> cursor = new Cursor<Department>(con, Department.class, "code");
        for (short i = 0; i < N_EMPLOYEES; i++)
        {  
            Department dept = cursor.find(ED[i].deptCode);
            Employee emp = new Employee();
            emp.dept_no = dept.dept_no;
            emp.name = ED[i].name;
            con.insert(emp);
            System.out.println("\tEmployee: " + emp.name + "Dept_no: " + emp.dept_no);
        }
        con.commitTransaction();
        cursor.close();
        
        // Search for all Employee objects from a specified Employee's Department
        String search_name = "William";
        con.startTransaction(Database.TransactionType.ReadOnly);
        // 1. Find the Employee object by name and extract dept_no
        Cursor<Employee> cursor1 = new Cursor<Employee>(con, Employee.class, "name");
        Employee emp1;     
        emp1 = cursor1.find(search_name);
        
        // 2. Find the Department object by its dept_no and display the Department name
        Cursor<Department> cursor2 = new Cursor<Department>(con, Department.class, "dept_no");
        Department d = cursor2.find(emp1.dept_no);
        System.out.println("\n\nFind " + search_name + "'s co-workers in " + d.name + " :\n");
        
        // 3. Position the cursor in the byDept_EmployeeName compound index to the first object with this dept_no
        con.startTransaction(Database.TransactionType.ReadWrite);
        Cursor<Employee> cursor3 = new Cursor<Employee>(con, Employee.class, "byDept_EmployeeName");
        {
           if (cursor3.search(Cursor.Operation.GreaterOrEquals, emp1.dept_no, ""))
           {     
               for (Employee e : cursor3)
               {
                   if(e.dept_no != emp1.dept_no)  // Exit loop when Dept_no is no longer equal
                   {
                      break;
                   }
                   else if ( !(e.name.equals(search_name)) )  // exclude serch_name from results
                   {
                       System.out.println("\t" + e.name);
                   }
               }
               cursor3.moveNext();
           }              
        }
        con.commitTransaction();
        cursor1.close();
        cursor2.close();
        cursor3.close();
        
        // Remove all data
        con.startTransaction(Database.TransactionType.ReadWrite);        
        con.removeAll(Employee.class);
        con.removeAll(Department.class);
        con.commitTransaction();
        
        // Disconnect and close database, stop runtime 
        con.disconnect();
        db.close();
        System.out.println("\nPress enter to finish");
        db.close();
    }
}
        
