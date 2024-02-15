// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VersionLog.cpp -- バージョンログファイル関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Version";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"
#include "SyFor.h"

#include "Version/Configuration.h"
#include "Version/File.h"
#include "Version/HashTable.h"
#include "Version/MasterData.h"
#include "Version/Message_AllocationBitInconsistent.h"
#include "Version/Message_BlockCountInconsistent.h"
#include "Version/Message_ChildCountInconsistent.h"
#include "Version/Message_LatestCountInconsistent.h"
#include "Version/Message_OlderNotIdentical.h"
#include "Version/Message_OlderTimeStampInconsistent.h"
#include "Version/Message_OldestTimeStampInconsistent.h"
#include "Version/Message_PhysicalLogIDInconsistent.h"
#include "Version/Message_PreservedDifferentPage.h"
#include "Version/Message_VersionLogFileNotFound.h"
#include "Version/Message_VersionLogIDInconsistent.h"
#include "Version/Message_VersionPageCountInconsistent.h"
#include "Version/Page.h"
#include "Version/PathParts.h"
#include "Version/Verification.h"
#include "Version/VersionLog.h"

#include "Buffer/AutoPool.h"
#include "Buffer/File.h"
#include "Checkpoint/TimeStamp.h"
#include "Common/Assert.h"
#include "Common/BitSet.h"
#include "Common/Message.h"
#include "Exception/BadDataPage.h"
#include "Lock/Name.h"
#include "Os/AutoCriticalSection.h"
#include "Trans/List.h"
#include "Trans/Transaction.h"

#include "Exception/PreservedDifferentPage.h"

#include "ModAlgorithm.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_VERSION_USING
_SYDNEY_VERSION_VERSIONLOG_USING

namespace
{

namespace _File
{
	// 格納する OS ファイルの絶対パス名を得る
	Os::Path				getPath(const Os::Path& parent);

	// 多重化されたブロックのどれを選択するかを決めるための情報を
	// すべて管理するハッシュ表に登録するためのハッシュ値を計算する
	unsigned int			infoTableHash(Block::ID id);
}

namespace _AllocationTable
{
	// あるブロックサイズのブロックに
	// アロケーションテーブルがあるときの全ビットマップの長さを求める
	unsigned int			getBitmapLength(VersionNumber::Value v,
											Os::Memory::Size size);
	// あるブロックサイズのブロックに
	// アロケーションテーブルがあるときの全ビットマップのサイズを求める
	Os::Memory::Size		getBitmapSize(VersionNumber::Value v,
										  Os::Memory::Size size);
	// あるブロックサイズのブロックに
	// アロケーションテーブルがあるときの全ビットマップのビット数を求める
	unsigned int			getBitCount(VersionNumber::Value v,
										Os::Memory::Size size);
	// あるブロックサイズのバージョンログファイルで、
	// 与えられたブロック識別子のブロックを管理する多重化された
	// アロケーションテーブルのうち、最初のもののブロック識別子を求める
	Block::ID				roundBlockID(VersionNumber::Value v,
										 Os::Memory::Size size, Block::ID id);
	// あるブロックサイズのバージョンログファイルで、
	// 与えられたブロック識別子のブロックを表すビットは、
	// アロケーションテーブルの先頭からなん番目のビットマップのものか調べる
	unsigned int			blockIDToBitmapIndex(
								VersionNumber::Value v,
								Os::Memory::Size size, Block::ID id);
	// あるブロックサイズのバージョンログファイルで、
	// 与えられたブロック識別子のブロックを表すビットは、
	// アロケーションテーブル中の
	// あるビットマップの先頭からなん番目のビットか調べる
	unsigned int			blockIDToBitIndexOfBitmap(
								VersionNumber::Value v,
								Os::Memory::Size size, Block::ID id);
	// あるブロックサイズのバージョンログファイルで、
	// 与えられたブロック識別子のブロックを表すビットは、
	// アロケーションテーブルの先頭からなん番目のビットか調べる
	unsigned int			blockIDToBitIndex(
								VersionNumber::Value v,
								Os::Memory::Size size, Block::ID id);

	// 1 ビットマップあたりのビット数
	const unsigned int		_bitCountPerBitmap = 8 * sizeof(unsigned int);
	
	//	1 バイト中の1が立っているビットの数を引くテーブル
	const ModSize _bitCountTable[] =
	{
		0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
		1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
		1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
		3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
	};
}

namespace _PBCTNode
{
	// あるブロックサイズの PBCT ノードの
	// 子ノードのブロック識別子を記録する配列において、
	// 与えられたバージョンページ識別子に関する要素の番号を求める
	unsigned int
	pageIDToArrayIndex(Page::ID pageID, PBCTLevel::Value current,
					   PBCTLevel::Value level, Os::Memory::Size size);
}

namespace _PBCTLeaf
{
	// あるブロックサイズの PBCT リーフの
	// 最新版のブロック識別子または最古版のタイムスタンプ値を
	// それぞれ記録する配列において、
	// 与えられたバージョンページ識別子に関する要素の番号を求める
	unsigned int
	pageIDToArrayIndex(Page::ID pageID, Os::Memory::Size size);
}

namespace _Log
{
	// 不正なブロック識別子か
	bool
	isIllegalID(Block::ID id);
}

namespace _Transaction
{
	// 現在実行中の版管理するトランザクションのうち、
	// あるタイムスタンプ以降で、指定された更新トランザクションが
	// すべて終了するまでに開始されたものがあるか調べる
	bool
	isOverlapped(Schema::ObjectID::Value dbID,
				 Trans::TimeStamp::Value t,
				 const ModVector<Trans::Transaction::ID>& ids,
				 Trans::TimeStamp& start);

	// 版管理するトランザクションが
	// あるバージョンページの最新版を参照するか調べる
	bool
	isRefered(const Trans::Transaction& trans,
			  Trans::TimeStamp::Value t,
			  const ModVector<Trans::Transaction::ID>& ids);
	// 現在実行中の版管理するトランザクションのうち、
	// あるバージョンページの最新版を参照するものがあるか調べる
	bool
	isRefered(Schema::ObjectID::Value dbID,
			  Trans::TimeStamp::Value t,
			  const ModVector<Trans::Transaction::ID>& ids);
	// 現在実行中の版管理するトランザクションのうち、
	// あるバージョンページの 2 番目に新しい版を参照するものがあるか調べる
	bool
	isRefered(Schema::ObjectID::Value dbID,
			  Trans::TimeStamp::Value t0, Trans::TimeStamp::Value t1,
			  const ModVector<Trans::Transaction::ID>& ids);
	
	// 最新版を更新した更新トランザクションが実行中か調べる
	// また、自身以外の更新トランザクションで実行中のものがあった場合、
	// 更新した更新トランザクションがすべて終了するまで、
	// 版管理するトランザクションの生成を抑制する必要があるので、
	// そのためのフラグを更新トランザクションにセットする
	bool
	isInProgress(const Trans::Transaction& trans_,
				 const ModVector<Trans::Transaction::ID>& ids_,
				 bool afterMostRecent_);
}

namespace _MultiplexInfo
{
	// 生成済の多重化されたブロックの
	// どれを選択すべきか決めるための情報を表すクラスを探す
	MultiplexInfo*
	find(HashTable<MultiplexInfo>::Bucket& bucket, Block::ID id);
}

namespace _Math
{
	// doubleのpowだと丸め誤差が発生する場合があるので、
	// unsigned int のpowを作成する
	unsigned int pow(unsigned int x, unsigned int y);
}

//	FUNCTION
//	$$$::_File::getPath --
//		バージョンログファイルを格納する OS ファイルの絶対パス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			parent
//			バージョンログファイルを格納する OS ファイルの
//			親ディレクトリの絶対パス名
//
//	RETURN
//		得られた絶対パス名
//
//	EXCEPTIONS

inline
Os::Path
_File::getPath(const Os::Path& parent)
{
	return Os::Path(parent).addPart(PathParts::VersionLog);
}

//	FUNCTION
//	$$$::_File::infoTableHash --
//		あるバージョンログファイルの多重化されたブロックの
//		どれを選択するかを決めるための情報をすべての管理する
//		ハッシュ表に登録するためにハッシュ値を計算する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::ID	id
//			多重化されたブロックのうち、先頭のもののブロック識別子
//
//	RETURN
//		得られたハッシュ値
//
//	EXCEPTIONS
//		なし

inline
unsigned int
_File::infoTableHash(Block::ID id)
{
	// 指定されたブロック識別子をそのまま返す

	return static_cast<unsigned int>(id);
}

//	FUNCTION
//	$$$::_AllocationTable::getBitmapLength --
//		あるブロックサイズのブロックにアロケーションテーブルがあるとき、
//		全ビットマップの長さを求める
//
//	NOTES
//
//	ARGUMENTS
//		Version::VersionNumber::Value	v
//			バージョン
//		Os::Memory::Size	size
//			全ビットマップの長さを求めるアロケーションテーブルが
//			存在するブロックのブロックサイズ(B 単位)
//
//	RETURN
//		得られた全ビットマップの長さ
//
//	EXCEPTIONS
//		なし

inline
unsigned int
_AllocationTable::getBitmapLength(VersionNumber::Value v,
								  Os::Memory::Size size)
{
	return (Block::getContentSize(size) - sizeof(unsigned int)) /
		sizeof(unsigned int) / ((v < VersionNumber::Second) ? 1 : 2);
}

//	FUNCTION
//	$$$::_AllocationTable::getBitmapSize --
//		あるブロックサイズのブロックにアロケーションテーブルがあるとき、
//		全ビットマップのサイズを求める
//
//	NOTES
//
//	ARGUMENTS
//		Version::VersionNumber::Value	v
//			バージョン
//		Os::Memory::Size	size
//			全ビットマップのサイズを求めるアロケーションテーブルが
//			存在するブロックのサイズ(B 単位)
//
//	RETURN
//		得られた全ビットマップのサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

inline
Os::Memory::Size
_AllocationTable::getBitmapSize(VersionNumber::Value v,
								Os::Memory::Size size)
{
	return sizeof(unsigned int) * _AllocationTable::getBitmapLength(v, size);
}

//	FUNCTION
//	$$$::_AllocationTable::getBitCount --
//		あるブロックサイズのブロックにアロケーションテーブルがあるとき、
//		全ビットマップのビット数を求める
//
//	NOTES
//
//	ARGUMENTS
//		Version::VersionNumber::Value	v
//			バージョン
//		Os::Memory::Size	size
//			全ビットマップのビット数を求めるアロケーションテーブルが
//			存在するブロックのブロックサイズ(B 単位)
//
//	RETURN
//		得られた全ビットマップのビット数
//
//	EXCEPTIONS
//		なし

inline
unsigned int
_AllocationTable::getBitCount(VersionNumber::Value v, Os::Memory::Size size)
{
	return _AllocationTable::_bitCountPerBitmap *
		_AllocationTable::getBitmapLength(v, size);
}

//	FUNCTION
//	$$$::_AllocationTable::roundBlockID --
//		あるブロックサイズのバージョンログファイルで、
//		与えられたブロック識別子のブロックを管理する
//		多重化されたアロケーションテーブルのうち、
//		最初のもののブロック識別子を求める
//
//	NOTES
//		アロケーションテーブルのブロック識別子が与えられると、
//		それを含む多重化されたアロケーションテーブルの組の中の
//		最初のもののブロック識別子を求める
//
//		ファイルヘッダのブロック識別子が与えられたとき、
//		動作は保証しない
//
//	ARGUMENTS
//		Version::VersionNumber::Value	v
//			バージョン
//		Os::Memory::Size	size
//			調べるバージョンログファイルのブロックサイズ(B 単位)
//		Version::Block::ID	id
//			調べるブロック識別子
//
//	RETURN
//		得られたブロック識別子
//
//	EXCEPTIONS
//		なし

inline
Block::ID
_AllocationTable::roundBlockID(VersionNumber::Value v,
							   Os::Memory::Size size, Block::ID id)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID && id >= MultiplexCount);

	const unsigned int n = getBitCount(v, size);
	return (id - MultiplexCount) /
		(n + MultiplexCount) * (n + MultiplexCount) + MultiplexCount;
}

//	FUNCTION
//	$$$::_AllocationTable::blockIDToBitmapIndex --
//		あるブロックサイズのバージョンログファイルで、
//		与えられたブロック識別子のブロックを表すビットは、
//		アロケーションテーブルの先頭からなん番目のビットマップのものか調べる
//
//	NOTES
//		ファイルヘッダおよびアロケーションテーブルの
//		ブロック識別子が与えられたとき、動作は保証しない
//
//	ARGUMENTS
//		Version::VersionNumber::Value	v
//			バージョン
//		Os::Memory::Size	size
//			調べるバージョンログファイルのブロックサイズ(B 単位)
//		Version::Block::ID	id
//			調べるブロック識別子
//
//	RETURN
//		アロケーションテーブル中の先頭からなん番目のビットマップかを表す数
//
//	EXCEPTIONS
//		なし

inline
unsigned int
_AllocationTable::blockIDToBitmapIndex(VersionNumber::Value v,
									   Os::Memory::Size size, Block::ID id)
{
	// 与えられたブロック識別子のブロックを管理するビットが
	// アロケーションテーブルの先頭からなん番目のビットか求め、
	// それと 1 ビットマップあたりのビット数の商が求める値である

	return _AllocationTable::blockIDToBitIndex(v, size, id) /
		_AllocationTable::_bitCountPerBitmap;
}

//	FUNCTION
//	$$$::_AllocationTable::blockIDToBitIndexOfBitmap --
//		あるブロックサイズのバージョンログファイルで、
//		与えられたブロック識別子のブロックを表すビットは、
//		アロケーションテーブル中の
//		あるビットマップの先頭からなん番目のビットか調べる
//
//	NOTES
//		ファイルヘッダおよびアロケーションテーブルの
//		ブロック識別子が与えられたとき、動作は保証しない
//
//	ARGUMENTS
//		Version::VersionNumber::Value	v
//			バージョン
//		Os::Memory::Size	size
//			調べるバージョンログファイルのブロックサイズ(B 単位)
//		Version::Block::ID	id
//			調べるブロック識別子
//
//	RETURN
//		アロケーションテーブル中のあるビットマップの
//		先頭からなん番目のビットかを表す数
//
//	EXCEPTIONS
//		なし

inline
unsigned int
_AllocationTable::blockIDToBitIndexOfBitmap(VersionNumber::Value v,
											Os::Memory::Size size, Block::ID id)
{
	// 与えられたブロック識別子のブロックを管理するビットが
	// アロケーションテーブル中の先頭からなん番目のビットかもとめ、
	// それと 1 ビットマップあたりのビット数の余りが求める値である

	return _AllocationTable::blockIDToBitIndex(v, size, id) %
		_AllocationTable::_bitCountPerBitmap;
}

//	FUNCTION
//	$$$::_AllocationTable::blockIDToBitIndex --
//		あるブロックサイズのバージョンログファイルで、
//		与えられたブロック識別子のブロックを表すビットは、
//		アロケーションテーブルの先頭からなん番目のビットか調べる
//
//	NOTES
//		ファイルヘッダおよびアロケーションテーブルの
//		ブロック識別子が与えられたとき、動作は保証しない
//
//	ARGUMENTS
//		Version::VersionNumber::Value	v
//			バージョン
//		Os::Memory::Size	size
//			調べるバージョンログファイルのブロックサイズ(B 単位)
//		Version::Block::ID	id
//			調べるブロック識別子
//
//	RETURN
//		アロケーションテーブル中の先頭からなん番目のビットかを表す数
//		
//	EXCEPTIONS
//		なし

inline
unsigned int
_AllocationTable::blockIDToBitIndex(VersionNumber::Value v,
									Os::Memory::Size size, Block::ID id)
{
	// 与えられたブロック識別子のブロックを管理する
	// アロケーションテーブルが管理するブロックのうち、
	// 最も先頭に位置するもののブロック識別子を求め、
	// それを与えられたブロック識別子の差が求める値である

	return id - (_AllocationTable::roundBlockID(v, size, id) + MultiplexCount);
}

//	FUNCTION
//	$$$::_PBCTNode::pageIDToArrayIndex --
//		あるブロックサイズの PBCT ノードの
//		子ノードのブロック識別子を記録する配列において、
//		与えられたバージョンページ識別子に関する要素の番号を求める
//
//	NOTES
//
//	ARGUMENTS
//		Version::Page::ID	pageID
//			最新のバージョンを取得しようとしている
//			バージョンページのバージョンページ識別子
//		Version::VersionLog::PBCTLevel::Value	current
//			調べる PBCT ノードの深さ(0 以上)
//		Version::VersionLog::PBCTLevel::Value	level
//			調べる PBCT ノードを含む PBCT の深さ(1 以上)
//		Os::Memory::Size	size
//			調べる PBCT ノードが存在するブロックのサイズ(B 単位)
//
//	RETURN
//		子ノードのブロック識別子を記録する配列の要素番号
//
//	EXCEPTIONS
//		なし

inline
unsigned int
_PBCTNode::pageIDToArrayIndex(Page::ID pageID, PBCTLevel::Value current,
							  PBCTLevel::Value level, Os::Memory::Size size)
{
	; _SYDNEY_ASSERT(current != PBCTLevel::Illegal);
	; _SYDNEY_ASSERT(level != PBCTLevel::Illegal);
	; _SYDNEY_ASSERT(current < level);

	const unsigned int n = PBCTNode::getCountMax(true, size);
	const unsigned int l = PBCTLeaf::getCountMax(true, size);

	const unsigned int m =
		static_cast<unsigned int>(l * _Math::pow(n, level - current - 1));

	return (pageID % (m * ((current > 0) ? n :
						   PBCTNode::getCountMax(false, size)))) / m;
}

//	FUNCTION
//	$$$::_PBCTLeaf::pageIDToArrayIndex --
//		あるブロックサイズの PBCT リーフの
//		最新版のブロック識別子または最古版のタイムスタンプ値を
//		それぞれ記録する配列において、
//		与えられたバージョンページ識別子に関する要素の番号を求める
//
//	NOTES
//
//	ARGUMENTS
//		Version::Page::ID	pageID
//			最新のバージョンを取得しようとしている
//			バージョンページのバージョンページ識別子
//		Os::Memory::Size	size
//			調べる PBCT リーフが存在するブロックのサイズ(B 単位)
//
//	RETURN
//		最新版のブロック識別子または最古版のタイムスタンプ値を
//		記録する配列の要素番号
//
//	EXCEPTIONS
//		なし

inline
unsigned int
_PBCTLeaf::pageIDToArrayIndex(Page::ID pageID, Os::Memory::Size size) 
{
	//【注意】	リーフが PBCT のルートであるときと
	//			ないときを区別する必要はない

	return pageID % PBCTLeaf::getCountMax(true, size);
}

//	FUNCTION
//	$$$::_Log::isIllegalID -- バージョンログのブロック識別子として不正か
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::ID	id
//			調べるブロック識別子
//
//	RETURN
//		true
//			不正である
//		false
//			不正でない
//
//	EXCEPTIONS
//		なし

inline
bool
_Log::isIllegalID(Block::ID id)
{
	return id == Block::IllegalID || id < MultiplexCount * 2;
}

//	FUNCTION
//	$$$::_Transaction::isOverlapped --
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
_Transaction::isOverlapped(Schema::ObjectID::Value dbID,
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
		Trans::Transaction::getInProgressList(dbID, false);

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
					//		実際unfixされるときは、デクリメントされるので、
					//		開始時のタイムスタンプでunfixされる
					
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

//	FUNCTION
//	$$$::_Transaction::isRefered --
//		ある版管理するトランザクションが
//		あるバージョンページの最新版を参照するか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			あるバージョンページの最新版を参照するか調べる
//			版管理するトランザクションのトランザクション記述子
//		Trans::TimeStamp::Value	t
//			版管理するトランザクションが参照するか調べる
//			バージョンページの最終更新時タイムスタンプ値
//		ModVector<Trans::Transaction::ID>&	ids
//			版管理するトランザクションが参照するか調べる
//			最新版を更新した更新トランザクションの
//			トランザクション識別子を要素として持つリストで、
//			要素は格納するトランザクション識別子の表す
//			トランザクションの開始時刻の昇順に並んでいる必要がある
//
//	RETURN
//		true
//			参照する
//		false
//			参照しない
//
//	EXCEPTIONS

inline
bool
_Transaction::isRefered(const Trans::Transaction& trans,
						Trans::TimeStamp::Value t,
						const ModVector<Trans::Transaction::ID>& ids)
{
	; _SYDNEY_ASSERT(!trans.isNoVersion());
	; _SYDNEY_ASSERT(!Trans::TimeStamp::isIllegal(t));

	// 最新版を最後に更新してから版管理するトランザクションが開始され、
	// その開始時に実行されていた更新トランザクションのうち、
	// 最新版を更新したものがないか調べる

	return t < trans.getBirthTimeStamp() && !trans.isOverlapped(ids);
}

//	FUNCTION
//	$$$::_Transaction::isRefered --
//		現在実行中の版管理するトランザクションのうち、
//		あるバージョンページの最新版を参照するものがあるか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp::Value	t
//			最新版の最終更新時タイムスタンプ値
//		ModVector<Trans::Transaction::ID>&	ids
//			最新版を更新した更新トランザクションの
//			トランザクション識別子を要素として持つリストで、
//			要素は格納するトランザクション識別子の表す
//			トランザクションの開始時刻の昇順に並んでいる必要がある
//
//	RETURN
//		true
//			参照する版管理するトランザクションがある
//		false
//			参照する版管理するトランザクションがない
//
//	EXCEPTIONS

bool
_Transaction::isRefered(Schema::ObjectID::Value dbID,
						Trans::TimeStamp::Value t,
						const ModVector<Trans::Transaction::ID>& ids)
{
	; _SYDNEY_ASSERT(!Trans::TimeStamp::isIllegal(t));

	// 現在実行中の版管理するトランザクションを求める

	const Trans::List<Trans::Transaction>& list =
		Trans::Transaction::getInProgressList(dbID, false);

	Os::AutoCriticalSection latch(list.getLatch());

	if (list.getSize()) {

		// 現在実行中の版管理するトランザクションごとに調べる

		Trans::List<Trans::Transaction>::ConstIterator
			ite(list.begin());
		const Trans::List<Trans::Transaction>::ConstIterator&
			end = list.end();

		do {
			const Trans::Transaction& trans = *ite;
			; _SYDNEY_ASSERT(!trans.isNoVersion());

			if (isRefered(trans, t, ids))

				// 今調べている版管理するトランザクションは
				// あるバージョンページの最新版を参照する

				return true;

		} while (++ite != end) ;
	}

	return false;
}

//	FUNCTION
//	$$$::_Transaction::isRefered --
//		現在実行中の版管理するトランザクションのうち、
//		あるバージョンページの 2 番目に新しい版を参照するものがあるか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp::Value	t0
//			現在実行中の版管理するトランザクションのうち、
//			参照するものがあるか調べるあるバージョンページの
//			2 番目に新しい版の最終更新時タイムスタンプ値
//		Trans::TimeStamp::Value t1
//			最新版の最終更新時タイムスタンプ値
//		ModVector<Trans::Transaction::ID>&	ids
//			最新版を更新した更新トランザクションの
//			トランザクション識別子を要素として持つリストで、
//			要素は格納するトランザクション識別子の表す
//			トランザクションの開始時刻の昇順に並んでいる必要がある
//
//	RETURN
//		true
//			参照する版管理するトランザクションがある
//		false
//			参照する版管理するトランザクションがない
//
//	EXCEPTIONS

bool
_Transaction::isRefered(Schema::ObjectID::Value dbID,
						Trans::TimeStamp::Value t0,
						Trans::TimeStamp::Value t1,
						const ModVector<Trans::Transaction::ID>& ids)
{
	; _SYDNEY_ASSERT(!Trans::TimeStamp::isIllegal(t0));
	; _SYDNEY_ASSERT(!Trans::TimeStamp::isIllegal(t1));

	// 現在実行中の版管理するトランザクションを求める

	const Trans::List<Trans::Transaction>& list =
		Trans::Transaction::getInProgressList(dbID, false);

	Os::AutoCriticalSection	latch(list.getLatch());

	if (list.getSize()) {

		// 現在実行中の版管理するトランザクションごとに調べる

		Trans::List<Trans::Transaction>::ConstIterator
			ite(list.begin());
		const Trans::List<Trans::Transaction>::ConstIterator&
			end = list.end();

		do {
			const Trans::Transaction& trans = *ite;
			; _SYDNEY_ASSERT(!trans.isNoVersion());

			if (t0 < trans.getBirthTimeStamp())

				// 参照されるか調べている版を最後に更新した後で
				// 今調べている版管理するトランザクションが開始されている

				// 参照されるか調べている版の直後の版である最新版を、
				// 今調べている版管理するトランザクションが参照しないか調べる
				//
				//【注意】	もし、このトランザクションが
				//			最新版を参照するのであれば、
				//			このトランザクション以降に開始された
				//			トランザクションは調べるまでもなく最新版を参照する

				return !isRefered(trans, t1, ids);

		} while (++ite != end) ;
	}
	
	return false;
}

//	FUNCTION
//	$$$$::_Transaction::isInProgress --
//		最新版を更新した更新トランザクションが実行中か調べる
//
//	NOTES
//
//	ARGUMENTS
//		const Trans::Transaction& trans
//			ページを更新する更新トランザクション
//		const ModVector<Trans::Transaction::ID>& ids
//			最新版を更新した更新トランザクションのリスト
//		bool afterMostRecent_
//			最新版が直前のチェックポイント以降に更新されているか否か
//
//	RETURN
//		true
//			最新版を更新した更新トランザクションが実行中だった
//		false
//			実行中ではなかった
//
//	EXCEPTIONS

bool
_Transaction::isInProgress(const Trans::Transaction& trans_,
						   const ModVector<Trans::Transaction::ID>& ids_,
						   bool afterMostRecent_)
{
	bool result = false;
	
	if (ids_.getSize())
	{
		// 現在実行中の更新トランザクションを求める

		const Trans::List<Trans::Transaction>& list =
			Trans::Transaction::getInProgressList(
				trans_.getDatabaseID(),
				Trans::Transaction::Category::ReadWrite);

		bool deterrent = false;

		Os::AutoCriticalSection	latch(list.getLatch());

		if (list.getSize())
		{
			List<Trans::Transaction>::ConstIterator	ite0(list.begin());
			const List<Trans::Transaction>::ConstIterator& end0 = list.end();
			ModVector<Trans::Transaction::ID>::ConstIterator ite1(ids_.begin());
			const ModVector<Trans::Transaction::ID>::ConstIterator& end1
				= ids_.end();

			// 現在実行中の更新トランザクションの中に、
			// この版を更新した更新トランザクションがいないか確認する
			// 自身以外の更新トランザクションがいた場合には、
			// それらのトランザクションが終了した後、
			// 版管理トランザクションを抑制する必要あり
				
			do {
				if ((*ite0).getID() == *ite1)
				{
					result = true;

					if (*ite1 != trans_.getID())
					{
						deterrent = true;
						break;
					}
				}
			} while (((*ite0).getID() < *ite1) ?
					 (++ite0 != end0) : (++ite1 != end1)) ;

			if (afterMostRecent_ && deterrent)
			{
				// 版管理トランザクションを抑制する必要があるので、
				// 最新版を更新したすべての更新トランザクションに
				// フラグを設定する

				ite0 = list.begin();
				ite1 = ids_.begin();
				
				do {
					if ((*ite0).getID() == *ite1)
					{
						(*ite0).setUpdatedSameVersion();
					}
				} while (((*ite0).getID() < *ite1) ?
						 (++ite0 != end0) : (++ite1 != end1)) ;

				// これから更新するトランザクションにもフラグを設定する

				trans_.setUpdatedSameVersion();
			}
		}
	}

	return result;
}

//	FUNCTION
//	$$$::_MultiplexInfo::find --
//		生成済の多重化されたブロックの
//		どれを選択すべきか決めるための情報を表すクラスを探す
//
//	NOTES
//
//	ARGUMENTS
//		Version::HashTable<VersionLog::MultiplexInfo>::Bucket&	bucket
//			多重化されたブロックの
//			どれを選択すべきか決めるための情報を表すクラスが
//			格納されるべきハッシュ表のバケット
//		Version::Block::ID	id
//			このブロック識別子を表すブロックを先頭にして多重化されたものの
//			どれを選択すべきかを決めるための情報を表すクラスを探す
//
//	RETURN
//		0 以外の値
//			得られたクラスを格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPTIONS

MultiplexInfo*
_MultiplexInfo::find(HashTable<MultiplexInfo>::Bucket& bucket, Block::ID id)
{
	//【注意】	呼び出し側で bucket.getLatch() をラッチする必要がある

	switch (bucket.getSize()) {
	default:
	{
		// バケットに登録されているクラスのうち、
		// 与えられたブロック識別子に関するものを探す

		HashTable<MultiplexInfo>::Bucket::Iterator	begin(bucket.begin());
		HashTable<MultiplexInfo>::Bucket::Iterator	ite(begin);
		const HashTable<MultiplexInfo>::Bucket::Iterator&	end = bucket.end();

		do {
			MultiplexInfo& info = *ite;

			if (info.getID() == id) {

				// 見つかったクラスをバケットの先頭に移動して、
				// 最近に参照されたものほど、見つかりやすくする

				bucket.splice(begin, bucket, ite);

				return &info;
			}
		} while (++ite != end) ;

		break;
	}
	case 1:
	{
		MultiplexInfo& info = bucket.getFront();

		if (info.getID() == id)

			// 見つかった

			return &info;

		break;
	}
	case 0:
		break;
	}

	// 見つからなかった

	return 0;
}

//	FUNCTION
//	$$$::_Math::pow -- x の y 乗を計算する
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int	x
//			底
//		unsigned int	y
//			指数
//
//	RETURN
//		x の y 乗
//
//	EXCEPTIONS
//		なし

unsigned int
_Math::pow(unsigned int x, unsigned int y)
{
	unsigned int n = (y == 0) ? 1 : x;
	for (unsigned int i = 1; i < y; ++i)
		n *= x;
	return n;
}

}

//	FUNCTION public
//	Version::VersionLog::File::File --
//		バージョンログファイル記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Version::File&		versionFile
//			バージョンログファイルを持つバージョンファイル
//		Version::File::StorageStrategy&	storageStrategy
//			バージョンログファイルを持つバージョンファイルのファイル格納戦略
//		Version::File::BufferingStrategy&	bufferingStrategy
//			バージョンログファイルを持つバージョンファイルのバッファリング戦略
//
//	RETURN
//		なし
//
//	EXCEPTIONS

VersionLog::
File::File(Version::File& versionFile,
		   const Version::File::StorageStrategy& storageStrategy,
		   const Version::File::BufferingStrategy& bufferingStrategy)
	: _bufFile(0),
	  _versionFile(versionFile),
	  _parent(storageStrategy._path._versionLog),
	  _infoTable(0),
	  _sizeMax(Version::File::verifySizeMax(
				   storageStrategy._sizeMax._versionLog)),
	  _extensionSize(Version::File::verifyExtensionSize(
						(storageStrategy._extensionSize._versionLog) ?
						storageStrategy._extensionSize._versionLog :
						Configuration::VersionLogExtensionSize::get()))
{
	// バージョンログファイルを格納する
	// OS ファイルの絶対パス名は必ず指定されている必要がある

	; _SYDNEY_ASSERT(storageStrategy._path._versionLog.getLength());

	try {
		// バージョンログファイルをバッファリングするための
		// バッファプール記述子を得る

		Buffer::AutoPool pool(Buffer::Pool::attach(
								  bufferingStrategy._category));

		// ファイル格納戦略からバージョンログファイルの絶対パス名を生成し、
		// バージョンログファイルの実体である
		// バッファファイルのバッファファイル記述子を得る
		//
		//【注意】	現状では、読み込み時に CRC による整合性の検証を行わない

		_bufFile = Buffer::File::attach(
			*pool, _File::getPath(_parent),	storageStrategy._pageSize,
			storageStrategy._mounted, storageStrategy._readOnly, true);

		// このバージョンログファイルの多重化されたブロックの
		// どれを選択するか決めるための情報をすべて管理するハッシュ表を確保する

		_infoTable =
			new HashTable<MultiplexInfo>(
				Configuration::MultiplexInfoTableSize::get(),
				&MultiplexInfo::_hashPrev, &MultiplexInfo::_hashNext);
		; _SYDNEY_ASSERT(_infoTable);
		
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		destruct();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Version::VersionLog::File::destruct --
//		バージョンログファイル記述子を表すクラスのデストラクター下位関数
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

void
VersionLog::
File::destruct()
{
	if (_infoTable) {

		// バージョンログファイルの多重化されたブロックを
		// 選択するための情報を表すクラスをすべて破棄し、
		// それらを管理するためのハッシュ表も破棄する

		clearMultiplexInfoTable();
		delete _infoTable, _infoTable = 0;
	}

	// バージョンログファイルの実体である
	// バッファファイルのバッファファイル記述子を破棄する

	Buffer::File::detach(_bufFile);
}

//	FUNCTION public
//	Version::VersionLog::File::create -- 生成する
//
//	NOTES
//		すでに生成されている
//		バージョンログファイルを生成してもエラーにならない
//
//	ARGUMENTS
//		unsigned int		pageCount
//			指定されたとき
//				生成するバージョンログファイルが所属する
//				バージョンファイルの総バージョンページ数
//			指定されないとき
//				0 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::create(const Trans::Transaction& trans,
			 unsigned int pageCount)
{
	if(!isMountedAndAccessible()) {

		// まず、バージョンログファイルの実体である
		// バッファファイルを生成する

		; _SYDNEY_ASSERT(_bufFile);
		_bufFile->create(false);

		try {
			// ファイルヘッダを確保する

			Block::Memory	headerMemory(FileHeader::allocate(trans, 0, *this));
			; _SYDNEY_ASSERT(headerMemory.isDirty());

			if (pageCount)

				// 指定された総バージョンページ数をファイルヘッダに記録する

				FileHeader::get(headerMemory).setPageCount(pageCount);

			//【注意】	ここでフラッシュしなくても、
			//			recover 時には、多重化したファイルヘッダを
			//			読み出せなければ、ファイル作成前の状態に
			//			回復しようとしているとみなすので、大丈夫

		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{

			// 生成したバッファファイルを破棄する

			_bufFile->destroy();

			_SYDNEY_RETHROW;
		}
	}
}

//	FUNCTION public
//	Version::VersionLog::File::destroy -- 破棄する
//
//	NOTES
//		バージョンログファイルが生成されていなくても例外は発生しない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::destroy()
{
	// バージョンログファイルの実体である
	// バッファファイルを破棄する

	_bufFile->destroy();

	// バージョンログファイルの多重化されたブロックを
	// 選択するための情報を表すクラスをすべて破棄する

	clearMultiplexInfoTable();
}

//	FUNCTION public
//	Version::VersionLog::File::unmount -- アンマウントする
//
//	NOTES
//		マウントされていない
//		バージョンログファイルをアンマウントしてもエラーにならない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::unmount()
{
	// バージョンログファイルの実体である
	// バッファファイルをアンマウントする

	_bufFile->unmount();

	// バージョンログファイルの多重化されたブロックを
	// 選択するための情報を表すクラスをすべて破棄する

	clearMultiplexInfoTable();
}

//	FUNCTION public
//	Version::VersionLog::File::truncate -- トランケートして空にする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			トランケートしても、バージョンログファイルは空にならなかった
//		false
//			トランケートしたら、バージョンログファイルは空になった
//
//	EXCEPTIONS

bool
VersionLog::
File::truncate(const Trans::Transaction& trans)
{
	// ファイルヘッダをフィックスして、存在するブロック数を求める
	//
	//【注意】	リカバリによって将来回復されるブロックをトランケートすると、
	//			リカバリできなくなってしまう
	//			そこでそれらのブロックも勘定に入れるために、
	//			多重化されたファイルヘッダーをすべてフィックスし、
	//			最大のブロック数を求める

	MultiplexBlock multi;
	FileHeader::fix(trans, 0, *this, Buffer::Page::FixMode::Write, multi);

	FileHeader* headers[MultiplexCount];
	unsigned int max = 0;
	bool empty = true;
	VersionNumber::Value v;

	unsigned int i = 0;
	do {
		if (headers[i] = (multi._memories[i].isOwner() ?
						  &FileHeader::get(multi._memories[i]) : 0))
		{
			v = headers[i]->getVersion();
			if (max < headers[i]->_blockCount)
				max = headers[i]->_blockCount;
			if (!headers[i]->isPBCTEmpty())
				empty = false;
		}
	} while (++i < MultiplexCount) ;

	if (max > MultiplexCount) {

		// バージョンログファイルには多重化された
		// ファイルヘッダ以外のブロックが確保されている

		// バージョンログファイルの最後の使用中のブロックまで保持するには
		// ファイルの先頭からいくつのブロックが必要か求める
		//
		//【注意】	多重化された PBCT がすべて空であれば、
		//			アロケーションテーブルを調べるまでもなく、
		//			ファイルヘッダを除くすべてのブロックが
		//			解放されているとみなせる

		const unsigned int n =
			(empty) ? MultiplexCount : (getLastBoundBlockID(trans, v, max) + 1);

		// 求めたブロック数以降の部分がなくなるかもしれないので、
		// なくなる部分に多重化されたブロックが存在し、
		// 多重化されたブロックのうち、
		// どれを選択すべきかを決めるための情報があれば、破棄する

		clearMultiplexInfoTable(n);

		if (headers[multi._master]->_blockCount > n) {

			// ファイルヘッダの総ブロック数を変更する

			headers[multi._master]->_blockCount = n;
			multi._memories[multi._master].dirty();

			// 最大の総ブロック数を求めなおす

			max = n;

			unsigned int i = 0;
			do {
				if (headers[i] && max < headers[i]->_blockCount)
					max = headers[i]->_blockCount;
			} while (++i < MultiplexCount) ;
		}
	}

	// バージョンログファイルの先頭から
	// 求めたブロック数分のブロックより後の部分をすべてトランケートする

	_bufFile->truncate(static_cast<Os::File::Offset>(getBlockSize()) * max);

	// バージョンログファイルが空でないかを返す

	return max > MultiplexCount;
}

//	FUNCTION public
//	Version::VersionLog::File::truncate --
//		あるバージョンページ以降のすべてのバージョンページに関する領域を
//		使用済にし、可能な限りトランケートする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			トランケートするトランザクションのトランザクション記述子
//		Version::Page::ID	pageID
//			このバージョンページ識別子のバージョンページ以降の
//			すべてのバージョンページに関する領域を使用済にする
//
//	RETURN
//		true
//			トランケートしても、バージョンログファイルは空にならなかった
//		false
//			トランケートしたら、バージョンログファイルは空になった
//
//	EXCEPTIONS

bool
VersionLog::
File::truncate(const Trans::Transaction& trans, Page::ID pageID)
{
	{
	// バージョンログファイルのファイルヘッダをフィックスする

	Block::Memory headerMemory(
		FileHeader::fix(trans, 0, *this, Buffer::Page::FixMode::Write));
	FileHeader& header = FileHeader::get(headerMemory);
	VersionNumber::Value v = header.getVersion();

	if (header.getPageCount() <= pageID)

		// 存在していないページについてはなにも使用済にできない

		// 最初っから空かもしれないので、念のため調べておく

		return getSize() / getBlockSize() > MultiplexCount;

	// 今から処理するバージョンページの
	// 最新版を記録するバージョンログのブロック識別子を記録する
	// PBCT リーフを使用するバージョンページの
	// バージョンページ識別子のうち、最も小さいものを求める

	const unsigned int l =
		PBCTLeaf::getCountMax(header.getPBCTLevel(), getBlockSize());
	Page::ID j = pageID / l * l;

	do {
		// 処理しようとしているバージョンページに関する
		// PBCT リーフがあれば、フィックスする

		Block::Memory	leafMemory(
			traversePBCT(trans,
						 0, headerMemory, j, Buffer::Page::FixMode::Write));

		if (leafMemory.getID() == Block::IllegalID)

			// 次の PBCT リーフを使用する
			// バージョンページについて調べることにする

			continue;

		VersionLog::PBCTLeaf& leaf = PBCTLeaf::get(leafMemory);

		if (!leaf.getCount())

			// あるバージョンページの最新版を記録する
			// バージョンログのブロック識別子が
			// この PBCT リーフにひとつも記録されていない

			// PBCT のルートからこの PBCT リーフまでの経路上で
			// PBCT ノードおよびリーフを可能な限り使用済にする

			freePBCT(trans, 0, headerMemory, j);
		else {

			// この PBCT リーフに記録されている
			// ブロック識別子をひとつひとつ処理していく

			const unsigned int n = ((header.getPageCount() < j + l) ?
									header.getPageCount() - j : l);

			for (unsigned int i = (j < pageID) ? pageID - j : 0; i < n; ++i) {

				// 処理しようとしているバージョンページの
				// 最新版を記録するバージョンログのブロック識別子を得る

				const Block::ID id = leaf.getLatestID(i);

				if (id != Block::IllegalID) {

					// このバージョンページの版を記録する
					// バージョンログをすべて使用済にする

					freeLog(trans, v, 0, id, leaf.getOldestTimeStamp(i));

					// PBCT リーフ上の
					// このバージョンページに関する情報を初期化する

					leaf.setOldestTimeStamp(i, Trans::IllegalTimeStamp);
					leaf.setLatestID(i, Block::IllegalID);
					leafMemory.dirty();
				}
			}

			if (!leaf.getCount())

				// この PBCT リーフに
				// ブロック識別子がひとつも記録されなくなった

				freePBCT(trans, 0, headerMemory, j);
		}
	} while ((j += l) < header.getPageCount()) ;

	// 新しいページ数をファイルヘッダに記録する

	header.setPageCount(pageID);
	headerMemory.dirty();
	}
	// 実際に可能な限りトランケートする

	return truncate(trans);
}

//	FUNCTION private
//	Version::VersionLog::File::getLastBoundBlockID --
//		ファイルで最後の使用中のブロックのブロック識別子を求める
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		blockCount
//			バージョンログファイルで確保されているブロックの数
//
//	RETURN
//		求めたブロック識別子
//
//	EXCEPTIONS

Block::ID
VersionLog::
File::getLastBoundBlockID(const Trans::Transaction& trans,
						  VersionNumber::Value v, unsigned int blockCount)
{
	; _SYDNEY_ASSERT(blockCount > MultiplexCount);

	const unsigned int bitCount =
		_AllocationTable::getBitCount(v, getBlockSize());
	const unsigned int bitmapLength =
		_AllocationTable::getBitmapLength(v, getBlockSize());

	// ファイルの末尾から、アロケーションテーブルをひとつづつ調べていく

	Block::ID tableID =
		_AllocationTable::roundBlockID(v, getBlockSize(), blockCount - 1);

	do {
		// アロケーションテーブルをフィックスする

		MultiplexBlock multi;
		AllocationTable::fix(
			trans, 0, *this, tableID, Buffer::Page::FixMode::ReadOnly, multi);

		const AllocationTable* tables[MultiplexCount];
		unsigned int n = 0;

		unsigned int i = 0;
		do {
			if (tables[i] = (multi._memories[i].isOwner() ?
							 &AllocationTable::get(
								 static_cast<const Block::Memory&>(
									 multi._memories[i])) : 0))
				n += tables[i]->getCount();
		} while (++i < MultiplexCount) ;

		if (n) {

			// アロケーションテーブルの
			// 使用中のブロックであるかを表すビットマップを調べていく

			unsigned int j = bitmapLength - 1;
			do {
				unsigned int bitmap = 0;

				unsigned int i = 0;
				do {
					if (tables[i])
						bitmap |= tables[i]->_bitmap[j];
				} while (++i < MultiplexCount) ;

				if (bitmap) {
					unsigned int k = _AllocationTable::_bitCountPerBitmap - 1;
					unsigned int m = 1 << k;

					do {
						if (bitmap & m) {

							// このビットは立っているので、
							// このビットが表すブロックのブロック識別子を求める

							const Block::ID id =
								tableID + MultiplexCount +
								j * _AllocationTable::_bitCountPerBitmap + k;

							// ファイルヘッダと
							// 最初のアロケーションテーブルより後にあるはず

							; _SYDNEY_ASSERT(id >= MultiplexCount << 1);

							return id;
						}

						m >>= 1;

					} while (k--) ;
				}
			} while (j--) ;

			; _SYDNEY_ASSERT(false);
		}

		// このアロケーションテーブルには使用済のブロックは存在しないので、
		// ひとつ前のアロケーションテーブルを調べることにする

	} while (tableID > bitCount + MultiplexCount &&
			 (tableID -= bitCount + MultiplexCount)) ;

	return MultiplexCount - 1;
}

//	FUNCTION public
//	Version::VersionLog::File::move -- 移動する
//
//	NOTES
//		バージョンログファイルが生成されていなくてもエラーにならない
//
//	ARGUMENTS
//		Os::Path&			path
//			移動先の絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::move(const Os::Path& path)
{
	if (path.compare(getParent()) != Os::Path::CompareResult::Identical) {

		// 指定された移動先の絶対パス名が
		// バージョンログファイルの実体である
		// バッファファイルを格納場所と同じでない

		// 指定された移動先の絶対パス名から、
		// バージョンログファイルの実体である
		// バッファファイルの新しい絶対パス名を生成し、
		// それに変更する
		//
		//【注意】	改名するバッファファイルの実体である
		//			OS ファイルが存在しなくても、例外を発生しない

		; _SYDNEY_ASSERT(_bufFile);
		_bufFile->rename(_File::getPath(path));

		// 新しい格納場所を記憶しておく

		_parent = path;
	}
}

//	FUNCTION public
//	Version::VersionLog::File::startBackup -- バックアップの開始を指示する
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

void
VersionLog::
File::startBackup()
{
	// バックアップの開始を指示する
	// バージョンログファイルはマウントされている必要がある

	; _SYDNEY_ASSERT(isMountedAndAccessible());

	// バージョンログファイルの
	//
	// * ファイルヘッダ
	// * アロケーションテーブル、
	// * PBCT
	//
	// を記録するバッファページのうち、ダーティなものがあっても、
	// 強制的にフラッシュしない限り、フラッシュされないようにする
	//
	//【注意】	現状では、自分だけでなく
	//			すべてのバージョンログファイルに影響する

	; _SYDNEY_ASSERT(_bufFile);
	_bufFile->startDeterrent();
}

//	FUNCTION public
//	Version::VersionLog::File::endBackup -- バックアップの終了を指示する
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

void
VersionLog::
File::endBackup()
{
	// バックアップの終了を指示する
	// バージョンログファイルはマウントされている必要がある

	; _SYDNEY_ASSERT(isMountedAndAccessible());

	// ダーティなバッファページのフラッシュの抑止をやめる
	//
	//【注意】	現状では、自分だけでなく
	//			すべてのバージョンログファイルに影響する

	; _SYDNEY_ASSERT(_bufFile);
	_bufFile->endDeterrent();
}

//	FUNCTION private
//	Version::VersionLog::File::extend -- 拡張する
//
//	NOTES
//		拡張された領域を管理するためのアロケーションテーブルは確保されない
//
//	ARGUMENTS
//		Version::Block::ID	id
//			このブロック識別子のブロックの直前のものまでが
//			確保可能になるまで、拡張する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::extend(Block::ID id)
{
	// 指定されたブロックの直前のブロックまでが確保可能になるまで、
	// ファイル拡張サイズごとに実体であるバッファファイルを拡張していく
	//
	//【注意】	ただし、指定されたブロックを確保する領域が
	//			ファイル拡張サイズより小さければ、
	//			指定されたブロックを確保する領域ぶん、拡張する

	const Os::File::Offset offset =
		static_cast<Os::File::Offset>(getBlockSize()) * id;

	while (getSize() < static_cast<Os::File::Size>(offset))
		_bufFile->extend(getSize() + getExtensionSize());
}

//	FUNCTION public
//	Version::VersionLog::File::restore --
//		あるタイムスタンプの表す時点に開始された版管理するトランザクションの
//		参照する版が最新版になるようにバージョンログファイルを変更する
//		
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			変更を行うトランザクションのトランザクション記述子
//		Trans::TimeStamp&	point
//			このタイムスタンプの表す時点に開始された
//			版管理するトランザクションの参照する版が
//			最新版になるようにバージョンログファイルを変更する
//
//	RETURN
//		true
//			あるトランザクションが参照する版を最新版になるように変更した
//		false
//			ファイルを生成した時点より前に開始された
//			トランザクションが参照する版は存在しない
//
//	EXCEPTIONS

bool
VersionLog::
File::restore(const Trans::Transaction& trans, const Trans::TimeStamp& point)
{
	; _SYDNEY_ASSERT(!point.isIllegal());

	// バージョンログファイルのファイルヘッダをフィックスする
	//
	//【注意】	ファイルヘッダは更新しないが、
	//			traversePBCT で Buffer::Page::FixMode::Write で
	//			フィックスされた PBCT リーフを得る必要があるので、
	//			ファイルヘッダも Buffer::Page::FixMode::Write でフィックスする

	Block::Memory headerMemory;
	try {
		headerMemory = FileHeader::fix(trans,
									   0, *this, Buffer::Page::FixMode::Write);

	} catch (Exception::BadDataPage&) {

		// 多重化されたファイルヘッダの中に読み出せないものがあったので、
		// 与えられたタイムスタンプが表す時点より後に
		// ファイルが生成されたものとみなす

		return false;
	}

	const FileHeader& header =
		FileHeader::get(static_cast<const Block::Memory&>(headerMemory));

	if (point < header._creationTimeStamp)

		// ファイルを生成した時点より前に開始された
		// トランザクションが参照する版はこのバージョンログファイルに存在しない

		return false;

	// バージョン番号を取得する

	VersionNumber::Value v = header.getVersion();

	// バージョンファイルに格納されている
	// バージョンページを先頭からひとつひとつ処理していく

	for (Page::ID i = 0; i < header.getPageCount(); ++i) {

		// 今調べているバージョンページの最新版を記録する
		// バージョンログのブロック識別子を記録する PBCT リーフを得る

		Block::Memory	leafMemory(
			traversePBCT(trans,
						 0, headerMemory, i, Buffer::Page::FixMode::Write));

		if (leafMemory.getID() != Block::IllegalID) {

			// 最新版のバージョンログの
			// ブロック識別子を記録する PBCT リーフが存在した

			PBCTLeaf& leaf = PBCTLeaf::get(leafMemory);

			// 得られた PBCT リーフに記録されている
			// 最新版のバージョンログのブロック識別子を求める

			Block::ID id = leaf.getLatestID(i, getBlockSize());

			if (id != Block::IllegalID) {

				// 最新版のバージョンログから最古のものへたどりながら、
				// 指定されたタイムスタンプの時点に開始された
				// 版管理するトランザクションが
				// 参照すべき版のバージョンログを探す

				const Trans::TimeStamp::Value oldest =
					leaf.getOldestTimeStamp(i, getBlockSize());
				; _SYDNEY_ASSERT(!Trans::TimeStamp::isIllegal(oldest));

				ModVector<Block::ID> freeList;

				do {
					const Block::Memory& logMemory =
						Log::fix(trans, 0, *this, id,
								 Buffer::Page::FixMode::ReadOnly,
								 Buffer::ReplacementPriority::Low);

					if (logMemory.getLastModification() < point)

						// 参照すべき版が見つかった
						
						break;

					// 使用済にするバージョンログのブロック識別子をおぼえておく

					freeList.pushBack(id);

					const Log& log = Log::get(logMemory);

					if (v == VersionNumber::First &&
						log._olderTimeStamp < oldest)

						// 最古のバージョンログまで調べた

						break;

					// ひとつ古い版のバージョンログを調べることにする

					id = log._older;

				} while (id != Block::IllegalID) ;

				if (freeList.getSize()) {

					// 見つかった参照すべき版が最新版になるように、
					// PBCT リーフに記録する

					leaf.setLatestID(i, id, getBlockSize());

					if (id == Block::IllegalID)

						// 最新版がマスタデータファイルにあることになったので、
						// 最古版のタイムスタンプを未定にする

						leaf.setOldestTimeStamp(
							i, Trans::IllegalTimeStamp, getBlockSize());

					if (v >= VersionNumber::Second)
					{
						// 最新の版が変更されたので最新版のビットを変更する

						if (id != Block::IllegalID)

							// 最新版にする

							setNewest(trans, v, 0, id, true);

						// これまで最新版だったものを最新版ではなくする
						// 最新版からたどって、freeList に pushBack して
						// いるので、最新版は先頭要素にある

						setNewest(trans, v, 0, freeList.getFront(), false);
					}

					leafMemory.dirty();

					// おぼえておいたブロック識別子の表す
					// ブロックをすべて使用済にする

					do {
						VersionNumber::Value v = header.getVersion();
						Log::free(trans, v, 0, *this, freeList.getBack());
						freeList.popBack();
					} while (freeList.getSize()) ;
				}
			}
		}
	}

	return true;
}

//	FUNCTION public
//	Version::VersionLog::File::recover --
//		あるタイムスタンプの表す時点の状態に
//		バージョンログファイルを障害回復する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp&	point
//			このタイムスタンプの表す時点以前のチェックポイント処理の
//			終了時の状態にバージョンログファイルを回復する
//		unsigned int		pageCount
//			この関数が true を返すとき、
//			このタイムスタンプの表す時点以前のチェックポイント処理の
//			終了時の総バージョンページ数が設定される
//
//	RETURN
//		true
//			障害回復できた
//		false
//			バージョンログファイルを生成したときより
//			前の状態には障害回復できなかった
//
//	EXCEPTIONS

bool
VersionLog::
File::recover(const Trans::Transaction& trans,
			  const Trans::TimeStamp& point, unsigned int& pageCount)
{
	; _SYDNEY_ASSERT(!point.isIllegal());

	// まず、ファイルヘッダを与えられたタイムスタンプ以前の
	// チェックポイント処理の終了時の状態に回復する

	const Block::Memory& headerMemory = recoverMaster(trans, 0, point);
	if (headerMemory.getID() == Block::IllegalID)

		// ファイルヘッダを記録する多重化されたブロックがすべて壊れているので、
		// ファイルを生成した時点より前の状態に回復しようとしているとみなす

		return false;

	const FileHeader& header = FileHeader::get(headerMemory);

	if (!header._creationTimeStamp || point < header._creationTimeStamp)

		// 回復されたファイルヘッダに記録されている
		// ファイル作成時タイムスタンプ値が
		// 0 または与えられたタイムスタンプより大きいとき、
		// ファイルを生成した時点より前の状態に回復しようとしている
		//
		//【注意】	回復されたファイルヘッダに記録されている
		//			ファイル作成時タイムスタンプ値が 0 のときは、
		//			回復されたファイルヘッダを記録するブロックが
		//			ファイルヘッダを確保したときに 0 埋めされた
		//			多重化ブロックのうちのひとつであることがわかる

		return false;

	// 与えられたタイムスタンプ以前の
	// チェックポイント処理の終了時の総バージョンページ数をおぼえておく

	pageCount = header.getPageCount();

	// ファイルの先頭から、アロケーションテーブルをひとつひとつ調べていく

	const unsigned int bitCount =
		_AllocationTable::getBitCount(header.getVersion(), getBlockSize());

	for (Block::ID tableID = MultiplexCount;
		 tableID < header._blockCount; tableID += bitCount + MultiplexCount)

		// 多重化されて記録されているアロケーションテーブルを
		// 与えられたタイムスタンプ以前の
		// チェックポイント処理の終了時の状態に回復する

		(void) recoverMaster(trans, tableID, point);

	// PBCT をルートから子供へ辿りながら、
	// すべてのノード、リーフを与えられたタイムスタンプ以前の
	// チェックポイント処理の終了時の状態に回復する

	recoverPBCT(trans, headerMemory, 0, header.getPBCTLevel(), point);

	return true;
}

//	FUNCTION private
//	Version::VersionLog::File::recoverPBCT --
//		バージョンログファイルの多重化された PBCT ノード、リーフのうち、
//		あるタイムスタンプ以降に更新されたものをなくす
//
//	NOTES
//
//	ARGUMENTS
//		Block::Memory&		nodeMemory
//			処理する PBCT ノード、リーフのバッファリング内容
//		Version::VersionLog::PBCTLevel::Value	current
//			今処理している PBCT ノード、リーフの深さ(0 以上)
//		Version::VersionLog::PBCTLevel::Value	level
//			処理している PBCT の深さ(0 以上)
//		Trans::TimeStamp&	point
//			多重化された PBCT  ノード、リーフのうち、
//			このタイムスタンプ以降に更新されたものをなくす
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::recoverPBCT(const Trans::Transaction& trans,
				  const Block::Memory& nodeMemory, PBCTLevel::Value current,
				  PBCTLevel::Value level, const Trans::TimeStamp& point)
{
	; _SYDNEY_ASSERT(current != PBCTLevel::Illegal);
	; _SYDNEY_ASSERT(!point.isIllegal());

	if (level != PBCTLevel::Illegal && current < level) {

		// 与えられたものがノードであれば、
		// その子供のノード、リーフをひとつづつ得て、処理する

		const PBCTNode& node = PBCTNode::get(nodeMemory);

		unsigned int i = 0;
		const unsigned int n = PBCTNode::getCountMax(current, getBlockSize());

		do {
			// 子供のノード、リーフを
			// 与えられたタイムスタンプ時点の状態に戻す

			const Block::ID id = node.getChildID(i);
			if (id != Block::IllegalID) {
				const Block::Memory& childMemory = recoverMaster(trans,
																 id, point);
				; _SYDNEY_ASSERT(childMemory.getID() != Block::IllegalID);

				// この子供のノード、リーフに子供があれば、同様に処理する

				recoverPBCT(trans, childMemory, current + 1, level, point);
			}
		} while (++i < n) ;
	}
}

//	FUNCTION private
//	Version::VersionLog::File::recoverMaster --
//		バージョンログファイルの多重化されたブロックのうち、
//		あるタイムスタンプ以降に更新されたものをなくす
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::ID	id
//			多重化されたブロックの先頭のもののブロック識別子
//		Trans::TimeStamp&	point
//			多重化されたバージョンログのうち、
//			このタイムスタンプ以降に更新されたものをなくす
//
//	RETURN
//		あるタイムスタンプより前で
//		最近に更新されたマスタブロックのバッファリング内容
//
//		ただし、それに対する Version::Block::Memory::getID の結果が	0 のとき、
//		与えられたタイムスタンプより前に更新されているものが
//		ひとつもなかったことを表す
//
//	EXCEPTIONS

Block::Memory
VersionLog::
File::recoverMaster(const Trans::Transaction& trans,
					Block::ID id, const Trans::TimeStamp& point)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);
	; _SYDNEY_ASSERT(!point.isIllegal());

	// 多重化されたブロックのうち、
	// どれをマスタブロックにすべきか決めるための情報を表すクラスを得て、
	// 保護するためにラッチする

	MultiplexInfo& info = *MultiplexInfo::attach(*this, id);
	Os::AutoCriticalSection	latch(info.getLatch());

	//【注意】	多重化されたブロックがすべて与えられた
	//			タイムスタンプ以降に更新されているかは、
	//			すべてフィックスしないとわからないので、
	//			しかたなく 2 パスで処理する

	// 多重化されたブロックをすべてフィックスする

	Block::Memory	memories[MultiplexCount];
	Trans::TimeStamp::Value	oldest = Trans::IllegalTimeStamp;
	{
	unsigned int	i = 0;
	do {
		try {
			// 最終更新時タイムスタンプをおぼえておく

			info._lastModification[i] =
				(memories[i] = fixBlock(
					trans, 0, id + i, Buffer::Page::FixMode::Write,
					Buffer::ReplacementPriority::Low)).getLastModification();

			if (Trans::TimeStamp::isIllegal(oldest) ||
				oldest > info._lastModification[i])

				// 最終更新時タイムスタンプがこれまでで最も古いのでおぼえておく

				oldest = info._lastModification[i];

		} catch (Exception::BadDataPage&) {

			// このブロックはフィックスできなかったが、
			// ほかのブロックはフィックスできるかもしれない

			info._lastModification[i] = Trans::IllegalTimeStamp;
		}
	} while (++i < MultiplexCount) ;
	}
	if (Trans::TimeStamp::isIllegal(oldest) || oldest > point)

		// 多重化されたブロックをひとつもフィックスできなかった、
		// または、多重化されたブロックのうちフィックスできたものは
		// すべて与えられたタイムスタンプ以降に更新されている

		return Block::Memory();

	// 多重化されたブロックのうち、
	// 与えられたタイムスタンプより前に更新されているものがある

	unsigned int	latest = MultiplexCount;
	unsigned int	i = 0;

	do {
		if (Trans::TimeStamp::isIllegal(info._lastModification[i]))

			// フィックスできなかったブロックは確保しなおす

			fixBlock(trans, 0, id + i, Buffer::Page::FixMode::Allocate,
					 Buffer::ReplacementPriority::Low).
				unfix(Trans::TimeStamp(0));

		else if (info._lastModification[i] > point) {

			// 与えられたタイムスタンプ以降に更新されたものは初期化しなおす

			memories[i].reset().unfix(Trans::TimeStamp(0));

			// 最終更新時タイムスタンプは不正にしておく

			info._lastModification[i] = Trans::IllegalTimeStamp;

		} else if (latest == MultiplexCount ||
				   info._lastModification[i] > info._lastModification[latest])

			// 与えられたタイムスタンプより前で
			// 最近に更新されているものの候補である

			latest = i;

	} while (++i < MultiplexCount) ;

	; _SYDNEY_ASSERT(latest < MultiplexCount);
	; _SYDNEY_ASSERT(memories[latest].getID() != Block::IllegalID);

	return memories[latest];
}

//	FUNCTION public
//	Version::VersionLog::File::startVerification -- 整合性検査の開始を指示する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification&	verification
//			あるトランザクションの整合性検査に関する情報を表すクラス
//		Version::MasterData::File&	masterData
//			検査するバージョンログファイルを持つバージョンファイルの
//			マスタデータファイルのマスタデータファイル記述子
//		Admin::Verification::Progress&	result
//			整合性検査の経過を設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::startVerification(const Trans::Transaction& trans,
						Verification& verification,
						MasterData::File& masterData,
						Admin::Verification::Progress& result)
{
	; _SYDNEY_ASSERT(result.isGood());

	if (!isAccessible(true)) {

		// バージョンファイルを構成する
		// バージョンログファイルが生成されていない

		_SYDNEY_VERIFY_INCONSISTENT(
			result, getParent(), Message::VersionLogFileNotFound());
		return;
	}

	// バージョンログファイルのファイルヘッダをフィックスする

	const Block::Memory& headerMemory =
		FileHeader::fix(trans,
						&verification, *this, Buffer::Page::FixMode::ReadOnly);
	const FileHeader& header = FileHeader::get(headerMemory);

	if (header._blockCount > getSize() / getBlockSize()) {

		// ファイルヘッダに記録されている総ブロック数よりも、
		// バージョンログファイルのサイズから計算される
		// 確保可能なブロック数のほうが少ない

		_SYDNEY_VERIFY_INCONSISTENT(
			result, getParent(), Message::BlockCountInconsistent());
		return;
	}

	if (header.getPageCount() > masterData.getBlockCount(trans)) {

		// ファイルヘッダに記録されている総バージョンページ数が
		// マスタデータファイルの総ブロック数と異なる

		_SYDNEY_VERIFY_INCONSISTENT(
			result, getParent(), Message::VersionPageCountInconsistent());
		return;
	}

	// 検査済のバージョンページと、バージョンログファイルの
	// 検査済のブロックを管理するためのビットマップを
	// 記録するために必要な領域をそれぞれ確保する

	verification.getPageBitmap().reserve(header.getPageCount());
	verification.getBlockBitmap().reserve(header._blockCount);

	// バージョンログファイルのバージョンを設定する

	verification.setVersionNumber(header.getVersion());
}

//	FUNCTION public
//	Version::VersionLog::File::endVerification -- 整合性検査の終了を指示する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification&	verification
//			あるトランザクションの整合性検査に関する情報を表すクラス
//		Admin::Verification::Progress&	result
//			整合性検査の経過を設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::endVerification(const Trans::Transaction& trans,
					  Verification& verification,
					  Admin::Verification::Progress& result)
{
	while (result.isGood()) {

		// バージョンログファイルのファイルヘッダをフィックスする

		const Block::Memory& headerMemory =
			FileHeader::fix(
				trans,
				&verification, *this, Buffer::Page::FixMode::ReadOnly);
		const FileHeader& header = FileHeader::get(headerMemory);

		if (verification.getBlockBitmap().getMaxIndex() !=
				Verification::Bitmap::Index::Invalid &&
			header._blockCount <=
				verification.getBlockBitmap().getMaxIndex()) {
	
			// ファイルヘッダに記録されている総ブロック数が
			// 整合性検査中にフィックスしたブロックのブロック識別子のうち、
			// 最大のもの以下である

			_SYDNEY_VERIFY_INCONSISTENT(
				result, getParent(), Message::BlockCountInconsistent());
			break;
		}

		if ((verification.isOverall()) ?
			header.getPageCount() !=
				verification.getPageBitmap().getBitCount() :
			(verification.getPageBitmap().getMaxIndex() !=
				Verification::Bitmap::Index::Invalid &&
			 header.getPageCount() <=
				verification.getPageBitmap().getMaxIndex())) {

			// ファイルヘッダに記録されている総バージョンページ数が
			// 整合性検査中にフィックスしたバージョンページの
			// バージョンページ識別子のうち、最大のもの以下である

			_SYDNEY_VERIFY_INCONSISTENT(
				result, getParent(), Message::VersionPageCountInconsistent());
			break;
		}

		// バージョンファイルの
		// すべてのアロケーションテーブルの整合性検査を行う

		verifyAllocationTable(trans, verification, header, result);

		// バージョンログファイルの整合性検査終了

		break;
	}

	// 検査済のバージョンページと、バージョンログファイルの
	// 検査済のブロックを管理するためのビットマップを初期化する

	verification.getPageBitmap().clear();
	verification.getBlockBitmap().clear();
}

//	FUNCTION public
//	Version::VersionLog::File::verify --
//		あるバージョンページに関して整合性を検査する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			整合性を検査するトランザクションのトランザクション記述子
//		Version::Verification&	verification
//			あるトランザクションの整合性検査に関する情報を表すクラス
//		Version::Page::ID	pageID
//			整合性を検査するバージョンページのバージョンページ識別子
//		Admin::Verification::Progress&	result
//			整合性検査の経過を設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::verify(const Trans::Transaction& trans, Verification& verification,
			 Page::ID pageID, Admin::Verification::Progress& result)
{
	if (verification.getPageBitmap().getBit(pageID))

		// すでに指定されたバージョンページ識別子の
		// バージョンページに関して整合性を検査している

		return;

	// 指定されたバージョンページの最新版を記録する
	// バージョンログのブロック識別子を記録する PBCT リーフを得る

	Admin::Verification::Progress progress(result.getConnection());
	const Block::Memory& leafMemory =
		verifyPBCT(trans, verification, pageID, progress);
	result += progress;

	if (!progress.isGood())
		return;

	if (leafMemory.getID() != Block::IllegalID) {

		// 最新版を記録するバージョンログの
		// ブロック識別子を記録する PBCT リーフが存在した

		const PBCTLeaf& leaf = PBCTLeaf::get(leafMemory);

		// PBCT リーフに記録されている
		// このバージョンページの最新版を記録する
		// バージョンログのブロック識別子を求める

		const Block::ID id = leaf.getLatestID(pageID, getBlockSize());

		if (id != Block::IllegalID) {

			// PBCT リーフに記録されている
			// このバージョンページの最古のバージョンログの
			// 最終更新時タイムスタンプ値を求める

			const Trans::TimeStamp::Value oldest =
				leaf.getOldestTimeStamp(pageID, getBlockSize());

			if (Trans::TimeStamp::isIllegal(oldest)) {

				// 最古のバージョンログの最終更新時タイムスタンプ値が不正である

				_SYDNEY_VERIFY_INCONSISTENT(
					result, getParent(),
					Message::OldestTimeStampInconsistent());
				return;
			}

			// このバージョンページのすべての版の
			// バージョンログに対して整合性検査する

			verifyLog(trans, verification, pageID, id, oldest, result);
		}
	}

	// 検査したバージョンページに対応するビットを立てる

	verification.getPageBitmap().setBit(pageID, true);
}

//	FUNCTION private
//	Version::VersionLog::File::verifyPBCT --
//		あるバージョンページの最新版のバージョンログの
//		ブロック識別子を得るための PBCT ノード、リーフを整合性検査する
//
//	NOTES
//		同時に、あるバージョンページの最新版を表すバージョンログの
//		ブロック識別子を記録する PBCT リーフを得る
//
//	ARGUMENTS
//		Version::Verification&	verification
//			あるトランザクションの整合性検査に関する情報を表すクラス
//		Version::Page::ID	pageID
//			このバージョンページ識別子のバージョンページの最新版を表す
//			バージョンログのブロック識別子を得るための
//			PBCT ノード、リーフを整合性検査する
//		Admin::Verification::Progress&	result
//			整合性検査の経過を設定する
//
//	RETURN
//		見つかった PBCT リーフのバッファリング内容
//
//		ただし、それに対する Version::Block::Memory::getID の結果が 0 のとき、
//		PBCT リーフが存在しない、または矛盾が検出されたことを表す
//
//	EXCEPTIONS

Block::Memory
VersionLog::
File::verifyPBCT(const Trans::Transaction& trans,
				 Verification& verification, Page::ID pageID,
				 Admin::Verification::Progress& result)
{
	// 今からフィックスするファイルヘッダに記録されている
	// ブロックに記録されている PBCT ノード、リーフを
	// すでに整合性検査しているか調べる

	const bool verified = verification.getBlockBitmap().getBit(0);

	// ファイルヘッダをフィックスする

	const Block::Memory& headerMemory =
		FileHeader::fix(trans,
						&verification, *this, Buffer::Page::FixMode::ReadOnly);
	const FileHeader& header = FileHeader::get(headerMemory);

	if (!header.isPBCTEmpty())

		// PBCT が存在する

		if (!header.getPBCTLevel()) {

			// PBCT にはルートリーフしかない

			if (pageID < PBCTLeaf::getCountMax(false, getBlockSize())) {

				// ルートリーフだけで管理できるバージョンページの最大数より、
				// 指定されたバージョンページ識別子が小さい

				if (!verified) {

					// まだ整合性検査されていないので、検査する

					Admin::Verification::Progress
						progress(result.getConnection());
					PBCTLeaf::get(headerMemory).verify(*this, false, progress);
					result += progress;

					if (!progress.isGood())
						return Block::Memory(); 
				}

				// リーフにひとつでもブロック識別子が記録されていれば、
				// ファイルヘッダを記録するブロックを返す

				return headerMemory;
			}
		} else if (PBCTNode::getCountMax(false, getBlockSize()) *
				   _Math::pow(PBCTNode::getCountMax(true, getBlockSize()),
							  header.getPBCTLevel() - 1) *
				   PBCTLeaf::getCountMax(true, getBlockSize()) > pageID) {

			// 現在の PBCT で管理できるバージョンページの最大数より、
			// 指定されたバージョンページ識別子が小さい

			const PBCTNode& node = PBCTNode::get(headerMemory);

			if (!verified) {

				// まだ整合性検査されていないので、検査する

				Admin::Verification::Progress progress(result.getConnection());
				node.verify(*this, false, progress);
				result += progress;

				if (!progress.isGood())
					return Block::Memory(); 
			}

			// 与えられたバージョンページの版をたどるための
			// 子供のノード、リーフを記録するブロックのブロック識別子を得る

			Block::ID id = node.getChildID(
				pageID, 0, header.getPBCTLevel(), getBlockSize());

			for (PBCTLevel::Value i = 1; id != Block::IllegalID; ++i) {

				// 今からフィックスする PBCT ノード、リーフを
				// すでに整合性検査しているか調べる

				const bool verified = verification.getBlockBitmap().getBit(id);

				if (i == header.getPBCTLevel()) {

					// 整合性検査する PBCT リーフをフィックスする

					const Block::Memory& leafMemory =
						PBCTLeaf::fix(trans, &verification, *this, id,
									  Buffer::Page::FixMode::ReadOnly);

					if (!verified) {
						Admin::Verification::Progress
							progress(result.getConnection());
						PBCTLeaf::get(leafMemory).verify(*this, true, progress);
						result += progress;

						if (!progress.isGood())
							break;
					}

					// 整合性検査済の PBCT リーフのバッファリング内容を返す

					return leafMemory;
				}

				// 整合性検査する PBCT ノードをフィックスする

				const Block::Memory& nodeMemory =
					PBCTNode::fix(trans, &verification, *this, id,
								  Buffer::Page::FixMode::ReadOnly);
				const PBCTNode& node = PBCTNode::get(nodeMemory);

				id = node.getChildID(
					pageID, i, header.getPBCTLevel(), getBlockSize());

				if (!verified) {
					Admin::Verification::Progress
						progress(result.getConnection());
					node.verify(*this, true, progress);
					result += progress;

					if (!progress.isGood())
						break;
				}
			}
		}

	// 整合性検査済の PBCT リーフを得られなかった

	return Block::Memory();
}

//	FUNCTION private
//	Version::VersionLog::File::verifyLog -- 
//		あるバージョンページのすべての版のバージョンログに対して整合性検査する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			整合性検査を行うトランザクションのトランザクション記述子
//		Version::Verification&	verification
//			あるトランザクションの整合性検査に関する情報を表すクラス
//		Version::Page::ID	pageID
//			バージョンページ	
//		Version::Block::ID	id
//			最新版のバージョンログのブロック識別子
//		Trans::TimeStamp::Value oldest
//			最古のバージョンログの最終更新時タイムスタンプ値
//		Admin::Verification::Progress&	result
//			整合性検査の経過を設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::verifyLog(
	const Trans::Transaction& trans, Verification& verification,
	Page::ID pageID, Block::ID id,
	Trans::TimeStamp::Value oldest, Admin::Verification::Progress& result)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);
	; _SYDNEY_ASSERT(!Trans::TimeStamp::isIllegal(oldest));

	// 指定されたブロック識別子の表す最新版のバージョンログから
	// 最古のバージョンログへひとつひとつ調べていく

	Common::BitSet bitmap;
	Trans::TimeStamp::Value older = Trans::IllegalTimeStamp;

	do {
		if (_Log::isIllegalID(id) || bitmap.test(id)) {

			// バージョンログのブロック識別子が不正である

			_SYDNEY_VERIFY_INCONSISTENT(
				result, getParent(), Message::VersionLogIDInconsistent());
			return;
		}

		// このブロック識別子のバージョンログをフィックスする

		const Block::Memory& logMemory =
			Log::fix(trans, &verification, *this, id,
					 Buffer::Page::FixMode::ReadOnly,
					 Buffer::ReplacementPriority::Low);
		const Trans::TimeStamp::Value t = logMemory.getLastModification();

		if (verification.getVersionNumber() < VersionNumber::Second &&
			(t < oldest ||
			 !(Trans::TimeStamp::isIllegal(older) || t == older))) {

			// ひとつ前のバージョンログの最終更新時タイムスタンプ値が不正である

			_SYDNEY_VERIFY_INCONSISTENT(
				result, getParent(), Message::OlderTimeStampInconsistent());
			return;
		}

		const Log& log = Log::get(logMemory);

		if ((pageID == 0 || log._pageID != 0) && log._pageID != pageID)
		{
			// 違うページの版が保存されている

			_SYDNEY_VERIFY_INCONSISTENT(
				result, getParent(),
				Message::PreservedDifferentPage(pageID, log._pageID));
			return;
		}

		// 今調べたバージョンログをおぼえておく

		bitmap.set(id, true);

		if (verification.getVersionNumber() >= VersionNumber::Second)

			// バージョン２
			// リンク先のブロックが必ず存在しているわけでない

			break;

		// ひとつ古い版に対する物理ログがあれば、検査する

		Admin::Verification::Progress progress(result.getConnection());
		verifyPhysicalLog(
			trans, verification, log._physicalLog, log._older, progress);
		result += progress;

		if (!progress.isGood())
			return;

		if ((older = log._olderTimeStamp) < oldest)

			// 最古のバージョンログまで調べた

			break;

		// ひとつ古い版のバージョンログを調べることにする

		id = log._older;

	} while (id != Block::IllegalID) ;
}

//	FUNCTION private
//	Version::VersionLog::File::verifyPhysicalLog --
//		あるバージョンログに対するすべての物理ログに対して整合性検査する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			整合性検査を行うトランザクションのトランザクション記述子
//		Version::Verification&	verification
//			あるトランザクションの整合性検査に関する情報を表すクラス
//		Version::Block::ID	id
//			あるバージョンログに対する最新の物理ログのブロック識別子
//		Version::Block::ID	older
//			このブロック識別子のバージョンログに対する
//			すべての物理ログに対して整合性検査する
//		Admin::Verification::Progress&	result
//			整合性検査の経過を設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::verifyPhysicalLog(
	const Trans::Transaction& trans, Verification& verification, Block::ID id,
	Block::ID older, Admin::Verification::Progress& result)
{
	// 指定されたブロック識別子のバージョンログに対する物理ログを、
	// 指定されたブロック識別子の最新のものから、最古のものに向かって、
	// ひとつひとつ古い物理ログをフィックスしていく

	Common::BitSet bitmap;

	while (id != Block::IllegalID) {
		if (_Log::isIllegalID(id) || bitmap.test(id)) {

			// 物理ログのブロック識別子が不正である

			_SYDNEY_VERIFY_INCONSISTENT(
				result, getParent(), Message::PhysicalLogIDInconsistent());
			return;
		}

		// このブロック識別子の物理ログをフィックスする

		const Block::Memory& logMemory =
			Log::fix(trans, &verification, *this, id,
					 Buffer::Page::FixMode::ReadOnly,
					 Buffer::ReplacementPriority::Low);
		const Log& log = Log::get(logMemory);

		if (log._older != older) {

			// 同じバージョンログに対する物理ログでない

			_SYDNEY_VERIFY_INCONSISTENT(
				result, getParent(), Message::OlderNotIdentical());
			return;
		}

		// 今調べた物理ログをおぼえておく

		bitmap.set(id, true);

		// ひとつ古い物理ログのブロック識別子を得る

		id = log._physicalLog;
	}
}

//	FUNCTION private
//	Version::VersionLog::File::verifyAllocationTable --
//		すべてのアロケーションテーブルを整合性検査する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification&	verification
//			あるトランザクションの整合性検査に関する情報を表すクラス
//		Version::VersionLog::FileHeader&	header
//			整合性検査するバージョンログファイルの
//			ファイルヘッダを表すクラス
//		Admin::Verification::Progress&	result
//			整合性検査の経過を設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::verifyAllocationTable(
	const Trans::Transaction& trans, Verification& verification,
	const FileHeader& header, Admin::Verification::Progress& result)
{
	const unsigned int bitCount =
		_AllocationTable::getBitCount(header.getVersion(), getBlockSize());

	// ファイルの先頭から、
	// アロケーションテーブルをひとつひとつフィックスしていく

	for (Block::ID tableID = MultiplexCount;
		 tableID < header._blockCount; tableID += MultiplexCount + bitCount) {

		// このブロック識別子のアロケーションテーブルをフィックスする

		const Block::Memory& tableMemory =
			AllocationTable::fix(trans, &verification, *this, tableID,
								 Buffer::Page::FixMode::ReadOnly);
		const AllocationTable& table = AllocationTable::get(tableMemory);

		// フィックスしたアロケーションテーブルの
		// 使用中のブロックであるかを表すビットマップを調べていく

		Block::ID id = tableID + MultiplexCount;

		for (unsigned int i = 0;
			 i < bitCount && id + i < header._blockCount; ++i) {

			// 整合性検査中にフィックスされた
			// ブロックが使用中であることを確認する

			const bool allocated =
				verification.getBlockBitmap().getBit(id + i);

			if (((header.getVersion() < VersionNumber::Second &&
				  verification.isOverall()) || allocated) &&
				table.getBoundBit(header.getVersion(),
								  getBlockSize(), id + i) != allocated) {

				// 使用中であるはずのブロックがフィックスされなかった、
				// または、使用済であるはずのブロックがフィックスされた

				_SYDNEY_VERIFY_INCONSISTENT(
					result, getParent(), Message::AllocationBitInconsistent());
				return;
			}
		}
	}
}

//	FUNCTION public
//	Version::VersionLog::File::syncLog --
//		あるバージョンページの版のうち、
//		2 つ前のチェックポイント処理より前に生成されたもので、
//		できる限り最新のものを、マスタデータファイルへ複写する
//
//	NOTES
//		マスタデータファイルに複写したバージョンログと
//		それより前のものはすべて使用済になる
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンログを複写するトランザクションのトランザクション記述子
//		Version::Page&		page
//			このバージョンページ記述子の表す
//			バージョンページのバージョンログについて処理する
//		Version::Block::ID	id
//			指定されたバージョンページの最新版のバージョンログのブロック識別子
//		Trans::TimeStamp::Value	oldest
//			指定されたバージョンページの
//			最も古いバージョンログの最終更新時タイムスタンプ値
//		Trans::TimeStamp::Value	eldest
//			前々回のチェックポイント処理の終了したときと、
//			現在実行中の版管理するトランザクションのうち、
//			最も昔に開始されたものの開始されたときの
//			いずれか昔のほうのタイムスタンプ値
//		Version::MasterData::File&	masterData
//			バージョンログを複写する
//			マスタデータファイルのマスタデータファイル記述子
//		Trans::TimeStamp::Value	allocation
//			バージョンログによって上書きされる
//			マスタデータファイルのブロックが確保されたときのタイムスタンプ値
//
//	RETURN
//		Trans::IllegalTimeStamp
//			指定されたバージョンページに関するバージョンログはすべて破棄された
//		oldest
//			指定されたバージョンページに関する
//			最も古いバージョンログは変わらなかった
//		上記以外の値
//			指定されたバージョンページに関する
//			最も古いバージョンログの最終更新時タイムスタンプ値
//
//	EXCEPTIONS

Trans::TimeStamp::Value
VersionLog::
File::syncLog(const Trans::Transaction& trans,
			  Block::Memory& headerMemory, Page& page, Block::ID id,
			  Trans::TimeStamp::Value oldest, Trans::TimeStamp::Value eldest,
			  MasterData::File& masterData, Trans::TimeStamp::Value allocation)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);
	; _SYDNEY_ASSERT(!Trans::TimeStamp::isIllegal(oldest));
	; _SYDNEY_ASSERT(!Trans::TimeStamp::isIllegal(eldest));
	; _SYDNEY_ASSERT(oldest < eldest);
	; _SYDNEY_ASSERT(!Trans::TimeStamp::isIllegal(allocation));

	FileHeader& header = FileHeader::get(headerMemory);

	// 最新版を記録するバージョンログをフィックスする

	const Block::Memory& logMemory =
		Log::fix(trans, 0, *this, id,
				 Buffer::Page::FixMode::ReadOnly,
				 Buffer::ReplacementPriority::Low);
	; _SYDNEY_ASSERT(logMemory.getLastModification() >= oldest);

	if (logMemory.getLastModification() < eldest) {

		// 最新版を記録するバージョンログが最後に更新されてから
		// 前々回のチェックポイント処理が行われ、
		// 現在実行中のすべての版管理するトランザクションは開始された

		// バージョンページの
		// ページ更新トランザクションリストを保護するためにラッチする

		Os::AutoCriticalSection	latch(page.getLatch());

		Schema::ObjectID::Value dbid
			= page.getFile().getLockName().getDatabasePart();

		if (!Trans::Transaction::isInProgress(
				dbid,
				page.getModifierList(),
				Trans::Transaction::Category::ReadWrite) &&
			!_Transaction::isRefered(
				dbid,
				logMemory.getLastModification(),
				page.getModifierList())) {

			// 最新版を更新したトランザクションはすべて終了しており、
			// 最新版を参照する版管理するトランザクションは存在しない

			// ページ更新トランザクションリストは不要なので、
			// ページ更新トランザクションリストを空にする
			page._modifierList.clear();

			// 最新版のバージョンログの内容をマスタデータファイルへ反映する

			(void) masterData.syncData(
				trans, static_cast<Block::ID>(page.getID()),
				logMemory, allocation);

			// すべてのバージョンログを使用済にする

			freeLog(trans, header.getVersion(), 0, id, oldest);

			return Trans::IllegalTimeStamp;
		}
	}

	if (header.getVersion() < VersionNumber::Second)
	{

		const Log& log = Log::get(logMemory);

		if (oldest < log._olderTimeStamp) {

			// 最新版のバージョンログより古いものがあり、
			// それはどうも最古のものでないらしい

			// ひとつ古い版のバージョンログを調べることにする

			id = log._older;

			// 先ほど調べたバージョンログのバッファリング内容を記憶しておく

			Block::Memory prev(logMemory);

			while (id != Block::IllegalID) {

				// このブロック識別子のバージョンログをフィックスする

				const Block::Memory& logMemory =
					Log::fix(trans, 0, *this, id,
							 Buffer::Page::FixMode::ReadOnly,
							 Buffer::ReplacementPriority::Low);
				const Log& log = Log::get(logMemory);

				if (log._olderTimeStamp < oldest)

					// このバージョンログを読み出してみたところ、
					// 実は最古のものだった

					return logMemory.getLastModification();

				if (logMemory.getLastModification() < eldest) {

					// このバージョンログが最後に更新されてから
					// 前々回のチェックポイント処理が行われ、
					// 現在実行中のすべての版管理するトランザクションは
					// 開始された

					// このバージョンログより古いものが存在するので、
					// すべて使用済にする
					//
					//【注意】	ひとつ古い版に対する物理ログについては、
					//			このバージョンログを使用済にするときに、
					//			同じく使用済にする

					freeLog(trans, header.getVersion(), 0, log._older, oldest);

					// このバージョンログを最古にする

					return logMemory.getLastModification();
				}

				// このバージョンログを最古にできなかった

				if (log._olderTimeStamp == oldest)

					// ひとつ古い版のバージョンログが最古のもの

					break;

				id = log._older;
				prev = logMemory;
			}
		}
	}

	// 最も古いバージョンログは変わらなかった

	return oldest;
}

//	FUNCTION private
//	Version::VersionLog::File::allocate --
//		ブロックを(必要があれば確保し、)使用中にする
//
//	NOTES
//		使用中にする連続したブロックの数として
//		0 を指定したときの動作は保証しない
//
//		確保した連続するブロックのうちの最初のものの
//		ブロック識別子は必ず n で割り切れる
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				ブロックを確保するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				ブロックを確保するトランザクションは整合性検査中でない
//		Version::VersionLog::MultiplexBlock& headerMulti
//			ファイルヘッダを格納する多重化されたブロックに関する情報
//		unsigned int		n
//			指定されたとき
//				使用中にする連続したブロックの数
//			指定されないとき
//				1 が指定されたものとみなす
//
//	RETURN
//		確保した連続するブロックのうちの最初のもののブロック識別子
//
//	EXCEPTIONS

Block::ID
VersionLog::
File::allocate(const Trans::Transaction& trans,
			   Verification* verification,
			   MultiplexBlock& headerMulti, unsigned int n)
{
	; _SYDNEY_ASSERT(n);

	// 存在するブロック数を求める
	//
	//【注意】	リカバリによって将来回復されるブロックを再確保すると、
	//			リカバリできなくなってしまう
	//			そこでそれらのブロックも勘定に入れるために、
	//			多重化されたファイルヘッダーをすべてフィックスし、
	//			最大のブロック数を求める

	FileHeader* headers[MultiplexCount];
	unsigned int maxBlockCount = 0;
	VersionNumber::Value v = VersionNumber::First;

	unsigned int i = 0;
	do {
		if ((headers[i] = (headerMulti._memories[i].isOwner() ?
						   &FileHeader::get(headerMulti._memories[i]) : 0)) &&
			maxBlockCount < headers[i]->_blockCount)
		{
			maxBlockCount = headers[i]->_blockCount;
			v = headers[i]->getVersion();
		}
	} while (++i < MultiplexCount) ;

	const unsigned int bitCount =
		_AllocationTable::getBitCount(v, getBlockSize());
	const unsigned int bitmapLength =
		_AllocationTable::getBitmapLength(v, getBlockSize());

	Trans::TimeStamp::Value second = Trans::IllegalTimeStamp;
	int needApply = -1;

	Block::ID tableID = MultiplexCount;
	for (; tableID < maxBlockCount; tableID += bitCount + MultiplexCount) {

		// アロケーションテーブルをフィックスしながら、
		// 未使用または使用済のブロックの数を求める

		MultiplexBlock tableMulti;
		AllocationTable::fix(trans, verification, *this, tableID,
							 Buffer::Page::FixMode::Write, tableMulti);

		if (v >= VersionNumber::Second)
		{
			// 使用済みのブロックが反映されていなかったら、反映する
			
			AllocationTable& t = AllocationTable::get(
				tableMulti._memories[tableMulti._master]);
			
			if (t.isApplyFree() == false)
			{
				if (needApply == -1)
				{
					// まだ反映されていない
					// 前々回のチェックポイント処理終了のタイムスタンプから、
					// 現在実行中のトランザクションが途切れることなく
					// 実行されているかチェックし、
					// 途切れていれば、使用済みのブロックを反映する

					second = Checkpoint::TimeStamp::getSecondMostRecent(
						_versionFile.getLockName());

					if (Trans::Transaction::getBeginningID(
							_versionFile.getLockName().getDatabasePart())
						> second)

						// トランザクションが途切れているので反映できる
						
						needApply = 1;

					else

						// トランザクションが途切れていないので反映できない
						// 何回も調べないようにフラグを設定

						needApply = 0;

				}

				if (needApply == 1)

					// 使用済みを反映する
					
					AllocationTable::applyFree(tableMulti, second,
											   v, getBlockSize());

			}
		}

		AllocationTable* tables[MultiplexCount];
		unsigned int maxCount = 0;

		unsigned int i = 0;
		do {
			if ((tables[i] = (tableMulti._memories[i].isOwner() ?
						&AllocationTable::get(tableMulti._memories[i]) : 0)) &&
				maxCount < tables[i]->getCount())
				maxCount = tables[i]->getCount();
		} while (++i < MultiplexCount) ;

		if (n > bitCount - maxCount)

			// このアロケーションテーブルで管理する
			// 未使用または使用済のブロックでは、
			// 指定された数には足りない

			continue;

		// アロケーションテーブルの
		// 使用中のブロックであるかを表すビットマップを調べていく

		unsigned int rest = n;

		unsigned int j = 0;
		do {
			unsigned int bitmap = 0;

			unsigned int i = 0;
			do {
				if (tables[i])
					bitmap |= tables[i]->_bitmap[j];
			} while (++i < MultiplexCount) ;

			if (bitmap == ~static_cast<unsigned int>(0)) {

				// このビットマップはすべて立っているので、
				// これまでに見つけているものはチャラにする

				rest = n;
				continue;
			}

			unsigned int k = 0;
			unsigned int m = 1;

			do {
				if (bitmap & m)

					// このビットは立っているので、
					// これまで見つけているものはチャラにする

					rest = n;

				else if (!--rest) {

					// 指定された数の連続した
					// 未確保または使用済のブロックが見つかった

					// 見つかった連続したブロックのうち、
					// 最初のものと、最後の次のものの
					// ブロック識別子を求める

					Block::ID end = tableID + MultiplexCount +
						_AllocationTable::_bitCountPerBitmap * j + k + 1;
					Block::ID begin = end - n;

					if (begin % n)

						// 見つかった連続したブロックの最初のものの
						// ブロック識別子は n で割り切れないので、
						// 次のブロックが空いているか調べる

						rest = 1;
					else {
						if (end > maxBlockCount)

							// 見つかったブロックのうちのいくつかは
							// 未確保のブロックなので、
							// それらを確保できるようにファイルを拡張する

							extend(end);

						if (end > headers[headerMulti._master]->_blockCount) {

							// 未使用ブロックを使うときは、
							// ヘッダの総ブロック数を変更して、
							// それらを使用済のブロックとする

							headers[headerMulti._master]->_blockCount = end;
							headerMulti._memories[headerMulti._master].dirty();
						}

						// 見つかったブロックを使用中になるように
						// アロケーションテーブルのビットを立てる

						; _SYDNEY_ASSERT(tables[tableMulti._master]);
						tables[tableMulti._master]->setBoundBit(
							v, verification, getBlockSize(), begin, n, true);
						tables[tableMulti._master]->setNewestBit(
							v, verification, getBlockSize(), begin, n, true);
						tableMulti._memories[tableMulti._master].dirty();

						return begin;
					}
				}

				m <<= 1;

			} while (++k < _AllocationTable::_bitCountPerBitmap) ;
		} while (++j < bitmapLength) ;
	}

	// 指定された数の連続した未使用または使用済ブロックは
	// 既存のアロケーションテーブルで管理されるブロックには存在しない

	// 新しいアロケーションテーブルを確保する前に、
	// その直前のブロックまでのいくつかのブロックは
	// 未確保のブロックかもしれないので、
	// それらを確保できるようにファイルを拡張する

	extend(tableID);

	// ヘッダの総ブロック数を変更して、
	// それらを使用済のブロックとする

	headers[headerMulti._master]->_blockCount = tableID;
	headerMulti._memories[headerMulti._master].dirty();

	// 新しいアロケーションテーブルを確保する

	Block::Memory tableMemory(
		AllocationTable::allocate(trans, verification, *this, headerMulti));
	; _SYDNEY_ASSERT(tableMemory.isDirty());

	// 確保したアロケーションテーブルで管理するブロックのうち、
	// 最初のものと、それから指定された数ぶん後のものの
	// ブロック識別子を求める
	//
	//【注意】	最初のブロック識別子は n で割り切れる必要がある

	Block::ID begin = (tableID + MultiplexCount + n - 1) / n * n;
	Block::ID end = begin + n;

	// 最初のものから指定された数ぶんの
	// 連続したブロックは未確保なので、
	// それらを確保できるようにファイルを拡張する

	extend(end);

	// ヘッダの総ブロック数を変更して、
	// それらを使用済のブロックとする

	headers[headerMulti._master]->_blockCount = end;
	; _SYDNEY_ASSERT(headerMulti._memories[headerMulti._master].isDirty());

	// それらが使用中になるように
	// アロケーションテーブルのビットを立てる

	AllocationTable::get(tableMemory).setBoundBit(
		v, verification, getBlockSize(), begin, n, true);
	AllocationTable::get(tableMemory).setNewestBit(
		v, verification, getBlockSize(), begin, n, true);

	return begin;
}

//	FUNCTION private
//	Version::VersionLog::File::free -- ブロックを使用済にする
//
//	NOTES
//		ファイルヘッダおよびアロケーションテーブルの
//		ブロック識別子が与えられたとき、動作は保証しない
//
//		使用中にする連続したブロックの数として
//		0 を指定したときの動作は保証しない
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				ブロックを使用済にするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				ブロックを使用済にするトランザクションは整合性検査中でない
//		Version::Block::ID	id
//			使用済にする連続したブロックの先頭のもののブロック識別子
//		unsigned int		n
//			指定されたとき
//				使用済にする連続したブロックの数
//			指定されないとき
//				1 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::free(const Trans::Transaction& trans,
		   VersionNumber::Value v,
		   Verification* verification, Block::ID id, unsigned int n)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);
	; _SYDNEY_ASSERT(n == 1 || id == id / n * n);

	// 指定されたブロック識別子のブロックを管理する
	// アロケーションテーブルのブロック識別子を求める

	Block::ID tableID = _AllocationTable::roundBlockID(v, getBlockSize(), id);
	; _SYDNEY_ASSERT(tableID != id);

	// アロケーションテーブルをフィックスする

	Block::Memory tableMemory(
		AllocationTable::fix(
			trans, verification, *this, tableID, Buffer::Page::FixMode::Write));

	// アロケーションテーブルの指定されたブロック識別子から
	// 連続して n 個のブロックの使用中かを表すビットを落とす

	AllocationTable::get(tableMemory).setBoundBit(
		v, verification, getBlockSize(), id, n, false);
	AllocationTable::get(tableMemory).setNewestBit(
		v, verification, getBlockSize(), id, n, false);
	tableMemory.dirty();
}

//	FUNCTION private
//	Version::VersionLog::File::setNewest -- ブロックが最新かどうかを設定する
//
//	NOTES
//		ファイルヘッダおよびアロケーションテーブルの
//		ブロック識別子が与えられたとき、動作は保証しない
//
//		使用中にする連続したブロックの数として
//		0 を指定したときの動作は保証しない
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//			   	実行するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				実行するトランザクションは整合性検査中でない
//		Version::Block::ID	id
//			ブロックのブロック識別子
//		bool on
//			true	最新のブロックの場合
//			false	最新のブロックではない場合
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::setNewest(const Trans::Transaction& trans, VersionNumber::Value v,
				Verification* verification, Block::ID id, bool on)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	// 指定されたブロック識別子のブロックを管理する
	// アロケーションテーブルのブロック識別子を求める

	Block::ID tableID = _AllocationTable::roundBlockID(v, getBlockSize(), id);
	; _SYDNEY_ASSERT(tableID != id);

	// アロケーションテーブルをフィックスする

	Block::Memory tableMemory(
		AllocationTable::fix(
			trans, verification, *this, tableID, Buffer::Page::FixMode::Write));

	// アロケーションテーブルの指定されたブロック識別子の
	// 最新かどうかのビットを設定する

	AllocationTable::get(tableMemory).setNewestBit(
		v, verification, getBlockSize(), id, 1, on);
	tableMemory.dirty();
}

//	FUNCTION public
//	Version::VersionLog::File::traversePBCT --
//		あるバージョンページの最新版を表す
//		バージョンログのブロック識別子を記録する PBCT リーフを得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				ある最新版を表すバージョンログのブロック識別子を記録する
//				PBCT リーフを探すトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				ある最新版を表すバージョンログのブロック識別子を記録する
//				PBCT リーフを探すトランザクションは整合性検査中でない
//		Block::Memory&		headerMemory
//			ファイルヘッダのバッファリング内容
//		Version::Page::ID	pageID
//			このバージョンページ識別子のバージョンページの最新版を表す
//			バージョンログのブロック識別子を記録する PBCT リーフを探す
//		Buffer::Page::FixMode::Value	mode
//			Buffer::Page::FixMode::ReadOnly
//				見つかった PBCT リーフは参照用にフィックスされる
//				このとき、headerMemory は Buffer::Page::FixMode::ReadOnly で
//				フィックスされていなければならない
//			Buffer::Page::FixMode::Write または
//			Buffer::Page::FixMode::Allocate
//				見つかった PBCT リーフは更新用にフィックスされる
//				このとき、headerMemory は Buffer::Page::FixMode::
//
//	RETURN
//		見つかった PBCT リーフのバッファリング内容
//
//		ただし、それに対する Version::Block::Memory::getID の結果が
//		0 のとき、PBCT リーフが存在しないことを表し、
//		その場合、指定されたバージョンページ識別子の
//		バージョンページの最新版は、バージョンログファイルに格納されていない
//
//	EXCEPTIONS

Block::Memory
VersionLog::
File::traversePBCT(const Trans::Transaction& trans,
				   Verification* verification,
				   const Block::Memory& headerMemory,
				   Page::ID pageID, Buffer::Page::FixMode::Value mode)
{
	if (mode == Buffer::Page::FixMode::Allocate)
		mode = Buffer::Page::FixMode::Write;
	; _SYDNEY_ASSERT(headerMemory.isUpdatable() ==
					 (mode != Buffer::Page::FixMode::ReadOnly));
	const FileHeader& header = FileHeader::get(headerMemory);

	if (!header.isPBCTEmpty())

		// PBCT が存在する

		if (!header.getPBCTLevel()) {

			// PBCT にはルートリーフしかない

			if (pageID < PBCTLeaf::getCountMax(false, getBlockSize()))

				// ルートリーフだけで管理できるバージョンページの最大数より、
				// 指定されたバージョンページ識別子が小さい

				// ファイルヘッダを記録するブロックをフィックスし、返す
				//
				//【注意】	与えられたファイルヘッダをそのまま返すと、
				//			与えられたファイルヘッダの所有権を失うので、
				//			フィックスしなおす必要がある
				//
				//【注意】	同じスレッドで同じブロックを
				//			複数回フィックスするのは例外的である
				//
				//			呼び出し側で気をつけないと更新内容が失われる

				return headerMemory.refix();

		} else if (PBCTNode::getCountMax(false, getBlockSize()) *
				   _Math::pow(PBCTNode::getCountMax(true, getBlockSize()),
							  header.getPBCTLevel() - 1) *
				   PBCTLeaf::getCountMax(true, getBlockSize()) > pageID) {

			// 現在の PBCT で管理できるバージョンページの最大数より、
			// 指定されたバージョンページ識別子が小さい

			// 与えられたバージョンページの版をたどるための
			// 子供のノード、リーフを記録するブロックのブロック識別子を得る

			Block::ID id = PBCTNode::get(headerMemory).getChildID(
				pageID, 0, header.getPBCTLevel(), getBlockSize());

			for (PBCTLevel::Value i = 1; id != Block::IllegalID; ++i) {
				if (i == header.getPBCTLevel())

					// 見つかったリーフをフィックスし、返す

					return PBCTLeaf::fix(
						trans, verification, *this, id,
						(mode != Buffer::Page::FixMode::Allocate) ?
						mode : Buffer::Page::FixMode::Write);

				// 今調べているノードをフィックスし、
				// 次に調べるべき子供のブロック識別子を得る

				const Block::Memory& nodeMemory =
					PBCTNode::fix(trans, verification, *this, id,
								  Buffer::Page::FixMode::ReadOnly);

				id = PBCTNode::get(nodeMemory).getChildID(
					pageID, i, header.getPBCTLevel(), getBlockSize());
			}
		}

	// リーフまでたどれなかった

	return Block::Memory();
}

//	FUNCTION public
//	Version::VersionLog::File::traverseLog --
//		あるバージョンページの版のうち、
//		あるトランザクションが参照すべきものを記録するバージョンログを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			参照すべき版を表すバージョンログを得る
//			トランザクションのトランザクション記述子
//		Version::Verification*	verification
//			0 以外の値
//				参照すべき版を表すバージョンログを得るトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				参照すべき版を表すバージョンログを得る
//				トランザクションは整合性検査中でない
//		Version::Page&		page
//			参照するバージョンページのバージョンページ識別子
//		Version::Block::ID	id
//			最新版のバージョンログのブロック識別子
//		Trans::TimeStamp::Value	oldest
//			最古のバージョンログの最終更新時タイムスタンプ値
//		Buffer::ReplacementPriority::Value	priority
//			Buffer::ReplacementPriority::Low
//				フィックスするバージョンログは、バッファから破棄されやすい
//			Buffer::ReplacementPriority::Middle
//				フィックスするバージョンログは、バッファにある程度残る
//			Buffer::ReplacementPriority::High
//				フィックスするバージョンログは、バッファからかなりの間残る
//
//	RETURN
//		見つかったバージョンログのバッファリング内容
//
//		ただし、それに対する Version::Block::Memory::getID の結果が
//		0 のとき、バージョンログが見つからなかったことを表し、
//		その場合、指定されたトランザクションが参照すべき版は
//		バージョンログファイルに格納されていない
//
//	EXCEPTIONS

Block::Memory
VersionLog::
File::traverseLog(const Trans::Transaction& trans,
				  Verification* verification, const Page& page,
				  Block::ID id, Trans::TimeStamp::Value oldest,
				  Buffer::ReplacementPriority::Value priority)
{
	if (id != Block::IllegalID) {

		// 最新版を記録するバージョンログをフィックスする
		/*
		  【注意】	探索中も指定された優先度で
					フィックスしてしまうが、現状ではしょうがない
		*/
		const Block::Memory& logMemory =
			Log::fix(trans, verification, *this, id,
					 Buffer::Page::FixMode::ReadOnly, priority);

		const Log& log = Log::get(logMemory);

		if ((log._pageID != 0 || page.getID() == 0) &&
			log._pageID != page.getID())
				
			// 違うページIDのページが格納されている
			_SYDNEY_THROW3(Exception::PreservedDifferentPage,
						   _bufFile->getPath(),
						   page.getID(), log._pageID);

		if (trans.isNoVersion())

			// 版管理しないトランザクションは、必ず、最新版を参照する

			return logMemory;

		// バージョンページの
		// ページ更新トランザクションリストを保護するためにラッチする

		Os::AutoCriticalSection	latch(page.getLatch());

		// 版管理するトランザクションを開始してから、
		// 最新版を記録するバージョンログが更新されていないか調べる

		if (trans.getBirthTimeStamp() > logMemory.getLastModification()) {

			// 版管理するトランザクションを開始してから最新版は更新されていない

			// 最新版は直前のチェックポイント以降に更新されたか調べる

			const Trans::TimeStamp::Value first =
				Checkpoint::TimeStamp::getMostRecent(
					_versionFile.getLockName());

			if (log._category == Log::Category::Copy &&
				logMemory.getLastModification() > first)

				// 現在の最新版は直前のチェックポイント以降に
				// 生成されたままで、同じイメージが直前の版に存在しているので、
				// この最新版は更新トランザクションが利用する

				goto older;
			
			if (trans.getStartingList().getSize()) {

				// 版管理するトランザクションの開始時に
				// 実行されていた更新トランザクションのうち、
				// 最新版を更新したものがないか調べる

				if (trans.isOverlapped(page.getModifierList()))

					// 参照すべき版は直前の版である

					goto older;
			}

			// 参照すべき版は最新版である

			return logMemory;
		}
		
older:
		if (oldest != Trans::IllegalTimeStamp && log._olderTimeStamp < oldest)

			// 最新版のバージョンログしかなかった

			return Block::Memory();

		// ひとつ古い版のバージョンログを調べることにする

		id = log._older;
		Trans::TimeStamp::Value timestamp = log._olderTimeStamp;
	

		while (id != Block::IllegalID) {

			// このブロック識別子のバージョンログをフィックスする
			/*
			  【注意】	探索中も指定された優先度で
						フィックスしてしまうが、現状ではしょうがない
			*/
			const Block::Memory& logMemory =
				Log::fix(trans, verification, *this, id,
						 Buffer::Page::FixMode::ReadOnly, priority);

			const Log& log = Log::get(logMemory);

			if ((log._pageID != 0 || page.getID() == 0) &&
				log._pageID != page.getID())
				
				// 違うページIDのページが格納されている
				_SYDNEY_THROW3(Exception::PreservedDifferentPage,
							   _bufFile->getPath(),
							   page.getID(), log._pageID);
			

			if (logMemory.getLastModification() != timestamp)
			{
				// ひとつ新しい版のリンク情報のタイムスタンプと
				// この版のタイムスタンプは同じはずであるが、
				// リンクが切れていることがあり、タイムスタンプがことなっている
				// 通常ありえないが、olderがおかしいことがあるので、
				// ここでチェックする
				
				break;
			}

			// バージョンページを参照しようとしている
			// 版管理するトランザクションの開始時タイムスタンプより
			// 今調べている版の最終更新時タイムスタンプのほうが小さいか調べる

			if (trans.getBirthTimeStamp() >	logMemory.getLastModification())

				// 参照すべき版が見つかった

				return logMemory;

			if (oldest != Trans::IllegalTimeStamp &&
				log._olderTimeStamp < oldest)
				break;

			id = log._older;
			timestamp = log._olderTimeStamp;
		}
	}

	// 参照すべき版を記録するバージョンログは存在しない

	return Block::Memory();
}

//	FUNCTION public
//	Version::VersionLog::File::allocatePBCT --
//		あるバージョンページの最新版を表す
//		バージョンログのブロック識別子を記録する PBCT リーフを確保する
//
//	NOTES
//		PBCT のルート以下で探した結果、見つからなければ、
//		記録できるように複数のノード、リーフを確保する
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				ある最新版を表すバージョンログのブロック識別子を記録する
//				PBCT リーフを確保するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				ある最新版を表すバージョンログのブロック識別子を記録する
//				PBCT リーフを確保するトランザクションは整合性検査中でない
//		Version::VersionLog::MultiplexBlock& headerMulti
//			ファイルヘッダを格納する多重化されたブロックに関する情報
//		Version::Page::ID	pageID
//			このバージョンページ識別子のバージョンページの最新版を表す
//			バージョンログのブロック識別子を記録する PBCT リーフを確保する
//
//	RETURN
//		確保された PBCT リーフのバッファリング内容
//
//	EXCEPTIONS

Block::Memory
VersionLog::
File::allocatePBCT(const Trans::Transaction& trans,
				   Verification* verification,
				   MultiplexBlock& headerMulti, Page::ID pageID)
{
	Block::Memory& headerMemory = headerMulti._memories[headerMulti._master];
	; _SYDNEY_ASSERT(headerMemory.isUpdatable());
	FileHeader&	header = FileHeader::get(headerMemory);

	; _SYDNEY_ASSERT(pageID < header.getPageCount());

	// 指定されたバージョンページの最新版を記録する
	// バージョンログのブロック識別子を記録するには、
	// PBCT にレベルがどれだけ必要か調べる

	const unsigned int n = PBCTNode::getCountMax(true, getBlockSize());
	const unsigned int l = PBCTLeaf::getCountMax(true, getBlockSize());

	PBCTLevel::Value level = 0;
	for (unsigned int m = PBCTLeaf::getCountMax(false, getBlockSize());
		 m <= pageID; ++level)
		if (level)
			m *= n;
		else
			m = PBCTNode::getCountMax(false, getBlockSize()) * l;

	if (header.isPBCTEmpty()) {
		
		// 既存の PBCT は空なので、ルートリーフを確保する

		header._PBCTLevel = 0;
		headerMemory.dirty();
	}

	// 既存の PBCT ではレベルが足りなければ、
	// 足りないぶん増やして、後の処理で増えないようにする
	//
	//【注意】	バージョンファイル内部の使い方では
	//			一度に 2 段以上増やすことはないはず

	while (header.getPBCTLevel() < level) {

		PBCTNode& root = PBCTNode::get(headerMemory);
		Block::ID id;

		if (header.getPBCTLevel()) {

			// 新しいノードを確保し、そのブロック識別子を得る

			Block::Memory	nodeMemory(
				PBCTNode::allocate(trans, verification, *this, headerMulti));
			; _SYDNEY_ASSERT(nodeMemory.isDirty());
			id = nodeMemory.getID();
			; _SYDNEY_ASSERT(id != Block::IllegalID);

			// ファイルヘッダーを記録するブロック上のノードで
			// 確保したノードを上書きする

			PBCTNode::get(nodeMemory).copy(root, false, getBlockSize());
		} else {

			// 新しいリーフを確保し、そのブロック識別子を得る

			Block::Memory	leafMemory(
				PBCTLeaf::allocate(trans, verification, *this, headerMulti));
			; _SYDNEY_ASSERT(leafMemory.isDirty());
			id = leafMemory.getID();
			; _SYDNEY_ASSERT(id != Block::IllegalID);

			// ファイルヘッダーを記録するブロック上のリーフで
			// 確保したリーフを上書きする

			PBCTLeaf::get(leafMemory).copy(
				PBCTLeaf::get(headerMemory), false, getBlockSize());
		}

		// ファイルヘッダの PBCT のレベルを 1 増やす

		++header._PBCTLevel;

		// ファイルヘッダーを記録するブロック上にノードを初期化し、
		// 新しいノード、リーフのブロック識別子を記録する

		root.initialize(false, getBlockSize());
		root.setChildID(0, 0, id, header.getPBCTLevel(), getBlockSize());

		headerMemory.dirty();
	}

	// ルートから、必要に応じてノード、リーフを確保しながら、
	// リーフに向かって探索する

	if (!header.getPBCTLevel())

		// PBCT にはリーフだけがあれば問題ないので、
		// ファイルヘッダを記録するブロックをフィックスし、返す
		//
		//【注意】	与えられたファイルヘッダをそのまま返すと、
		//			与えられたファイルヘッダの所有権を失うので、
		//			フィックスしなおす必要がある
		//
		//【注意】	同じスレッドで同じブロックを
		//			複数回フィックスするのは例外的である
		//
		//			呼び出し側で気をつけないと更新内容が失われる

		return headerMemory.refix();

	// 指定されたバージョンページの最新版を記録する
	// バージョンログのブロック識別子を得るために辿るべき
	// ルートノードの子供のブロック識別子を得る

	Block::ID id = PBCTNode::get(
		static_cast<const Block::Memory&>(
			headerMemory)).getChildID(
				pageID, 0, header.getPBCTLevel(), getBlockSize());

	Block::Memory parentMemory(headerMemory.refix());

	for (PBCTLevel::Value i = 1; true; ++i) {
		if (id == Block::IllegalID) {
			if (i == header.getPBCTLevel()) {

				// 今調べているリーフが存在しないので、確保する

				const Block::Memory& leafMemory =
					PBCTLeaf::allocate(trans, verification, *this, headerMulti);
				; _SYDNEY_ASSERT(leafMemory.isDirty());

				// 確保したリーフのブロック識別子を親ノードに記録する

				PBCTNode::get(parentMemory).setChildID(
					pageID, i - 1, leafMemory.getID(),
					header.getPBCTLevel(), getBlockSize());
				parentMemory.dirty();

				// 確保したリーフを返す

				return leafMemory;
			} else {

				// 今調べているノードが存在しないので、確保する

				const Block::Memory& nodeMemory =
					PBCTNode::allocate(trans, verification, *this, headerMulti);
				; _SYDNEY_ASSERT(nodeMemory.isDirty());

				// 確保したノードのブロック識別子を親ノードに記録する

				PBCTNode::get(parentMemory).setChildID(
					pageID, i - 1, nodeMemory.getID(),
					header.getPBCTLevel(), getBlockSize());
				parentMemory.dirty();

				// 確保したノードを次に調べる子供の親としておぼえておく

				parentMemory = nodeMemory;
			}
		} else {
			if (i == header.getPBCTLevel())

				// 今調べているリーフをフィックスし、返す

				return PBCTLeaf::fix(
					trans,
					verification, *this, id, Buffer::Page::FixMode::Write);
			else {

				// 今調べているノードをフィックスし、
				// 次に調べる子供の親としておぼえておく

				parentMemory = PBCTNode::fix(
					trans,
					verification, *this, id, Buffer::Page::FixMode::Write);

				// 指定されたバージョンページの最新版を記録する
				// バージョンログのブロック識別子を得るために辿るべき
				// フィックスしたノードの子供のブロック識別子を得る

				id = PBCTNode::get(
					static_cast<const Block::Memory&>(
						parentMemory)).getChildID(
							pageID, i, header.getPBCTLevel(), getBlockSize());
			}
		}
	}

	; _SYDNEY_ASSERT(false);
	return parentMemory;
}

//	FUNCTION public
//	Version::VersionLog::File::allocateLog --
//		あるバージョンページの新たな最新版を表すバージョンログを確保する
//
//	NOTES
//		バージョンログを確保後、チェックポイント処理が行われ、
//		かつ、そのバージョンログを更新すると、バージョンログファイルの
//		整合性が失われる可能性がある
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			新たな最新版を表すバージョンログを確保する
//			更新トランザクションのトランザクション記述子
//		Version::Verification*	verification
//			0 以外の値
//				新たな最新版を表すバージョンログを確保するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				新たな最新版を表すバージョンログを確保する
//				トランザクションは整合性検査中でない
//		Version::VersionLog::MultiplexBlock& headerMulti
//			ファイルヘッダを格納する多重化されたブロックに関する情報
//		Version::Page&		page
//			このバージョンページ記述子のバージョンページの
//			新たな最新版を表すバージョンログを確保する
//		Version::Block::Memory&	src
//			あるバージョンページの現在の最新版のバッファリング内容
//		Trans::TimeStamp::Value	oldest
//			最古のバージョンログの最終更新時タイムスタンプ値
//		Buffer::Replacement::Priority::Value priority
//			Buffer::ReplacementPriority::Low
//				確保するバージョンログは、バッファから破棄されやすい
//			Buffer::ReplacementPriority::Middle
//				確保するバージョンログは、バッファにある程度残る
//			Buffer::ReplacementPriority::High
//				確保するバージョンログは、バッファからかなりの間残る
//
//	RETURN
//		確保されたバージョンログのバッファリング内容
//
//	EXCEPTIONS

Block::Memory
VersionLog::
File::allocateLog(const Trans::Transaction& trans, Verification* verification,
				  MultiplexBlock& headerMulti, Page& page,
				  Block::Memory& src, Trans::TimeStamp::Value oldest,
				  Buffer::ReplacementPriority::Value priority)
{
	; _SYDNEY_ASSERT(trans.getCategory() ==
					 Trans::Transaction::Category::ReadWrite);
	; _SYDNEY_ASSERT(!src.isDirty());

	// 最新版は直前のチェックポイント以降に更新されたか調べる

	const Trans::TimeStamp::Value first =
		Checkpoint::TimeStamp::getMostRecent(_versionFile.getLockName());
	const bool afterMostRecent = (src.getLastModification() > first);

	Log& srcLog = Log::get(src);
	if ((srcLog._pageID != 0 || page.getID() == 0) &&
		srcLog._pageID != page.getID())

		// 違うページIDのページが格納されている

		_SYDNEY_THROW3(Exception::PreservedDifferentPage,
					   _bufFile->getPath(),
					   page.getID(), srcLog._pageID);

	if ((srcLog._category == Log::Category::Copy ||
		 page._file->isBatchInsert() ) && afterMostRecent)

		// 現在の最新版は直前のチェックポイント以降に
		// 生成されたままで、同じイメージが直前の版に存在しているので、
		// あるいは、バッチインサート中なので、
		// 新しい最新版を生成する必要はない

		return src;

	bool inProgress = false;
	bool overlapped = false;
	Trans::TimeStamp start;

	// 指定されたバージョンページの
	// ページ更新トランザクションリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(page.getLatch());

	// データベースIDを得る
	
	Schema::ObjectID::Value dbid
		= page.getFile().getLockName().getDatabasePart();

	if (page.getModifierList().getSize()) {

		// 指定されたバージョンページの最新版は、
		// 生成されてから更新されたことがある

		// 最新版を更新した更新トランザクションが実行中か調べる
		//
		//【注意】	ページ更新トランザクションリスト中の
		//			トランザクション識別子は、それの表す
		//			トランザクションの開始時刻の昇順に並んでいる

		inProgress = _Transaction::isInProgress(trans,
												page.getModifierList(),
												afterMostRecent);

		if (!inProgress)
		{
			// 最新版を更新した更新トランザクションが終了しているとき、
			// 最新版を最後に更新してから、
			// 最新版を更新した更新トランザクションがすべて終了するまでに
			// 開始された版管理するトランザクションのうち、
			// 現在実行中のものがあるか調べる
			//
			// また、最新版を更新した更新トランザクションが
			// すべてが終了してから開始されたものがあるか調べ、
			// あれば、その中で一番最初に開始されたものの
			// 開始時タイムスタンプを求める

			overlapped = _Transaction::isOverlapped(
				dbid,
				src.getLastModification(), page.getModifierList(), start);
		}
		else if (afterMostRecent)
		{
			// 最新版が直前のチェックポイント以降に更新されていれば、
			// 新しい最新版を生成する必要はない
			//
			// 同じ版を更新するので、版管理するトランザクションに影響する

			return src;
		}
	}

	// バージョンファイルのバージョンを得る
	VersionNumber::Value v =
		FileHeader::get(headerMulti._memories[headerMulti._master])
		.getVersion();

	// 新しい最新版を表すバージョンログのバッファリング内容
	Block::Memory	allocated;

	// 新しいバージョンログを記録するブロックを確保する
	//
	// ただし、現在の最新版の直前の版が存在し、
	// それは直前のチェックポイント以降に更新されており、
	// 実行中の版管理するトランザクションから参照されていなければ、
	// それを再利用する
	/*
	  【注意】	結果的に最新版にならなくても、指定された優先度になってしまう
	*/
	const bool reuse =
		!(srcLog._older == Block::IllegalID ||
		  (v == VersionNumber::First && srcLog._olderTimeStamp < oldest) ||
		  srcLog._olderTimeStamp < first ||
		  _Transaction::isRefered(dbid, srcLog._olderTimeStamp,
								  src.getLastModification(),
								  page.getModifierList()));
	Block::Memory	dst0((reuse) ?
		Log::fix(trans, verification, *this, srcLog._older,
				 Buffer::Page::FixMode::Write, priority) :
		Log::allocate(trans, verification, *this, headerMulti, priority));
	Log& dstLog0 = Log::get(dst0);

	if (reuse) {

		// 現在の最新版の直前の版を再利用するときは、
		// 2 つ前の版が直前の版になるように最新版に情報を記録する

		srcLog._older = dstLog0._older;
		srcLog._physicalLog = dstLog0._physicalLog;
		srcLog._olderTimeStamp = dstLog0._olderTimeStamp;

		src.dirty();
#ifdef DEBUG
		; _SYDNEY_ASSERT(srcLog._category != Log::Category::Oldest);
		; _SYDNEY_ASSERT(!dst0.isDirty());
	} else {
		; _SYDNEY_ASSERT(!src.isDirty());
		; _SYDNEY_ASSERT(dst0.isDirty());
#endif
	}

	// 新たに確保された版に現在の最新版を複写する

	(void) dst0.copy(src);
	dstLog0._category = Log::Category::Copy;
	dstLog0._pageID = page.getID();

	if (v >= VersionNumber::Second) {

		// 最新版が変更されたので、その情報をアロケーションテーブルに反映する

		if (reuse)

			// dst0 が最新版
			//
			//【注意】
			//	Log::allcoate で確保した版は最新版のフラグはすでに立っている

			setNewest(trans, v, verification, dst0.getID(), true);

		if (srcLog._category != Log::Category::Oldest)
			
			// src は最新版ではない

			setNewest(trans, v, verification, src.getID(), false);

	}

	if (inProgress) {

		// 最新版を更新した更新トランザクションは実行中であり、
		// 最新版は直前のチェックポイント以降に更新されていない

		// 直前の版の再利用はしていないはず

		; _SYDNEY_ASSERT(!reuse);

		// 現在の最新版は物理ログ専用とし、
		// 版管理するトランザクションから参照されないように
		// 新たに確保された版に情報を記録する

		if (srcLog._category != Log::Category::Oldest)
			dstLog0._physicalLog = src.getID();
		else {
			; _SYDNEY_ASSERT(dstLog0._older == Block::IllegalID);
			; _SYDNEY_ASSERT(dstLog0._physicalLog == Block::IllegalID);
			dstLog0._olderTimeStamp = Trans::IllegalTimeStamp;
		}

		// 新たに確保された版を新しい最新版とする

		allocated = dst0;

	} else if (!overlapped) {

		// 最新版を更新した更新トランザクションは終了しており、
		// 最新版を最後に更新してから、
		// 最新版を更新した更新トランザクションがすべて終了するまでに
		// 開始された版管理するトランザクションはすべて終了している

		if (srcLog._category != Log::Category::Oldest) {

			// 直前の版の再利用をしても、現在の最新版の
			// 最終更新時タイムスタンプ値は変更されないようにする

			start = src.getLastModification();
			if (src.isDirty())
				src.unfix(start);

			// 現在の最新版は新たに確保された版の直前の版となるように
			// 新たに確保された版に情報を記録する

			dstLog0._older = src.getID();
			dstLog0._physicalLog = Block::IllegalID;
			dstLog0._olderTimeStamp = start;
			dst0.dirty();
		} else {
			; _SYDNEY_ASSERT(dstLog0._older == Block::IllegalID);
			; _SYDNEY_ASSERT(dstLog0._physicalLog == Block::IllegalID);
			dstLog0._olderTimeStamp = Trans::IllegalTimeStamp;
		}

		// 新たに確保された版を新しい最新版とする

		allocated = dst0;

	} else if (afterMostRecent && srcLog._category != Log::Category::Oldest) {

		// 最新版を更新した更新トランザクションは終了しており、
		// 最新版は直前のチェックポイント以降に更新されており、
		// 最新版を更新した更新トランザクションがすべて終了するまでに
		// 開始された版管理するトランザクションは実行中である

		// 現在の最新版は、最新版を更新した更新トランザクションが
		// 終了してから開始された版管理するトランザクションから
		// 参照されるように、最終更新時タイムスタンプ値を変更する

		if (start.isIllegal())
			start = Trans::TimeStamp::assign();
		else
			--start;
		src.unfix(start);

		// 現在の最新版は新たに確保された版の直前の版となるように
		// 新たに確保された版に情報を記録する

		dstLog0._older = src.getID();
		dstLog0._physicalLog = Block::IllegalID;
		dstLog0._olderTimeStamp = start;
		dst0.dirty();

		// 新たに確保された版を新しい最新版とする

		allocated = dst0;

	} else {

		// 最新版を更新した更新トランザクションは終了しており、
		// 最新版は直前のチェックポイント以降に更新されておらず、
		// 最新版を更新した更新トランザクションがすべて終了するまでに
		// 開始された版管理するトランザクションは実行中である

		if (v >= VersionNumber::Second)

			// dst0 も最新版ではない

			setNewest(trans, v, verification, dst0.getID(), false);

		// 直前の版の再利用はしていないはず

		; _SYDNEY_ASSERT(!reuse);

		// 現在の最新版は物理ログ専用にし、
		// 最新版を更新した更新トランザクションがすべて終了するまでに
		// 開始された版管理するトランザクションから参照されないように
		// 新たに確保された版に情報を記録する

		if (srcLog._category != Log::Category::Oldest)
			dstLog0._physicalLog = src.getID();
		else {
			; _SYDNEY_ASSERT(dstLog0._older == Block::IllegalID);
			; _SYDNEY_ASSERT(dstLog0._physicalLog == Block::IllegalID);
			dstLog0._olderTimeStamp = Trans::IllegalTimeStamp;
		}

		if (start.isIllegal())
			start = Trans::TimeStamp::assign();
		else
			--start;
		dst0.unfix(start);

		// 新しいバージョンログを記録するブロックをさらに確保し、
		// 現在の最新版を複写する

		Block::Memory	dst1(
			Log::allocate(trans, verification, *this, headerMulti, priority));
		; _SYDNEY_ASSERT(dst1.isDirty());
		(void) dst1.copy(src);

		Log& dstLog1 = Log::get(dst1);
		dstLog1._category = Log::Category::Copy;
		dstLog1._pageID = page.getID();

		// 先ほど確保した版は新たに確保された版の直前の版となるように
		// 新たに確保された版に情報を記録する

		dstLog1._older = dst0.getID();
		dstLog1._physicalLog = Block::IllegalID;
		dstLog1._olderTimeStamp = start;

		// 新たに確保された版を新しい最新版とする

		allocated = dst1;
	}

	// 最新版が新しくなったので、
	// ページ更新トランザクションリストを空にし、
	// 最新版を確保した更新トランザクションについて、新たに登録する

	page._modifierList.clear();
	
	if (page._file->isBatchInsert() == false)

		// バッチインサートの場合は登録しない
		
		page._modifierList.pushFront(trans.getID());

	// コピーした版のタイムスタンプを更新する
	
	allocated.touch();

	return allocated;
}

//	FUNCTION public
//	Version::VersionLog::File::allocateLogForBackup --
//		あるバージョンページの新しい最新版のバージョンログを
//		必要があれば、確保し、バックアップを可能にする
//
//	NOTES
//		あるバージョンページに関するバックアップを開始する
//		版管理するトランザクションにとっての版を、
//		バックアップされたバージョンログファイルから選択可能なように、
//		必要があれば、新たな最新版を表すバージョンログを確保する
//
//		バージョンログを確保後、チェックポイント処理が行われ、
//		かつ、そのバージョンログを更新すると、
//		バージョンログファイルの整合性が失われる可能性がある
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バックアップを開始するアイソレーションレベルが SERIALIZABLE の
//			版管理するトランザクションのトランザクション記述子
//		Version::VersionLog::MultiplexBlock& headerMulti
//			ファイルヘッダを格納する多重化されたブロックに関する情報
//		Version::Page&		page
//			このバージョンページ記述子のバージョンページの
//			新たな最新版を表すバージョンログを必要があれば、確保する
//		Version::Block::Memory&	src
//			バックアップされるバージョンログファイルの
//			あるバージョンページの現在の最新版のバッファリング内容
//
//	RETURN
//		最新版を記録するバージョンログのバッファリング内容
//		(ただし、アンフィックスされている可能性がある)
//
//	EXCEPTIONS

Block::Memory
VersionLog::
File::allocateLogForBackup(const Trans::Transaction& trans,
						   MultiplexBlock& headerMulti,
						   Page& page, const Block::Memory& src)
{
	; _SYDNEY_ASSERT(!trans.isNoVersion() &&
					 trans.getIsolationLevel() ==
						Trans::Transaction::IsolationLevel::Serializable);

	// 指定されたバージョンページの
	// ページ更新トランザクションリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(page.getLatch());

	// データベースIDを得る

	Schema::ObjectID::Value dbid
		= page.getFile().getLockName().getDatabasePart();


	if (page.getModifierList().isEmpty() ||
		trans.getBirthTimeStamp() < src.getLastModification())

		// 指定されたバージョンページの最新版を更新した
		// トランザクションが存在しなければ、
		// すべでの版管理するトランザクションが参照すべき版は最新版である
		//
		// また、指定された版管理するトランザクションが
		// 開始されてから最新版が更新されていれば、
		// 指定された版管理するトランザクションは
		// ページ更新トランザクションリストを使わなくても
		// 参照すべき版を決定できる

		return src;

	Trans::TimeStamp start;

	if (Trans::Transaction::isInProgress(
			dbid,
			page.getModifierList(), Trans::Transaction::Category::ReadWrite)) {

		// 最新版を更新中の更新トランザクションが存在する

		if (src.getLastModification() >
			Checkpoint::TimeStamp::getMostRecent(_versionFile.getLockName())) {

			// 最新版は、前回のチェックポイント処理からすでに更新されている

			// 最新版をここで更新したことにすれば、
			// すべての版管理するトランザクションは
			// ページ更新トランザクションリストなしで
			// 参照すべき版を決定できる

			Block::Memory dst(src);
			dst.dirty();

			// 最新版を更新したことにした結果、
			// ページ更新トランザクションリストを使わなくても
			// 参照すべき版を決定できるようになったので、空にする

			page._modifierList.clear();

			return dst;
		}
	} else if (!_Transaction::isOverlapped(
				   dbid,
				   src.getLastModification(), page.getModifierList(), start)) {

		// 最新版を更新したトランザクションはすべて終了しており、
		// 最新版が最後に更新されてから、
		// それらのトランザクションがすべて終了する間に
		// 開始された版管理するトランザクションのうち、
		// 現在実行中のものは存在しないので、
		// すべての版管理するトランザクションは
		// ページ更新トランザクションリストなしで
		// 参照すべき版を決定できる

		// そこで、ページ更新トランザクションリストを空にする

		page._modifierList.clear();

		return src;
	}

	// 新しいバージョンログを記録するブロックを確保し、
	// 現在の最新版を複写する

	Block::Memory dst(
		Log::allocate(trans, 0, *this, headerMulti,
					  Buffer::ReplacementPriority::Low));
	; _SYDNEY_ASSERT(dst.isDirty());
	(void) dst.copy(src);

	Log& dstLog = Log::get(dst);
	dstLog._category = Log::Category::Copy;
	dstLog._pageID = page.getID();

	// 現在の最新版は物理ログ専用にし、最新版が最後に更新されてから、
	// 最新版を更新した更新トランザクションがすべて終了する間に
	// 開始された実行中の版管理するトランザクションから参照されないように
	// 新たに確保された版に情報を記憶する

	if (Log::get(src)._category != Log::Category::Oldest)
	{
		dstLog._physicalLog = src.getID();

		// バージョンファイルのバージョンを得る
		VersionNumber::Value v =
			FileHeader::get(headerMulti._memories[headerMulti._master])
			.getVersion();

		if (v >= VersionNumber::Second)

			// 最新版が更新されたので、その情報を
			// アロケーションテーブルに反映する

			setNewest(trans, v, 0, src.getID(), false);

	}
	else {
		; _SYDNEY_ASSERT(dstLog._older == Block::IllegalID);
		; _SYDNEY_ASSERT(dstLog._physicalLog == Block::IllegalID);
		dstLog._olderTimeStamp = Trans::IllegalTimeStamp;
	}

	if (start.isIllegal())
		start = Trans::TimeStamp::assign();
	else
		--start;
	dst.unfix(start);

	// 新しい最新版を確保した結果、
	// ページ更新トランザクションリストを使わなくても
	// 参照すべき版を決定できるようになったので、空にする

	page._modifierList.clear();

	return dst;
}

//	FUNCTION public
//	Version::VersionLog::File::freePBCT --
//		あるバージョンページの最新版のブロック識別子を得るためにたどる
//		PBCT ノード、リーフを可能な限り使用済にする
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				PBCT ノード、リーフを使用済にするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				PBCT ノード、リーフを使用済にする
//				トランザクションは整合性検査中でない
//		Version::Block::Memory&	headerMemory
//			使用済にする PBCT ノード、リーフが存在する
//			バージョンログファイルのファイルヘッダのバッファリング内容
//		Version::Page::ID	pageID
//			このバージョンページ識別子のバージョンページの
//			最新版のブロック識別子を得るためにたどる
//			PBCT ノード、リーフを可能な限り使用済にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::freePBCT(const Trans::Transaction& trans,
			   Verification* verification,
			   Block::Memory& headerMemory, Page::ID pageID)
{
	FileHeader&	header = FileHeader::get(headerMemory);

	// PBCT が存在するとき、ルートノード、リーフから処理していく

	if (!header.isPBCTEmpty() &&
		freePBCT(trans, header.getVersion(), verification, pageID,
				 headerMemory, 0, header.getPBCTLevel())) {

		// ルートがノードのときは子供のブロック、
		// リーフのときは最新版のブロックのブロック識別子が
		// ひとつも記録されていないので、PBCT が存在しないことにする

		header._PBCTLevel = PBCTLevel::Illegal;
		headerMemory.dirty();
	}
}

//	FUNCTION private
//	Version::VersionLog::File::freePBCT --
//		あるバージョンページの最新版のブロック識別子を得るためにたどる
//		PBCT ノード、リーフのうち、あるレベル以下のものを
//		可能な限り使用済にする
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				PBCT ノード、リーフを使用済にするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				PBCT ノード、リーフを使用済にする
//				トランザクションは整合性検査中でない
//		Version::Page::ID	pageID
//			このバージョンページ識別子のバージョンページの
//			最新版のブロック識別子を得るためにたどる
//			PBCT ノード、リーフを可能な限り使用済にする
//		Block::Memory&		nodeMemory
//			処理する PBCT ノード、リーフのバッファリング内容
//		Version::VersionLog::PBCTLevel::Value	current
//			処理する PBCT ノード、リーフの深さ(0 以上)
//		Version::VersionLog::PBCTLevel::Value	level
//			処理する PBCT の深さ(0 以上)
//
//	RETURN
//		true
//			指定されたレベルの PBCT ノード、リーフは使用済である
//		false
//			指定されたレベルの PBCT ノード、リーフは使用済でない
//
//	EXCEPTIONS

bool
VersionLog::
File::freePBCT(const Trans::Transaction& trans,
			   VersionNumber::Value v,
			   Verification* verification, Page::ID pageID,
			   Block::Memory& nodeMemory, PBCTLevel::Value current,
			   PBCTLevel::Value level)
{
	; _SYDNEY_ASSERT(current != PBCTLevel::Illegal);
	; _SYDNEY_ASSERT(level != PBCTLevel::Illegal);
	; _SYDNEY_ASSERT(current <= level);

	if (current == level) {

		// 与えられたバッファリング内容は PBCT リーフのものである

		if (!PBCTLeaf::get(nodeMemory).getCount()) {

			// この PBCT リーフには最新版のブロックのブロック識別子が
			// ひとつも記録されていない

			if (current)

				// この PBCT リーフはルートリーフでないので、使用済にする

				PBCTLeaf::free(trans,
							   v, verification, *this, nodeMemory.getID());

			return true;
		}
	} else {

		// 与えられたバッファリング内容は PBCT ノードのものである

		PBCTNode& node = PBCTNode::get(nodeMemory);

		// 次に調べるべき子供の PBCT ノード、リーフのブロック識別子を得る

		const Block::ID id = node.getChildID(
			pageID, current, level, getBlockSize());
		if (id != Block::IllegalID) {

			// 得られたブロック識別子のブロックをフィックスする

			Block::Memory	childMemory(
				(current + 1 < level) ?
				PBCTNode::fix(trans, verification, *this, id,
							  Buffer::Page::FixMode::Write) :
				PBCTLeaf::fix(trans, verification, *this, id,
							  Buffer::Page::FixMode::Write));

			// フィックスした子供の PBCT ノード、リーフを処理する

			if (freePBCT(trans, v, verification, pageID,
						 childMemory, current + 1, level)) {

				// 子供の PBCT ノード、リーフが使用済になったので、
				// そのブロック識別子を記憶しないようにする

				node.setChildID(pageID, current,
								Block::IllegalID, level, getBlockSize());
				nodeMemory.dirty();
			}
		}

		if (!node.getCount()) {

			// この PBCT ノードには子供のブロックのブロック識別子が
			// ひとつも記録されていない

			if (current)

				// この PBCT ノードはルートノードでないので、使用済にする

				PBCTNode::free(trans,
							   v, verification, *this, nodeMemory.getID());

			return true;
		}
	}

	return false;
}

//	FUNCTION public
//	Version::VersionLog::File::freeLog --
//		あるバージョンログとそれより前のものをすべて使用済にする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンログを使用済にする
//			トランザクションのトランザクション記述子
//		Version::Verification*	verification
//			0 以外の値
//				バージョンログを使用済にするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				バージョンログを使用済にする
//				トランザクションは整合性検査中でない
//		Version::Block::ID	id
//			使用済にする先頭のバージョンログのブロック識別子
//		Trans::TimeStamp::Value	oldest
//			最古のバージョンログの最終更新時タイムスタンプ値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::freeLog(const Trans::Transaction& trans,
			  VersionNumber::Value v, Verification* verification,
			  Block::ID id, Trans::TimeStamp::Value oldest)
{
	Trans::TimeStamp::Value older = Trans::IllegalTimeStamp;
	
	while (id != Block::IllegalID) {

		// このブロック識別子のバージョンログをフィックスする

		const Block::Memory& logMemory =
			Log::fix(trans, 0, *this, id,
					 Buffer::Page::FixMode::ReadOnly,
					 Buffer::ReplacementPriority::Low);

		; _SYDNEY_ASSERT(logMemory.getLastModification() >= oldest);

		if (older != Trans::IllegalTimeStamp
			&& logMemory.getLastModification() != older)

			// ひとつ新しい版のリンク情報のタイムスタンプと
			// この版のタイムスタンプは同じはずであるが、
			// リンクが切れていることがあり、タイムスタンプがことなっている
			// 通常ありえないが、olderがおかしいことがあるので、
			// ここでチェックする

			break;

		const Log& log = Log::get(logMemory);

		// このブロック識別子のバージョンログを使用済にする

		Log::free(trans, v, verification, *this, id);

		if (v >= VersionNumber::Second)

			// バージョン２以降は、最新の版だけを使用済にすればいい
			// それ以外の版は、自動的に使用済になる

			break;

		// ひとつ古い版に対する物理ログがあれば、すべて使用済にする

		freePhysicalLog(trans, v, verification, log._physicalLog);

		if (log._olderTimeStamp < oldest)

			// 最古のバージョンログまで処理した

			break;

		// ひとつ古い版のバージョンログのブロック識別子とタイムスタンプを得る

		id = log._older;
		older = log._olderTimeStamp;
	}
}

// FUNCTION private
//	Version::VersionLog::File::freePhysicalLog --
//		あるバージョンログに対するすべての物理ログを使用済にする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			物理ログを使用済にするトランザクションのトランザクション記述子
//		Version::Verification*	verification
//			0 以外の値
//				物理ログを使用済にするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				物理ログを使用済にするトランザクションは整合性検査中でない
//		Version::Block::ID	id
//			あるバージョンログに対する最新の物理ログのブロック識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::freePhysicalLog(const Trans::Transaction& trans,
					  VersionNumber::Value v,
					  Verification* verification, Block::ID id)
{
	while (id != Block::IllegalID) {

		// このブロック識別子の物理ログをフィックスする

		const Block::Memory& logMemory =
			Log::fix(trans, 0, *this, id,
					 Buffer::Page::FixMode::ReadOnly,
					 Buffer::ReplacementPriority::Low);

		// このブロック識別子の物理ログを使用済にする

		Log::free(trans, v, verification, *this, id);

		// ひとつ古い物理ログのブロック識別子を得る

		id = Log::get(logMemory)._physicalLog;
	}
}

//	FUNCTION public
//	Version::VersionLog::File::getBoundSize --
//		使用中のブロックの総サイズを求める
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				使用中のブロックの総サイズを求めるトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				使用中のブロックの総サイズを求める
//				トランザクションは整合性検査中でない
//		Version::VersionLog::FileHeader&	header
//			使用中のブロックの総サイズを求める
//			バージョンログファイルのファイルヘッダを表すクラス
//
//	RETURN
//		求めたサイズ(B 単位)
//
//	EXCEPTIONS

Os::File::Size
VersionLog::
File::getBoundSize(const Trans::Transaction& trans,
				   Verification* verification, const FileHeader& header)
{
	const unsigned int bitCount =
		_AllocationTable::getBitCount(header.getVersion(), getBlockSize());

	unsigned int n = MultiplexCount;

	// ファイルの先頭から、アロケーションテーブルを
	// ひとつひとつ調べて、使用中のブロック数を数えていく

	for (Block::ID tableID = MultiplexCount;
		 tableID < header._blockCount; tableID += bitCount) {

		// アロケーションテーブルをフィックスする

		const Block::Memory& tableMemory =
			AllocationTable::fix(
				trans,
				verification, *this, tableID, Buffer::Page::FixMode::ReadOnly);

		n += MultiplexCount + AllocationTable::get(tableMemory).getCount();
	}

	return static_cast<Os::File::Size>(getBlockSize()) * n;
}

//	FUNCTION private
//	Version::VersionLog::File::fixBlock -- ブロックをフィックスする
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				ブロックをフィックスするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				ブロックをフィックスするトランザクションは整合性検査中でない
//		Version::Block::ID	id
//			フィックスするブロックのブロック識別子
//		Buffer::Page::FixMode::Value	mode
//			Buffer::Page::FixMode::ReadOnly
//				フィックスするブロックは参照しかされない
//			Buffer::Page::FixMode::Write
//				フィックスするブロックは更新される
//			Buffer::Page::FixMode::Allocate
//				フィックスするブロックはその領域の初期化のために使用する
//		Buffer::ReplacementPriority::Value	priority
//			Buffer::ReplacementPriority::Low
//				フィックスするブロックは、バッファから破棄されやすい
//			Buffer::ReplacementPriority::Middle
//				フィックスするブロックは、バッファにある程度残る
//			Buffer::ReplacementPriority::High
//				フィックスするブロックは、バッファからかなりの間残る
//
//	RETURN
//		フィックスしたブロックのバッファリング内容
//
//	EXCEPTIONS

Block::Memory
VersionLog::
File::fixBlock(const Trans::Transaction& trans,
			   Verification* verification,
			   Block::ID id, Buffer::Page::FixMode::Value mode,
			   Buffer::ReplacementPriority::Value priority)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	if (verification)

		// 整合性検査中にフィックスしたブロックを管理する
		// ビットマップのフィックスしようとしている
		// ブロックに対応するビットを立てる
		//
		//【注意】	次のフィックスでエラーが起きても、
		//			立てたビットは元に戻さない

		verification->getBlockBitmap().setBit(id, true);

	; _SYDNEY_ASSERT(_bufFile);
	return Block::Memory(
		id, Buffer::Page::fix(
			*_bufFile,
			static_cast<Os::File::Offset>(getBlockSize()) * id,
			mode, priority, &trans));
}

//	FUNCTION private
//	Version::VersionLog::File::fixMaster --
//		バージョンログファイルの多重化されたブロックのうち、
//		マスタブロックをフィックスする
//
//	NOTES
//		マスタブロックをフィックス後、チェックポイント処理が行われ、
//		かつ、そのマスタブロックを更新すると、バージョンログファイルの
//		整合性が失われる可能性がある
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				マスタブロックをフィックスするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				マスタブロックをフィックスする
//				トランザクションは整合性検査中でない
//		Version::Block::ID	id
//			多重化されたブロックの先頭のもののブロック識別子
//		Buffer::Page::FixMode::Value	mode
//			Buffer::Page::FixMode::ReadOnly または指定されないとき
//				フィックスするマスタブロックは参照しかされない
//			Buffer::Page::FixMode::Write
//				フィックスするマスタブロックは更新される
//			Buffer::Page::FixMode::Allocate
//				フィックスするマスタブロックは
//				その領域の初期化のために使用する
//		Buffer::ReplacementPriority::Value	priority
//			Buffer::ReplacementPriority::Low
//				フィックスするマスタブロックは、バッファから破棄されやすい
//			Buffer::ReplacementPriority::Middle または指定されないとき
//				フィックスするマスタブロックは、バッファにある程度残る
//			Buffer::ReplacementPriority::High
//				フィックスするマスタブロックは、バッファからかなりの間残る
//
//	RETURN
//		フィックスしたマスタブロックのバッファリング内容
//
//	EXCEPTIONS

Block::Memory
VersionLog::
File::fixMaster(const Trans::Transaction& trans,
				Verification* verification,
				Block::ID id, Buffer::Page::FixMode::Value mode,
				Buffer::ReplacementPriority::Value priority,
				void (*initializeFunc)(Block::Memory&))
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	if (verification)

		// 整合性検査中にフィックスしたブロックを管理する
		// ビットマップのフィックスするマスタブロックが含まれる
		// 多重化されたブロックに対応するビットを立てる
		//
		//【注意】	次のフィックスでエラーが起きても、
		//			立てたビットは元に戻さない

		verification->getBlockBitmap().setBit(id, MultiplexCount, true);

	// 多重化されたブロックのうち、
	// どれをマスタブロックにすべきか決めるための情報を表すクラスを得て、
	// 保護するためにラッチする

	MultiplexInfo& info = *MultiplexInfo::attach(*this, id);
	Os::AutoCriticalSection	latch(info.getLatch());

	// 直前のチェックポイント処理の終了時のタイムスタンプ値を求める

	const Trans::TimeStamp::Value first =
		Checkpoint::TimeStamp::getMostRecent(_versionFile.getLockName());

	if (mode == Buffer::Page::FixMode::Allocate) {

		// 初期化のためにフィックスするときは、
		// 多重化されたブロックのうちの先頭のものを
		// マスタブロックとして、フィックスする
		//
		//【注意】	初期化のためにフィックスするとき、
		//			既存の領域を再利用せずに未使用の領域を使用するので、
		//			フラッシュの抑制を可能にする必要はない

		const Block::Memory& memory =
			fixBlock(trans, 0, id, Buffer::Page::FixMode::Allocate, priority);

		// 先頭のものの最終更新時タイムスタンプとして、
		// 直前のチェックポイント + 多重化数 を記憶しておく
		//
		//【注意】	このブロックの実際の最終更新時タイムスタンプとは異なるが、
		//			多重化されたブロックから選択するには、この値で十分である

		unsigned int i = 0;
		info._lastModification[i] = first + MultiplexCount;

		while (++i < MultiplexCount) {

			// 多重化されたブロックのうち、
			// 先頭のもの以外も初期化のためにフィックスする

			(void) fixBlock(trans, 0, id + i,
							Buffer::Page::FixMode::Allocate, priority);

			// 先頭のもの以外の最終更新時タイムスタンプとして、
			// 直前のチェックポイント + 先頭からの位置 を記憶しておく

			info._lastModification[i] = first + i;
		}

		return memory;

	}

	// 多重化されたブロックのうち、
	// 一番最近に更新されているものを探す

	unsigned int	i = 0, latest;

	if (mode == Buffer::Page::FixMode::ReadOnly) {

		Block::Memory	memory;

		do {
			Trans::TimeStamp::Value& t = info._lastModification[i];

			if (Trans::TimeStamp::isIllegal(t)) {

				// このブロックは一度もフィックスしていないので、
				// フィックスして、最終更新時タイムスタンプを得る

				const Block::Memory& tmp
					= fixBlock(trans, 0, id + i, mode, priority);
				t = tmp.getLastModification();

				if (t > first)

					// 多重化されたブロックのうち、
					// 直前のチェックポイント処理以降に
					// 更新されるブロックはひとつしかないはずなので、
					// このブロックが一番最近に更新されたものである

					return tmp;

				if (!i || t > info._lastModification[latest]) {

					// 今調べているブロックはこれまで調べた中で
					// 一番最近に更新されているものなので、記憶しておく
					//
					//【注意】	Block::Memory::operator = の
					//			呼び出し回数を減らすため、
					//			フィックス直後でなく、ここで代入する

					latest = i;
					memory = tmp;
				}
			} else {

				// このブロックはこれまでにフィックスされ、
				// 最終更新時タイムスタンプも記憶されている

				if (t > first)
					return fixBlock(trans, 0, id + i, mode, priority);
				if (!i || t > info._lastModification[latest]) {
					latest = i;
					memory = Block::Memory();
				}
			}
		} while (++i < MultiplexCount) ;

		// 多重化されたブロックに
		// 直前のチェックポイント処理以降に更新されたものはなかった

		// 参照のためにフィックスするときは、
		// 一番最近に更新されたものをマスタブロックとする

		return (memory.getID() != Block::IllegalID) ?
			memory : fixBlock(trans, 0, id + latest, mode, priority);
	} else {

		// 更新のためにマスタブロックをフィックスするときは、
		// フラッシュの抑制を可能にする

		mode = Buffer::Page::FixMode::Write |
			Buffer::Page::FixMode::Deterrentable;

		Block::Memory	memories[MultiplexCount];

		do {
			Trans::TimeStamp::Value& t = info._lastModification[i];

			if (Trans::TimeStamp::isIllegal(t)) {
				const Block::Memory& tmp
					= fixBlock(trans, 0, id + i, mode, priority);
				t = tmp.getLastModification();

				if (t > first)
					return tmp;

				if (!i || t > info._lastModification[latest])

					// 一番最近に更新されているものの候補である

					memories[latest = i] = tmp;

				else if (i == latest + 1)

					// 一番最近に更新されているものの候補の直後は
					// 一番昔に更新されているものの候補である

					memories[i] = tmp;
			} else {
				if (t > first)
					return fixBlock(trans, 0, id + i, mode, priority);
				if (!i || t > info._lastModification[latest])
					latest = i;
			}
		} while (++i < MultiplexCount) ;

		// 更新のためにフィックスするときは、
		// 一番昔に更新されたものをマスタブロックとする
		//
		// ただし、それには一番最近に更新されたものの内容を複写し、
		// 更新されたことにしておく

		const unsigned int oldest = (latest + 1) % MultiplexCount;

		if (memories[oldest].getID() == Block::IllegalID)
			memories[oldest] = fixBlock(trans, 0, id + oldest, mode, priority);
		if (memories[latest].getID() == Block::IllegalID)
			memories[latest] = fixBlock(trans, 0, id + latest, mode, priority);

		memories[oldest].copy(memories[latest]).dirty();
		info._lastModification[oldest] = first + 1;
		
		// 初期化関数が設定されている場合は、それを呼び出す

		if (initializeFunc)
			(*initializeFunc)(memories[oldest]);

		return memories[oldest];
	}
}

//	FUNCTION private
//	Version::VersionLog::File::fixMasterAndSlaves --
//		バージョンログファイルの多重化されたブロックのうち、
//		マスタブロックおよびリカバリによって将来回復される
//		可能性のあるスレーブブロックをフィックスする
//
//	NOTES
//		マスタブロックおよびリカバリによって将来回復される
//		可能性のあるスレーブブロックをフィックス後、
//		チェックポイント処理が行われ、そのマスタブロックを更新すると、
//		バージョンログファイルの整合性が失われる可能性がある
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				ブロック達をフィックスするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				ブロック達をフィックスする
//				トランザクションは整合性検査中でない
//		Version::Block::ID	id
//			多重化されたブロックの先頭のもののブロック識別子
//		Buffer::Page::FixMode::Value	mode
//			Buffer::Page::FixMode::ReadOnly または指定されないとき
//				フィックスするブロック達は参照しかされない
//			Buffer::Page::FixMode::Write
//				フィックスするブロック達は更新される
//			Buffer::Page::FixMode::Allocate
//				フィックスするブロック達はその領域の初期化のために使用する
//		Buffer::ReplacementPriority::Value	priority
//			Buffer::ReplacementPriority::Low
//				フィックスするブロック達は、バッファから破棄されやすい
//			Buffer::ReplacementPriority::Middle または指定されないとき
//				フィックスするブロック達は、バッファにある程度残る
//			Buffer::ReplacementPriority::High
//				フィックスするブロック達は、バッファからかなりの間残る
//		Version::VersionLog::MultiplexBlock& multi
//			マスタブロックおよびリカバリによって将来回復される
//			可能性のあるスレーブブロックに関する情報が設定される
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::fixMasterAndSlaves(const Trans::Transaction& trans,
						 Verification* verification,
						 Block::ID id, Buffer::Page::FixMode::Value mode,
						 Buffer::ReplacementPriority::Value priority,
						 MultiplexBlock& multi,
						 void (*initializeFunc)(Block::Memory&))
{
#ifdef DEBUG
	; _SYDNEY_ASSERT(id != Block::IllegalID);
	{
	unsigned int i = 0;
	do {
		; _SYDNEY_ASSERT(multi._memories[i].getID() == Block::IllegalID);
	} while (++i < MultiplexCount) ;
	}
#endif
	if (verification)

		// 整合性検査中にフィックスしたブロックを管理する
		// ビットマップのフィックスするマスタブロックが含まれる
		// 多重化されたブロックに対応するビットを立てる
		//
		//【注意】	次のフィックスでエラーが起きても、
		//			立てたビットは元に戻さない

		verification->getBlockBitmap().setBit(id, MultiplexCount, true);

	// 多重化されたブロックのうち、
	// どれをマスタブロックにすべきか決めるための情報を表すクラスを得て、
	// 保護するためにラッチする

	MultiplexInfo& info = *MultiplexInfo::attach(*this, id);
	Os::AutoCriticalSection	latch(info.getLatch());

	// 前回のチェックポイント処理の終了時のタイムスタンプ値を求める

	const Trans::TimeStamp::Value first =
		Checkpoint::TimeStamp::getMostRecent(_versionFile.getLockName());

	if (mode == Buffer::Page::FixMode::Allocate) {

		// 初期化のためにフィックスするときは、
		// 多重化されたブロックの先頭のものを
		// マスタブロックとしてフィックスする
		//
		//【注意】	初期化のためにフィックスするとき、
		//			既存の領域を再利用せずに未使用の領域を使用するので、
		//			フラッシュの抑制を可能にする必要はない

		unsigned int i = 0;
		multi._memories[i] = fixBlock(
			trans, 0, id, Buffer::Page::FixMode::Allocate, priority);

		// 先頭のものの最終更新時タイムスタンプとして
		// 前回のチェックポイントの直後のタイムスタンプを記憶しておく
		//
		//【注意】	このブロックの実際の最終更新時タイムスタンプとは異なるが、
		//			多重化されたブロックから選択するには、この値で十分である

		info._lastModification[i] = first + MultiplexCount;

		while (++i < MultiplexCount) {

			// 多重化されたブロックのうち、
			// 先頭のもの以外も初期化のためにフィックスする

			(void) fixBlock(trans, 0, id + i,
							Buffer::Page::FixMode::Allocate, priority);

			// 先頭のもの以外の最終更新時タイムスタンプとして、
			// 直前のチェックポイント + 先頭からの位置 を記憶しておく

			info._lastModification[i] = first + i;
		}

		multi._master = 0;
		return;

	} else if (mode == Buffer::Page::FixMode::Write)

		// 更新のためにマスタブロックをフィックスするときは、
		// フラッシュの抑制を可能にする

		mode = Buffer::Page::FixMode::Write |
			Buffer::Page::FixMode::Deterrentable;

	// 前々回のチェックポイント処理の終了時のタイムスタンプを求める

	const Trans::TimeStamp::Value second =
		Checkpoint::TimeStamp::getSecondMostRecent(_versionFile.getLockName());

	// 多重化されたブロックのうち、一番最近に更新されているものを求める

	unsigned int i = 0, latest;
	do {
		Trans::TimeStamp::Value& t = info._lastModification[i];
		if (Trans::TimeStamp::isIllegal(t))

			// このブロックは一度もフィックスしていないので、
			// フィックスして、最終更新時タイムスタンプを得る

			t = (multi._memories[i] = fixBlock(
					 trans, 0, id + i, mode, priority)).getLastModification();

		if (t > first) {

			// 多重化されたブロックのうち、
			// 前回のチェックポイント処理以降に
			// 更新されるブロックはひとつしかないはず
			//
			//【注意】	初期化のためにフィックスしたときは
			//			前回のチェックポイント処理以降に
			//			すべてのブロックが更新されているが、
			//			最初のブロックがマスタブロックになっているので、
			//			以下のコードでうまくいく

			// リカバリによって復活するブロックは
			// ひとつまたはふたつ前のブロックのどちらかである

			const unsigned int older =
				(i + MultiplexCount - 1) % MultiplexCount;
			const unsigned int oldest = (i + 1) % MultiplexCount;

			if (Trans::TimeStamp::isIllegal(info._lastModification[older]))

				// ひとつ前のブロックは一度もフィックスしていないので、
				// フィックスして、最終更新時タイムスタンプを得る
				//
				//【注意】	初期化のためにフィックスしたとき、
				//			実際のものでなく、
				//			前回のチェックポイントのタイムスタンプから
				//			計算したものを格納しているので、
				//			ここだけ無条件に上書きするとうまく動作しなくなる

				info._lastModification[older] =
					(multi._memories[older] = fixBlock(
						trans, 0, id + older,
						mode, priority)).getLastModification();

			if (Trans::TimeStamp::isIllegal(info._lastModification[oldest]))

				// ふたつ前のブロックは一度もフィックスしていないので、
				// フィックスして、最終更新時タイムスタンプを得る

				info._lastModification[oldest] =
					(multi._memories[oldest] = fixBlock(
						trans, 0, id + oldest,
						mode, priority)).getLastModification();

			if (multi._memories[i].getID() == Block::IllegalID)

				// このブロックはマスタブロックなので、
				// フィックスされていなければ、フィックスする

				multi._memories[i] = fixBlock(trans, 0, id + i, mode, priority);

			//【注意】	以下のようにリカバリによって回復されないか、
			//			将来リカバリによって回復されもしないブロックのうち、
			//			フィックスされているものは、アンフィックスする
			//
			//				  second		    first
			//	────────┼────────┼──────────
			//	×oldest ○older│				  │                 ○i
			//	○oldest		│         ○older│                 ○i
			//					│×oldest ○older│                 ○i
			//					│				  │×oldest ×older ○i

			if (info._lastModification[older] > first)

				// ひとつ前のブロックはリカバリに必要ないので
				// フィックスされている必要はない

				multi._memories[older].unfix();

			else if (multi._memories[older].getID() == Block::IllegalID)

				// フィックスされていなければ、フィックスする

				multi._memories[older] =
					fixBlock(trans, 0, id + older, mode, priority);

			if (!(info._lastModification[older] > second &&
				  info._lastModification[oldest] < second))

				// ふたつ前のブロックはリカバリに必要ないので
				// フィックスされている必要はない

				multi._memories[oldest].unfix();

			else if (multi._memories[oldest].getID() == Block::IllegalID)
				multi._memories[oldest] =
					fixBlock(trans, 0, id + oldest, mode, priority);

			// このブロックをマスタブロックとする

			multi._master = i;
			return;
		}

		if (!i || t > info._lastModification[latest])

			// 一番最近に更新されているものの候補である

			latest = i;

	} while (++i < MultiplexCount) ;

	// 一番最近に更新されたブロックは、
	// 前回のチェックポイント処理以前に更新されている

	// リカバリによって復活するブロックは
	// 一番最近に更新されたブロックまたはそのひとつ前のもののどちらかである

	const unsigned int older = (latest + MultiplexCount - 1) % MultiplexCount;
	const unsigned int oldest = (latest + 1) % MultiplexCount;

	; _SYDNEY_ASSERT(
		!Trans::TimeStamp::isIllegal(info._lastModification[latest]));
	; _SYDNEY_ASSERT(
		!Trans::TimeStamp::isIllegal(info._lastModification[older]));
	; _SYDNEY_ASSERT(
		!Trans::TimeStamp::isIllegal(info._lastModification[oldest]));

	if (multi._memories[latest].getID() == Block::IllegalID)

		// 一番最近に更新されたブロックはマスタブロックか、
		// リカバリによって回復されるか、
		// 将来リカバリによって回復されるかのいずれかなので、
		// フィックスされていなければ、フィックスする

		multi._memories[latest]
			= fixBlock(trans, 0, id + latest, mode, priority);

	//【注意】	以下のようにリカバリによって回復されないか、
	//			将来リカバリによって回復されもしないブロックのうち、
	//			フィックスされているものは、アンフィックスする
	//
	//							second						first
	//	─────────────┼─────────────┼─
	//  ×oldest ×older ○latest │                          │
	//	×oldest ○older          │○latest                  │
	//	                          │×oldest ×older ○latest │

	if (!(info._lastModification[latest] > second &&
		  info._lastModification[older] < second))

		// ひとつ前のブロックはリカバリに必要ないので
		// フィックスされている必要はない

		multi._memories[older].unfix();

	else if (multi._memories[older].getID() == Block::IllegalID)
		multi._memories[older] = fixBlock(trans, 0, id + older, mode, priority);

	if (mode == Buffer::Page::FixMode::ReadOnly) {

		// 参照のみのためにフィックスするとき

		// 一番昔に更新されたブロックはリカバリに必要ないので
		// フィックスされている必要はない

		multi._memories[oldest].unfix();

		// 一番最近に更新されたブロックをマスタブロックとする

		multi._master = latest;
	} else {

		// 更新可でフィックスするとき、
		// 一番昔に更新されたものをマスタブロックとする
		//
		// ただし、それには一番最近に更新されたものの内容を複写し、
		// 更新されたことにしておく

		if (multi._memories[oldest].getID() == Block::IllegalID)
			multi._memories[oldest]
				= fixBlock(trans, 0, id + oldest, mode, priority);

		multi._memories[oldest].copy(multi._memories[latest]).dirty();
		info._lastModification[oldest] = first + 1;
		
		// 初期化関数が設定されている場合は、それを呼び出す

		if (initializeFunc)
			(*initializeFunc)(multi._memories[oldest]);

		multi._master = oldest;
	}
}

//	FUNCTION private
//	Version::VersionLog::File::clearMultiplexInfoTable --
//		バージョンログファイルの多重化されたブロックの
//		どれを選択するかを決めるための情報をすべて破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::ID	id
//			指定されたとき
//				指定されたブロック識別子以上の多重化されたブロックの
//				どれを選択するかを決めるための情報を破棄する
//			指定されないとき
//				0 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::clearMultiplexInfoTable(Block::ID id)
{
	; _SYDNEY_ASSERT(_infoTable);

	// このバージョンログファイルの多重化されたブロックを
	// 選択するための情報を表すクラスをひとつひとつ破棄していく

	unsigned int i = 0;
	do {
		// 今調べているバケットを得る

		HashTable<MultiplexInfo>::Bucket& bucket = _infoTable->getBucket(i);

		// バケットを保護するためにラッチをかける

		Os::AutoCriticalSection latch(bucket.getLatch());

		// このバケットに登録されているクラスをひとつひとつ処理していく

		HashTable<MultiplexInfo>::Bucket::Iterator	ite(bucket.begin());
		const HashTable<MultiplexInfo>::Bucket::Iterator&	end = bucket.end();

		while (ite != end) {

			// 反復子の指す要素を削除すると
			// 次の要素が得られなくなるので、ここで反復子を次に進めておく

			MultiplexInfo* info = &*ite;
			++ite;

			if (info->getID() >= id) {

				// このクラスをバケットから除き、破棄する

				bucket.erase(*info);
				delete info;
			}
		}
	} while (++i < _infoTable->getLength()) ;
}

//	FUNCTION public
//	Version::VersionLog::File::applyFree --
//		使用済み(開放された)ブロックをアロケーションテーブルに反映する
//
//	NOTES
//		2つ前のチェックポイント時に最新の版ではないブロックを
//		すべて使用済みにする
//
//		使用済みにするブロックを参照しているトランザクションがいる場合には、
//		なにもしない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
VersionLog::
File::applyFree(const Trans::Transaction& trans)
{
	// ファイルヘッダをフィックスして、存在するブロック数を求める
	//
	//【注意】	すべてのファイルヘッダをフィックスるするのは、リカバリによって
	//			将来回復される部分もあるので、最新のヘッダーだけでは正確な
	//			ブロック数を求めることはできないからである

	MultiplexBlock headerMulti;
	FileHeader::fix(trans, 0, *this, Buffer::Page::FixMode::Write, headerMulti);

	unsigned int i = 0;
	unsigned int maxBlockCount = 0;
	VersionNumber::Value v = VersionNumber::First;
	
	do {
		FileHeader* header = 0;
		if (header = (headerMulti._memories[i].isOwner() ?
					  &FileHeader::get(headerMulti._memories[i]) : 0)) {
			if (maxBlockCount < header->_blockCount)
				maxBlockCount = header->_blockCount;
			v = header->getVersion();
		}
	} while (++i < MultiplexCount) ;

	// バージョン２未満はこの処理は行わない

	if (v < VersionNumber::Second)
		return;

	// 2つ前のチェックポイントのタイムスタンプを得る
	
	Trans::TimeStamp::Value second =
		Checkpoint::TimeStamp::getSecondMostRecent(
			_versionFile.getLockName());

	// 2つ前のチェックポイント処理終了時点から、
	// 現在実行中のトランザクションが途切れることなく
	// 実行されているかチェックし、途切れていない場合は反映しない

	if (Trans::Transaction::getBeginningID(
			_versionFile.getLockName().getDatabasePart()) < second)
		return;

	const unsigned int bitCount =
		_AllocationTable::getBitCount(v, getBlockSize());

	Block::ID tableID = MultiplexCount;
	for (; tableID < maxBlockCount; tableID += bitCount + MultiplexCount) {

		// アロケーションテーブルをフィックスして、
		// 使用済みのブロックが反映されていないかチェックする

		MultiplexBlock tableMulti;
		AllocationTable::fix(trans, 0, *this, tableID,
							 Buffer::Page::FixMode::Write, tableMulti);

		// 使用済みのブロックが反映されていなかったら、反映する
			
		AllocationTable& t = AllocationTable::get(
			tableMulti._memories[tableMulti._master]);
			
		if (t.isApplyFree() == false) {
			
			// まだ反映されていないので、反映する
			
			AllocationTable::applyFree(tableMulti, second,
									   v, getBlockSize());
		}
	}
}

//	FUNCTION public
//	Version::VersionLog::FileHeader::allocate --
//		バージョンログファイルにファイルヘッダを確保する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				ファイルヘッダを確保するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				ファイルヘッダを確保するトランザクションは整合性検査中でない
//		Version::VersionLog::File&	file
//			ファイルヘッダを確保するバージョンログファイルの
//			バージョンログファイル記述子
//
//	RETURN
//		確保したファイルヘッダを記録するブロックのバッファリング内容
//
//	EXCEPTIONS

// static
Block::Memory
FileHeader::allocate(const Trans::Transaction& trans,
					 Verification* verification, VersionLog::File& file)
{
	; _SYDNEY_ASSERT(!file.getSize());

	// ファイルの先頭から MultiplexCount 個連続する
	// 複数のファイルヘッダを確保できるように、
	// ファイルを拡張する

	file.extend(MultiplexCount);

	// ファイルヘッダをフィックスして、初期化する

	Block::Memory	headerMemory(
		FileHeader::fix(trans,
						verification, file, Buffer::Page::FixMode::Allocate));
	FileHeader&	header = FileHeader::get(headerMemory);

	header._versionNumber = VersionNumber::Current;
	header._blockCount = MultiplexCount;
	header._pageCount = 0;
	header._PBCTLevel = PBCTLevel::Illegal;
	header._creationTimeStamp = Trans::TimeStamp::assign();

	// ファイルヘッダを記録するブロック上に
	// PBCT のルートとなる PBCT リーフを初期化する

	PBCTLeaf::get(headerMemory).initialize(false, file.getBlockSize());
	headerMemory.dirty();

	return headerMemory;
}

//	FUNCTION public
//	Version::VersionLog::AllocationTable::allocate --
//		バージョンログファイルにアロケーションテーブルを確保する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				アロケーションテーブルを確保するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				アロケーションテーブルを確保する
//				トランザクションは整合性検査中でない
//		Version::VersionLog::File&	file
//			アロケーションテーブルを確保する
//			バージョンログファイルのバージョンログファイル記述子
//		Version::VersionLog::MultiplexBlock& headerMulti
//			ファイルヘッダを格納する多重化されたブロックに関する情報
//
//	RETURN
//		確保したアロケーションテーブルを記録するブロックのバッファリング内容
//
//	EXCEPTIONS

// static
Block::Memory
AllocationTable::allocate(const Trans::Transaction& trans,
						  Verification* verification, VersionLog::File& file,
						  MultiplexBlock& headerMulti)
{
	FileHeader&	header =
		FileHeader::get(headerMulti._memories[headerMulti._master]);

	// 現在確保されているブロックの直後から
	// MultiplexCount 個連続する
	// 複数のアロケーションテーブルを確保できるように、
	// ファイルを拡張する

	file.extend(header._blockCount + MultiplexCount);

	// アロケーションテーブルをフィックスして、初期化する

	Block::Memory	tableMemory(
		AllocationTable::fix(trans, verification, file, header._blockCount,
							 Buffer::Page::FixMode::Allocate));
	AllocationTable& table = AllocationTable::get(tableMemory);

	//【注意】
	//	ページ全体を初期化するので、最初のバージョンのサイズを渡す
	
	table._count = 0;
	(void) Os::Memory::reset(
		table._bitmap, _AllocationTable::getBitmapSize(VersionNumber::First,
													   file.getBlockSize()));
	
	if (header.getVersion() >= VersionNumber::Second)
		
		// 初めて確保されたアロケーションテーブルなので、
		// 2つ前のチェックポイント以前の
		// アロケーションテーブルなど存在していない
		// 使用済みブロックが反映されていることにする
		
		table.setApplyFree();
	
	tableMemory.dirty();

	// ファイルヘッダのブロック数を、
	// 確保したアロケーションテーブル数ぶん、増やす

	header._blockCount += MultiplexCount;
	headerMulti._memories[headerMulti._master].dirty();

	return tableMemory;
}

//	FUNCTION public
//	Version::VersionLog::AllocationTable::setNewestBit --
//		ブロックが最新のものかをを表す複数の連続したビットを操作する
//
//	NOTES
//		アロケーションテーブルで管理しないブロック識別子が与えられたとき、
//		動作は保証しない
//
//		指定したブロック識別子のブロックが使用中かを表すビットから
//		アロケーションテーブル中のビットマップの末尾まで操作しても、
//		操作したビット数が指定したビット数に足りない場合、
//		次のアロケーションテーブルを操作することはない
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				ビットを操作するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				ビットを操作するトランザクションは整合性検査中でない
//		Os::Memory::Size	size
//			アロケーションテーブルが記録されている
//			ブロックのブロックサイズ(B 単位)
//		Version::Block::ID	id
//			操作する複数の連続したビットのうち、
//			先頭のものが使用中かを表すブロックのブロック識別子
//		unsigned int		n
//			いくつの連続したビットを操作するか
//		bool				on
//			true
//				ビットを立てる
//			false
//				ビットを落とす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
AllocationTable::setNewestBit(
	VersionNumber::Value v, Verification* verification,
	Os::Memory::Size size, Block::ID id, unsigned int n, bool on)
{
	if (v < VersionNumber::Second)
		return;
	
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	unsigned int i = _AllocationTable::blockIDToBitmapIndex(v, size, id);
	unsigned int j = _AllocationTable::blockIDToBitIndexOfBitmap(v, size, id);
	unsigned int bitmapLength = _AllocationTable::getBitmapLength(v, size);

	do {
		
		// 最新かどうかのビットは、使用中かどうかのビットの後にある
		
		unsigned int* bp = _bitmap + i + bitmapLength;

		do {
			const unsigned int m = 1 << j;
			if (*bp & m) {
				if (!on) {
					*bp &= ~m;
				}
			} else
				if (on) {
					*bp |= m;
				}

			if (!--n)
				return;

		} while (++j < _AllocationTable::_bitCountPerBitmap) ;

		j = 0;

	} while (++i < bitmapLength) ;

//	まだ未実装というか、必要かどうかすら不明
//
//	if (verification)
//
//		// 整合性検査中にフィックスしたブロックを管理する
//		// ビットマップのビットも操作する
//
//		verification->getBlockBitmap().setBit(id, n, on);
}

//	FUNCTION public
//	Version::VersionLog::AllocationTable::applyFree
//		使用済み(フリー済み)のブロックをアロケーションテーブルに反映する
//
//	NOTES
//		アロケーションテーブルの後半には、最新のブロックかどうかが管理されている
//
//		2つ前のチェックポイント以前のブロックは、それを参照するトランザクション
//		が存在しない場合、最新のブロック以外は永遠に利用されない
//
//		そこで、2つ前のチェックポイント以前の最新ではないブロックは、
//		使用中のブロックではないので、ビットを落とす
//
//		ただし、マスターブロックは多重化されているブロックすべてが最新のブロック
//		となるよう、AllocationTable::allocate で設定されている
//
//	ARGUMENTS
//		Version::VersionLog::MultiplexBlock& tableMulti
//			アロケーションテーブルを格納する多重化されたブロックに間する情報
//		const Trans::TimeStamp::Value& second
//			2つ前のチェックポイントのタイムスタンプ
//		Version::VersionNumber::Value v
//			バージョンのバージョン
//		Os::Memory::Size size
//			ページサイズ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

/* static */
void
AllocationTable::applyFree(MultiplexBlock& tableMulti,
						   const Trans::TimeStamp::Value& second,
						   VersionNumber::Value v, Os::Memory::Size size)
{
	if (v < VersionNumber::Second)
		return;
	
	// 2つ前のチェックポイント以前に更新されたブロックを得る
	// 該当するブロックは1つだけしか fix されていないはず

	unsigned int	latest = MultiplexCount;
	unsigned int	i = 0;
	
	do {
		if (tableMulti._master != i &&
			tableMulti._memories[i].isOwner() &&
			tableMulti._memories[i].getLastModification() < second)
		{
			; _TRMEISTER_ASSERT(latest == MultiplexCount);

			// 2つ前のチェックポイントより前に更新されているブロックである
			
			latest = i;
		}
		
	} while (++i < MultiplexCount) ;

	// 現時点のマスターブロック
	
	AllocationTable& table
		= AllocationTable::get(tableMulti._memories[tableMulti._master]);
	
	if (latest != MultiplexCount)
	{
		// 2つ前のチェックポイントより前に更新されているブロックが存在したので、
		// その時点の最新のブロック以外は再利用可能である

		unsigned int bitmapLength = _AllocationTable::getBitmapLength(v, size);

		// 2つ前のチェックポイント時点のアロケーションテーブル
		const unsigned int* bp1	=
			AllocationTable::get(tableMulti._memories[latest])._bitmap;
		const unsigned int* bp2 = bp1 + bitmapLength;

		// 最新のアロケーションテーブル
		unsigned int* bpm = table._bitmap;
		// ビット数をクリアする
		table._count = 0;
		
		unsigned int i = 0;
		do
		{
			// bp1は使用中ブロックのビットマップ、
			// bp2は使用中の最新ブロックのビットマップである。
			// bp1 と bp2 のXORは、使用中のブロックの内、最新ではないものになる
			// その補数とANDをとればいい。
			
			*bpm &= ~((*bp1++) ^ (*bp2++));

			// ビットが立っている数を求める
			
			unsigned char* p = syd_reinterpret_cast<unsigned char*>(bpm);
			unsigned char* ep = p + sizeof(unsigned int);
			for (; p < ep; ++p)
				table._count += _AllocationTable::_bitCountTable[*p];

			// 次へ
			++bpm;
			
		} while (++i < bitmapLength) ;
	}

	// 反映済みにする
	
	table.setApplyFree();
	tableMulti._memories[tableMulti._master].dirty();
}

//	FUNCTION public
//	Version::VersionLog::AllocationTable::initializeBlock --
//		チェックポイント後に初めて確保されたアロケーションテーブルを初期化する
//
//	NOTES
//
//	ARGUMENTS
//	Version::Block::Memory& memory
//		チェックポイント後に初めて確保されらアロケーションテーブル
//
//	RETURN
//	なし
//
//	EXCEPTIONS

/* static */
void
AllocationTable::initializeBlock(Block::Memory& memory)
{
	AllocationTable::get(memory).unsetApplyFree();
	memory.dirty();
}

//	FUNCTION public
//	Version::VersionLog::AllocationTable::setBoundBit --
//		ブロックが使用中かを表す複数の連続したビットを操作する
//
//	NOTES
//		アロケーションテーブルで管理しないブロック識別子が与えられたとき、
//		動作は保証しない
//
//		指定したブロック識別子のブロックが使用中かを表すビットから
//		アロケーションテーブル中のビットマップの末尾まで操作しても、
//		操作したビット数が指定したビット数に足りない場合、
//		次のアロケーションテーブルを操作することはない
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				ビットを操作するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				ビットを操作するトランザクションは整合性検査中でない
//		Os::Memory::Size	size
//			アロケーションテーブルが記録されている
//			ブロックのブロックサイズ(B 単位)
//		Version::Block::ID	id
//			操作する複数の連続したビットのうち、
//			先頭のものが使用中かを表すブロックのブロック識別子
//		unsigned int		n
//			いくつの連続したビットを操作するか
//		bool				on
//			true
//				ビットを立てる
//			false
//				ビットを落とす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
AllocationTable::setBoundBit(
	VersionNumber::Value v, Verification* verification,
	Os::Memory::Size size, Block::ID id, unsigned int n, bool on)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	unsigned int i = _AllocationTable::blockIDToBitmapIndex(v, size, id);
	unsigned int j = _AllocationTable::blockIDToBitIndexOfBitmap(v, size, id);
	unsigned int bitmapLength = _AllocationTable::getBitmapLength(v, size);

	do {
		unsigned int* bp = _bitmap + i;

		do {
			const unsigned int m = 1 << j;
			if (*bp & m) {
				if (!on) {
					*bp &= ~m;
					--_count;
				}
			} else
				if (on) {
					*bp |= m;
					++_count;
				}

			if (!--n)
				return;

		} while (++j < _AllocationTable::_bitCountPerBitmap) ;

		j = 0;

	} while (++i < bitmapLength) ;

	if (verification)

		// 整合性検査中にフィックスしたブロックを管理する
		// ビットマップのビットも操作する

		verification->getBlockBitmap().setBit(id, n, on);
}

//	FUNCTION public
//	Version::VersionLog::AllocationTable::getBoundBit --
//		ブロックが使用中かを表すビットが立っているか
//
//	NOTES
//		アロケーションテーブルで管理しないブロック識別子が与えられたとき、
//		動作は保証しない
//
//	ARGUMENTS
//		Os::Memory::Size	size
//			アロケーションテーブルが記録されている
//			ブロックのブロックサイズ(B 単位)
//		Version::Block::ID	id
//			調べるビットが使用中かを表すブロックのブロック識別子
//
//	RETURN
//		true
//			立っている
//		false
//			落ちている
//
//	EXCEPTIONS

bool
AllocationTable::getBoundBit(VersionNumber::Value v,
							 Os::Memory::Size size, Block::ID id) const
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	const unsigned int* bp =
		_bitmap + _AllocationTable::blockIDToBitmapIndex(v, size, id);
	const unsigned int m =
		1 << _AllocationTable::blockIDToBitIndexOfBitmap(v, size, id);

	return *bp & m;
}

//	FUNCTION public
//	Version::VersionLog::PBCTNode::allocate --
//		バージョンログファイルに PBCT ノードを確保する
//
//	NOTES
//		Version::Verification*	verification
//			0 以外の値
//				PBCT ノードを確保するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				PBCT ノードを確保するトランザクションは整合性検査中でない
//		Version::VersionLog::File&	file
//			PBCT ノードを確保する
//			バージョンログファイルのバージョンログファイル記述子
//		Version::VersionLog::MultiplexBlock& headerMulti
//			ファイルヘッダを格納する多重化されたブロックに関する情報
//
//	RETURN
//		確保した PBCT ノードを記録するブロックのバッファリング内容
//
//	EXCEPTIONS

// static
Block::Memory
PBCTNode::allocate(const Trans::Transaction& trans,
				   Verification* verification, VersionLog::File& file,
				   MultiplexBlock& headerMulti)
{
	// PBCT ノードを記録するための
	// MultiplexCount 個連続する複数のブロックを確保する

	Block::ID id = file.allocate(trans,
								 verification, headerMulti, MultiplexCount);
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	// PBCT ノードをフィックスして、初期化する
	//
	//【注意】	確保する PBCT ノードは必ずルートでないので、
	//			getCountMax には PBCT ノードの深さとして 0 以外の値を渡す

	Block::Memory	nodeMemory(
		PBCTNode::fix(trans, verification,
					  file, id, Buffer::Page::FixMode::Allocate));

	PBCTNode::get(nodeMemory).initialize(true, file.getBlockSize());
	nodeMemory.dirty();

	return nodeMemory;
}

//	FUNCTION public
//	Version::VersionLog::PBCTNode::free --
//		バージョンログファイルの PBCT ノードを使用済にする
//
//	NOTES
//		Version::Verification*	verification
//			0 以外の値
//				PBCT ノードを使用済にするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				PBCT ノードを使用済にするトランザクションは整合性検査中でない
//		Version::VersionLog::File&	file
//			PBCT ノードを使用済にする
//			バージョンログファイルのバージョンログファイル記述子
//		Version::Block::ID	id
//			使用済にする PBCT ノードが存在するブロックのブロック識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
PBCTNode::free(const Trans::Transaction& trans,
			   VersionNumber::Value v,
			   Verification* verification, VersionLog::File& file, Block::ID id)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	// 使用済にする PBCT ノードが存在する多重化されたブロックの
	// 先頭のもののブロック識別子を求める

	id = normalizeID(id);

	// 実際に使用済にする

	file.free(trans, v, verification, id, MultiplexCount);

	// 使用済にした PBCT ノードが存在する
	// 多重化されたブロックのどれを選択すべきかを決めるための
	// 情報を表すクラスが存在すれば、破棄する

	MultiplexInfo::detach(file, id);
}

//	FUNCTION public
//	Version::VersionLog::PBCTNode::setChildID --
//		あるバージョンページの版をたどるための
//		子ノードのブロック識別子を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Page::ID	pageID
//			このバージョンページ識別子の表すバージョンページの
//			最新版を得るための子ノードのブロック識別子を設定しようとしている
//		Version::VersionLog::PBCTLevel::Value	current
//			ブロック識別子を設定する PBCT ノードの深さ(0 以上)
//		Version::Block::ID	id
//			設定するブロック識別子
//		Version::VersionLog::PBCTLevel::Value	level
//			ブロック識別子を設定する PBCT ノードを含む PBCT の深さ(1 以上)
//		Os::Memory::Size	size
//			ブロック識別子を設定する PBCT ノードが
//			存在するブロックのサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
PBCTNode::setChildID(Page::ID pageID, PBCTLevel::Value current, Block::ID id,
					 PBCTLevel::Value level, Os::Memory::Size size)
{
	setChildID(_PBCTNode::pageIDToArrayIndex(
				   pageID, current, level, size), id);
}

//	FUNCTION private
//	Version::VersionLog::PBCTNode::setChildID --
//		なん番目に記録されている子ノードのブロック識別子を設定する
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		i
//			設定しようとしているブロック識別子がなん番目に記録されているものか
//		Version::Block::ID	id
//			設定するブロック識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
PBCTNode::setChildID(unsigned int i, Block::ID id)
{
	Block::ID& child = _child[i];

	if (id == Block::IllegalID) {
		if (child != Block::IllegalID)
			--_count;
	} else
		if (child == Block::IllegalID)
			++_count;

	child = id;
}

//	FUNCTION public
//	Version::VersionLog::PBCTNode::getChildID -- 子ノードのブロック識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::Page::ID	pageID
//			このバージョンページ識別子の表すバージョンページの
//			最新版を得るための子ノードのブロック識別子を得ようとしている
//		Version::VersionLog::PBCTLevel::Value	current
//			得ようとしているブロック識別子が設定されている
//			PBCT ノードの深さ(0 以上)
//		Version::VersionLog::PBCTLevel::Value	level
//			得ようとしているブロック識別子が設定されている
//			PBCT ノードを含む PBCT の深さ(1 以上)
//		Os::Memory::Size	size
//			得ようとしているブロック識別子が設定されている
//			PBCT ノードが存在するブロックのサイズ(B 単位)
//
//	RETURN
//		得られたブロック識別子
//
//	EXCEPTIONS
//		なし

Block::ID
PBCTNode::getChildID(Page::ID pageID, PBCTLevel::Value current,
					 PBCTLevel::Value level, Os::Memory::Size size) const
{
	return getChildID(
		_PBCTNode::pageIDToArrayIndex(pageID, current, level, size));
}

//	FUNCTION private
//	Version::VersionLog::PBCTNode::initialize --
//		あるブロックサイズのブロック上の PBCT ノードを初期化する
//
//	NOTES
//
//	ARGUMENTS
//		bool				isNotRoot
//			true
//				初期化する PBCT ノードはルートでない
//			false
//				初期化する PBCT ノードはルートである
//		Os::Memory::Size	size
//			初期化する PBCT ノードが存在するブロックのサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
PBCTNode::initialize(bool isNotRoot, Os::Memory::Size size)
{
	_count = 0;

	//【注意】	Block::IllegalID が ~static_cast<Block::ID>(0) で
	//			あることを前提にしたコードである

	(void) Os::Memory::set(
		_child, ~static_cast<unsigned char>(0),
		sizeof(Block::ID) * PBCTNode::getCountMax(isNotRoot, size));
}

//	FUNCTION private
//	Version::VersionLog::PBCTNode::copy --
//		あるブロックサイズのブロック上の PBCT ノードで上書きする
//
//	NOTES
//
//	ARGUMENTS
//		PBCTNode&			src
//			自分自身を上書きする PBCT ノード
//		bool				isNotRoot
//			true
//				自分自身を上書きする PBCT ノードはルートでない
//			false
//				自分自身を上書きする PBCT ノードはルートである
//		Os::Memory::Size	size
//			自分自身を上書きする PBCT ノードが存在するブロックのサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
PBCTNode::copy(const PBCTNode& src, bool isNotRoot, Os::Memory::Size size)
{
	_count = src._count;
	(void) Os::Memory::copy(
		_child, src._child,
		sizeof(Block::ID) * PBCTNode::getCountMax(isNotRoot, size));
}

//	FUNCTION private
//	Version::VersionLog::PBCTNode::verify -- PBCT ノードの整合性を検査する
//
//	NOTES
//
//	ARGUMENTS
//		Version::VersionLog::File&	file
//			整合性を検査する PBCT ノードが存在する
//			バージョンログファイルのバージョンログファイル記述子
//		bool				isNotRoot
//			true
//				整合性を検査する PBCT ノードはルートでない
//			false
//				整合性を検査する PBCT ノードはルートである
//		Admin::Verification::Progress&	result
//			整合性検査の経過を設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
PBCTNode::verify(const VersionLog::File& file, bool isNotRoot,
				 Admin::Verification::Progress& result) const
{
	// ノードに記録されている下位のノード、リーフの
	// ブロック識別子の数が正しいか調べる

	unsigned int i = 0;
	const unsigned int n =
		PBCTNode::getCountMax(isNotRoot, file.getBlockSize());
	unsigned int count = 0;

	do {
		if (getChildID(i) != Block::IllegalID)
			++count;
	} while (++i < n) ;

	if (getCount() != count)

		// ノードに記録されている下位のノードの数が不正である

		_SYDNEY_VERIFY_INCONSISTENT(
			result, file.getParent(), Message::ChildCountInconsistent());
}

//	FUNCTION public
//	Version::VersionLog::PBCTLeaf::allocate --
//		バージョンログファイルに PBCT リーフを確保する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				PBCT リーフを確保するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				PBCT リーフを確保するトランザクションは整合性検査中でない
//		Version::VersionLog::File&	file
//			PBCT リーフを確保する
//			バージョンログファイルのバージョンログファイル記述子
//		Version::VersionLog::MultiplexBlock& headerMulti
//			ファイルヘッダを格納する多重化されたブロックに関する情報
//
//	RETURN
//		確保した PBCT リーフを記録するブロックのバッファリング内容
//
//	EXCEPTIONS

// static
Block::Memory
PBCTLeaf::allocate(const Trans::Transaction& trans,
				   Verification* verification, VersionLog::File& file,
				   MultiplexBlock& headerMulti)
{
	// PBCT リーフを記録するための
	// MultiplexCount 個連続する複数のブロックを確保する

	Block::ID id = file.allocate(trans,
								 verification, headerMulti, MultiplexCount);
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	// PBCT リーフをフィックスして、初期化する
	//
	//【注意】	確保する PBCT リーフは必ずルートでないので、
	//			getCountMax には PBCT リーフの深さとして 0 以外の値を渡す

	Block::Memory	leafMemory(
		PBCTLeaf::fix(trans, verification,
					  file, id, Buffer::Page::FixMode::Allocate));

	PBCTLeaf::get(leafMemory).initialize(true, file.getBlockSize());
	leafMemory.dirty();

	return leafMemory;
}

//	FUNCTION public
//	Version::VersionLog::PBCTLeaf::free --
//		バージョンログファイルの PBCT リーフを使用済にする
//
//	NOTES
//		Version::Verification*	verification
//			0 以外の値
//				PBCT リーフを使用済にするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				PBCT リーフを使用済にするトランザクションは整合性検査中でない
//		Version::VersionLog::File&	file
//			PBCT リーフを使用済にする
//			バージョンログファイルのバージョンログファイル記述子
//		Version::Block::ID	id
//			使用済にする PBCT リーフが存在するブロックのブロック識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
PBCTLeaf::free(const Trans::Transaction& trans,
			   VersionNumber::Value v,
			   Verification* verification, VersionLog::File& file, Block::ID id)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	// 使用済にする PBCT リーフが存在する多重化されたブロックの
	// 先頭のもののブロック識別子を求める

	id = normalizeID(id);

	// 実際に使用済にする

	file.free(trans, v, verification, id, MultiplexCount);

	// 使用済にした PBCT リーフが存在する
	// 多重化されたブロックのどれを選択すべきかを決めるための
	// 情報を表すクラスが存在すれば、破棄する

	MultiplexInfo::detach(file, id);
}

//	FUNCTION public
//	Version::VersionLog::PBCTLeaf::setLatestID --
//		あるバージョンページの最新版のバージョンログのブロック識別子を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Page::ID	pageID
//			このバージョンページ識別子の表すバージョンページの
//			最新版のバージョンログのブロック識別子を設定しようとしている
//		Version::Block::ID	id
//			設定するブロック識別子
//		Os::Memory::Size	size
//			ブロック識別子を設定する PBCT リーフが存在する
//			ブロックのサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
PBCTLeaf::setLatestID(Page::ID pageID, Block::ID id, Os::Memory::Size size)
{
	setLatestID(_PBCTLeaf::pageIDToArrayIndex(pageID, size), id);
}

//	FUNCTION private
//	Version::VersionLog::PBCTLeaf::setLatestID --
//		なん番目に記録されている
//		最新版のバージョンログのブロック識別子を設定する
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		i
//			設定しようとしているブロック識別子がなん番目に記録されているものか
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
PBCTLeaf::setLatestID(unsigned int i, Block::ID id)
{
	Block::ID& latest = *syd_reinterpret_cast<Block::ID*>(
		syd_reinterpret_cast<char*>(_latest) +
		(sizeof(Block::ID) + sizeof(Trans::TimeStamp::Value)) * i);
	
	if (id == Block::IllegalID) {
		if (latest != Block::IllegalID)
			--_count;
	} else
		if (latest == Block::IllegalID)
			++_count;

	latest = id;
}

//	FUNCTION public
//	Version::VersionLog::PBCTLeaf::getLatestID --
//		あるバージョンページの最新版のバージョンログのブロック識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::Page::ID	pageID
//			このバージョンページ識別子の表すバージョンページの
//			最新版のバージョンログのブロック識別子を得ようとしている
//		Os::Memory::Size	size
//			得ようとしているブロック識別子が設定されている
//			PBCT リーフが存在するブロックのサイズ(B 単位)
//
//
//	RETURN
//		得られたブロック識別子
//
//	EXCEPTIONS
//		なし

Block::ID
PBCTLeaf::getLatestID(Page::ID pageID, Os::Memory::Size size) const
{
	return getLatestID(_PBCTLeaf::pageIDToArrayIndex(pageID, size));
}

//	FUNCTION public
//	Version::VersivonLog::PBCTLeaf::setNewestTimeStamp --
//		あるバージョンページの最新版の最終更新時タイムスタンプ値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Page::ID	pageID
//			このバージョンページ識別子の表すバージョンページの
//			最古版の最終更新時タイムスタンプ値を設定しようとしている
//		Trans::TimeStamp::Value	t
//			設定するタイムスタンプ値
//		Os::Memory::Size	size
//			タイムスタンプ値を設定する PBCT リーフが
//			存在するブロックのサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
PBCTLeaf::setNewestTimeStamp(
	Page::ID pageID, Trans::TimeStamp::Value t, Os::Memory::Size size)
{
	setNewestTimeStamp(_PBCTLeaf::pageIDToArrayIndex(pageID, size), t);
}

//	FUNCTION private
//	Version::VersionLog::PBCTLeaf::setNewestTimeStamp --
//		なん番目に記録されている最新版の最終更新時タイムスタンプ値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		i
//			設定しようとしている最終更新時タイムスタンプ値が
//			なん番目に記録されているものか
//		Trans::TimeStamp::Value	t
//			設定するタイムスタンプ値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
PBCTLeaf::setNewestTimeStamp(unsigned int i, Trans::TimeStamp::Value t)
{
	Trans::TimeStamp::Value& oldest =
		*syd_reinterpret_cast<Trans::TimeStamp::Value*>(
			syd_reinterpret_cast<char*>(_latest) +
			(sizeof(Block::ID) + sizeof(Trans::TimeStamp::Value)) * i +
			sizeof(Block::ID));
	
	oldest = t;
}

//	FUNCTION public
//	Version::VersionLog::PBCTLeaf::getNewestTimeStamp --
//		あるバージョンページの最新版の最終更新時タイムスタンプ値を得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::Page::ID	pageID
//			このバージョンページ識別子の表すバージョンページの
//			最古版の最終更新時タイムスタンプ値を得ようとしている
//		Os::Memory::Size	size
//			得ようとしているタイムスタンプ値が設定されている
//			PBCT リーフが存在するブロックのサイズ(B 単位)
//
//
//	RETURN
//		得られたブ最終更新時タイムスタンプ値
//
//	EXCEPTIONS
//		なし

Trans::TimeStamp::Value
PBCTLeaf::getNewestTimeStamp(Page::ID pageID, Os::Memory::Size size) const
{
	return getNewestTimeStamp(_PBCTLeaf::pageIDToArrayIndex(pageID, size));
}

//	FUNCTION private
//	Version::VersionLog::PBCTLeaf::initialize --
//		あるブロックサイズのブロック上の PBCT リーフを初期化する
//
//	NOTES
//
//	ARGUMENTS
//		bool				isNotRoot
//			true
//				初期化する PBCT リーフはルートでない
//			false
//				初期化する PBCT リーフはルートである
//		Os::Memory::Size	size
//			初期化する PBCT リーフが存在するブロックのサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
PBCTLeaf::initialize(bool isNotRoot, Os::Memory::Size size)
{
	_count = 0;

	//【注意】	Block::IllegalID が ~static_cast<Block::ID>(0) で、
	//			static_cast<Trans::TimeStamp::Value>(Trans::IllegalTimeStamp)
	//			が ~static_cast<Trans::TimeStamp::Value>(0) で
	//			あることを前提にしたコードである

	(void) Os::Memory::set(
		_latest, ~static_cast<unsigned char>(0),
		(sizeof(Block::ID) + sizeof(Trans::TimeStamp::Value)) *
			PBCTLeaf::getCountMax(isNotRoot, size));
}

//	FUNCTION private
//	Version::VersionLog::PBCTNode::copy --
//		あるブロックサイズのブロック上の PBCT リーフで上書きする
//
//	NOTES
//
//	ARGUMENTS
//		PBCTNode&			src
//			自分自身を上書きする PBCT リーフ
//		bool				isNotRoot
//			true
//				自分自身を上書きする PBCT リーフはルートでない
//			false
//				自分自身を上書きする PBCT リーフはルートである
//		Os::Memory::Size	size
//			自分自身を上書きする PBCT リーフが存在するブロックのサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
PBCTLeaf::copy(const PBCTLeaf& src, bool isNotRoot, Os::Memory::Size size)
{
	_count = src._count;
	(void) Os::Memory::copy(
		_latest, src._latest,
		(sizeof(Block::ID) + sizeof(Trans::TimeStamp::Value)) *
			PBCTLeaf::getCountMax(isNotRoot, size));
}

//	FUNCTION private
//	Version::VersionLog::PBCTLeaf::verify -- PBCT リーフの整合性を検査する
//
//	NOTES
//
//	ARGUMENTS
//		Version::VersionLog::File&	file
//			整合性を検査する PBCT リーフが存在する
//			バージョンログファイルのバージョンログファイル記述子
//		bool				isNotRoot
//			true
//				整合性を検査する PBCT リーフはルートでない
//			false
//				整合性を検査する PBCT リーフはルートである
//		Admin::Verification::Progress&	result
//			整合性検査の経過を設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
PBCTLeaf::verify(const VersionLog::File& file, bool isNotRoot,
				 Admin::Verification::Progress& result) const
{
	// リーフに記録されている最新版のブロックの
	// ブロック識別子の数が不正でないことを確認する

	unsigned int i = 0;
	const unsigned int n =
		PBCTLeaf::getCountMax(isNotRoot, file.getBlockSize());
	unsigned int count = 0;

	do {
		if (getLatestID(i) != Block::IllegalID)
			++count;
	} while (++i < n) ;

	if (getCount() != count)
	
		// リーフに記録されている最新版のブロックの数が不正である

		_SYDNEY_VERIFY_INCONSISTENT(
			result, file.getParent(), Message::LatestCountInconsistent());
}

//	FUNCTION public
//	Version::VersionLog::Log::allocate --
//		バージョンログファイルにバージョンログを確保する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				バージョンログを確保するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				バージョンログを確保するトランザクションは整合性検査中でない
//		Version::VersionLog::File&	file
//			バージョンログを確保する
//			バージョンログファイルのバージョンログファイル記述子
//		Version::VersionLog::MultiplexBlock& headerMulti
//			ファイルヘッダを格納する多重化されたブロックに関する情報
//		Buffer::ReplacementPriority::Value	priority
//			Buffer::ReplacementPriority::Low
//				確保するバージョンログは、バッファから破棄されやすい
//			Buffer::ReplacementPriority::Middle
//				確保するバージョンログは、バッファにある程度残る
//			Buffer::ReplacementPriority::High
//				確保するバージョンログは、バッファからかなりの間残る
//
//	RETURN
//		確保したバージョンログを記録するブロックのバッファリング内容
//
//	EXCEPTIONS

// static
Block::Memory
Log::allocate(const Trans::Transaction& trans,
			  Verification* verification, VersionLog::File& file,
			  MultiplexBlock& headerMulti,
			  Buffer::ReplacementPriority::Value priority)
{
	// バージョンログを記録するためのブロックを確保する

	Block::ID id = file.allocate(trans, verification, headerMulti);
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	// バージョンログをフィックスして、初期化する

	Block::Memory	logMemory(
		file.fixBlock(
			trans,
			verification, id, Buffer::Page::FixMode::Allocate, priority));
	Log& log = Log::get(logMemory);

	log._older = Block::IllegalID;
	log._physicalLog = Block::IllegalID;
	log._olderTimeStamp = Trans::IllegalTimeStamp;
	log._category = Category::Unknown;
	logMemory.dirty();

	return logMemory;
}

//	FUNCTION public
//	Version::VersionLog::Log::fix -- バージョンログをフィックスする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&		trans
//			バージョンログをフィックスする
//			トランザクションのトランザクション記述子
//		Version::Verification*	verification
//			0 以外の値
//				バージョンログをフィックスするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				バージョンログをフィックスするトランザクションは
//				整合性検査中でない
//		Version::VersionLog::File&	file
//			フィックスするバージョンログが存在する
//			バージョンログファイルのバージョンログファイル記述子
//		Version::Block::ID	id
//			フィックスするバージョンログが存在するブロックのブロック識別子
//		Buffer::Page::FixMode::Value	mode
//			Buffer::Page::FixMode::ReadOnly
//				フィックスするバージョンログは参照しかされない
//			Buffer::Page::FixMode::Write
//				フィックスするバージョンログは更新される
//			Buffer::Page::FixMode::Allocate
//				フィックスするバージョンログはその領域の初期化のために使用する
//		Buffer::ReplacementPriority::Value	priority
//			Buffer::ReplacementPriority::Low
//				フィックスするバージョンログは、バッファから破棄されやすい
//			Buffer::ReplacementPriority::Middle
//				フィックスするバージョンログは、バッファにある程度残る
//			Buffer::ReplacementPriority::High
//				フィックスするバージョンログは、バッファからかなりの間残る
//
//	RETURN
//		フィックスしたバージョンログのバッファリング内容
//
//	EXCEPTIONS

// static
Block::Memory
Log::fix(const Trans::Transaction& trans, Verification* verification,
		 VersionLog::File& file, Block::ID id,
		 Buffer::Page::FixMode::Value mode,
		 Buffer::ReplacementPriority::Value priority)
{
	; _SYDNEY_ASSERT(!_Log::isIllegalID(id));

	if (!trans.isNoVersion() &&	mode & Buffer::Page::FixMode::ReadOnly)

		// 版管理するトランザクションが
		// 参照のみのためにフィックスしたバージョンログを
		// 更新するものはいないはずなので、
		// 読み取り書き込みロックする必要はない
		//
		//【注意】	最新版を格納するバージョンログは更新されるが、
		//			版の探索時にバージョンログのヘッダ部分しか
		//			参照しないので、大丈夫なはず

		mode |= Buffer::Page::FixMode::NoLock;

	return file.fixBlock(trans, verification, id, mode, priority);
}

//	FUNCTION public
//	Version::VersionLog::MultiplexInfo::attach --
//		多重化されたブロックのどれを選択すべきか
//		決めるための情報を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::VersionLog::File&	file
//			多重化されたブロックが存在する
//			バージョンログファイルのバージョンログファイル記述子
//		Version::Block::ID	id
//			このブロック識別子を表すブロックを先頭にして多重化されたものの
//			どれを選択すべきかを決めるための情報を表すクラスを得る
//
//	RETURN
//		得られたクラスを格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
MultiplexInfo*
MultiplexInfo::attach(VersionLog::File& file, Block::ID id)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	// 指定されたブロック識別子を先頭のブロックとする
	// 多重化されたブロックのどれを選択すべきか決めるための
	// 情報を表すクラスを格納すべきハッシュ表のバケットを求める

	; _SYDNEY_ASSERT(file._infoTable);
	const unsigned int addr =
		_File::infoTableHash(id) % file._infoTable->getLength();
	HashTable<MultiplexInfo>::Bucket& bucket =
		file._infoTable->getBucket(addr);

	// バケットを保護するためにラッチをかける

	Os::AutoCriticalSection	latch(bucket.getLatch());

	// 指定されたブロック識別子を先頭のブロックとする
	// 多重化されたブロックのどれを選択すべきか決めるための
	// 情報を表すクラスが求めたバケットに登録されていれば、それを得る

	MultiplexInfo* info = _MultiplexInfo::find(bucket, id);
	if (!info) {

		// 見つからなかったので、生成する

		info = new MultiplexInfo(id);
		; _SYDNEY_ASSERT(info);

		// ハッシュ表のバケットの先頭に挿入して、
		// 最近に挿入されたものほど、見つかりやすくする

		bucket.pushFront(*info);
	}

	return info;
}

//	FUNCTION public
//	Version::VersionLog::MultiplexInfo::detach --
//		多重化されたブロックのどれを選択すべきか
//		決めるための情報を表すクラスを破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Version::VersionLog::File&	file
//			多重化されたブロックが存在する
//			バージョンログファイルのバージョンログファイル記述子
//		Version::Block::ID	id
//			このブロック識別子を表すブロックを先頭にして多重化されたものの
//			どれを選択すべきかを決めるための情報を表すクラスを廃棄する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
MultiplexInfo::detach(VersionLog::File& file, Block::ID id)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	// 使用済にした PBCT リーフが存在する
	// 多重化されたブロックのどれを選択すべきかを決めるための
	// 情報を表すクラスを格納すべきハッシュ表のバケットを求める

	; _SYDNEY_ASSERT(file._infoTable);
	const unsigned int addr =
		_File::infoTableHash(id) % file._infoTable->getLength();
	HashTable<MultiplexInfo>::Bucket& bucket =
		file._infoTable->getBucket(addr);

	// バケットを保護するためにラッチをかける

	Os::AutoCriticalSection	latch(bucket.getLatch());

	// 指定されたブロック識別子を先頭のブロックとする
	// 多重化されたブロックのどれを選択すべきか決めるための
	// 情報を表すクラスが求めたバケットに登録されているか調べる

	if (MultiplexInfo* info = _MultiplexInfo::find(bucket, id)) {

		// 登録されていれば、破棄する

		bucket.erase(*info);
		delete info;
	}
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
