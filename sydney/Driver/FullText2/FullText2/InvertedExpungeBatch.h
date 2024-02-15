// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedExpungeBatch.h --
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

#ifndef __SYDNEY_FULLTEXT2_INVERTEDEXPUNGEBATCH_H
#define __SYDNEY_FULLTEXT2_INVERTEDEXPUNGEBATCH_H

#include "FullText2/Module.h"
#include "FullText2/InvertedBatch.h"

#include "Common/LargeVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::InvertedExpungeBatch
//		-- バッチインサート用のメモリ上で実装された転置ファイルを表すクラス
//		   削除用
//
//	NOTES
//
class InvertedExpungeBatch : public InvertedBatch
{
public:
	// コンストラクタ
	InvertedExpungeBatch(InvertedSection& cSection_);
	// デストラクタ
	virtual ~InvertedExpungeBatch();

	// ファイルの内容をクリアする
	void clear();

	// 小転置の文書IDを大転置の文書IDに変換する
	DocumentID convertToBigDocumentID(DocumentID uiSmallID_);
	// 大転置の文書IDを登録し、小転置の文書IDを採番する
	DocumentID assignDocumentID(DocumentID uiBigDocumentID_);

	// すべての文書IDを取得する(appendする)
	void getAll(Common::LargeVector<DocumentID>& vecID_);

private:
	// 小転置の文書IDがキー、大転置の文書IDがバリューのベクター
	Common::LargeVector<DocumentID> m_vecDocumentID;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_INVERTEDEXPUNGEBATCH_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
