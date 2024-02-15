// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedTermElement.h -- 検索語要素の定義
// 
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedTermElement_H__
#define __ModInvertedTermElement_H__

#include "ModUnicodeString.h"
#include "ModInvertedManager.h"
#include "ModInvertedTypes.h"
#include "ModInvertedException.h"

//
// CONST
// ModInvertedTermCategoryCharacterXXX -- 検索語カテゴリの文字表現
//
// NOTES
// 検索語のカテゴリを文字で表す。
//
const char ModInvertedTermCategoryCharacterUndefined	= 'U';
const char ModInvertedTermCategoryCharacterEssential	= 'E';
const char ModInvertedTermCategoryCharacterImportant	= 'I';
const char ModInvertedTermCategoryCharacterHelpful		= 'H';
const char ModInvertedTermCategoryCharacterEssentialRelated	= 'e';
const char ModInvertedTermCategoryCharacterImportantRelated	= 'i';
const char ModInvertedTermCategoryCharacterHelpfulRelated	= 'h';

//
// ENUM
// ModInvertedTermCategory -- 検索語カテゴリ
//
// NOTES
// 検索語のカテゴリを表す。
//
enum ModInvertedTermCategory
{
	ModInvertedTermCategoryUndefined = 0,		// 未定義
	ModInvertedTermCategoryEssential,			// 必須
	ModInvertedTermCategoryImportant,			// 重要
	ModInvertedTermCategoryHelpful,				// 有用
	ModInvertedTermCategoryEssentialRelated,	// 必須関連語
	ModInvertedTermCategoryImportantRelated,	// 重要関連語
	ModInvertedTermCategoryHelpfulRelated,		// 有用関連語
	ModInvertedTermCategoryNumber				// カテゴリ数（最後に置くこと）
};

//
// CLASS
// ModInvertedTermElement -- 検索語要素
//
// NOTES
// 自然文検索における検索語要素を表す。
//
class ModInvertedTermElement
    : public ModInvertedObject
{
public:
	ModInvertedTermElement() :
		term(), category(ModInvertedTermCategoryUndefined), weight(0.0) {}
	ModInvertedTermElement(const ModUnicodeString& term_,
						   const ModInvertedTermCategory category_,
						   const ModInvertedTermScore weight_,
						   const ModSize df_) :
		term(term_), category(category_), weight(weight_), df(df_) {
			// 変なカテゴリが来たら強制的に Undefined にする
			if (category >= ModInvertedTermCategoryNumber) {
				ModErrorMessage << "invalid category: "
								<< static_cast<int>(category) << ModEndl;
				category = ModInvertedTermCategoryUndefined;
			}
	}
	ModInvertedTermElement(const ModUnicodeString& term_,
						   const char category_,
						   const ModInvertedTermScore weight_,
						   const ModSize df_) :
		term(term_), category(ModInvertedTermCategoryUndefined),
		weight(weight_), df(df_) {
			setCategoryByCharacter(category_); }

	void setTerm(const ModUnicodeString& term_) {
		term = term_; }
	void getTerm(ModUnicodeString& term_) const {
		term_ = term; }
	const ModUnicodeString& getTerm() const {
		return term; }

	void setCategory(const ModInvertedTermCategory category_) {
		category = category_; }
	ModInvertedTermCategory getCategory() const {
		return category; }

	void setCategoryByCharacter(const char);
	char getCategoryAsCharacter() const;

	void setWeight(const ModInvertedTermScore weight_) {
		weight = weight_; }
	ModInvertedTermScore getWeight() const {
		return weight; }

	void setDf(const ModSize df_) {
		df = df_; }
	ModSize getDf() const {
		return df; }

	ModBoolean operator==(const ModInvertedTermElement& target_) const {
		return (category == target_.category &&
				weight == target_.weight &&
				term == target_.term &&
				df == target_.df) ?
			ModTrue : ModFalse; }

private:
	ModUnicodeString term;
	ModInvertedTermCategory category;
	ModInvertedTermScore weight;
	ModSize df;
};


// 
// FUNCTION public
// ModInvertedTermElement::setCategoryByCharacter -- 文字でカテゴリを指定する
// 
// NOTES
// 文字でカテゴリを指定する。
// 
// ARGUMENTS
// const char category_
//		カテゴリ
// 
// RETURN
// なし
// 
// EXCEPTIONS
// ModInvertedErrorNotSupported
//		不正なカテゴリ
//
inline void
ModInvertedTermElement::setCategoryByCharacter(const char category_)
{
	switch (category_) {
	case ModInvertedTermCategoryCharacterUndefined:
		category = ModInvertedTermCategoryUndefined;
		return;
	case ModInvertedTermCategoryCharacterEssential:
		category = ModInvertedTermCategoryEssential;
		return;
	case ModInvertedTermCategoryCharacterImportant:
		category = ModInvertedTermCategoryImportant;
		return;
	case ModInvertedTermCategoryCharacterHelpful:
		category = ModInvertedTermCategoryHelpful;
		return;
	case ModInvertedTermCategoryCharacterEssentialRelated:
		category = ModInvertedTermCategoryEssentialRelated;
		return;
	case ModInvertedTermCategoryCharacterImportantRelated:
		category = ModInvertedTermCategoryImportantRelated;
		return;
	case ModInvertedTermCategoryCharacterHelpfulRelated:
		category = ModInvertedTermCategoryHelpfulRelated;
		return;
	default:
		break;
	}

	ModErrorMessage << "Invalid ModInvertedTermCategoryCharacter: "
					<< category_ << ModEndl;
	ModThrowInvertedFileError(ModInvertedErrorNotSupported);
}

// 
// FUNCTION public
// ModInvertedTermElement::getCategoryAsCharacter -- 文字でカテゴリを取得する
// 
// NOTES
// 文字でカテゴリを取得する。
// 
// ARGUMENTS
// なし
// 
// RETURN
// カテゴリの文字表現
// 
// EXCEPTIONS
// なし
//
inline char
ModInvertedTermElement::getCategoryAsCharacter() const
{
	switch (category) {
	case ModInvertedTermCategoryUndefined:
		return ModInvertedTermCategoryCharacterUndefined;
	case ModInvertedTermCategoryEssential:
		return ModInvertedTermCategoryCharacterEssential;
	case ModInvertedTermCategoryImportant:
		return ModInvertedTermCategoryCharacterImportant;
	case ModInvertedTermCategoryHelpful:
		return ModInvertedTermCategoryCharacterHelpful;
	case ModInvertedTermCategoryEssentialRelated:
		return ModInvertedTermCategoryCharacterEssentialRelated;
	case ModInvertedTermCategoryImportantRelated:
		return ModInvertedTermCategoryCharacterImportantRelated;
	case ModInvertedTermCategoryHelpfulRelated:
		return ModInvertedTermCategoryCharacterHelpfulRelated;
	default:
		break;
	}

	// ここに来るはずがない - Assertでもよい
	ModErrorMessage << "Invalid ModInvertedTermCategoryCharacter: "
					<< category << ModEndl;
	ModThrowInvertedFileError(ModInvertedErrorInternal);
	return ModInvertedTermCategoryCharacterUndefined;
}

#endif	// __ModInvertedTermElement_H__

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
