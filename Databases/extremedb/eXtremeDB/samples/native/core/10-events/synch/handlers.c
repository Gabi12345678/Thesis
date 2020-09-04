/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
/* Event handlers (call back functions) implementation */

#include <stdio.h>
#include <stdlib.h>

#include "evdb.h"
#include "handlers.h"

/* helper function to manage 64 bit ints */
int autoid2int(autoid_t id)
{
#ifdef MCO_CFG_QUAD_STRUCT
  return (int) id.lo;
#else
  return (int) id;
#endif    
}

/* Handler for the "<new>" event. Reads the autoid and prints it out */
MCO_RET new_handler(mco_trans_h t, MyClass* obj, MCO_EVENT_TYPE et,  /*INOUT */void* param)
{
  autoid_t id;

  MyClass_autoid_get(obj, &id);
  printf("\tEvent \"Object New\" : object (%d) created\n", autoid2int(id));
  return MCO_S_OK;
}


/* Handler for the "<delete>" event. Note that the handler is called before
 * the current transaction is committed. Therefore, the object is still
 * valid; the object handle is passed back to the handler and is used to obtain the
 * autoid of the object.
 * The event's handler return value is passed into the "delete" function and is
 * later examined by the mco_trans_commit(). If the value is anything but
 * MCO_S_OK, the transaction is rollbacked. In this sample every delete
 * transaction for an odd autoid is rolled back.
 */
MCO_RET delete_handler(mco_trans_h t, MyClass* obj, MCO_EVENT_TYPE et,  /*INOUT*/void* user_param)
{
  autoid_t id;

  MyClass_autoid_get(obj, &id);
  printf("\tEvent \"Object Delete\" : object (%d) delete attempted", autoid2int(id));
  return (autoid2int(id) % 2) ? MCO_S_NOTFOUND : MCO_S_OK;
}


/* Handler for the "update" event.  This handler is called before the update transaction
 * is commited - hence the value of the field being changed is reported unchanged yet.
 */
MCO_RET update_handler1(mco_trans_h t, MyClass* obj, MCO_EVENT_TYPE et,  /*INOUT*/void* param)
{
  uint4 u4;
  autoid_t id;

  MyClass_autoid_get(obj, &id);
  MyClass_u4_get(obj, &u4);
  printf("\tEvent \"Object Update\" (before commit): object (%d) value = %d\n", autoid2int(id), u4);
  return MCO_S_OK;
}

/* Another second handler for the "<update>" event, this one is called after the
 * transaction is committed. So, the updated field value is already commited to
 * the database.
 */
MCO_RET update_handler2(mco_trans_h t, MyClass* obj, MCO_EVENT_TYPE et,  /*INOUT*/void* param)
{
  uint4 u4;
  autoid_t id;

  MyClass_autoid_get(obj, &id);
  MyClass_u4_get(obj, &u4);
  printf("\tEvent \"Object Update\" (after commit): object (%d) value = %d\n", autoid2int(id), u4);
  return MCO_S_OK;
}


/* Handler for the "checkpoint" event.  This handler is called before the checkpoint transaction
 * is commited - hence the object coudn't be found in indexes.
 */
MCO_RET checkpoint_handler1(mco_trans_h t, MyClass* obj, MCO_EVENT_TYPE et,  /*INOUT*/void* param)
{
  uint4 u4;
  autoid_t id;
  autoid_t id2;
  MCO_RET rc;
  MyClass _obj;

  MyClass_autoid_get(obj, &id);
  MyClass_u4_get(obj, &u4);
  if ((rc = MyClass_hashIdx_find(t,u4,&_obj))==MCO_S_OK){
      MyClass_autoid_get(obj, &id2);
      rc = (id == id2) ?MCO_S_OK:MCO_S_NOTFOUND;
  }

  if (rc == MCO_S_OK){
      printf("\tEvent \"Object checkpoint\" (before commit): object (%d) value = %d, already in index\n", autoid2int(id), u4);
  }else{
      printf("\tEvent \"Object checkpoint\" (before commit): object (%d) value = %d, wasn't found in index\n", autoid2int(id), u4);
  }
  return MCO_S_OK;
}

/* Another second handler for the "<checkpoint>" event, this one is called after the
 * transaction is committed. So, the object is already located in indexes.
 */
MCO_RET checkpoint_handler2(mco_trans_h t, MyClass* obj, MCO_EVENT_TYPE et,  /*INOUT*/void* param)
{
    uint4 u4;
    autoid_t id;
    autoid_t id2;
    MCO_RET rc;
    MyClass _obj;
    
    MyClass_autoid_get(obj, &id);
    MyClass_u4_get(obj, &u4);
    if ((rc = MyClass_hashIdx_find(t,u4,&_obj))==MCO_S_OK){
        MyClass_autoid_get(obj, &id2);
        rc = (id == id2) ?MCO_S_OK:MCO_S_NOTFOUND;
    }
    
    if (rc == MCO_S_OK){
        printf("\tEvent \"Object checkpoint\" (after commit): object (%d) value = %d, already in index\n", autoid2int(id), u4);
    }else{
        printf("\tEvent \"Object checkpoint\" (after commit): object (%d) value = %d, wasn't found in index\n", autoid2int(id), u4);
    }
    return MCO_S_OK;
}


MCO_RET register_events(mco_db_h db)
{
  MCO_RET rc;
  mco_trans_h t;

  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {
    mco_register_newEvent_handler(t, &new_handler, (void*)0);
    mco_register_deleteEvent_handler(t, &delete_handler, (void*)0);
    mco_register_updateEvent_handler(t, &update_handler1, (void*)0, MCO_BEFORE_UPDATE);
    mco_register_updateEvent_handler(t, &update_handler2, (void*)0, MCO_AFTER_UPDATE);
    mco_register_checkpointEvent_handler(t, &checkpoint_handler1, (void*)0, MCO_BEFORE_UPDATE);
    mco_register_checkpointEvent_handler(t, &checkpoint_handler2, (void*)0, MCO_AFTER_UPDATE);
    rc = mco_trans_commit(t);
  }
  return rc;
}

MCO_RET unregister_events(mco_db_h db)
{
  MCO_RET rc;
  mco_trans_h t;

  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {
    mco_unregister_newEvent_handler(t, &new_handler);
    mco_unregister_deleteEvent_handler(t, &delete_handler);
    mco_unregister_updateEvent_handler(t, &update_handler1);
    mco_unregister_updateEvent_handler(t, &update_handler2);
    mco_unregister_checkpointEvent_handler(t, &checkpoint_handler1);
    mco_unregister_checkpointEvent_handler(t, &checkpoint_handler2);
    rc = mco_trans_commit(t);
  }
  return rc;
}
