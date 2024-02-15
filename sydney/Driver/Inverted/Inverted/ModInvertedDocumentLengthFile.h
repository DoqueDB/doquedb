// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedDocumentLengthFile.h --
// 
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_MODINVERTEDDOCUMENTLENGTHFILE_H
#define __SYDNEY_INVERTED_MODINVERTEDDOCUMENTLENGTHFILE_H

#include "ModTypes.h"

//
//	CLASS
//	Inverted::ModInvertedDocumentLengthFile --
//
//	NOTES
//	本クラスはFTSの検索処理が必要とするインターフェース
//
class ModInvertedDocumentLengthFile 
{
public:
	//コンストラクタ/デストラクタ
	ModInvertedDocumentLengthFile() {}
	virtual ~ModInvertedDocumentLengthFile() {}

//	ModFileSize getTotalDocumentLength() const;
//	ModSize getDocumentNumber() const;
	virtual ModSize getAverageDocumentLength() const = 0;

	// 文書長を取得
	virtual ModBoolean search(const ModUInt32	ulDocumentID_,
							  ModSize&			ulDocumentLength_) const = 0;

};

#endif //__SYDNEY_INVERTED_MODINVERTEDDOCUMENTLENGTHFILE_H

//
//	Copyright (c) 2000, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
