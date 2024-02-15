// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IndexFileSet.cpp -- 転置ファイルのラッパークラス(遅延更新用)
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "ModVector.h"

#include "Inverted/Sign.h"
#include "Inverted/IndexFile.h"
#include "Inverted/IntermediateFileID.h"
#include "Inverted/IndexFileSet.h"

#include "Common/Assert.h"
#include "Common/StringArrayData.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"
#include "Trans/Transaction.h"
#include "Os/Path.h"
#include "Inverted/SortParameter.h"
#include "Inverted/FileIDNumber.h"
#include "Inverted/FieldType.h"
	
_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::IndexFileSet::IndexFileSet -- コンストラクタ(1)
//
//	NOTES
//	delayedオプション用
//
//	ARGUMENTS
//	Inverted::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
IndexFileSet::IndexFileSet(IntermediateFileID& cFileID_)
{
	m_cPath = cFileID_.getPath();
	// !!! 次の順序で必ずpush backする必要がある。 !!!
	// 転置ファイルのsignatureは、
	// 大転置: FullInvert
	// 挿入用小転置1：InsertInvert
	// 挿入用小転置2：InsertInvert1
	// 削除用小転置1：DeleteInvert
	// 削除用小転置2：DeleteInvert1
	this->reserve(5);
	pushBack(IndexFile(cFileID_.getInverted(),
					   cFileID_.getPath()));
	pushBack(IndexFile(cFileID_.getInsert0(),
					   cFileID_.getPath(), _Insert0,FileIDNumber::_Ins0));
	pushBack(IndexFile(cFileID_.getInsert1(),
					   cFileID_.getPath(), _Insert1,FileIDNumber::_Ins1));
	pushBack(IndexFile(cFileID_.getExpunge0(),
					   cFileID_.getPath(), _Delete0,FileIDNumber::_Exp0));
	pushBack(IndexFile(cFileID_.getExpunge1(),
					   cFileID_.getPath(), _Delete1,FileIDNumber::_Exp1));
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::IndexFileSet -- コンストラクタ(2)
//
//	NOTES
//	delayedオプションなし用
//
//	ARGUMENTS
//	LogicalFile::FileID& cFileID_
//		ファイルID
//	const Os::Path& cPath_
//		パス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
IndexFileSet::IndexFileSet(LogicalFile::FileID& cFileID_,const Os::Path& cPath_)
{
	m_cPath = cPath_;
	pushBack(IndexFile(cFileID_, cPath_));
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::~IndexFileSet -- デストラクタ
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
IndexFileSet::~IndexFileSet()
{
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::getFullInvert --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModVector<IndexFile>::Iterator
IndexFileSet::getFullInvert()
{
	return this->find(Inverted::Sign::_FullInvert);
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::getSize -- ファイルサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		ファイルサイズ
//
//	EXCEPTIONS
//
ModUInt64
IndexFileSet::getSize(const Trans::Transaction& cTrans_)
{
	ModUInt64 size = 0;
	for (Iterator iter = begin(); iter != end(); ++iter)
	{
		size += (*iter).getSize(cTrans_);
	}
	return size;
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::getCount -- ファイルに挿入されているタプル数を得る
//  転置ファイルに格納されている文書頻度を求めるのと同じ
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInt64
//		タプル数
//
//	EXCEPTIONS
//
ModInt64
IndexFileSet::getCount()
{
	ModUInt64 count = 0;
	for (Iterator iter = begin(); iter != end(); ++iter)
	{
		if ((*iter).signature() & Inverted::Sign::DELETE_MASK)
		{
			count -= (*iter).getCount();	// 削除転置
		}
		else
		{
			count += (*iter).getCount();	// 挿入転置
		}
	}
	return count;
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::getTotalDocumentLength -- 総文書長を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		総文書長
//
//	EXCEPTIONS
//
ModUInt64
IndexFileSet::getTotalDocumentLength()
{
	ModUInt64 length = 0;
	for (Iterator iter = begin(); iter != end(); ++iter)
	{
		if ((*iter).signature() & Inverted::Sign::DELETE_MASK)
		{
			length -= (*iter).getTotalDocumentLength();	// 削除転置
		}
		else
		{
			length += (*iter).getTotalDocumentLength();	// 挿入転置
		}
	}
	return length;
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::create -- ファイルを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	const LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
void
IndexFileSet::create(const Trans::Transaction& cTransaction_,
					 IntermediateFileID& cFileID_)
{
	Iterator iter;
	try
	{
		iter = begin();
		if ((*iter).signature() != Inverted::Sign::_FullInvert)
			// 先頭は必ず大転置
			_SYDNEY_THROW0(Exception::BadArgument);
		
		cFileID_.setInverted((*iter).create(cTransaction_, true));

		for (++iter; iter != end(); ++iter)
		{
			cFileID_.setInverted((*iter).create(cTransaction_, false),
								 (*iter).getID());
		}
	}
	catch (...)
	{
		for (; iter >= begin(); --iter)
		{
			(*iter).destroy(cTransaction_);
		}
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::destroy -- ファイルを破棄する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IndexFileSet::destroy(const Trans::Transaction& cTransaction_)
{
	for (Iterator iter = begin(); iter != end(); ++iter)
	{
		(*iter).destroy(cTransaction_);
	}
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::mount -- マウントする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IndexFileSet::mount(const Trans::Transaction& cTransaction_)
{
	Iterator iter;
	try
	{
		for (iter = begin(); iter != end(); ++iter)
		{
			(*iter).mount(cTransaction_);
		}
	}
	catch (...)
	{
		for (; iter >= begin(); --iter)
		{
			(*iter).unmount(cTransaction_);
		}
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::unmount -- アンマウントする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IndexFileSet::unmount(const Trans::Transaction& cTransaction_)
{
	Iterator iter;
	try
	{
		for (iter = begin(); iter != end(); ++iter)
		{
			(*iter).unmount(cTransaction_);
		}
	}
	catch (...)
	{
		for (; iter >= begin(); --iter)
		{
			(*iter).mount(cTransaction_);
		}
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::flush -- フラッシュする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IndexFileSet::flush(const Trans::Transaction& cTransaction_)
{
	for (Iterator iter = begin(); iter != end(); ++iter)
	{
		(*iter).flush(cTransaction_);
	}
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::startBackup -- バックアップを開始する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const bool bRestorable_
//		リストアフラグ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IndexFileSet::startBackup(const Trans::Transaction& cTransaction_,
							const bool bRestorable_)
{
	Iterator iter;
	try
	{
		for (iter = begin(); iter != end(); ++iter)
		{
			(*iter).startBackup(cTransaction_, bRestorable_);
		}
	}
	catch (...)
	{
		for (;iter >= begin(); --iter)
		{
			(*iter).endBackup(cTransaction_);
		}
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::endBackup -- バックアップを終了する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IndexFileSet::endBackup(const Trans::Transaction& cTransaction_)
{
	for (Iterator iter = begin(); iter != end(); ++iter)
	{
		(*iter).endBackup(cTransaction_);
	}
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::recover -- 障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Trans::TimeStamp& cPoint_
//		チェックポイント
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IndexFileSet::recover(const Trans::Transaction& cTransaction_,
						const Trans::TimeStamp& cPoint_)
{
	for (Iterator iter = begin(); iter != end(); ++iter)
	{
		(*iter).recover(cTransaction_, cPoint_);
	}
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::restore
//		-- ある時点に開始された読取専用トランザクションが
//			参照する版を最新版とする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Trans::TimeStamp& cPoint_
//		チェックポイント
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IndexFileSet::restore(const Trans::Transaction& cTransaction_,
						const Trans::TimeStamp& cPoint_)
{
	for (Iterator iter = begin(); iter != end(); ++iter)
	{
		(*iter).restore(cTransaction_, cPoint_);
	}
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::open -- ファイルをオープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const LogicalFile::OpenOption& cOption_
//		オープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IndexFileSet::open(const Trans::Transaction& cTransaction_,
				   const LogicalFile::OpenOption& cOption_,
				   bool bBatch)
{
	if (bBatch == true)
	{
		getFullInvert()->open(cTransaction_, cOption_, bBatch);
	}
	else
	{
		for (Iterator iter = begin(); iter != end(); ++iter)
		{
			(*iter).open(cTransaction_, cOption_, bBatch);
		}
	}
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::close -- ファイルをクローズする
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
IndexFileSet::close(bool bBatch)
{
	if (bBatch == true)
	{
		getFullInvert()->close(bBatch);
	}
	else
	{
		for(Iterator iter = begin(); iter != end(); ++iter)
		{
			(*iter).close(bBatch);
		}
	}
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::sync -- 同期を取る
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	bool& incomplete
//			処理し残したかどうか
//	bool& modified
//		更新されたかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IndexFileSet::sync(const Trans::Transaction& cTransaction_,
					 bool& incomplete, bool& modified)
{
	for (Iterator iter = begin(); iter != end(); ++iter)
	{
		(*iter).sync(cTransaction_, incomplete, modified);
	}
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::recoverAllPages -- すべてのページの更新を破棄する
//
//	NOTES
//	といっても、本当に破棄できるのはInfoFileのみ。
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
IndexFileSet::recoverAllPages()
{
	for (Iterator iter = begin(); iter != end(); ++iter)
	{
		(*iter).recoverAllPages();
	}
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::flushAllPages -- すべてのページの更新を確定する
//
//	NOTES
//	といっても、本当に確定できるのはInfoFileのみ。
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
IndexFileSet::flushAllPages()
{
	for (Iterator iter = begin(); iter != end(); ++iter)
	{
		(*iter).flushAllPages();
	}
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::move -- ファイルを移動する
//
//	NOTES
//	移動元と移動先のパスが異なっていることが前提。
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Common::StringArrayData& cArea_
//		移動先のエリア
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IndexFileSet::move(const Trans::Transaction& cTransaction_,
					const Common::StringArrayData& cArea_)
{
	// 現在のパスを得る
	Common::StringArrayData cOrgArea;
	cOrgArea.setElement(0, m_cPath);
	Iterator iter;
	try
	{
		for (iter = begin(); iter != end(); ++iter)
		{
			(*iter).move(cTransaction_, cArea_);
		}
		m_cPath = cArea_.getElement(0);
	}
	catch (...)
	{
		for (; iter >= begin(); --iter)
		{
			(*iter).move(cTransaction_, cOrgArea);
		}
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::find --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModVector<IndexFile>::Iterator
IndexFileSet::find(ModUInt32 signature )
{
	for (Iterator iter = begin();  iter != end(); iter++)
	{
		if((*iter).signature() == signature)
			return iter;
	}
	return NULL;
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::findEntry --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModVector<IndexFile>::Iterator
IndexFileSet::findEntity(ModUInt32 signature )
{
	// hash関数により転置ファイルが格納されているindex値を求める
	int n = hash(signature);
	// 値の検査
	if( n >= 0 && n < (int)ModVector <IndexFile>::getSize()){
		return this->begin() + n;
	}
	return NULL;
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::hash -- bit位置を返す
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
int
IndexFileSet::hash(ModUInt32 sign)
{
	int i;
	for(i = 0 ; i < sizeof(ModUInt32)*8 ; i++){
		if(sign & 0x01){
			return i;
		}
		sign >>= 1;
	}
	return i;
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::setTokenizer --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
IndexFileSet::setTokenizer(ModInvertedTokenizer *tokenizer_)
{
	for (IndexFileSet::Iterator iter = this->begin();
		iter != this->end(); ++iter)
	{
		(*iter).setTokenizer(tokenizer_);
	}
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::getDocumentLength --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModSize
IndexFileSet::getDocumentLength(ModUInt32 uiRowID_)
{
	ModSize uiLength = 0;
	
	ModUInt32 uiDocumentID = UndefinedDocumentID;
	for (Iterator i = begin(); i != end(); ++i)
	{
		uiLength = i->getDocumentLength(uiRowID_);
		if (uiLength > 0)
		{
			break;
		}
	}

	; _TRMEISTER_ASSERT(uiLength > 0);
	return uiLength;
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::contains --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
bool
IndexFileSet::contains(ModUInt32 uiRowID_)
{
	for (Iterator iter = begin(); iter != end(); ++iter)
	{
		if ((*iter).contains(uiRowID_))
			return true;
	}
	return false;
}

//
//	FUNCTION public
//	Inverted::IndexFileSet::check --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
bool
IndexFileSet::check(const ModUnicodeString& cstrDocument_,
					ModUInt32 uiTupleID_,
					const ModVector<ModLanguageSet>& vecLanguage_,
					ModVector<ModSize>& vecSectionByteOffset_)
{
	for (Iterator iter = begin(); iter != end(); ++iter)
	{
		if ((*iter).check(cstrDocument_,
						  uiTupleID_,
						  vecLanguage_,
						  vecSectionByteOffset_))
			return true;
	}
	return false;
}

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
