/*******************************************************************
 *                                                                 *
 *  mcoxml.h                                                       *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           * 
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/

#ifndef MCO_XML_H__
    #define MCO_XML_H__

    /*
     * ++
     * 
     * PROJECT:   eXtremeDB(tm) (c) McObject LLC
     *
     * SUBSYSTEM: XML support
     *
     * MODULE:    MCOXML.H
     *
     * ABSTRACT:  Header file of data xml i/o
     *
     *
     * VERSION:   1.0
     *
     * HISTORY:
     *            1.0- 1 AD     08-Aug-2002 Created it was (xml output)
     *                 2 AD     19-Aug-2002 added Base64 encoding
     *                 3 AD     28-Aug-2002 XML parsing was created
     *                 4 AD     05-Sep-2002 Added flagsfor string coding
     *                   TP     27-Sep-2003 Alternative XML representation
     *			
     *
     * --
     */

    #ifdef __cplusplus
        extern "C"
        {
        #endif 

        #include "mco.h"

        /*
         * Data format policy
         */
        typedef enum MCO_NUM_BASE_E
        {
            MCO_NUM_OCT = 8, MCO_NUM_DEC = 10, MCO_NUM_HEX = 16
        } MCO_NUM_BASE;

        typedef enum MCO_TEXT_CODING_E
        {
            MCO_TEXT_ASCII = 1, MCO_TEXT_BINHEX = 2, MCO_TEXT_BASE64 = 3
        } MCO_TEXT_CODING;

        typedef enum MCO_FLOAT_FORMAT_E
        {
            MCO_FLOAT_FIXED = 1, MCO_FLOAT_EXPONENT = 2
        } MCO_FLOAT_FORMAT;

        typedef struct mco_xml_policy_t_
        {
            MCO_NUM_BASE int_base;
            MCO_NUM_BASE quad_base;
            MCO_TEXT_CODING text_coding;
            MCO_TEXT_CODING blob_coding;
            MCO_FLOAT_FORMAT float_format;
            mco_bool indent;
            mco_bool ignore_field; /* ignore field in xml, that is not in class         */
            mco_bool encode_spec; /* encode chars with code < 32, except LF            */
            mco_bool encode_lf; /* encode line feeds                                 */
            mco_bool encode_nat; /* encode national chars (code > 127)                */
            mco_bool truncate_sp; /* truncate trailing spaces in chars                 */
            mco_bool use_xml_attrs; /* alternative XML representation, using atributes   */
            mco_bool ignore_autoid; /* ignore autoid value in put and create operations  */
            mco_bool ignore_autooid; /* pass auto_oid in get/put/create operations        */
        } mco_xml_policy_t;

        void mco_xml_get_default_policy( /*OUT*/mco_xml_policy_t* p);
        MCO_RET mco_xml_get_policy(mco_trans_h t,  /*OUT*/mco_xml_policy_t* p);
        MCO_RET mco_xml_set_policy(mco_trans_h t, const mco_xml_policy_t* p);

        MCO_RET mco_db_xml_export(mco_trans_h t, void* stream_handle, mco_stream_write output_stream_writer);
        MCO_RET mco_db_xml_import(mco_trans_h t, void* stream_handle, mco_stream_read input_stream_reader);

        #ifdef __cplusplus
        }
    #endif 


#endif
