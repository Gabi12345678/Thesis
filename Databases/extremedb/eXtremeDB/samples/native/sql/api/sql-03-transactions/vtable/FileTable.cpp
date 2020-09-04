#include <stdio.h>
#include "FileTable.h"

#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
#define ASSERT(cond) do if (!(cond)) { throw RuntimeError(#cond " failed"); } while (0)
#else
#define ASSERT(cond) do if (!(cond)) { printf("'%s' failed\n", #cond); exit(-1); } while (0)
#define THROW_MESSAGE(c) do { printf("%s\n", c); exit(-1); } while (0)
#endif

Vector < Field > * FileTable::fields()
{
    return _fields;
}

Cursor* FileTable::cursor(Runtime* runtime)
{
    return new (&runtime->memory) FileCursor(&runtime->memory, this);
}

int64_t FileTable::nRecords(Runtime* runtime)
{
    FILE *file = fopen(_path, "rb");
    ASSERT(file != NULL);
    ASSERT(fseek(file, 0, SEEK_END) == 0);
    long sz = ftell(file);
    fclose(file);

    if (sz%_recordSize)
#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
        throw RuntimeError("FileTable::nRecords file size is not aligned with record size");
#else
        THROW_MESSAGE("FileTable::nRecords file size is not aligned with record size\n");
#endif

    return (int64_t)(sz/_recordSize);
}

bool FileTable::isNumberOfRecordsKnown()
{
    return true;
}

int FileTable::compareRID(Record* r1, Record* r2)
{
#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
    throw InvalidOperation("FileTable::compareRID");
#else
    THROW_MESSAGE("FileTable::compareRID\n");
#endif
}

Reference* FileTable::getRID(Record* rec)
{
#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
    throw InvalidOperation("FileTable::getRID");
#else
    THROW_MESSAGE("FileTable::getRID\n");
#endif
}

bool FileTable::isRIDAvailable()
{
    return false;
}

String* FileTable::name()
{
    return String::create(allocator, _name);
}

static Vector<Index> noIndexes(0);
Vector < Index > * FileTable::indexes()
{
    return &noIndexes;
}

void FileTable::drop(Transaction* trans)
{
#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
    throw InvalidOperation("FileTable::drop");
#else
    THROW_MESSAGE("FileTable::drop\n");
#endif
}

void FileTable::updateRecord(Transaction* trans, Record* rec)
{
    FileRecord* r = (FileRecord *)rec;
    FILE *file = fopen(_path, "r+b");
    ASSERT(file != NULL);
    if (r->_tell == -1) { 
        ASSERT(fseek(file, 0, SEEK_END) == 0);
        r->_tell = ftell(file);
    } else { 
        ASSERT(fseek(file, r->_tell, SEEK_SET) == 0);
    }
    ASSERT(fwrite(r->_data, _recordSize, 1, file) == 1);
    fclose(file);
}

void FileTable::deleteRecord(Transaction* trans, Record* rec)
{
    FileRecord* r = (FileRecord *)rec;
    if (r->_tell != -1)
    {
        String *tmp_path = String::format(allocator, "%s.tmp", _path);
        char buf[MAX_RECORD_SIZE];
        FILE *ftmp, *file;
        file = fopen(_path, "rb");
        ftmp = fopen(tmp_path->cstr(), "wb");
        ASSERT(file != NULL);
        ASSERT(ftmp != NULL);
        ASSERT(fseek(file, 0, SEEK_END) == 0);
        int nRecords = ftell(file)/_recordSize;
        int delRecordId = r->_tell/_recordSize;
        ASSERT(fseek(file, 0, SEEK_SET) == 0);
        for (int i = 0; i < nRecords; i++)
        {
            ASSERT(fread(buf, _recordSize, 1, file) == 1);
            if (i != delRecordId) { 
                ASSERT(fwrite(buf, _recordSize, 1, ftmp) == 1);
            }
        }
        fclose(file);
        fclose(ftmp);
        ASSERT(::remove(_path) == 0);
        ASSERT(::rename(tmp_path->cstr(),_path) == 0);
    }
}

void FileTable::deleteAllRecords(Transaction* trans)
{
    FILE *file = fopen(_path, "wb");
    ASSERT(file != NULL);
    fclose(file);
}

Record* FileTable::createRecord(Allocator* allocator, Transaction* trans)
{
    return new (allocator) FileRecord(allocator, this, -1);
}

static Vector<Constraint> noConstraints(0);
Vector < Constraint > * FileTable::constraints()
{
    return &noConstraints;
}

FileTable::FileTable(Allocator *allocator, char const* name, char const* filePath, FieldDescriptor* fields, int nFields)
    : Table(allocator)
{
    _name = name;
    _path = filePath;
    _fields = Vector<Field>::create(allocator, nFields);
    _recordSize = 0;
    for (int i = 0; i < nFields; i++) { 
        FileField* f = new (allocator) FileField;
        _fields->at(i) = f;
        _recordSize = (_recordSize + fields[i].size - 1) & ~(fields[i].size - 1); // align on field size
        f->_table = this;
        f->_id = i;
        f->_type = fields[i].type;
        f->_name = fields[i].name;
        f->_size = fields[i].size;
        f->_offset = _recordSize;
        _recordSize += fields[i].size;
    }
    ASSERT(_recordSize <= MAX_RECORD_SIZE);
    FILE *file = fopen(_path, "ab");
    ASSERT(file != NULL);
    fclose(file);
}

FileTable::~FileTable()
{
    DELETE_OBJ(allocator, _fields);
}

int FileRecord::nComponents()
{
    return _table->_fields->length;
}

McoSql::ValueRef FileRecord::get(size_t index)
{
    return McoSql::ValueRef(_table->allocator, createValue(index));
}

McoSql::Value* FileRecord::getCopy(size_t index)
{
    return createValue(index);
}

McoSql::Value* FileRecord::update(size_t index)
{
#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
    throw InvalidOperation("FileRecord::update");
#else
    THROW_MESSAGE("FileRecord::update\n");
#endif
}

void FileRecord::set(size_t index, McoSql::Value* value, bool append)
{
    FileField* field = (FileField*)_table->_fields->at(index);
    char* data = _data + field->_offset;

    switch (field->_type)
    {
      case tpBool:
          *data = ((BoolValue *)value)->isTrue() ? 1 : 0;
        break;
      case tpInt1:
          *data = (char)(((IntValue *)value)->intValue());
        break;
      case tpUInt1:
          *(unsigned char*)data = (unsigned char)(((IntValue *)value)->intValue());
        break;
      case tpInt2:
          *(short*)data = (short)(((IntValue *)value)->intValue());
        break;
      case tpUInt2:
          *(unsigned short*)data = (unsigned short)(((IntValue *)value)->intValue());
        break;
      case tpInt4:
          *(int*)data = (int)(((IntValue *)value)->intValue());
        break;
      case tpUInt4:
          *(unsigned int*)data = (unsigned int)(((IntValue *)value)->intValue());
        break;
      case tpInt8:
      case tpUInt8:
          *(int64_t*)data = ((IntValue *)value)->intValue();
        break;
      case tpReal4:
          *(float*)data = (float)((RealValue *)value)->intValue();
        break;
      case tpReal8:
          *(double*)data = (double)((RealValue *)value)->intValue();
        break;
      case tpDateTime:
          *(time_t*)data = (time_t)(((DateTime *)value)->intValue());
        break;
      case tpString:
          memcpy(data, ((StringLiteral *)value)->cstr(), field->_size);
        break;
      default:
#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
        throw RuntimeError("Invalid field type");
#else
        THROW_MESSAGE("Invalid field type\n");
#endif
    }
}

McoSql::Struct* FileRecord::source()
{
    return this;
}

void FileRecord::deleteRecord()
{
#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
    throw InvalidOperation("FileRecord::deleteRecord");
#else
    THROW_MESSAGE("FileRecord::deleteRecord\n");
#endif
}

void FileRecord::updateRecord()
{
#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
    throw InvalidOperation("FileRecord::updateRecord");
#else
    THROW_MESSAGE("FileRecord::updateRecord\n");
#endif
}

size_t FileRecord::size()
{
    return _table->_fields->length;
}

FileRecord::FileRecord(Allocator* allocator, FileTable* table, int tell)
    : Record(allocator)
{
    _table = table;
    _tell = tell;
}

McoSql::Value* FileRecord::createValue(size_t index)
{
    FileField* field = (FileField*)_table->_fields->at(index);
    char* data = _data + field->_offset;
    Allocator* allocator = _table->allocator;
    
    switch (field->_type)
    {
      case tpBool:
        return BoolValue::create(*data != 0);
      case tpInt1:
        return IntValue::create(allocator, *data);
      case tpUInt1:
        return IntValue::create(allocator, *(unsigned char*)data);
      case tpInt2:
        return IntValue::create(allocator, *(short*)data);
      case tpUInt2:
        return IntValue::create(allocator, *(unsigned short*)data);
      case tpInt4:
        return IntValue::create(allocator, *(int*)data);
      case tpUInt4:
        return IntValue::create(allocator, *(unsigned int*)data);
      case tpInt8:
      case tpUInt8:
        return IntValue::create(allocator, *(int64_t*)data);
      case tpReal4:
        return RealValue::create(allocator, *(float*)data);
      case tpReal8:
        return RealValue::create(allocator, *(double*)data);
      case tpDateTime:
        return DateTime::create(allocator, *(time_t*)data);
      case tpString:
        {
          return String::create(allocator, data, strlen(data));
        }
      default:
#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
        throw RuntimeError("Invalid field type");
#else
        THROW_MESSAGE("Invalid field type\n");
#endif
    }
}

size_t FileField::position()
{
    return _id;
}

String* FileField::name()
{
    return String::create(_table->allocator, _name);
}

Type FileField::type()
{
    return _type;
}

Table* FileField::table()
{
    return _table;
}

Field* FileField::scope()
{
    return NULL;
}

McoSql::ValueRef FileField::get(Struct* rec)
{
    return rec->get(_id);
}

McoSql::Value* FileField::getCopy(Struct* rec)
{
    return rec->getCopy(_id);
}

void FileField::set(Struct* rec, McoSql::Value* val, bool append)
{
    return rec->set(_id, val, append);
}

McoSql::Value* FileField::update(Struct* rec)
{
#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
    throw InvalidOperation("FileField::update");
#else
    THROW_MESSAGE("FileField::update\n");
#endif
}

String* FileField::referencedTableName()
{
    return NULL;
}

Vector < Field > * FileField::components()
{
#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
    throw InvalidOperation("FileField::components");
#else
    THROW_MESSAGE("FileField::components\n");
#endif
}

Type FileField::elementType()
{
    return tpNull;
}

int FileField::elementSize()
{
    return -1;
}

int FileField::precision()
{
    return 0;
}

int FileField::width()
{
    return -1;
}

SortOrder FileField::order()
{
    return (McoSql::SortOrder)0;
}

Field* FileField::element()
{
#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
    throw InvalidOperation("FileField::element");
#else
    THROW_MESSAGE("FileField::element\n");
#endif
}

int FileField::fixedSize()
{
    return _size;
}

bool FileField::isAutoGenerated()
{
    return false;
}

bool FileField::isNullable()
{
    return false;
}

bool FileCursor::hasNext()
{
    if (_next == NULL) {
        FILE *file = fopen(_table->_path, "rb");
        ASSERT(file != NULL);
        _next = new (allocator) FileRecord(allocator, _table, _tell);
        if (fseek(file, _tell, SEEK_SET) == 0 && fread(_next->_data, _table->_recordSize, 1, file) == 1) { 
            fclose(file);
            return true;
        }
        fclose(file);
        _next = NULL;
        return false;
    }
    return true;
}

McoSql::Record* FileCursor::next()
{
    if (!hasNext())
    {
        return NULL;
    }
    McoSql::Record *rec = _next;
    _next = NULL;
    _tell += _table->_recordSize;
    return rec;
}

FileCursor::FileCursor(Allocator* allocator, FileTable* table)
    : Cursor(allocator)
{
    _table = table;
    _next = NULL;
    _tell = 0;
}

Value* FileRecord::clone(Allocator* allocator)
{
	FileRecord* copy = new (allocator) FileRecord(allocator, _table, _tell);
	memcpy(copy->_data, _data, MAX_RECORD_SIZE);
	return copy;
}
