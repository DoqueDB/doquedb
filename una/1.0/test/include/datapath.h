// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// datapath.h -- 
// 
// Copyright (c) 2000, 2001, 2003, 2023 Ricoh Company, Ltd.
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
#ifndef	__datapath_H_
#define __datapath_H_

#include "ModUnicodeChar.h"

const char* dataDirPath
	= "../unadic-n/";
const char* ruleDicPath
	= "../unadic-n/ruleWrd.dic";
const char* ruleAppPath
	= "../unadic-n/ruleApp.dic";
const char* expDicPath
	= "../unadic-n/expWrd.dic";
const char* expAppPath
	= "../unadic-n/expApp.dic";
const char* connectPath
	= "../unadic-n/connect.tbl";
const char* unknownTablePath
	= "../unadic-n/unkmk.tbl";
const char* unknownCostPath
	= "../unadic-n/unkcost.tbl";
const char* normalTablePath
	= "../unadic-n/unastd.tbl";
const char* preMapPath
	= "../unadic-n/preMap.dat";
const char* postMapPath
	= "../unadic-n/postMap.dat";
const char* combiMapPath
	= "../unadic-n/combiMap.dat";

const ModUnicodeChar delim1 = 0x7b;		// {
const ModUnicodeChar delim2 = 0x2c;		// ,
const ModUnicodeChar delim3 = 0x7d;		// }
const ModUnicodeChar delim4 = 0x5c;		// \\

#endif // __datapath_H_
//
// Copyright (c) 2000, 2001, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
