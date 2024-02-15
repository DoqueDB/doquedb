// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// GetLocationCapsule.h --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2009, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_GETLOCATIONCAPSULE_H
#define __SYDNEY_INVERTED_GETLOCATIONCAPSULE_H

#include "Inverted/Module.h"
#include "Inverted/SearchCapsule.h"

class ModInvertedQuery;
class ModInvertedTokenizer;

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class InvertedFile;
class SearchCapsule;
//
//	CLASS
//	Inverted::GetLocationCapsule --
//
//	NOTES
//
//
class GetLocationCapsule
{
public:
	//
	//	TYPEDEF
	//	Inverted::GetLocationCapsule::SubResult
	//		-- 検索結果
	//
	//	NOTE
	//	第一要素：索引語の終了位置
	//	第二要素：索引語のID
	//
	typedef ModPair<ModSize, ModSize> SubResult;
	
	//
	//	TYPEDEF
	//	Inverted::GetLocationCapsule::Result
	//		-- 検索結果
	//
	//	NOTE
	//	第一要素：索引語の開始位置
	//
	typedef ModPair<ModSize, SubResult> Result;

	//
	//	TYPEDEF
	//	Inverted::GetLocationCapsule::ResultSet
	//		-- 検索結果集合
	//
	typedef ModVector<Result> ResultSet;

	// コンストラクタ
	SYD_INVERTED_FUNCTION
#if 0
	GetLocationCapsule(SearchCapsule* pSearchCapsule_);
#else
	GetLocationCapsule(SearchCapsule& cSearchCapsule_);	
#endif
	// デストラクタ
	SYD_INVERTED_FUNCTION
	virtual ~GetLocationCapsule();

	// パラメータ設定
	SYD_INVERTED_FUNCTION
    void setUpperLimitPerTerm(ModSize uiLimit_)
		{ m_uiUpperLimitPerTerm = uiLimit_; }
	SYD_INVERTED_FUNCTION
    void setUpperTermLimit(ModSize uiLimit_)
		{ m_uiUpperTermLimit = uiLimit_; }

	// 実行する
	SYD_INVERTED_FUNCTION
	const ResultSet& execute(ModUInt32 uiRowID_,
							 ModSize& uiTermCount_);
	
private:
	// 1つのファイルの位置情報取得を実行する
	void search(InvertedFile* pFile_,
				ModUInt32 uiDocumentID_,
				ModSize& uiTermCount_);
	// ページをdetachする
	void detach();

#if 0
	// 検索クラス
	SearchCapsule * m_pSearchCapsule;
#endif
	// 検索クラス
	SearchCapsule& m_cSearchCapsule;

	// 検索結果集合
	ResultSet m_cResultSet;

	// 検索ノード
	ModInvertedQuery* m_pQuery;	// わざわざポインタにする必要はないが、利用者に
								// 余計なヘッダーファイルをincludeして
								// ほしくないので、ポインタにする

	// 1単語あたり何個の位置を取得するか？
	ModSize m_uiUpperLimitPerTerm;
	// 何単語分の位置リストを取得するか？
	ModSize m_uiUpperTermLimit;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_GETLOCATIONCAPSULE_H

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2009, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
