/***************************************************************
 *                                                             *
 * Copyright (c) 2001-2019 McObject LLC. All Right Reserved.   *
 *                                                             *
 ***************************************************************/

#ifndef HANDLERS_H__
#define HANDLERS_H__

#ifdef __cplusplus
extern "C"
{
#endif 

    MCO_RET register_events(mco_db_h db);
    MCO_RET unregister_events(mco_db_h db); 
    /* convert autoid_t to int */
    int autoid2int(autoid_t id);


#ifdef __cplusplus
}
#endif 

#endif /* HANDLERS_H__ */
