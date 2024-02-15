// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
//
// Unicode 文字情報ファイルのクラス
// 
// Copyright (c) 2023 Ricoh Company, Ltd.
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

#include "UnicodeDataRow.h"

//
// １つの文字に関する情報をあらわすクラス
//
class UnicodeDataRowUnicodeData : public UnicodeDataRow
{
public:

 	//
	// TYPE DEF
	//

	typedef unsigned short CodeValue;

	//
	// コンストラクタ、デストラクタ
	//

	UnicodeDataRowUnicodeData(const char* const line) : UnicodeDataRow(line, 14) {;}
	virtual ~UnicodeDataRowUnicodeData() {;}

	//
	// アクセッサ
	//

	CodeValue getCodeValue() const;

	const char*		getName() const
	{ return getField(1); }

	const char*		getCharacterType() const
	{ return getField(2); }

	const char*		getCombiningClass() const
	{ return getField(3); }

	const char*		getBidirectionalCategory() const
	{ return getField(4); }

	const char*		getCharacterDecomposition() const
	{ return getField(5); }

	int				getDecimalDigitValue() const;
	
	int				getDigitValue()	const;
	
	int				getNumericValue() const;

	bool			isMirrored() const;

	const char*		getOldName() const
	{ return getField(10); }

	const char*		getComment() const
	{ return getField(11); }

    CodeValue		getUpperCaseCodeValue() const;

	CodeValue 		getLowerCaseCodeValue() const;

	CodeValue 		getTitleCaseCodeValue() const;
};
