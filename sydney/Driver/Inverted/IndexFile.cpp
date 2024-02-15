// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IndexFile.cpp -- 転置ファイルのラッパークラス
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#include "Common/Object.h"

#include "Inverted/DocumentIDVectorFile.h"
#include "Inverted/Sign.h"
#include "Inverted/IndexFile.h"

#include "Common/Assert.h"
#include "Common/StringArrayData.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"

#include "LogicalFile/OpenOption.h"
#include "LogicalFile/FileID.h"

#include "Trans/Transaction.h"

#include "Os/Path.h"

#include "ModLanguageSet.h"
#include "ModOsDriver.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::IndexFile::IndexFile -- コンストラクタ(1)
//
//	NOTES
//	大転置用のコンストラクタ
//
//	ARGUMENTS
//	LogicalFile::FileID& cFileID_
//		ファイルID
//	const Os::Path& cParent_
//		親ディレクトリ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
IndexFile::IndexFile(LogicalFile::FileID& cFileID_,
					 const Os::Path& cPath_)
	: IntermediateFile(&cFileID_, cPath_, _Directory)
		,m_bOpened(false)
{
	sign = Inverted::Sign::_FullInvert;
	id = -1;
}

//
//	FUNCTION public
//	Inverted::IndexFile::IndexFile -- コンストラクタ(2)
//
//	NOTES
//	小転置用のコンストラクタ
//
//	ARGUMENTS
//	LogicalFile::FileID& cFileID_
//		ファイルID
//	const Os::Path& cParent_
//		親ディレクトリ
//	const ModUnicodeString& cDirectory_
//		サブディレクトリ名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//

IndexFile::IndexFile(LogicalFile::FileID& cFileID_,
					 const Os::Path& cParent_,
					 const ModUnicodeString& cDirectory_,
					 const int   id_ )
	: IntermediateFile(&cFileID_, cParent_, cDirectory_)
{
	if(cDirectory_ == Inverted::_Insert0 )
	{
		sign = Inverted::Sign::_Insert0;
	}
	else if(cDirectory_ == Inverted::_Insert1)
	{
		sign = Inverted::Sign::_Insert1;
	}
	else if(cDirectory_ == Inverted::_Delete0)
	{
		sign = Inverted::Sign::_Delete0;
	}
	else if(cDirectory_ == Inverted::_Delete1)
	{
		sign = Inverted::Sign::_Delete1;
	}
	id = id_;
}

//
//	FUNCTION public
//	Inverted::IndexFile::~IndexFile -- デストラクタ
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
IndexFile::~IndexFile()
{
}

//	FUNCTION public
//	Inverted::IndexFile::destroy -- ファイルを破棄する
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

void
IndexFile::destroy(const Trans::Transaction& cTransaction_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく削除する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	InvertedFile::destroy(cTransaction_);
	ModOsDriver::File::rmAll(getPath(), ModTrue);
}

//
//	FUNCTION public
//	Inverted::IndexFile::recover -- 障害から回復する
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
IndexFile::recover(const Trans::Transaction& cTransaction_,
					 const Trans::TimeStamp& cPoint_)
{
	if (isMounted(cTransaction_))
	{
		InvertedFile::recover(cTransaction_, cPoint_);
		if (!isAccessible())
		{
			// リカバリの結果
			// 実体である OS ファイルが存在しなくなったので、
			// サブディレクトを削除する
			Os::Directory::remove(getPath());
		}
	}
}

//
//	FUNCTION public
//	Inverted::IndexFile::open -- ファイルをオープンする
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
IndexFile::open(const Trans::Transaction& cTransaction_,
				const LogicalFile::OpenOption& cOption_,
				bool bBatch)
{

	//
	//	【注意】
	//	転置ファイルのオープン・クローズは以下のように行う
	//	1. beginBatchInsert
	//	2. open
	//	...
	//	3. endBatchInsert
	//	4. close
	//
	if (bBatch == true)
	{
		beginBatchInsert();
	}
	InvertedFile::open(cTransaction_, cOption_);

	m_bOpened = true;

}
//
//	FUNCTION public
//	Inverted::IndexFile::close -- ファイルをクローズする
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
IndexFile::close(bool bBatch)
{
	if (bBatch == true)
	{
		endBatchInsert();
	}
	InvertedFile::close();
	m_bOpened = false;
}
//
//	FUNCTION public
//	Inverted::IndexFile::move -- ファイルを移動する
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
IndexFile::move(const Trans::Transaction& cTransaction_,
				const Common::StringArrayData& cArea_)
{
	// 現在のパスを得る
	Os::Path cOrgPath = getPath();

	// 新しいパス
	Os::Path cPath = getNewPath(cArea_.getElement(0));
	bool accessible = (isAccessible() &&
					   Os::Path::compare(cOrgPath, cPath)
					   == Os::Path::CompareResult::Unrelated);
	int step = 0;
	try
	{
		InvertedFile::move(cTransaction_, cPath);
		step++;
		if (accessible)
			// 古いディレクトリを削除する
			Os::Directory::remove(cOrgPath);
		step++;
	}
	catch (...)
	{
		switch (step)
		{
		case 1:
			InvertedFile::move(cTransaction_, cOrgPath);
		case 0:
			if (accessible)
				Os::Directory::remove(cPath);
		}
		_SYDNEY_RETHROW;
	}

	// 新しいパスを設定する
	setNewPath(cArea_.getElement(0));
}
//
//	FUNCTION public
//	Inverted::IndexFile::mergeVectorFile -- ベクターファイルの内容をマージする
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::IndexFile* pInsertFile_
//		挿入用転置ファイル
//	Inverted::IndexFile* pExpungeFile_
//		削除用転置ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IndexFile::mergeVectorFile(IndexFile* pInsertFile_, IndexFile* pExpungeFile_)
{
	InvertedFile::mergeVectorFile(pInsertFile_,pExpungeFile_);
}

//
//  FUNCTION public
//  Inverted::IndexFile::getDocumentLength --
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
ModSize
IndexFile::getDocumentLength(ModUInt32 uiRowID_)
{
	ModSize uiLength = 0;
	
	if(sign & Sign::INSERT_MASK)
	{
		if (getCount() > 0)
		{
			// このファイルが空ではない場合
			
			ModUInt32 uiDocumentID = convertToDocumentID(uiRowID_);
			if (uiDocumentID != UndefinedDocumentID)
			{
				// このファイルに格納されている場合
			
				// ベクターを検索する
				ModUInt32 dummy;
				; _TRMEISTER_ASSERT(getDocumentIDVectorFile() != 0);
				getDocumentIDVectorFile()->find(uiDocumentID, dummy, uiLength);
			
				; _TRMEISTER_ASSERT(uiLength > 0);
			}
		}
	}
	
	return uiLength;
}

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
