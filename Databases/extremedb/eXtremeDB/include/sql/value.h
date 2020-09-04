/*******************************************************************
 *                                                                 *
 *  value.h                                                      *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           *
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/
#ifndef __VALUE_H__
#define __VALUE_H__

#include "config.h"
#include "basedef.h"
#include "exceptions.h"

#if MCO_CFG_SQL_SUPPORT_STD_STRING
#include <string>
#endif

namespace McoSql
{
    class Record;
    class String;
    class Binary;
    class Runtime;
    class IntValue;
    class RealValue;
    class UnicodeString;

    /**
     * Field types
     */
    enum Type
    {
        tpNull, tpBool, tpInt1, tpUInt1, tpInt2, tpUInt2, tpInt4, tpUInt4, tpInt8, tpUInt8, tpReal4, tpReal8,
        tpDateTime,  /* mco_datetime */
        tpNumeric,
        tpUnicode, tpString, tpBinary, tpReference, tpArray, tpStruct, tpBlob, tpDataSource, tpList,
        tpSequence,
        tpAverage,
        tpNullableArray,
        tpAsArg1Type,
        tpAsArg2Type,
        tpAsArg3Type,
        tpAsArg1XType,
        tpAsArg1ElemType,
        tpAny,
        tpInt = tpInt8, tpReal = tpReal8
    };

    inline bool isComparableType(Type t)
    {
        /*return unsigned(t - tpBool) <= unsigned(tpReference - tpBool);*/
        return unsigned(t - tpBool) <= unsigned(tpArray - tpBool);
    }

    inline bool isScalarType(Type t)
    {
        return unsigned(t - tpBool) <= unsigned(tpNumeric - tpBool);
    }

    inline bool isIntType(Type t)
    {
        return unsigned(t - tpInt1) <= unsigned(tpUInt8 - tpInt1);
    }

	inline bool isRealType(Type t)
    {
        return unsigned(t - tpReal4) <= unsigned(tpReal8 - tpReal4);
    }

    inline bool isStringType(Type t)
    {
        return unsigned(t - tpUnicode) <= unsigned(tpString - tpUnicode);
    }

    MCOSQL_API char const* getTypeName(Type type);
	MCOSQL_API Type getTypeByName(char const* name);

    class MCOSQL_API FormattedOutput
    {
      public:
        /* default constants */
        enum {
            DEFAULT_SEQ_FIRST   = 5,
            DEFAULT_SEQ_LAST    = 5,
            DEFAULT_ARRAY_FIRST = 5,
            DEFAULT_ARRAY_LAST  = 5,
            UNLIMITED = (size_t)(-1)
        };

        /* output parameters */
        size_t seq_first;
        size_t seq_last;
        bool   seq_show_indexes;
        size_t array_first;
        size_t array_last;
        bool   array_show_indexes;
        int    string_width;

        void initFormat(bool full = true);
        FormattedOutput(FILE *f = 0) : fd(f), disabled(0), htmlEncode(false) { initFormat(); };

        void disable()    { disabled++; }
        void enable()     { disabled--; }
        bool isActive()   { return (fd && disabled == 0); }
        void reset()      { disabled = 0; htmlEncode = false; };
        FILE* &getFd()    { return fd; };
        void setFd(FILE *f);
        void setHTMLEncode(bool enable) { htmlEncode = enable; }
        void print(const char *format, va_list *list);
        void print(const char *format, ...);
        void put(const char *data, size_t len);
        void flush()      { if (fd) fflush(fd); }
      private:
        FILE *fd;
        int disabled;
        bool htmlEncode;

        const static size_t ENCODE_BUF_SIZE = 1024;
        void encode(char c);
    };

    template<class T>
    class Ref
    {
        Allocator* allocator;
        T* ref;
      public:
        T* operator->() const {
            return ref;
        }

        bool operator == (T* val) const {
            return ref == val;
        }

        bool operator != (T* val) const {
            return ref != val;
        }

        bool isNull() const {
            return ref == NULL || ref->isNull();
        }

        T* value() const {
            return ref;
        }

        T* grab() {
			allocator = NULL;
            return ref;
        }

        #if 0 // unsafe: can cause implicit conversion in such code:
              // Value* v = rec->get(i);
              // int64_t x = v->intValue();
        template<class P>
        operator P* () const { return (P*)ref; }
        #endif

        template<class P>
        P* as() const { return (P*)ref; }

        void defuse() {
            allocator = NULL;
        }

        T* clone() {
            return allocator ? (T*)ref->clone(allocator) : ref;
        }


        Ref(Allocator* _allocator, T* _ref) : allocator(_allocator), ref(_ref) {}

        Ref(Ref<T> const& other) : allocator(other.allocator), ref(other.ref) {
            ((Ref<T>&)other).allocator = NULL; // grab object reference
        }

        ~Ref() {
            if (allocator != NULL) {
                ref->destroy(allocator);
            }
        }
    };


    /**
     * Base class for all values
     */
	class MCOSQL_API Value : public DynamicObject
    {
      public:
        /**
         * Get value type.
         * @return type of the value
         */
        virtual Type type() = 0;

        /**
         * Compare values.
         * @param x value to be compared with
         * @param nullsFirst if true than NULL is treated smaller than any other value (default), otherwise it will be treated larger than any other value
         * @return negative integer if this value is less than <code>val</code>, 0 if the same,
         * and positive integer if this value is greater than <code>val</code>
         */
        virtual int compare(Value* x, bool nullsFirst = true) = 0;

        /**
         * Check if true boolean constant.
         * @return <code>true</code> if value is true boolean constant
         */
        virtual bool isTrue();

        /**
         * Check if value is null.
         * @return  <code>true</code> if value is Null
         */
        bool isNull();

        /**
         * Check if this values contains components
         */
        virtual bool isContainer();


        /**
         * Get raw pointer to the value data.
         * For example, for integer value it points to 64 bit integer, for string value - points
         * to zero string,...
         * @return pointer to the value data
         */
        virtual void* pointer();

        /**
         * Convert value to string.
         * This method prints values to the specified buffer.
         * @param buf buffer to receive string representation of value
         * @param bufSize size of buffer
         * Not more than <code>bufSize</code> bytes of
         * string representation of the value are copied. If there is no NULL
         * byte among the first <code>bufSize</code> bytes, then result will not
         * be null-terminated.
         * @return number of bytes actually written (NULL byte is not included)
         */
        virtual size_t toString(char* buf, size_t bufSize) = 0;

        /**
         * Check if two values are equal.
         * Note: this method will return <code>true</code>
         * when comparing two Null values, but in SQL statement this comparison returns <code>Null</code>.
         * @param val value to be compared with
         * @return  <code>true</code> if values are equal
         */
        bool equals(Value* val)
        {
            return type() == val->type() && compare(val) == 0;
        }

        /**
         * Get integer value.
         * This method tries to cast value to integer and throw
         * InvalidTypeCast exception if it is not possible.
         * @return integer value
         * @throws InvalidTypeCast exception if this value is not convertible to integer
         */
        virtual int64_t intValue();

        /**
         * Get integer values scaled to the specified precision
         */
        virtual int64_t intValue(int precision);


        #ifndef NO_FLOATING_POINT
        /**
         * Get real value.
         * This method tries to cast value to floating point value and throw
         * InvalidTypeCast exception if it is not possible.
         * @return floating point value
         * @throws InvalidTypeCast exception if this value is not convertible to real
         */
        virtual double realValue();
        #endif

        /**
         * Get string representation of value.
         * Any scalar type can be converted to string.
         * @return string representation of value
         * @throws InvalidTypeCast exception if this value is array, struct or BLOB
         */
        virtual String* stringValue(Allocator* allocator);

#if MCO_CFG_SQL_SUPPORT_STD_STRING
        /**
         * Get std::string representation of value.
         * Any scalar type can be converted to string.
         * @return string representation of value
          * @throws InvalidTypeCast exception if this value is array, struct or BLOB
         */
        std::string stdStringValue(Allocator* allocator);
#endif

        /**
         * Get string representation of value.
         * Any scalar type can be converted to string.
         * @return reference to string representation of value
         * @throws InvalidTypeCast exception if this value is array, struct or BLOB
         */
        virtual Ref<String> stringRef(Allocator* allocator);

		/**
		 * Get binary value
         * @return reference to binary value
         * @throws InvalidTypeCast exception if this value is not of binary type
		 */
		virtual Ref<Binary> binaryValue(Allocator* allocator);

        /**
         * Get timestamp value.
         * @return int64_t value
         * @throws InvalidTypeCast exception if this value is not of DateTime and String types.
         */
        virtual int64_t timeValue();

        #ifdef UNICODE_SUPPORT
        /**
         * Get Unicode string representation of value.
         * Any scalar type can be converted to to string.
         * @return unicode string representation of value
         * @throws InvalidTypeCast exception if this value is array, struct or BLOB
         */
        virtual UnicodeString* unicodeStringValue(Allocator* allocator);

        /**
         * Get Unicode string representation of value.
         * Any scalar type can be converted to to string.
         * @return reference to unicode string representation of value
         * @throws InvalidTypeCast exception if this value is array, struct or BLOB
         */
        virtual Ref<UnicodeString> unicodeStringRef(Allocator* allocator);
        #endif

        /**
         * Serialize value to the buffer.
         * @param buf buffer where to place serialized value
         * @param size size of the buffer.
         * @return number of bytes written to the buffer or 0 if value doesn't fit in the buffer
         */
        virtual size_t serialize(char* buf, size_t size) = 0;

        /**
         * Deserialize value from the buffer.
         * @param buf buffer with serialized data (here to place serialized value
         * @param pos number of bytes read fetched from the buffer
         * @return unpacked value
         */
        static Value* deserialize(Allocator* allocator, char* buf, size_t &pos);

        /**
         * Get iterator through compund value
         * @return iterator through compund value components or NULL if value is not iterable
         */
        virtual Iterator<Value>* elements();

        /**
         * Return size of array  or string
         */
        virtual size_t size();

        virtual void output(FormattedOutput *streams, size_t n_streams);

        /**
         * Calculate value's hash code
         */
        virtual uint64_t hashCode();

        /**
         * Clone this value
         */
        virtual Value* clone(Allocator* alloc);

        static Value* clone(Allocator* allocator, Value* value) {
            return value == NULL ? value : value->clone(allocator);
        }

      protected:
        /**
         * This method is used to copy string representation of value into specified buffer.
         * Up to <code>size</code> bytes will be copied to the destination buffer.
         * @param dst destination buffer
         * @param src zero terminated string to be copied to buffer
         * @param size size of buffer
         * @return number of bytes actually placed in buffer
         */
        static size_t fillBuf(char* dst, char const* src, size_t size);
    };

    typedef Ref<Value> ValueRef;

    /**
     * Null (unknown) value from QSl point of view
     */
	class MCOSQL_API NullValue: public Value
    {
      public:
        virtual Type type();
        virtual bool isNull();
        virtual void* pointer();
        virtual int compare(Value* x, bool nullsFirst);
        virtual size_t toString(char* buf, size_t bufSize);
        virtual String* stringValue(Allocator* allocator);
        virtual int64_t intValue();
        virtual uint64_t hashCode();
        virtual Value* clone(Allocator* alloc);
        #ifndef NO_FLOATING_POINT
        virtual double realValue();
        #endif
        virtual size_t serialize(char* buf, size_t size);
        static NullValue* create();
        virtual void destroy(Allocator*) {}
        /**
         * String used to output null values
         */
        static char const* format;
    };

    /**
     * Single instance representing NULL value
     */
    extern MCOSQL_API NullValue Null;

    inline bool Value::isNull()
    {
        return this == &Null;
    }

    /**
     * Boolean value
     */
	class MCOSQL_API BoolValue: public Value
    {
      public:
        /**
         * <code>true</code> or <code>false</code>
         */
        const bool val;

        virtual Type type();
        virtual void* pointer();
        virtual int compare(Value* x, bool nullsFirst);
        virtual int64_t intValue();
        virtual uint64_t hashCode();
        virtual Value* clone(Allocator* alloc);
        #ifndef NO_FLOATING_POINT
        virtual double realValue();
        #endif
        virtual String* stringValue(Allocator* allocator);
        virtual size_t toString(char* buf, size_t bufSize);
        virtual void destroy(Allocator*) {}

        /**
         * Construct boolean value
         * @param val <code>true</code> or <code>false</code>
         */
        static BoolValue* create(bool val);

        virtual size_t serialize(char* buf, size_t size);

        /**
         * Instance of true boolean value
         */
        static BoolValue True;

        /**
         * Instance of false boolean value
         */
        static BoolValue False;

      private:
        BoolValue(bool v): val(v){}
    };

    /**
     * Signed 64-bit integer value
     * SQL engine doesn't support manipulation with unsigned 64-bit integer values.
     */
	class MCOSQL_API IntValue: public Value
    {
      public:
        int64_t val;

        virtual Type type();
        virtual void* pointer();
        virtual int compare(Value* x, bool nullsFirst);
        virtual int64_t intValue();
        virtual int64_t timeValue();
        virtual uint64_t hashCode();
        virtual Value* clone(Allocator* alloc);
        #ifndef NO_FLOATING_POINT
        virtual double realValue();
        #endif
        virtual String* stringValue(Allocator* allocator);
        virtual size_t toString(char* buf, size_t bufSize);
        virtual size_t serialize(char* buf, size_t size);
		virtual bool   isTrue();

        static IntValue* create(Allocator* allocator, int64_t v);

        IntValue(int64_t v = 0): val(v) {}
        DESTROY(IntValue)
   };

    /**
     * Numeric value with fixed precision
     */
	class MCOSQL_API NumericValue: public IntValue
    {
      public:
        int precision;

        int64_t scale() const {
            int64_t s = 1;
            for (int prec = precision; --prec >= 0; s *= 10);
            return s;
        }

        virtual Type type();
        virtual void* pointer();
        virtual int compare(Value* x, bool nullsFirst);
        virtual int64_t intValue();
        virtual int64_t intValue(int precision);
        virtual int64_t timeValue();
        virtual uint64_t hashCode();
        virtual Value* clone(Allocator* alloc);
        #ifndef NO_FLOATING_POINT
        virtual double realValue();
        #endif
        virtual String* stringValue(Allocator* allocator);
        virtual size_t toString(char* buf, size_t bufSize);
        virtual size_t serialize(char* buf, size_t size);

        bool parse(char const* str);

        int64_t scale(int prec) const {
            int64_t v = val;
            while (prec > precision) {
                v *= 10;
                prec -= 1;
            }
            return v;
        }

        static NumericValue* create(Allocator* allocator, int64_t scaledVal, int prec);
        NumericValue(int64_t scaledVal, int prec) : IntValue(scaledVal), precision(prec) {}
        #ifndef NO_FLOATING_POINT
        NumericValue(double realVal, int prec);
        #endif
        NumericValue(char const* strVal, int prec);
        NumericValue(char const* strVal);
        NumericValue(int64_t intPart, int64_t fracPart, int prec);
        DESTROY(NumericValue)
    };

    /**
     * Value of user defined type
     */
	class MCOSQL_API Binary: public Value
    {
      public:
        /**
         * Length of value
         */
        const size_t length;

		char* body() {
			return (char*)(this+1);
		}

        virtual Type type();
        virtual void* pointer();
		virtual size_t size();
		virtual uint64_t hashCode();
        virtual int compare(Value* x, bool nullsFirst);
        virtual String* stringValue(Allocator* allocator);
        virtual size_t toString(char* buf, size_t bufSize);
        virtual size_t serialize(char* buf, size_t size);
		virtual Value* clone(Allocator* allocator);
		virtual Ref<Binary> binaryValue(Allocator* allocator);

		static Binary* create(Allocator* allocator, void const* data, size_t len);
		static Binary* create(Allocator* allocator, size_t len);
		Binary(void const* data, size_t len);
		Binary(size_t len);
        virtual void destroy(Allocator* allocator);
    };


    /**
     * Value specifying date and time based on standard C time_t (seconds since 1970)
     */
	class MCOSQL_API DateTime: public IntValue
    {
      public:
        virtual Type type();
        virtual String* stringValue(Allocator* allocator);
        virtual size_t toString(char* buf, size_t bufSize);
        virtual size_t serialize(char* buf, size_t size);

        static DateTime* create(Allocator* allocator, int64_t v);
        static DateTime* create(Allocator* allocator, int64_t ticks, int scale);
		static DateTime* now(Allocator* allocator);

        virtual Value* clone(Allocator* alloc);

		double getSeconds() const;
		int64_t getTicks(int scale) const;

        DateTime(int64_t v): IntValue(v){}
        DateTime(int64_t ticks, int scale);
		DateTime();
        DESTROY(DateTime)

        /**
         * Format used to convert date to string by strftime function
         */
        static char const* format;
    };

    #ifndef NO_FLOATING_POINT

    /**
     * 64-bit ANSI floating point value
     */
	class MCOSQL_API RealValue: public Value
    {
      public:
        double val;

        virtual Type type();
        virtual void* pointer();
        virtual int compare(Value* x, bool nullsFirst);
        virtual int64_t intValue();
        virtual int64_t intValue(int precision);
        virtual double realValue();
        virtual uint64_t hashCode();
        virtual Value* clone(Allocator* alloc);
        virtual String* stringValue(Allocator* allocator);
        virtual size_t toString(char* buf, size_t bufSize);
        virtual size_t serialize(char* buf, size_t size);

        static RealValue* create(Allocator* allocator, double v);

        /**
         * Format used to convert real to string by sprintf function
         */
        static char const* format;

        RealValue(double v): val(v){}
        DESTROY(RealValue)
    };

	class MCOSQL_API AvgValue : public RealValue
    {
      public:
        int64_t count;
        virtual Type type();

        AvgValue(double v, int64_t c) : RealValue(v), count(c) {}
        virtual size_t serialize(char* buf, size_t size);
        virtual Value* clone(Allocator* alloc);
        DESTROY(AvgValue)
    };

    #endif


	class MCOSQL_API Container : public Value
    {
      public:
        Allocator* const allocator;

        Container(Allocator* _allocator) : allocator(_allocator) {}

        void shallowCopy(Container* src, size_t size);

        virtual bool isContainer();

        /**
         * Remove component
         */
        void remove(DynamicObject* obj) {
            obj->destroy(allocator);
        }

        void release() {
            destroy(allocator);
        }
    };

    class ContainerResource : public Resource
    {
    public:
        ContainerResource(Container* c) : Resource(c->allocator), container(c) {}
        ~ContainerResource() {
            container->release();
        }
        DESTROY(ContainerResource);
    private:
        Container* container;
    };

    /**
     * Common interface for strings and arrays
     */
    class MCOSQL_API List : public Container
    {
      public:
        class ListIterator : public Iterator<Value>
        {
            List*  const list;
            size_t const length;
            size_t curr;

          public:
            Value* next() {
                return curr < length ? list->getCopyAt(curr++) : NULL;
            }
            ListIterator(List* _list) : Iterator<Value>(_list->allocator), list(_list), length(_list->size()), curr(0) {}
            DESTROY(ListIterator)
        };

        Value* operator[](size_t index) {
            return getCopyAt(index);
        }

        /**
         * Get iterator through list elements
         */
        virtual Iterator<Value>* elements()
        {
            return new (allocator) ListIterator(this);
        }

        /**
         * Get element with specified index. Returned value should be deallocated by caller.
         * @param index element index
         * @return value of element with specified index
         * @throws OutOfBounds exception if index is out of bounds
         */
        virtual Value* getCopyAt(size_t index) = 0;

        /**
         * Get element with specified index. Returned value should be deallocated by caller.
         * @param index element index
         * @return value of element with specified index
         * @throws OutOfBounds exception if index is out of bounds
         */
        virtual Value* getCopyAt(Allocator* allocator, size_t index) = 0;

        /**
         * Get element with specified index.
         * @param index element index
         * @return value of element with specified index
         * @throws OutOfBounds exception if index is out of bounds
         */
        virtual ValueRef getAt(size_t index) = 0;

        /**
         * Get element with specified index.
         * @param index element index
         * @return value of element with specified index
         * @throws OutOfBounds exception if index is out of bounds
         */
        virtual ValueRef getAt(Allocator* allocator, size_t index) = 0;

        List(Allocator* allocator) : Container(allocator) {}
    };

    /**
     * String value
     */
	class MCOSQL_API String : public Value
    {
      public:
        virtual Value* charAt(Allocator* allocator, size_t index);
        virtual Type type();
        virtual void* pointer();
        virtual int compare(Value* x, bool nullsFirst = true);
        virtual String* stringValue(Allocator* allocator);
        virtual Ref<String> stringRef(Allocator* allocator);
        virtual int64_t intValue();
        virtual int64_t intValue(int precision);
        virtual uint64_t hashCode();
        virtual double realValue();
        virtual int64_t timeValue();
        virtual size_t toString(char* buf, size_t bufSize);
        virtual size_t serialize(char* buf, size_t size);
        virtual void output(FormattedOutput *streams, size_t n_streams);
		virtual Ref<Binary> binaryValue(Allocator* allocator);

        bool equals(String* str) {
            return compare(str) == 0;
        }

        bool equals(char const* str) {
            return compare(str) == 0;
        }

        /**
         * Compare with zero-terminated string.
         * @param str zero terminate string to be compared with
         * @return negative integer if this value is less than <code>val</code>, 0 if they are the same
         * and positive integer if this value is greater than <code>val</code>.
         */
        int compare(char const* str);

        /**
         * Check if string has specified prefix
         * @param prefix prefix string
         * @return true if strings has specified prefix
         */
        bool startsWith(String* prefix);

        bool endsWith(String* suffix);

        /**
         * Find position of specified substring.
         * @param s substring to be located
         * @return position of specified substring or -1 if not found
         */
        int indexOf(String* s);

        /**
         * Create string with specified content and length.
         * @param str string body
         * @param len string length
         * @return created string
         */
        static String* create(Allocator* allocator, char const* str, size_t len);

        /**
         * Create string with specified content.
         * @param str zero terminated string body
         * @return created string
         */
        static String* create(Allocator* allocator, char const* str);

        /**
         * Create string with specified length.
         * @param len string length
         * @return created string
         */
        static String* create(Allocator* allocator, size_t len);

        /**
         * Convert string to upper case.
         * @return uppercase string
         */
        virtual String* toUpperCase(Allocator* allocator);

        /**
         * Convert string to lower case.
         * @return lowercase string
         */
        virtual String* toLowerCase(Allocator* allocator);

        /**
         * Concatenate two strings.
         * @param head left string
         * @param tail right string
         * @return concatenation of two strings
         */
        static String* concat(Allocator* allocator, String* head, String* tail);

        /**
         * Create string with print-like formatting.
         * @param fmt print-like format string
         * @return string with all placeholders substituted with values
         */
        static String* format(Allocator* allocator, char const* fmt, ...);

        /**
         * Get string body.
         * @return char array (may be not zero terminated)
         */
        virtual char* body() = 0;

        /**
         * Convert to zero-terminated string.
         * @return C zero-terminated string
         */
        virtual char* cstr() = 0;

        /**
         * Get substring of this string.
         * @param pos start position of substring
         * @param len substring length
         * @return substring started at specified position and length chars long
         */
        String* substr(Allocator* allocator, size_t pos, size_t len);
    };

    class StringLiteral: public String
    {
        friend class String;
      public:
        virtual size_t size();
        virtual char* body();
        virtual char* cstr();

      protected:
        const uint32_t length;
        char chars[1];

      public:
        virtual Value* clone(Allocator* alloc);

        virtual Value* charAt(Allocator* allocator, size_t index);

        StringLiteral(char const* s, size_t l): length((uint32_t)l)
        {
            memcpy(chars, s, l);
            chars[l] = '\0';
        }
        StringLiteral(size_t l): length((uint32_t)l){}
        virtual void destroy(Allocator* allocator);
    };

	class MCOSQL_API StringRef: public String
    {
        friend class String;
      public:
        virtual Value* charAt(Allocator* allocator, size_t index);
        virtual Value* clone(Allocator* alloc);
        virtual size_t size();
        virtual char* body();
        virtual char* cstr();

      private:
        const uint32_t length;
        char* chars;

      public:
        StringRef(char const* s, size_t l): length((int)l)
        {
            chars = (char*)s;
        }
        StringRef(char const* s);

        DESTROY(StringRef)
    };

    #ifdef UNICODE_SUPPORT

    /**
     * Unicode string value
     */
    class MCOSQL_API UnicodeString : public Value
    {
      public:
        virtual Value* charAt(Allocator* allocator, size_t index) = 0;
        virtual Type type();
        virtual void* pointer();
        virtual int compare(Value* x, bool nullsFirst = true);
        virtual String* stringValue(Allocator* allocator);
        virtual size_t toString(char* buf, size_t bufSize);
        virtual uint64_t hashCode();

        virtual UnicodeString* unicodeStringValue(Allocator* allocator);
        virtual Ref<UnicodeString> unicodeStringRef(Allocator* allocator);
        virtual size_t serialize(char* buf, size_t size);

        /**
         * Compare with zero-terminated string.
         * @param str zero terminate string to be compared with
         * @return negative integer if this value is less than <code>val</code>, 0 if they are the same
         * and positive integer if this value is greater than <code>val</code>
         */
        int compare(wchar_t const* str);

        /**
         * Check if string has specified prefix
         * @param prefix prefix string
         * @return true if strings has specified prefix
         */
        bool startsWith(UnicodeString* prefix);

        /**
         * Find position of specified substring.
         * @param s substring to be located
         * @return position of specified substring or -1 if not found
         */
        int indexOf(UnicodeString* s);

        /**
         * Create string with specified content and length.
         * @param str string body
         * @param len string length
         * @return created string
         */
        static UnicodeString* create(Allocator* allocator, wchar_t const* str, size_t len);

        /**
         * Create string with specified content.
         * @param str zero terminated string body
         * @return created string
         */
        static UnicodeString* create(Allocator* allocator, wchar_t const* str);

        /**
         * Create string with specified length.
         * @param len string length
         * @return created string
         */
        static UnicodeString* create(Allocator* allocator, size_t len);
        static UnicodeString* create(Allocator* allocator, String* mbs);

        /**
         * Convert string to upper case.
         * @return uppercase string
         */
        virtual UnicodeString* toUpperCase(Allocator* allocator);

        /**
         * Convert string to lower case.
         * @return lowercase string
         */
        virtual UnicodeString* toLowerCase(Allocator* allocator);

        /**
         * Concatenate two strings.
         * @param head left string
         * @param tail right string
         * @return concatenation of two strings
         */
        static UnicodeString* concat(Allocator* allocator, UnicodeString* head, UnicodeString* tail);

        /**
         * Create string with print-like formatting.
         * @param fmt print-like format string
         * @return string with all placeholders substituted with values
         */
        static UnicodeString* format(Allocator* allocator, wchar_t const* fmt, ...);

        /**
         * Get string body.
         * @return char array (may be not zero-terminated)
         */
        virtual wchar_t* body() = 0;

        /**
         * Convert to zero-terminated string.
         * @return C zero-terminated string
         */
        virtual wchar_t* wcstr() = 0;

        /**
         * Get substring of this string.
         * @param pos start position of substring
         * @param len substring length
         * @return substring started at specified position and length chars long
         */
        UnicodeString* substr(Allocator* allocator, size_t pos, size_t len);
    };


    class UnicodeStringLiteral: public UnicodeString
    {
        friend class UnicodeString;
      public:
        virtual Value* charAt(Allocator* allocator, size_t index);
        virtual size_t size();
        virtual wchar_t* body();
        virtual wchar_t* wcstr();
        virtual Value* clone(Allocator* alloc);

        const uint32_t length;
      protected:
        wchar_t chars[1];

      public:
        UnicodeStringLiteral(wchar_t const* s, size_t l): length((uint32_t)l)
        {
            memcpy(chars, s, l* sizeof(wchar_t));
            chars[l] = '\0';
        }
        UnicodeStringLiteral(size_t l): length((uint32_t)l){}
        virtual void destroy(Allocator* allocator);
    };

    class UnicodeStringRef: public UnicodeString
    {
        friend class UnicodeString;
      public:
        virtual Value* charAt(Allocator* allocator, size_t index);
        virtual size_t size();
        virtual wchar_t* body();
        virtual wchar_t* wcstr();
        virtual Value* clone(Allocator* alloc);

      private:
        const uint32_t length;
        wchar_t* chars;

      public:
        UnicodeStringRef(wchar_t const* s, size_t l): length((uint32_t)l)
        {
            chars = (wchar_t*)s;
        }
        DESTROY(UnicodeStringRef)
    };
    #endif

    /**
     * Reference value
     */
    class MCOSQL_API Reference: public Value
    {
      public:
        virtual Type type();
        virtual size_t serialize(char* buf, size_t size);
        virtual uint64_t hashCode();

        /**
         * Get referenced record.
         * @return record referenced by this reference
         */
        virtual Record* dereference(Runtime* runtime) = 0;
        DESTROY(Reference)
    };

    /**
     * Compound (structure) value
     * This value is mostly needed for array elements (array of structs).
     * In this case, this value provides base address for the structure.
     * It can also be used to extract structure components by index and
     * perform component-by-component comparison of records.
     */
	class MCOSQL_API Struct : public Container
    {
      public:
        virtual Type type();
        virtual size_t toString(char* buf, size_t bufSize);
        virtual int compare(Value* x, bool nullsFirst = true);
        virtual size_t serialize(char* buf, size_t size);
        virtual void output(FormattedOutput *streams, size_t n_streams);
        virtual bool isTuple();

        /**
         * Get value of component of the record with the specified index.
         * @param index record component index
         * @return value of the record with specified index
         * @throws OutOfBounds exception if there is no field with such index
         */
        virtual ValueRef get(size_t i) = 0;

        /**
         * Get copy of the record's component with specified index.
         * This value should be deallocated by caller.
         * @param index record component index
         * @return value of the record with specified index
         * @throws OutOfBounds exception if there is no field with such index
         */
        virtual Value*   getCopy(size_t i) = 0;

        /**
         * Get value of integer record component with specified index.
         * @param index record component index
         * @return value of the record with specified index
         * @throws OutOfBounds exception if there is no field with such index
         */
        virtual int64_t  getInt(size_t i);

        /**
         * Get value of floating point record component with specified index.
         * @param index record component index
         * @return value of the record with specified index
         * @throws OutOfBounds exception if there is no field with such index
         */
        virtual double  getReal(size_t i);

        /**
         * Get number of components in the records.
         * It is the same as number of fields in the corresponding type.
         * @return number of records components
         */
        virtual size_t   size() = 0;

        /**
         * Set component of the record with specified index.
         * @param index record component index
         * @param value new value of the component
         * @throws OutOfBounds exception if there is no field with such index
         */
        virtual void set(size_t index, Value* value, bool append = false);

        /**
         * Get struct or array component of the record with specified index for update. Value should be deallocated by caller
         * @param index record component index
         * @return value which components or elements will be updated
         * @throws OutOfBounds exception if there is no field with such index
         */
        virtual Value* update(size_t index);

        class StructIterator : public Iterator<Value>
        {
            Struct* s;
            size_t curr;
            size_t nComponents;

          public:
            Value* next() {
                return curr < nComponents ? s->getCopy(curr++) : NULL;
            }
            StructIterator(Struct* _s) : Iterator<Value>(_s->allocator), s(_s), curr(0), nComponents(_s->size()) {}
            DESTROY(StructIterator)
        };

        virtual Iterator<Value>* elements()
        {
            return new (allocator) StructIterator(this);
        }

        /**
         * Get value to which this value belongs.
         * @return outer struct or array or <code>NULL</code> if none
         */
        virtual Value* scope();

        virtual Value* clone(Allocator* allocator);

        uint64_t hashCode();

        Struct(Allocator* allocator) : Container(allocator) {}
    };

    /**
     * Array value
     */
    class MCOSQL_API Array : public List
    {
      public:
        Array(Allocator* allocator) : List(allocator) {}

		static Array* create(Allocator* allocator, Type elemType, int elemSize, size_t size);
		Array* makeNullable();

        virtual String* stringValue(Allocator* allocator);
        virtual Type type();
        virtual int compare(Value* x, bool nullsFirst);
        virtual size_t toString(char* buf, size_t bufSize);
        virtual size_t serialize(char* buf, size_t size);
        virtual void output(FormattedOutput *streams, size_t n_streams);
		virtual Array* getNullBitmap();

        /**
         * Set array element.
         * @param index index of the element
         * @param value new element value
         * @throws IndexOutOfBounds exception if index is out of range
         */
        virtual void setAt(size_t index, Value* value) = 0;

        /**
         * Get structure or array element with specified index for update.
         * @param index element index
         * @return value of element with specified index which components or elements will be updated
         * @throws OutOfBounds exception if index is out of bounds
         */
        virtual Value* updateAt(size_t index) = 0;

        /**
         * Change array size.
         * @param newSize new size of the array (can be larger or smaller than original value)
         */
        virtual void setSize(size_t newSize) = 0;

        /**
         * Get array body: copy specified number of elements with offset to the buffer.
         * This method can be used only for arrays of scalar types.
         * @param dst buffer to receive array elements
         * @param offs offset in array from which elements will be taken
         * @param len how many elements will be copied to the buffer
         * @throws OutOfBounds exception if offs and len don't specify valid segment within array
         */
        virtual void getBody(void* dst, size_t offs, size_t len) = 0;

        /**
         * Set array body: copy specified number of elements from buffer to the array with specified offset.
         * This method can be used only for arrays of scalar types
         * @param src buffer with elements to be copied
         * @param offs offset in array from which elements will be stored
         * @param len how much elements will be copied from the buffer
         * @throws OutOfBounds exception if offs and len don't specify valid segment within array
         */
        virtual void setBody(void* src, size_t offs, size_t len) = 0;

        /**
         * Get size of array element
         * @return size in bytes of array element
         */
        virtual size_t getElemSize() const = 0;

        /**
         * Get type of array element
         * @return type of array element
         */
        virtual Type getElemType() const = 0;

        /**
         * Check if all array data is stored in single segment of memory
         * @return true if all array data is located at memory segment at address returned by pointer()
         */
        virtual bool isPlain();

        /**
         * Calculate value's hash code
         */
        virtual uint64_t hashCode();
    };


    /**
     * Large binary object
     */
    class MCOSQL_API Blob: public Container
    {
      public:
        virtual Type type();
        virtual int compare(Value* x, bool nullsFirst);
        virtual size_t toString(char* buf, size_t bufSize);
        virtual size_t serialize(char* buf, size_t size);

		Ref<Binary> binaryValue(Allocator* allocator);

        /**
         * Return number of bytes available to be extracted.
         * It is not the total size of BLOB. It can be smaller than BLOB size. For
         * example, if BLOB consists of several segments, it can be size of the segment.
         * @return number of bytes which can be read using one operation
         */
        virtual size_t available() = 0;

        /**
         * Copy BLOB data to the buffer.
         * This method copies up to <code>size</code> bytes
         * from the current position in the BLOB to the specified buffer. Then, current position
         * is moved forward to number of fetched bytes.
         * @param buffer destination for BLOB data
         * @param size buffer size
         * @return actual number of bytes transferred
         * It can be smaller than <code>size</code> if end of BLOB or BLOB segment is reached.
         */
        virtual size_t get(void* buffer, size_t size) = 0;

        /**
         * Append new data to the BLOB.
         * Append always performed at the end of BLOB and doesn't change current position for GET method.
         * @param buffer source of the data to be inserted in BLOB
         * @param size number of bytes to be added to BLOB
         */
        virtual void append(void const* buffer, size_t size) = 0;

        /**
         * Reset current read position to the beginning of the BLOB.
         * Or set it to a specified value.
         * @param optional beginning read position
         */
        virtual void reset(size_t pos = 0) = 0;

        /**
         * Eliminate content of the BLOB.
         */
        virtual void truncate() = 0;

        Blob(Allocator* allocator) : Container(allocator) {}
    };

#if MCO_CFG_SQL_SUPPORT_STD_STRING
    inline std::string Value::stdStringValue(Allocator* allocator)
    {
        Ref<String> ref(stringRef(allocator));
        return std::string(ref->cstr(), ref->size());
    }
#endif


}

#endif
