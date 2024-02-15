// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// GetDocumentFrequencyForEstimate.h --
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

#ifndef __SYDNEY_FULLTEXT2_GETDOCUMENTFREQUENCYFORESTIMATE_H
#define __SYDNEY_FULLTEXT2_GETDOCUMENTFREQUENCYFORESTIMATE_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"
#include "Utility/OpenMP.h"

#include "FullText2/SearchInformation.h"

#include "Os/CriticalSection.h"

#include "ModMap.h"
#include "ModPair.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::GetDocumentFrequencyForEstimate -- 
//
//	NOTES
//	OpenMPで並列処理を行いDF値等を取得する
//
class GetDocumentFrequencyForEstimate : public Utility::OpenMP
{
public:
	// コンストラクタ
	GetDocumentFrequencyForEstimate(SearchInformation& cSearchInfo_);
	// デストラクタ
	virtual ~GetDocumentFrequencyForEstimate();

	// 前処理
	void prepare();
	// マルチスレッドで実行するメソッド
	void parallel();
	// 後処理
	void dispose();

private:
	// 次に検索結果件数を見積もるノードを得る
	SearchInformation::MapValue* getNextMapValue();
	
	// 検索情報クラス
	SearchInformation& m_cSearchInfo;
	// スレッド数分のコピー
	ModVector<SearchInformation*> m_vecpSearchInfo;

	// ノードへのイテレータ
	ModMap<ModUnicodeString, SearchInformation::MapValue,
		   ModLess<ModUnicodeString> >::Iterator m_ite;
	ModMap<ModUnicodeString, SearchInformation::MapValue,
		   ModLess<ModUnicodeString> >::Iterator m_end;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_GETDOCUMENTFREQUENCYFORESTIMATE_H

//
//  Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
