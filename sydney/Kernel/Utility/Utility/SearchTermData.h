// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchTermData -- 
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

#ifndef __TRMEISTER_UTILITY_CHARTRAIT_H
#define __TRMEISTER_UTILITY_CHARTRAIT_H

#include "Utility/Module.h"

#include "Common/Data.h"

#include "ModUnicodeString.h"
#include "ModLanguageSet.h"

_TRMEISTER_BEGIN
_TRMEISTER_UTILITY_BEGIN

//	CLASS
//	Utility::SearchTermData --
//
//	NOTES
//	This class is created by refference to Common::WordData.
//
class SYD_UTILITY_FUNCTION SearchTermData : public Common::Data
{
public:
	//
	//	STRUCT
	//	Utility::SearchTermData::MatchMode
	//
	//	NOTES
	//	See FullText2/Types.h (and FtsInverted/ModInvertedTypes.h) for details.
	//
	struct MatchMode
	{
		enum Value
		{
			Unknown,
			
			String,
			WordHead,
			WordTail,
			SimpleWord,
			ExactWord,
			
			MultiLanguage,

			ValueNum
		};
	};

	//
	//	STRUCT
	//	Utility::SearchTermData::CharacterType
	//
	struct CharacterType
	{
		enum Value
		{
			Unknown,
			NoCJK,
			CJK
		};
	};
	
	// Constructor
	SearchTermData();
	explicit SearchTermData(const ModUnicodeString& cTerm_);
	SearchTermData(const SearchTermData& cSearchTermData_);
	// Destructor
	virtual ~SearchTermData();

	// Term
	const ModUnicodeString& getTerm() const
		{ return m_cTerm; }
	void setTerm(const ModUnicodeString& cTerm_)
	{
		m_cTerm = cTerm_;
		m_eCharacterType = CharacterType::Unknown;
	}

	// Language
	const ModLanguageSet& getLanguage() const
		{ return m_cLanguage; }
	void setLanguage(const ModLanguageSet& cLanguage_)
		{ m_cLanguage = cLanguage_; }

	// Match Mode
	MatchMode::Value getMatchMode() const
		{ return m_eMatchMode; }
	void setMatchMode(MatchMode::Value eMatchMode_)
		{ m_eMatchMode = eMatchMode_; }

	// Character Type
	CharacterType::Value getCharacterType() const;
	// void setCharacterType(CharacterType::Value eCharacterType_);

	// Assignment
	SearchTermData& operator= (const SearchTermData& cSearchTermData_);

	// シリアル化する
//	Common::Data
//	virtual void
//	serialize(ModArchive& archiver);

	// コピーする
//	Common::Data
//	virtual Pointer
//	copy() const;
	// キャストする
//	Common::Data
//	virtual Pointer
//	cast(DataType::Type type) const;
//	virtual Pointer
//	cast(const Data& target) const;

	// 文字列の形式で値を得る
//	Common::Data
//	virtual ModUnicodeString
//	getString() const;

	// 等しいか調べる
	virtual bool
	equals(const Data* r) const;
	// 大小比較を行う
//	Common::Data
//	virtual int
//	compareTo(const Data* r) const;

	// 代入を行う
//	Common::Data
//	virtual bool
//	assign(const Data* r);
	// 四則演算を行う
//	Common::Data
//	virtual bool
//	operateWith(DataOperation::Type op, const Data* r, Pointer& result) const;
	// 単項演算を行う
//	Common::Data
//	virtual bool
//	operateWith(DataOperation::Type op, Pointer& result) const;
//	virtual bool
//	operateWith(DataOperation::Type op);

	// クラスIDを得る
//	Common::Data
//	virtual int
//	getClassID() const;

	//ハッシュコードを取り出す
	virtual ModSize hashCode() const;

protected:
	// シリアル化する(自分自身が NULL 値でない)
	virtual void
	serialize_NotNull(ModArchive& archiver);

	// コピーする(自分自身が NULL 値でない)
	virtual Common::Data::Pointer
	copy_NotNull() const;

	// 文字列の形式で値を得る(自分自身が NULL 値でない)
	virtual ModUnicodeString
	getString_NotNull() const;

	// 等しいか調べる(キャストなし)
	virtual bool
	equals_NoCast(const Data& r) const;

	// 大小比較を行う(キャストなし)
//	Common::Data
//	virtual int
//	compareTo_NoCast(const Data& r) const;

	// 代入を行う(キャストなし)
	virtual bool
	assign_NoCast(const Data& r);

	// 四則演算を行う(キャストなし)
//	Common::Data
//	virtual bool
//	operateWith_NoCast(DataOperation::Type op, const Data& r);

	// クラスIDを得る(自分自身が NULL 値でない)
	virtual int
	getClassID_NotNull() const;
	
private:
	// Term
	ModUnicodeString m_cTerm;
	// Language
	ModLanguageSet m_cLanguage;
	// Match Mode
	MatchMode::Value m_eMatchMode;
	// Character Type
	mutable CharacterType::Value m_eCharacterType;
};

_TRMEISTER_UTILITY_END
_TRMEISTER_END

#endif // __TRMEISTER_UTILITY_CHARTRAIT_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
