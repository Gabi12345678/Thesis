/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <commonSQL.h>
#include "persondb.h"
#include <ctype.h>

using namespace McoSql;

char sample_descr[] = 
"Sample 'imp_csv' initializes an all-in-memory database\n"
"from an external .csv file.\n";
char sample_explanation[] = 
"\n---------------------------------------------------------------------\n"
"If you respond 'Y' to the prompt, the file Person.csv will be generated.\n"
"Either way, Person.csv will be opened from the working directory and its\n"
"contents read and inserted into the database.  Then the contents of table\n"
"Person are listed.\n"
"\n---------------------------------------------------------------------\n";

const char * db_name = "persondb";
const size_t MAX_LINE_LENGTH = 2048;
const int n_persons = 10;
const char * fileName = "Person.csv";
const char * tableName = "Person";

// Define the structure correponding to database table Person
struct _Person
{
  char const* name;
  uint4       ordinal;
};

/* forward declarations */
void generate_persons_file();
void import_data( McoSqlEngine& engine );
void show_results( McoSqlEngine& engine );
int showPersons( QueryResult * result );

int main(int argc, char** argv)
{
  McoSqlEngine engine; 

  sample_os_initialize(DEFAULT|DISK);

  sample_header(sample_descr);
  printf(sample_explanation);
       
  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  engine.open( db_name, persondb_get_dictionary(), DATABASE_SIZE, MEMORY_PAGE_SIZE);
    
  printf("\n\tGenerate %s with %d Person records ? (Y/N) ", fileName, n_persons );
  char response = getchar();
  if ( 'Y' == toupper(response) ) {
		generate_persons_file();
    getchar(); // skip over '\n'
  } else if ( !isspace(response) ) {
    getchar(); // skip over '\n'
  }
 
  import_data(engine);

  show_results(engine);

  engine.close();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

void generate_persons_file()
{
  char			buffer[128];
  int			  n_bytes;
  char      name[64];
  int		    ord;
  
  FILE *    f = fopen( fileName, "wb" );
  if ( 0 != f ) {
    sprintf( buffer, "name,ordinal\n" );
    n_bytes = fwrite( buffer, 1, strlen( buffer ), f );
	  for( ord=1; ord <= n_persons; ord++ ) {
      sprintf( name, "Person%d", ord );
      sprintf( buffer, "%s,%d\n", name, ord );
      n_bytes = fwrite( buffer, 1, strlen( buffer ), f );
    }
  }
  fclose(f);
}

void import_data(McoSqlEngine& engine)
{
  char buf[MAX_LINE_LENGTH];
  char sql[MAX_LINE_LENGTH * 2];
  int count = 0;
  FILE* f = fopen(fileName, "r");
  if (f == NULL) {
    fprintf(stderr, "Failed to open file %s\n", fileName);
  } else {

    printf( "\n\tImport records from %s\n", fileName);

    if ( fgets(buf, sizeof(buf), f) ) {
      int len = (int)strlen(buf);
      while ( --len >= 0 && ( buf[len] == '\n' || buf[len] == '\r' || buf[len] == ',' ) )
        ;
      buf[len + 1] = '\0';
      char* values = sql + sprintf(sql, "insert into %s (%s) values ", tableName, buf);
      while ( fgets(buf, (int)sizeof(buf), f) ) {
        len = (int)strlen(buf);
        if ( len > 1 ) {
          buf[len - 1] = '\0';
	        if( buf[len - 2] == '\r') {
		        buf[len - 2] = '\0';
	        }
          char* dst = values;
          char* src = buf;
          char sep = '(';
          while (*src != '\0') {
            char* comma = strchr(src, ',');
            if ( comma != NULL ) {
              *comma = '\0';
            } else {
                comma = src + strlen(src);
            }
            dst += sprintf(dst, "%c'%s'", sep, src);
            sep = ',';
            src = comma + 1;
          }
          *dst++ = ')';
          *dst = '\0';
          engine.executeStatement(sql);
          count++;
        }
      }
    }
    fclose(f);
    printf("\n\tImported %d records into table %s\n", count, tableName);
  }
}

void show_results(McoSqlEngine& engine)
{
  QueryResult result(engine.executeQuery("select * from Person"));
  
  showPersons( &result );
}

int showPersons(QueryResult * result)
{
  int count = 1;
  Cursor* cursor = (*result)->records();

  printf("\n\tContents of table %s:\n", tableName );
  while (cursor->hasNext())
  {
    Record* rec = cursor->next();
    _Person p;

    // Extract the Person record to the corresponding struct
    (*result)->extract(rec, &p, sizeof(p));
    printf("\t\t%d) Name=%s, Ordinal=%u\n", count, p.name, p.ordinal );
    count++;
  }
  return count;
}

