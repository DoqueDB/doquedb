// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statistics.cpp -- 統計情報関連の関数定義
// 
// Copyright (c) 2003, 2008, 2009, 2013, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Buffer";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"

#include "Buffer/Statistics.h"
#include "Buffer/Debug.h"

#include "Os/AutoCriticalSection.h"
#include "Os/Memory.h"

_SYDNEY_USING
_SYDNEY_BUFFER_USING

namespace
{
	// システム唯一のインスタンス

	Statistics	_statistics;
}

//	FUNCTION public
//	Buffer::Statistics::Statistics --
//		統計情報を表すクラスのコピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Statistics&		v
//			コピー元の統計情報を表すクラスへのリファレンス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Statistics::Statistics(const Statistics& v)
{
	(void) Os::Memory::copy(_count, v._count, sizeof(_count));
	(void) Os::Memory::copy(_size, v._size, sizeof(_size));
}

//	FUNCTION public
//	Buffer::Statistics::operator = --
//		統計情報を表すクラスの代入演算子
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Statistics&		v
//			コピー元の統計情報を表すクラスへのリファレンス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Statistics&
Statistics::operator =(const Statistics& v)
{
	(void) Os::Memory::copy(_count, v._count, sizeof(_count));
	(void) Os::Memory::copy(_size, v._size, sizeof(_size));

	return *this;
}

//	FUNCTION public static
//	Buffer::Statistics::record --
//		バッファページに対する操作を記録する
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Statistics::Category::Value	category
//			記録する操作を表す値
//		Os::Memory::Size		size
//			記録するサイズ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Statistics::record(Category::Value category, Os::Memory::Size size)
{
	Os::AutoCriticalSection cAuto(_statistics._latch);
	
	++_statistics._count[category];
	_statistics._size[category] += size;
}

//	FUNCTION public static
//	Buffer::Statistics::printLog --
//		バッファページに関する統計情報をログに出力する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//

void
Statistics::printLog()
{
	Statistics v;
	
	{
		// ログへの出力はファイルIOを含んでおり、時間がかかる可能性がある
		// ロック時間をなるべく少なくしたいので、出力データをコピーする
		// 出力した内容はクリアする
		
		Os::AutoCriticalSection cAuto(_statistics._latch);
		v = _statistics;
		_statistics.clear();
	}

	// ログに出力する
	
	for (unsigned int i = 0; i < Statistics::Category::Count; ++i) {
		const char* name = 0;
		switch (i) {
		case Statistics::Category::Fix:
			name = "Fix";		break;
		case Statistics::Category::Unfix:
			name = "Unfix";		break;
		case Statistics::Category::Read:
			name = "Read";		break;
		case Statistics::Category::Write:
			name = "Write";		break;
		case Statistics::Category::Allocate:
			name = "Allocate";	break;
		case Statistics::Category::Free:
			name = "Free";		break;
		case Statistics::Category::Replace:
			name = "Replace";	break;
		case Statistics::Category::Exhaust:
			name = "Exhaust";	break;
		}

		_SYDNEY_BUFFER_DEBUG_MESSAGE
			<< "[Statistics] "
			<< name
			<< " : " << v._count[i]
			<< " times : " << v._size[i]
			<< " bytes"
			<< ModEndl;
	}
}

//	FUNCTION private
//	Buffer::Pool::Statistics::clear -- 記録している情報を初期化する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Statistics::clear()
{
	(void) Os::Memory::reset(_count, sizeof(_count));
	(void) Os::Memory::reset(_size, sizeof(_size));
}

//
// Copyright (c) 2003, 2008, 2009, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
