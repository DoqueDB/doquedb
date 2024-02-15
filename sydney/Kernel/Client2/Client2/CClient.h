/* -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
 * 
 * Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
   vi:set ts=4 sw=4:

   CClient.h -- 
*/

#ifndef __TRMEISTER_CLIENT2_CCLIENT_C_H
#define __TRMEISTER_CLIENT2_CCLIENT_C_H

#include "SyDLL.h"

#ifdef __cplusplus
extern "C" {
#endif

	/* DataSource */
	typedef void tr_ds_t;
	/* Session */
	typedef void tr_ss_t;
	/* ResultSet */
	typedef void tr_rs_t;
	/* Tuple */
	typedef void tr_tp_t;
	/* PrepareStatement */
	typedef void tr_ps_t;

	/* bool */
	typedef enum
	{
		tr_ok = 1,
		tr_ng = 0
	} tr_ok_t;

	/* Kanji Code */
	/* NOTE: The character code is always utf8 in this version */
	/*
	typedef enum
	{
		tr_kc_euc		= 0,
		tr_kc_jis		= 1,
		tr_kc_shiftJis	= 2,
		tr_kc_utf8		= 3,
		tr_kc_ucs2		= 4,	// ModUnicodeChar 型の文字は UCS-2 の相当する
		tr_kc_unknown			// 未知の文字コード種類
	} tr_kc_t;
	*/

	/* Status */
	typedef enum
	{
		tr_st_undefined = 0,	/* 不明なステータス */

		tr_st_data,				/* データである */
		tr_st_endofdata,		/* データ終了である */
		tr_st_success,			/* 正常終了 */
		tr_st_canceled,			/* キャンセルされた */
		tr_st_error,			/* エラーが発生した */
		tr_st_metadata,			/* 結果集合メタデータ */
		tr_st_hasmoredata		/* (複文で)続きのデータがある */
	} tr_st_t;

	/* Data Type */
	typedef enum
	{
		tr_type_data =					1000,

		tr_type_minscalar,
		/* ------------------------------- */
		tr_type_integer =				tr_type_minscalar,
		tr_type_unsignedinteger,
		tr_type_integer64,
		tr_type_unsignedinteger64,
		tr_type_string,
		tr_type_float,
		tr_type_double,
		tr_type_decimal,
		tr_type_date,
		tr_type_datetime,
		tr_type_binary,
		tr_type_bitset,
		tr_type_objectid,
		tr_type_language,
		tr_type_columnmetadata,
		tr_type_word,
		/* ------------------------------- */
		tr_type_maxscalar,

		tr_type_array =					2000,
		
		tr_type_null =					3000,

		tr_type_default =				4000,
		
		tr_type_undefined =				9999
	} tr_type_t;

	/* Error */
	/* return value: 0 means no error occurred */
	SYD_FUNCTION int tr_errno(const tr_ds_t* ds);
	SYD_FUNCTION const char* tr_sqlstate(const tr_ds_t* ds);
	SYD_FUNCTION unsigned int tr_error_message_size(const tr_ds_t* ds);
	/* return:
	   string size [byte] (>=0)
	   bad argument (=-1)
	   short of bufsize (=-2)
	   some errors occurred (=-3) */
	SYD_FUNCTION int tr_error_message(tr_ds_t* ds,
									  char* buf,
									  unsigned int bufsize);
	
	SYD_FUNCTION tr_ok_t tr_reset_error(tr_ds_t* ds);

	/* Singleton */
	SYD_FUNCTION tr_ds_t* tr_initialize();
	SYD_FUNCTION tr_ok_t tr_terminate(tr_ds_t* ds);

	/* DataSource */
	SYD_FUNCTION tr_ok_t tr_open(tr_ds_t* ds,
								 const char* host_name,
								 int port_number);
	
	SYD_FUNCTION tr_ok_t tr_close(tr_ds_t* ds);

	SYD_FUNCTION tr_ss_t* tr_create_session(tr_ds_t* ds,
											const char* database_name,
											const char* user_name,
											const char* password);
	
	/* Session */
	SYD_FUNCTION tr_rs_t* tr_execute_statement(tr_ss_t* ss,
											   const char* statement);
	/*
	SYD_FUNCTION tr_rs_t* tr_execute_statement(tr_ss_t* ss,
											   const char* statement,
											   tr_kc_t kanji);
	*/
	SYD_FUNCTION tr_ps_t* tr_create_preparestatement(tr_ss_t* ss,
													 const char* statement);
	SYD_FUNCTION tr_rs_t* tr_execute_preparestatement(tr_ss_t* ss,
													  tr_ps_t* ps,
													  tr_tp_t* array);
	SYD_FUNCTION tr_ok_t tr_erase_preparestatement(tr_ps_t* ps);

	SYD_FUNCTION tr_ok_t tr_close_session(tr_ss_t* ss);

	/* ResultSet */
	SYD_FUNCTION tr_tp_t* tr_get_next_tuple(tr_rs_t* rs,
											tr_st_t* status);	/* ステータス */
	
	SYD_FUNCTION unsigned int tr_get_metadata_size(const tr_rs_t* rs);
	/* return: see tr_error_message */
	SYD_FUNCTION int tr_get_metadata(tr_rs_t* rs,
									 char* buf,
									 unsigned int bufsize);
	
	SYD_FUNCTION tr_ok_t tr_close_resultset(tr_rs_t* rs);
	
	/* Tuple */
	/* return: the number of elements */
	SYD_FUNCTION int tr_get_tuple_size(const tr_tp_t* tuple);
	
	SYD_FUNCTION tr_type_t tr_get_tuple_type(const tr_tp_t* tuple,
											 int i);	/* 要素番号 */
	
	SYD_FUNCTION tr_ok_t tr_release_tuple(tr_tp_t* tuple);
	
	/* Element */
	/* general purpose */
	SYD_FUNCTION unsigned int tr_get_string_size(const tr_tp_t* tuple,
												 int i);
	/* return: see tr_error_message */
	SYD_FUNCTION int tr_get_string(const tr_tp_t* tuple,
								   int i,
								   char* buf,
								   unsigned int bufsize);
	/* single purpose */
	SYD_FUNCTION int tr_get_int(const tr_tp_t* tuple,
								int i);
	SYD_FUNCTION unsigned int tr_get_unsignedint(const tr_tp_t* tuple,
												 int i);
	SYD_FUNCTION long long tr_get_bigint(const tr_tp_t* tuple,
										 int i);
	SYD_FUNCTION double tr_get_float(const tr_tp_t* tuple,
									 int i);
	SYD_FUNCTION unsigned int tr_get_decimal_size(const tr_tp_t* tuple,
												  int i);
	SYD_FUNCTION void tr_get_decimal(const tr_tp_t* tuple,
									 int i,
									 char* decimal);
	SYD_FUNCTION unsigned int tr_get_language_size(const tr_tp_t* tuple,
												   int i);
	SYD_FUNCTION void tr_get_language(const tr_tp_t* tuple,
									  int i,
									  char* language);
	SYD_FUNCTION unsigned int tr_get_datetime_size(const tr_tp_t* tuple,
												   int i);
	SYD_FUNCTION void tr_get_datetime(const tr_tp_t* tuple,
									  int i,
									  char* datetime);
	SYD_FUNCTION unsigned int tr_get_binary_size(const tr_tp_t* tuple,
												 int i);
	/* return: see tr_error_message */
	SYD_FUNCTION void tr_get_binary(const tr_tp_t* tuple,
									int i,
									void* buf,
									unsigned int bufsize);
	SYD_FUNCTION tr_tp_t* tr_get_array(tr_tp_t* tuple,
									   int i);

	/* Array */
	SYD_FUNCTION tr_ok_t tr_release_array(tr_tp_t* array);
	SYD_FUNCTION int tr_get_array_size(const tr_tp_t* array);
	SYD_FUNCTION tr_tp_t* tr_create_array();
	SYD_FUNCTION tr_ok_t tr_set_string(tr_tp_t* array,
									   int i,
									   char* buf,
									   unsigned int size);
	SYD_FUNCTION tr_ok_t tr_set_int(tr_tp_t* array,
									int i,
									int v);
	SYD_FUNCTION tr_ok_t tr_set_unsignedint(tr_tp_t* array,
											int i,
											unsigned int v);
	SYD_FUNCTION tr_ok_t tr_set_bigint(tr_tp_t* array,
									   int i,
									   long long v);
	SYD_FUNCTION tr_ok_t tr_set_float(tr_tp_t* array,
									  int i,
									  double v);
	SYD_FUNCTION tr_ok_t tr_set_decimal(tr_tp_t* array,
										int i,
										char* decimal,
										unsigned int size);
	SYD_FUNCTION tr_ok_t tr_set_language(tr_tp_t* array,
										 int i,
										 char* language,
										 unsigned int size);
	SYD_FUNCTION tr_ok_t tr_set_datetime(tr_tp_t* array,
										 int i,
										 char* datetime,
										 unsigned int size);
	SYD_FUNCTION tr_ok_t tr_set_binary(tr_tp_t* array,
									   int i,
									   void* buf,
									   unsigned int size);
	SYD_FUNCTION tr_ok_t tr_set_array(tr_tp_t* array,
									  int i,
									  tr_tp_t* v);
	SYD_FUNCTION tr_ok_t tr_set_null(tr_tp_t* array,
									 int i);

#ifdef __cplusplus
}
#endif

#endif /* __TRMEISTER_CLIENT2_CCLIENT_C_H */

/*
*/
