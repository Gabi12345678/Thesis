/****************************************************************
 *                                                              *
 *  mco.hpp                                                     *
 *                                                              *
 *  This file is a part of the eXtremeDB source code.           *
 *                                                              *
 *  Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.  *
 *                                                              *
 *  New C++ API                                                 *
 *                                                              *
 ****************************************************************/

#ifndef MCO_HPP__
#define MCO_HPP__

#include <assert.h>
#include <string>
#include <vector>

#include "mco.h"

class McoException : public std::exception
{
    MCO_RET rc;
    char const* call;
    char const* file;
    int line;
  public:
    McoException(MCO_RET rc, char const* call, char const* file, int line) {
        this->rc = rc;
        this->call = call;
        this->file = file;
        this->line = line;
    }

    const char *what() const throw() {
        static char msgBuf[1024];
        sprintf(msgBuf, "%s:%d: %s returns %d\n", file, line, call, rc);
        return msgBuf;
    }

    MCO_RET get_rc() const throw() { return rc; }
};

#define MCO_CPP_CHECK(call) do { MCO_RET rc_ = (call); if (rc_ != MCO_S_OK) throw McoException(rc_, #call, __FILE__, __LINE__); } while (0)


template<unsigned n>
struct Char
{
    char body[n];

    char& operator[](int i) {
        return body[i];
    }
    char operator[](int i) const {
        return body[i];
    }
    int size() const {
        return n;
    }

    operator char*() {
        return body;
    }

#if defined(__IBMC__)
    Char() { *body = '\0'; }

    Char(const char* str) {
          strncpy(body, str, n);
    }

    void operator=(Char<n> const& other) {
          memcpy(body, other.body, n);
    }
#endif
};


/**
 * Scoped eXtremeDB transaction.
 * Intended usage:
 * {
 *     McoTrans trans(con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND);
 *     try {
 *        ... // do some work
 *     } catch (...) {
 *         trans.rollback();
 *         throw;
 *     }
 * }
 */
class McoTrans
{
    mco_trans_h trans;

  public:
    /**
     * Start transaction
     */
    McoTrans(mco_db_h db, MCO_TRANS_TYPE type, MCO_TRANS_PRIORITY pri) {
        MCO_CPP_CHECK(mco_trans_start(db, type, pri, &trans));
    }

    /**
     * Commit transaction on normal exit (if transaction was not explicitly aborted before)
     */
    ~McoTrans() throw (McoException) {
        if (trans) {
            MCO_CPP_CHECK(mco_trans_commit(trans));
        }
    }

    /**
     * Get transaction handle
     */
    operator mco_trans_h() {
        return trans;
    }

    /**
     * Get transaction handle
     */
    mco_trans_h operator*() {
        return trans;
    }

    /**
     * Rollback transaction
     */
    void rollback() {
        mco_trans_rollback(trans);
        trans = 0;
    }
};

#define FIXCHAR(type) Char<sizeof(type)>

#endif
