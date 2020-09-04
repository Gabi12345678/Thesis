import com.mcobject.extremedb.*;

// Class for Department data 
class Department_Data
{
    String code;
    String name;

    Department_Data(String code, String name)
    { 
        this.code = code;
        this.name = name;
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

@Persistent(autoid = true)    
class Department 
{
    @Indexable(type=Database.IndexType.BTree, unique=true) // Declare unique tree index by "code" field
    String code;
    String name;
    
    long[] employees;
}

@Persistent(autoid = true)
//@Index(name="byDept_EmployeeName", keys={@Key("dept_no"), @Key("name")}, unique=true)   
class Employee 
{ 
    @Indexable(type=Database.IndexType.BTree, unique=true) // Declare unique tree index by "name" field
    String name;
    long dept;
}

public class Autoid_vector
{
    static final int PAGE_SIZE = 128;
    static final int DATABASE_SIZE = 16*1024*1024;
    static final int VECTOR_SIZE = 10;

    public static void main(String[] args) throws Exception
    {     
        final int N_DEPARTMENTS = 4;
        Department_Data[] DD = new Department_Data[N_DEPARTMENTS];
        DD[0] = new Department_Data("Acct", "Accounting");
        DD[1] = new Department_Data("Eng", "Engineering");
        DD[2] = new Department_Data("CS", "Customer Service");
        DD[3] = new Department_Data("TS", "Technical Support");

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
        System.out.println("Sample 'autoid_vector' demonstrates using a vector of autoids to join two classes.\n");
        // Open and connect to database 
        db.open("autoid_vec_db", params, DATABASE_SIZE);
        db.generateMcoFile("autoid_vec_db.mco"); // Generate database schema file
        Connection con = new Connection(db);
        
        // Create and insert Department objects
        System.out.println("\nCreate Departments:\n");
        con.startTransaction(Database.TransactionType.ReadWrite);        
        for (short i = 0; i < N_DEPARTMENTS; i++)
        {
            Department dept = new Department();
            dept.name = DD[i].name;
            dept.code = DD[i].code;
            // Allocate space for Employee autoids and initialize to 0
            dept.employees = new long[VECTOR_SIZE];
            for (short j = 0; j < VECTOR_SIZE; j++) dept.employees[j] = 0;
            long autoid = con.insert(dept); 
            System.out.println("\t" + i + ") " + dept.code + ", " + dept.name + ", Autoid=" + autoid);  
        }
        con.commitTransaction();
        
        // Create and insert Employee objects
          System.out.println("\nCreate employees and join each to a department:\n"); 
          for (short i = 0; i < N_EMPLOYEES; i++)
          {
              // Find the Department object for this Employee by the BTree index on Department.code
              con.startTransaction(Database.TransactionType.ReadWrite);
              Cursor<Department> cursor = new Cursor<Department>(con, Department.class, "code");
              Department dept = cursor.find(ED[i].deptCode);
              if (null != dept)
              {
                    // Find the first vacant Employee vector element
                    for (short j = 0; j < VECTOR_SIZE; j++)
                    {
                        if (0 == dept.employees[j])
                        {
                            // Create Employee object and store its autoid in vector Department.employees
                            Employee emp = new Employee();
                            emp.name = ED[i].name;
                            // Assign the Department autoid for this Department object and insert new Employee object
                            emp.dept = cursor.getAutoId();
                            dept.employees[j] = con.insert(emp);
                            cursor.update();        // Assure that the inserted autoid is made permanent 
                            System.out.println("\t" + i + ") " + emp.name + ", Department.Autoid=" + emp.dept);
                            break;
                        }
                    }
              }
              else
              {
                  System.out.println("\tDepartment.code (" + ED[i].deptCode + ") not found!");
              }
              con.commitTransaction();
              cursor.close();
          }
         // Search for all Employee objects from a specified Employee's Department
        String search_name = "William";
        con.startTransaction(Database.TransactionType.ReadOnly);
        // 1. Find the Employee object by name and extract its autoid
        Cursor<Employee> cursor1 = new Cursor<Employee>(con, Employee.class, "name");
        Employee emp1 = cursor1.find(search_name);
        if ( null != emp1 )
        {
            // 2. Find the Department object by its autoid and display the Department name
            Cursor<Department> cursor2 = new Cursor<Department>(con, Department.class);
            Department dept1 = cursor2.find(emp1.dept);
            System.out.println("\n\n" + search_name + "'s co-workers in " + dept1.name + " are:\n");
            // 3. Scroll through the vector of Employee autoids, find the Employee object and display its name
            Cursor<Employee> cursor3 = new Cursor<Employee>(con, Employee.class);
            for (short j = 0; j < VECTOR_SIZE && 0 != dept1.employees[j]; j++)
            {
                 // Skip if this is the autoid of the "search_name" object
                if (cursor1.getAutoId() != dept1.employees[j])
                {
                    Employee emp = cursor3.find(dept1.employees[j]);
                    System.out.println("\t" + emp.name);
                }
            }
            cursor1.close();
            cursor2.close();
            cursor3.close();
        }
        else
        {
            System.out.println("\n\t" + search_name + "not found!"); 
        }
        con.commitTransaction();
        cursor1.close();
        
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
        
