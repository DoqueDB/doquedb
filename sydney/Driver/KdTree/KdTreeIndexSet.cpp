// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// KdTreeIndexSet.cpp --
// 
// Copyright (c) 2013, 2014, 2023 Ricoh Company, Ltd.
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
#include "KdTree/KdTreeIndexSet.h"

#include "KdTree/Archiver.h"
#include "KdTree/BtreeDataFile.h"
#include "KdTree/CalcVariance.h"
#include "KdTree/IndexFile.h"
#include "KdTree/KdTreeIndex.h"
#include "KdTree/LoadEntry.h"
#include "KdTree/MakeTreeRecursive.h"
#include "KdTree/SortEntry.h"

#include "Common/DoubleLinkedList.h"
#include "Common/Message.h"

#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"
#include "Os/Memory.h"

#include "Trans/List.h"
#include "Trans/Transaction.h"

#include "Exception/Cancel.h"

#include "ModAutoPointer.h"
#include "ModHashMap.h"
#include "ModHasher.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
	//
	//	KdTreeIndexSet を管理するハッシュマップ
	//
	ModHashMap<ModUnicodeString, KdTreeIndexSet*,
			   ModHasher<ModUnicodeString> > _cKdTreeIndexSetHashMap;

	//
	//	排他制御用
	//
	Os::CriticalSection _cLatch;

	namespace _Transaction
	{
		// 現在実行中の版管理するトランザクションのうち、
		// あるタイムスタンプ以降で、指定された更新トランザクションが
		// すべて終了するまでに開始されたものがあるか調べる
		bool
		isOverlapped(Schema::ObjectID::Value db_,
					 Trans::TimeStamp::Value t,
					 const ModVector<Trans::Transaction::ID>& ids,
					 Trans::TimeStamp& start);

	}
}

//	FUNCTION
//	_$$::_Transaction::isOverlapped --
//		現在実行中の版管理するトランザクションのうち、
//		あるタイムスタンプ以降で、指定された更新トランザクションが
//		すべて終了するまでに開始されたものがあるか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp::Value	t
//			このタイムスタンプ値以降に開始された
//			版管理するトランザクションがあるか調べる
//		ModVector<Trans::Transaction::ID>&	ids
//			現在実行中の版管理するトランザクションが開始されたときに、
//			実行中だったか調べる更新トランザクションの
//			トランザクション識別子を要素として持つリストで、
//			要素は格納するトランザクション識別子の表す
//			トランザクションの開始時刻の昇順に並んでいる必要がある
//		Trans::TimeStamp&	start
//			現在実行中の版管理するトランザクションのうち、
//			指定されたタイムスタンプ以降で、
//			指定された更新トランザクションがすべて終了してから
//			いちばん最初に開始されたものの開始時タイムスタンプが設定される
//
//	RETURN
//		true
//			ある
//		false
//			ない
//
//	EXCEPTIONS

bool
_Transaction::isOverlapped(Schema::ObjectID::Value db,
						   Trans::TimeStamp::Value t,
						   const ModVector<Trans::Transaction::ID>& ids,
						   Trans::TimeStamp& start)
{
	bool	overlapped = false;

	// 現在実行中の版管理するトランザクションを求める
	//
	//【注意】	実行中トランザクションリスト内のトランザクション識別子は、
	//			それの表すトランザクションの開始時刻の昇順に並んでいる

	const Trans::List<Trans::Transaction>& list =
		Trans::Transaction::getInProgressList(db, false);

	Os::AutoCriticalSection	latch(list.getLatch());

	if (list.getSize()) {

		// 現在実行中の版管理するトランザクションごとに調べる

		Trans::List<Trans::Transaction>::ConstIterator
			ite(list.begin());
		const Trans::List<Trans::Transaction>::ConstIterator&
			end = list.end();

		do {
			const Trans::Transaction& trans = *ite;

			if (t < trans.getBirthTimeStamp())
				if (trans.isOverlapped(ids))
				{

					// 指定されたタイムスタンプ以降で、
					// 指定された更新トランザクションがすべて終了するまでに
					// 開始された版管理するトランザクションが見つかった

					overlapped = true;

					// この版管理するトランザクションは最新の版を参照しては
					// ならないので、トランザクションが開始したタイムスタンプ
					// より大きくする
					//
					// 【注意】
					//		実際設定されるときは、デクリメントされる
					
					start = trans.getBirthTimeStamp();
					++start;
				}
				else
				{
					// 指定されたタイムスタンプ以降で、
					// 指定された更新トランザクションがすべて終了してから
					// 最初に開始された版管理するトランザクションが見つかった

					start = trans.getBirthTimeStamp();
					break;
				}
		} while (++ite != end) ;
	}

	return overlapped;
}

//
//	FUNCTION public
//	KdTree::KdTreeIndexSet::KdTreeIndexSet -- コンストラクタ
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
KdTreeIndexSet::KdTreeIndexSet()
	: m_cMainList(&KdTreeIndex::m_pPrev, &KdTreeIndex::m_pNext),
	  m_cSmall1List(&KdTreeIndex::m_pPrev, &KdTreeIndex::m_pNext),
	  m_cSmall2List(&KdTreeIndex::m_pPrev, &KdTreeIndex::m_pNext),
	  m_bLoaded(false), m_iDimension(0)
{
}

//
//	FUNCTION public
//	KdTree::KdTreeIndexSet::~KdTreeIndexSet -- デストラクタ
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
KdTreeIndexSet::~KdTreeIndexSet()
{
	freeList(m_cMainList);
	freeList(m_cSmall1List);
	freeList(m_cSmall2List);
}

//
//	FUNCTION public static
//	KdTree::KdTreeIndexSet::initialize -- 初期化する
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
KdTreeIndexSet::initialize()
{
}

//
//	FUNCTION public static
//	KdTree::KdTreeIndexSet::terminate -- 後処理する
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
KdTreeIndexSet::terminate()
{
	Os::AutoCriticalSection cAuto(_cLatch);

	// メモリ上にロードしてあるすべての索引を delete する
	ModHashMap<ModUnicodeString, KdTreeIndexSet*,
			   ModHasher<ModUnicodeString> >::Iterator i =
		_cKdTreeIndexSetHashMap.begin();
	for (; i != _cKdTreeIndexSetHashMap.end(); ++i)
	{
		delete (*i).second;
	}
	_cKdTreeIndexSetHashMap.clear();
}

//
//	FUNCTION public static
//	KdTree::KdTreeIndexSet::attach -- KD-Tree索引を得る
//
//	NOTES
//
//	ARGUMENTS
//	FileID& cFileID_
//		ファイルID
//
// 	RETURN
//	KdTree::KdTreeIndexSet*
//		KD-Tree索引
//
//	EXCEPTIONS
//
KdTreeIndexSet*
KdTreeIndexSet::attach(const Trans::Transaction& cTransaction_,
					   FileID& cFileID_)
{
	KdTreeIndexSet* pIndexSet = 0;
	
	{
		Os::AutoCriticalSection cAuto(_cLatch);

		// パスで探す
		ModUnicodeString cPath = cFileID_.getPath();	
		ModHashMap<ModUnicodeString, KdTreeIndexSet*,
			ModHasher<ModUnicodeString> >::Iterator i
			= _cKdTreeIndexSetHashMap.find(cPath);

		if (i == _cKdTreeIndexSetHashMap.end())
		{
			// まだ存在していない

			pIndexSet = new KdTreeIndexSet;
			_cKdTreeIndexSetHashMap.insert(cPath, pIndexSet);
		}
		else
		{
			// 存在している

			pIndexSet = (*i).second;
		}
	}

	// 索引セットをロックする
	
	Os::AutoCriticalSection cAuto2(pIndexSet->getLatch());

	if (pIndexSet->isLoaded() == false)
	{
		// まだ索引がロードされていないので、ロードする

		pIndexSet->load(cTransaction_, cFileID_);
	}

	return pIndexSet;
}

//
//	FUNCTION public static
//	KdTree::KdTreeIndexSet::drop -- 索引をメモリ上から削除する
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeIndexSet::drop(FileID& cFileID_)
{
	Os::AutoCriticalSection cAuto(_cLatch);

	// 指定された索引を削除する
	
	ModUnicodeString cPath = cFileID_.getPath();
	ModHashMap<ModUnicodeString, KdTreeIndexSet*,
		ModHasher<ModUnicodeString> >::Iterator i
		= _cKdTreeIndexSetHashMap.find(cPath);
	if (i != _cKdTreeIndexSetHashMap.end())
	{
		delete (*i).second;
		_cKdTreeIndexSetHashMap.erase(i);
	}
}

//
//	FUNCTION public static
//	KdTree::KdTreeIndexSet::create -- 索引を作る
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const KdTree::FileID& cFileID_
//		ファイルID
//	KdTree::DataFile& cDataFile_
//		データファイル
//	KdTree::IndexFile& cIndexFile_
//		索引ダンプファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeIndexSet::create(const Trans::Transaction& cTransaction_,
					   const FileID& cFileID_,
					   DataFile& cDataFile_,
					   IndexFile& cIndexFile_,
					   KdTreeIndex::Direction& cDirection_)
{
	//【注意】	索引の作成中は、上位層でラッチされているので、
	//			別のスレッドが同じ索引を同時に作成することは想定しない
	
	//【注意】	引数の cDataFile_ と cIndexFile_ は、
	//			適切なモードで open されている必要がある
	
	ModAutoPointer<KdTreeIndex> pIndex
		= new KdTreeIndex(cFileID_.getDimension(),
						  Trans::TimeStamp::assign());
	pIndex->m_vecModifierList.pushBack(cTransaction_.getID());

	// データファイルを読み込み、索引を作成する
	
	pIndex->create(cDataFile_, cDirection_, false);

	if (cDirection_.isAbort())
		_SYDNEY_THROW0(Exception::Cancel);

	// サイズをログに出力する
	ModUnicodeString cPath = cFileID_.getPath();
	SydMessage << "KDTree (" << cPath << ") Size : "
			   << (pIndex->getSize() >> 20) << " MB" << ModEndl;

	// 永続化する

	pIndex->dump(cIndexFile_);
	
	{
		// 索引を管理しているハッシュマップに
		// 作成した索引を登録する

		Os::AutoCriticalSection cAuto(_cLatch);

		KdTreeIndexSet* pIndexSet = 0;
		
		ModHashMap<ModUnicodeString, KdTreeIndexSet*,
			ModHasher<ModUnicodeString> >::Iterator i
			= _cKdTreeIndexSetHashMap.find(cPath);

		if (i == _cKdTreeIndexSetHashMap.end())
		{
			// まだ存在していない

			pIndexSet = new KdTreeIndexSet;
			_cKdTreeIndexSetHashMap.insert(cPath, pIndexSet);
		}
		else
		{
			// 存在している

			pIndexSet = (*i).second;
		}

		// 索引セットをロックする
	
		Os::AutoCriticalSection cAuto2(pIndexSet->getLatch());
		
		pIndexSet->setIndex(pIndex.release());
	}
}

//
//	FUNCTION public
//	KdTreeIndexSet::attachMain -- 検索用の索引を得る
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		参照するトランザクション
//
//	RETURN
//	const KdTree::KdTreeIndex*
//		検索用の索引
//
//	EXCEPTIONS
//
const KdTreeIndex*
KdTreeIndexSet::attachMain(const Trans::Transaction& cTransaction_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	
	const KdTreeIndex* pIndex = traverseIndex(cTransaction_, m_cMainList);
	if (pIndex && pIndex->isEmpty())
		pIndex = 0;

	return pIndex;
}

//
//	FUNCTION public
//	KdTreeIndexSet::attachLog1 -- 検索用の索引を得る
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		参照するトランザクション
//
//	RETURN
//	const KdTree::KdTreeIndex*
//		検索用の索引
//
//	EXCEPTIONS
//
const KdTreeIndex*
KdTreeIndexSet::attachLog1(const Trans::Transaction& cTransaction_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	
	const KdTreeIndex* pIndex = traverseIndex(cTransaction_, m_cSmall1List);
	if (pIndex && pIndex->isEmpty())
		pIndex = 0;

	return pIndex;
}

//
//	FUNCTION public
//	KdTreeIndexSet::attachLog2 -- 検索用の索引を得る
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		参照するトランザクション
//
//	RETURN
//	const KdTree::KdTreeIndex*
//		検索用の索引
//
//	EXCEPTIONS
//
const KdTreeIndex*
KdTreeIndexSet::attachLog2(const Trans::Transaction& cTransaction_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	
	return traverseIndex(cTransaction_, m_cSmall2List);
}

//
//	FUNCTION public
//	KdTreeIndexSet::allocateLog1 -- 更新用の索引を得る
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		更新するトランザクション
//
//	RETURN
//	KdTree::KdTreeIndex*
//		検索用の索引
//
//	EXCEPTIONS
//
KdTreeIndex*
KdTreeIndexSet::allocateLog1(const Trans::Transaction& cTransaction_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	
	return allocateIndex(cTransaction_, m_cSmall1List);
}

//
//	FUNCTION public
//	KdTreeIndexSet::allocateLog2 -- 更新用の索引を得る
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		更新するトランザクション
//
//	RETURN
//	KdTree::KdTreeIndex*
//		検索用の索引
//
//	EXCEPTIONS
//
KdTreeIndex*
KdTreeIndexSet::allocateLog2(const Trans::Transaction& cTransaction_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	
	return allocateIndex(cTransaction_, m_cSmall2List);
}

//
//	FUNCTION public
//	KdTreeIndexSet::discard -- 不要な版を削除する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		不要な版が存在しなくなった場合はtrue、まだ存在している場合はfalse
//
//	EXCEPTIONS
//
bool
KdTreeIndexSet::discard()
{
	bool result = true;
	
	Os::AutoCriticalSection cAuto(m_cLatch);

	// オーバーラップしているトランザクションの内、
	// 最初に開始したトランザクションが参照する版より古い版は削除する

	Trans::TimeStamp t
		= Trans::Transaction::getBeginningID(m_cLockName.getDatabasePart());

	// 不要な版を消す
	
	result = ((discardIndex(t, m_cSmall1List) == false) ? false : result);
	result = ((discardIndex(t, m_cSmall2List) == false) ? false : result);
	result = ((discardIndex(t, m_cMainList) == false) ? false : result);

	return result;
}

//
//	FUNCTION private
//	KdTreeIndexSet::traverseIndex -- 検索用の索引を得る
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		参照するトランザクション
//	Common::DoubleLinkedList<KdTreeIndex>& cList_
//		探索するリスト
//
//	RETURN
//	KdTree::KdTreeIndex*
//		メイン索引。参照すべき索引がない場合は0を返す
//
//	EXCEPTIONS
//
const KdTreeIndex*
KdTreeIndexSet::traverseIndex(const Trans::Transaction& trans_,
							  Common::DoubleLinkedList<KdTreeIndex>& cList_)
{
	//【注意】	呼び出し元で必要に応じてロックすること
	
	if (cList_.getSize() == 0)
		// 空なので、参照すべき版はない
		return 0;

	Common::DoubleLinkedList<KdTreeIndex>::Iterator i = cList_.begin();
	
	//【注意】	基本的には Version::VersionLog::traverseLog と同じ

	//	版管理しないトランザクションは、常に最新版
	//
	//	版管理するトランザクションは以下の通り
	//
	//	トランアクションの開始時点より前に更新された場合で、かつ、
	//	更新したトランザクションが、版管理するトランザクションの開始時点で、
	//	実行されていなかったら、最新版
	//
	//	そうじゃなかったら、最新版以外で、
	//	トランアクションの開始時点より前に更新されたものを探す
	//	該当するものがなければ 0 を返す

	if (trans_.isNoVersion())
		
		// 版管理しないトランザクションは、常に最新版を参照する
		
		return &(*i);

	if (trans_.getBirthTimeStamp() > (*i).m_cTimeStamp)
	{
		// 版管理するトランザクションを開始してから最新版は更新されていない

		if (trans_.getStartingList().getSize())
		{
			// 版管理するトランザクションの開始時に
			// 実行されていた更新トランザクションのうち、
			// 最新版を更新したものがないか調べる

			if (trans_.isOverlapped((*i).m_vecModifierList))

				// 参照すべき版は、最新版ではない

				goto older;
		}

		// 参照すべき版は最新版である

		return &(*i);
	}

 older:

	// 1つ古い版にする
	++i;

	while (i != cList_.end())
	{
		// 版管理するトランザクションの開始時タイムスタンプより
		// 今調べている版の最終更新時タイムスタンプの方が小さいか調べる

		if (trans_.getBirthTimeStamp() > (*i).m_cTimeStamp)

			// 参照すべき版が見つかった

			return &(*i);

		// さらに古い版を調べる
		
		++i;
	}

	// 参照すべき版が見つからなかった

	return 0;
}

//
//	FUNCTION private
//	KdTree::KdTreeIndexSet::allocateIndex -- 更新用の索引を得る
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& trans_
//		トランザクション
//	Common::DoubleLinkedList<KdTreeIndex>& list_
//		索引リスト
//
//	RETURN
//	KdTree::KdTreeIndex*
//		更新用の索引
//		ただし、新しい版の場合、索引は空なので、
//		呼び出し元で create する必要がある
//
//	EXCEPTIONS
//
KdTreeIndex*
KdTreeIndexSet::allocateIndex(const Trans::Transaction& trans_,
							  Common::DoubleLinkedList<KdTreeIndex>& list_)
{
	//【注意】	呼び出し元で必要に応じてロックすること

	if (list_.getSize() == 0)
	{
		// 索引がないので、ここでインスタンスを確保して返す

		ModAutoPointer<KdTreeIndex> p
			= new KdTreeIndex(m_iDimension,
							  Trans::TimeStamp::assign());

		list_.pushFront(*p.release());

		return p.get();
	}

	// 最新版を得る

	KdTreeIndex* pIndex = &(list_.getFront());

	//【注意】	基本的には Version::VersionLog::allocateLog と同じ
	
	if (pIndex->m_eStatus == KdTreeIndex::Status::Copy)

		// 現在の最新版は、同じイメージが直前の版に存在しているので、
		// 新しい最新版を生成する必要はない

		return pIndex;

	bool inProgress = false;
	bool overlapped = false;
	Trans::TimeStamp start;

	if (pIndex->m_vecModifierList.getSize())
	{
		// 最新版は生成されてから更新されたことがある

		// 最新版を更新した更新トランザクションが実行中か調べる
		// ⇒ 実行中であれば、現在の最新版をそのまま返す

		inProgress = Trans::Transaction::isInProgress(
			m_cLockName.getDatabasePart(),
			pIndex->m_vecModifierList, Trans::Transaction::Category::ReadWrite);

		if (inProgress)

			// 実行中なので、そのまま返す

			return pIndex;

		// 最新版を更新した更新トランザクションが終了していて、
		// 最新版を最後に更新してから、
		// 最新版を更新した更新トランザクションがすべて終了するまでに
		// 開始された版管理するトランザクションのうち、
		// 現在実行中のものがあるか調べる
		//
		// また、最新版の更新した更新トランザクションが
		// すべて終了してから開始された版管理するトランザクションがあるか
		// 調べ、あれば、その中で一番最初に開始されたものの
		// 開始時のタイムスタンプを求める

		overlapped = _Transaction::isOverlapped(
			m_cLockName.getDatabasePart(),
			pIndex->m_cTimeStamp, pIndex->m_vecModifierList, start);
		
	}
	
	// ここから先は、新しい版が必要になる

	if (overlapped)
	{
		// 最新版を更新した更新トランザクションは終了しており、
		// 最新版を更新した更新トランザクションがすべて終了するまでに
		// 開始された版管理するトランザクションは実行中である

		// 現在の最新版は、最新版を更新した更新トランザクションが
		// 終了してから開始された版管理するトランザクションから
		// 参照されるように、最終更新時タイムスタンプ値を変更する

		--start;
		pIndex->m_cTimeStamp = start;
		pIndex->m_vecModifierList.clear();
	}
	else
	{
		// 最新版を更新した更新トランザクションは終了しており、
		// 最新版を最後に更新してから、
		// 最新版を更新した更新トランザクションがすべて終了するまでに
		// 開始された版管理するトランザクションはすべて終了している

		// タイムスタンプ値はそのまま

		pIndex->m_vecModifierList.clear();
	}

	// オーバーラップしているトランザクションの内、
	// 最初に開始したトランザクションが参照する版より古い版は削除する

	Trans::TimeStamp t
		= Trans::Transaction::getBeginningID(m_cLockName.getDatabasePart());
	discardIndex(t, list_);

	// 新しい版を作成する

	pIndex = new KdTreeIndex(m_iDimension, Trans::TimeStamp::assign());
	pIndex->m_vecModifierList.pushBack(trans_.getID());
	list_.pushFront(*pIndex);

	return pIndex;
}

//
//	FUNCTION private
//	KdTree::KdTreeIndexSet::discardIndex -- 不要な索引を削除する
//
//	NOTES
//
//	ARGUMENTS
//	Common::DoubleLinkedList<KdTreeIndex>& list_
//		索引リスト
//
//	RETURN
//	bool
//		版が1つ以下の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
KdTreeIndexSet::discardIndex(const Trans::TimeStamp& cTimeStamp_,
							 Common::DoubleLinkedList<KdTreeIndex>& list_)
{
	if (list_.getSize() <= 1)
		
		// 最新版しかないので、調べる必要なし
		
		return true;
	
	Common::DoubleLinkedList<KdTreeIndex>::Iterator i = list_.begin();

	if ((*i).m_vecModifierList.getSize())
	{
		// 最新版を更新したトランザクションが実行中か調べる

		if (Trans::Transaction::isInProgress(
				m_cLockName.getDatabasePart(),
				(*i).m_vecModifierList,
				Trans::Transaction::Category::ReadWrite))
		{
			// 実行中のトランザクションがあるので、
			// １つ古い版から調べる

			++i;
		}
		else
		{
			// 実行中のトランザクションはない
			
			(*i).m_vecModifierList.clear();
		}
	}

	while (i != list_.end())
	{
		// 指定されたタイムスタンプより小さいタイムスタンプを持つ版が
		// 見つかったら、それ以降の版は不要となる
		
		// 指定されたタイムスタンプが不正なタイムスタンプだった場合は、
		// 最新版のみ残し、それ以降の版は不要となる

		if (cTimeStamp_.isIllegal() ||
			(*i).m_cTimeStamp < cTimeStamp_)
		{
			// 参照すべき版が見つかった
			// この版より古い版は不要

			++i;
			break;
		}

		++i;
	}

	while (i != list_.end())
	{
		// 古い版を削除する
		
		KdTreeIndex* p = &(*i);
		++i;	// 削除する前に次に進める
		
		list_.erase(*p);
		delete p;
	}

	return (list_.getSize() <= 1) ? true : false;
}

//
//	FUNCTION private
//	KdTree::KdTreeIndexSet::setIndex -- メイン索引を設定する
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::KdTreeIndex* pIndex_
//		索引
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeIndexSet::setIndex(KdTreeIndex* pIndex_)
{
	m_cMainList.pushFront(*pIndex_);

	m_iDimension = pIndex_->getDimension();
	m_bLoaded = true;
}

//
//	FUNCTION private
//	KdTree::KdTreeIndexSet::load -- 索引をロードする
//
//	NOTES
//
//	ARGUMETNS
//	KdTree::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeIndexSet::load(const Trans::Transaction& cTransaction_,
					 FileID& cFileID_)
{
	//【注意】	初めてファイルにアクセスされたということは、
	//			システムが起動されてから一度も更新されていない
	//			ということなので、
	//			タイムスタンプはシステムが初期化されたときのものとする
	
	ModAutoPointer<KdTreeIndex> pIndex;

	{
		//
		// 本体の索引をロードする
		// 本体の索引はダンプファイルがあるので、そこからロードする
		//

		IndexFile cIndexFile(cFileID_);

		if (cIndexFile.isMounted(cTransaction_))
		{
			// ファイルが存在するので、ロードする
		
			cIndexFile.open(cTransaction_,
							Buffer::Page::FixMode::ReadOnly);
			try
			{
				// インスタンスを確保する
		
				pIndex
					= new KdTreeIndex(cFileID_.getDimension(),
									  Trans::TimeStamp::getSystemInitialized());

				// ロードする
		
				pIndex->load(cIndexFile);
			}
			catch (...)
			{
				cIndexFile.close();
				_SYDNEY_RETHROW;
			}
			cIndexFile.close();

			// リストに追加する
		
			m_cMainList.pushFront(*(pIndex.release()));
			
			// サイズをログに出力する
			
			ModUnicodeString cPath = cFileID_.getPath();
			SydMessage << "KDTree (" << cPath << ") Size : "
					   << (pIndex->getSize() >> 20) << " MB" << ModEndl;

		}

	}

	{
		// 小さい索引1を作る
		
		Os::Path path = cFileID_.getPath();
		path.addPart(cFileID_.getSmallPath1());

		BtreeDataFile cDataFile(cFileID_, path);
		cDataFile.open(cTransaction_,
					   Buffer::Page::FixMode::ReadOnly);

		try
		{
			if (cDataFile.getCount() != 0)
			{
				// 0件ではないので、索引を作成する

				pIndex
					= new KdTreeIndex(cFileID_.getDimension(),
									  Trans::TimeStamp::getSystemInitialized());

				// 作成する

				pIndex->create(cDataFile);
				
				// リストに追加する
		
				m_cSmall1List.pushFront(*(pIndex.release()));
			}
		}
		catch (...)
		{
			cDataFile.close();
			freeList(m_cMainList);
			_SYDNEY_RETHROW;
		}
		cDataFile.close();
	}
				
	{
		// 小さい索引2を作る
		
		Os::Path path = cFileID_.getPath();
		path.addPart(cFileID_.getSmallPath2());

		BtreeDataFile cDataFile(cFileID_, path);
		cDataFile.open(cTransaction_,
					   Buffer::Page::FixMode::ReadOnly);

		try
		{
			if (cDataFile.getCount() != 0)
			{
				// 0件ではないので、索引を作成する

				pIndex
					= new KdTreeIndex(cFileID_.getDimension(),
									  Trans::TimeStamp::getSystemInitialized());

				// 作成する

				pIndex->create(cDataFile);
				
				// リストに追加する
		
				m_cSmall1List.pushFront(*(pIndex.release()));
			}
		}
		catch (...)
		{
			cDataFile.close();
			freeList(m_cMainList);
			freeList(m_cSmall1List);
			_SYDNEY_RETHROW;
		}
		cDataFile.close();
	}

	m_iDimension = cFileID_.getDimension();
	m_cLockName = cFileID_.getLockName();
	m_bLoaded = true;
}

//
//	FUNCTION private
//	KdTree::KdTreeIndexSet::freeList -- 索引リスト内の索引を解放する
//
//	NOTES
//
//	ARGUMETNS
//	Common::DoubleLinkedList<KdTreeIndex>& list_
//		解放するリスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeIndexSet::freeList(Common::DoubleLinkedList<KdTreeIndex>& list_)
{
	while (list_.getSize())
	{
		KdTreeIndex* p = &(list_.getFront());
		list_.popFront();
		delete p;
	}
}

//
//	Copyright (c) 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
