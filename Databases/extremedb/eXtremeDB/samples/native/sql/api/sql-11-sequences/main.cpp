/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include "sqlcln.h"
#include "sqlsrv.h"
#include <mcoapiseq.h>
#include <commonSQL.h>
#include "bbfisearch.h"


#ifdef DATABASE_SIZE
#undef DATABASE_SIZE
#endif

const int SERVER_PORT = 5001;
char const* SERVER_HOST = "localhost";

// #define DATABASE_SIZE (2LL * 1024 * 1024 * 1024)
#define DATABASE_SIZE (256 * 1024 * 1024)

char sample_descr[] = 
"Sample 'sequences' opens and initializes a database with sequences\n"
"and performs select for sequences.\n";

const char * db_name = "bbfisearch";
const size_t MAX_LINE_LENGTH = 2048;

const int N_CUSIPS = 100;//60994;
const int N_PRICE_PER_CUSIP = 2000;

// Define global SQL engine for local and remote connections
using namespace McoSql;
McoSqlEngine engine; 
RemoteSqlEngine rengine;

// Define the structure corresponding to database tables
struct _Address
{
  int4        zipcode;
  char const* city;
  char const* street;
};

// timer variables
time_t start_time;
time_t delta;

/* forward declarations */
void insertRecords();
void showSelect(SqlEngine *engine);
void deleteRecords();

int main( int argc, char ** argv )
{

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  engine.open( db_name, bbfisearch_get_dictionary(), 2 * DATABASE_SIZE, MEMORY_PAGE_SIZE);

  SqlServer server(&engine, SERVER_PORT);
  server.start();

  rengine.open( SERVER_HOST, SERVER_PORT );

  // Insert records

  insertRecords();

  showSelect(&engine);
  showSelect(&rengine);

  // Clean up
  deleteRecords();

  rengine.close();

  server.stop();
  engine.close();
  
  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

void insertRecords()
{
  Allocator allocator;      // Objects created with this allocator will be automatically destroyed
							// by the allocator's destructor when this function returns
  int i;
  char cusip[64];

  start_time = time(NULL);

  Array* dates = Array::create(&allocator, tpUInt4, 0, N_PRICE_PER_CUSIP);
  Array* prices = Array::create(&allocator, tpReal4, 0, N_PRICE_PER_CUSIP);
  Array* marks = Array::create(&allocator, tpString, 0, N_PRICE_PER_CUSIP);

  for (i = 0; i < N_PRICE_PER_CUSIP; i++) {
      uint4 year = i / 365;
      uint2 mon = (i - year*365) / 30;
      uint2 day = (i - year*365 - mon * 30) % 30;      
      mco_date date = (((2010 + year) * 100) + 1+ mon) * 100 + 1 + day;
      dates->setAt(i, IntValue::create(&allocator, date));
      marks->setAt(i, String::create(&allocator, "mark"));
  }

  Transaction* trans = engine.beginTransaction(Transaction::ReadWrite);

  for (int icus = 0; icus < N_CUSIPS; icus++) {
      sprintf(cusip, "cusip_%d", icus);
      for (i = 0; i < N_PRICE_PER_CUSIP; i++) {
          float price = 10 + 90 * (rand() / (1.0 + RAND_MAX));
          prices->setAt(i, RealValue::create(&allocator, price));
      }
      engine.executeStatement(trans, 
                              "insert into BondHistory (sid, cusip, dates, prices, marks) values (%i,%s,%v,%v,%v)", 
                              icus, cusip, dates, prices, marks);
  }
  trans->commit();
  trans->release();

  delta = time(NULL) - start_time;
  printf("\n\t%d objects: %d seconds (%d object/second)\n", N_CUSIPS, (int)delta,
                                (int)delta != 0 ? N_CUSIPS / (int)delta: N_CUSIPS);
}

void deleteRecords()
{
  engine.executeStatement("delete from BondHistory");
}


void showSelect(SqlEngine *engine)
{
  printf("\n\n\t0) \n");
  
  uint4 nRecords = 0;
  const char *query = "SELECT seq_search(dates, 20100101, 20100331) AS bond_date, seq_project(prices, bond_date), seq_project(marks, bond_date) FROM BondHistory";

  printf("\n\n\t%s'\n", query);

  start_time = time(NULL);
  QueryResult result( engine->executeQuery(query));

  size_t nColumns = result->nFields();
  McoSql::Vector<McoSql::Field>* fields = result->fields();
  for (int i = 0; i < nColumns; i++) { 
      McoSql::Field* field = fields->items[i];
      McoSql::Type type = field->type();
      McoSql::Type etype = field->elementType();
      /*
        Find out type of column field and type of element type.
        In case of local connection sequence is present as type tpSequnce while
        in case of remote connection the same sequence is present as type tpArray.
      */
  }
  Cursor* iterator = result->records();
  while ( iterator->hasNext() ) {
      Record* rec = iterator->next();
      McoSql::ValueRef dateRef(rec->get(0));
      McoSql::ValueRef priceRef(rec->get(1));
      McoSql::ValueRef markRef(rec->get(2));
      McoSql::Value * pDate = dateRef.value();
      McoSql::Value * pPrice = priceRef.value();
      McoSql::Value * pMark = markRef.value();

      if (pDate->type() == McoSql::tpSequence && pPrice->type() == McoSql::tpSequence
          && pMark->type() == McoSql::tpSequence)
      {
          // Local connection
          McoGenericSequence *dateseq = static_cast<McoGenericSequence *> (pDate);
          McoGenericSequence *priceseq = static_cast<McoGenericSequence *> (pPrice);
          McoGenericSequence *markseq = static_cast<McoGenericSequence *> (pMark);

          McoSql::Type dateseq_type = dateseq->elemType;
          McoSql::Type priceseq_type = priceseq->elemType;
          McoSql::Type markseq_type = markseq->elemType;

          /* Print sequence as text. It is internally limited to 100 chars */
          Allocator allocator;      // Objects created with this allocator will be automatically destroyed
                                    // by the allocator's destructor at the end of this code block
          if (nRecords == 0) {
              printf("date sequence: %s\n", dateseq->stringValue(&allocator)->cstr());
              printf("price sequence: %s\n", priceseq->stringValue(&allocator)->cstr());
              printf("mark sequence: %s\n", markseq->stringValue(&allocator)->cstr());
              dateseq->reset();
              priceseq->reset();
              markseq->reset();
          }

          McoSql::Value *dt;
          McoSql::Value *pr;
          McoSql::Value *mr;
          int i = 10;

          while ((dt = dateseq->next()) != NULL && 
              (pr = priceseq->next()) != NULL && (mr = markseq->next()) != NULL) {
              /* Print first 10 sequence elements one by one */
              if (nRecords == 0 && i) {
                  printf("(%s: %s, %s)\n", dt->stringValue(&allocator)->cstr(), pr->stringValue(&allocator)->cstr(),
                      mr->stringValue(&allocator)->cstr());
                  i--;
              }
          }
      } else {
          // Remote connection
          Array *dateseq = static_cast<Array *> (pDate);
          Array *priceseq = static_cast<Array *> (pPrice);
          Array *markseq = static_cast<Array *> (pMark);

          McoSql::Type dateseq_type = dateseq->getElemType();
          McoSql::Type priceseq_type = priceseq->getElemType();
          McoSql::Type markseq_type = markseq->getElemType();

          Allocator allocator;      // Objects created with this allocator will be automatically destroyed
                                    // by the allocator's destructor at the end of this code block
          if (nRecords == 0) {
              printf("date sequence: %s\n", dateseq->stringValue(&allocator)->cstr());
              printf("price sequence: %s\n", priceseq->stringValue(&allocator)->cstr());
              printf("mark sequence: %s\n", markseq->stringValue(&allocator)->cstr());

              for (int i  = 0; i < 10; i++) {
                  /* Print first 10 sequence elements one by one */
                  printf("(%s: %s, %s)\n", dateseq->getAt(i)->stringValue(&allocator)->cstr(), priceseq->getAt(i)->stringValue(&allocator)->cstr(),
                      markseq->getAt(i)->stringValue(&allocator)->cstr());
              }
          }
      }
      nRecords ++;
  }


  delta = time(NULL) - start_time;
  printf("\t\t%d objects: %d seconds (%d object/second)\n", nRecords, (int)delta,
                                (int)delta != 0 ? nRecords / (int)delta: nRecords);
}
