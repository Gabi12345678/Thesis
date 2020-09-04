/*******************************************************************
 *                                                                 *
 *  tlogiterpipe_ddl.cpp                                           *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           *
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/
#include <common.h>
#include "mcosmp.h"
#include "mcosql.h"
#include "mcotime.h"
#include "mcolog.h"

using namespace McoSql;

#define PIPE_SEGMENT_SIZE       1024
#define BUFFER_SEGMENT_SIZE     1024*1024
#define DDL_DICT_SIZE           64*1024
#define MAX_CLASSES             100
#define MAX_INDEXES             1000

const char *db_name = "iterpipeddl";

typedef struct {
    int                     dict_version;
    mco_metadict_header_t   *metadict;
    mco_trans_h             trn;
    mco_bool                trn_started;
} db_context_t;

void print_object( mco_metadict_header_t *metadict, MCO_Hf* obj, int cid )
{
	unsigned short tableCount = 0;
	mco_dict_struct_info_t struct_info;
	mco_dict_field_info_t field_info;
	mco_uda_object_handle_t udahandle;
	int i, j;
	char str[128];
	mco_uda_value_t value; 

	printf( "UDA: " );
	
	CHECK(mco_uda_from_mco( obj, &udahandle ));
	
	CHECK(mco_dict_struct_count( metadict, 0, &tableCount ));

	for ( i = 0; i < tableCount; i++ )
	{
		mco_dict_struct( metadict, 0, i, &struct_info );
		if ( struct_info.flags & MCO_DICT_SI_CLASS && struct_info.class_code == cid)
		{
            printf( "table '%s', fields ", struct_info.name );
			for ( j =0; j < struct_info.field_count; j++ )
			{
				CHECK(mco_dict_field( metadict, 0, i, j, &field_info ));
				switch( field_info.type )
				{
					case MCO_DD_AUTOOID:
						value.type = MCO_DD_AUTOOID;
						mco_uda_get( &udahandle, field_info.field_no, 0, &value );
						printf( "%s = %d\t", field_info.name, value.v.i4 );
						break;
					case MCO_DD_INT4:
						value.type = MCO_DD_INT4;
						mco_uda_get( &udahandle, field_info.field_no, 0, &value );
						printf( "%s = %d\t", field_info.name, value.v.i4 );
						break;
					case MCO_DD_CHAR:
						value.type = MCO_DD_CHAR;
						value.v.p.p.c = str;
						value.v.p.size = sizeof(str);
						mco_uda_get( &udahandle, field_info.field_no, 0, &value );
						printf( "%s = %s\t", field_info.name, value.v.p.p.c );
						break;
					case MCO_DD_STRING:
						value.type = MCO_DD_STRING;
						value.v.p.p.c = str;
						value.v.p.size = sizeof(str);
						mco_uda_get( &udahandle, field_info.field_no, 0, &value );
						printf( "%s = %s\t", field_info.name, value.v.p.p.c );
						break;
					//to add other types
					default:
						break;
				}
			}
		}
	}
	printf("\n");
}

// This function is called by mco_translog_iterate as a callback, to process transaction data passed through a pipe
MCO_RET iteration_proc( mco_trans_h trans, MCO_Hf* obj, int cid, int obj_state, void* user_ctx )
{
    MCO_RET rc;
    db_context_t *dbctx = (db_context_t *)user_ctx;

    int dict_version = 0;
    CHECK(mco_dict_version( (mco_db_h) trans, &dict_version ))
    if ( dict_version != dbctx->dict_version ) {
        if ( dbctx->dict_version == 0 ) {
            // Register meta-dictionary initially to be able use UDA dictionary querying    
            CHECK(mco_metadict_register_conn( dbctx->metadict, (mco_db_h) trans, 0 ));
        }
        dbctx->dict_version = dict_version;
        printf( "Schema changed\n" );
    }

    if ( trans == 0 )
    {
        printf ("Main DB was cleaned by function mco_db_clean\n");
        return MCO_S_OK;
    }

    if ( obj == 0 )
    {
        if ( dbctx->trn_started == MCO_NO )
            return MCO_S_OK;
        dbctx->trn_started = MCO_NO;
        printf( "Transaction has been finished.\n" );
        return MCO_S_OK;
    }
    else if ( dbctx->trn_started == MCO_NO )
    {
        printf( "New transaction has been started.\n" );
        dbctx->trn_started = MCO_YES;
    }

    /* Check the kind of object being processed */
    if ( obj_state == MCO_TRANS_OBJ_ALL_DELETED )
    {
        printf( "Delete all of objects of class %d\n", cid );
    }
    else if ( obj_state == MCO_TRANS_OBJ_DELETED )
    {
        printf( "Delete single object of class %d\n", cid );
        print_object(dbctx->metadict, obj, cid);
    }
    else if ( obj_state == MCO_TRANS_OBJ_CREATED )
    {
        printf( "Create an object of class %d\n", cid );
        print_object(dbctx->metadict, obj, cid);
    }
    else
    {
        printf( "Update an object of class %d\n", cid );
        print_object(dbctx->metadict, obj, cid);
    }

    return MCO_S_OK;
}

// This thread iterates the transaction
void IteratorThread( sample_task_t * descriptor )
{
    MCO_RET rc;
    void *buf_ptr;
    mco_db_h db;

    buf_ptr = malloc( BUFFER_SEGMENT_SIZE );

    // Connect to main database for mco_translog_iterate internal purposes
    CHECK(mco_db_connect( db_name, &db ));

    db_context_t dbctx;
    dbctx.dict_version = 0;
    dbctx.trn = 0;
    dbctx.trn_started = MCO_NO;

    unsigned int metadict_size = 0;
    mco_metadict_size( 1, &metadict_size );
    dbctx.metadict = (mco_metadict_header_t *) (malloc( metadict_size ));
    CHECK(mco_metadict_init( dbctx.metadict, metadict_size,  MCO_METADICT_DONT_LOAD_EXISTING_DBS ));

    mco_TL_play_params_t play_params;
    mco_translog_play_params_init( &play_params );
    play_params.pipe_db           = db;
    play_params.iteration_proc    = iteration_proc;
    play_params.iterproc_user_ctx = (void *)&dbctx;
    play_params.mem_ptr           = buf_ptr;
    play_params.mem_size          = BUFFER_SEGMENT_SIZE;
    play_params.ddl_dict_size     = DDL_DICT_SIZE;
    play_params.max_classes       = MAX_CLASSES;
    play_params.max_indexes       = MAX_INDEXES;

    // Main iteration call
restart_reading:
    rc = mco_translog_play_ex( &play_params );

    /* Wait for main thread until it has started transaction logging (create pipe) */
    if ( rc == MCO_E_TL_LOG_NOT_OPENED )
    {
        sample_sleep( 100 );
        goto restart_reading;
    }
    CHECK(rc);
  
    // Disconnect database
    CHECK(mco_db_disconnect( db ));

    free( dbctx.metadict );
    free( buf_ptr );
}

int main(int argc, char *argv[])
{
    McoSqlEngine engine;

    sample_os_initialize(DEFAULT);
    mco_translog_init();

    /* This option causes an additional call of iteration_proc() at the end of each transaction
       with a null object handle to indicate 'end of transaction'. This enables iteration_proc()
       to detect the last object accessed in the current transaction.
    */
    mco_runtime_setoption( MCO_RT_OPTION_MARK_LAST_OBJ, 1 );
    // This option allows to add pipe memory device after initialization of the DB
    mco_runtime_setoption( MCO_RT_MAX_DYNAMIC_PIPES, 1 );
    
    // Open DB and try to load a previously saved DB snapshot
    engine.open( db_name, 0 /* no static dictionary */, 16 * 1024 * 1024, 256,
                    0, 0, McoSqlOpenParameters::DEFAULT_OPEN_FLAGS, 0, MAX_CLASSES, MAX_INDEXES, DDL_DICT_SIZE );

    // Create additional pipe device
    mco_device_t pipe_dev;
    pipe_dev.type       = MCO_MEMORY_CONV;
    pipe_dev.assignment = MCO_MEMORY_ASSIGN_PIPE_BUF;
    pipe_dev.dev.conv.ptr = malloc (PIPE_SEGMENT_SIZE);
    pipe_dev.size       = PIPE_SEGMENT_SIZE;
    CHECK(mco_db_extend_dev( db_name, &pipe_dev ));

    // Start the iterator thread
    sample_task_t iter_task;
    sample_start_task( &iter_task, IteratorThread, 0 );

    // Set up and run transaction logging (pipe is created here)
    mco_TL_start_data_t tl_parms;
    tl_parms.flags = MCO_TRANSLOG_ITERABLE | MCO_TRANSLOG_PIPE;
    tl_parms.disk_page_size = PSTORAGE_PAGE_SIZE;

    CHECK(mco_translog_start( engine.getHandle(), 0, &tl_parms ));

    // Wait while iterator thread is alive
    while(1) {
        mco_TL_current_info_t tl_info;

        CHECK(mco_translog_get_info( engine.getHandle(), &tl_info ));
        if ( tl_info.pipe_readers_connected )
            break;
        else
            sample_sleep( 100 );
    }

    // Create table a
    engine.executeStatement( "create table a (id int primary key, value string)" );

    // Insert records to table a
    engine.executeStatement( "start transaction" );
    for ( int i = 0; i < 5; i++ )
    {
        char buff[16];
        sprintf( buff, "val_%d", i );
        engine.executeStatement( "insert into a (id, value) values (%i, %s)", i, buff );
    }
    engine.executeStatement( "commit transaction" );

    // Execute DDL operation
    engine.executeStatement( "alter table a rename to b" );

    // Insert more records to table a with new schema
    engine.executeStatement( "start transaction" );
    for ( int i = 5; i < 10; i++ )
    {
        char buff[16];
        sprintf( buff, "val_%d", i );
        engine.executeStatement( "insert into b (id, value) values (%i, %s)", i, buff );
    }
    engine.executeStatement( "commit transaction" );

    // Execute DDL operation
    engine.executeStatement( "create table c (id int primary key, value string)" );

    // Insert records to table b
    engine.executeStatement( "start transaction" );
    for ( int i = 0; i < 5; i++ )
    {
        char buff[16];
        sprintf( buff, "val_%d", i );
        engine.executeStatement( "insert into c (id, value) values (%i, %s)", i, buff );
    }
    engine.executeStatement( "commit transaction" );

    // Execute DDL operation
    engine.executeStatement( "alter table b rename column value to val" );

    // Update record in a just altered  table
    engine.executeStatement( "update b set val='magic' where id = 3");

    // Close the database
    CHECK(mco_translog_stop( engine.getHandle() ));
    sample_join_task( &iter_task );

    engine.close();

    free( pipe_dev.dev.conv.ptr );
    sample_os_shutdown();

    return 0;
}
