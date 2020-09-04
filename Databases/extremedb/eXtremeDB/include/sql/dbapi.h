/*******************************************************************
 *                                                                 *
 *  dbapi.h                                                      *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           *
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/
#ifndef __DBAPI_H__
#define __DBAPI_H__

#include "basedef.h"
#include "value.h"

namespace McoSql
{

    /**/
    /* The classes below have to be implemented by data provider layer.*/
    /**/

    class Key;
    class Field;
    class Table;
    class Index;
    class Mutex;
    class Semaphore;

    /**
     * Mode of extracting structs
     */
    enum ExtractMode
    {
        emReferenceToBody, emCopyFixedSizeStrings, emCloneStrings
    };
    
    /**
     * Key sort order in index
     */
    enum SortOrder
    {
        UNSPECIFIED_ORDER, 
        ASCENT_ORDER, 
        DESCENT_ORDER
    };

    /**
     * Index operation kind
     */
    enum IndexOperationKind 
    { 
        IDX_PRIMARY_KEY,
        IDX_EXACT_MATCH,
        IDX_RANGE,
        IDX_SPATIAL,
        IDX_SUBSTRING,
        IDX_PREFIX_MATCH,
    };


    /**
     * Transaction
     */
    class MCOSQL_API Transaction: public Resource
    {
    public:
        enum Mode
        {
            ReadOnly, Update, ReadWrite, Exclusive
        };
        enum IsolationLevel
        {
            DefaultIsolationLevel = 0,
            ReadCommitted = 1,
            RepeatableRead = 2,
            Serializable = 4
        };

        /**
         * Upgrade transaction mode to ReadWrite.
         * If transaction is already in ReadWrite mode, this method does nothing.
         * @return true of transaction is upgraded to read-write, false if upgrade is not possible
         */
        virtual bool upgrade() = 0;

        /**
         * Commit current transaction.
         * @param phases bit mask of commit transaction phases
         * @return true is transaction is successfully committed, false if conflict of MVCC transactions is encountered
         */
        virtual bool commit(int phases = 1+2) = 0;

            
        /**
         * Roll back current transaction.
         */
        virtual void rollback() = 0;


        /*
         * Checkpoint all changes of transaction
         */
        virtual void checkpoint() = 0;

		/*
		 * Start new SQL command
		 */
		virtual void startCommand() = 0;

		/*
		 * Get identifier of the current command
		 */
		virtual uint32_t getCommandId() = 0;

		/*
		 * Get unique identifier of the transaction 
		 */
		virtual uint64_t getTransactionId() = 0; 

        Transaction(Allocator* allocator) : Resource(allocator) {}
    };


    /**
     * Database interface
     */
	class MCOSQL_API Database: public Resource
    {
        friend class SqlEngine;
    public:
        /**
         * Get iterator for all tables in the database.
         * @return table iterator
         */
        virtual Vector<Table>* tables(Allocator* allocator) = 0;

        /**
         * Create new table in the database.
         * @param trans current transaction
         * @param table descriptor of the created table
         */
        virtual void createTable(Allocator* allocator, Transaction* trans, Table* table) = 0;

        /**
         * Rename existed table
         * @param trans current transaction
         * @param oldTable old table name
         * @param newTable new table name
         */
        virtual void renameTable(Allocator* allocator, Transaction* trans, String* oldName, String* newName) = 0;

        /**
         * Create new index in the database.
         * @param trans current transaction
         * @param index descriptor of the created index
         */
        virtual void createIndex(Allocator* allocator, Transaction* trans, Index* index) = 0;

        /**
         * Start new transaction.
         * @param mode transaction mode
         * @param prioroty transaction priority
         * @param level transaction isolation level
         * @return new transaction
         */
        virtual Transaction* beginTransaction(Allocator* allocator, Transaction::Mode mode, int priority = 0, Transaction::IsolationLevel level = Transaction::DefaultIsolationLevel, bool nolock = false) = 0;

        /**
         * Create reference from its integer representation.
         * @param ref integer representation of reference produced by <code>Reference.inValue()</code> method
         * @return reference created from specified integer ID
         * @throws InvalidOperation exception if database doesn't support conversion from integer to reference
         */
        virtual Reference* createReference(Allocator* allocator, int64_t ref) = 0;

        virtual int getSchemaVersion() = 0;
		virtual void copySchemaVersion() = 0;

        virtual int getUDFVersion() = 0;
		virtual void incrementUDFVersion() = 0;
        /**
         * Close database
         */
        virtual void close() = 0;
        
        /** 
         * This method is called by exception handler to unwind database state.
         * All scope operations with database which requires explicit close should be reset by this method
         */
        virtual void unwind() = 0;

        /**
         * Constructor of database class
         * This constructor sets static instance variable to point to itself.
         * If no database is explicitly specified in SqlEngine open method,
         * then value of <code>Database::instance</code> will be used.
         */
        Database(Allocator* allocator);

    protected:
        static Database* instance;
    };

    /**
     * Table record
     */
	class MCOSQL_API Record : public Struct
    {
    public:
        Record(Allocator* allocator) : Struct(allocator) {}
    };

    typedef Iterator<Record> Cursor;

    /**
     * Data source identifier type
     */
    class MCOSQL_API DataSource : public Container
    {
      public:
        virtual Type type();
        virtual int compare(Value* x, bool nullsFirst);
        virtual size_t toString(char* buf, size_t bufSize);
        virtual size_t serialize(char* buf, size_t size);
        
        /**
         * Get data source fields
         * @return fields
         */
        virtual Vector<Field>* fields() = 0;

        /**
         * Get number of fields (compatibility with old API)
         */
        size_t nFields() { 
            return fields()->length;
        }

        /**
         * Find field with specified name
         * @param name field name
         * @return located field or NULL if not found
         */
        Field* findField(String* name); 

        /**
         * Find field with specified name
         * @param name field name
         * @return located field or NULL if not found
         */
        Field* findField(char const* name); 

        /**
         * Get iterator for records (tuples) in the data source.
         * @param runtime query execution runtime
         * @return record iterator
         */
        virtual Cursor* cursor(Runtime* runtime);
        
        /**
         * Get iterator for records (tuples) in the result set. Not supported by all data sources.
         * @return record iterator
         */
        virtual Cursor* records();

        /**
         * Check if number of records is known for the data source.
         * Data source can retrieve record incrementally (on demand).
         * In this case, total number of records is unknown - iterate through all records
         * to know actual number of records in data source.
         * @return <code>true</code> if number of records in data source is known
         */
        virtual bool isNumberOfRecordsKnown();
        
        /**
         * Get number of records in data source.
         * This method can be used only
         * if <code>isNumberOfRecordsKnown()</code> method returns <code>true</code>.
         * @param runtime current runtime
         * @return number of records in data source
         * @throws InvalidOperation exception if number of records is not known
         */
        virtual int64_t nRecords(Runtime* runtime);

        /**
         * Extract record components to corresponding C struct.
         * The rules below describe C struct to be used as destination of this method:
         * <OL>
         * <LI>C struct should have exactly the same components in the same order as
         * database structure.
         * <LI>OpenSQL assumes <I>default alignment</I> of all struct members - i.e.,
         * alignment used by the compiler without some special align pragmas.
         * <LI>Array components are represented by pointer to <code>Array</code> value.
         * <LI>String components are stored as zero-termninated ANSI string.
         * <LI>If component of the structure doesn't belong to any table and is result of some
         * SELECT statement expression calculation, then its type is determined by the following rules:
         * <UL>
         * <LI>integer types (char, short, unsigned short, int,...)
         * are represented by int64_t type
         * <LI>floating point types (float, double) - by double type
         * <LI>Other types are represented by themselves
         * </UL>
         * <LI>Nested structures should be represented by the same C structs.
         * </OL>
         * @param rec record to be extracted
         * @param dst pointer to the C struct to receive components of this database structures
         * @param size Size of C struct should match the size of database record and is used only
         * for verification.
         * @param nullIndicators array which elements will be set to <code>true</code> if
         * value of correspondent field is Null. If this array is not specified (is NULL), then
         * attempting to extract Null value of scalar field will cause RuntimeException.
         * This array should be large enough to collect indicators of all fields. In case of nested
         * structure, it should contain elements for each component of this substructure.
         * @param mode struct component extraction mode.
         * <UL>
         * <LI><code>emReferenceToBody</code> set pointer to the string
         * body inside the database.
         * Structure should have the char* element. Cursor::next() methods
         * release memory after each iteration, it means that field values
         * returned by next() are valid only until next invocation of
         * next() method.
         * <LI><code>emCopyFixedSizeStrings</code> copy fixed-size strings
         * Structure should have char[N] element where N is the size of
         * the fixed-size string.
         * <LI><code>emCloneStrings</code> clone string components.
         * Structure should have the char* element. In this case it is
         * responsibility of the programmer to deallocate string bodies.
         * (using "delete[]" operator).
         */
        virtual void extract(Record* rec, void* dst, size_t size, bool nullIndicators[] = NULL, 
                             ExtractMode mode=emReferenceToBody);

        /**
         * This method is reverse to extract and store in database values from C struct
         * Restrictions are the same as for extract
         */
        virtual void store(Record* rec, void const* src, size_t size, bool const nullIndicators[] = NULL,
                           ExtractMode mode = emReferenceToBody);

   
        /**
         * Explain plan of obtaining this data source
         */
        virtual String* explain(int level);
        
		virtual void cleanup();
		
        DataSource(Allocator* allocator) : Container(allocator) {}
    };
    

    /**
     * Class used to specify index keys
     */
    class Key: public DynamicObject
    {
    public:
        /**
         * Order in which records are in index by this field
         * @return element of Order enum
         */
        virtual SortOrder order() = 0;

        /**
         * Field by which records will be sorted in index
         */
        virtual Field* field() = 0;
    };

    /**
     * Class specifying table constraints (used for <code>Database::createTable</code> method
     * SQL engine doesn't enforce or check any constraints.
     */
    class Constraint: public DynamicObject
    {
    public:
        /**
         * Constraint type
         */
        enum ConstraintType
        {
            UNIQUE, PRIMARY_KEY, USING_INDEX, USING_INCLUSIVE_INDEX, USING_THICK_INDEX, INDEX, USING_HASH_INDEX, USING_PTREE_INDEX, USING_RTREE_INDEX, USING_TRIGRAM_INDEX, FOREIGN_KEY, NOT_NULL, MAY_BE_NULL
        };
        /**
         * Get constraint type.
         * @return type of this constraint
         */
        virtual ConstraintType type() = 0;

        /**
         * Get iterator of fields for which this constraint is defined.
         * @return field iterator for constrained fields
         */
        virtual Vector<Field>* fields() = 0;

        /**
         * Get table this constraint belongs to.
         * @return table for which this constraint is defined
         */
        virtual Table* table() = 0;

        /**
         * Foreign key constraint only
         * Determines if database should delete all members of the relation when owner of the relation is removed.
         * @return <code>true</code> if delete operation of relation owner should cause cascade delete of all
         * relation members
         * @throws InvalidOperation exception is called not for foreign key constraint
         */
        virtual bool onDeleteCascade() = 0;

        /**
         * Foreign key constraint only
         * Reference to the table containing primary key
         * @return name of the database which is primary key for this foreign key
         * @throws InvalidOperation exception is called not for foreign key constraint
         */
        virtual String* primaryKeyTable() = 0;

        /**
         * Foreign key constraint only
         * Get information about primary key fields.
         * @return iterator of names of primary key fields
         * @throws InvalidOperation exception is called not for foreign key constraint
         */
        virtual Vector<String>* primaryKey() = 0;

        /**
         * Constraint name
         */
        virtual String* name() = 0;
    };

    /**
     * Table index
     */
    class Index : public DynamicObject
    {
    public:
        /**
         * Get name of the index.
         * @return index name
         */
        virtual String* name() = 0;

        /**
         * Get iterator for key fields used in this index.
         * @return field iterator for index key fields
         */
        virtual Vector<Key>* keys() = 0;

        enum SearchOperation
        {
            opNop, opEQ, opGT, opGE, opLT, opLE, opLike, opILike, opExactMatch, opPrefixMatch, opOverlaps, opContains, opBelongs, opNear, opContainsAll, opContainsAny
        };


        /**
         * Search records matching search criteria.
         * @param runtime execution runtime
         * @param op search operation code
         * @param keyValue key values
         * @param order iteration direction
         * @return index data source
         */
        virtual Cursor* search(Runtime* runtime, SearchOperation cop, Vector<Value>* keyValues, SortOrder order) = 0;

        /**
         * Check if index is applicable for specified operation.
         * @param op search operation code
         * @return <code>true</code> is index is applicable, <code>false</code> otherwise
         */
        virtual bool isApplicable(SearchOperation cop);

        /**
         * Check if requested operation can be completely performed by index or index search may return some records
         * not matching searhc condition
         */
        virtual bool isRecheckNeeded(SearchOperation cop);

        /**
         * Drop index
         * @param trans current transaction
         */
        virtual void drop(Transaction* trans);

        /**
         * Get table this index belongs to.
         * @return table for which this index is defined
         */
        virtual Table* table() = 0;

        /**
         * If index is unique
         * @return <code>true</code> if index is unique, <code>false</code> if it is not unique or is unknown
         */
        virtual bool isUnique() = 0;

        /**
         * If index performs sort
         * @return <code>true</code> if index is ordered, <code>false</code> if it is not ordered or is unknown
         */
        virtual bool isOrdered() = 0;

        /**
         * If index is spatial index (R-Tree)
         * @return <code>true</code> if index is spatial, <code>false</code> if it is not spatial or is unknown
         */
        virtual bool isSpatial() = 0;

        /**
         * If index is trigram index 
         * @return <code>true</code> if index is trigram index, <code>false</code> if it is not spatial or is unknown
         */
        virtual bool isTrigram() = 0;

        /**
         * If index is inclusive 
         * @return <code>true</code> if index is inclusive, <code>false</code> otherwise
         */
        virtual bool isInclusive() = 0;

        /**
         * If index is thick (optimized for large number of duplicates)
         * @return <code>true</code> if index is thick, <code>false</code> otherwise
         */
        virtual bool isThick() = 0;

         /**
         * If index is radix true (patricia)
         * @return <code>true</code> if index is patricia, <code>false</code> otherwise
         */
        virtual bool isRadix() = 0;

       /**
         * Update index statistic
         * @param trans current transaction
         * @param stat statistic record which fields should be updated
         */
        virtual void updateStatistic(Transaction* trans, Record* stat);

        /**
         * Print index full name: table-name.index-name(key1,key2,...)
         */
        virtual String* fullName(Allocator* allocator);
    };

    /**
     * Database table
     */
    class MCOSQL_API Table : public DataSource
    {
    public:
        /**
         * Name of the table
         * @return name of the table
         */
        virtual String* name() = 0;

        /**
         * Check if table is temporary (means not stored on the disk)
         */
        virtual bool isTemporary();

        /**
         * Check if table is local (not replicated)
         */
        virtual bool isLocal();

        /**
         * Check if table is dropped (descriptors of dropped tables can still present in doctionary)
         */
        virtual bool isDropped();

        /**
         * Check if external table is read-only (no need to commit())
         */
        virtual bool isReadOnly();

		/**
		 * Time to live
		 */
		virtual int64_t TTL();

        /**
         * Whether to hide or show table in metatables
         */
        virtual bool isHidden();

        /**
         * Get iterator through indexes defined for this table.
         * @return index iterator for this table
         */
        virtual Vector<Index>* indexes();

        /**
         * Drop table
         * @param trans current transaction
         */
        virtual void drop(Transaction* trans);

        /**
         * Alter table
         * @param trans current transaction
         * @param newTable updated table
         */
        virtual void alter(Transaction* trans, Table* newTable);

        /**
         * Update specified record.
         * @param trans current transaction
         * @param rec records which columns were changed
         */
        virtual void updateRecord(Transaction* trans, Record* rec);

        /**
         * Insert updated records in indexes
         * @param trans current transaction
         * @param rec records which columns were changed
         */
        virtual void checkpointRecord(Transaction* trans, Record* rec);

        /**
         * Delete record.
         * @param trans current transaction
         * @param rec record to be deleted
         */
        virtual void deleteRecord(Transaction* trans, Record* rec);

        /**
         * Delete all records in the table (but not the table itself).
         * @param trans current transaction
         */
        virtual void deleteAllRecords(Transaction* trans);

        /**
         * Create new record in the table.
         * Record is created with default values of the fields.
         * Update this value using <code>Field.set</code> method and
         * store updated record in the database using <code>Record.updateRecord</code> or
         * <code>Table.updateRecord</code> methods.
         * @param trans current transaction
         * @return created record with default values of columns
         */
        virtual Record* createRecord(Allocator* allocator, Transaction* trans);

        /**
         * Get iterator through table constraints.
         * @return constraint iterator
         */
        virtual Vector<Constraint>* constraints();

        /**
         * Commit current transaction. This method is needed only for virtual (user defined) tables.
         * @param phases bit mask of commit transaction phases
         * @return true is transaction is successfully committed, false if conflict of MVCC transactions is encountered
         */
        virtual bool commit(int phases = 1+2);

        /**
         * Roll back current transaction. This method is needed only for virtual (user defined) tables.
         */
        virtual void rollback();

        /**
         * Update table statistic
         * @param trans current transaction
         * @param stat statistic record which fields should be updated
         */
        virtual void updateStatistic(Transaction* trans, Record* stat);

        /**
         * Locate applicable indexes for the specified field
         */
        virtual Vector<Index>* findIndexes(Allocator* alocator, Field* key, IndexOperationKind kind);
        
        /**
         * Find index by name
         * @param name index name
         * @return index ort NULL if not found
         */
        virtual Index* findIndex(String* name);

        /**
         * Find index by name
         * @param name index name
         * @return index ort NULL if not found
         */
        virtual Index* findIndex(char const* name);

        /**
         * Rename table
         */
        virtual void rename(String* newName);

        Table(Allocator* allocator) : DataSource(allocator) {}
    };

    /**
     * Table field (column)
     */
    class MCOSQL_API Field : public DynamicObject
    {
    public:
        /**
         * Field position in record (0-based)
         */
        virtual size_t position() = 0;

        /**
         * Get name of the fields.
         * @return field name
         */
        virtual String* name() = 0;

        /**
         * Get type of the field.
         * @return field type
         */
        virtual Type type() = 0;

        /**
         * Get value of this field.
         * @param rec record or stucture containing this field
         * In most cases rec is just table record (even for components of columns of this record having structure
         * type). But for component of array of struct element, <code>rec</code> should be
         * <code>Struct</code> value returned for this array element
         * @return value of this field
         */
        virtual ValueRef get(Struct* rec);

        /**
         * Get copy of value of this field. This value shouldbe deallocated by caller.
         * @param rec record or stucture containing this field
         * In most cases rec is just table record (even for components of columns of this record having structure
         * type). But for component of array of struct element, <code>rec</code> should be
         * <code>Struct</code> value returned for this array element
         * @return value of this field
         */
        virtual Value* getCopy(Struct* rec);

        /**
         * Get value of integer field. 
         * @param rec record or stucture containing this field
         * In most cases rec is just table record (even for components of columns of this record having structure
         * type). But for component of array of struct element, <code>rec</code> should be
         * <code>Struct</code> value returned for this array element
         * @return value of this field
         */
        virtual int64_t getInt(Struct* rec);

        /**
         * Get value of floating point field. 
         * @param rec record or stucture containing this field
         * In most cases rec is just table record (even for components of columns of this record having structure
         * type). But for component of array of struct element, <code>rec</code> should be
         * <code>Struct</code> value returned for this array element
         * @return value of this field
         */
        virtual double getReal(Struct* rec);

        /**
         * Set new value of this field.
         * @param rec record or stucture containing this field
         * @param val value assigned to the field
         * @see Field.get
         */
        virtual void set(Struct* rec, Value* val, bool append = false);

		/**
		 * Insert data in sequence field
         * @param rec record or stucture containing this field
         * @param elems inserted elements
		 * @param position insert position, if -1 then data will be appended. 
		 * If append of ordered sequence is not possible (becaused violates sequence order), 
		 * then attempt to locate correct position and insert data at this place. 
		 * If all elements can be correctly inserted in this position (don't interleave with existed elements),
		 * then return this position.
		 */
        virtual void insert(Struct* rec, Array* elems, uint64_t& position);

        /**
         * Get structure or array value for update.
         * @param rec record or stucture containing this field
         * @return value which components or elements will be updated
         * @see Field.get
         */
        virtual Value* update(Struct* rec);

        /**
         * Get table this field belongs to.
         * @return table containing this field
         */
        virtual Table* table() = 0;

        /**
         * Get scope field of this field.
         * @return structure or array field containing this field as component or
         * <code>NULL</code> if this field is not component of any structure or array
         */
        virtual Field* scope() = 0;

        /**
         * Only for reference field
         * Get name of referenced table.
         * @return name of referenced table or <code>NULL</code> if it not statically known (generic reference)
         * @throws InvalidOperation exception when applied to non-reference field
         */
        virtual String* referencedTableName() = 0;

        /**
         * Only for structure field
         * Get vector of structure components
         * @return field iterator for structure fields
         * @throws InvalidOperation exception when applied to non-structure field
         */
        virtual Vector<Field>* components() = 0;

        /**
         * Get type of sequence or array element.
         * Only for array or sequence fields.
         * @return element type or tpNull if field has type other than sequence or array
         */
        virtual Type elementType() = 0;

        /**
         * Get size of sequence element.
         * Only for sequence fields.
         * @return element type or -1l if field has type other than sequence
         */
        virtual int elementSize() = 0;

        /**
         * Only for array field
         * Get descriptor of array element.
         * @return descriptor of array element
         * @throws InvalidOperation exception when applied to non-array field
         */
        virtual Field* element() = 0;

        /**
         * Only for array field.
         * Get size of fixed arrays.
         * @return size of fixed array or 0 for arrays with varying size
         * @throws InvalidOperation exception when applied to non-array field
         */
        virtual int fixedSize() = 0;

        /**
         * Only for numeric field.
         * Get field precision (digits after comma)
         * @return precision of numeric field or -1 for other fields
         */
        virtual int precision() = 0;

        /**
         * Only for numeric field.
         * Get field width (number of meaningful digits)
         * @return width of numeric field or -1 for other fields
         */
        virtual int width() = 0;

        /**
         * Field is nullable.
         * @return true if field can contain NULL values
         */
        virtual bool isNullable() = 0;

        /**
         * Column is autogenerated by database and should not be considered in "natural" join.
         */
        virtual bool isAutoGenerated() = 0;

        /**
         * Sequence order
         */
        virtual SortOrder order() = 0;

        /**
         * Find structure structe component
         * @param name component name
         * @return located component with this name or NULL if not found
         */
        Field* findComponent(String* name);

        /**
         * Find structure structe component
         * @param name component name
         * @return located component with this name or NULL if not found
         */
        Field* findComponent(char const* name);

        /**
         * Calculate struct alignment.
         * @return structure alignment
         */
        size_t calculateStructAlignment();

		/**
		 * Clone field 
		 */
		virtual Field* clone(Allocator* allocator);

        size_t serialize(char* buf, size_t bufSize);
    };

}

#endif
