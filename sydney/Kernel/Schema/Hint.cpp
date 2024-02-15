// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Hint.cpp --
// 
// Copyright (c) 2000, 2005, 2006, 2011, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Schema/Hint.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerData.h"
#include "Common/InputArchive.h"
#include "Common/Message.h"
#include "Common/OutputArchive.h"
#include "Common/StringArrayData.h"
#include "Common/UnicodeString.h"

#include "Exception/SQLSyntaxError.h"

#include "Statement/Hint.h"
#include "Statement/HintElementList.h"
#include "Statement/HintElement.h"
#ifdef OBSOLETE
#include "Statement/ItemReference.h"
#endif
#include "Statement/Literal.h"
#include "Statement/StringValue.h"
#include "Statement/Type.h"

#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_USING

namespace {

//	CONST local
//	_pszHintHeap -- ヒープを使うことを指示するヒント
//
//	NOTES

const char* _pszHintHeap = "heap";

//	CONST local
//	_pszHintNonTruncate -- 可変長文字列で末尾の空白を削らないことを指示するヒント
//
//	NOTES

const char* _pszHintNonTruncate = "nontruncate";

//	CONST local
//	_pszHintUnique -- Uniqueであることを指示するヒント
//
//	NOTES

const char* _pszHintUnique = "unique";

//	CONST local
//	_chDelimiter -- ヒントの区切り文字
//
//	NOTES

const char _chDelimiter = ',';

namespace _Function {

	//	Common::Dataをヒントの文字列にする
	void _makeString(const Common::Data::Pointer& pData_, ModUnicodeOstrStream& cStream_);
}

} // namespace

//	FUNCTION local
//	$$::_Function::_makeString -- Common::Dataをヒントの文字列にする
//
//	NOTES
//		ArrayDataが入れ子になっていてもフラットな文字列にする
//
//	ARGUMENTS
//		const Common::Data::Pointer& pData_
//			文字列にするデータ
//		ModUnicodeOstrStream& cStream_
//			結果の文字列を格納するストリーム
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
_Function::_makeString(const Common::Data::Pointer& pData_, ModUnicodeOstrStream& cStream_)
{
	; _SYDNEY_ASSERT(pData_.get());

	if (pData_->getType() == Common::DataType::Array) {
		const Common::ArrayData* pArray =
			_SYDNEY_DYNAMIC_CAST(const Common::ArrayData*, pData_.get());

		int n = pArray->getCount();
		if (!n) return;

		; _SYDNEY_ASSERT(n > 0);

		switch (pArray->getElementType()) {
		case Common::DataType::Data:
		{
			const Common::DataArrayData* pData =
				_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData_.get());

			for (int i = 0; i < n; i++)
				_makeString(pData->getElement(i), cStream_);

			break;
		}
		case Common::DataType::String:
		{
			const Common::StringArrayData* pData =
				_SYDNEY_DYNAMIC_CAST(const Common::StringArrayData*, pData_.get());

			if (!cStream_.isEmpty())
				cStream_ << _chDelimiter;

			cStream_ << pData->getElement(0);

			for (int i = 1; i < n; i++)
				cStream_ << _chDelimiter << pData->getElement(i);

			break;
		}
		default:
		{
			// 対応しない
			; _SYDNEY_ASSERT(false);
			break;
		}
		}

	} else {

		// スカラー型はそのまま文字列にして追加する

		if (!cStream_.isEmpty())
			cStream_ << _chDelimiter;
		cStream_ << pData_->getString();
	}
}

_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::Hint::Hint -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Hint::
Hint()
	: m_pElement(), m_iCategory(Category::Unknown)
	, m_uLowerBound(TupleID::Invalid), m_uUpperBound(TupleID::Invalid)
{ }

//	FUNCTION public
//	Schema::Hint::Hint -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::Hint& cStatement_
//			ヒントを表すSQL構文要素
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Hint::
Hint(const Statement::Hint& cStatement_)
	: m_pElement(), m_iCategory(Category::Unknown)
	, m_uLowerBound(TupleID::Invalid), m_uUpperBound(TupleID::Invalid)
{
	if (int n = cStatement_.getHintElementCount()) {
		; _SYDNEY_ASSERT(n == 1);

		if (Statement::HintElement* element = cStatement_.getHintElementAt(0)) {
			bool bLower = true;
			m_pElement = makeElementData(*element, &m_iCategory,
										 &m_uLowerBound, &m_uUpperBound, bLower);
		}
	}
}

//	FUNCTION public
//	Schema::Hint::Hint -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::Hint& cStatement_
//			ヒントを表すSQL構文要素
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Hint::
Hint(const Statement::Literal& cStatement_)
	: m_pElement(), m_iCategory(Category::Unknown)
	, m_uLowerBound(TupleID::Invalid), m_uUpperBound(TupleID::Invalid)
{
	bool bLower = true;
	m_pElement = makeElementData(cStatement_, &m_iCategory,
									 &m_uLowerBound, &m_uUpperBound, bLower);
}

//	FUNCTION public
//	Schema::Hint::Hint -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Hint& cHint_
//			コピー元のオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Hint::
Hint(const Hint& cHint_)
	: m_pElement(), m_iCategory(cHint_.m_iCategory)
{
	if (cHint_.getElement())
		m_pElement = cHint_.getElement()->copy();
}

//	FUNCTION public
//	Schema::Hint::~Hint -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Hint::
~Hint()
{
	destruct();
}

//	FUNCTION public
//	Schema::Hint::getString -- ヒントを文字列で得る
//
//	NOTES
//		ヒントの要素をそれぞれ文字列にして空白でつなげる
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた文字列
//
//	EXCEPTIONS
//		なし

ModUnicodeString
Hint::
getString() const
{
	ModUnicodeOstrStream cStream;

	if (m_pElement.get())
		_Function::_makeString(m_pElement, cStream);

	return ModUnicodeString(cStream.getString());
}

//	FUNCTION public
//	Schema::Hint::getWholeString -- get whole hint string
//
//	NOTES
//		this method is used to get whole hint string including heap, nontruncate and so on.
//
//	ARGUMENTS
//		Nothing
//
//	RETURN
//		ModUnicodeString
//
//	EXCEPTIONS

ModUnicodeString
Hint::
getWholeString() const
{
	ModUnicodeOstrStream cStream;

	if (m_iCategory & Category::Heap) {
		if (!cStream.isEmpty()) {
			cStream << ' ';
		}
		cStream << _pszHintHeap;
	}
	if (m_iCategory & Category::NonTruncate) {
		if (!cStream.isEmpty()) {
			cStream << ' ';
		}
		cStream << _pszHintNonTruncate;
	}
	if (m_iCategory & Category::Unique) {
		if (!cStream.isEmpty()) {
			cStream << ' ';
		}
		cStream << _pszHintUnique;
	}
	if (m_iCategory & Category::LogicalFile) {
		if (m_pElement.get() && !m_pElement->isNull()) {
			ModUnicodeOstrStream cFileHint;
			_Function::_makeString(m_pElement, cFileHint);
			if (!cStream.isEmpty()) {
				cStream << ' ';
			}
			cStream << '\'' << cFileHint.getString() << '\'';
		}
	}

	return ModUnicodeString(cStream.getString());
}

//	FUNCTION public
//	Schema::Hint::getCategory -- ヒントの種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::Hint::Category::Value
//			ヒントの種別
//
//	EXCEPTIONS
//		なし

Hint::Category::Value
Hint::
getCategory() const
{
	return m_iCategory;
}

//	FUNCTION public
//	Schema::Hint::getElement -- ヒントの要素を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const Common::Data*
//			ヒントの要素
//
//	EXCEPTIONS
//		なし

const Common::Data*
Hint::
getElement() const
{
	return m_pElement.get();
}

//	FUNCTION protected
//	Schema::Hint::setElement -- ヒントの要素を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Common::Data* pData_
//			設定するヒントの要素を表すデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Hint::
setElement(Common::Data* pData_)
{
	m_pElement = pData_;
}

//	FUNCTION protected
//	Schema::Hint::makeElementData --
//		ヒントを表すSQLの構文要素からデータを作る
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::Object& cObject_
//			ヒントを表す構文要素
//		Schema::Hint::Category::Value* pCategory_
//			ヒントの種別をセットするビットマップへのポインター
//
//	RETURN
//		変換したデータ
//
//	EXCEPTIONS
//		なし

// static
Common::Data::Pointer
Hint::
makeElementData(const Statement::Object& cObject_,
				Category::Value* pCategory_,
				TupleID::Value* pLowerBound_,
				TupleID::Value* pUpperBound_,
				bool& bLower_)
{
	; _SYDNEY_ASSERT(pCategory_);

	Common::Data::Pointer pResult;

	switch (cObject_.getType()) {
#ifdef OBSOLETE
	case Statement::ObjectType::ItemReference:
	{
		const Statement::ItemReference* pStatement = 
			_SYDNEY_DYNAMIC_CAST(const Statement::ItemReference*, &cObject_);

		const ModUnicodeString* pName = pStatement->getItemNameString();
		if (*pName == "B") {
			; _SYDNEY_ASSERT(bLower_);
			; _SYDNEY_ASSERT(pLowerBound_);
			bLower_ = false;
			*pCategory_ |= Category::PartialImport;
		}
		if (*pName == "E") {
			; _SYDNEY_ASSERT(!bLower_);
			; _SYDNEY_ASSERT(pUpperBound_);
			; _SYDNEY_ASSERT(*pCategory_ & Category::PartialImport);
		}
		break;
	}
#endif
	case Statement::ObjectType::Literal:
	{
		const Statement::Literal* pStatement =
			_SYDNEY_DYNAMIC_CAST(const Statement::Literal*, &cObject_);
		; _SYDNEY_ASSERT(pStatement);

		if (pStatement->isStringLiteral()) {

			// 文字列リテラルは論理ファイルに渡すヒントになる

			Common::Data::Pointer pData = pStatement->createData();
			if (pData.get())
				pResult = pData;

			*pCategory_ |= Category::LogicalFile;
		}
		else {
#ifdef OBSOLETE
			Common::Data::Pointer pValue =
				pStatement->createData(Common::DataType::Integer, true /* for assign */);
			Common::IntegerData* pData =
				_SYDNEY_DYNAMIC_CAST(Common::IntegerData*, pValue.get());
			; _SYDNEY_ASSERT(pData);

			if (bLower_) {
				; _SYDNEY_ASSERT(pLowerBound_);
				*pLowerBound_ = static_cast<TupleID::Value>(pData->getValue());
				bLower_ = false;
				*pCategory_ |= Category::PartialImport;

			} else {
				; _SYDNEY_ASSERT(pUpperBound_);
				; _SYDNEY_ASSERT(!bLower_);
				; _SYDNEY_ASSERT(*pCategory_ & Category::PartialImport);

				*pUpperBound_ = static_cast<TupleID::Value>(pData->getValue());
			}
#endif
			_SYDNEY_THROW1(Exception::SQLSyntaxError,
						   _TRMEISTER_U_STRING("Illegal hint element"));
		}
		break;
	}
	case Statement::ObjectType::StringValue:
	{
		const Statement::StringValue* pStatement =
			_SYDNEY_DYNAMIC_CAST(const Statement::StringValue*, &cObject_);
		; _SYDNEY_ASSERT(pStatement);
		; _SYDNEY_ASSERT(pStatement->getValue());

		if (pStatement->getValue()->compare(_TRMEISTER_U_STRING(_pszHintHeap),
											ModFalse) == 0) {
			// "heap"は無制限可変長の列に対し、ヒープファイルを作ることを
			// 指示する

			*pCategory_ |= Category::Heap;

		} else if (pStatement->getValue()->compare(_TRMEISTER_U_STRING(_pszHintNonTruncate),
												   ModFalse) == 0) {
			// "nontruncate"は可変長文字列型の列に対し、末尾の空白を削らないことを
			// 指示する

			*pCategory_ |= Category::NonTruncate;

		} else if (pStatement->getValue()->compare(_TRMEISTER_U_STRING(_pszHintUnique),
												   ModFalse) == 0) {
			// "unique"はB木索引に対し、Unique制約つきであることを指示する

			*pCategory_ |= Category::Unique;

		} else {
			// HINTの種類を補足的に示すものなので無視する
			// メッセージは出す
			SydInfoMessage << "Unrecognizable hint element ignored: " << *(pStatement->getValue());
		}

		break;
	}
	case Statement::ObjectType::HintElement:
	{
		// ArrayDataにする

		const Statement::HintElement* pStatement =
			_SYDNEY_DYNAMIC_CAST(const Statement::HintElement*, &cObject_);
		; _SYDNEY_ASSERT(pStatement);

		if (int n = pStatement->getHintPrimaryCount()) {

			ModVector<Common::Data::Pointer> vecData;

			for (int i = 0; i < n; i++) {
				if (Statement::Object*
					object = pStatement->getHintPrimaryAt(i)) {

					Common::Data::Pointer
						pData = makeElementData(*object, pCategory_,
												pLowerBound_,
												pUpperBound_,
												bLower_);
					if (pData.get())
						vecData.pushBack(pData);
				}
			}

			if (vecData.getSize() > 1)
				pResult = new Common::DataArrayData(vecData);
			else if (vecData.getSize() == 1) {
				pResult = vecData[0];
			}
		}

		break;
	}
	case Statement::ObjectType::HintElementList:
	{
		// ArrayDataにする

		const Statement::HintElementList* pStatement =
			_SYDNEY_DYNAMIC_CAST(const Statement::HintElementList*, &cObject_);
		; _SYDNEY_ASSERT(pStatement);

		if (int n = pStatement->getCount()) {

			ModVector<Common::Data::Pointer> vecData;

			for (int i = 0; i < n; i++) {
				if (Statement::HintElement*
					element = pStatement->getHintElementAt(i)) {

					Common::Data::Pointer
						pData = makeElementData(*element, pCategory_,
												pLowerBound_,
												pUpperBound_,
												bLower_);
					if (pData.get())
						vecData.pushBack(pData);
				}
			}

			if (vecData.getSize() > 1)
				pResult = new Common::DataArrayData(vecData);
			else if (vecData.getSize() == 1) {
				pResult = vecData[0];
			}
		}

		break;
	}
	default:
		; _SYDNEY_ASSERT(false);
		break;
	}

	return pResult;
}

//	FUNCTION private
//	Schema::Hint::destruct -- デストラクターの内部関数
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Hint::
destruct()
{
	m_iCategory = Category::Unknown;
}

//	FUNCTION public
//	Schema::Hint::serialize -- 
//		ヒントを表すクラスのシリアライザー
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&			archiver
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Hint::
serialize(ModArchive& archiver)
{
	if (archiver.isStore()) {

		{
			bool hasData = (m_pElement.get())?1:0;
			archiver << hasData;
			if (hasData) {
				Common::OutputArchive& cOutput =
					dynamic_cast<Common::OutputArchive&>(archiver);
				cOutput.writeObject(m_pElement.get());
			}
		}
		archiver << m_iCategory;

	} else {

		// メンバーをすべて初期化しておく

		destruct();

		{
			bool hasData;
			archiver >> hasData;
			if (hasData) {
				Common::InputArchive& cInput =
					dynamic_cast<Common::InputArchive&>(archiver);
				Common::Data* pData =
					dynamic_cast<Common::Data*>(cInput.readObject());
				if (pData)
					m_pElement = pData;
			}
		}
		archiver >> m_iCategory;
	}
}

//	FUNCTION public
//	Schema::Hint::getClassID -- このクラスのクラス ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

int
Hint::
getClassID() const
{
	return Externalizable::Category::Hint +
		Common::Externalizable::SchemaClasses;
}

//
//	Copyright (c) 2000, 2005, 2006, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
