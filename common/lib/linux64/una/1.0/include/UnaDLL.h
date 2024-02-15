// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UnaDLL.h -- definition of UNA's macro
// 
// Copyright (c) 2000, 2003, 2023 Ricoh Company, Ltd.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

#ifndef __UNA_DLL_H
#define __UNA_DLL_H

#if defined(UNA_DLL)
	#define UNA_EXPORT
	#define UNA_IMPORT
	#if defined(UNA_EXPORT_DATA)
		#define UNA_DATA		UNA_EXPORT
	#else
		#define UNA_DATA		UNA_IMPORT
	#endif
	#if defined(UNA_LOCAL_EXPORT_FUNCTION)
		#define UNA_LOCAL_FUNCTION	UNA_EXPORT
		#define UNA_UNAWD_EXPORT_FUNCTION
		#define UNA_UNANP_EXPORT_FUNCTION
	#else
		#define UNA_LOCAL_FUNCTION	UNA_IMPORT
	#endif
#else
	#define	UNA_FUNCTION
	#define	UNA_DATA
	#define UNA_LOCAL_FUNCTION
#endif

#if defined(SHOW_CALL)
	#include <stdio.h>
	#define DEBUGPRINT(x,y) fprintf(stdout,x,y);fflush(stdout)
#else
	#define DEBUGPRINT(x,y)
#endif

#endif //__UNA_DLL_H

//
// Copyright (c) 2000, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
