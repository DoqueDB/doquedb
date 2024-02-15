// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchTermData.cpp --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Utility";
const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Utility/SearchTermData.h"

#include "Common/Assert.h"
#include "Common/ClassID.h"

#include "Exception/BadArgument.h"

#include "ModHasher.h"
#include "ModIos.h"
#include "ModUnicodeOstrStream.h"

_TRMEISTER_USING
_TRMEISTER_UTILITY_USING

namespace
{
	//
	//	VARIABLE local
	//	MatchModeの文字列表現
	//
	ModUnicodeString _MatchMode_Unknown = "Unknown";
	ModUnicodeString _MatchMode_String = "String";
	ModUnicodeString _MatchMode_WordHead = "WordHead";
	ModUnicodeString _MatchMode_WordTail = "WordTail";
	ModUnicodeString _MatchMode_SimpleWord = "SimpleWord";
	ModUnicodeString _MatchMode_ExactWord = "ExactWord";
	ModUnicodeString _MatchMode_MultiLanguage = "MultiLanguage";

	ModUnicodeString*
	_MatchMode_String_Table[SearchTermData::MatchMode::ValueNum] =
	{
		&_MatchMode_Unknown,
		&_MatchMode_String,
		&_MatchMode_WordHead,
		&_MatchMode_WordTail,
		&_MatchMode_SimpleWord,
		&_MatchMode_ExactWord,
		&_MatchMode_MultiLanguage
	};
}

//
//	FUNCTION public
//	Utility::SearchTermData::SearchTermData -- コンストラクタ
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
SearchTermData::SearchTermData()
	: Common::Data(Common::DataType::SearchTerm),
	  m_eMatchMode(MatchMode::Unknown), m_eCharacterType(CharacterType::Unknown)
{
}

//
//	FUNCTION public
//	Utility::SearchTermData::SearchTermData -- コンストラクタ
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
SearchTermData::SearchTermData(const ModUnicodeString& cTerm_)
	: Common::Data(Common::DataType::SearchTerm),
	  m_cTerm(cTerm_),
	  m_eMatchMode(MatchMode::Unknown),
	  m_eCharacterType(CharacterType::Unknown)
{
}

//
//	FUNCTION public
//	Utility::SearchTermData::SearchTermData -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Utility::SearchTermData& cSearchTermData_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SearchTermData::SearchTermData(const SearchTermData& cSearchTermData_)
	: Common::Data(Common::DataType::SearchTerm),
	  m_cTerm(cSearchTermData_.m_cTerm),
	  m_cLanguage(cSearchTermData_.m_cLanguage),
	  m_eMatchMode(cSearchTermData_.m_eMatchMode),
	  m_eCharacterType(cSearchTermData_.m_eCharacterType)
{
}

//
//	FUNCTION public
//	Utility::SearchTermData::~SearchTermData -- デストラクタ
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
SearchTermData::~SearchTermData()
{
}

//
//	FUNCTION public
//	Utility::SearchTermData::getCharacterType --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
SearchTermData::CharacterType::Value
SearchTermData::getCharacterType() const
{
	if (m_eCharacterType == CharacterType::Unknown)
	{
		// m_eCharacterType is mutable value.
		m_eCharacterType = CharacterType::NoCJK;
		
		const ModUnicodeChar* p = m_cTerm;
		while (*p != 0)
		{
			if (ModUnicodeCharTrait::isAlphabet(*p) == ModTrue ||
				ModUnicodeCharTrait::isDigit(*p) == ModTrue ||
				ModUnicodeCharTrait::isSymbol(*p) == ModTrue ||
				ModUnicodeCharTrait::isSpace(*p) == ModTrue)
			{
				++p;
				continue;
			}
			else
			{
				m_eCharacterType = CharacterType::CJK;
				break;
			}
		}
	}
	return m_eCharacterType;
}

//
//	FUNCTION public
//	Utility::SearchTermData::operator= -- 代入演算子
//
//	NOTES
//
//	ARGUMENTS
// 	const Utility::SearchTermData& cSearchTermData_
//		代入元
//
//	RETURN
//	Utility::SearchTermData&
//		代入先
//
//	EXCEPTIONS
//
SearchTermData&
SearchTermData::operator= (const SearchTermData& cSearchTermData_)
{
	m_cTerm = cSearchTermData_.m_cTerm;
	m_cLanguage = cSearchTermData_.m_cLanguage;
	m_eMatchMode = cSearchTermData_.m_eMatchMode;
	m_eCharacterType = cSearchTermData_.m_eCharacterType;
	setNull(cSearchTermData_.isNull());

	return *this;
}

//
//	FUNCTION public
//	Utility::SearchTermData::equals -- 等しいか調べる
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
SearchTermData::equals(const Data* r) const
{
	if (!r)

		// 右辺の値が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

	// 左辺または右辺のいずれかが NULL であれば、
	// NULL より常に他の型の値のほうが大きいとみなす

	return (isNull()) ? r->isNull() :
		(r->isNull() || r->getType() != Common::DataType::SearchTerm) ?
		false : equals_NoCast(*r);
}

// FUNCTION public
//	Utility::SearchTermData::hashCode -- ハッシュコードを取り出す
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
SearchTermData::
hashCode() const
{
	return ModUnicodeStringHasher()(getString());
}

//
//	FUNCTION protected
//	Utility::SearchTermData::serialize_NotNull -- シリアル化
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
SearchTermData::serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	Common::Data::serialize_NotNull(cArchiver_);

	if (cArchiver_.isStore())
	{
		//書出し
		cArchiver_ << m_cTerm;
		cArchiver_ << m_cLanguage;
		int tmp = m_eMatchMode;
		cArchiver_ << tmp;
		tmp = m_eCharacterType;
		cArchiver_ << tmp;
	}
	else
	{
		//読込み
		cArchiver_ >> m_cTerm;
		cArchiver_ >> m_cLanguage;
		int tmp;
		cArchiver_ >> tmp;
		m_eMatchMode = static_cast<MatchMode::Value>(tmp);
		cArchiver_ >> tmp;
		m_eCharacterType = static_cast<CharacterType::Value>(tmp);
	}
}

//
//	FUNCTION private
//	Utility::SearchTermData::copy_NotNull -- 自分自身のコピーを作成する
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

Common::Data::Pointer
SearchTermData::copy_NotNull() const
{
	return new SearchTermData(*this);
}

//
//	FUNCTION private
//	Utility::SearchTermData::getString_NotNull -- 内容を表す文字列を得る
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
SearchTermData::getString_NotNull() const
{
	ModUnicodeOstrStream ostr;
	// This class is for internal use, but this function is used by hashCode(),
	// so it is NOT enough to only print m_cTerm.
	ostr << "'" << m_cTerm << "'"
		 << " language '" << m_cLanguage.getName() << "'"
		 << " matchmode '" << *_MatchMode_String_Table[m_eMatchMode] << "'";

	return ModUnicodeString(ostr.getString());
}

//
//	FUNCTION protected
//	Utility::SearchTermData::equals_NoCast -- 等しいか調べる
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
SearchTermData::equals_NoCast(const Common::Data& r) const
{
	; _TRMEISTER_ASSERT(r.getType() == Common::DataType::SearchTerm);

	const SearchTermData& data = _SYDNEY_DYNAMIC_CAST(const SearchTermData&, r);

	// m_eCharacterType is ignored, because it keeps the internal status.
	if (m_cTerm == data.m_cTerm)
	if (m_cLanguage == data.m_cLanguage)
	if (m_eMatchMode == data.m_eMatchMode)
		return true;

	return false;
}

// 代入を行う(キャストなし)
//virtual
bool
SearchTermData::
assign_NoCast(const Data& r)
{
	; _TRMEISTER_ASSERT(r.getType() == getType());
	; _TRMEISTER_ASSERT(!r.isNull());

	*this = _SYDNEY_DYNAMIC_CAST(const SearchTermData&, r);
	return true;
}

//
//	FUNCTION protected
//	Utility::SearchTermData::getClassID_NotNull -- クラスIDを得る
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
SearchTermData::getClassID_NotNull() const
{
	return Common::ClassID::SearchTermDataClass;
}

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
