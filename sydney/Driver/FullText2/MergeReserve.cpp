// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MergeReserve.cpp -- マージ処理へのエントリを管理する
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/MergeReserve.h"

#include "Os/CriticalSection.h"
#include "Os/AutoCriticalSection.h"
#include "Os/Event.h"

#include "Common/Assert.h"

#include "ModHashMap.h"
#include "ModList.h"
#include "ModPair.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//
	//	TYPEDEF
	//	_$$::_Entry -- 格納するデータの型
	//
	typedef ModPair<Lock::FileName, int>	_Entry;
	
	//
	//	CLASS
	//	_$$::_Hasher -- _Entry用のハッシュ関数
	//
	class _Hasher
	{
	public:
		ModSize operator() (const _Entry& e) const
		{
			return (e.first.getValue() << 8) + e.second;
		}
	};

	//
	//	VARIABLE local
	//	_$$::_entryMap -- エントリのマップ(重複チェック用)
	//
	ModHashMap<_Entry, int, _Hasher> _entryMap;

	//
	//	VARIABLE local
	//	_$$::_entryList -- エントリのリスト(マージ順保存用)
	//
	ModList<_Entry> _entryList;

	//
	//	VARIABLE local
	//	_$$::_latch -- 排他制御用のクリティカルセクション
	//
	Os::CriticalSection _latch;

	//
	//	VARIABLE local
	//	_$$::_event -- イベント
	//
	Os::Event _event;

}

//
//	FUNCTION public static
//	FullText2::MergeReserve::pushBack -- 末尾にエントリを追加する
//
//	NOTES
//	重複チェックされ、同じエントリは複数追加できない
//
//	ARGUMENTS
//	const Lock::FileName& cFileName_
//		マージ処理を開始するファイルのロック名
//	int iElement_
//		マージ処理を開始するファイルの要素番号
//
//	RETURN
//	bool
//		末尾に追加できた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
MergeReserve::pushBack(const Lock::FileName& cFileName_, int iElement_)
{
	Os::AutoCriticalSection cAuto(_latch);

	// 重複チェック
	_Entry e(cFileName_, iElement_);
	if (_entryMap.find(e) == _entryMap.end())
	{
		// 存在しないので挿入
		_entryMap.insert(e, 0);
		_entryList.pushBack(e);

		// イベント発行
		_event.set();
		
		return true;
	}
	return false;
}

//
//	FUNCTION public static
//	FullText2::MergeReserve::getFront -- 先頭のエントリを得る
//
//	NOTES
//
//	ARGUMENTS
//	Lock::FileName& cFileName_
//		先頭ファイルのロック名
//	int& iElement_
//		先頭ファイルの要素番号
// 	unsigned int msec_
//		エントリがない場合に待つ時間(ミリ秒)
//
//	RETURN
//	bool
//		エントリが得られた場合はtrue、タイムアウトした場合はfalse
//
//	EXCEPTIONS
//
bool
MergeReserve::getFront(Lock::FileName& cFileName_,
					   int& iElement_,
					   unsigned int msec_)
{
	do
	{
		Os::AutoCriticalSection cAuto(_latch);

		if (_entryList.getSize() != 0)
		{
			_Entry& e = _entryList.getFront();
			cFileName_ = e.first;
			iElement_ = e.second;
			
			return true;
		}
	}
	while (_event.wait(msec_) == true);

	return false;
}

//
//	FUNCTION public static
//	FullText2::MergeReserve::popFront -- 先頭のエントリを削除する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MergeReserve::popFront()
{
	Os::AutoCriticalSection cAuto(_latch);

	;_SYDNEY_ASSERT(_entryList.getSize());

	_Entry& e = _entryList.getFront();
	_entryMap.erase(e);
	_entryList.popFront();
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
