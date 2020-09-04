/****************************************************************
 *                                                              *
 *  mcoseq.hpp                                                  *
 *                                                              *
 *  This file is a part of the eXtremeDB source code.           *
 *                                                              *
 *  Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.  *
 *                                                              *
 *  Sequence C++ API                                            * 
 *                                                              *
 ****************************************************************/

#ifndef MCO_SEQ_HPP__
#define MCO_SEQ_HPP__

#include "mcoseq.h"
#include "mco.hpp"

template<class T>
class Sequence;

#ifdef max // min/max macros are defined in windows.h
#undef max
#undef min
#endif

class GenericSequence 
{   
  public:
    class SequenceIterator :  public mco_seq_iterator_t { 
      public:
        SequenceIterator(); 
        ~SequenceIterator();
    };
    SequenceIterator* iter;

    void reset();   
    bool nextTile();

    uint2 tileSize() const { return iter->tile_size; }
    uint2 elemSize() const { return iter->elem_size; }

    uint8 count() const;
    uint4 approxDistinctCount() const;

    Sequence<uint4> groupAggApproxDistinctCount(GenericSequence const& groupBy) const;
    Sequence<uint8> groupAggCount() const;

    Sequence<uint4> hash() const;
    Sequence<uint4> hash(Sequence<uint4> const& other) const;

    void hashAggCount(Sequence<uint8>& result, GenericSequence& groups) const;
    void hashAggApproxDistinctCount(Sequence<uint4>& result, GenericSequence& groups, GenericSequence const& groupBy) const;
    void hashAggDistinctCount(Sequence<uint8>& result, GenericSequence& groups, GenericSequence const& groupBy) const;
    void hashAggDupCount(Sequence<uint8>& result, GenericSequence& groups, GenericSequence const& groupBy, mco_size_t minOccurrences) const;

    mco_seq_rle_count_t rleCount(int pos) const;
    
  protected: 
    GenericSequence(SequenceIterator* it = new SequenceIterator()) : iter(it) {}
    ~GenericSequence();
};

template<class T>
class Sequence : public GenericSequence
{
  public:
    typedef mco_seq_bool (*predicate_t)(T x);
    typedef T (*unary_function_t)(T x);
    typedef T (*binary_function_t)(T x, T y);

    T const* tile() const { return (T*)iter->tile.u.arr_char; }
    T const* operator*() const { return tile(); }

    bool next(T& val);
    T operator++(); 

    Sequence<T> operator +(Sequence<T> const& other) const;
    Sequence<T> operator -(Sequence<T> const& other) const;
    Sequence<T> operator *(Sequence<T> const& other) const;
    Sequence<T> operator /(Sequence<T> const& other) const;
    Sequence<T> operator %(Sequence<T> const& other) const;
    Sequence<T> operator -() const;

    Sequence<mco_seq_bool> operator ==(Sequence<T> const& other) const;
    Sequence<mco_seq_bool> operator !=(Sequence<T> const& other) const;
    Sequence<mco_seq_bool> operator >=(Sequence<T> const& other) const;
    Sequence<mco_seq_bool> operator <=(Sequence<T> const& other) const;
    Sequence<mco_seq_bool> operator >(Sequence<T> const& other) const;
    Sequence<mco_seq_bool> operator <(Sequence<T> const& other) const;

    Sequence<mco_seq_bool> operator ||(Sequence<mco_seq_bool> const& other) const;
    Sequence<mco_seq_bool> operator &&(Sequence<mco_seq_bool> const& other) const;
    Sequence<mco_seq_bool> operator ^(Sequence<mco_seq_bool> const& other) const;
    Sequence<mco_seq_bool> operator !() const;

    Sequence<T> max(Sequence<T> const& other) const;
    Sequence<T> min(Sequence<T> const& other) const;

    Sequence<T> abs() const;
    Sequence<T> diff() const;
    Sequence<T> unique() const;
    Sequence<T> reverse() const;
    Sequence<double> norm() const;

    void get(std::vector<T>& result);

    Sequence<T> apply(unary_function_t func) const;
    Sequence<T> apply(Sequence<T> const& other, binary_function_t func) const;
    Sequence<mco_seq_bool> check(predicate_t predicate) const;

    Sequence<T> map(Sequence<mco_seq_no_t> const& positions) const;
    Sequence<T> combine(Sequence<T> const& other, mco_seq_order_t order) const;
    Sequence<T> thin(mco_size_t origin, mco_size_t step) const;
    Sequence<T> repeat(int nTimes) const;
    Sequence<T> concat(Sequence<T> const& other) const;
    Sequence<T> limit(mco_seq_no_t from, mco_seq_no_t till) const;
    
    Sequence<T> operator[](Sequence<mco_seq_no_t> const& positions) const { 
        return map(positions);
    }
    
    Sequence<T> operator()(mco_seq_no_t from, mco_seq_no_t till) const { 
        return limit(from, till);
    }

    Sequence<T> operator,(Sequence<T> const& other) const { 
        return concat(other);
    }

    template<class R>
    Sequence<R> iif(Sequence<R> const& then, Sequence<R> const& otherwise) const;

    template<class R>
    Sequence<R> cond(Sequence<R> const& then, Sequence<R> const& otherwise) const;

    Sequence<T> filter(Sequence<mco_seq_bool> const& condition) const;
    Sequence<mco_seq_no_t> filterPos() const;

    double wsum(Sequence<T> const& weights) const;
    double wavg(Sequence<T> const& weights) const;
    double corr(Sequence<T> const& other) const;
    double cov(Sequence<T> const& other) const;

    T aggMax() const;
    T aggMin() const;

    template<class R>
    R aggSum() const;
    
    template<class R>
    R aggPrd() const;

    double aggAvg() const;    
    double aggVar() const;
    double aggDev() const;
    double aggVarSamp() const;
    double aggDevSamp() const;

    Sequence<T> groupAggMax(GenericSequence const& groupBy) const;
    Sequence<T> groupAggMin(GenericSequence const& groupBy) const;
    Sequence<T> groupAggLast(GenericSequence const& groupBy) const;
    Sequence<T> groupAggFirst(GenericSequence const& groupBy) const;

    Sequence<double> groupAggWavg(GenericSequence const& weight, GenericSequence const& groupBy) const;

    template<class R>
    Sequence<R> groupAggSum(GenericSequence const& groupBy) const;

    Sequence<double> groupAggAvg(GenericSequence const& groupBy) const;
    Sequence<double> groupAggVar(GenericSequence const& groupBy) const;
    Sequence<double> groupAggDev(GenericSequence const& groupBy) const;
    Sequence<double> groupAggVarSamp(GenericSequence const& groupBy) const;
    Sequence<double> groupAggDevSamp(GenericSequence const& groupBy) const;

    Sequence<T> gridAggMax(mco_size_t interval) const;
    Sequence<T> gridAggMin(mco_size_t interval) const;

    template<class R>
    Sequence<R> gridAggSum(mco_size_t interval) const;

    Sequence<double> gridAggAvg(mco_size_t interval) const;
    Sequence<double> gridAggVar(mco_size_t interval) const;
    Sequence<double> gridAggDev(mco_size_t interval) const;
    Sequence<double> gridAggVarSamp(mco_size_t interval) const;
    Sequence<double> gridAggDevSamp(mco_size_t interval) const;

    Sequence<T> windowAggMax(mco_size_t interval) const;
    Sequence<T> windowAggMin(mco_size_t interval) const;

    template<class R>
    Sequence<R> windowAggSum(mco_size_t interval) const;

    Sequence<double> windowAggAvg(mco_size_t interval) const;
    Sequence<double> windowAggVar(mco_size_t interval) const;
    Sequence<double> windowAggDev(mco_size_t interval) const;
    Sequence<double> windowAggVarSamp(mco_size_t interval) const;
    Sequence<double> windowAggDevSamp(mco_size_t interval) const;

    void hashAggMax(Sequence<T>& result, GenericSequence& groups, GenericSequence const& groupBy) const;
    void hashAggMin(Sequence<T>& result, GenericSequence& groups, GenericSequence const& groupBy) const;

    template<class R>
    void hashAggSum(Sequence<R>& result, GenericSequence& groups, GenericSequence const& groupBy) const;

    void hashAggAvg(Sequence<double>& result, GenericSequence& groups, GenericSequence const& groupBy) const;

    Sequence<T> topMax(mco_size_t top) const;    
    Sequence<T> topMin(mco_size_t top) const;
    Sequence<mco_seq_no_t> topPosMax(mco_size_t top) const;    
    Sequence<mco_seq_no_t> topPosMin(mco_size_t top) const;

    Sequence<T> cumAggMax() const;
    Sequence<T> cumAggMin() const;

    template<class R>
    Sequence<R> cumAggSum() const;

    template<class R>
    Sequence<R> cumAggPrd() const;

    Sequence<double> cumAggAvg() const;
    Sequence<double> cumAggVar() const;
    Sequence<double> cumAggDev() const;
    Sequence<double> cumAggVarSamp() const;
    Sequence<double> cumAggDevSamp() const;

    Sequence<uint8> histogram(T minValue, T maxValue, mco_size_t nIntervals) const;
    Sequence<mco_seq_no_t> cross(int first_cross_direction) const;    
    Sequence<mco_seq_no_t> extrema(int first_extremum) const;

    static void sort(std::vector<T> const& elements, std::vector<mco_seq_no_t>& permutation, mco_seq_order_t order);    
    Sequence<T> permutate(std::vector<mco_seq_no_t> const& permutation) const;

    template<class R>
    operator Sequence<R>() const;

    template<class TS>
    Sequence<T> stretch(Sequence<TS> const& ts1, Sequence<TS> const& ts2, T filler) const;

    template<class TS>
    Sequence<T> stretch0(Sequence<TS> const& ts1, Sequence<TS> const& ts2, T filler) const;

    template<class TS>
    Sequence<T> asofJoin(Sequence<TS> const& ts1, Sequence<TS> const& ts2) const;

    uint4 aggApproxDistinctHashValues() const;
    Sequence<uint4> groupAggApproxDistinctHashValues(GenericSequence const& groupBy) const;

    Sequence<T> rleDecode() const;

    Sequence& operator= (Sequence const& other) { 
        delete iter;
        iter = other.iter;
        ((Sequence&)other).iter = NULL;  // grab iterator
        return *this;
    }

    Sequence() {}
    Sequence(Sequence const& other) : GenericSequence(other.iter) { ((Sequence&)other).iter = NULL; } // grab iterator
    Sequence(T const& val);
    Sequence<T>(char const* str);
    Sequence(std::vector<T> const& val);

};

//
// Implementation of GenericSequence methods
//

inline GenericSequence::SequenceIterator::SequenceIterator() 
{ 
    opd[0] = NULL;
    opd[1] = NULL;
    opd[2] = NULL;
    buf = NULL;
    reset = NULL;
    bounded = false;
}
        
inline GenericSequence::SequenceIterator::~SequenceIterator() 
{ 
    next = NULL; // prevent any further access
    if (mco_seq_is_hash_aggreate(this)) {
        if (!mco_seq_free_hash(this)) { 
            return;
        }
    }
    for (int i = 0; i < 3; i++) { 
        if (opd[i]) { 
            if (opd[i]->bounded) { 
                opd[i]->bounded = false;
            } else { 
                delete (SequenceIterator*)opd[i];
            }
        }
    }   
    mco_seq_free_buffer(this);
}

inline GenericSequence::~GenericSequence()
{
    if (iter) { 
        if (iter->bounded) { 
            iter->bounded = false;
        } else { 
            delete iter;
        }
    }
}

inline uint8 GenericSequence::count() const {
    mco_seq_iterator_t result;
    uint8 count;
    MCO_CPP_CHECK(mco_seq_agg_count(&result, iter)); 
    MCO_CPP_CHECK(mco_seq_next_uint8(&result, &count));         
    return count;                                                         
}

inline uint4 GenericSequence::approxDistinctCount() const {
    mco_seq_iterator_t result;
    uint4 count;
    MCO_CPP_CHECK(mco_seq_agg_approxdc(&result, iter)); 
    MCO_CPP_CHECK(mco_seq_next_uint4(&result, &count));         
    return count;                                                         
}

inline Sequence<uint4> GenericSequence::groupAggApproxDistinctCount(GenericSequence const& groupBy) const {
    Sequence<uint4> result;
    MCO_CPP_CHECK(mco_seq_group_agg_approxdc(result.iter, iter, groupBy.iter)); 
    return result;                                                         
}

inline Sequence<uint8> GenericSequence::groupAggCount() const {
    Sequence<uint8> result;
    MCO_CPP_CHECK(mco_seq_group_agg_count(result.iter, iter)); 
    return result;                                                         
}

inline Sequence<uint4> GenericSequence::hash() const {
    Sequence<uint4> result;
    MCO_CPP_CHECK(mco_seq_hash(result.iter, iter, NULL)); 
    return result;                                                         
}

inline Sequence<uint4> GenericSequence::hash(Sequence<uint4> const& other) const {
    Sequence<uint4> result;
    MCO_CPP_CHECK(mco_seq_hash(result.iter, iter, other.iter)); 
    return result;                                                         
}

inline void GenericSequence::hashAggCount(Sequence<uint8>& result, GenericSequence& groups) const {
    MCO_CPP_CHECK(mco_seq_hash_agg_count(result.iter, groups.iter, iter, 0)); 
}

inline void GenericSequence::hashAggApproxDistinctCount(Sequence<uint4>& result, GenericSequence& groups, GenericSequence const& groupBy) const {
    MCO_CPP_CHECK(mco_seq_hash_agg_approxdc(result.iter, groups.iter, iter, groupBy.iter, 0)); 
}

inline void GenericSequence::hashAggDistinctCount(Sequence<uint8>& result, GenericSequence& groups, GenericSequence const& groupBy) const {
    MCO_CPP_CHECK(mco_seq_hash_agg_distinct_count(result.iter, groups.iter, iter, groupBy.iter, 0, 0)); 
}

inline void GenericSequence::hashAggDupCount(Sequence<uint8>& result, GenericSequence& groups, GenericSequence const& groupBy, mco_size_t minOccurrences) const {
    MCO_CPP_CHECK(mco_seq_hash_agg_dup_count(result.iter, groups.iter, iter, groupBy.iter, 0, 0, minOccurrences)); 
}

inline void GenericSequence::reset() { 
    MCO_CPP_CHECK(iter->reset(iter));
}

inline bool GenericSequence::nextTile() { 
    MCO_RET rc = iter->next(iter);
    if (rc != MCO_S_CURSOR_END) {                   
        MCO_CPP_CHECK(rc);                          
    }                                               
    return rc == MCO_S_OK;                          
}

inline mco_seq_rle_count_t GenericSequence::rleCount(int pos) const { 
    return mco_seq_rle_count(iter, pos);
}


//
// Implementation of Sequence<T> methods
//


#define MCO_SEQ_SCALAR_OP(cpp_op, c_op, args, params)           \
    template<>                                                  \
    inline Sequence<int1> Sequence<int1>::cpp_op params const { \
        Sequence<int1> result;                                  \
        MCO_CPP_CHECK(mco_seq_##c_op##_int1 args);              \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<int2> Sequence<int2>::cpp_op params const { \
        Sequence<int2> result;                                  \
        MCO_CPP_CHECK(mco_seq_##c_op##_int2 args);              \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<int4> Sequence<int4>::cpp_op params const { \
        Sequence<int4> result;                                  \
        MCO_CPP_CHECK(mco_seq_##c_op##_int4 args);              \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<mco_int8> Sequence<mco_int8>::cpp_op params const { \
        Sequence<mco_int8> result;                              \
        MCO_CPP_CHECK(mco_seq_##c_op##_int8 args);              \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<uint1> Sequence<uint1>::cpp_op params const {\
        Sequence<uint1> result;                                 \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint1 args);             \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<uint2> Sequence<uint2>::cpp_op params const {\
        Sequence<uint2> result;                                 \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint2 args);             \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<uint4> Sequence<uint4>::cpp_op params const {\
        Sequence<uint4> result;                                 \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint4 args);             \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<uint8> Sequence<uint8>::cpp_op params const {\
        Sequence<uint8> result;                                 \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint8 args);             \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<float> Sequence<float>::cpp_op params const {\
        Sequence<float> result;                                 \
        MCO_CPP_CHECK(mco_seq_##c_op##_float args);             \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<double> Sequence<double>::cpp_op params const { \
        Sequence<double> result;                                \
        MCO_CPP_CHECK(mco_seq_##c_op##_double args);            \
        return result;                                          \
    }                                                           \

#define MCO_SEQ_VOID_OP(cpp_op, c_op, args, params)             \
    template<>                                                  \
    inline void Sequence<int1>::cpp_op params const {           \
        MCO_CPP_CHECK(mco_seq_##c_op##_int1 args);              \
    }                                                           \
    template<>                                                  \
    inline void Sequence<int2>::cpp_op params const {           \
        MCO_CPP_CHECK(mco_seq_##c_op##_int2 args);              \
    }                                                           \
    template<>                                                  \
    inline void Sequence<int4>::cpp_op params const {           \
        MCO_CPP_CHECK(mco_seq_##c_op##_int4 args);              \
    }                                                           \
    template<>                                                  \
    inline void Sequence<mco_int8>::cpp_op params const {           \
        MCO_CPP_CHECK(mco_seq_##c_op##_int8 args);              \
    }                                                           \
    template<>                                                  \
    inline void Sequence<uint1>::cpp_op params const {          \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint1 args);             \
    }                                                           \
    template<>                                                  \
    inline void Sequence<uint2>::cpp_op params const {          \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint2 args);             \
    }                                                           \
    template<>                                                  \
    inline void Sequence<uint4>::cpp_op params const {          \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint4 args);             \
    }                                                           \
    template<>                                                  \
    inline void Sequence<uint8>::cpp_op params const {          \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint8 args);             \
    }                                                           \
    template<>                                                  \
    inline void Sequence<float>::cpp_op params const {          \
        MCO_CPP_CHECK(mco_seq_##c_op##_float args);             \
    }                                                           \
    template<>                                                  \
    inline void Sequence<double>::cpp_op params const {         \
        MCO_CPP_CHECK(mco_seq_##c_op##_double args);            \
    }                                                           



#define MCO_SEQ_SCALAR_XOP(cpp_op, c_op, args, params)          \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<mco_int8> Sequence<int1>::cpp_op params const {  \
        Sequence<mco_int8> result;                              \
        MCO_CPP_CHECK(mco_seq_##c_op##_int1 args);              \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<mco_int8> Sequence<int2>::cpp_op params const {  \
        Sequence<mco_int8> result;                              \
        MCO_CPP_CHECK(mco_seq_##c_op##_int2 args);              \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<mco_int8> Sequence<int4>::cpp_op params const {  \
        Sequence<mco_int8> result;                              \
        MCO_CPP_CHECK(mco_seq_##c_op##_int4 args);              \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<mco_int8> Sequence<mco_int8>::cpp_op params const {  \
        Sequence<mco_int8> result;                              \
        MCO_CPP_CHECK(mco_seq_##c_op##_int8 args);              \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<uint8> Sequence<uint1>::cpp_op params const {\
        Sequence<uint8> result;                                 \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint1 args);             \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<uint8> Sequence<uint2>::cpp_op params const {\
        Sequence<uint8> result;                                 \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint2 args);             \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<uint8> Sequence<uint4>::cpp_op params const {\
        Sequence<uint8> result;                                 \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint4 args);             \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<uint8> Sequence<uint8>::cpp_op params const {\
        Sequence<uint8> result;                                 \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint8 args);             \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<double> Sequence<float>::cpp_op params const { \
        Sequence<double> result;                                \
        MCO_CPP_CHECK(mco_seq_##c_op##_float args);             \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<double> Sequence<double>::cpp_op params const { \
        Sequence<double> result;                                \
        MCO_CPP_CHECK(mco_seq_##c_op##_double args);            \
        return result;                                          \
    }                                                           \

#define MCO_SEQ_SCALAR_SOP(type, cpp_op, c_op, args, params)    \
    template<>                                                  \
    inline Sequence<type> Sequence<int1>::cpp_op params const { \
        Sequence<type> result;                                  \
        MCO_CPP_CHECK(mco_seq_##c_op##_int1 args);              \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<type> Sequence<int2>::cpp_op params const { \
        Sequence<type> result;                                  \
        MCO_CPP_CHECK(mco_seq_##c_op##_int2 args);              \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<type> Sequence<int4>::cpp_op params const { \
        Sequence<type> result;                                  \
        MCO_CPP_CHECK(mco_seq_##c_op##_int4 args);              \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<type> Sequence<mco_int8>::cpp_op params const { \
        Sequence<type> result;                                  \
        MCO_CPP_CHECK(mco_seq_##c_op##_int8 args);              \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<type> Sequence<uint1>::cpp_op params const {\
        Sequence<type> result;                                  \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint1 args);             \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<type> Sequence<uint2>::cpp_op params const {\
        Sequence<type> result;                                  \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint2 args);             \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<type> Sequence<uint4>::cpp_op params const {\
        Sequence<type> result;                                  \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint4 args);             \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<type> Sequence<uint8>::cpp_op params const {\
        Sequence<type> result;                                  \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint8 args);             \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<type> Sequence<float>::cpp_op params const {\
        Sequence<type> result;                                  \
        MCO_CPP_CHECK(mco_seq_##c_op##_float args);             \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    inline Sequence<type> Sequence<double>::cpp_op params const {\
        Sequence<type> result;                                  \
        MCO_CPP_CHECK(mco_seq_##c_op##_double args);            \
        return result;                                          \
    }                                                           


#define MCO_SEQ_CAST(type)                                      \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<int1>::operator Sequence<mco_##type>() const { \
        Sequence<mco_##type> result;                            \
        MCO_CPP_CHECK(mco_seq_##type##_from_int1(result.iter, iter)); \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<int2>::operator Sequence<mco_##type>() const { \
        Sequence<mco_##type> result;                            \
        MCO_CPP_CHECK(mco_seq_##type##_from_int2(result.iter, iter)); \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<int4>::operator Sequence<mco_##type>() const { \
        Sequence<mco_##type> result;                            \
        MCO_CPP_CHECK(mco_seq_##type##_from_int4(result.iter, iter)); \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<mco_int8>::operator Sequence<mco_##type>() const {    \
        Sequence<mco_##type> result;                            \
        MCO_CPP_CHECK(mco_seq_##type##_from_int8(result.iter, iter)); \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<uint1>::operator Sequence<mco_##type>() const { \
        Sequence<mco_##type> result;                            \
        MCO_CPP_CHECK(mco_seq_##type##_from_uint1(result.iter, iter)); \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<uint2>::operator Sequence<mco_##type>() const { \
        Sequence<mco_##type> result;                            \
        MCO_CPP_CHECK(mco_seq_##type##_from_uint2(result.iter, iter)); \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<uint4>::operator Sequence<mco_##type>() const { \
        Sequence<mco_##type> result;                            \
        MCO_CPP_CHECK(mco_seq_##type##_from_uint4(result.iter, iter)); \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<uint8>::operator Sequence<mco_##type>() const { \
        Sequence<mco_##type> result;                            \
        MCO_CPP_CHECK(mco_seq_##type##_from_uint8(result.iter, iter)); \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<float>::operator Sequence<mco_##type>() const { \
        Sequence<mco_##type> result;                            \
        MCO_CPP_CHECK(mco_seq_##type##_from_float(result.iter, iter)); \
        return result;                                          \
    }                                                           \
    template<>                                                  \
    template<>                                                  \
    inline Sequence<double>::operator Sequence<mco_##type>() const { \
        Sequence<mco_##type> result;                            \
        MCO_CPP_CHECK(mco_seq_##type##_from_double(result.iter, iter)); \
        return result;                                          \
    }                                                           

#define MCO_SEQ_SCALAR_AGG(cpp_op, c_op, args, params)          \
    template<>                                                  \
    inline double Sequence<int1>::cpp_op params const {         \
        mco_seq_iterator_t result;                              \
        double agg;                                             \
        MCO_CPP_CHECK(mco_seq_##c_op##_int1 args);              \
        MCO_CPP_CHECK(mco_seq_next_double(&result, &agg));      \
        return agg;                                             \
    }                                                           \
    template<>                                                  \
    inline double Sequence<int2>::cpp_op params const {         \
        mco_seq_iterator_t result;                              \
        double agg;                                             \
        MCO_CPP_CHECK(mco_seq_##c_op##_int2 args);              \
        MCO_CPP_CHECK(mco_seq_next_double(&result, &agg));      \
        return agg;                                             \
    }                                                           \
    template<>                                                  \
    inline double Sequence<int4>::cpp_op params const {         \
        mco_seq_iterator_t result;                              \
        double agg;                                             \
        MCO_CPP_CHECK(mco_seq_##c_op##_int4 args);              \
        MCO_CPP_CHECK(mco_seq_next_double(&result, &agg));      \
        return agg;                                             \
    }                                                           \
    template<>                                                  \
    inline double Sequence<mco_int8>::cpp_op params const {     \
        mco_seq_iterator_t result;                              \
        double agg;                                             \
        MCO_CPP_CHECK(mco_seq_##c_op##_int8 args);              \
        MCO_CPP_CHECK(mco_seq_next_double(&result, &agg));      \
        return agg;                                             \
    }                                                           \
    template<>                                                  \
    inline double Sequence<uint1>::cpp_op params const {        \
        mco_seq_iterator_t result;                              \
        double agg;                                             \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint1 args);             \
        MCO_CPP_CHECK(mco_seq_next_double(&result, &agg));      \
        return agg;                                             \
    }                                                           \
    template<>                                                  \
    inline double Sequence<uint2>::cpp_op params const {        \
        mco_seq_iterator_t result;                              \
        double agg;                                             \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint2 args);             \
        MCO_CPP_CHECK(mco_seq_next_double(&result, &agg));      \
        return agg;                                             \
    }                                                           \
    template<>                                                  \
    inline double Sequence<uint4>::cpp_op params const {        \
        mco_seq_iterator_t result;                              \
        double agg;                                             \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint4 args);             \
        MCO_CPP_CHECK(mco_seq_next_double(&result, &agg));      \
        return agg;                                             \
    }                                                           \
    template<>                                                  \
    inline double Sequence<uint8>::cpp_op params const {        \
        mco_seq_iterator_t result;                              \
        double agg;                                             \
        MCO_CPP_CHECK(mco_seq_##c_op##_uint8 args);             \
        MCO_CPP_CHECK(mco_seq_next_double(&result, &agg));      \
        return agg;                                             \
    }                                                           \
    template<>                                                  \
    inline double Sequence<float>::cpp_op params const {        \
        mco_seq_iterator_t result;                              \
        double agg;                                             \
        MCO_CPP_CHECK(mco_seq_##c_op##_float args);             \
        MCO_CPP_CHECK(mco_seq_next_double(&result, &agg));      \
        return agg;                                             \
    }                                                           \
    template<>                                                  \
    inline double Sequence<double>::cpp_op params const {       \
        mco_seq_iterator_t result;                              \
        double agg;                                             \
        MCO_CPP_CHECK(mco_seq_##c_op##_double args);            \
        MCO_CPP_CHECK(mco_seq_next_double(&result, &agg));      \
        return agg;                                             \
    }                                                           \

MCO_SEQ_SCALAR_OP(operator +, add, (result.iter, iter, other.iter), (Sequence const& other))
MCO_SEQ_SCALAR_OP(operator -, sub, (result.iter, iter, other.iter), (Sequence const& other))
MCO_SEQ_SCALAR_OP(operator /, div, (result.iter, iter, other.iter), (Sequence const& other))
MCO_SEQ_SCALAR_OP(operator *, mul, (result.iter, iter, other.iter), (Sequence const& other))
MCO_SEQ_SCALAR_OP(operator %, mod, (result.iter, iter, other.iter), (Sequence const& other))
MCO_SEQ_SCALAR_OP(operator -, neg, (result.iter, iter), ())
MCO_SEQ_SCALAR_OP(abs, abs, (result.iter, iter), ())
MCO_SEQ_SCALAR_OP(diff, diff, (result.iter, iter), ())
MCO_SEQ_SCALAR_OP(max, max, (result.iter, iter, other.iter), (Sequence const& other))
MCO_SEQ_SCALAR_OP(min, min, (result.iter, iter, other.iter), (Sequence const& other))

MCO_SEQ_SCALAR_OP(reverse, reverse, (result.iter, iter), ())
MCO_SEQ_SCALAR_OP(permutate, order_by, (result.iter, iter, &permutation[0], permutation.size(), NULL), (std::vector<mco_seq_no_t> const& permutation))

MCO_SEQ_SCALAR_SOP(mco_seq_bool, operator ==, eq, (result.iter, iter, other.iter), (Sequence const& other))
MCO_SEQ_SCALAR_SOP(mco_seq_bool, operator !=, ne, (result.iter, iter, other.iter), (Sequence const& other))
MCO_SEQ_SCALAR_SOP(mco_seq_bool, operator >=, ge, (result.iter, iter, other.iter), (Sequence const& other))
MCO_SEQ_SCALAR_SOP(mco_seq_bool, operator <=, le, (result.iter, iter, other.iter), (Sequence const& other))
MCO_SEQ_SCALAR_SOP(mco_seq_bool, operator >, gt, (result.iter, iter, other.iter), (Sequence const& other))
MCO_SEQ_SCALAR_SOP(mco_seq_bool, operator <, lt, (result.iter, iter, other.iter), (Sequence const& other))

template<>
inline Sequence<mco_seq_bool> Sequence<mco_seq_bool>::operator ||(Sequence<mco_seq_bool> const& other) const {
    Sequence<mco_seq_bool> result;
    MCO_CPP_CHECK(mco_seq_or(result.iter, iter, other.iter));
    return result;
}

template<>
inline Sequence<mco_seq_bool> Sequence<mco_seq_bool>::operator &&(Sequence<mco_seq_bool> const& other) const {
    Sequence<mco_seq_bool> result;
    MCO_CPP_CHECK(mco_seq_and(result.iter, iter, other.iter));
    return result;
}

template<>
inline Sequence<mco_seq_bool> Sequence<mco_seq_bool>::operator ^(Sequence<mco_seq_bool> const& other) const {
    Sequence<mco_seq_bool> result;
    MCO_CPP_CHECK(mco_seq_xor(result.iter, iter, other.iter));
    return result;
}

template<>
inline Sequence<mco_seq_bool> Sequence<mco_seq_bool>::operator !() const {
    Sequence<mco_seq_bool> result;
    MCO_CPP_CHECK(mco_seq_not(result.iter, iter));
    return result;
}

#define MCO_SEQ_NEXT(type)                          \
template<>                                          \
inline bool Sequence<mco_##type>::next(mco_##type& val) {       \
    MCO_RET rc = mco_seq_next_##type(iter, &val);  \
    if (rc != MCO_S_CURSOR_END) {                   \
        MCO_CPP_CHECK(rc);                          \
    }                                               \
    return rc == MCO_S_OK;                          \
}
MCO_SEQ_NEXT(int1)
MCO_SEQ_NEXT(int2)
MCO_SEQ_NEXT(int4)
MCO_SEQ_NEXT(int8)
MCO_SEQ_NEXT(uint1)
MCO_SEQ_NEXT(uint2)
MCO_SEQ_NEXT(uint4)
MCO_SEQ_NEXT(uint8)
MCO_SEQ_NEXT(float)
MCO_SEQ_NEXT(double)

template<class T>
inline T Sequence<T>::operator++() {
    T val;
    if (!next(val)) { 
        throw McoException(MCO_S_CURSOR_EMPTY, "Sequence::next", __FILE__, __LINE__);
    }
    return val;
}

#define MCO_SEQ_CONST(type)                             \
template<>                                              \
inline Sequence<mco_##type>::Sequence(mco_##type const& val) {\
    MCO_CPP_CHECK(mco_seq_const_##type(iter, val));    \
}

MCO_SEQ_CONST(int1)
MCO_SEQ_CONST(int2)
MCO_SEQ_CONST(int4)
MCO_SEQ_CONST(int8)
MCO_SEQ_CONST(uint1)
MCO_SEQ_CONST(uint2)
MCO_SEQ_CONST(uint4)
MCO_SEQ_CONST(uint8)
MCO_SEQ_CONST(float)
MCO_SEQ_CONST(double)

#define MCO_SEQ_PARSE(type)                             \
template<>                                              \
inline Sequence<mco_##type>::Sequence(char const* str) {\
    MCO_CPP_CHECK(mco_seq_parse_##type(iter, str));     \
}

MCO_SEQ_PARSE(int1)
MCO_SEQ_PARSE(int2)
MCO_SEQ_PARSE(int4)
MCO_SEQ_PARSE(int8)
MCO_SEQ_PARSE(uint1)
MCO_SEQ_PARSE(uint2)
MCO_SEQ_PARSE(uint4)
MCO_SEQ_PARSE(uint8)
MCO_SEQ_PARSE(float)
MCO_SEQ_PARSE(double)

#define MCO_SEQ_UNGET(type)                             \
template<>                                              \
inline Sequence<mco_##type>::Sequence(std::vector<mco_##type> const& vals) {  \
    MCO_CPP_CHECK(mco_seq_unget_##type(iter, &vals[0], vals.size()));   \
}

MCO_SEQ_UNGET(int1)
MCO_SEQ_UNGET(int2)
MCO_SEQ_UNGET(int4)
MCO_SEQ_UNGET(int8)
MCO_SEQ_UNGET(uint1)
MCO_SEQ_UNGET(uint2)
MCO_SEQ_UNGET(uint4)
MCO_SEQ_UNGET(uint8)
MCO_SEQ_UNGET(float)
MCO_SEQ_UNGET(double)

#define MCO_SEQ_GET(type)                                               \
template<>                                                              \
inline void Sequence<mco_##type>::get(std::vector<mco_##type>& vals) {  \
    mco_size_t size;                                                    \
    if (vals.size() == 0) {                                             \
        vals.resize((mco_size_t)count());                               \
        reset();                                                        \
    }                                                                   \
    size = vals.size();                                                 \
    MCO_CPP_CHECK(mco_seq_get_##type(iter, &vals[0], &size)); \
    if (size != vals.size()) {                                          \
        vals.resize(size);                                              \
    }                                                                   \
}

MCO_SEQ_GET(int1)
MCO_SEQ_GET(int2)
MCO_SEQ_GET(int4)
MCO_SEQ_GET(int8)
MCO_SEQ_GET(uint1)
MCO_SEQ_GET(uint2)
MCO_SEQ_GET(uint4)
MCO_SEQ_GET(uint8)
MCO_SEQ_GET(float)
MCO_SEQ_GET(double)

MCO_SEQ_SCALAR_OP(apply, func, (result.iter, iter, func), (Sequence::unary_function_t func))
MCO_SEQ_SCALAR_OP(apply, func2, (result.iter, iter, other.iter, func), (Sequence const& other, Sequence::binary_function_t func))
MCO_SEQ_SCALAR_SOP(mco_seq_bool, check, cond, (result.iter, iter, func), (Sequence::predicate_t func))

MCO_SEQ_SCALAR_OP(map, map, (result.iter, iter, positions.iter), (Sequence<mco_seq_no_t> const& positions))
MCO_SEQ_SCALAR_OP(combine, union, (result.iter, iter, other.iter, order), (Sequence const& other, mco_seq_order_t order))

MCO_SEQ_SCALAR_OP(thin, thin, (result.iter, iter, origin, step), (mco_size_t origin, mco_size_t step))
MCO_SEQ_SCALAR_OP(filter, filter, (result.iter, condition.iter, iter), (Sequence<mco_seq_bool> const& condition))
MCO_SEQ_SCALAR_OP(unique, unique, (result.iter, iter), ())
MCO_SEQ_SCALAR_SOP(double, norm, norm, (result.iter, iter), ())


template<>
inline Sequence<mco_seq_no_t> Sequence<mco_seq_bool>::filterPos() const 
{
    Sequence<mco_seq_no_t> result;
    MCO_CPP_CHECK(mco_seq_filter_pos(result.iter, iter));
    return result;
}


#define MCO_SEQ_IIF(type, cpp_op, c_op)                                 \
template<>                                                              \
template<>                                                              \
inline Sequence<mco_##type> Sequence<mco_seq_bool>::cpp_op(Sequence<mco_##type> const& then, Sequence<mco_##type> const& otherwise) const { \
    Sequence<mco_##type> result;                                        \
    MCO_CPP_CHECK(mco_seq_##c_op##_##type(result.iter, iter, then.iter, otherwise.iter)); \
    return result;                                                      \
}

MCO_SEQ_IIF(int1, iif, iif)
MCO_SEQ_IIF(int2, iif, iif)
MCO_SEQ_IIF(int4, iif, iif)
MCO_SEQ_IIF(int8, iif, iif)
MCO_SEQ_IIF(uint1, iif, iif)
MCO_SEQ_IIF(uint2, iif, iif)
MCO_SEQ_IIF(uint4, iif, iif)
MCO_SEQ_IIF(uint8, iif, iif)
MCO_SEQ_IIF(float, iif, iif)
MCO_SEQ_IIF(double, iif, iif)


MCO_SEQ_IIF(int1, cond, if)
MCO_SEQ_IIF(int2, cond, if)
MCO_SEQ_IIF(int4, cond, if)
MCO_SEQ_IIF(int8, cond, if)
MCO_SEQ_IIF(uint1, cond, if)
MCO_SEQ_IIF(uint2, cond, if)
MCO_SEQ_IIF(uint4, cond, if)
MCO_SEQ_IIF(uint8, cond, if)
MCO_SEQ_IIF(float, cond, if)
MCO_SEQ_IIF(double, cond, if)

#define MCO_SEQ_AGG(agg_type, inp_type, cpp_op, c_op)                   \
template<>                                                              \
inline mco_##agg_type Sequence<mco_##inp_type>::agg##cpp_op() const {   \
    mco_seq_iterator_t result;                                          \
    mco_##agg_type agg;                                                 \
    MCO_CPP_CHECK(mco_seq_agg_##c_op##_##inp_type(&result, iter)); \
    MCO_CPP_CHECK(mco_seq_next_##agg_type(&result, &agg));              \
    return agg;                                                         \
}
MCO_SEQ_AGG(int1, int1, Max, max)
MCO_SEQ_AGG(int2, int2, Max, max)
MCO_SEQ_AGG(int4, int4, Max, max)
MCO_SEQ_AGG(int8, int8, Max, max)
MCO_SEQ_AGG(uint1, uint1, Max, max)
MCO_SEQ_AGG(uint2, uint2, Max, max)
MCO_SEQ_AGG(uint4, uint4, Max, max)
MCO_SEQ_AGG(uint8, uint8, Max, max)
MCO_SEQ_AGG(float, float, Max, max)
MCO_SEQ_AGG(double, double, Max, max)

MCO_SEQ_AGG(int1, int1, Min, min)
MCO_SEQ_AGG(int2, int2, Min, min)
MCO_SEQ_AGG(int4, int4, Min, min)
MCO_SEQ_AGG(int8, int8, Min, min)
MCO_SEQ_AGG(uint1, uint1, Min, min)
MCO_SEQ_AGG(uint2, uint2, Min, min)
MCO_SEQ_AGG(uint4, uint4, Min, min)
MCO_SEQ_AGG(uint8, uint8, Min, min)
MCO_SEQ_AGG(float, float, Min, min)
MCO_SEQ_AGG(double, double, Min, min)

#define MCO_SEQ_XAGG(agg_type, inp_type, cpp_op, c_op)                   \
template<>                                                              \
template<>                                                              \
inline mco_##agg_type Sequence<mco_##inp_type>::agg##cpp_op() const {   \
    mco_seq_iterator_t result;                                          \
    mco_##agg_type agg;                                                 \
    MCO_CPP_CHECK(mco_seq_agg_##c_op##_##inp_type(&result, iter)); \
    MCO_CPP_CHECK(mco_seq_next_##agg_type(&result, &agg));              \
    return agg;                                                         \
}

MCO_SEQ_XAGG(int8, int1, Sum, sum)
MCO_SEQ_XAGG(int8, int2, Sum, sum)
MCO_SEQ_XAGG(int8, int4, Sum, sum)
MCO_SEQ_XAGG(int8, int8, Sum, sum)
MCO_SEQ_XAGG(uint8, uint1, Sum, sum)
MCO_SEQ_XAGG(uint8, uint2, Sum, sum)
MCO_SEQ_XAGG(uint8, uint4, Sum, sum)
MCO_SEQ_XAGG(uint8, uint8, Sum, sum)
MCO_SEQ_XAGG(double, float, Sum, sum)
MCO_SEQ_XAGG(double, double, Sum, sum)

MCO_SEQ_XAGG(int8, int1, Prd, prd)
MCO_SEQ_XAGG(int8, int2, Prd, prd)
MCO_SEQ_XAGG(int8, int4, Prd, prd)
MCO_SEQ_XAGG(int8, int8, Prd, prd)
MCO_SEQ_XAGG(uint8, uint1, Prd, prd)
MCO_SEQ_XAGG(uint8, uint2, Prd, prd)
MCO_SEQ_XAGG(uint8, uint4, Prd, prd)
MCO_SEQ_XAGG(uint8, uint8, Prd, prd)
MCO_SEQ_XAGG(double, float, Prd, prd)
MCO_SEQ_XAGG(double, double, Prd, prd)

MCO_SEQ_SCALAR_AGG(aggAvg, agg_avg, (&result, iter), ())
MCO_SEQ_SCALAR_AGG(aggVar, agg_var, (&result, iter), ())
MCO_SEQ_SCALAR_AGG(aggDev, agg_dev, (&result, iter), ())
MCO_SEQ_SCALAR_AGG(aggVarSamp, agg_var_samp, (&result, iter), ())
MCO_SEQ_SCALAR_AGG(aggDevSamp, agg_dev_samp, (&result, iter), ())

MCO_SEQ_SCALAR_AGG(wsum, wsum, (&result, iter, weights.iter), (Sequence const& weights))
MCO_SEQ_SCALAR_AGG(wavg, wavg, (&result, iter, weights.iter), (Sequence const& weights))
MCO_SEQ_SCALAR_AGG(corr, corr, (&result, iter, other.iter), (Sequence const& other))
MCO_SEQ_SCALAR_AGG(cov, cov, (&result, iter, other.iter), (Sequence const& other))


MCO_SEQ_SCALAR_OP(groupAggMax, group_agg_max, (result.iter, iter, groupBy.iter), (GenericSequence const& groupBy))
MCO_SEQ_SCALAR_OP(groupAggMin, group_agg_min, (result.iter, iter, groupBy.iter), (GenericSequence const& groupBy))
MCO_SEQ_SCALAR_OP(groupAggFirst, group_agg_first, (result.iter, iter, groupBy.iter), (GenericSequence const& groupBy))
MCO_SEQ_SCALAR_OP(groupAggLast, group_agg_last, (result.iter, iter, groupBy.iter), (GenericSequence const& groupBy))
MCO_SEQ_SCALAR_XOP(groupAggSum, group_agg_sum, (result.iter, iter, groupBy.iter), (GenericSequence const& groupBy))
MCO_SEQ_SCALAR_SOP(double, groupAggWavg, group_agg_wavg, (result.iter, weight.iter, iter, groupBy.iter), (GenericSequence const& weight, GenericSequence const& groupBy))
MCO_SEQ_SCALAR_SOP(double, groupAggAvg, group_agg_avg, (result.iter, iter, groupBy.iter), (GenericSequence const& groupBy))
MCO_SEQ_SCALAR_SOP(double, groupAggVar, group_agg_var, (result.iter, iter, groupBy.iter), (GenericSequence const& groupBy))
MCO_SEQ_SCALAR_SOP(double, groupAggDev, group_agg_dev, (result.iter, iter, groupBy.iter), (GenericSequence const& groupBy))
MCO_SEQ_SCALAR_SOP(double, groupAggVarSamp, group_agg_var_samp, (result.iter, iter, groupBy.iter), (GenericSequence const& groupBy))
MCO_SEQ_SCALAR_SOP(double, groupAggDevSamp, group_agg_dev_samp, (result.iter, iter, groupBy.iter), (GenericSequence const& groupBy))

MCO_SEQ_SCALAR_OP(gridAggMax, grid_agg_max, (result.iter, iter, interval), (mco_size_t interval))
MCO_SEQ_SCALAR_OP(gridAggMin, grid_agg_min, (result.iter, iter, interval), (mco_size_t interval))
MCO_SEQ_SCALAR_XOP(gridAggSum, grid_agg_sum, (result.iter, iter, interval), (mco_size_t interval))
MCO_SEQ_SCALAR_SOP(double, gridAggAvg, grid_agg_avg, (result.iter, iter, interval), (mco_size_t interval))
MCO_SEQ_SCALAR_SOP(double, gridAggVar, grid_agg_var, (result.iter, iter, interval), (mco_size_t interval))
MCO_SEQ_SCALAR_SOP(double, gridAggDev, grid_agg_dev, (result.iter, iter, interval), (mco_size_t interval))
MCO_SEQ_SCALAR_SOP(double, gridAggVarSamp, grid_agg_var_samp, (result.iter, iter, interval), (mco_size_t interval))
MCO_SEQ_SCALAR_SOP(double, gridAggDevSamp, grid_agg_dev_samp, (result.iter, iter, interval), (mco_size_t interval))

MCO_SEQ_SCALAR_OP(windowAggMax, window_agg_max, (result.iter, iter, interval), (mco_size_t interval))
MCO_SEQ_SCALAR_OP(windowAggMin, window_agg_min, (result.iter, iter, interval), (mco_size_t interval))
MCO_SEQ_SCALAR_XOP(windowAggSum, window_agg_sum, (result.iter, iter, interval), (mco_size_t interval))
MCO_SEQ_SCALAR_SOP(double, windowAggAvg, window_agg_avg, (result.iter, iter, interval), (mco_size_t interval))
MCO_SEQ_SCALAR_SOP(double, windowAggVar, window_agg_var, (result.iter, iter, interval), (mco_size_t interval))
MCO_SEQ_SCALAR_SOP(double, windowAggDev, window_agg_dev, (result.iter, iter, interval), (mco_size_t interval))
MCO_SEQ_SCALAR_SOP(double, windowAggVarSamp, window_agg_var_samp, (result.iter, iter, interval), (mco_size_t interval))
MCO_SEQ_SCALAR_SOP(double, windowAggDevSamp, window_agg_dev_samp, (result.iter, iter, interval), (mco_size_t interval))


MCO_SEQ_VOID_OP(hashAggMax, hash_agg_max, (result.iter, groups.iter, iter, groupBy.iter, 0), (Sequence& result, GenericSequence& groups, GenericSequence const& groupBy))
MCO_SEQ_VOID_OP(hashAggMin, hash_agg_min, (result.iter, groups.iter, iter, groupBy.iter, 0), (Sequence& result, GenericSequence& groups, GenericSequence const& groupBy))
MCO_SEQ_VOID_OP(hashAggAvg, hash_agg_avg, (result.iter, groups.iter, iter, groupBy.iter, 0), (Sequence<double>& result, GenericSequence& groups, GenericSequence const& groupBy))


#define MCO_SEQ_HASH_AGG_SUM(res_type, type)                    \
    template<>                                                  \
    template<>                                                  \
    inline void Sequence<mco_##type>::hashAggSum(Sequence<mco_##res_type>& result, GenericSequence& groups, GenericSequence const& groupBy) const { \
        MCO_CPP_CHECK(mco_seq_hash_agg_sum_##type(result.iter, groups.iter, iter, groupBy.iter, 0)); \
    }

MCO_SEQ_HASH_AGG_SUM(int8, int1)
MCO_SEQ_HASH_AGG_SUM(int8, int2)
MCO_SEQ_HASH_AGG_SUM(int8, int4)
MCO_SEQ_HASH_AGG_SUM(int8, int8)
MCO_SEQ_HASH_AGG_SUM(uint8, uint1)
MCO_SEQ_HASH_AGG_SUM(uint8, uint2)
MCO_SEQ_HASH_AGG_SUM(uint8, uint4)
MCO_SEQ_HASH_AGG_SUM(uint8, uint8)
MCO_SEQ_HASH_AGG_SUM(double, float)
MCO_SEQ_HASH_AGG_SUM(double, double)


MCO_SEQ_SCALAR_OP(topMax, top_max, (result.iter, iter, top), (mco_size_t top))
MCO_SEQ_SCALAR_OP(topMin, top_min, (result.iter, iter, top), (mco_size_t top))
MCO_SEQ_SCALAR_SOP(mco_seq_no_t, topPosMax, top_pos_max, (result.iter, iter, top), (mco_size_t top))
MCO_SEQ_SCALAR_SOP(mco_seq_no_t, topPosMin, top_pos_min, (result.iter, iter, top), (mco_size_t top))

MCO_SEQ_SCALAR_OP(cumAggMax, cum_agg_max, (result.iter, iter), ())
MCO_SEQ_SCALAR_OP(cumAggMin, cum_agg_min, (result.iter, iter), ())
MCO_SEQ_SCALAR_XOP(cumAggSum, cum_agg_sum, (result.iter, iter), ())
MCO_SEQ_SCALAR_XOP(cumAggPrd, cum_agg_prd, (result.iter, iter), ())
MCO_SEQ_SCALAR_SOP(double, cumAggAvg, cum_agg_avg, (result.iter, iter), ())
MCO_SEQ_SCALAR_SOP(double, cumAggVar, cum_agg_var, (result.iter, iter), ())
MCO_SEQ_SCALAR_SOP(double, cumAggDev, cum_agg_dev, (result.iter, iter), ())
MCO_SEQ_SCALAR_SOP(double, cumAggVarSamp, cum_agg_var_samp, (result.iter, iter), ())
MCO_SEQ_SCALAR_SOP(double, cumAggDevSamp, cum_agg_dev_samp, (result.iter, iter), ())

#define MCO_SEQ_HISTOGRAM(type)                                         \
template<>                                                              \
inline Sequence<uint8> Sequence<mco_##type>::histogram(mco_##type minValue, mco_##type maxValue, mco_size_t nIntervals) const { \
    Sequence<uint8> result;                                             \
    MCO_CPP_CHECK(mco_seq_histogram_##type(result.iter, iter, minValue, maxValue, nIntervals)); \
    return result;                                                      \
}                                                                       



MCO_SEQ_HISTOGRAM(int1)
MCO_SEQ_HISTOGRAM(int2)
MCO_SEQ_HISTOGRAM(int4)
MCO_SEQ_HISTOGRAM(int8)
MCO_SEQ_HISTOGRAM(uint1)
MCO_SEQ_HISTOGRAM(uint2)
MCO_SEQ_HISTOGRAM(uint4)
MCO_SEQ_HISTOGRAM(uint8)
MCO_SEQ_HISTOGRAM(float)
MCO_SEQ_HISTOGRAM(double)

MCO_SEQ_SCALAR_SOP(mco_seq_no_t, cross, cross, (result.iter, iter, firstCrossDirection), (int firstCrossDirection))
MCO_SEQ_SCALAR_SOP(mco_seq_no_t, extrema, extrema, (result.iter, iter, firstExtremum), (int firstExtremum))

#define MCO_SEQ_SORT(type)                                              \
template<>                                                              \
inline void Sequence<mco_##type>::sort(std::vector<mco_##type> const& elements, std::vector<mco_seq_no_t>& permutation, mco_seq_order_t order){ \
    permutation.resize(elements.size());                                \
    MCO_CPP_CHECK(mco_seq_sort_##type(&elements[0], &permutation[0], elements.size(), order)); \
}                                                                       

MCO_SEQ_SORT(int1)
MCO_SEQ_SORT(int2)
MCO_SEQ_SORT(int4)
MCO_SEQ_SORT(int8)
MCO_SEQ_SORT(uint1)
MCO_SEQ_SORT(uint2)
MCO_SEQ_SORT(uint4)
MCO_SEQ_SORT(uint8)
MCO_SEQ_SORT(float)
MCO_SEQ_SORT(double)

MCO_SEQ_SCALAR_OP(repeat, repeat, (result.iter, iter, nTimes), (int nTimes))

template<class T>                                                              
inline Sequence<T> Sequence<T>::concat(Sequence<T> const& other) const {
    Sequence<T> result;
    MCO_CPP_CHECK(mco_seq_concat(result.iter, iter, other.iter)); 
    return result;
}                                                                       

template<class T>                                                              
inline Sequence<T> Sequence<T>::limit(mco_seq_no_t from, mco_seq_no_t till) const {
    Sequence<T> result;
    MCO_CPP_CHECK(mco_seq_limit(result.iter, iter, from, till)); 
    return result;
}                                                                       

template<class T>                                                              
inline Sequence<T> Sequence<T>::rleDecode() const {
    Sequence<T> result;
    MCO_CPP_CHECK(mco_seq_rle_decode(result.iter, iter)); 
    return result;
}                                                                       

MCO_SEQ_CAST(int1)
MCO_SEQ_CAST(int2)
MCO_SEQ_CAST(int4)
MCO_SEQ_CAST(int8)
MCO_SEQ_CAST(uint1)
MCO_SEQ_CAST(uint2)
MCO_SEQ_CAST(uint4)
MCO_SEQ_CAST(uint8)
MCO_SEQ_CAST(float)
MCO_SEQ_CAST(double)

#define MCO_SEQ_STRETCH(op, ts_type, val_type)                           \
template<>                                                              \
template<>                                                              \
inline Sequence<mco_##val_type> Sequence<mco_##val_type>::op(Sequence<mco_##ts_type> const& ts1, Sequence<mco_##ts_type> const& ts2, mco_##val_type filler) const { \
    Sequence<mco_##val_type> result;                                    \
    MCO_CPP_CHECK(mco_seq_##op##_##ts_type##_##val_type(result.iter, ts1.iter, ts2.iter, iter, filler)); \
    return result;                                                      \
}                                                                       

MCO_SEQ_STRETCH(stretch, uint4, int1)
MCO_SEQ_STRETCH(stretch, uint4, int2)
MCO_SEQ_STRETCH(stretch, uint4, int4)
MCO_SEQ_STRETCH(stretch, uint4, int8)
MCO_SEQ_STRETCH(stretch, uint4, uint1)
MCO_SEQ_STRETCH(stretch, uint4, uint2)
MCO_SEQ_STRETCH(stretch, uint4, uint4)
MCO_SEQ_STRETCH(stretch, uint4, uint8)
MCO_SEQ_STRETCH(stretch, uint4, float)
MCO_SEQ_STRETCH(stretch, uint4, double)
MCO_SEQ_STRETCH(stretch, uint8, int1)
MCO_SEQ_STRETCH(stretch, uint8, int2)
MCO_SEQ_STRETCH(stretch, uint8, int4)
MCO_SEQ_STRETCH(stretch, uint8, int8)
MCO_SEQ_STRETCH(stretch, uint8, uint1)
MCO_SEQ_STRETCH(stretch, uint8, uint2)
MCO_SEQ_STRETCH(stretch, uint8, uint4)
MCO_SEQ_STRETCH(stretch, uint8, uint8)
MCO_SEQ_STRETCH(stretch, uint8, float)
MCO_SEQ_STRETCH(stretch, uint8, double)

MCO_SEQ_STRETCH(stretch0, uint4, int1)
MCO_SEQ_STRETCH(stretch0, uint4, int2)
MCO_SEQ_STRETCH(stretch0, uint4, int4)
MCO_SEQ_STRETCH(stretch0, uint4, int8)
MCO_SEQ_STRETCH(stretch0, uint4, uint1)
MCO_SEQ_STRETCH(stretch0, uint4, uint2)
MCO_SEQ_STRETCH(stretch0, uint4, uint4)
MCO_SEQ_STRETCH(stretch0, uint4, uint8)
MCO_SEQ_STRETCH(stretch0, uint4, float)
MCO_SEQ_STRETCH(stretch0, uint4, double)
MCO_SEQ_STRETCH(stretch0, uint8, int1)
MCO_SEQ_STRETCH(stretch0, uint8, int2)
MCO_SEQ_STRETCH(stretch0, uint8, int4)
MCO_SEQ_STRETCH(stretch0, uint8, int8)
MCO_SEQ_STRETCH(stretch0, uint8, uint1)
MCO_SEQ_STRETCH(stretch0, uint8, uint2)
MCO_SEQ_STRETCH(stretch0, uint8, uint4)
MCO_SEQ_STRETCH(stretch0, uint8, uint8)
MCO_SEQ_STRETCH(stretch0, uint8, float)
MCO_SEQ_STRETCH(stretch0, uint8, double)

#define MCO_SEQ_ASOF_JOIN(ts_type, val_type)                             \
    template<>                                                          \
template<>                                                              \
inline Sequence<mco_##val_type> Sequence<mco_##val_type>::asofJoin(Sequence<mco_##ts_type> const& ts1, Sequence<mco_##ts_type> const& ts2) const { \
    Sequence<mco_##val_type> result;                                    \
    MCO_CPP_CHECK(mco_seq_asof_join_##ts_type##_##val_type(result.iter, ts1.iter, ts2.iter, iter)); \
    return result;                                                      \
}                                                                       \

MCO_SEQ_ASOF_JOIN(uint4, int1)
MCO_SEQ_ASOF_JOIN(uint4, int2)
MCO_SEQ_ASOF_JOIN(uint4, int4)
MCO_SEQ_ASOF_JOIN(uint4, int8)
MCO_SEQ_ASOF_JOIN(uint4, uint1)
MCO_SEQ_ASOF_JOIN(uint4, uint2)
MCO_SEQ_ASOF_JOIN(uint4, uint4)
MCO_SEQ_ASOF_JOIN(uint4, uint8)
MCO_SEQ_ASOF_JOIN(uint4, float)
MCO_SEQ_ASOF_JOIN(uint4, double)
MCO_SEQ_ASOF_JOIN(uint8, int1)
MCO_SEQ_ASOF_JOIN(uint8, int2)
MCO_SEQ_ASOF_JOIN(uint8, int4)
MCO_SEQ_ASOF_JOIN(uint8, int8)
MCO_SEQ_ASOF_JOIN(uint8, uint1)
MCO_SEQ_ASOF_JOIN(uint8, uint2)
MCO_SEQ_ASOF_JOIN(uint8, uint4)
MCO_SEQ_ASOF_JOIN(uint8, uint8)
MCO_SEQ_ASOF_JOIN(uint8, float)
MCO_SEQ_ASOF_JOIN(uint8, double)


template<>
inline uint4 Sequence<uint4>::aggApproxDistinctHashValues() const {
    mco_seq_iterator_t result;
    uint4 count;
    MCO_CPP_CHECK(mco_seq_agg_approxdc_hash(&result, iter)); 
    MCO_CPP_CHECK(mco_seq_next_uint4(&result, &count)); 
    return count;
}

template<>
inline Sequence<uint4> Sequence<uint4>::groupAggApproxDistinctHashValues(GenericSequence const& groupBy) const {
    Sequence<uint4> result;
    MCO_CPP_CHECK(mco_seq_group_agg_approxdc_hash(result.iter, iter, groupBy.iter)); 
    return result;
}



template<unsigned n>
class Sequence< Char<n> > : public GenericSequence
{
  public:
    typedef Char<n> Chars;

    bool next(Chars& val) {
        assert(n == elemSize());
        MCO_RET rc = mco_seq_next_char(iter, val.body);
        if (rc != MCO_S_CURSOR_END) { 
            MCO_CPP_CHECK(rc);
        }
        return rc == MCO_S_OK;
    }

    Chars operator++() {
        Chars val;
        if (!next(val)) { 
            throw McoException(MCO_S_CURSOR_EMPTY, "Sequence::next", __FILE__, __LINE__);
        }
        return val;
    }

    Sequence<Chars> filter(Sequence<mco_seq_bool> const& condition) const { 
        Sequence<Chars> result;                                          
        MCO_CPP_CHECK(mco_seq_filter_char(result.iter, iter, condition.iter));
        return result;
    }   

    Sequence<Chars> thin(mco_size_t origin, mco_size_t step) const { 
        Sequence<Chars> result;                                          
        MCO_CPP_CHECK(mco_seq_thin_char(result.iter, iter, origin, step));
        return result;
    }   

    Sequence<Chars> repeat(int nTimes) const { 
        Sequence<Chars> result;                                          
        MCO_CPP_CHECK(mco_seq_repeat_char(result.iter, iter, nTimes));
        return result;
    }   

    Sequence<Chars> map(Sequence<mco_seq_no_t> const& positions) const { 
        Sequence<Chars> result;                                          
        MCO_CPP_CHECK(mco_seq_map_char(result.iter, iter, positions.iter));
        return result;
    }   
    
    Sequence<Chars> concat(Sequence<Chars> const& other) const {
        Sequence<Chars> result;
        MCO_CPP_CHECK(mco_seq_concat(result.iter, iter, other.iter)); 
        return result;
    }                                                                       

    Sequence<Chars> limit(mco_seq_no_t from, mco_seq_no_t till) const {
        Sequence<Chars> result;
        MCO_CPP_CHECK(mco_seq_limit(result.iter, iter, from, till)); 
        return result;
    }        

    Sequence<Chars> operator[](Sequence<mco_seq_no_t> const& positions) const { 
        return map(positions);
    }
    
    Sequence<Chars> operator()(mco_seq_no_t from, mco_seq_no_t till) const { 
        return limit(from, till);
    }

    Sequence<Chars> operator,(Sequence<Chars> const& other) const { 
        return concat(other);
    }


    Sequence<Chars> reverse() const { 
        Sequence<Chars> result;                                          
        MCO_CPP_CHECK(mco_seq_reverse_char(result.iter, iter));
        return result;
    }   
    
    Sequence<mco_seq_bool> operator == (Sequence<Chars> const& other) const {
        Sequence<mco_seq_bool> result;                                          
        MCO_CPP_CHECK(mco_seq_eq_char(result.iter, iter, other.iter));
        return result;
    }   
    
    Sequence<mco_seq_bool> operator != (Sequence<Chars> const& other) const {
        Sequence<mco_seq_bool> result;                                          
        MCO_CPP_CHECK(mco_seq_ne_char(result.iter, iter, other.iter));
        return result;
    }   
    
    Sequence<mco_seq_bool> operator > (Sequence<Chars> const& other) const {
        Sequence<mco_seq_bool> result;                                          
        MCO_CPP_CHECK(mco_seq_gt_char(result.iter, iter, other.iter));
        return result;
    }   
    
    Sequence<mco_seq_bool> operator >= (Sequence<Chars> const& other) const {
        Sequence<mco_seq_bool> result;                                          
        MCO_CPP_CHECK(mco_seq_ge_char(result.iter, iter, other.iter));
        return result;
    }   
    
    Sequence<mco_seq_bool> operator < (Sequence<Chars> const& other) const {
        Sequence<mco_seq_bool> result;                                          
        MCO_CPP_CHECK(mco_seq_lt_char(result.iter, iter, other.iter));
        return result;
    }   
     
    Sequence<mco_seq_bool> operator <= (Sequence<Chars> const& other) const {
        Sequence<mco_seq_bool> result;                                          
        MCO_CPP_CHECK(mco_seq_le_char(result.iter, iter, other.iter));
        return result;
    }   

    Sequence<mco_seq_bool> match(char const* pattern) const {
        Sequence<mco_seq_bool> result;                                          
        MCO_CPP_CHECK(mco_seq_match(result.iter, iter, pattern));
        return result;
    }   
    
    void permutate(Sequence<Chars>& result, std::vector<mco_seq_no_t> const& permutation) const {
        MCO_CPP_CHECK(mco_seq_order_by_char(result.iter, iter, &permutation[0], permutation.size(), 0));
    }   
    
    void get(std::vector<Chars>& vals) {                 
        mco_size_t size;                                                        
        if (vals.size() == 0) {                                             
            vals.resize((mco_size_t)count());                                   
            reset();
        }                                                                   
        size = vals.size();                                                 
        MCO_CPP_CHECK(mco_seq_get_char(&vals[0], &size));                 
        if (size != vals.size()) {                                          
            vals.resize(size);                                             
        }                                                                   
    }

    Sequence<Chars> rleDecode() const {
        Sequence<Chars> result;
        MCO_CPP_CHECK(mco_seq_rle_decode(result.iter, iter)); 
        return result;
    }                                                                       

    Sequence(Sequence const& other) : GenericSequence(other.iter) { ((Sequence&)other).iter = NULL; } // grab iterator

    Sequence& operator= (Sequence const& other) { 
        delete iter;
        iter = other.iter;
        ((Sequence&)other).iter = NULL;  // grab iterator
    }

    Sequence(Chars const& val) { 
        MCO_CPP_CHECK(mco_seq_const_char(iter, val.body, n));
    }
    
    Sequence(std::vector<Chars> const& vals) {     
        MCO_CPP_CHECK(mco_seq_unget_char(iter, &vals[0], vals.size(), n)); 
    }

    Sequence() {}
};

#endif
