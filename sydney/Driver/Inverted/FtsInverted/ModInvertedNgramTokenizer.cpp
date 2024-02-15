// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedNgramTokenizer.cpp -- N-gram 分割器の実装
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifdef SYD_INVERTED // SYDNEY 対応
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#endif

#include "ModAlgorithm.h"
#include "ModOstrStream.h"
#include "ModString.h"
#include "ModUnicodeString.h"
#ifdef SYD_USE_UNA_V10
#include "ModNLP.h"
#else
#include "ModNormalizer.h"
#endif
#include "ModAutoPointer.h"

#include "ModInvertedException.h"
#ifdef  SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedFile.h"
#else
#include "ModInvertedFile.h"
#endif
#include "ModInvertedNgramTokenizer.h"

//
// CONST
// ModInvertedNgramTokenizer::tokenizerName -- 分割器の名称
//
// NOTES
// 分割器の名称を表す
//
/*static*/
const char ModInvertedNgramTokenizer::tokenizerName[] = "NGR";

#ifdef SYD_USE_UNA_V10
namespace {
	ModUnicodeString _DoNorm = "donorm";
	ModUnicodeString _True = "true";
	ModUnicodeString _False = "false";

	ModLanguageSet _defaultLanguageSet("ja+en");
}
#endif

//
// FUNCTION
// ModInvertedNgramTokenizer::clear -- クリア
//
// NOTES
// new された分割対象があれば delete する。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedNgramTokenizer::clear()
{
	if (isAllocated == ModTrue) {
		delete target, target = 0;
		isAllocated = ModFalse;
	}
}

//
// FUNCTION
// ModInvertedNgramTokenizer::setCharOffsets -- 文字位置の配列への変換
//
// NOTES
// バイト位置の配列から文字位置の配列への変換を行う。
//
// ARGUMENTS
// const ModVector<ModSize>* byteOffsets_
//		バイト位置の配列
// ModVector<ModSize>* characterOffsets_
//		文字位置をセットする配列
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void 
ModInvertedNgramTokenizer::setCharOffsets(const ModVector<ModSize>* byteOffsets_,
										  ModVector<ModSize>* charOffsets_)
{
	// バイトオフセット〜文字オフセットの変換を行う
	// UnicodeString では１文字２バイトに固定である

	charOffsets_->clear();
	for (ModVector<ModSize>::ConstIterator b(byteOffsets_->begin());
		 b != byteOffsets_->end(); ++b) {
		charOffsets_->pushBack(*b/2);
	}
	; ModAssert(byteOffsets_->getSize() == charOffsets_->getSize());
}

//
// FUNCTION
// ModInvertedNgramTokenizer::prepareNormalizedText -- 正規化テキストの用意
//
// NOTES
// 正規化テキストの用意する。
// 必要であれば、バイト位置の配列から文字位置の配列への変換も行う。
//
// ARGUMENTS
// const ModUnicodeString& orignal_
//		もとテキスト
// const ModVector<ModSize>* byteOffsets_
//		バイト位置の配列
// ModVector<ModSize>* characterOffsets_
//		文字位置をセットする配列
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedNgramTokenizer::prepareNormalizedText(
	const ModUnicodeString& original_,
	const ModVector<ModSize>* byteOffsets_,
	ModVector<ModSize>* charOffsets_)
{
	target = getNormalizedText(original_,
#ifdef V1_6
							   0, // 言語指定は意味を持たない
#endif
							   byteOffsets_, charOffsets_);
	isAllocated = ModTrue;
}

//
// FUNCTION
// ModInvertedNgramTokenizer::getNormalizedText -- 正規化テキストの用意
//
// NOTES
// 正規化テキストの用意する。
// 必要であれば、バイト位置の配列から文字位置の配列への変換も行う。
//
// ARGUMENTS
// const ModUnicodeString& orignal_
//		もとテキスト
// ModVector<ModLanguageSet>* langSet_
//		言語指定（v1_6のみ）
// const ModVector<ModSize>* byteOffsets_
//		バイト位置の配列
// ModVector<ModSize>* characterOffsets_
//		文字位置をセットする配列
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModUnicodeString*
ModInvertedNgramTokenizer::getNormalizedText(
	const ModUnicodeString& original_,
#ifdef V1_6
	ModVector<ModLanguageSet>* langSet_,
#endif
	const ModVector<ModSize>* byteOffsets_,
	ModVector<ModSize>* charOffsets_)
{
	ModUnicodeString section;
	ModAutoPointer<ModUnicodeString> result(new ModUnicodeString);
	ModLanguageSet defaultLangSet;
	// 正規化表記の領域確保（多めに確保する）
	result->reallocate(original_.getLength());

#ifdef SYD_USE_UNA_V10
	// 正規化条件を設定する
	setNormalizedFlagForNormalizer(true);
	UNA::ModNlpAnalyzer* a = getNormalizer(ModCharString());
#endif

	if (byteOffsets_ == 0) {
		// 全体を処理する場合
#ifdef SYD_USE_UNA_V10
#ifdef SYD_USE_UNA_V12
		a->set(original_,defaultLangSet);
		a->getNormalizeBuf(*result);
#else
		a->set(original_);
		a->getWholeText(*result);
#endif
#else
		getNormalizer(ModCharString())
			->normalizeBuf(original_, *result, 0, 0, ModNormalized);
#endif
	}
	else {
		// セクションごとに処理する場合
		; ModAssert(charOffsets_ != 0);

		charOffsets_->clear();
		ModSize beginOffset(0), endOffset(0);
#ifdef SYD_USE_UNA_V10
		ModUnicodeString s;
		const ModUnicodeChar* p = original_;
#endif
		for (ModVector<ModSize>::ConstIterator b(byteOffsets_->begin());
			 b != byteOffsets_->end(); ++b) {
			endOffset = *b/2;
			if (endOffset < beginOffset) {
				ModErrorMessage << "invalid character offset: "
								<< beginOffset << ' ' << endOffset << " at "
								<< (b - byteOffsets_->begin()) << ModEndl;
				ModThrowInvertedFileError(ModInvertedErrorInternal);
			}
			if (endOffset != beginOffset) {
				// セクションの長さが０でない場合の処理
				section.clear();
#ifdef SYD_USE_UNA_V10
				s.allocateCopy(p, endOffset - beginOffset);
#ifdef SYD_USE_UNA_V12
				a->set(s,defaultLangSet);
				a->getNormalizeBuf(section);
#else
				a->set(s);
				a->getWholeText(section);
#endif
				p += (endOffset - beginOffset);
#else
				getNormalizer(ModCharString())
					->normalizeBuf(original_, section,
								   beginOffset, endOffset,
								   ModNormalized);
#endif
				result->append(section);
				beginOffset = endOffset;
			}
			charOffsets_->pushBack(result->getLength());
		}
		; ModAssert(byteOffsets_->getSize() == charOffsets_->getSize());

	}

	return result.release();
}

//
// FUNCTION
// ModInvertedNgramTokenizer::ModInvertedNgramTokenizer -- コンストラクタ
//
// NOTES
// Ngram 分割器をコンストラクトする。
//
// ARGUMENTS
// const ModString& description_
//		パラメータ記述
// const ModSize minLength_
//		最短切り出し長さ
// const ModSize maxLength_
//		最長切り出し長さ
// ModInvertedFile* file_
//		転置ファイル
// const ModBoolean normalizing_
//		正規化処理の指示
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModInvertedNgramTokenizer::ModInvertedNgramTokenizer(
	const ModString& description_,
	ModInvertedFile* file_,
	const ModBoolean normalizing_)
	: target(0), isAllocated(ModFalse),
	  normalizing(normalizing_), normalizer(0),
	  normRscID(getResourceID(description_, normalizerResourceID)),
	  currentLength(0), headOffset(0), tailOffset(0), nextOffset(0),
	  isShortWord(ModFalse),
	  currentMaxLength(0),
	  minLength(1), maxLength(1),
	  ModInvertedTokenizer(file_)
{
	parse(description_);
}

ModInvertedNgramTokenizer::ModInvertedNgramTokenizer(
	const ModSize minLength_,
	const ModSize maxLength_,
	ModInvertedFile* file_,
	const ModBoolean normalizing_)
	: target(0), isAllocated(ModFalse),
	  normalizing(normalizing_), normalizer(0), normRscID(0),
	  currentLength(0), headOffset(0), tailOffset(0), nextOffset(0),
	  isShortWord(ModFalse),
	  currentMaxLength(0),
	  minLength(minLength_), maxLength(maxLength_),
	  ModInvertedTokenizer(file_)
{
}

//
// FUNCTION
// ModInvertedNgramTokenizer::~ModInvertedNgramTokenizer -- デストラクタ
//
// NOTES
// Ngram 分割器をデストラクトする。
// 正規化器があれば、それもデストラクトする。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModInvertedNgramTokenizer::~ModInvertedNgramTokenizer()
{
	clear();
#ifdef SYD_USE_UNA_V10
	if (normalizer) normalizer->releaseResource();
#endif
	delete normalizer, normalizer = 0;
}

//
// FUNCTION
// ModInvertedNgramTokenizer::set -- 処理対象テキストのセット
//
// NOTES
// 処理対象のテキストをセットする。
//
// ARGUMENTS
// const ModUnicodeString& target_
//		テキスト
// const TokenizeMode mode_
//		分割モード
// const ModVector<ModSize>* byteOffsets_
//		バイト位置の配列
// ModVector<ModSize>* characterOffsets_
//		文字位置をセットする配列
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//		参照: STL
//
void
ModInvertedNgramTokenizer::set(const ModUnicodeString& target_,
							   const TokenizeMode mode_,
							   const ModVector<ModSize>* byteOffsets_,
							   ModVector<ModSize>* charOffsets_)
{
	try {
		mode = mode_;

		clear();
		; ModAssert(isAllocated == ModFalse);

		if (normalizing == ModTrue && (mode&skipNormalizing) == 0) {
			// 正規化が必要な場合
			prepareNormalizedText(target_, byteOffsets_, charOffsets_);
			
		} else {
			target = const_cast<ModUnicodeString*>(&target_);
			if (byteOffsets_ != 0) {
				; ModAssert(charOffsets_ != 0);
				setCharOffsets(byteOffsets_, charOffsets_);
			}
		}
		targetLength = target->getLength();

		headOffset = 0;
		tailOffset = 0;
		nextOffset = 0;
		isShortWord = ModFalse;

		if (targetLength == 0) {
			// 空文字列の場合 -- つぎの yield ですぐに false が返るようにする
			currentLength = 0;
			return;
		}

		currentLength
			= ((mode&baseModeMask) == document) ? minLength : maxLength;

		// 先頭位置を進めておく
		if (currentLength > targetLength) {
			// 長さが足りない場合
			headOffset = targetLength;
			currentMaxLength = targetLength;
			if ((mode&baseModeMask) != document) {
				if (minLength > targetLength) {
					// MIN よりも検索文字列が短ければショートワード処理
					isShortWord = ModTrue;
				}
				currentLength = currentMaxLength;
			}
		} else {
			headOffset = currentLength;
			if (maxLength > targetLength) {
				currentMaxLength = targetLength;
			} else {
				currentMaxLength = maxLength;
			}
		}

	} catch (ModException& exception) {
		ModErrorMessage << "set failed: target=" << target_
						<< " mode=" << mode_
						<< ": " << exception << ModEndl;
		ModRethrow(exception);
#ifndef SYD_INVERTED
	} catch (...) {
/* purecov:begin deadcode */
		ModUnexpectedThrow(ModModuleInvertedFile);
/* purecov:end */
#endif
	}
}

//
// FUNCTION
// ModInvertedNgramTokenizer::tokenizeSub -- トークン分割の下請
//
// NOTES
// 分割できた場合は ModTrue が返る。
// 分割対象が短すぎて分割できない場合は ModFalse が返り、分割対象を
// OR展開で検索すべき先頭および末尾の文字列を from, to で返す。
// 
// ARGUMENTS
// const ModUnicodeString& target_
//		分割対象テキスト
// const TokenizeMode mode_
//		分割モード
// ModInvertedLocationListMap& result_
//		分割結果
// ModSize& tokenizedEnd_
//		通常処理の終了位置
// ModUnicodeString& shortWord_
//		ショートワード部分の文字列
// ModUnicodeString& from_
//		ショートワード処理で追加的に展開すべき先頭の文字列
// ModUnicodeString& to_
//		ショートワード処理で追加的に展開すべき末尾の文字列
// QueryTokenizedResult*& result
//		分割結果の配列
// const ModLanguageSet& langSet_
//		言語指定（v1_6のみ）
//
// RETURN
// ショートワード処理が不要ならば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedNgramTokenizer::tokenizeSub(const ModUnicodeString& target_,
									   const TokenizeMode mode_,
									   ModInvertedLocationListMap& result,
									   ModSize& tokenizedEnd,
									   ModUnicodeString& shortWord,
									   ModUnicodeString& from,
									   ModUnicodeString& to
#ifdef V1_6
									   , const ModLanguageSet& langSet_
#endif // V1_6
									   )
{
	try {
		// 古い結果をクリアする
		result.erase(result.begin(), result.end());

#ifdef V1_6
		ModVector<ModLanguageSet> langSets;
		langSets.pushBack(langSet_);
#endif // V1_6

		// まず分割してみる
		ModInvertedTokenizer::tokenize(target_, mode_, result, tokenizedEnd,
									   0, 0
#ifdef V1_6
									   , &langSets
#endif // V1_6
									   , 0, 0);

		if (isShortWord == ModTrue && headOffset > 0) {
			// short word の処理 -- from, to をセットする
			// ただし、空文字列は除外する
			; ModAssert(result.getSize() == 0);
			; ModAssert(currentLength > 0);

			// ngram では先頭からの場合しか short word にならない
			// - shortWord の開始位置は normalWord の終了位置
			tokenizedEnd = 1;	

			shortWord = target->copy(tailOffset, currentLength);

#ifdef DEBUG
			ModMessage << *target << ' ' << shortWord << ' '
					   << headOffset << ' ' << tailOffset << ' '
					   << currentLength << ' ' << int(isShortWord) << ModEndl;
#endif

			// ここで生成される from, to は正しい文字列とはかぎらない
			from = shortWord;
			from += ModUnicodeChar(0x0001);
			to = shortWord;
			to += ModUnicodeChar(0xFFFF);

			return ModFalse;
		}

		// 通常の処理 -- そのまま返ればよい
		; ModAssert(tokenizedEnd == headOffset);
		return ModTrue;

	} catch (ModException& exception) {
		ModErrorMessage << "tokenize failed: target=" << target_
						<< " mode=" << mode_ 
						<< ": " << exception << ModEndl;
		ModRethrow(exception);
#ifndef SYD_INVERTED
	} catch (...) {
/* purecov:begin deadcode */
		ModUnexpectedThrow(ModModuleInvertedFile);
/* purecov:end */
#endif
	}
}

//
// FUNCTION
// ModInvertedNgramTokenizer::tokenizeMulti -- トークン分割
//
// NOTES
// トークン分割を行う。
// 異表記展開が行われた場合、その個数分の結果を生成する。
// 分割結果を呼び出し側が delete しなければならない。
// 
// ARGUMENTS
// const ModUnicodeString& target_
//		分割対象テキスト
// const TokenizeMode mode_
//		分割モード
// QueryTokenizedResult*& result
//		分割結果の配列
// const ModLanguageSet& langSet_
//		言語指定（v1_6のみ）
//
// RETURN
// 分割結果の配列の個数
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModSize
ModInvertedNgramTokenizer::tokenizeMulti(
	const ModUnicodeString& target_,
	const TokenizeMode mode_,
#ifdef INV_TOK_USEVECTOR
	ModVector<QueryTokenizedResult>& result
#else
	QueryTokenizedResult*& result
#endif
#ifdef V1_6
	, const ModLanguageSet& langSet_
#endif // V1_6
	)
{
#ifdef SYD_USE_UNA_V10
	ModVector<ModUnicodeString> expanded;
#else
	ModUnicodeString* expanded = 0;
#endif
	ModSize num(0);

	if (normalizing == ModTrue) {
		
		if ((mode_&skipExpansion) == 0) {
			// 展開を行う
			// 第６引数で正規化を行うか否かを制御（true なら展開だけを実施）
#ifdef SYD_USE_UNA_V10
			// 正規化するかどうかを設定する
			UNA::ModNlpAnalyzer* p = getNormalizer(ModCharString());
			setNormalizedFlagForNormalizer((mode_ & skipNormalizing) == 0);
#ifdef SYD_USE_UNA_V12
			p->set(target_, langSet_);
			p->getExpandBuf(expanded);
#else
			p->set(target_);
			p->getExpandTexts(expanded);
#endif
			num = expanded.getSize();
#else
			num = normalizer->expandBuf(
				target_, expanded, ModNormExpChkOrigStr, 0, 0,
				ModBoolean((mode_&skipNormalizing) != 0));
#endif
		} else if ((mode_&skipNormalizing) == 0) {
			// 正規化だけ行う

			num = 1;
#ifdef SYD_USE_UNA_V10
			ModUnicodeString n;
			// 正規化するかどうかを設定する
			UNA::ModNlpAnalyzer* p = getNormalizer(ModCharString());
			setNormalizedFlagForNormalizer(true);
#ifdef SYD_USE_UNA_V12
			p->set(target_, langSet_);
			p->getNormalizeBuf(n);
#else
			p->set(target_);
			p->getWholeText(n);
#endif
			expanded.pushBack(n);
#else
			expanded = new ModUnicodeString[num];
			normalizer->normalizeBuf(target_, expanded[0], 0, 0, ModNormalized);
#endif

		} else {
			// 正規化も展開も行わない
			// num = 1 としないと tokenizeSub のなかで正規化してしまう

			num = 1;
#ifdef SYD_USE_UNA_V10
			expanded.pushBack(target_);
#else
			expanded = new ModUnicodeString[num];
			expanded[0] = target_;
#endif
		}
	}

	if (num == 0) {
#ifndef SYD_USE_UNA_V10
		; ModAssert(expanded == 0);
#endif
#ifdef INV_TOK_USEVECTOR
		switch (result.getSize()) {
		case 0:
			result.pushBack(QueryTokenizedResult());
			break;
		case 1:
			// 何もしない
			break;
		default:
			result.clear();
			result.pushBack(QueryTokenizedResult());
			break;
		}
		; ModAssert(result.getSize() == 1);
#else
		result = new QueryTokenizedResult[1];
#endif
#ifdef DEBUG
		ModDebugMessage << "no-expanded: " << target_ << ModEndl;
#endif
		// この場合、expand() のなかで正規化されないので、正規化の抑制はしない
		result[0].isNormalWord
			= tokenizeSub(target_, mode_,
						  result[0].locationListMap,
						  result[0].tokenizedEnd,
						  result[0].shortWord,
						  result[0].fromWord,
						  result[0].toWord
#ifdef V1_6
						  , langSet_
#endif // V1_6
						  );
		result[0].target = target_;
		return 1;
	}

	; ModAssert(num != 0);
//	; ModAssert((mode_&skipExpansion) == 0);
#ifdef INV_TOK_USEVECTOR
	if (result.getSize() > num) {
		result.clear();
	}
#else
	result = new QueryTokenizedResult[num];
#endif

	for (ModSize n(0); n < num; ++n) {
#ifdef INV_TOK_USEVECTOR
		if (result.getSize() < n) {
			result.pushBack(QueryTokenizedResult());
		}
#endif
#ifdef DEBUG
		ModDebugMessage << "expanded: " << n << ' ' << expanded[n] << ModEndl;
#endif
		// この場合、expand() のなかで正規化されているので、正規化を抑制する
		result[n].isNormalWord
			= tokenizeSub(expanded[n],
						  TokenizeMode(mode_|skipNormalizing),
						  result[n].locationListMap,
						  result[n].tokenizedEnd,
						  result[n].shortWord,
						  result[n].fromWord,
						  result[n].toWord
#ifdef V1_6
						  , langSet_
#endif // V1_6
						  );
		result[n].target = expanded[n];
	}
#ifndef SYD_USE_UNA_V10
	delete [] expanded;
#endif

	return num;

}

//
// FUNCTION
// ModInvertedNgramTokenizer::yield --- 新たなトークンを出力する
//
// NOTES
// 処理対象のテキストから新たなトークンを獲得し、位置情報とともに出力する。
// 新たなトークンを生成できない場合は ModFalse を返す。
//
// ARGUMENTS
// ModUnicodeString& token
//		トークン
// Occurrence& occurrence
//		位置情報
//
// RETURN
// トークンを生成できた場合には ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
ModBoolean
ModInvertedNgramTokenizer::yield(ModUnicodeString& token,
								 Occurrence& occurrence)
{
	if (currentLength == 0 || isShortWord == ModTrue) {
		return ModFalse;
	} if (currentLength < currentMaxLength) {
		token = target->copy(tailOffset, currentLength);
#ifdef DEBUG
		if (ModInvertedFile::debugLevel > 0) {
			ModDebugMessage << headOffset << ' ' << tailOffset << ' '
							<< currentLength << ' ' << currentMaxLength << ' '
							<< token << ' ' << int(isShortWord) << ModEndl;
		}
#endif
		++currentLength;
		++headOffset;
		occurrence.setOffset(tailOffset);
		return ModTrue;
	}
	else if ((mode&baseModeMask) != document && currentLength < maxLength &&
			 tailOffset != 0 && headOffset == targetLength) {
		// MAX 未満の検索文字列で、ショートワードは false を返しておしまい
		// MIN 以上でショートワードでなければ else で n-gram を返す
		return ModFalse;
	}
	else {
///		; ModAssert(currentLength == currentMaxLength);
		token = target->copy(tailOffset, currentLength);
#ifdef DEBUG
		if (ModInvertedFile::debugLevel > 0) {
			ModDebugMessage << headOffset << ' ' << tailOffset << ' '
							<< currentLength << ' ' << currentMaxLength << ' '
							<< token << ' ' << int(isShortWord) << ModEndl;
		}
#endif
		occurrence.setOffset(tailOffset);
		++tailOffset;
		if (tailOffset >= targetLength) {
			// 末尾に達した -- 現在のトークンは出力する
			currentLength = 0;
			return ModTrue;
		}

		currentLength
			= ((mode&baseModeMask) == document) ? minLength : maxLength;

		headOffset = tailOffset + currentLength;
		if (headOffset >= targetLength) {
			headOffset = targetLength;
		}
		if (tailOffset + maxLength >= targetLength) {
			currentMaxLength = targetLength - tailOffset;
			if (currentLength > currentMaxLength) {
				currentLength = currentMaxLength;
			}
		} else {
			currentMaxLength = maxLength;
		}

		return ModTrue;
	}
}

#ifdef SYD_USE_UNA_V10
//
//	FUNCTION protected
//	ModInvertedNgramTokenizer::setNormalizedFlagForNormalizer
//		-- UNAにパラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	bool isNormalized_
//		正規化するかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ModInvertedNgramTokenizer::setNormalizedFlagForNormalizer(bool isNormalized_)
{
	UNA::ModNlpAnalyzer* p = getNormalizer(ModCharString());
	m_cUnaParameter[_DoNorm] = (isNormalized_) ? _True : _False;
	p->prepare(m_cUnaParameter);
}
#endif

//
// FUNCTION
// ModInvertedNgramTokenizer::parse -- 記述文字列の解析
//
// NOTES
// 記述文字列を解析、それを対応したメンバに設定する。
// 記述法は以下の通り。
//
//		<記述> ::= <最小切り出し文字数>[:<最大切り出し文字数>] [<異表記正規化器リソース指定>]
//		<最小切り出し文字数> ::= 1以上8以下の整数
//		<最大切り出し文字数> ::= 1以上8以下の整数
//				ただし、<最小切り出し文字数>以上であること
//		<異表記正規化器リソース指定> ::= <異表記正規化器リソースキー>:<リソース番号>
//		<異表記正規化器リソースキー> ::= @NORMRSCID
//		<リソース番号> ::= 0以上の整数
//
// デフォルトでは、2 文字単位に切り出し、リソース番号は 0 とする。
// 異表記正規化器リソースキーは ModInvertedTokenizer.cpp で定義されている。
// 異表記正規化器リソース指定は異表記正規化時にのみ有効。
// 記述例を以下に示す。
//
//		eg.1 "2"
//				2 文字単位に切り出す。
//				異表記正規化器リソース番号はデフォルト通り 0。
//		eg.2 "1:2"
//				1 文字以上 2 文字以下を単位に切り出す。
//		eg.3 "@NORMRSC:5"
//				デフォルト通り、2 文字単位に切り出す。
//				異表記正規化器リソース番号は 5。
//
// ※ V1.1 で可能であった不要文字の指定はできなくなった。
//    不要文字は正規化器で削除ればよい。
//
// ARGUMENTS
// const ModString& description
//		記述文字列
//
// RETURN
// なし
//
// EXCEPTIONS
// ModInvertedErrorInvalidTokenizerParameterDescription
//		パラメータ記述が不正である
//
void 
ModInvertedNgramTokenizer::parse(const ModString& description)
{
	ModSize	index(0);
	ModSize	length(description.getLength());
	ModCharString buffer;		// 切り出した記述
	ModCharString token[2];		// bufferから切り出したトークン

	// デフォルトの設定
	minLength = 2;
	maxLength = 2;

	while (index <= length) {
		// スペースを記述の区切りと見なす．
		if (index == length || description[index] == ' ') {

			if (buffer.getLength() == 0 || buffer[0] == '@') {
				// トークンの長さが 0, あるいは @ で始まってれば無視
				++index;
				continue;
			}

			// <最小切り出し文字数>[:<最大切り出し文字数>] の処理
			ModSize count(divideIntoToken(token, buffer));

			// 最小切り出し文字数の指定
			ModInt32 minValue(token[0].toInt());
			if (minValue < 1 || 8 < minValue) {
				// minValue が不正な値
				ModErrorMessage << "invalid minValue: "
								<< token[0] << ModEndl;
				ModThrowInvertedFileError(
					ModInvertedErrorInvalidTokenizerParameterDescription);
			}

			// 最大切り出し文字数の指定
			ModInt32 maxValue(minValue);
			if (count == 2) {
				// maxValue が指定されている
				maxValue = token[1].toInt();
				if (maxValue < minValue || 8 < maxValue) {
					// maxValue が不正な値
					ModErrorMessage << "invalid maxValue: "
									<< token[1] << ModEndl;
					ModThrowInvertedFileError(
						ModInvertedErrorInvalidTokenizerParameterDescription);
				}
			}

			minLength = minValue;
			maxLength = maxValue;

			// 処理の後変数をクリアする必要あり．
			buffer.clear();

		}
		else {
			buffer += description[index];
		}
		++index;
	}
}

//
// FUNCTION
// ModInvertedNgramTokenizer::divideIntoToken
//
// NOTES
// 文字列から区切り文字':'で区切られた単語を取出す．
//	
//
// ARGUMENTS
// ModCharString* token,
//		トークン
// const ModCharString& description
//		記述文字列
//
// RETURN
// 切り出した文字数(最大2単語まで)
//
// EXCEPTIONS
// ModInvertedErrorInvalidTokenizerParameterDescription
//		パラメータ記述が不正である
//
ModSize
ModInvertedNgramTokenizer::divideIntoToken(
	ModCharString* token,
	const ModCharString& description)
{
	ModSize	count(0);
	ModSize	index(0);
	ModSize length(description.getLength());

	token[0].clear();
	while (index < length) {
		if (description[index] == ':') {
			if (++count >= 2) {
				//	記述エラー トークンが2以上
				ModErrorMessage << "too many tokens: " << description << ModEndl;
				ModThrowInvertedFileError(
					ModInvertedErrorInvalidTokenizerParameterDescription);
			}		
			token[count].clear();
		}
		else {
			token[count] += description[index];
			
		}
		++index;
	}
	return ++count;
}

//
// FUNCTION
// ModInvertedNgramTokenizer::getDescription -- 分割器記述の取得
//
// NOTES
// 分割器記述を取得する。
//
// ARGUMENTS
// ModCharString& description_
//		出力バッファ
// const ModBoolean withName_
//		分割器名称の付与指示
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedNgramTokenizer::getDescription(ModCharString& description_,
										  const ModBoolean withName_) const
{
	description_.clear();
	if (withName_ == ModTrue) {
		description_.append(tokenizerName);
		description_.append(':');
	}

	ModOstrStream stream;
	stream << minLength << ':' << maxLength;
	if (normalizing == ModTrue || normRscID != 0) {
		stream << ' ' << normalizerResourceID << ':' << normRscID;
	}
	description_.append(stream.getString());
}

//
// FUNCTION
// ModInvertedNgramTokenizer::isSupported -- 索引付けタイプの判定
//
// NOTES
// 与えられた索引付けタイプをサポートするか否かを判定する。
//
// ARGUMENTS
// const ModInvertedFileIndexingType indexingType_
//		索引付けタイプ
//
// RETURN
// サポートする索引付けタイプなら ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
ModBoolean
ModInvertedNgramTokenizer::isSupported(
	const ModInvertedFileIndexingType indexingType_) const
{
	return (indexingType_ == ModInvertedFileNgramIndexing) ?
		ModTrue : ModFalse;
}

//
// FUNCTION
// ModInvertedNgramTokenizer::isPrenormalizable -- プレノーマライズが適用可能かの判定
//
// NOTES
// プレノーマライズが適用可能か否かを判定する。
// このクラスは常に適用可能。
//
// ARGUMENTS
// なし
//
// RETURN
// 常に ModTrue
//
// EXCEPTIONS
// なし
//
ModBoolean
ModInvertedNgramTokenizer::isPrenormalizable() const
{
	return ModTrue;
}

//
// FUNCTION
// ModInvertedNgramTokenizer::getNormalizer -- 正規化器の作成
//
// NOTES
// 正規化器を作成する。
// 引数で与えられた文字列中のリソースIDを、正規化器作成関数を用いて作成する。
//
// ARGUMENTS
// const ModString& description_
//		記述文字列
//
// RETURN
// 正規化器
//
// EXCEPTIONS
// ModInvertedErrorGetNormalizerFail
//		正規化器作成に失敗した
//
#ifdef SYD_USE_UNA_V10
UNA::ModNlpAnalyzer*
#else
ModNormalizer*
#endif
ModInvertedNgramTokenizer::getNormalizer(const ModCharString& description_)
{
	if (normalizer == 0) {
		if (normRscID == 0) {
			normRscID = getResourceID(description_, normalizerResourceID);
		}
#ifdef DEBUG
		ModDebugMessage << normalizerResourceID << '=' << normRscID << ModEndl;
#endif
		if (getNormalizerFunc == 0 ||
			(normalizer = (*getNormalizerFunc)(normRscID)) == 0) {
			ModErrorMessage << "getNormalizer failed: "
							<< intptr_t(getNormalizerFunc) << ' '
							<< normRscID << ' '
							<< intptr_t(normalizer) << ModEndl;
			ModThrowInvertedFileError(ModInvertedErrorGetNormalizerFail);
		}

#ifdef SYD_USE_UNA_V10
		// UNAに渡すパラメータを作成する
		m_cUnaParameter.insert("expmode", "0");
		m_cUnaParameter.insert("getorg", "false");
		m_cUnaParameter.insert("donorm", "true");
		m_cUnaParameter.insert("normalizeonly", "true");
#endif
	}

	; ModAssert(normalizer != 0);
	return normalizer;
}

//
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
