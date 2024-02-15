// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MergeReserve.cpp -- マージ処理へのエントリを管理する
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "KdTree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "KdTree/MergeReserve.h"

#include "Os/CriticalSection.h"
#include "Os/AutoCriticalSection.h"
#include "Os/Event.h"

#include "Common/Assert.h"
#include "Common/Configuration.h"
#include "Common/HashTable.h"
#include "Common/DoubleLinkedList.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
	// リストの型
	typedef Common::DoubleLinkedList<MergeReserve::Entry>	_List;

	// ハッシュ表の型
	typedef Common::HashTable<_List, MergeReserve::Entry>	_HashTable;

	//
	//	VARIABLE local
	//	_$$::_entryTable -- エントリのハッシュ表(重複チェック用)
	//
	_HashTable _entryTable(1024,
						   &MergeReserve::Entry::m_pBucketPrev,
						   &MergeReserve::Entry::m_pBucketNext);

	//
	//	VARIABLE local
	//	_$$::_entryList -- エントリのリスト(順序保存用)
	//
	_List _entryList(&MergeReserve::Entry::m_pListPrev,
					 &MergeReserve::Entry::m_pListNext);

	namespace _Hash
	{
		// ハッシュコードを得る
		unsigned int hashCode(const Lock::FileName& cFileName_,
							  int iType_)
		{
			return (cFileName_.getValue() << 2) + iType_;
		}

		// リストを検索する
		MergeReserve::Entry* find(_HashTable::Bucket& bucket,
								  const Lock::FileName& cFileName_, int iType_)
		{
			MergeReserve::Entry* r = 0;
			
			_HashTable::Bucket::Iterator b(bucket.begin());
			_HashTable::Bucket::Iterator i(b);
			_HashTable::Bucket::Iterator e(bucket.end());

			for (; i != e; ++i)
			{
				if ((*i).m_cFileName == cFileName_ &&
					(*i).m_iType == iType_)
				{
					// 見つかった
					r = &(*i);

					// 見つかりやすくする
					bucket.splice(b, bucket, i);

					break;
				}
			}

			return r;
		}
	}

	//
	//	VARIABLE local
	//	_$$::_latch -- 排他制御用のクリティカルセクション
	//
	Os::CriticalSection _latch;

	//
	//	VARIABLE local
	//	_$$::_Interval -- 指定した時間登録がなかったら処理する(秒)
	//
	Common::Configuration::ParameterInteger _Interval("KdTree_MergeInterval",
													  30);
}

//
//	FUNCTION public static
//	KdTree::MergeReserve::pushBack -- エントリを追加する
//
//	NOTES
//	重複チェックされ、同じエントリは複数追加できない
//
//	ARGUMENTS
//	const Lock::FileName& cFileName_
//		処理を開始するファイルのロック名
//	int iType_
//		処理タイプ
//
//	RETURN
//	bool
//		末尾に追加できた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
MergeReserve::pushBack(const Lock::FileName& cFileName_, int iType_)
{
	bool r = false;

	Os::AutoCriticalSection cAuto(_latch);

	// 重複チェック
	
	unsigned int addr = _Hash::hashCode(cFileName_, iType_)
		% _entryTable.getLength();
	_HashTable::Bucket& bucket = _entryTable.getBucket(addr);

	// リストを検索する
	Entry* e = _Hash::find(bucket, cFileName_, iType_);
	if (e == 0)
	{
		// 存在しないので挿入

		e = new Entry(cFileName_, iType_);
		bucket.pushFront(*e);
		_entryList.pushBack(*e);

		r = true;
	}
	else
	{
		// 存在したので、時間を現在時刻にする

		e->m_cTime = ModTime::getCurrentTime();

		// リストの末尾に移動する

		_List::Iterator ite(_entryList, e);
		_entryList.splice(_entryList.end(), _entryList, ite);
	}

	return r;
}

//
//	FUNCTION public static
//	KdTree::MergeReserve::popFront -- 先頭のエントリを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	KdTree::MergeReserve::Entry*
//		エントリが得られなかった場合は0を返す
//
//	EXCEPTIONS
//
MergeReserve::Entry*
MergeReserve::getFront()
{
	Os::AutoCriticalSection cAuto(_latch);

	if (_entryList.getSize() != 0)
	{
		Entry* e = &(_entryList.getFront());
		
		if ((ModTime::getCurrentTime() - e->m_cTime)
			> ModTimeSpan(_Interval.get()))
		{
			// 一定間隔更新処理が発生していないので、条件を満たしている
			
			return e;
		}
	}

	return 0;
}

//
//	FUNCTION public static
//	KdTree::MergeReserve::erase -- エントリを削除する
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::MergeReserve::Entry* pEntry_
//		削除するエントリ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MergeReserve::erase(Entry* pEntry_)
{
	Os::AutoCriticalSection cAuto(_latch);

	// バケットを得る
	unsigned int addr
		= _Hash::hashCode(pEntry_->m_cFileName, pEntry_->m_iType)
		% _entryTable.getLength();
	_HashTable::Bucket& bucket = _entryTable.getBucket(addr);

	// バケットから削除する
	bucket.erase(*pEntry_);

	// リストから削除する
	_entryList.erase(*pEntry_);

	delete pEntry_;
}

//
//	Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
