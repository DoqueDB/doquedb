/* -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
 * vi:set ts=4 sw=4:
 *
 * SyDLL.h -- TRMeister の DLL 関連のマクロを定義する
 * 
 * Copyright (c) 2000, 2003, 2007, 2009, 2011, 2012, 2023 Ricoh Company, Ltd.
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
 */

#ifndef __SY_DLL_H
#define __SY_DLL_H

#ifdef SYD_DLL
#ifdef SYD_OS_WINDOWS
#define SYD_EXPORT		__declspec(dllexport)
#define SYD_IMPORT		__declspec(dllimport)
#else
#define SYD_EXPORT
#define SYD_IMPORT
#endif
#ifdef SYD_EXPORT_FUNCTION
#define SYD_FUNCTION	SYD_EXPORT
#else
#define SYD_FUNCTION	SYD_IMPORT
#endif
#ifdef SYD_EXPORT_DATA
#define SYD_DATA		SYD_EXPORT
#else
#define SYD_DATA		SYD_IMPORT
#endif
#else
#define	SYD_FUNCTION
#define	SYD_DATA
#endif
#ifdef SYD_KERNEL_EXPORT_FUNCTION
#define SYD_ADMIN_EXPORT_FUNCTION
#define SYD_ANALYSIS_EXPORT_FUNCTION
#define SYD_BUFFER_EXPORT_FUNCTION
#define SYD_CHECKPOINT_EXPORT_FUNCTION
#define SYD_COMMON_EXPORT_FUNCTION
#define SYD_COMMUNICATION_EXPORT_FUNCTION
#define SYD_EXCEPTION_EXPORT_FUNCTION
#define SYD_EXECUTION_EXPORT_FUNCTION
#define SYD_LOCK_EXPORT_FUNCTION
#define SYD_LOGICALFILE_EXPORT_FUNCTION
#define SYD_LOGICALLOG_EXPORT_FUNCTION
#define SYD_OPT_EXPORT_FUNCTION
#define SYD_OS_EXPORT_FUNCTION
#define SYD_PHYSICALFILE_EXPORT_FUNCTION
#define SYD_PLAN_EXPORT_FUNCTION
#define SYD_SCHEMA_EXPORT_FUNCTION
#define SYD_SERVER_EXPORT_FUNCTION
#define SYD_STATEMENT_EXPORT_FUNCTION
#define SYD_TRANS_EXPORT_FUNCTION
#define SYD_UTILITY_EXPORT_FUNCTION
#define SYD_VERSION_EXPORT_FUNCTION
#define SYD_DSERVER_EXPORT_FUNCTION
#define SYD_DPLAN_EXPORT_FUNCTION
#define SYD_DEXECUTION_EXPORT_FUNCTION
#endif

#endif /*__SY_DLL_H */

/*
 * Copyright (c) 2000, 2003, 2007, 2009, 2011, 2012, 2023 Ricoh Company, Ltd.
 * All rights reserved.
 */
