// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PathParts.h -- ファイルのパス名関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_TRANS_PATHPARTS_H
#define	__SYDNEY_TRANS_PATHPARTS_H

#include "Trans/Module.h"

#include "Os/Unicode.h"

_SYDNEY_BEGIN
_SYDNEY_TRANS_BEGIN

namespace PathParts
{
	//	CONST
	//	Trans::PathParts::TimeStamp --
	//		タイムスタンプ値の上位 32 ビットを記録するためのファイルの名前
	//
	//	NOTES

	const Os::Ucs2 TimeStamp[] =
		{ 'T','I','M','E','S','T','A','M','P','.','S','Y','D', 0 };

	//	CONST
	//	Trnas::PathParts::LockFile --
	//		二重立ち上げ防止用のロックファイルのファイルの名前
	//
	//	NOTES

	const Os::Ucs2 LockFile[] =
		{ 'L','O','C','K','.','S','Y','D', 0 };
}

_SYDNEY_TRANS_END
_SYDNEY_END

#endif	// __SYDNEY_TRANS_PATHPARTS_H

//
// Copyright (c) 2000, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
