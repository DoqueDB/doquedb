// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BasicString -- 文字列型データ関連のクラス定義、関数宣言
// 
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_BASICSTRING_H
#define __TRMEISTER_COMMON_BASICSTRING_H

#include "Common/Module.h"
#include "Common/Collation.h"
#include "Common/ScalarData.h"
#include "Common/UnicodeString.h"

#include "Exception/InvalidEscapeSequence.h"

#include "ModTypes.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	TEMPLATE CLASS
//	Common::BasicString<TYPE> -- 
//
//	TEMPLATE ARGUMENTS
//		class TYPE
//
//	NOTES

template <class TYPE>
class BasicString
{
public:

//	TEMPLATE FUNCTION public
//	Common::BasicString::like --
//		Whether the pattern matches to oneself is examined.
//
//	TEMPLATE ARGUMENTS
//		class TYPE
//	
//	NOTES
//	スピードを考慮してエスケープ文字があるものとないものの実装を完全に分離した
//
//	ARGUMENTS
//	const TYPE* pHead_
//		調べる対象の文字列の先頭
//	ModSize uiLength_
//		調べる対象の文字列の文字数(終端文字を含まない)
//	const TYPE* pPatternHead_
//		パターン文字列の先頭
//	ModSize uiPatternLength_
//		パターン文字列の文字数(終端文字を含まない)
//	Common::Collation::Type::Value eCollation_
//		文字列のCollation
//
//	RETURN
//	マッチした場合はtrue、それ以外の場合はfalseを返す。
//
//	EXCEPTIONS
// 
static bool
like(const TYPE* pHead_, ModSize uiLength_,
	 const TYPE* pPatternHead_, ModSize uiPatternLength_,
	 Collation::Type::Value eCollation_ = Collation::Type::NoPad)
{
//	; _TRMEISTER_ASSERT(pHead_);
//	; _TRMEISTER_ASSERT(pPatternHead_);

	if (uiPatternLength_ == 0) {		// 空文字列は空文字列にのみマッチ
		return (uiLength_ == 0);
	}

	const TYPE* pszThisHead = pHead_;
	const TYPE* pszThisTail = pHead_ + uiLength_;
	const TYPE* pszPatternHead = pPatternHead_;
	const TYPE* pszPatternTail = pPatternHead_ + uiPatternLength_;

	// 文字列照合では以下の性質を利用する
	// 1．パターンの先頭および末尾から最初に現れる%までは長さ指定の文字列一致で調べられる
	//		abc%foobar%def =(match)=> "abc 〜 def"
	// 2．%ではさまれたパターンについてマッチする部分が複数あったとしても
	//	  先頭から見て最初にマッチした部分を採用して問題はない
	//		%foo%bar% =(match)=> "〜 foo 〜 bar 〜(1)〜"
	//		※(1)の部分に"foo"や"bar"があってもマッチングに影響はない
	
	// パターンの中で最初と最後の%を探す
	// (見つからなかったら終端文字の場所が入る)
	const TYPE* pFirstPercent = getFirstPercent(pszPatternHead, pszPatternTail);
	const TYPE* pLastPercent = getLastPercent(pFirstPercent, pszPatternTail);

	const TYPE* pHead = 0;
	const TYPE* pTail = 0;

	// 先頭から最初の%までをマッチングする
	// 返り値にはマッチした部分の1つ後の文字をさすポインターが入る
	pHead = comparePattern(pszThisHead, pszThisTail, pszPatternHead, pFirstPercent);

	if (pHead) {

		if (pFirstPercent == pLastPercent) {
			// この条件が満たされるときは%がないとき
			// -> pHead が文字列の終端であれば一致

			// %がないときはCollationにより空白を無視する
			// その後の検討でCollationに関係なくlikeはNo Padで処理するという仕様になった
#ifdef LIKE_COLLATION
			if (eCollation_ != Collation::Type::NoPad) {
				// 空白をすべて読み飛ばす
				while (*pHead == UnicodeChar::usSpace) ++pHead;
			}
#endif

			return pHead == pszThisTail;
		}

		// 末尾から最後の%までをマッチングする
		// 返り値にはマッチした部分の先頭の文字をさすポインターが入る
		pTail = reverseComparePattern(pszThisTail, pHead, pszPatternTail, pLastPercent);

		if (pTail) {

			// 残りのパターンを先頭から順にマッチさせる
			pszPatternTail = pLastPercent - 1;
			pszPatternHead = pFirstPercent + 1;
			pszThisHead = pHead;
			pszThisTail = pTail;

			while (pszPatternHead < pszPatternTail) {
				// 残ったパターンの中で最初の%を探す
				pFirstPercent = getFirstPercent(pszPatternHead, pszPatternTail);

				// 先頭から最初の%までがマッチする部分を探す
				// 返り値にはマッチした部分の1つ後の文字をさすポインターが入る
				pHead = searchPattern(pszThisHead, pszThisTail, pszPatternHead, pFirstPercent);

				if (!pHead) break;

				// 次の部分を調べる
				pszPatternHead = pFirstPercent + 1;
				pszThisHead = pHead;
			}
		}
	}

	return pHead && pTail;
}

//	TEMPLATE FUNCTION public
//	Common::BasicString::like -- 文字列マッチを行う
//
//	TEMPLATE ARGUMENTS
//		class TYPE
//	
//	NOTES
//	スピードを考慮してエスケープ文字があるものとないものの実装を完全に分離した
//
//	ARGUMENTS
//	const TYPE* pHead_
//		調べる対象の文字列の先頭
//	ModSize uiLength_
//		調べる対象の文字列の文字数(終端文字を含まない)
//	const TYPE* pPatternHead_
//		パターン文字列の先頭
//	ModSize uiPatternLength_
//		パターン文字列の文字数(終端文字を含まない)
//	TYPE cEscape
//		エスケープ文字
//	Common::Collation::Type::Value eCollation_
//		文字列のCollation
//
//	RETURN
//	マッチした場合はtrue、それ以外の場合はfalseを返す。
//
//	EXCEPTIONS
//
static bool
like(const TYPE* pHead_, ModSize uiLength_,
	 const TYPE* pPatternHead_, ModSize uiPatternLength_,
	 TYPE cEscape_,
	 Collation::Type::Value eCollation_ = Collation::Type::NoPad)
{
//	; _TRMEISTER_ASSERT(pHead_);
//	; _TRMEISTER_ASSERT(pPatternHead_);

	if (uiPatternLength_ == 0) {		// 空文字列は空文字列にのみマッチ
		return (uiLength_ == 0);
	}

	const TYPE* pszThisHead = pHead_;
	const TYPE* pszThisTail = pHead_ + uiLength_;
	const TYPE* pszPatternHead = pPatternHead_;
	const TYPE* pszPatternTail = pPatternHead_ + uiPatternLength_;

	// 文字列照合では以下の性質を利用する
	// 1．パターンの先頭および末尾から最初に現れる%までは長さ指定の文字列一致で調べられる
	//		abc%foobar%def =(match)=> "abc 〜 def"
	// 2．%ではさまれたパターンについてマッチする部分が複数あったとしても
	//	  先頭から見て最初にマッチした部分を採用して問題はない
	//		%foo%bar% =(match)=> "〜 foo 〜 bar 〜(1)〜"
	//		※(1)の部分に"foo"や"bar"があってもマッチングに影響はない
	
	// パターンの中で最初と最後の%を探す
	// (見つからなかったら終端文字の場所が入る)
	const TYPE* pFirstPercent = getFirstPercent(pszPatternHead, pszPatternTail, cEscape_);
	const TYPE* pLastPercent = getLastPercent(pFirstPercent, pszPatternTail, cEscape_);

	const TYPE* pHead = 0;
	const TYPE* pTail = 0;

	// 先頭から最初の%までをマッチングする
	// 返り値にはマッチした部分の1つ後の文字をさすポインターが入る
	pHead = comparePattern(pszThisHead, pszThisTail, pszPatternHead, pFirstPercent, cEscape_);

	if (pHead) {

		if (pFirstPercent == pLastPercent) {
			// この条件が満たされるときは%がないとき
			// -> pHead が文字列の終端であれば一致

			// %がないときはCollationにより空白を無視する
			// その後の検討でCollationに関係なくlikeはNo Padで処理するという仕様になった
#ifdef LIKE_COLLATION
			if (eCollation_ != Collation::Type::NoPad) {
				// 空白をすべて読み飛ばす
				while (*pHead == UnicodeChar::usSpace) ++pHead;
			}
#endif

			return pHead == pszThisTail;
		}

		// 末尾から最後の%までをマッチングする
		// 返り値にはマッチした部分の先頭の文字をさすポインターが入る
		pTail = reverseComparePattern(pszThisTail, pHead, pszPatternTail, pLastPercent, cEscape_);

		if (pTail) {

			// 残りのパターンを先頭から順にマッチさせる
			pszPatternTail = pLastPercent - 1;
			pszPatternHead = pFirstPercent + 1;
			pszThisHead = pHead;
			pszThisTail = pTail;

			while (pszPatternHead < pszPatternTail) {
				// 残ったパターンの中で最初の%を探す
				pFirstPercent = getFirstPercent(pszPatternHead, pszPatternTail, cEscape_);

				// 先頭から最初の%までがマッチする部分を探す
				// 返り値にはマッチした部分の1つ後の文字をさすポインターが入る
				pHead = searchPattern(pszThisHead, pszThisTail, pszPatternHead, pFirstPercent, cEscape_);

				if (!pHead) break;

				// 次の部分を調べる
				pszPatternHead = pFirstPercent + 1;
				pszThisHead = pHead;
			}
		}
	}

	return pHead && pTail;
}

private:

//	TEMPLATE FUNCTION private
//	Common::BasicString::traceEscape --
//		指定された文字から前に連続しているエスケープ文字の先頭を得る
//
//	TEMPLATE ARGUMENTS
//		class TYPE
//	
//	NOTES
//
//	ARGUMENTS
//		const TYPE* p_
//			エスケープされているか調べる文字をさすポインター
//		const TYPE* pHead_
//			調べる文字列の先頭をさすポインター
//		const TYPE& cEscape_
//			エスケープ文字
//
//	RETURN
//		const TYPE*
//			0		エスケープされていない場合
//			0以外	エスケープされているとき、連続したエスケープ文字の先頭を返す
//
//	EXCEPTIONS
//		なし

static const TYPE*
traceEscape(const TYPE* p_, const TYPE* pHead_, const TYPE& cEscape_)
{
	if (cEscape_ == static_cast<TYPE>(UnicodeChar::usNull))
		return 0;

	const TYPE* q = p_ - 1;
	while (q >= pHead_ && *q == cEscape_)
		--q;
	if (((p_ - q) % 2) == 0) {
		// p_以前で最初にエスケープでない文字との差が
		// 2ならp_の文字はエスケープされている(2の倍数でも同じ)
		// (*pHead_)がエスケープ文字でないことが前提条件
		return q + 1;
	}
	return 0;
}

//	TEMPLATE FUNCTION private
//	Common::BasicString::getFirstPercent -- 最初の%を得る
//
//	TEMPLATE ARGUMENTS
//		class TYPE
//	
//	NOTES
//
//	ARGUMENTS
//		const TYPE* pHead_
//			調べる文字列の先頭をさすポインター
//		const TYPE* pTail_
//			調べる文字列の終端位置をさすポインター
//		const TYPE& cEscape_
//			エスケープ文字
//
//	RETURN
//		const TYPE*
//			最初の%の位置をさすポインター
//			%がないときはpTail_が返る
//
//	EXCEPTIONS
//		なし

static const TYPE*
getFirstPercent(const TYPE* pHead_, const TYPE* pTail_,
				const TYPE& cEscape_)
{
	for (const TYPE* p = pHead_; p < pTail_; ++p)
		if (*p == cEscape_)
			++p;
		else if (*p == static_cast<TYPE>(UnicodeChar::usPercent))
			return p;
	return pTail_;
}

//	TEMPLATE FUNCTION private
//	Common::BasicString::getLastPercent -- 最後の%を得る
//
//	TEMPLATE ARGUMENTS
//		class TYPE
//	
//	NOTES
//
//	ARGUMENTS
//		const TYPE* pHead_
//			調べる文字列の先頭をさすポインター
//		const TYPE* pTail_
//			調べる文字列の終端位置をさすポインター
//		const TYPE& cEscape_
//			エスケープ文字
//
//	RETURN
//		const TYPE*
//			最後の%の次の位置をさすポインター
//			%がないときはpHead_が返る
//
//	EXCEPTIONS
//		なし

static const TYPE*
getLastPercent(const TYPE* pHead_, const TYPE* pTail_,
			   const TYPE& cEscape_)
{
	const TYPE* p = pTail_ - 1;
	for (; p >= pHead_; --p)
		if (*p == static_cast<TYPE>(UnicodeChar::usPercent))
			// '%'がエスケープされているか調べる
			if (const TYPE* q =
				traceEscape(p, pHead_, cEscape_))
				p = q;
			else
				// エスケープされていない '%' を見つけた
				return p + 1;

	return pHead_;
}

//	TEMPLATE FUNCTION private
//	Common::BasicString::comparePattern -- パターンの前方一致
//
//	TEMPLATE ARGUMENTS
//		class TYPE
//	
//	NOTES
//
//	ARGUMENTS
//		const TYPE* pHead_
//			調べる文字列の先頭をさすポインター
//		const TYPE* pTail_
//			調べる文字列の終端位置をさすポインター
//		const TYPE* pPatternHead_
//			パターン文字列の先頭をさすポインター
//		const TYPE* pPatternTail_
//			パターン文字列の終端位置をさすポインター
//		const TYPE& cEscape_
//			エスケープ文字
//
//	RETURN
//		const TYPE*
//			先頭からパターンに合致した部分の次の文字をさすポインター
//
//	EXCEPTIONS
//		なし

static const TYPE*
comparePattern(const TYPE* pHead_, const TYPE* pTail_,
			   const TYPE* pPatternHead_, const TYPE* pPatternTail_,
			   const TYPE& cEscape_)
{
	if (pPatternHead_ == pPatternTail_)
		return pHead_;

	const TYPE* p = pHead_;
	const TYPE* pPattern = pPatternHead_;
	for (; p != pTail_ && pPattern != pPatternTail_; ++p, ++pPattern) {
		if (*pPattern == cEscape_) {
			++pPattern;
			if (pPattern == pPatternTail_)
				// エスケープ文字でパターンが終了している場合
				throw Exception::InvalidEscapeSequence("Common",
													   __FILE__,
													   __LINE__);

		} else if (*pPattern == static_cast<TYPE>(UnicodeChar::usLowLine)) {
			continue;
		}

		if (*pPattern != *p)
			break;
	}

	return (pPattern == pPatternTail_) ? p : 0;
}

//	TEMPLATE FUNCTION private
//	Common::BasicString::reverseComparePattern -- パターンの後方一致
//
//	TEMPLATE ARGUMENTS
//		class TYPE
//	
//	NOTES
//
//	ARGUMENTS
//		const TYPE* pTail_
//			調べる文字列の終端位置をさすポインター
//		const TYPE* pHead_
//			調べる文字列の先頭をさすポインター
//		const TYPE* pPatternTail_
//			パターン文字列の終端位置をさすポインター
//		const TYPE* pPatternHead_
//			パターン文字列の先頭をさすポインター
//		const TYPE& cEscape_
//			エスケープ文字
//
//	RETURN
//		const TYPE*
//			最後尾からパターンに合致した部分の先頭文字をさすポインター
//
//	EXCEPTIONS
//		なし

static const TYPE*
reverseComparePattern(const TYPE* pTail_, const TYPE* pHead_,
					  const TYPE* pPatternTail_, const TYPE* pPatternHead_,
					  const TYPE& cEscape_)
{
	if (pPatternTail_ == pPatternHead_)
		return pTail_;

	const TYPE* p = pTail_ - 1;
	const TYPE* pPattern = pPatternTail_ - 1;

	if (*pPattern == cEscape_) {
		// 最終文字がエスケープ文字ならrun outしていないか調べる
		const TYPE* q = traceEscape(pPattern, pPatternHead_, cEscape_);
		if (!q)
			// 最終文字がエスケープされていないということは
			// 単独のエスケープ文字でパターンが終わっていることを意味する
			throw Exception::InvalidEscapeSequence("Common",
												   __FILE__,
												   __LINE__);
	}

	for (; p >= pHead_ && pPattern >= pPatternHead_; --p, --pPattern) {
		const TYPE* q = traceEscape(pPattern, pPatternHead_, cEscape_);

		if (!q) {
			// エスケープされていないので特殊文字の処理をする
			if (*pPattern == static_cast<TYPE>(UnicodeChar::usLowLine)) {
				continue;
			}
		}

		if (*pPattern != *p)
			break;

		if (q) {
			// エスケープされている文字を処理したので
			// エスケープ文字を1つ読み飛ばす
			--pPattern;
		}
	}

	return (pPattern < pPatternHead_)? p + 1 : 0;
}

//	TEMPLATE FUNCTION private
//	Common::BasicString::searchPattern -- パターンの中間一致
//
//	TEMPLATE ARGUMENTS
//		class TYPE
//	
//	NOTES
//
//	ARGUMENTS
//		const TYPE* pHead_
//			調べる文字列の先頭をさすポインター
//		const TYPE* pTail_
//			調べる文字列の終端位置をさすポインター
//		const TYPE* pPatternHead_
//			パターン文字列の先頭をさすポインター
//		const TYPE* pPatternTail_
//			パターン文字列の終端位置をさすポインター
//		const TYPE& cEscape_
//			エスケープ文字
//
//	RETURN
//		const TYPE*
//			先頭から調べてパターンに最初に合致した部分の次の文字をさすポインター
//
//	EXCEPTIONS
//		なし

static const TYPE*
searchPattern(const TYPE* pHead_, const TYPE* pTail_,
			  const TYPE* pPatternHead_, const TYPE* pPatternTail_,
			  const TYPE& cEscape_)
{
	if (pPatternHead_ == pPatternTail_)
		return pHead_;

	const TYPE* p = pHead_;
	const TYPE* pPatternHead = pPatternHead_;
	const TYPE* pNext = 0;

	// 先頭が'_'ならパターンを読み進める
	if (cEscape_ != static_cast<TYPE>(UnicodeChar::usLowLine))
		while (pPatternHead != pPatternTail_ && *pPatternHead == static_cast<TYPE>(UnicodeChar::usLowLine) && p != pTail_)
			++p, ++pPatternHead;

	if (pPatternHead == pPatternTail_)
		// パターンが終わっていたら合致
		return p;

	else if (p == pTail_)
		// パターンが残っているのに文字列が尽きたら合致しない
		return 0;

	// パターンの先頭文字
	TYPE cPatternHead =
		(*pPatternHead == cEscape_ && pPatternHead + 1 != pPatternTail_) ?
		*(pPatternHead + 1) : *pPatternHead;

 ReSearch:
//	; _TRMEISTER_ASSERT(p != pTail_);

	while (*p != cPatternHead)
		if (++p == pTail_) break;		// &&でやるよりp == pTail_の比較が1回だけ減る

	const TYPE* pPattern = pPatternHead;
	for (; p != pTail_ && pPattern != pPatternTail_; ++p, ++pPattern, pNext = (!pNext && *p == cPatternHead) ? p : pNext) {
		if (*pPattern == cEscape_) {
			++pPattern;
			if (pPattern == pPatternTail_)
				// エスケープ文字でパターンが終了している場合
				throw Exception::InvalidEscapeSequence("Common",
													   __FILE__,
													   __LINE__);

		} else if (*pPattern == static_cast<TYPE>(UnicodeChar::usLowLine)) {
			continue;
		}

		if (*pPattern != *p)
			break;
	}

	if (pPattern != pPatternTail_) {
		if (p == pTail_)
			return 0;

		if (pNext)
			p = pNext;

		pNext = 0;

		goto ReSearch;
	}

	return p;
}

//	TEMPLATE FUNCTION private
//	Common::BasicString::getFirstPercent -- 最初の%を得る(エスケープ文字なし)
//
//	TEMPLATE ARGUMENTS
//		class TYPE
//	
//	NOTES
//
//	ARGUMENTS
//		const TYPE* pHead_
//			調べる文字列の先頭をさすポインター
//		const TYPE* pTail_
//			調べる文字列の終端位置をさすポインター
//
//	RETURN
//		const TYPE*
//			最初の%の位置をさすポインター
//			%がないときはpTail_が返る
//
//	EXCEPTIONS
//		なし

static const TYPE*
getFirstPercent(const TYPE* pHead_, const TYPE* pTail_)
{
	for (const TYPE* p = pHead_; p < pTail_; ++p)
		if (*p == static_cast<TYPE>(UnicodeChar::usPercent))
			return p;
	return pTail_;
}

//	TEMPLATE FUNCTION private
//	Common::BasicString::getLastPercent -- 最後の%を得る(エスケープ文字なし)
//
//	TEMPLATE ARGUMENTS
//		class TYPE
//	
//	NOTES
//
//	ARGUMENTS
//		const TYPE* pHead_
//			調べる文字列の先頭をさすポインター
//		const TYPE* pTail_
//			調べる文字列の終端位置をさすポインター
//
//	RETURN
//		const TYPE*
//			最後の%の次の位置をさすポインター
//			%がないときはpHead_が返る
//
//	EXCEPTIONS
//		なし

static const TYPE*
getLastPercent(const TYPE* pHead_, const TYPE* pTail_)
{
	const TYPE* p = pTail_ - 1;
	for (; p >= pHead_; --p)
		if (*p == static_cast<TYPE>(UnicodeChar::usPercent))
			return p + 1;
	return pHead_;
}

//	TEMPLATE FUNCTION private
//	Common::BasicString::comparePattern -- パターンの前方一致
//
//	TEMPLATE ARGUMENTS
//		class TYPE
//	
//	NOTES
//
//	ARGUMENTS
//		const TYPE* pHead_
//			調べる文字列の先頭をさすポインター
//		const TYPE* pTail_
//			調べる文字列の終端位置をさすポインター
//		const TYPE* pPatternHead_
//			パターン文字列の先頭をさすポインター
//		const TYPE* pPatternTail_
//			パターン文字列の終端位置をさすポインター
//
//	RETURN
//		const TYPE*
//			先頭からパターンに合致した部分の次の文字をさすポインター
//
//	EXCEPTIONS
//		なし

static const TYPE*
comparePattern(const TYPE* pHead_, const TYPE* pTail_,
			   const TYPE* pPatternHead_,
			   const TYPE* pPatternTail_)
{
	if (pPatternHead_ == pPatternTail_)
		return pHead_;

	const TYPE* p = pHead_;
	const TYPE* pPattern = pPatternHead_;
	for (; p != pTail_ && pPattern != pPatternTail_; ++p, ++pPattern) {
		
		if (*pPattern == static_cast<TYPE>(UnicodeChar::usLowLine))
			continue;

		if (*pPattern != *p)
			break;
	}

	return (pPattern == pPatternTail_) ? p : 0;
}

//	TEMPLATE FUNCTION private
//	Common::BasicString::reverseComparePattern -- パターンの後方一致
//
//	TEMPLATE ARGUMENTS
//		class TYPE
//	
//	NOTES
//
//	ARGUMENTS
//		const TYPE* pTail_
//			調べる文字列の終端位置をさすポインター
//		const TYPE* pHead_
//			調べる文字列の先頭をさすポインター
//		const TYPE* pPatternTail_
//			パターン文字列の終端位置をさすポインター
//		const TYPE* pPatternHead_
//			パターン文字列の先頭をさすポインター
//
//	RETURN
//		const TYPE*
//			最後尾からパターンに合致した部分の先頭文字をさすポインター
//
//	EXCEPTIONS
//		なし

static const TYPE*
reverseComparePattern(const TYPE* pTail_, const TYPE* pHead_,
					  const TYPE* pPatternTail_, const TYPE* pPatternHead_)
{
	if (pPatternTail_ == pPatternHead_)
		return pTail_;

	const TYPE* p = pTail_ - 1;
	const TYPE* pPattern = pPatternTail_ - 1;

	for (; p >= pHead_ && pPattern >= pPatternHead_; --p, --pPattern) {
		// エスケープされていないので特殊文字の処理をする
		if (*pPattern == static_cast<TYPE>(UnicodeChar::usLowLine))
			continue;

		if (*pPattern != *p)
			break;
	}

	return (pPattern < pPatternHead_)? p + 1 : 0;
}

//	TEMPLATE FUNCTION private
//	Common::BasicString::searchPattern -- パターンの中間一致
//
//	TEMPLATE ARGUMENTS
//		class TYPE
//	
//	NOTES
//
//	ARGUMENTS
//		const TYPE* pHead_
//			調べる文字列の先頭をさすポインター
//		const TYPE* pTail_
//			調べる文字列の終端位置をさすポインター
//		const TYPE* pPatternHead_
//			パターン文字列の先頭をさすポインター
//		const TYPE* pPatternTail_
//			パターン文字列の終端位置をさすポインター
//
//	RETURN
//		const TYPE*
//			先頭から調べてパターンに最初に合致した部分の次の文字をさすポインター
//
//	EXCEPTIONS
//		なし

static const TYPE*
searchPattern(const TYPE* pHead_, const TYPE* pTail_,
			  const TYPE* pPatternHead_, const TYPE* pPatternTail_)
{
	if (pPatternHead_ == pPatternTail_)
		return pHead_;

	const TYPE* p = pHead_;
	const TYPE* pPatternHead = pPatternHead_;
	const TYPE* pNext = 0;

	// 先頭が'_'ならパターンを読み進める
	while (pPatternHead != pPatternTail_ && *pPatternHead == static_cast<TYPE>(UnicodeChar::usLowLine) && p != pTail_)
		++p, ++pPatternHead;

	if (pPatternHead == pPatternTail_)
		// パターンが終わっていたら合致
		return p;

	else if (p == pTail_)
		// パターンが残っているのに文字列が尽きたら合致しない
		return 0;

	// パターンの先頭文字
	TYPE cPatternHead = *pPatternHead;

 ReSearch:
//	; _TRMEISTER_ASSERT(p != pTail_);

	while (*p != cPatternHead)
		if (++p == pTail_) break;		// &&でやるよりp == pTail_の比較が1回だけ減る

	const TYPE* pPattern = pPatternHead;
	for (; p != pTail_ && pPattern != pPatternTail_; ++p, ++pPattern, pNext = (!pNext && *p == cPatternHead) ? p : pNext) {
		if (*pPattern == static_cast<TYPE>(UnicodeChar::usLowLine))
			continue;

		if (*pPattern != *p)
			break;
	}

	if (pPattern != pPatternTail_) {
		if (p == pTail_)
			return 0;

		if (pNext)
			p = pNext;

		pNext = 0;

		goto ReSearch;
	}

	return p;
}

};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_BASICSTRING_H

//
//	Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
