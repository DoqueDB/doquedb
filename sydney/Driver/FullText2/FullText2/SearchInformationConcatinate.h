// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchInformationConcatinate.h --
// 
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_SEARCHINFORMATIONCONCATINATE_H
#define __SYDNEY_FULLTEXT2_SEARCHINFORMATIONCONCATINATE_H

#include "FullText2/Module.h"
#include "FullText2/SearchInformationArray.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::SearchInformationConcatinate -- 
//
//	NOTES
//	連結セクション用の検索情報クラス
//
class SearchInformationConcatinate : public SearchInformationArray
{
public:
	// コンストラクタ
	SearchInformationConcatinate(FileID& cFileID_);
	// デストラクタ
	virtual ~SearchInformationConcatinate();
	// コピーコンストラクタ
	SearchInformationConcatinate(const SearchInformationConcatinate& cSrc_);

	// 文書長を得る
	bool getDocumentLength(DocumentID id_, ModSize& length_);
	
	// コピーを得る
	SearchInformation* copy() const;
	
private:
	// 総文書長を得る
	ModUInt64 getTotalDocumentLengthImpl();
	// 総文書長
	ModUInt64 m_ulTotalDocumentLength;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_SEARCHINFORMATIONCONCATINATE_H

//
//  Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
