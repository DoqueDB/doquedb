// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DocumentIDVectorFile.h --
// 
// Copyright (c) 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_DOCUMENTIDVECTORFILE_H
#define __SYDNEY_INVERTED_DOCUMENTIDVECTORFILE_H

#include "Inverted/Module.h"
#include "Inverted/VectorFile.h"
#include "Inverted/ModInvertedDocumentLengthFile.h"
#include "Inverted/FileID.h"

#include "Trans/Transaction.h"
#include "Os/Path.h"

#include "ModPair.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

//
//	CLASS
//	Inverted::DocumentIDVectorFile --
//
//	NOTES
//
//
class DocumentIDVectorFile : public VectorFile<ModPair<ModUInt32, ModUInt32> >,
							 public ModInvertedDocumentLengthFile
{
public:

	//
	//	CONST
	//	Inverted::DocumentIDVectorFile::_SUBCLASS_HEADER_SIZE
	//
	enum { _SUBCLASS_HEADER_SIZE = 256 };
	
	//
	//	STRUCT
	//	Inverted::DocumentIDVectorFile::Header
	//
	struct Header
	{
		ModUInt32 m_uiLastDocumentID;		// 最終文書ID
		ModUInt32 m_uiDocumentCount;		// 文書数
		ModUInt64 m_ulTotalDocumentLength;	// 総文書長
		ModUInt32 m_uiVersion;				// 転置ファイルのバージョン
		ModUInt32 m_uiListCount;			// 転置リスト数

		// 以下分散環境でのみ使用
		ModInt32  m_iUnitCount;				// 転置ユニット数
		ModInt32  m_iInsertUnit;			// 挿入転置ユニット
		ModUInt64 m_ulMaxDocumentLength;	// １ユニットあたりの最大文書長
	};

	//
	//	STRUCT
	//	Inverted::DocumentIDVectorFile::Unit
	//
	struct Unit
	{
		ModUInt32 m_uiDocumentCount;
		ModUInt32 m_uiListCount;
		ModUInt64 m_ulTotalDocumentLength;
	};

	// コンストラクタ
	DocumentIDVectorFile(const FileID& cFileID_,
						 bool batch_);
	// デストラクタ
	virtual ~DocumentIDVectorFile();

	// ファイルを作成する
	void create();

	// 移動する
	void move(const Trans::Transaction& cTransaction_, const Os::Path& cPath_);

	// クリアする
	void clear(const Trans::Transaction& cTransaction_, bool bForce_);

	// 挿入
	void insert(ModUInt32 uiDocumentID_, ModInt32 iUnit_,
				ModUInt32 uiRowID_, ModUInt32 uiDocumentLength_);
	// 削除
	void expunge(ModUInt32 uiDocumentID_, ModInt32 iUnit_);
	// 検索
	bool find(ModUInt32 uiDocumentID_, ModUInt32& uiRowID_, ModUInt32& uiDocumentLength_);

	// 最終文書IDを得る
	ModUInt32 getLastDocumentID();

	// 最大文書IDを得る
	ModUInt32 getMaximumDocumentID();
	// 最小文書IDを得る
	ModUInt32 getMinimumDocumentID();

	// 総文書長を得る
	ModUInt64 getTotalDocumentLength();
	// 文書数を得る
	ModUInt32 getDocumentCount();
	// 転置リスト数を得る
	ModUInt32 getListCount();

	// 転置リスト数を増やす
	void incrementListCount(int element_);

	// 最大可能ユニット数を得る
	ModInt32 getMaxUnitCount();
	// ユニットの文書数を得る
	ModUInt32 getUnitDocumentCount(int element_);
	// ユニットの総文書長を得る
	ModUInt64 getUnitTotalDocumentLength(int element_);
	// 挿入に利用するユニット番号を得る
	ModInt32 getInsertUnit();
	// 挿入ユニット番号をチェックし、必要なら変更する
	bool checkInsertUnit();
	// ユニットにデータが挿入されているかどうか
	bool isInserted(int element_);
	// ユニットデータをクリアする
	void clearUnit(int element_);

	// 以下 ModInvertedDocumentLengthFile 用

	// 文書長を得る
	ModBoolean search(const ModUInt32 uiDocumentID_, ModSize& uiDocumentLength_) const;
	// 平均文書長を得る
	ModSize getAverageDocumentLength() const;

private:
	// ヘッダーを初期化する
	void initializeHeaderPage();

	// ユニット毎の文書長配列の先頭を得る
	Unit* getUnitArray();

	// ファイル分散数
	int m_iUnitCount;
	// ファイル分散を利用しているかどうか
	bool m_bDistribution;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_DOCUMENTIDVECTORFILE_H

//
//	Copyright (c) 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
