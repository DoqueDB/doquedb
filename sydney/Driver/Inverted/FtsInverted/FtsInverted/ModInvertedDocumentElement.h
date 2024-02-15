// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedDocumentElement.h -- 文書要素の定義
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

#ifndef	__ModInvertedDocumentElement_H__
#define __ModInvertedDocumentElement_H__

#include "ModInvertedManager.h"
#include "ModInvertedTypes.h"
#include "ModInvertedException.h"

//
// CONST
// ModInvertedDocumentCategoryCharacterXXX -- 文書カテゴリの文字表現
//
// NOTES
// 文書のカテゴリを文字で表す。
//
const char ModInvertedDocumentCategoryCharacterUndefined		= 'U';
const char ModInvertedDocumentCategoryCharacterRelevant			= 'R';
const char ModInvertedDocumentCategoryCharacterSimilar			= 'S';
const char ModInvertedDocumentCategoryCharacterNonrelevant		= 'N';
const char ModInvertedDocumentCategoryCharacterPsuedoRelevant	= 'P';

//
// ENUM
// ModInvertedDocumentCategory -- 検索語カテゴリ
//
// NOTES
// 検索語のカテゴリを表す。
//
enum ModInvertedDocumentCategory {
	ModInvertedDocumentCategoryUndefined = 0,	// 未定義
	ModInvertedDocumentCategoryRelevant,		// 適合
	ModInvertedDocumentCategorySimilar,			// 類似
	ModInvertedDocumentCategoryNonrelevant,		// 非類似
	ModInvertedDocumentCategoryPsuedoRelevant,	// 疑類似
	ModInvertedDocumentCategoryNumber			// カテゴリ数（最後におくこと）
};

//
// CLASS
// ModInvertedDocumentElement -- 
//
// NOTES
//
class ModInvertedDocumentElement
    : public ModInvertedObject
{
public:
	ModInvertedDocumentElement() :
		id(), category(ModInvertedDocumentCategoryUndefined) {}
	ModInvertedDocumentElement(const ModInvertedDocumentID id_,
							   const ModInvertedDocumentCategory category_) :
		id(id_), category(category_) {}
	ModInvertedDocumentElement(const ModInvertedDocumentID id_,
							   const char category_) :
		id(id_), category(ModInvertedDocumentCategoryUndefined) {
			setCategoryByCharacter(category_); }

	void setDocumentID(const ModInvertedDocumentID id_) {
		id = id_; }
	ModInvertedDocumentID getDocumentID() const {
		return id; }

	void setCategory(const ModInvertedDocumentCategory category_) {
		category = category_; }
	ModInvertedDocumentCategory getCategory() const {
		return category; }

	void setCategoryByCharacter(const char);
	char getCategoryAsCharacter() const;

	ModBoolean operator==(const ModInvertedDocumentElement& target_) const {
		return (category == target_.category && id == target_.id) ?
			ModTrue : ModFalse; }

private:
	ModInvertedDocumentID id;
	ModInvertedDocumentCategory category;
};


// 
// FUNCTION public
// ModInvertedDocumentElement::setCategoryByCharacter -- 文字でカテゴリを指定する
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
ModInvertedDocumentElement::setCategoryByCharacter(const char category_)
{
	switch (category_) {
	case ModInvertedDocumentCategoryCharacterUndefined:
		category = ModInvertedDocumentCategoryUndefined;
		return;
	case ModInvertedDocumentCategoryCharacterRelevant:
		category = ModInvertedDocumentCategoryRelevant;
		return;
	case ModInvertedDocumentCategoryCharacterSimilar:
		category = ModInvertedDocumentCategorySimilar;
		return;
	case ModInvertedDocumentCategoryCharacterNonrelevant:
		category = ModInvertedDocumentCategoryNonrelevant;
		return;
	case ModInvertedDocumentCategoryCharacterPsuedoRelevant:
		category = ModInvertedDocumentCategoryPsuedoRelevant;
		return;
	default:
		break;
	}

	ModErrorMessage << "Invalid ModInvertedDocumentCategoryCharacter: "
					<< category_ << ModEndl;
	ModThrowInvertedFileError(ModInvertedErrorNotSupported);
}

// 
// FUNCTION public
// ModInvertedDocumentElement::getCategoryAsCharacter -- 文字でカテゴリを取得する
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
ModInvertedDocumentElement::getCategoryAsCharacter() const
{
	switch (category) {
	case ModInvertedDocumentCategoryUndefined:
		return ModInvertedDocumentCategoryCharacterUndefined;
	case ModInvertedDocumentCategoryRelevant:
		return ModInvertedDocumentCategoryCharacterRelevant;
	case ModInvertedDocumentCategorySimilar:
		return ModInvertedDocumentCategoryCharacterSimilar;
	case ModInvertedDocumentCategoryNonrelevant:
		return ModInvertedDocumentCategoryCharacterNonrelevant;
	case ModInvertedDocumentCategoryPsuedoRelevant:
		return ModInvertedDocumentCategoryCharacterPsuedoRelevant;
	default:
		break;
	}

	// ここに来るはずがない
	; ModAssert(0);
	return ModInvertedDocumentCategoryCharacterUndefined;
}

#endif	// __ModInvertedDocumentElement_H__

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
