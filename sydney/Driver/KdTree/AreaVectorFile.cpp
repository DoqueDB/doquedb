// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaVectorFile.cpp -- ベクターファイル
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
#include "KdTree/AreaVectorFile.h"

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
	Os::Path _cSubPath("Vector");

	// 格納領域がnullかどうか
	bool _isNull(const char* p_)
	{
		char c = static_cast<char>(-1);
		for (int i = 0; i < 6; ++i, ++p_)
			if (*p_ != c)
				return false;
		return true;
	}

	// ダンプする
	void _dump(char* p_, const PhysicalFile::DirectArea::ID& id_)
	{
		// DirectArea::ID は 6バイト
		
		ModUInt32 pid = id_.m_uiPageID;
		unsigned short aid = static_cast<unsigned short>(id_.m_uiAreaID);
		
		Os::Memory::copy(p_, &pid, sizeof(pid));
		p_ += sizeof(pid);
		Os::Memory::copy(p_, &aid, sizeof(aid));
	}

	// アンダンプする
	void _undump(PhysicalFile::DirectArea::ID& id_, const char* p_)
	{
		// DirectArea::ID は 6バイト
		
		ModUInt32 pid;
		unsigned short aid;
		
		Os::Memory::copy(&pid, p_, sizeof(pid));
		p_ += sizeof(pid);
		Os::Memory::copy(&aid, p_, sizeof(aid));

		id_.m_uiPageID = pid;
		id_.m_uiAreaID = (aid == 0xffff) ? 0xffffffff : aid;
	}
}

//
//	FUNCTION public
//	KdTree::AreaVectorFile::AreaVectorFile -- コンストラクタ
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
AreaVectorFile::AreaVectorFile(FileID& cFileID_, const Os::Path& cPath_)
	: VectorFile(cFileID_, Os::Path(cPath_).addPart(_cSubPath)),
	  m_uiCountPerPage(0)
{
	// エントリのサイズを求める
	m_uiElementSize
		= static_cast<ModSize>(sizeof(ModUInt32) + sizeof(unsigned short));
	
	// 1ページに格納できるエントリ数を求める
	m_uiCountPerPage
		= Version::Page::getContentSize(m_pVersionFile->getPageSize())
		/ m_uiElementSize;
}

//
//	FUNCTION public
//	KdTree::AreaVectorFile::~AreaVectorFile -- デストラクタ
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
AreaVectorFile::~AreaVectorFile()
{
}

//
//	FUNCTION public
//	KdTree::AreaVectorFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		キー
//	PhysicalFile::DirectArea::ID& id_
//		バリュー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
AreaVectorFile::insert(ModUInt32 uiRowID_,
					   PhysicalFile::DirectArea::ID& id_)
{
	// 格納領域を得る
	char* p = getBuffer(uiRowID_);
	// 書き出す
	_dump(p, id_);

	// ヘッダーを更新する
	if (m_cHeader.m_uiMaxKey < uiRowID_)
		m_cHeader.m_uiMaxKey = uiRowID_;
	++m_cHeader.m_uiCount;
	dirtyHeaderPage();
}

//
//	FUNCTION public
//	KdTree::AreaVectorFile::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
AreaVectorFile::expunge(ModUInt32 uiRowID_)
{
	// 格納されている領域を得る
	char* p = getBuffer(uiRowID_);
	// 0xff で初期化する
	unsigned char c = 0xff;
	Os::Memory::set(p, c, m_uiElementSize);

	// ヘッダーを更新する
	--m_cHeader.m_uiCount;
	dirtyHeaderPage();
}

//
//	FUNCTION public
//	KdTree::AreaVectorFile::get -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	PhysicalFile::DirectArea::ID& id_
//		エリアID
//
//	RETURN
//	bool
//		null以外の場合はtrue、nullの場合はfalse
//
//	EXCEPTIONS
//
bool
AreaVectorFile::get(ModUInt32 uiRowID_, PhysicalFile::DirectArea::ID& id_)
{
	// 格納されている領域を得る
	const char* p = getConstBuffer(uiRowID_);
	if (p == 0)
		return false;

	// nullか?
	if (_isNull(p) == true)
		return false;
	
	// 値を得る
	_undump(id_, p);
	
	return true;
}

//
//	FUNCTION public
//	KdTree::AreaVectorFile::getAll -- 全件を得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::BitSet& cBitSet_
//		格納されているROWIDを格納するビットセット
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
AreaVectorFile::getAll(Common::BitSet& cBitSet_)
{
	ModUInt32 uiMaxKey = getMaxKey() + 1;
	for (ModUInt32 uiKey = 0; uiKey < uiMaxKey; uiKey += m_uiCountPerPage)
	{
		// ページの先頭を得る
		const char* p = getConstBuffer(uiKey);

		for (int i = 0; i < static_cast<int>(m_uiCountPerPage);
			 ++i, p += m_uiElementSize)
		{
			if (_isNull(p) == false)
				// 格納されている
				cBitSet_.set(uiKey + i);
		}
	}
}

//
//	FUNCTION public
//	KdTree::AreaVectorFile::getPageData
//		-- 指定ページ内のキーとバリューを得る
//
//	NOTES
//
//	ARGUMENTS
//	Version::Page::ID pageID_
//		ページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
AreaVectorFile::
getPageData(Version::Page::ID pageID_,
			Common::LargeVector<
				ModPair<ModUInt32, PhysicalFile::DirectArea::ID> >& vecData_)
{
	// 指定ページを attach する
	// 指定されたページが存在するかどうかなどのチェックはしない
	
	fixPage(pageID_);
	
	// このページの先頭のキー値を得る
	
	ModUInt32 uiKey = m_uiCountPerPage * (pageID_ - getMinPageID());

	// ページの先頭を得る
	
	const char* p = m_pCurrentPage->getConstBuffer();

	// 格納領域を確保する

	// ページ内のデータを呼び出す
	
	for (ModUInt32 i = 0; i < m_uiCountPerPage; ++i, p += m_uiElementSize)
	{
		if (_isNull(p) == false)
		{
			// エリアIDを得る
			PhysicalFile::DirectArea::ID id;
			_undump(id, p);

			// 配列に追加する
			vecData_.pushBack(
				ModPair<ModUInt32, PhysicalFile::DirectArea::ID>(
					uiKey + i, id));
		}
	}
}

//
//	FUNCTION public
//	KdTree::AreaVectorFile::move -- ファイルを移動する
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
AreaVectorFile::move(const Trans::Transaction& cTransaction_,
				 const Os::Path& cPath_)
{
	VectorFile::move(cTransaction_, Os::Path(cPath_).addPart(_cSubPath));
}

//
//	FUNCTION protected
//	KdTree::AreaVectorFile::getBuffer -- 格納領域のメモリを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//
//	RETURN
//	char*
//		格納領域へのポインタ
//
//	EXCEPTIONS
//
char*
AreaVectorFile::getBuffer(ModUInt32 uiRowID_)
{
	// キー -> ページID+オフセットへ変換
	int offset = 0;
	Version::Page::ID id = convertToPageID(uiRowID_, offset);

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
//	KdTree::AreaVectorFile::getConstBuffer -- 格納領域のメモリを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//
//	RETURN
//	const char*
//		格納領域へのポインタ
//
//	EXCEPTIONS
//
const char*
AreaVectorFile::getConstBuffer(ModUInt32 uiRowID_)
{
	// キー -> ページID+オフセットへ変換
	int offset = 0;
	Version::Page::ID id = convertToPageID(uiRowID_, offset);

	if (id > getMaxPageID())
		// 最大ページIDより大きい
		return 0;

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
