// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModLanguageSet.cpp -- 複数の言語タグ(以下言語セット)を扱うクラス
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


#include <stdarg.h>
#include "ModLanguageSet.h"

#include "ModOsDriver.h"
#include "ModError.h"
#include "ModUnicodeString.h"
#include "ModException.h"

#define	TAG_SEPARATOR				'+'
#define LANGUAGE_COUNTRY_SEPARATOR	'-'

//	FUNCTION public
//	ModLanguageSet::ModLanguageSet -- 
//		言語セットを扱うクラスのデフォルトコンストラクタ
//
//	NOTES
//	「言語無し(0)」状態となります。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//		
//	EXCEPTIONS
//	なし

ModLanguageSet::ModLanguageSet()
{
	this->clear();
}

//	FUNCTION public
//	ModLanguageSet::ModLanguageSet -- 
//		言語コードが指定された場合のコンストラクタ
//
//	NOTES
//	「言語不定(ModLanguage::undefined)」を用いたコンストラクトは
//	エラーとなります。
//
//	ARGUMENTS
//	ModLanguageCode	langCode_
//		言語コード
//
//	RETURN
//	なし
//		
//	EXCEPTIONS
//	ModCommonErrorOutOfRange
//		不正な言語コードが指定されている

ModLanguageSet::ModLanguageSet(ModLanguageCode	langCode_)
{
	this->clear();
	try {
		this->add(langCode_);
	} catch (ModException&	e) {
		this->clear();
		ModRethrow(e);
	}
}

//	FUNCTION public
//	ModLanguageSet::ModLanguageSet -- 
//		文字列で言語セットが指定された場合のコンストラクタ
//
//	NOTES
//	ModLanguageSet::operator=(const ModUnicodeString&) のコメントを
//	参照して下さい。
//
//	ARGUMENTS
//	const ModUnicodeString&	langSetSymbol_
//		言語セットを指定する文字列
//
//	RETURN
//	なし
//		
//	EXCEPTIONS
//	ModCommonErrorBadArgument
//		言語セットを指定する文字列中に不正な言語名や国・地域名があった

ModLanguageSet::ModLanguageSet(const ModUnicodeString&	langSetSymbol_)
{
	try {
		*this = langSetSymbol_;
	} catch (ModException&	e) {
		this->clear();
		ModRethrow(e);
	}
}

//	FUNCTION public
//	ModLanguageSet::ModLanguageSet --
//		言語タグが指定された場合のコンストラクタ
//
//	NOTES
//	「言語不定 (ModLanguage::undefined) 」を用いたコンストラクトは
//	エラーとなります。
//
//	ARGUMENTS
//	ModLanguageTag	langTag_
//		言語タグ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	ModCommonErrorOutOfRange
//		不正な言語コードが指定されている

ModLanguageSet::ModLanguageSet(ModLanguageTag	langTag_)
{
	this->clear();
	try {
		this->add(langTag_);
	} catch (ModException&	e) {
		this->clear();
		ModRethrow(e);
	}
}

//	FUNCTION public
//	ModLanguageSet::operator = -- = 演算子(言語コードのみの指定)
//
//	NOTES
//	「言語不定 (ModLanguage::undefined) 」の代入はエラーとなります。
//
//	ARGUMENTS
//	ModLanguageCode	langCode_
//		自分自身に代入する言語コード
//
//	RETURN
//	ModLanguageSet&
//		代入後の自分自身への参照
//
//	EXCEPTIONS
//	ModCommonErrorOutOfRange
//		不正な言語コードが指定されている

ModLanguageSet& 
ModLanguageSet::operator=(ModLanguageCode	langCode_)
{
	ModLanguageSet tmp;
	tmp.add(langCode_);
	return (*this = tmp);
}

//	FUNCTION public
//	ModLanguageSet::operator = -- = 演算子(文字列による言語セット指定)
//
//	NOTES
//	ひとつの言語タグは "zh-tw" のように言語と国・地域を '-' で連結して
//	指定します。
//	国・地域が未指定で言語のみが指定されたものに関しては、
//	国・地域コードとして ModCountry::undefined が設定されます。
//	言語タグ同士は "es+ja+en" や "zh-cn+en" のように '+' で連結して指定します。
//	空文字列は「言語無し(0)」の指定として扱います。
//
//	ARGUMENTS
//	const ModUnicodeString&	langSetSymbol_
//		自分自身に代入する言語セットを記述した文字列
//
//	RETURN
//	ModLanguageSet&
//		代入後の自分自身への参照
//
//	EXCEPTIONS
//	ModCommonErrorBadArgument
//		言語セットを指定する文字列中に不正な言語名や国・地域名があった

ModLanguageSet& 
ModLanguageSet::operator=(const ModUnicodeString&	langSetSymbol_)
{
	if (langSetSymbol_.getLength() == 0) {
		// 自明の戻り値 
		this->clear();
		return *this;
	}

	ModLanguageSet	tmpLangSet;
	ModLanguageCode	languageCode = ModLanguage::undefined;
	ModCountryCode	countryCode = ModCountry::undefined;
	ModBoolean		setCountryCode = ModFalse;
	ModSize	symbolLen = langSetSymbol_.getLength();
	ModSize	st = 0;
	ModSize	i;
	for (i = 0; i <= symbolLen; i++) {

		if (i == symbolLen || langSetSymbol_[i] == TAG_SEPARATOR) {

			if (setCountryCode == ModTrue) {

				const ModUnicodeChar*	countrySymbol =
					static_cast<const ModUnicodeChar*>(langSetSymbol_) + st;
				countryCode = ModCountry::toCode(countrySymbol, i - st);

				if (ModCountry::isValid(countryCode) != ModTrue) {

					// 不正な国・地域名
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument,
							 ModErrorLevelError);
				}

			} else {

				const ModUnicodeChar*	langSymbol =
					static_cast<const ModUnicodeChar*>(langSetSymbol_) + st;
				languageCode = ModLanguage::toCode(langSymbol, i - st);

				if (ModLanguage::isValid(languageCode) != ModTrue) {

					// 不正な言語名
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument,
							 ModErrorLevelError);
				}
			}

			ModLanguageTag	langTag(languageCode, countryCode);
			tmpLangSet.add(langTag);

			languageCode = ModLanguage::undefined;
			countryCode = ModCountry::undefined;
			setCountryCode = ModFalse;

			st = i + 1;

		} else if (langSetSymbol_[i] == LANGUAGE_COUNTRY_SEPARATOR) {

			setCountryCode = ModTrue;

			const ModUnicodeChar*	langSymbol =
				static_cast<const ModUnicodeChar*>(langSetSymbol_) + st;
			languageCode = ModLanguage::toCode(langSymbol, i - st);

			if (ModLanguage::isValid(languageCode) != ModTrue) {

				// 不正な言語名
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument,
						 ModErrorLevelError);
			}
			st = i + 1;
		}
	}

	return (*this = tmpLangSet);
}

//	FUNCTION public
//	ModLanguage::operator = -- = 演算子(言語タグ指定)
//
//	NOTES
//	「言語不定 (ModLanguage::undefined) 」の代入はエラーとなります。
//
//	ARGUMENTS
//	const ModLanguageTag&	langTag_
//		このオブジェクトに代入する言語タグ。
//
//	RETURN
//	ModLanguageSet&
//		このオブジェクトへの参照。
//
//	EXCEPTIONS
//	ModCommonErrorOutOfRange
//		不正な言語コードが指定されている

ModLanguageSet&
ModLanguageSet::operator=(const ModLanguageTag&	langTag_)
{
	ModLanguageSet tmp;
	tmp.add(langTag_);
	return (*this = tmp);
}

//	FUNCTION public
//	ModLanguage::operator = -- = 演算子(言語セット指定)
//
//	NOTES
//
//	ARGUMENTS
//	const ModLanguageSet&	langSet_
//		このオブジェクトに代入する言語セット。
//
//	RETURN
//	ModLanguageSet&
//		このオブジェクトへの参照。
//
//	EXCEPTIONS
//	ModCommonErrorOutOfRange
//		不正な言語コードが指定されている

ModLanguageSet&
ModLanguageSet::operator=(const ModLanguageSet&	langSet_)
{
	this->_v = langSet_._v;
	return *this;
}

//	FUNCTION public
//	ModLanguageSet::operator == -- == 演算子
//
//	NOTES
//
//	ARGUMENTS
//	ModLanguageSet	langSet_
//		比較対象の言語セット
//
//	RETURN
//	ModBoolean
//		ModTrue  : 同じだった
//		ModFalse : 違った
//
//	EXCEPTIONS
//	なし

ModBoolean 
ModLanguageSet::operator==(const ModLanguageSet& langSet_) const
{
	return
		(ModLanguageSet::compare(*this, langSet_) == 0) ? ModTrue : ModFalse;
}

//	FUNCTION public
//	ModLanguageSet::operator != -- != 演算子
//
//	NOTES
//
//	ARGUMENTS
//	ModLanguageSet	langSet_
//		比較対象の言語セット
//
//	RETURN
//	ModBoolean
//		ModTrue  : 違った
//		ModFalse : 同じだった
//
//	EXCEPTIONS
//	なし

ModBoolean 
ModLanguageSet::operator!=(const ModLanguageSet&	langSet_) const
{
	return
		(ModLanguageSet::compare(*this, langSet_) != 0) ? ModTrue : ModFalse;
}

//	FUNCTION public
//	ModLanguageSet::operator < -- < 演算子
//
//	NOTES
//
//	ARGUMENTS
//	ModLanguageSet	langSet_
//		比較対象の言語セット
//
//	RETURN
//	ModBoolean
//		ModTrue  : 比較対象よりも小さかった
//		ModFalse : 比較対象以上だった
//
//	EXCEPTIONS
//	なし

ModBoolean 
ModLanguageSet::operator<(const ModLanguageSet&	langSet_) const
{
	return
		(ModLanguageSet::compare(*this, langSet_) < 0) ? ModTrue : ModFalse;
}

//	FUNCTION public
//	ModLanguageSet::operator <= -- <= 演算子
//
//	NOTES
//
//	ARGUMENTS
//	ModLanguageSet	langSet_
//		比較対象の言語セット
//
//	RETURN
//	ModBoolean
//		ModTrue  : 比較対象以下だった
//		ModFalse : 比較対象より大きかった
//
// EXCEPTIONS
// なし

ModBoolean 
ModLanguageSet::operator<=(const ModLanguageSet&	langSet_) const
{
	return
		(ModLanguageSet::compare(*this, langSet_) <= 0) ? ModTrue : ModFalse;
}

//	FUNCTION public
//	ModLanguageSet::operator > -- > 演算子
//
//	NOTES
//
//	ARGUMENTS
//	ModLanguageSet	langSet_
//		比較対象の言語セット
//
//	RETURN
//	ModBoolean
//		ModTrue  : 比較対象よりも大きかった
//		ModFalse : 比較対象以下だった
//
//	EXCEPTIONS
//	なし

ModBoolean 
ModLanguageSet::operator>(const ModLanguageSet&	langSet_) const
{
	return
		(ModLanguageSet::compare(*this, langSet_) > 0) ? ModTrue : ModFalse;
}

//	FUNCTION public
//	ModLanguageSet::operator >= -- >= 演算子
//
//	NOTES
//
//	ARGUMENTS
//	ModLanguageSet	langSet_
//		比較対象の言語セット
//
//	RETURN
//	ModBoolean
//		ModTrue  : 比較対象以上だった
//		ModFalse : 比較対象より小さかった
//
//	EXCEPTIONS
//	なし

ModBoolean 
ModLanguageSet::operator>=(const ModLanguageSet&	langSet_) const
{
	return
		(ModLanguageSet::compare(*this, langSet_) >= 0) ? ModTrue : ModFalse;
}

//	FUNCTION public
//	ModLanguageSet::clear -- 言語セットのクリアします
//
//	NOTES
//	「言語無し(0)」の状態となります。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void 
ModLanguageSet::clear()
{
	this->_v.clear();
}

//	FUNCTION public
//	ModLanguageSet::add -- 指定言語を追加する
//
//	NOTES
//	「言語不定(undefined)」は追加できません。
//	この関数では、言語のみが指定されるため、言語タグのうちの
//	国・地域コードは ModCountry::undefined となります。
//	追加される言語タグはユニークです。
//	（言語単位でユニークではありません。）
//	例えば、このオブジェクトに既に "zh-tw" が追加されている状態で、
//	この関数に "zh" が渡された場合、
//	既存の "zh-tw" とは別に "zh" が追加されます。
//	（関数 ModLanguageSet::getName() で得られる文字列で表現すると、
//	"zh+zh-tw" となります。）
//	しかし、このオブジェクトに既に "zh" が追加されている状態で、
//	この関数に対して "zh" が渡された場合、
//	新たに "zh" が追加されることはありません。
//	（関数 ModLanguageSet::getName() で得られる文字列で表現すると、
//	"zh" となります。）
//
//	ARGUMENTS
//	ModLanguageCode langCode_
//		追加する言語
//
//	RETURN
//	なし
//		
//	EXCEPTIONS
//	ModCommonErrorOutOfRange
//		不正な言語コードが指定されている

void 
ModLanguageSet::add(ModLanguageCode langCode_)
{
	ModLanguageTag	tag(langCode_, ModCountry::undefined);
	this->add(tag);
}

//	FUNCTION public
//	ModLanguageSet::add -- 指定言語タグを追加する
//
//	NOTES
//	「国・地域不定(ModCountry::undefined)」は追加できますが、
//	「言語不定(ModLanguage::undefined)」は追加できません。
//	追加される言語タグはユニークです。
//	例えば、このオブジェクトに既に "zh-tw" が追加されている状態で、
//	この関数に "zh-cn" が渡された場合、
//	既存の "zh-tw" とは別に "zh-cn" が追加されます。
//	（ 関数 ModLanguageSet::getName() で得られる文字列で表現すると、
//	"zh-cn+zh-tw" となります。）
//	しかし、このオブジェクトに既に "zh-tw" が追加されている状態で、
//	この関数に "zh-tw" が渡された場合、
//	新たに "zh-tw" が追加されることはありません。
//	（関数 ModLanguageSet::getName() で得られる文字列で表現すると、
//	"zh-tw" となります。）
//
//	ARGUMENTS
//	const ModLanguageTag&	langTag_
//		追加する言語タグ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	ModCommonErrorOutOfRange
//		不正な言語コードが指定されている

void
ModLanguageSet::add(const ModLanguageTag&	langTag_)
{
	if (ModLanguage::isValid(langTag_.first) != ModTrue) {
		ModThrow(ModModuleStandard,
				 ModCommonErrorOutOfRange,
				 ModErrorLevelError);
	}

	// ※ ベクターには、言語および国・地域コード順にソートされているように
	// 　 追加する。

	ModVector<ModLanguageTag>::Iterator	tag = this->_v.begin();
	ModVector<ModLanguageTag>::Iterator	endOfTag = this->_v.end();
	while (tag != endOfTag) {

		if ((*tag).first == langTag_.first) {

			while (tag != endOfTag && (*tag).first == langTag_.first) {

				// 言語および国・地域コードが等しい言語タグが
				// 存在するのであれば、追加する必要はない。
				if ((*tag).second == langTag_.second) return;

				if (langTag_.second < (*tag).second) break;

				tag++;
			}

			// 途中（または最後）に追加する。
			this->_v.insert(tag, langTag_);

			return;

		} else if (langTag_.first < (*tag).first) {

			// 途中に追加する。
			this->_v.insert(tag, langTag_);
			return;
		}

		tag++;
	}

	// 最後に追加する。
	this->_v.pushBack(langTag_);
}

//	FUNCTION public
//	ModLanguageSet::isCotained --
//		指定言語が含まれているかどうかを調べます
//
//	NOTES
//	「言語不定(undefined)」ではチェックできないのでエラーとなります。
//	国・地域は無視します。
//
//	ARGUMENTS
//	ModLanguageCode	langCode_
//		含まれているかどうかを調べる対象言語
//
//	RETURN
//	ModBoolean
//		ModTrue  : 含まれる
//		ModFalse : 含まれない
//
//	EXCEPTIONS
//	ModCommonErrorOutOfRange
//		不正な言語コードが指定されている

ModBoolean
ModLanguageSet::isContained(ModLanguageCode	langCode_) const
{
	if (ModLanguage::isValid(langCode_) != ModTrue) { 
		ModThrow(ModModuleStandard,
				 ModCommonErrorOutOfRange,
				 ModErrorLevelError);
	}

	ModVector<ModLanguageTag>::ConstIterator	tag = this->_v.begin();
	ModVector<ModLanguageTag>::ConstIterator	endOfTag = this->_v.end();
	while (tag != endOfTag) {

		if ((*tag).first == langCode_) return ModTrue;

		tag++;
	}

	return ModFalse;
}

//	FUNCTION public
//	ModLanguageSet::isContained --
//		指定言語タグが含まれているかどうかを調べます
//
//	NOTES
//
//	ARGUMENTS
//	const ModLanguageTag&	langTag_
//		含まれているかどうかを調べる対象言語タグ
//
//	RETURN
//	ModBoolean
//		ModTrue  : 含まれる
//		ModFalse : 含まれない
//
//	EXCEPTIONS
//	ModCommonErrorOutOfRange
//		不正な言語コードが指定されている

ModBoolean
ModLanguageSet::isContained(const ModLanguageTag&	langTag_) const
{
	if (ModLanguage::isValid(langTag_.first) != ModTrue) {
		ModThrow(ModModuleStandard,
				 ModCommonErrorOutOfRange,
				 ModErrorLevelError);
	}
	if (langTag_.second != ModCountry::undefined &&
		ModCountry::isValid(langTag_.second) != ModTrue) {
		ModThrow(ModModuleStandard,
				 ModCommonErrorOutOfRange,
				 ModErrorLevelError);
	}

	ModVector<ModLanguageTag>::ConstIterator	tag = this->_v.begin();
	ModVector<ModLanguageTag>::ConstIterator	endOfTag = this->_v.end();
	while (tag != endOfTag) {

		if ((*tag).first == langTag_.first && (*tag).second == langTag_.second)
			return ModTrue;

		tag++;
	}

	return ModFalse;
}

//	FUNCTION public
//	ModLanguageSet::round --
//		言語タグから国・地域コードを除いた言語セットを取得します
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModLanguageSet
//		このオブジェクトの言語タグから国・地域コードを除いた言語セット
//
//	EXCEPTIONS
//	なし

ModLanguageSet
ModLanguageSet::round() const
{
	ModLanguageSet	noCountrySet;

	ModVector<ModLanguageTag>::ConstIterator	tag = this->_v.begin();
	ModVector<ModLanguageTag>::ConstIterator	endOfTag = this->_v.end();
	while (tag != endOfTag) {

		ModLanguageTag	noCountryTag((*tag).first, ModCountry::undefined);
		noCountrySet.add(noCountryTag);

		tag++;
	}

	return noCountrySet;
}

//	FUNCTION public
//	ModLanguageSet::getSize -- 言語タグ数を取得します
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		ベクターに積まれている言語タグ数
//
//	EXCEPTIONS
//	なし

ModSize
ModLanguageSet::getSize() const
{
	return this->_v.getSize();
}

//	FUNCTION public
//	ModLanguageSet::getName -- 言語セットの文字列表現を取得します
//
//	NOTES
//	「言語無し(0)」の場合、空文字列(言語無しの表現)を返します。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		言語セットに含まれる言語を '+' で連結して返す
//
//	EXCEPTIONS
//	なし

ModUnicodeString 
ModLanguageSet::getName() const
{
	ModUnicodeString	langSetSymbol;

	ModVector<ModLanguageTag>::ConstIterator	tag = this->_v.begin();
	ModSize	numberOfLangTags = this->_v.getSize();
	ModSize	i;
	for (i = 0; i < numberOfLangTags; i++, tag++) {

		langSetSymbol += ModLanguage::toSymbol((*tag).first);

		if ((*tag).second != ModCountry::undefined) {

			langSetSymbol += LANGUAGE_COUNTRY_SEPARATOR;

			langSetSymbol += ModCountry::toSymbol((*tag).second);
		}

		if (i < numberOfLangTags - 1) langSetSymbol += TAG_SEPARATOR;
	}

	return langSetSymbol;
}

//	FUNCTION public
//	ModLanguageSet::serialize -- 言語セットを単位とするシリアライザー
//
//	NOTES
//	言語タグ数は ModUInt32 、言語コードと国・地域コードは
//	unsigned short で読み書きします。
//
//	ARGUMENTS
//	ModArchive&	archiver_
//		シリアル化先(または元)のアーカイバー
//
//	RETURN
//	なし
//		
//	EXCEPTIONS
//	ModCommonErrorNotSupported
//		アーカイブ中の言語コードまたは国・地域コードが認知しないコードである、
//		など

void
ModLanguageSet::serialize(ModArchive&	archiver_)
{
	ModUInt32		numberOfLangTags, i;
	unsigned short	langCode, countryCode;

	if (archiver_.isStore() == ModTrue) {

		// 書く…

		// 言語タグ数
		numberOfLangTags = this->_v.getSize();
		archiver_ << numberOfLangTags;

		ModVector<ModLanguageTag>::ConstIterator	tag = this->_v.begin();
		for (i = 0; i < numberOfLangTags; i++, tag++) {

			// 言語コード
			langCode = (unsigned short)((*tag).first);
			archiver_ << langCode;

			// 国・地域コード
			countryCode = (unsigned short)((*tag).second);
			archiver_ << countryCode;
		}

	} else {

		// 読む…

		this->clear();

		// 言語タグ数
		archiver_ >> numberOfLangTags;

		for (i = 0; i < numberOfLangTags; i++) {

			// 言語コード
			archiver_ >> langCode;

			// 国・地域コード
			archiver_ >> countryCode;

			if (ModLanguage::isValid((ModLanguageCode)langCode) != ModTrue ||
				(countryCode != ModCountry::undefined &&
				 ModCountry::isValid((ModCountryCode)countryCode) != ModTrue)){
				ModThrow(ModModuleStandard,
						 ModCommonErrorNotSupported,
						 ModErrorLevelError);
			}

			ModLanguageTag	langTag((ModLanguageCode)langCode,
									(ModCountryCode)countryCode);
			this->add(langTag);
		}
	}
}

//	FUNCTION public
//	ModLanguageSet::compare -- 言語セット同士を比較します
//
//	NOTES
//
//	ARGUMENTS
//	const ModLanguageSet&	src_
//		比較元言語セット
//	const ModLanguageSet&	dst_
//		比較先言語セット
//
//	RETURN
//	int
//		 0 : 比較元と比較先が等しい
//		+1 : 比較元の方が大きい
//		-1 : 比較元の方が小さい
//
//	EXCEPTIONS
//	なし

// static
int
ModLanguageSet::compare(const ModLanguageSet&	src_,
						const ModLanguageSet&	dst_)
{
	ModBoolean	srcIsEmpty = src_._v.isEmpty();
	ModBoolean	dstIsEmpty = dst_._v.isEmpty();

	if (srcIsEmpty == ModTrue || dstIsEmpty == ModTrue) {

		// どちらか片方、または両方が言語なし…

		if (srcIsEmpty == dstIsEmpty) return 0; // 両方とも言語なし

		return (srcIsEmpty == ModTrue) ? -1 : +1; // 片方のみ言語なし
	}

	// 両方言語あり…

	ModVector<ModLanguageTag>::ConstIterator	src = src_._v.begin();
	ModVector<ModLanguageTag>::ConstIterator	dst = dst_._v.begin();
	ModSize	srcSize = src_.getSize() * sizeof(ModLanguageTag);
	ModSize	dstSize = dst_.getSize() * sizeof(ModLanguageTag);
	ModSize	compSize = (srcSize <= dstSize) ? srcSize : dstSize;
	int	compResult = ModOsDriver::Memory::compare(&(*src), &(*dst), compSize);
	if (compResult == 0) {
		if (srcSize != dstSize) {
			compResult = (srcSize < dstSize) ? -1 : +1;
		}
	} else {
		compResult = (compResult < 0) ? -1 : +1;
	}
	return compResult;
}

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
