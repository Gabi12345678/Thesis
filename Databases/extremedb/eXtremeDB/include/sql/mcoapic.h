/*******************************************************************
 *                                                                 *
 *  mcoapic.h                                                      *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           * 
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/
#ifndef __MCOAPIC_H__
    #define __MCOAPIC_H__

    #include "sqlc.h"
    #include "mco.h"

    #ifdef __cplusplus
        extern "C"
        {
        #endif 

        /**
         * Initialize eXtremeDB interaction module
         * This function imports database scheme 
         * and makes it possible to call <code>mcosql_open</code> function.
         * It is assumed that no more than one database can be opened by application at one time.
         * @param db handle of eXtremeDB database
         * @return status of operation as described in error_code enum
         */
		MCOSQL_API status_t mcoapi_initialize(mco_db_h db);

        /**
         * Creates a new eXtremeSQL session with the eXtremeDB runtime.
         * This function is used in conjunction with the <code>mcosql_open</code>
         * function.
         * This function should be used instead of the mcoapi_initialize() when
         * the application interacts with more than one database at a time
         * @param database handle to the eXtremeDB database
         * @param storage placeholder to receive the address of the created session
         * @return status of operation as described in the error_code enum
         */
		MCOSQL_API status_t mcoapi_create(mco_db_h db, storage_t* storage);

        /**
         * Destroys the eXtremeSQL session with eXtremeDB runtime created by the
         * mcoapi_create function
         * @return the status of the operation as described in the error_code enum
         */
		MCOSQL_API status_t mcoapi_destroy(storage_t storage);

        /**
         * Creates a new eXtremeSQL engine
         * @param database handle to the eXtremeDB database
         * @param database placeholder to receive the address of the created session
         * @return status of operation as described in the error_code enum
         */
		MCOSQL_API status_t mcoapi_create_engine(mco_db_h db, database_t* database);

        /**
         * Destroys the eXtremeSQL engine created by mcoapi_create_engine function
         * @return the status of the operation as described in the error_code enum
         */
		MCOSQL_API status_t mcoapi_destroy_engine(database_t storage);


        #ifdef __cplusplus
        }
    #endif 

#endif
