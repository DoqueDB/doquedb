// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DLL.h -- Statement の DLL 関連のマクロを定義する
// 
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_DLL_H
#define __SYDNEY_STATEMENT_DLL_H

#ifdef SYD_DLL
#ifdef SYD_STATEMENT_EXPORT_FUNCTION
#define SYD_STATEMENT_FUNCTION	SYD_EXPORT
#else
#define SYD_STATEMENT_FUNCTION	SYD_IMPORT
#endif
#else
#define	SYD_STATEMENT_FUNCTION
#endif

#endif //__SYDNEY_STATEMENT_DLL_H

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
