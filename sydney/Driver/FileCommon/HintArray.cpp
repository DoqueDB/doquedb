// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HintArray.cpp -- ヒント文字列の解析結果を入れる配列の実装クラス
// 
// Copyright (c) 2001, 2002, 2003, 2008, 2023 Ricoh Company, Ltd.
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

namespace
{
const char	srcFile[] = __FILE__;
const char	moduleName[] = "FileCommon";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "FileCommon/DataManager.h"
#include "FileCommon/HintArray.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Exception/NotSupported.h"
#include "Exception/BadArgument.h"
#include "PhysicalFile/Page.h"

_SYDNEY_USING

using namespace FileCommon;

//// ここよりHintElementの関数定義

//
//	FUNCTION
//	FileCommon::HintElement::HintElement -- デフォルトコンストラクタ
//
//	NOTE
//		デフォルトコンストラクタ(ModVectorが要求)
//
//	ARGUMENTS
//		なし
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//	
#ifndef SYD_COVERAGE
		
HintElement::HintElement(): 
	m_ulKeyOffset(HintOffset::Undefined), 
	m_ulKeySize(HintSize::Undefined),
	m_ulValueOffset(HintOffset::Undefined), 
	m_ulValueSize(HintSize::Undefined)
{
}

#endif // end of #ifndef SYD_COVERAGE

//
//	FUNCTION
//	FileCommon::HintElement::HintElement -- 引数つきコンストラクタ
//
//	NOTE
//		引数つきコンストラクタ
//
//	ARGUMENTS
//		ModUInt32 ulKeyOffset_
//			
//		ModUInt32 ulKeySize_
//			
//		ModUInt32 ulValueOffset_
//			
//		ModUInt32 ulValueSize_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//			
HintElement::HintElement(ModUInt32 ulKeyOffset_, ModUInt32 ulKeySize_,
						 ModUInt32 ulValueOffset_, ModUInt32 ulValueSize_)
	: m_ulKeyOffset(ulKeyOffset_),
	  m_ulKeySize(ulKeySize_),
	  m_ulValueOffset(ulValueOffset_),
	  m_ulValueSize(ulValueSize_)
{
}

//
//	FUNCTION
//	FileCommon::HintElement::HintElement -- コピーコンストラクタ
//
//	NOTE
//		コピーコンストラクタ
//
//	ARGUMENTS
//		const HintElement& cElement_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//	
#ifndef SYD_COVERAGE
		
HintElement::HintElement(const HintElement& cElement_)
	: m_ulKeyOffset(cElement_.m_ulKeyOffset),
	  m_ulKeySize(cElement_.m_ulKeySize),
	  m_ulValueOffset(cElement_.m_ulValueOffset),
	  m_ulValueSize(cElement_.m_ulValueSize) 
{
}

#endif // end of #ifndef SYD_COVERAGE

//
//	FUNCTION
//	FileCommon::HintElement::~HintElement -- デストラクタ
//
//	NOTE
//		デストラクタ
//
//	ARGUMENTS
//		なし
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//			
HintElement::~HintElement()
{
}

//
//	FUNCTION
//	FileCommon::HintElement::CompareToKey -- 文字列比較
//
//	NOTE
//		文字列比較。
//		cstrHint+m_ulKeyOffsetからのm_ulKeySize文字が
//		pKey_からのulLength文字に等しいかどうかを調べる。
//
//	ARGUMENTS
//		const ModUnicodeString& cstrHint_
//			
//		const ModUnicodeChar* pOtherKey_
//			
//		const ModUInt32 ulLength_
//			
//	RETURN
//		bool 
//
//	EXCEPTIONS
//		なし
//			
bool 
HintElement::CompareToKey(const ModUnicodeString& cstrHint_, 
						  const ModUnicodeChar* pOtherKey_,
						  const ModUInt32 ulLength_) const
{
	if (m_ulKeySize != ulLength_) return false;

	const ModUnicodeChar* pKey = cstrHint_;
	pKey += m_ulKeyOffset;
	return (ModUnicodeCharTrait::compare
			(pKey, pOtherKey_, ModFalse, ulLength_) == 0);
}

//
#ifndef SYD_COVERAGE

bool 
HintElement::CompareToValue(const ModUnicodeString& cstrHint_, 
						  const ModUnicodeChar* pOtherValue_,
						  const ModUInt32 ulLength_) const
{
	if (m_ulValueSize != ulLength_) return false;

	const ModUnicodeChar* pValue = cstrHint_;
	pValue += m_ulValueOffset;
	return (ModUnicodeCharTrait::compare
			(pValue, pOtherValue_, ModFalse, ulLength_) == 0);
}

#endif // end of #ifndef SYD_COVERAGE

//
//	FUNCTION
//	FileCommon::HintElement::hasValue -- Valueが存在するかどうか調べる
//
//	NOTE
//		Valueが存在するかどうか調べる
//
//	ARGUMENTS
//		なし
//			
//	RETURN
//		bool 
//
//	EXCEPTIONS
//		なし
//			
bool 
HintElement::hasValue() const
{
	return m_ulValueOffset != HintOffset::Undefined;
}

//
//	FUNCTION
//	FileCommon::HintElement::getKey -- Keyを得る
//
//	NOTE
//		Keyを得る。
//		新しいModUnicodeStringを生成していることに注意。
//
//	ARGUMENTS
//		const ModUnicodeString& cstrHint_
//			
//	RETURN
//		ModUnicodeString*
//
//	EXCEPTIONS
//		Exception::BadArgument
//			
#ifdef OBSOLETE
ModUnicodeString*
HintElement::getKey(const ModUnicodeString& cstrHint_) const
{
	switch (m_ulKeySize)
	{
	case HintSize::Undefined:
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		break;
	case 0:
		return new ModUnicodeString("");
		break;
	default:
		const ModUnicodeChar* pKey = cstrHint_ + m_ulKeyOffset;
		return new ModUnicodeString(pKey, m_ulKeySize);
		break;
	}
}
#endif//OBSOLETE

//
//	FUNCTION
//	FileCommon::HintElement::getValue -- Valueを得る
//
//	NOTE
//		Valueを得る。
//
//	ARGUMENTS
//		const ModUnicodeString& cstrHint_
//			
//	RETURN
//		ModUnicodeString*
//
//	EXCEPTIONS
//		Exception::BadArgument
//			
ModUnicodeString*
HintElement::getValue(const ModUnicodeString& cstrHint_) const
{
	switch (m_ulValueSize)
	{
	case HintSize::Undefined:
	case 0:
		return new ModUnicodeString("");
		break;
	default:
		const ModUnicodeChar* pValue = cstrHint_ + m_ulValueOffset;
		return new ModUnicodeString(pValue, m_ulValueSize);
		break;
	}
}

//// ここまでHintElementの関数定義

//// ここよりHintArrayの関数定義

//
//	FUNCTION
//	FileCommon::HintArray::~HintArray -- デストラクタ
//
//	NOTE
//		デストラクタ。
//		vectorの中身はポインタなのでデストラクタで責任を持って片付ける。
//
//	ARGUMENTS
//		なし
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//			
HintArray::~HintArray()
{
	ModUInt32 n = getSize();
	for(ModUInt32 i=0; i<n; i++)
	{
		HintElement*& p = at(i);
		delete p, p=0;
	}
}

//
//	FUNCTION
//	FileCommon::HintArray::initialize -- ヒント文字列を各要素に分解する
//
//	NOTE
//		ヒント文字列を各要素に分解する。
//		第二引数に結果が入る。
//
//	ARGUMENTS
//		ModUnicodeString& cstrHintString_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			
void
HintArray::initialize(const ModUnicodeString& cstrHintString_)
{
	// 結果を入れる配列が空の配列であることを確認
	SydAssert(getSize() == 0);
	
	const ModUnicodeChar* pHint = cstrHintString_;

	ModUInt32 ulMax = cstrHintString_.getLength();
	ModUInt32 ulWordHead = HintOffset::Undefined;
	ModUInt32 ulWordTail = HintOffset::Undefined;
	ModUInt32 ulKeyHead  = HintOffset::Undefined;
	ModUInt32 ulKeySize  = HintSize::Undefined;
	ModInt32  lDepth    = 0;
	// ベクタの大きさ
	ModUInt32 n = 0;

	// reallocateが発生するより配列をなめるほうが速いので
	// ここで一度必要なサイズを調べる。ついでに括弧の対応もチェックする。
	ModUInt32 i=0;
	for(; i<=ulMax; ++i)
	{
		switch (pHint[i]) 
		{
		case Common::UnicodeChar::usLparent: // '('
			lDepth++;
			break;
		case Common::UnicodeChar::usRparent: // ')'
			lDepth--;
			if (lDepth < 0)
			{
				SydErrorMessage << "Hint Syntax Error: Too many ')'s." << ModEndl;
				throw Exception::BadArgument(moduleName, srcFile, __LINE__);
			}
			break;
		case Common::UnicodeChar::usNull:  // '\0'
			if (lDepth > 0)
			{
				SydErrorMessage << "Hint Syntax Error: Too many '('s." << ModEndl;
				throw Exception::BadArgument(moduleName, srcFile, __LINE__);
			}
			n++;
			break;
		case Common::UnicodeChar::usComma: // ','
			if (lDepth == 0) // 追加
				n++;
			break;
		}
	}
	SydAssert(lDepth == 0);

	// ヒント文字列に何もなければ何もする必要がない
	if (n == 0) return;

	// ベクターを必要数確保する
	reserve(n);

	for(i=0; i<=ulMax; ++i)
	{
		switch(pHint[i])
		{
		case Common::UnicodeChar::usLparent: // '('
			{
				if (ulKeyHead  == HintOffset::Undefined)
				{
					SydErrorMessage << "Hint Syntax Error: No '=' before '('." << ModEndl;
					throw Exception::BadArgument(moduleName, srcFile, __LINE__);
				}
				if (lDepth == 0)
					ulWordHead= i+1;
				lDepth++;
				break;
			}
		case Common::UnicodeChar::usRparent: // ')'
			{
				lDepth--;
				if (lDepth == 0)
					ulWordTail= i-1;
				break;
			}
		case Common::UnicodeChar::usEqual: // '='
			{
				if (lDepth > 0) break;
				if (ulKeyHead  == HintOffset::Undefined 
				 && ulWordHead == HintOffset::Undefined)
				{
					SydErrorMessage << "Hint Syntax Error: No key string." << ModEndl;
					throw Exception::BadArgument(moduleName, srcFile, __LINE__);
				}	
				if (ulKeyHead != HintOffset::Undefined)
				{
					SydErrorMessage << "Hint Syntax Error: Multiple '='s." << ModEndl;
					throw Exception::BadArgument(moduleName, srcFile, __LINE__);
				}
				// ulWord{Head, Tail}をulKey{Head, Tail}に移動
				ulKeyHead = ulWordHead;
				ulKeySize = ulWordTail-ulWordHead+1;
				// ulWord{Head, Tail}をクリア
				ulWordHead = HintOffset::Undefined;
				ulWordTail = HintOffset::Undefined;
			}
			break;
		case Common::UnicodeChar::usNull:  // '\0'
		case Common::UnicodeChar::usComma: // ','
			{
				if (lDepth > 0) break;
				if (ulWordHead == HintOffset::Undefined)
				{
					// 念のためクリア
					if (ulKeyHead != HintOffset::Undefined)
					{
						SydErrorMessage << "Hint Syntax Error: No value string." << ModEndl;
						throw Exception::BadArgument(moduleName, srcFile, __LINE__);
					}	
					else
						break;
				}
				if (ulKeyHead != HintOffset::Undefined)
				{
					pushBack(new HintElement(
						ulKeyHead, ulKeySize, 
						ulWordHead, ulWordTail-ulWordHead+1));
				}
				else
				{
					pushBack(new HintElement(
						ulWordHead, ulWordTail-ulWordHead+1,
						HintOffset::Undefined, HintSize::Undefined));
				}
				// ulKeyHead等の値を全てクリア
				ulKeyHead  = HintOffset::Undefined;
				ulKeySize  = HintSize::Undefined;
				ulWordHead = HintOffset::Undefined;
				ulWordTail = HintOffset::Undefined;
			}
			break;
		case Common::UnicodeChar::usCtrlTab: // '\t'
		case Common::UnicodeChar::usCtrlRet: // '\n'
		case Common::UnicodeChar::usCtrlCr:	 // '\r'
		case Common::UnicodeChar::usSpace:   // ' '
			// 読み飛ばす
			break;
		default:
			{
				if (ulWordHead == HintOffset::Undefined)
					ulWordHead= i;
				ulWordTail = i;
			}
			break;
		}
	}
}

//// ここまでHintArrayの関数定義

//
//	Copyright (c) 2001, 2002, 2003, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

