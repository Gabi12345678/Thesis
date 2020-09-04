/*****************************************************************
 *                                                               *
 * Copyright (c) 2001-2019 McObject LLC. All Right Reserved.     *
 *                                                               *
 *****************************************************************/

/* This sample demonstrates asynchronous and synchronous database events
 * handling using UDA interface.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <common.h>
#include <mcouda.h>
#include "udaevent.h"

#ifndef INT8_FORMAT
#ifndef _WIN32
    #define INT8_FORMAT "ll"
#else
    #define INT8_FORMAT "I64"
#endif
#endif


char sample_descr[] = {
  "Sample 'udaevent' demonstrates asynchronous and synchronous database event \nhandling using the UDA interface.\n"
};
const char* db_name = "udaevent";
mco_metadict_header_t *metadict;
const int nRecords = 10;

void init_metadict()
{
    unsigned int size;
    mco_metadict_size(1, &size);
    metadict = (mco_metadict_header_t *) malloc(size);
    mco_metadict_init (metadict, size, 0);
}

void print_obj_field(mco_uda_object_handle_t *obj, mco_dict_field_info_t *field_info)
{
    mco_uda_value_t val;
    char buf[20];
    
    val.type = field_info->type;
    if (val.type == MCO_DD_CHAR || val.type == MCO_DD_STRING) {
        val.v.p.size = sizeof(buf);
        val.v.p.p.c = buf;
    }
    mco_uda_get(obj, field_info->field_no, 0, &val);
    switch (val.type) {
        case MCO_DD_UINT1: printf("%u", val.v.u1); break;
        case MCO_DD_UINT2: printf("%u", val.v.u2); break;
        case MCO_DD_DATE:  
        case MCO_DD_TIME:
        case MCO_DD_UINT4: printf("%u", val.v.u4); break;
        case MCO_DD_UINT8: printf("%" INT8_FORMAT "u", val.v.u8); break;
        case MCO_DD_INT1:  printf("%d", val.v.i1); break;
        case MCO_DD_INT2:  printf("%d", val.v.i2); break;
        case MCO_DD_INT4:  printf("%d", val.v.u4); break;
        case MCO_DD_INT8:  printf("%" INT8_FORMAT "d", val.v.i8); break;
        case MCO_DD_FLOAT: printf("%f", val.v.f); break;
        case MCO_DD_DOUBLE: printf("%f", val.v.d); break;
        case MCO_DD_CHAR:
        case MCO_DD_STRING: printf(buf); break;
        default: printf("UNKNOWN");
    }
}

typedef struct ThrParam_
{
    sample_task_t task;
    unsigned short struct_no;
    unsigned short event_no;
    int event_count;
} ThrParam;

ThrParam all_thr_param[100];


void async_event_handler( sample_task_t * descriptor ) 
{
    ThrParam* thr_p = (ThrParam*) descriptor->param;
    mco_dict_struct_info_t struct_info;
    mco_dict_field_info_t field_info;
    mco_dict_event_info_t event_info;
    mco_db_h db = descriptor->db_connection;
    char *event_type;
    MCO_RET rc;

    thr_p->event_count = 0;
    
    mco_dict_struct(metadict, 0, thr_p->struct_no, &struct_info);
    mco_dict_event(metadict, 0, thr_p->struct_no, thr_p->event_no, &event_info);
    if (event_info.type == MCO_EVENT_UPDATE) {
        mco_dict_field(metadict, 0, thr_p->struct_no, event_info.field_no, &field_info);
    }
    switch (event_info.type) {
        case MCO_EVENT_UPDATE       : event_type = "UPDATE"; break;
        case MCO_EVENT_NEW          : event_type = "NEW"; break;
        case MCO_EVENT_DELETE       : event_type = "DELETE"; break;
        case MCO_EVENT_DELETE_ALL   : event_type = "DELETE_ALL"; break;
        case MCO_EVENT_CHECKPOINT   : event_type = "CHECKPOINT"; break;
        case MCO_EVENT_CLASS_UPDATE : event_type = "CLASSUPDATE"; break;
    }

    for (;;)
    {
        rc = mco_uda_async_event_wait(db, thr_p->struct_no, thr_p->event_no);

        printf("Async. event : class %s, event_no=%d, type=%s ", struct_info.name, thr_p->event_no, event_type);
        if (event_info.type == MCO_EVENT_UPDATE) {
            printf("field=%s ", field_info.name);
        }    
        if (rc != MCO_S_OK) {
            printf(" exiting. Num. of events=%d\n", thr_p->event_count);
            return ;
        }
        ++thr_p->event_count;
        printf("\n");
    }
}

void start_async_handlers()
{
    unsigned short struct_count, s, e;
    mco_dict_struct_info_t struct_info;
    unsigned short thr_num = 0;
    
    mco_dict_struct_count(metadict, 0, &struct_count);
    for (s = 0; s < struct_count; ++s) {
        mco_dict_struct(metadict, 0, s, &struct_info);
        for (e = 0; e < struct_info.event_count; ++e) {
            all_thr_param[thr_num].struct_no = s;
            all_thr_param[thr_num].event_no  = e;
            sample_start_connected_task(&all_thr_param[thr_num].task, async_event_handler, db_name, &all_thr_param[thr_num]);
            ++thr_num;
            sample_sleep(100);
        }
    }
}

void stop_async_handlers(mco_db_h db)
{
    unsigned short struct_count, s, e;
    mco_dict_struct_info_t struct_info;
    unsigned short thr_num = 0;
    
    mco_uda_async_event_release_all(db);

    mco_dict_struct_count(metadict, 0, &struct_count);
    for (s = 0; s < struct_count; ++s) {
        mco_dict_struct(metadict, 0, s, &struct_info);
        for (e = 0; e < struct_info.event_count; ++e) {
            sample_join_task(&all_thr_param[thr_num++].task);
        }
    }
}


/* Generic synchronous event handler */
MCO_RET sync_event_handler(mco_trans_h t, mco_uda_object_handle_p obj, const mco_dict_event_info_t *event_info, void *param)
{
    mco_dict_struct_info_t struct_info;
    mco_dict_field_info_t field_info;
    mco_uda_value_t val;
    autoid_t obj_autoid;
    
    mco_dict_struct(metadict, 0, obj->struct_no, &struct_info);
    mco_dict_field_name(metadict, 0, obj->struct_no, "autoid", &field_info);
    val.type = MCO_DD_AUTOID;
    mco_uda_get(obj, field_info.field_no, 0, &val);
    obj_autoid = val.v.u8;
    
    if (event_info->type == MCO_EVENT_UPDATE || event_info->type == MCO_EVENT_CLASS_UPDATE) {
        mco_dict_field(metadict, 0, obj->struct_no, event_info->field_no, &field_info);
        printf("Sync. event for obj %" INT8_FORMAT "d: class %s, event_no=%d, type=UPDATE, when=%s, field=%s, value=", 
            obj_autoid, struct_info.name, event_info->event_no, ((mco_size_t)param == MCO_AFTER_UPDATE) ? "AFTER" : "BEFORE", field_info.name);
            
        print_obj_field(obj, &field_info);            
        
    } else {
        printf("Sync. event for obj %" INT8_FORMAT "d: class %s, event_no=%d, type=", obj_autoid, struct_info.name, event_info->event_no);
        switch (event_info->type) {
            case MCO_EVENT_NEW        : printf("NEW"); break;
            case MCO_EVENT_DELETE     : printf("DELETE"); break;
            case MCO_EVENT_DELETE_ALL : printf("DELETE_ALL"); break;
            case MCO_EVENT_CHECKPOINT : printf("CHECKPOINT"); break;
            default                   : printf("UNKNOWN"); dbg_exit(1);
        }
    }
    printf("\n");
    return MCO_S_OK;
}

MCO_RET put_obj_field(mco_uda_object_handle_t *obj, mco_dict_field_info_t *field_info, int value)
{
    mco_uda_value_t val;
    char buf[20];
    
    val.type = field_info->type;
    switch (val.type) {
        case MCO_DD_UINT1: val.v.u1 = (uint1)value; break;
        case MCO_DD_UINT2: val.v.u2 = (uint2)value; break;
        case MCO_DD_DATE:
        case MCO_DD_TIME:
        case MCO_DD_UINT4: val.v.u4 = (uint4)value; break;
        case MCO_DD_UINT8: val.v.u8 = (uint8)value; break;
        case MCO_DD_INT1: val.v.i1 = (int1)value; break;
        case MCO_DD_INT2: val.v.i2 = (int2)value; break;
        case MCO_DD_INT4: val.v.i4 = (int4)value; break;
        case MCO_DD_INT8: val.v.i8 = (mco_int8)value; break;
        case MCO_DD_FLOAT: val.v.f = (float)value; break;
        case MCO_DD_DOUBLE: val.v.d = (double)value; break;
        case MCO_DD_CHAR:
        case MCO_DD_STRING:
                        val.v.p.len = sprintf(buf, "value_%d", value);
                        val.v.p.p.c = buf;
                        break;
        default: return MCO_S_OK;
    }
    return mco_uda_put(obj, field_info->field_no, 0, &val);
}

void register_sync_events(mco_trans_h t)
{
    unsigned short struct_count, s, e;
    mco_dict_struct_info_t struct_info;
    mco_dict_event_info_t event_info;
    
    mco_dict_struct_count(metadict, 0, &struct_count);
    for (s = 0; s < struct_count; ++s) {
        mco_dict_struct(metadict, 0, s, &struct_info);
        for (e = 0; e < struct_info.event_count; ++e) {
            mco_dict_event(metadict, 0, s, e, &event_info);
            if (event_info.type == MCO_EVENT_UPDATE) {
                mco_uda_register_event_handler(t, s, e, &sync_event_handler, (void*)MCO_BEFORE_UPDATE, MCO_BEFORE_UPDATE);
                mco_uda_register_event_handler(t, s, e, &sync_event_handler, (void*)MCO_AFTER_UPDATE, MCO_AFTER_UPDATE);
            } else {
                mco_uda_register_event_handler(t, s, e, &sync_event_handler, 0, 0);
            }
        }
    }
}

void unregister_sync_events(mco_trans_h t)
{
    unsigned short struct_count, s, e;
    mco_dict_struct_info_t struct_info;
    mco_dict_event_info_t event_info;
    
    mco_dict_struct_count(metadict, 0, &struct_count);
    for (s = 0; s < struct_count; ++s) {
        mco_dict_struct(metadict, 0, s, &struct_info);
        for (e = 0; e < struct_info.event_count; ++e) {
            mco_dict_event(metadict, 0, s, e, &event_info);
            if (event_info.type == MCO_EVENT_UPDATE) {
                mco_uda_unregister_event_handler(t, s, e, &sync_event_handler);
            }
            mco_uda_unregister_event_handler(t, s, e, &sync_event_handler);
        }
    }
}

void insert_objs(mco_trans_h t, int value)
{
    unsigned short struct_count, s, f;
    mco_uda_object_handle_t obj;
    mco_dict_struct_info_t struct_info;
    mco_dict_field_info_t field_info;
    
    mco_dict_struct_count(metadict, 0, &struct_count);
    for (s = 0; s < struct_count; ++s) {
        mco_dict_struct(metadict, 0, s, &struct_info);
        
        if ((struct_info.flags & MCO_DICT_SI_CLASS) == 0) continue; /* skip structs */
        /* Create new object */
        mco_uda_new(t, s, 0, 0, 0, &obj);

        /* Fill object's fields */
        for (f = 0; f < struct_info.field_count; ++f) {
            mco_dict_field(metadict, 0, s, f, &field_info);
            if ((field_info.flags & (MCO_DICT_FI_VECTOR | MCO_DICT_FI_ARRAY)) == 0) {
                put_obj_field(&obj, &field_info, value);
            }
        }
    }
    
}

void update_objs(mco_trans_h t, int obj_num, int value)
{
    unsigned short struct_count, s, f;
    mco_uda_object_handle_t obj;
    mco_dict_struct_info_t struct_info;
    mco_dict_field_info_t field_info;
    mco_dict_index_info_t index_info;
    
    mco_dict_struct_count(metadict, 0, &struct_count);
    for (s = 0; s < struct_count; ++s) {
        mco_cursor_t csr;
        mco_uda_value_t key;
        mco_dict_struct(metadict, 0, s, &struct_info);
        if ((struct_info.flags & MCO_DICT_SI_CLASS) == 0) continue; /* Skip structs */
        
        mco_dict_index_name(metadict, 0, s, "autoid", &index_info);
        key.type = MCO_DD_AUTOID;
        key.v.u8 = (uint8)(obj_num * struct_count + s + 1);

        mco_uda_cursor(t, s, index_info.index_no, &csr);
        mco_uda_lookup(t, s,  index_info.index_no, MCO_EQ, &key, 1, &csr);
        mco_uda_from_cursor(t, &csr, &obj);
        /* Fill object's fields */
        for (f = 0; f < struct_info.field_count; ++f) {
            mco_dict_field(metadict, 0, s, f, &field_info);
            if ((field_info.flags & (MCO_DICT_FI_VECTOR | MCO_DICT_FI_ARRAY)) == 0) {
                put_obj_field(&obj, &field_info, value);
            }
        }
    }
}

void delete_objs(mco_trans_h t, int obj_num)
{
    unsigned short struct_count, s;
    mco_uda_object_handle_t obj;
    mco_dict_struct_info_t struct_info;
    mco_dict_index_info_t index_info;
    
    mco_dict_struct_count(metadict, 0, &struct_count);
    for (s = 0; s < struct_count; ++s) {
        mco_cursor_t csr;
        mco_uda_value_t key;
        mco_dict_struct(metadict, 0, s, &struct_info);
        
        if ((struct_info.flags & MCO_DICT_SI_CLASS) == 0) continue; /* Skip structs */
        
        mco_dict_index_name(metadict, 0, s, "autoid", &index_info);
        key.type = MCO_DD_AUTOID;
        key.v.u8 = (uint8)(obj_num * struct_count + s + 1);

        mco_uda_cursor(t, s,  index_info.index_no, &csr);
        mco_uda_lookup(t, s,  index_info.index_no, MCO_EQ, &key, 1, &csr);
        mco_uda_from_cursor(t, &csr, &obj);
        mco_uda_delete(&obj);
    }
}


int main(int argc, char* argv[])
{
    const char* dbName = "udaevent";
    mco_db_h db = 0;
    mco_trans_h t;
    uint4 i;
    MCO_RET rc;
    mco_runtime_info_t info;
    sample_memory_t dbmem;

    sample_os_initialize(DEFAULT);

    sample_header(sample_descr);

    mco_error_set_handler(&sample_errhandler);

    mco_runtime_start();

    /* Open and connect to database */
    CHECK(sample_open_database( db_name, udaevent_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, MAX_DB_CONNECTIONS, &dbmem ));

    init_metadict();
    /* For SHM registry not loaded */
    mco_get_runtime_info(&info);
    if (info.mco_shm_supported) {
        mco_metadict_register(metadict, dbName, udaevent_get_dictionary(), 0);
    }

    /* Connect to the database, obtain a database handle */
    CHECK(mco_db_connect(dbName, &db));

    /* Register events */
    start_async_handlers();
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
    register_sync_events(t);
    mco_trans_commit(t);

    printf("\nInsert records :\n\n");
    for (i = 0; i < nRecords; i++)
    {
        mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        insert_objs(t, i * 2);
        mco_trans_commit(t);
        sample_sleep(100);
    }

    printf("\nUpdate records :\n\n");
    for (i = 0; i < nRecords; i++)
    {
        mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        update_objs(t, i, nRecords * 2 + i * 3);
        mco_trans_commit(t);
        sample_sleep(100);
    }

    printf("\nDelete records :\n\n");
    for (i = 0; i < nRecords; i++)
    {
        mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        delete_objs(t, i);
        mco_trans_commit(t);
        sample_sleep(100);
    }
    
    /* Deregister events */
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
    unregister_sync_events(t);
    mco_trans_commit(t);
    printf("\nStop async handlers :\n\n");
    stop_async_handlers(db);

    /* Disconnect from the database, db is no longer valid */
    mco_db_disconnect(db);

    /* Destroy the database */
    sample_close_database(db_name, &dbmem);
    free(metadict);
    mco_runtime_stop();

    sample_pause_end("\n\nPress any key to continue . . . ");

    sample_os_shutdown();
    return 0;
}
