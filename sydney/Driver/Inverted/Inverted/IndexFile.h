// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IndexFile.h --
// 
// Copyright (c) 2003, 2004, 2005, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_INDEXFILE_H
#define __SYDNEY_INVERTED_INDEXFILE_H

#include "Common/Object.h"
#include "Common/DataArrayData.h"
#include "Common/StringArrayData.h"
#include "Common/Message.h"

#include "Inverted/Module.h"
#include "Inverted/Sign.h"
#include "Inverted/IntermediateFile.h"

#include "Inverted/InvertedList.h"
#include "Inverted/InvertedFile.h"

#include "LogicalFile/FileID.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "Os/Path.h"

#include "ModUnicodeString.h"
#include "ModLanguageSet.h"



_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

namespace
{
	//
	//	VARIABLE local
	//	_$$::_Directory -- ディレクトリ
	//
	ModUnicodeString _Directory("FullInvert");
	//
	//	VARIABLE local
	//	_$$::_Insert0 -- ディレクトリ
	//
	//	NOTE
	//	_Insert0と_Insert1は挿入用小転置のディレクトリ名を意味し、
	//	_Delete0と_Delete1は削除用小転置のディレクトリ名を意味する。
	//  小転置は、削除用とマージ用の役割が変わるため。
	//  ディレクトリ名で更新用とマージ用の区別はできない。
	//	参考 Inverted::Sign::Value
	//
	ModUnicodeString _Insert0("InsertInvert");
	ModUnicodeString _Insert1("InsertInvert1");
	ModUnicodeString _Delete0("DeleteInvert");
	ModUnicodeString _Delete1("DeleteInvert1");
}
//
//	CLASS
//	Inverted::IndexFile --
//
//	NOTES
//
class IndexFile : public IntermediateFile
{
public:
	SYD_INVERTED_FUNCTION
	IndexFile(){}
	// コンストラクタ(1) -- 大転置用
	SYD_INVERTED_FUNCTION
	IndexFile(LogicalFile::FileID& cFileID_,
				const Os::Path& cParent_);
	// コンストラクタ(2) -- 小転置用
	SYD_INVERTED_FUNCTION
	IndexFile(LogicalFile::FileID& cFileID_,
				const Os::Path& cParent_,
				const ModUnicodeString& cDirectory_,
				const int id_=-1);	// -1は、大転置ファイルを意味する
	// デストラクタ
	SYD_INVERTED_FUNCTION
	virtual ~IndexFile();



	// ファイルを破棄する
	SYD_INVERTED_FUNCTION
	void destroy(const Trans::Transaction& cTransaction_);


	// ファイルを障害から回復する
	SYD_INVERTED_FUNCTION
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);
	//論理ファイルをオープンする
	SYD_INVERTED_FUNCTION
	virtual void open(const Trans::Transaction& cTransaction_,
				const LogicalFile::OpenOption& cOption_,
				bool bBatch = false);

	//論理ファイルをクローズする
	SYD_INVERTED_FUNCTION
	void close(bool bBatch = false);

	//ファイルを移動する
	SYD_INVERTED_FUNCTION
	void move(const Trans::Transaction& cTransaction_,
				const Common::StringArrayData& cArea_);


	// すべてのページの更新を破棄する
	SYD_INVERTED_FUNCTION
	void recoverAllPages()
	{
		InvertedFile::detachAllPages();
	}

	// すべてのページの更新を確定する
	SYD_INVERTED_FUNCTION
	void flushAllPages()
	{
		InvertedFile::detachAllPages();
	}


	SYD_INVERTED_FUNCTION
	void insert(ModInvertedTokenizer *pTokenizer_,
				const ModUnicodeString& cstrDocument_,
				const ModVector<ModLanguageSet>& vecLanguage_,
				ModUInt32 uiTupleID_,
				ModVector<ModSize>& vecSectionOffset_,
				ModInvertedFeatureList& vecFeature_)
	{
		setTokenizer(pTokenizer_);
		InvertedFile::insert(cstrDocument_,
							 vecLanguage_,
							 uiTupleID_,
							 vecSectionOffset_,
							 vecFeature_);
	}

	SYD_INVERTED_FUNCTION
	void expunge(ModInvertedTokenizer *pTokenizer_,
				const ModUnicodeString& cstrDocument_,
				const ModVector<ModLanguageSet>& vecLanguage_,
				ModUInt32 uiTupleID_,
				const ModVector<ModSize>& vecSectionOffset_)
	{
		setTokenizer(pTokenizer_);
		InvertedFile::expunge(	cstrDocument_,
								vecLanguage_,
								uiTupleID_,
								vecSectionOffset_);
	}

	SYD_INVERTED_FUNCTION	
	ModUInt32 signature()
	{
		return sign;
	}

	SYD_INVERTED_FUNCTION	
	void signature(ModUInt32 sign_)
	{
		sign = sign_;
	}

	// ベクターファイルの内容をマージする
	SYD_INVERTED_FUNCTION
	void mergeVectorFile(IndexFile* pInsertFile_, IndexFile* pExpungeFile_);
	// オープンされているか
	SYD_INVERTED_FUNCTION
	bool isOpen() { return m_bOpened; }
	int getID() { return id; }
	
	// 文書長を取得
	ModSize getDocumentLength(ModUInt32 uiRowID_);
	
private:
	// オープンされているかどうか
	bool m_bOpened;
	ModUInt32 sign;
	int id;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_INDEXFILE_H

//
//	Copyright (c) 2003, 2004, 2005, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
