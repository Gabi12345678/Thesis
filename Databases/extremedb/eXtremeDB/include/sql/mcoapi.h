/*******************************************************************
 *                                                                 *
 *  mcoapi.h                                                       *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           *
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/
#ifndef __MCOAPI_H__
#define __MCOAPI_H__

#include "dbapi.h"
#include "sqlcpp.h"
#include "mcowrap.h"

class MCOSQL_API McoObjHandle
{
    MCO_Hf h;
public:
    void operator=(mco_objhandle_h hnd);
    mco_objhandle_h operator->() {
        return (mco_objhandle_h)&h;
    }
    operator mco_objhandle_h() { 
        return (mco_objhandle_h)&h;
    }
};

/* Forward declaration of get_dictionary function (if you do not want to include generated header file)*/
#define GET_DICTIONARY(db) extern "C" mco_dictionary_h db##_get_dictionary()

extern const McoSql::Type mco2sql[];

#if !defined(_MSC_VER) || _MSC_VER >= 1300
template<class T,class V>
class McoSequence;
#endif
class McoTable;
class McoIndex;
class McoField;
class McoTransaction;
class McoCursor;
class McoRecord;
class McoStruct;
class McoReference;
class McoArray;
class McoVector;
class McoBlob;
class McoKey;
class McoIndexCursor;
class McoSqlEngine;
class McoGenericSequence;
class McoParallelSequence;
class McoReferenceField;

typedef struct mco_database_t_* mco_database_h;

#define MCO_HA_NOT_MASTER_ERROR_MESSAGE "Node is not a HA master"

struct McoJobConnection;

class MCOSQL_API McoDatabase: public McoSql::Database
{
    friend class McoTable;
    friend class McoIndex;
    friend class McoField;
    friend class McoTransaction;
    friend class McoCursor;
    friend class McoRecord;
    friend class McoStruct;
    friend class McoReference;
    friend class McoArray;
    friend class McoVector;
    friend class McoBlob;
    friend class McoKey;
    friend class McoIndexCursor;
    friend class McoSqlEngine;

    void alterTable(McoSql::Allocator* allocator, McoSql::Transaction* trans, McoSql::Table* newTable, McoTable* oldTable);
	
  public:
    McoDatabase(McoSql::Allocator* allocator, bool set_err_handler = true);
    void open(mco_db_h con);
    void connect(McoDatabase const &d, mco_db_h con);
    void close();
    DESTROY(McoDatabase)

    McoSql::Vector<McoSql::Table>* tables(McoSql::Allocator* allocator);
    McoSql::Transaction* beginTransaction(McoSql::Allocator* allocator, McoSql::Transaction::Mode mode, int priority, McoSql::Transaction::IsolationLevel level, bool nolock = false);
    void createTable(McoSql::Allocator* allocator, McoSql::Transaction* trans, McoSql::Table* table);
    void renameTable(McoSql::Allocator* allocator, McoSql::Transaction* trans, McoSql::String* oldName, McoSql::String* newName);
    void createIndex(McoSql::Allocator* allocator, McoSql::Transaction* trans, McoSql::Index* index);
    McoSql::Reference* createReference(McoSql::Allocator* allocator, int64_t ref);
    int getSchemaVersion();
	void copySchemaVersion();
	int getUDFVersion();
	void incrementUDFVersion();

    static void checkStatus(MCO_RET ret);

    void registerExternalTables(McoSql::Table** table,  size_t nTables);

    /**
     * Create connection pool needed for parallel query execution.
     */
    void createConnectionPool();

    /**
     * Destroy connection pool created by  createConnectionPool()
     */
    void destroyConnectionPool();

    void interruptConnectionPool(size_t threadId);

    void unwind();

    mco_database_h db;
    mco_db_h con;
    McoJobConnection* connectionPool;
    size_t nPooledConnections;
    size_t nExternalTables;
    McoSql::Table** externalTables;
    bool hasWritableExternalTables;
};

class MCOSQL_API McoTransaction: public McoSql::Transaction
{
    friend class McoDatabase;
    friend class McoTable;
    friend class McoCursor;
    friend class McoReference;
    friend class McoIndex;
    friend class McoIndexCursor;
    friend class McoGenericSequence;
    friend class McoParallelSequence;

    #if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T,class V>
    friend class McoSequence;
    #endif
  public:
    mco_trans_h handle();

    virtual bool upgrade();
    virtual bool commit(int phases);
    virtual void checkpoint();
    virtual void rollback();
	virtual void startCommand();
	virtual uint32_t getCommandId();
	virtual uint64_t getTransactionId();

    McoTransaction(McoSql::Allocator* allocator, McoDatabase* db, mco_db_h con, MCO_TRANS_TYPE mode, MCO_TRANS_PRIORITY priority, MCO_TRANS_ISOLATION_LEVEL level, bool nolock = false);
    void start();

  protected:

    McoTransaction(McoSql::Allocator* allocator) : McoSql::Transaction(allocator) {}
    DESTROY(McoTransaction)

    void restart();

    MCO_TRANS_TYPE mode;
    MCO_TRANS_PRIORITY priority;
    MCO_TRANS_ISOLATION_LEVEL level;
    mco_trans_h transaction;
    mco_db_h con;
    McoDatabase* db;

    struct MCOSQL_API DerefCache {
        int64_t id;
        McoObjHandle hnd;

        DerefCache() { id = 0; }
    };
    DerefCache lastDeref;
};

class McoCursor: public McoSql::RuntimeCursor
{
    friend class McoTable;

    #if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T,class V>
    friend class McoSequence;
    #endif
  protected:
    bool hasNext() { 
        return hasCurrent;
    }
    McoSql::Record* next();

    void moveNext(mco_objhandle_h hnd);
    void open();
    McoCursor(McoSql::Runtime* runtime, McoTable* table, uint2 classCode, int2 indexCode);

    McoTable* const table;
    McoTransaction* trans;
    uint2 classCode;
    int2 indexCode;
    mco_cursor_t cursor;
    bool hasCurrent;

    ~McoCursor();
    DESTROY(McoCursor)
};

class McoShowProgressCursor: public McoCursor
{
  public:
    int64_t currRecNo;
    int64_t nextReportRecNo;;
    int64_t reportInterval;

    McoSql::Record* next();

    McoShowProgressCursor(McoSql::Runtime* runtime, McoTable* table, uint2 classCode, int2 indexCode);
    DESTROY(McoShowProgressCursor)
};

    

class MCOSQL_API McoTable: public McoSql::Table
{
    friend class McoDatabase;
    friend class McoCursor;
    friend class McoField;
    friend class McoReference;
    friend class McoIndex;
    friend class McoIndexCursor;

    #if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T,class V>
    friend class McoSequence;
    #endif
  public:
    virtual McoSql::Vector<McoSql::Field> * fields();
    virtual McoSql::Cursor* cursor(McoSql::Runtime* runtime);
    virtual int64_t nRecords(McoSql::Runtime* runtime);
    virtual bool isNumberOfRecordsKnown();
    virtual McoSql::String* name();
    virtual void rename(McoSql::String* newName);
    virtual McoSql::Vector<McoSql::Index>* indexes();
    virtual void drop(McoSql::Transaction* trans);
    virtual void alter(McoSql::Transaction* trans, McoSql::Table* newTable);
    virtual void updateRecord(McoSql::Transaction* trans, McoSql::Record* rec);
    virtual void checkpointRecord(McoSql::Transaction* trans, McoSql::Record* rec);
    virtual void deleteRecord(McoSql::Transaction* trans, McoSql::Record* rec);
    virtual void deleteAllRecords(McoSql::Transaction* trans);
    virtual McoSql::Record* createRecord(McoSql::Allocator* allocator, McoSql::Transaction* trans);
    virtual void extract(McoSql::Record* rec, void* dst, size_t size, bool nullIndicators[], McoSql::ExtractMode mode);
    virtual void store(McoSql::Record* rec, void const* src, size_t size, bool const nullIndicators[], McoSql::ExtractMode mode);
    virtual void updateStatistic(McoSql::Transaction* trans, McoSql::Record* stat);
    virtual bool isTemporary();
	virtual bool isLocal();
    virtual bool isDropped();
	virtual int64_t TTL();
    uint2 getClassCode() const;

    McoTable(McoSql::Allocator* allocator, McoDatabase* db);
    DESTROY(McoTable)
  private:
    void load(int cid, mco_database_h db, McoSql::Vector<McoSql::Table>* tables);
    McoField* findFieldByOffset(int offset);

    static McoField* findFieldByOffsetCompact(McoSql::Vector<McoSql::Field>* fields, int offset);
    static McoField* findFieldByOffsetNormal(McoSql::Vector<McoSql::Field>* fields, int offset);

    uint2 classCode;
    int2 indexCode;
    int2 listIndexCode;
    int2 autoId;
    uint2 autoidOffsetU;
    uint2 autoidOffsetC;
    size_t initSize;
	size_t plainSize;
    uint2 event_id_new;
    uint2 event_id_delete;
    uint2 event_id_delete_all;
    uint2 event_id_class_update;
    bool isCompact;
    bool isPlain;
    uint2 autoidFieldNo;
    uint1 alignment;
    int flags;
	int64_t ttl;

    McoSql::String* _name;
    McoSql::Vector<McoSql::Field>* _fields;
    McoSql::Vector<McoSql::Index>* _indices;
    McoDatabase* db;
};

class McoField: public McoSql::Field
{
    friend class McoTable;
    friend class McoCursor;
    friend class McoBlob;
    friend class McoArray;
    friend class McoVector;
    friend class McoRecord;
    friend class McoIndex;

    #if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T,class V>
    friend class McoSequence;
    #endif
  public:
    virtual size_t position();
    virtual McoSql::String* name();
    virtual McoSql::Type type();
    virtual McoSql::Table* table();
    virtual McoSql::Field* scope();
    virtual McoSql::Value* getCopy(McoSql::Struct* rec);
    virtual McoSql::ValueRef get(McoSql::Struct* rec) {
        return McoSql::ValueRef(rec->allocator, getCopy(rec));
    } 
    virtual void set(McoSql::Struct* rec, McoSql::Value* val, bool append);
	virtual void insert(McoSql::Struct* rec, McoSql::Array* elems, uint64_t& position);
    virtual McoSql::Value* update(McoSql::Struct* rec);
    virtual McoSql::String* referencedTableName();
    virtual McoSql::Vector<McoSql::Field>* components();
    virtual McoSql::Type elementType();
    virtual McoSql::Field* element();
    virtual int elementSize();
    virtual int fixedSize();
    virtual int precision();
    virtual int width();
    virtual McoSql::SortOrder order();
    virtual bool isAutoGenerated();
    virtual bool isNullable();

    McoField* convert(McoTable* table, size_t pos, mco_database_h db, mco_dict_field_t* field,
                      mco_dict_field_t* indicator, McoField* scope,
                      McoSql::Vector<McoSql::Table>* tables);
    McoField(){}
    DESTROY(McoField)

    static McoSql::Vector<McoSql::Field>* structComponents(McoTable* table, mco_database_h db,
                                                           mco_dict_struct_t* sp, McoField* scope,
                                                           McoSql::Vector<McoSql::Table>* tables, int64_t& ttl);

    bool loadStructComponent(mco_objhandle_h obj);
    void storeStructComponent(mco_objhandle_h obj, McoSql::Struct* rec);

    McoSql::String* _name;
    McoSql::Type sqlType;
    McoSql::Type sqlElemType;
    int mcoType;
    McoTable* _table;
    int2 indexAware;
    uint1 flags;
    uint2 arraySize;
    uint2 fieldSize;
    uint2 structSizeU;
    uint2 structSizeC;
    uint2 structAlignU;
    uint2 structAlignC;
    uint2 event_id;
    uint2 fieldPos;
    int structNum;
    McoField* _scope;
    mco_datalayout_t layout;
    mco_datalayout_t indicator_layout;
    McoField* _element;
    uint2 fieldNo;
    uint2 indicatorNo;
    McoSql::Vector<McoSql::Field>* _components;
    McoTable* referencedTable;
    int1 _precision;
    int1 seq_order;
    int1 seq_elem_size;
};


class McoRecord: public McoSql::Record
{
    friend class McoTable;
    friend class McoCursor;
    friend class McoReference;
    friend class McoField;
    friend class McoStruct;
  public:
    virtual size_t size();
    virtual McoSql::Value* getCopy(size_t index);
    virtual McoSql::ValueRef get(size_t index) { 
        return McoSql::ValueRef(allocator, getCopy(index));
    }
    virtual McoSql::Value* update(size_t index);
    virtual void set(size_t index, McoSql::Value* value, bool append);
    virtual McoSql::Value* clone(McoSql::Allocator* allocator);
            
    mco_size_t extract(char* dst);
    mco_size_t extractCompact(char* dst);
	void       extractDirect(char* dst, size_t size);

    mco_size_t store(char const* src);
    mco_size_t storeCompact(char const* src);
	void       storeDirect(char const* src, size_t size);

    McoRecord(McoSql::Allocator* allocator, McoSql::Vector<McoSql::Field>* fields, McoSql::Transaction* trans, mco_objhandle_h hnd);
    DESTROY(McoRecord)

    McoSql::Vector<McoSql::Field>* fields;
    McoTransaction* trans;
    McoObjHandle obj;

  public:
    mco_objhandle_h getHandle()
    {
        return obj;
    }
};

class McoStruct: public McoRecord
{
    friend class McoTable;
    friend class McoField;
    friend class McoArray;
    friend class McoVector;
  public:
    McoStruct(McoSql::Allocator* allocator, McoSql::Vector<McoSql::Field>* fields, McoSql::Transaction* trans, mco_objhandle_h hnd, McoSql::Value* scope)
    : McoRecord(allocator, fields, trans, hnd)
    {
        _scope = scope;
    }
    DESTROY(McoStruct);

    virtual McoSql::Value* clone(McoSql::Allocator* allocator);
    virtual McoSql::Value* scope();

  private:
    McoSql::Value* _scope;
};


class McoReference: public McoSql::Reference
{
    friend class McoTable;
    friend class McoField;
    friend class McoArray;
    friend class McoVector;
    friend class McoDatabase;
    friend class McoReferenceField;
  private:
    virtual int compare(McoSql::Value* x, bool nullsFirst);
    virtual size_t toString(char* buf, size_t bufSize);
    virtual int64_t intValue();
    virtual McoSql::String* stringValue(McoSql::Allocator* allocator);
    virtual McoSql::Record* dereference(McoSql::Runtime* runtime);
    virtual McoSql::Value* clone(McoSql::Allocator* allocator);
    virtual void* pointer();

    #ifndef NO_FLOATING_POINT
    virtual double realValue();
    #endif
    McoReference(McoTable* referencedTable, int64_t id);
    DESTROY(McoReference)

    int64_t id;
    McoTable* referencedTable;
    McoTransaction* trans;
};

class McoArray: public McoSql::Array
{
    friend class McoField;
  private:
    virtual McoSql::Value* getCopyAt(size_t index);
    virtual McoSql::ValueRef getAt(size_t index) { 
        return McoSql::ValueRef(allocator, getCopyAt(index));
    }
    virtual McoSql::Value* getCopyAt(McoSql::Allocator* allocator, size_t index);
    virtual McoSql::ValueRef getAt(McoSql::Allocator* allocator, size_t index) { 
        return McoSql::ValueRef(allocator, getCopyAt(allocator, index));
    }
    virtual void setAt(size_t index, McoSql::Value* value);
    virtual McoSql::Value* updateAt(size_t index);
    virtual size_t size();
    virtual void setSize(size_t newSize);
    virtual void getBody(void* dst, size_t offs, size_t len);
    virtual void setBody(void* src, size_t offs, size_t len);
    virtual size_t getElemSize() const;
    virtual McoSql::Type getElemType() const;
    virtual McoSql::Value* clone(McoSql::Allocator* allocator);

    McoArray(McoSql::Allocator* allocator, McoField* field, McoSql::Transaction* trans, mco_objhandle_h hnd);
    DESTROY(McoArray)

    McoField* field;
    McoSql::Transaction* trans;
    McoObjHandle hnd;
};

class McoVector: public McoSql::Array
{
    friend class McoField;
  private:
    virtual McoSql::Value* getCopyAt(size_t index);
    virtual McoSql::ValueRef getAt(size_t index) { 
        return McoSql::ValueRef(allocator, getCopyAt(index));
    }
    virtual McoSql::Value* getCopyAt(McoSql::Allocator* allocator, size_t index);
    virtual McoSql::ValueRef getAt(McoSql::Allocator* allocator, size_t index) { 
        return McoSql::ValueRef(allocator, getCopyAt(allocator, index));
    }
    virtual void setAt(size_t index, McoSql::Value* value);
    virtual McoSql::Value* updateAt(size_t index);
    virtual size_t size();
    virtual void setSize(size_t newSize);
    virtual void getBody(void* dst, size_t offs, size_t len);
    virtual void setBody(void* src, size_t offs, size_t len);
    virtual size_t getElemSize() const;
    virtual McoSql::Type getElemType() const;
    virtual McoSql::Value* clone(McoSql::Allocator* allocator);

    McoVector(McoSql::Allocator* allocator, McoField* field, McoSql::Transaction* trans, mco_objhandle_h hnd);
    DESTROY(McoVector)

    McoField* field;
    McoSql::Transaction* trans;
    McoObjHandle hnd;
};

class McoBlob: public McoSql::Blob
{
    friend class McoField;
  private:
    virtual size_t available();
    virtual size_t get(void* buffer, size_t size);
    virtual void append(void const* buffer, size_t size);
    virtual void reset(size_t pos = 0);
    virtual void truncate();
    virtual McoSql::Value* clone(McoSql::Allocator* allocator);

    McoBlob(McoSql::Allocator* allocator, McoField* field, mco_objhandle_h hnd);
    DESTROY(McoBlob)

    McoField* field;
    McoSql::Transaction* trans;
    McoObjHandle obj;
    size_t pos;
};

class McoKey: public McoSql::Key
{
    friend class McoIndex;
  private:
    virtual McoSql::SortOrder order();
    virtual McoSql::Field* field();

    McoKey(McoField* field, McoSql::SortOrder order);
    DESTROY(McoKey)

    McoField* _field;
    McoSql::SortOrder _order;
};

enum IndexType
{
    BTREE, PTREE, RTREE, HASH, TRIGRAM, INCLUSIVE
};

class McoIndexCursor : public McoSql::RuntimeCursor
{
    McoIndexCursor(McoSql::Runtime* runtime, McoIndex* index);
  public:
    static McoIndexCursor* create(McoSql::Runtime* runtime, McoIndex* index);
    ~McoIndexCursor();
    virtual void destroy(McoSql::Allocator* allocator);

    McoSql::Record* next();

    mco_cursor_t cursor;
    McoObjHandle obj;
    McoIndex* index;
    bool inverse;
    bool singleton;
    bool hasCurrent;
    MCO_OPCODE op;
    mco_external_field_t ek[MCO_VAR_SIZE_ARRAY];
};

class McoIndex: public McoSql::Index
{
    friend class McoTable;
    friend class McoDatabase;
    friend class McoIndexCursor;
  private:
    virtual McoSql::String* name();
    virtual McoSql::Vector<McoSql::Key>* keys();
    virtual McoSql::Cursor* search(McoSql::Runtime* runtime, SearchOperation cop, McoSql::Vector<McoSql::Value>* values, McoSql::SortOrder order);
    virtual bool isApplicable(SearchOperation cop);
    virtual bool isRecheckNeeded(SearchOperation cop);
    virtual void drop(McoSql::Transaction* trans);
    virtual McoSql::Table* table();
    virtual bool isUnique();
    virtual bool isOrdered();
    virtual bool isSpatial();
    virtual bool isRadix();
    virtual bool isTrigram();
    virtual bool isInclusive();
    virtual bool isThick();
    virtual void updateStatistic(McoSql::Transaction* trans, McoSql::Record* stat);

    McoIndex(McoTable* table, int indexCode, mco_database_h db);
    DESTROY(McoIndex)

	bool setFollowingKey(mco_external_field_t ek[]);
    void setKeys(McoSql::Allocator* allocator, mco_external_field_t ek[], McoSql::Vector<McoSql::Value>* values);
    void setMinValue(mco_external_field_t ek[], McoSql::Vector<McoSql::Value>* values);
    void setMaxValue(mco_external_field_t ek[], McoSql::Vector<McoSql::Value>* values);
    int  getPrefixLength(mco_external_field_t ek[], char esc);
    void incrementPrefix(mco_external_field_t ek[], int prefix, int inc);

    int indexCode;
    McoTable* _table;
    McoSql::String* _name;
    McoSql::Vector<McoSql::Key>* _keys;
    bool _unique;
    uint4 _flags;
    IndexType type;
    McoSql::SortOrder order;
};

class FixedString: public McoSql::StringLiteral
{
  public:
    virtual int compare(McoSql::Value* x, bool nullsFirst);
    virtual McoSql::String* toLowerCase(McoSql::Allocator* allocator);
    virtual McoSql::String* toUpperCase(McoSql::Allocator* allocator);
    virtual McoSql::Value* clone(McoSql::Allocator* allocator);
    virtual uint64_t hashCode();
    virtual size_t size();
	
    static FixedString* create(McoSql::Allocator* allocator, size_t len)
    {
        return new(len, allocator) FixedString(len);
    }

    FixedString(size_t l): McoSql::StringLiteral((uint32_t)l){}
};
#ifdef UNICODE_SUPPORT
class UnicodeFixedString: public McoSql::UnicodeStringLiteral
{
  public:
    virtual int compare(McoSql::Value* x, bool nullsFirst);
    virtual McoSql::UnicodeString* toLowerCase(McoSql::Allocator* allocator);
    virtual McoSql::UnicodeString* toUpperCase(McoSql::Allocator* allocator);
    virtual McoSql::Value* clone(McoSql::Allocator* allocator);
    virtual uint64_t hashCode();
    virtual size_t size();

    static UnicodeFixedString* create(McoSql::Allocator* allocator, size_t len)
    {
        return new(len*sizeof(wchar_t), allocator) UnicodeFixedString(len);
    }

    UnicodeFixedString(size_t l): McoSql::UnicodeStringLiteral((uint32_t)l){}
};
#endif

#endif
