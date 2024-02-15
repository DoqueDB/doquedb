// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// XID.cpp -- トランザクションブランチ識別子関連の関数定義
// 
// Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Trans";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Trans/XID.h"

#include "ModArchive.h"

_SYDNEY_USING
_SYDNEY_TRANS_USING

//	FUNCTION public
//	Trans::XID::serialize -- クラスをシリアル化する
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&			archiver
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
XID::serialize(ModArchive& archiver)
{
	// 自分自身をシリアル化する

	if (archiver.isStore())
		archiver << _gtrID << _bqual << _formatID;
	else
		archiver >> _gtrID >> _bqual >> _formatID;
}

//
// Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
