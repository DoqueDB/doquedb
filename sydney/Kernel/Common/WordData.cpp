// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// WordData.cpp -- 全文索引の単語を表すクラス
// 
// Copyright (c) 2004, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Common";
const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Common/WordData.h"
#include "Common/ClassID.h"
#include "Common/Assert.h"

#include "Exception/BadArgument.h"

#include "Os/Limits.h"

#include "ModHasher.h"
#include "ModUnicodeOstrStream.h"
#include "ModIos.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace
{
	//
	//	VARIABLE local
	//	Categoryの文字列表現
	//
	ModUnicodeString _Category_Undefined = "Undefined";
	ModUnicodeString _Category_Essential = "Essential";
	ModUnicodeString _Category_Important = "Important";
	ModUnicodeString _Category_Helpful = "Helpful";
	ModUnicodeString _Category_EssentialRelated = "EssentialRelated";
	ModUnicodeString _Category_ImportantRelated = "ImportantRelated";
	ModUnicodeString _Category_HelpfulRelated = "HelpfulRelated";
	ModUnicodeString _Category_Prohibitive = "Prohibitive";
	ModUnicodeString _Category_ProhibitiveRelated = "ProhibitiveRelated";

	ModUnicodeString* _Category_String[WordData::Category::ValueNum] =
	{
		&_Category_Undefined,
		&_Category_Essential,
		&_Category_Important,
		&_Category_Helpful,
		&_Category_EssentialRelated,
		&_Category_ImportantRelated,
		&_Category_HelpfulRelated,
		&_Category_Prohibitive,
		&_Category_ProhibitiveRelated
	};
	
	const double dMax = Os::Limits<double>::getMax();
	const int iMax = Os::Limits<int>::getMax();
}

//
//	FUNCTION public
//	Common::WordData::WordData -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
WordData::WordData()
	: Data(DataType::Word),
	  m_eCategory(Category::Undefined), m_dblScale(0.0),
	  m_iDocumentFrequency(0)
{
}

//
//	FUNCTION public
//	Common::WordData::WordData -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cTerm_
//		単語
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
WordData::WordData(const ModUnicodeString& cTerm_)
	: Data(DataType::Word),
	  m_cTerm(cTerm_),
	  m_eCategory(Category::Undefined), m_dblScale(0.0),
	  m_iDocumentFrequency(0)
{
}

//
//	FUNCTION public
//	Common::WordData::WordData -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Common::WordData& cWordData_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
WordData::WordData(const WordData& cWordData_)
	: Data(cWordData_),
	  m_cTerm(cWordData_.m_cTerm),
	  m_cLanguage(cWordData_.m_cLanguage),
	  m_eCategory(cWordData_.m_eCategory),
	  m_dblScale(cWordData_.m_dblScale),
	  m_iDocumentFrequency(cWordData_.m_iDocumentFrequency)
{
}

//
//	FUNCTION public
//	Common::WordData::~WordData -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
WordData::~WordData()
{
}

//
//	FUNCTION public
//	Common::WordData::operator= -- 代入演算子
//
//	NOTES
//
//	ARGUMENTS
// 	const Common::WordData& cWordData_
//		代入元
//
//	RETURN
//	Common::WordData&
//		代入先
//
//	EXCEPTIONS
//
WordData&
WordData::operator= (const WordData& cWordData_)
{
	m_cTerm = cWordData_.m_cTerm;
	m_cLanguage = cWordData_.m_cLanguage;
	m_eCategory = cWordData_.m_eCategory;
	m_dblScale = cWordData_.m_dblScale;
	m_iDocumentFrequency = cWordData_.m_iDocumentFrequency;
	setNull(cWordData_.isNull());

	return *this;
}

//
//	FUNCTION public
//	Common::WordData::equals -- 等しいか調べる
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data* r
//		右辺に与えられたデータを格納する領域の先頭アドレス
//
//	RETURN
//	true
//		自分自身と与えられたデータは等しい
//	false
//		自分自身と与えられたデータは等しくない
//
//	EXCEPTIONS
//
bool
WordData::equals(const Data* r) const
{
	if (!r)

		// 右辺の値が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

	// 左辺または右辺のいずれかが NULL であれば、
	// NULL より常に他の型の値のほうが大きいとみなす

	return (isNull()) ? r->isNull() :
		(r->isNull() || r->getType() != DataType::Word) ?
		false : equals_NoCast(*r);
}

//
//	FUNCTION public static
//	Common::WordData::toCategory -- 文字列カテゴリをenumに変換する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& category_
//		文字列カテゴリ
//
//	RETURN
//	Common::WordData::Category::Value
//		カテゴリ
//
//	EXCEPTIONS
//
WordData::Category::Value
WordData::toCategory(const ModUnicodeString& category_)
{
	if (_Category_Essential.compare(category_, ModFalse) == 0)
		return Category::Essential;
	if (_Category_Important.compare(category_, ModFalse) == 0)
		return Category::Important;
	if (_Category_Helpful.compare(category_, ModFalse) == 0)
		return Category::Helpful;
	if (_Category_EssentialRelated.compare(category_, ModFalse) == 0)
		return Category::EssentialRelated;
	if (_Category_ImportantRelated.compare(category_, ModFalse) == 0)
		return Category::ImportantRelated;
	if (_Category_HelpfulRelated.compare(category_, ModFalse) == 0)
		return Category::HelpfulRelated;
	if (_Category_Prohibitive.compare(category_, ModFalse) == 0)
		return Category::Prohibitive;
	if (_Category_ProhibitiveRelated.compare(category_, ModFalse) == 0)
		return Category::ProhibitiveRelated;
	return Category::Undefined;
}

// FUNCTION public
//	Common::WordData::hashCode -- ハッシュコードを取り出す
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
WordData::
hashCode() const
{
	if (isNull()) return 0;

	return ModUnicodeStringHasher()(getString());
}

//
//	FUNCTION protected
//	Common::WordData::serialize_NotNull -- シリアル化
//
//	NOTES
//
//	ARGUMENTS
//	ModArchive& cArchiver_
//		アーカイバ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
WordData::serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	Data::serialize_NotNull(cArchiver_);

	if (cArchiver_.isStore())
	{
		//書出し
		cArchiver_ << m_cTerm;
		cArchiver_ << m_cLanguage;
		int tmp = m_eCategory;
		cArchiver_ << tmp;
		cArchiver_ << m_dblScale;
		cArchiver_ << m_iDocumentFrequency;
	}
	else
	{
		//読込み
		cArchiver_ >> m_cTerm;
		cArchiver_ >> m_cLanguage;
		int tmp;
		cArchiver_ >> tmp;
		m_eCategory = static_cast<Category::Value>(tmp);
		cArchiver_ >> m_dblScale;
		cArchiver_ >> m_iDocumentFrequency;
	}
}

//
//	FUNCTION private
//	Common::WordData::copy_NotNull -- 自分自身のコピーを作成する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Data*
//		自分自身のコピー
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//

Data::Pointer
WordData::copy_NotNull() const
{
	return new WordData(*this);
}

//
//	FUNCTION private
//	Common::WordData::getString_NotNull -- 内容を表す文字列を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//
//	EXCEPTIONS

ModUnicodeString
WordData::getString_NotNull() const
{
	ModUnicodeOstrStream ostr;
	if (m_eCategory != Category::Undefined)
	{
		// word(xxx) 用
		ostr << "'" << m_cTerm << "'"
			 << " language '" << m_cLanguage.getName() << "'"
			 << " category '" << *_Category_String[m_eCategory] << "'"
			 << " scale " << ModIosSetPrecision(2) << m_dblScale
			 << " df " << m_iDocumentFrequency;
	}
	else
	{
		// cluster(xxx).keryword 用
		ostr << "'" << m_cTerm << "'"
			 << " scale " << ModIosSetPrecision(2) << m_dblScale;
	}
	return ModUnicodeString(ostr.getString());
}

//
//	FUNCTION protected
//	Common::WordData::equals_NoCast -- 等しいか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Common::Data&	r
//			右辺に与えられたデータ
//
//	RETURN
//		true
//			自分自身と与えられたデータは等しい
//		false
//			自分自身と与えられたデータは等しくない
//
//	EXCEPTIONS
//
bool
WordData::equals_NoCast(const Common::Data& r) const
{
	; _TRMEISTER_ASSERT(r.getType() == DataType::Word);

	const WordData& data = _SYDNEY_DYNAMIC_CAST(const WordData&, r);

	if (m_cTerm == data.m_cTerm)
	if (m_cLanguage == data.m_cLanguage)
	if (m_eCategory == data.m_eCategory)
	if (m_dblScale == data.m_dblScale)
	if (m_iDocumentFrequency == data.m_iDocumentFrequency)
		return true;

	return false;
}


//	FUNCTION private
//	Common::WordData::compareTo_NoCast -- 大小比較を行う
//
//	NOTES
//
//	ARGUMENTS
//		Common::Data&	r
//			右辺に与えられたデータ
//
//	RETURN
//		値を返すことはない
//
//	EXCEPTIONS
//		Exception::NotSupported
//			この型のデータの大小比較は提供されていない

int
WordData::compareTo_NoCast(const Data& r) const
{
#ifdef OBSOLETE // castの結果Nullになることがある場合生かす
	if (isNull())
		return NullData::getInstance()->compareTo(r);
#endif

	; _TRMEISTER_ASSERT(r.getType() == DataType::Word);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const WordData& data = _SYDNEY_DYNAMIC_CAST(const WordData&, r);
	
	if (m_cLanguage != data.m_cLanguage)
		return ModLanguageSet::compare(m_cLanguage, data.m_cLanguage);
	
	return m_cTerm.compare(data.m_cTerm);
}


// 代入を行う(キャストなし)
//virtual
bool
WordData::
assign_NoCast(const Data& r)
{
	; _TRMEISTER_ASSERT(r.getType() == getType());
	; _TRMEISTER_ASSERT(!r.isNull());

	*this = _SYDNEY_DYNAMIC_CAST(const WordData&, r);
	return true;
}


// 代入を行う(キャストなし)
//virtual
bool
WordData::operateWith_NoCast(
	DataOperation::Type op, const Data& r)
{
	; _TRMEISTER_ASSERT(r.getType() == DataType::Word);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());
	
	const WordData& data = _SYDNEY_DYNAMIC_CAST(const WordData&, r);
	switch (op) {
	case DataOperation::Addition:
		if (!((m_iDocumentFrequency > 0 &&
			  data.m_iDocumentFrequency > 0 &&
			  data.m_iDocumentFrequency > iMax-m_iDocumentFrequency)
			  ||(m_dblScale > 0 && data.m_dblScale > 0 &&
				 data.m_dblScale > dMax-m_dblScale))) {
			m_iDocumentFrequency += data.m_iDocumentFrequency;
			m_dblScale += data.m_dblScale;
			return true;
		}
		break;
	default:
		_TRMEISTER_THROW0(Exception::BadArgument);			
	}
	return false;
}


//
//	FUNCTION protected
//	Common::WordData::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		クラスID
//
//	EXCEPTIONS
//	なし
//
int
WordData::getClassID_NotNull() const
{
	return ClassID::WordDataClass;
}

//
//	Copyright (c) 2004, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
