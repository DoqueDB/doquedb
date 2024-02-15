// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DBGetFileDriver.cpp -- Function that obtains record file driver
// 
// Copyright (c) 2000, 2006, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "LogicalFile/FileDriver.h"
#include "Record2/FileDriver.h"

_SYDNEY_USING

#ifdef SYD_DLL

//
//	FUNCTION global
//	DBGetFileDriver -- The instance of the file driver of the record file is obtained. 
//
//	NOTES
//	The instance of the file driver of the record file is obtained. 
//
//	ARGUMENTS
//	None
//
//	RETURN
//	Logical::FileDriver*
//		File driver of record file
//
LogicalFile::FileDriver*
DBGetFileDriver()
{
	//obtain subtance and free by upper module
	return new Record2::FileDriver;
}

#endif

//
//	Copyright (c) 2000, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
