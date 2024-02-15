// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HintArray.h
// 
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FILECOMMON_HINTARRAY_H
#define __SYDNEY_FILECOMMON_HINTARRAY_H

#include "ModTypes.h"
#include "ModVector.h"

#include "FileCommon/Module.h"

#include "Common/Object.h"

_SYDNEY_BEGIN

namespace FileCommon
{

namespace HintOffset {
const ModUInt32	Undefined = ModUInt32Max;
}
namespace HintSize {
const ModUInt32	Undefined = ModUInt32Max;
}

class SYD_FILECOMMON_FUNCTION HintElement : public Common::Object
{
public:

#ifndef SYD_COVERAGE
	// デフォルトコンストラクタ
	HintElement();
#endif

	// 引数つきコンストラクタ
	HintElement(ModUInt32 ulKeyOffset_, ModUInt32 ulKeySize_,
				ModUInt32 ulValueOffset_, ModUInt32 ulValueSize_);

#ifndef SYD_COVERAGE
    // コピーコンストラクタ
	HintElement(const HintElement& cElement_);
#endif

	// デストラクタ
	~HintElement();
		 
	bool CompareToKey
		(const ModUnicodeString& hint, 
		 const ModUnicodeChar* key, const ModUInt32 length) const;

#ifndef SYD_COVERAGE
	bool CompareToValue
		(const ModUnicodeString& hint, 
		 const ModUnicodeChar* value, const ModUInt32 length) const;
#endif

	bool hasValue() const;
	// 新しいModUnicodeStringを生成していることに注意
	ModUnicodeString* getKey(const ModUnicodeString& cstrHint_) const;
	ModUnicodeString* getValue(const ModUnicodeString& cstrHint_) const;

private:
	// 先頭オフセットと文字数の対にする。
	// 以下の変数が取る値は全てUnicodeとしての文字数であり、
	// バイト数ではないことに注意!
	ModUInt32 m_ulKeyOffset;
	ModUInt32 m_ulKeySize;
	ModUInt32 m_ulValueOffset;
	ModUInt32 m_ulValueSize;
};

class SYD_FILECOMMON_FUNCTION HintArray : public ModVector<HintElement*>
{
public:
	// デフォルトコンストラクタ
	HintArray()
	{
	}

	// 同
	HintArray(const ModUnicodeString& cstrHint_)
	{
		initialize(cstrHint_);
	}

	// デストラクタ
	~HintArray();

	// 初期化関数（ヒント文字列を各要素に分解する）
	void initialize(const ModUnicodeString& cstrHint_);
};

} // end of namespace FileCommon

_SYDNEY_END

#endif // __SYDNEY_FILECOMMON_HINTARRAY_H

//
//	Copyright (c) 2001, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

