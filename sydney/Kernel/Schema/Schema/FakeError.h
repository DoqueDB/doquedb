// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FakeError.h -- スキーマモジュールの擬似エラー発生のための定義
// 
// Copyright (c) 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_FAKEERROR_H
#define	__SYDNEY_SCHEMA_FAKEERROR_H

#include "Schema/Module.h"
#include "Exception/FakeError.h"
#include "Exception/Unexpected.h"

//	MACRO
//		SCHEMA_FAKE_ERROR -- 擬似エラーを発生させるためのマクロ
//
//	NOTES
//		マクロの引数には文字列のリテラルしか書けないので注意

#define SCHEMA_FAKE_ERROR(__func__, __key__, __value__) \
	_SYDNEY_FAKE_ERROR(__func__ "_" __key__ "_" __value__, \
					   Exception::Unexpected(moduleName, srcFile, __LINE__))

#define SCHEMA_FAKE_ERROR2(__func__, __key__, __value__, __exception__) \
	_SYDNEY_FAKE_ERROR(__func__ "_" __key__ "_" __value__, \
					   Exception::__exception__(moduleName, srcFile, __LINE__))

#endif	// __SYDNEY_SCHEMA_FAKEERROR_H

//
// Copyright (c) 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
