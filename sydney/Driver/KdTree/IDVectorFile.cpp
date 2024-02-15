// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IDVectorFile.cpp -- ベクターファイル
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
#include "KdTree/IDVectorFile.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "FileCommon/OpenOption.h"

#include "LogicalFile/OpenOption.h"

#include "Version/File.h"

#include "Os/File.h"
#include "Os/Memory.h"
#include "Os/Path.h"

#include "Schema/File.h"

#include "Exception/VerifyAborted.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
	// パス
	Os::Path _cSubPath("Expunge");

	// 格納領域がnullかどうか
	bool _isNull(const char* p_)
	{
		char c = static_cast<char>(-1);
		for (int i = 0; i < sizeof(ModUInt32); ++i, ++p_)
			if (*p_ != c)
				return false;
		return true;
	}

	// 格納領域をnullにする
	void _setNull(char* p_)
	{
		char c = static_cast<char>(-1);
		for (int i = 0; i < sizeof(ModUInt32); ++i, ++p_)
			*p_ = c;
	}

	// ダンプする
	void _dump(char* p_, ModUInt32 id_)
	{
		Os::Memory::copy(p_, &id_, sizeof(id_));
	}

	// アンダンプする
	void _undump(ModUInt32& id_, const char* p_)
	{
		Os::Memory::copy(&id_, p_, sizeof(id_));
	}
}

//
//	FUNCTION public
//	KdTree::IDVectorFile::IDVectorFile -- コンストラクタ
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
IDVectorFile::IDVectorFile(FileID& cFileID_, const Os::Path& cPath_)
	: VectorFile(cFileID_, Os::Path(cPath_).addPart(_cSubPath)),
	  m_uiCountPerPage(0)
{
	// エントリのサイズを求める
	m_uiElementSize = static_cast<ModSize>(sizeof(ModUInt32));
	
	// 1ページに格納できるエントリ数を求める
	m_uiCountPerPage
		= Version::Page::getContentSize(m_pVersionFile->getPageSize())
		/ m_uiElementSize;
}

//
//	FUNCTION public
//	KdTree::IDVectorFile::~IDVectorFile -- デストラクタ
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
IDVectorFile::~IDVectorFile()
{
}

//
//	FUNCTION public
//	KdTree::IDVectorFile::pushBack -- 末尾に追加する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiID_
//		バリュー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IDVectorFile::pushBack(ModUInt32 uiID_)
{
	// 最大のキー値を得る
	ModUInt32 key = getMaxKey();
	// その次に格納する
	++key;
	// 格納領域を得る
	char* p = getBuffer(key);
	// 書き出す
	_dump(p, uiID_);

	// ヘッダーを更新する
	m_cHeader.m_uiMaxKey = key;
	++m_cHeader.m_uiCount;
	dirtyHeaderPage();
}

//
//	FUNCTION public
//	KdTree::IDVectorFile::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiID_
//		削除対象のID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IDVectorFile::expunge(ModUInt32 uiID_)
{
	//【注意】	このベクターは、追加された順に格納されている
	//			ここでは、もっとも最近追加されたエントリを削除することを
	//			想定し、終わりから探索する
	//			過去に挿入されたものを削除すると、全数チェックになる場合がある
	//			また、もっとも最近登録されたものを削除すると、
	//			最大のキー値も小さくなるが、そんなに削除されないだろうから
	//			そのままとする

	// 最大のキー値を得る
	ModUInt32 key = getMaxKey();
	while (key)
	{
		// 0 のキー値は使われていない

		const char* p = getConstBuffer(key);
		if (_isNull(p) == false)
		{
			ModUInt32 id;
			_undump(id, p);

			if (id == uiID_)
			{
				// 削除対象のデータが見つかった
				// 更新モードでメモリ領域を取得しなおし、null にする
			
				char* q = getBuffer(key);
				_setNull(q);

				break;
			}
		}

		--key;
	}
}

//
//	FUNCTION public
//	KdTree::IDVectorFile::getAllValue -- 全バリューを取得する
//
//	NOTES
//
//	ARGUMENTS
//	Common::BitSet& cBitSet_
//		格納されているバリューを格納するビットセット
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IDVectorFile::getAllValue(Common::BitSet& cBitSet_)
{
	if (getCount() == 0)
		// 1件も挿入されていない
		return;
	
	ModUInt32 uiMaxKey = getMaxKey() + 1;
	const char* p = 0;
	for (ModUInt32 uiKey = 0; uiKey < uiMaxKey; ++uiKey)
	{
		if ((uiKey % m_uiCountPerPage) == 0)
		{
			// ページの先頭を得る
			p = getConstBuffer(uiKey);
		}

		if (_isNull(p) == false)
		{
			// 格納されている
			
			ModUInt32 id;
			_undump(id, p);
			cBitSet_.set(id);
		}

		p += m_uiElementSize;
	}
}

//
//	FUNCTION public
//	KdTree::IDVectorFile::move -- ファイルを移動する
//
//	NOTES
//	移動元と移動先のパスが異なっていることが前提。
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cPath_
//		移動先のパス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IDVectorFile::move(const Trans::Transaction& cTransaction_,
				 const Os::Path& cPath_)
{
	VectorFile::move(cTransaction_, Os::Path(cPath_).addPart(_cSubPath));
}

//
//	FUNCTION protected
//	KdTree::IDVectorFile::getBuffer -- 格納領域のメモリを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiKey_
//	   	キー値
//
//	RETURN
//	char*
//		格納領域へのポインタ
//
//	EXCEPTIONS
//
char*
IDVectorFile::getBuffer(ModUInt32 uiKey_)
{
	// キー -> ページID+オフセットへ変換
	int offset = 0;
	Version::Page::ID id = convertToPageID(uiKey_, offset);

	if (id > getMaxPageID())
	{
		// 最大ページIDより大きい -> 必要なところまでallocateする
		allocatePage(id);
	}

	// ページを得る
	fixPage(id);

	// メモリーを得る
	char* buf = m_pCurrentPage->getBuffer();
	// 更新するために取得しているので dirty にする
	m_pCurrentPage->dirty();

	return buf + offset;
}

//
//	FUNCTION protected
//	KdTree::IDVectorFile::getConstBuffer -- 格納領域のメモリを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiKey_
//		キー値
//
//	RETURN
//	const char*
//		格納領域へのポインタ
//
//	EXCEPTIONS
//
const char*
IDVectorFile::getConstBuffer(ModUInt32 uiKey_)
{
	// キー -> ページID+オフセットへ変換
	int offset = 0;
	Version::Page::ID id = convertToPageID(uiKey_, offset);

	// ページを得る
	fixPage(id);

	// メモリーを得る
	const char* buf = m_pCurrentPage->getConstBuffer();
    
	return buf + offset;
}

//
//	Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
