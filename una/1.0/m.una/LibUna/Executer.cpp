// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
//	Executer.cpp -- Implement file of Executer class
// 
// Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
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

#include "LibUna/Executer.h"
#include "LibUna/Algorithm.h"
#include "LibUna/Morph.h"
#include "LibUna/Keyword.h"
#include "LibUna/Data.h"
#include "ModAutoMutex.h"
#include "ModCriticalSection.h"
#include "ModUnicodeCharTrait.h"

_UNA_USING

namespace Local {

	// TYPEDEF
	//	RxResult -- ModUnicodeRegularExpression の walk 結果を格納するためのコンテナ
	//
	// MEMO
	//
	typedef		ModVector< Type::Range<const ModUnicodeChar*> >
		RxResult;

	// FUNCTION 
	//	makeRxResult -- Common::Data::RegularExpression の結果を作成する
	//		
	// NOTES
	//	rx の結果を取得するためにロックをかける。
	//	そのため、とりあえず結果だけを作成する
	//
	// ARGUMENTS
	//	Common::Type::RegularExpression& rxdata_
	//		正規表現クラス
	//	const ModUnicodeChar* string_
	//		walk する開始位置
	//	ModSize len_
	//		walk length
	//	RxResult& result_
	//		結果格納コンテナ
	//
	// RETURN
	//
	// EXCEPTIONS
	//
	void
	makeRxResult(Type::RegularExpression& rxdata_,
				 const ModUnicodeChar* string_,
				 ModSize len_,
				 RxResult& result_)
	{
		// ModUnicodeRegularExpression の為のロック
		ModAutoMutex<ModCriticalSection> mutex(rxdata_._cs.get());
		mutex.lock();

		ModUnicodeRegularExpression& rx = *(rxdata_._rx);
		int iMatchNum = rx.walk(string_, len_);

		// 予約しておく
		result_.reserve(iMatchNum);

		// マッチング結果を参照する
		const ModUnicodeChar* tmp = 0;
		for ( int i = 0; i < iMatchNum; ++i ) {
			tmp = rx.matchBegin(i, 0);
			result_.pushBack(Type::Range<const ModUnicodeChar*>(
								 tmp,
								 (ModSize)(rx.matchEnd(i, 0) - tmp)));
		}
	}

	// FUNCTION 
	//	isNorm -- 形態素列に正規表現を持った形態素があるか問い合わせる
	//		
	// NOTES
	//
	//
	// ARGUMENTS
	//	ModVector<Common::Morph>::ConstIterator start_
	//		morph start position
	//	ModVector<Common::Morph>::ConstIterator fin_
	//		morph end position
	//
	// RETURN
	//	ModBoolean	ModTrue		正規表現を持った形態素がある
	//				ModFalse	正規表現を持った形態素がない
	//
	// EXCEPTIONS
	//
	ModBoolean
	isNorm(ModVector<Morph>::ConstIterator start_,
		   ModVector<Morph>::ConstIterator fin_)
	{
		for ( ; start_ != fin_; ++start_ )
			if ( (*start_).isNorm() )
				return ModTrue;
		return ModFalse;
	}

} // end of namespace Local

// FUNCTION pubic
//	Executer::getKeywordCombination
//		-- 形態素の並びより全てのキーワードの組み合わせを追加する
//
//NOTES
//	[it  fin) までがキーワード。
//
// ARGUMENTS
//	ModVector<const Common::Morph>::ConstIterator it_,
//		morph start position
//	ModVector<const Common::Morph>::ConstIterator fin_,
//		morph end position
//	Common::KeywordMap& keyword_
//		キーワード格納コンテナ
//	ModVector<Common::Keyword*>* pNewKeyword = 0
//		新たに追加したキーワードを格納するコンテナへのポインタ
//		設定されている時のみ、追加する
//
// RETURN
//
// EXCEPTIONS
//
void
Executer::getKeywordCombination(ModVector<Morph>::ConstIterator it_,
				ModVector<Morph>::ConstIterator fin_,
				KeywordMap& keyword_,
				ModVector<Keyword*>* pNewKeyword)
{
	int lenMax = fin_ - it_;

	int lenMin = 1;
	//Local::getUsingOneWord() ? 1 : 2;

	// 形態素の並びをキーワードにする
	for ( ; lenMax >= lenMin; --lenMax ) {
		ModVector<Morph>::ConstIterator tgt = it_;
		ModVector<Morph>::ConstIterator tgtFin = fin_ - lenMax + 1;

		for ( ; tgt != tgtFin; ++tgt ) {
			Keyword tmpkw;
#if 1
			tmpkw.pushBackPosition(&(*tgt), lenMax);
#else
			tmpkw.pushBackPosition(tgt, lenMax);
#endif

			// 既に登録されているか検索する
			ModUnicodeString tmpstr = tmpkw.getNorm();
			KeywordMap::Iterator findkey = keyword_.find(tmpstr);
			if ( findkey == keyword_.end() ) {

				// 同じキーワードが無い場合に保存する
				keyword_.insert(tmpstr, tmpkw);

				if ( pNewKeyword != 0 ) {
					// 新たに作成したキーワードを保存する

					// 改めて検索し、追加する
					findkey = keyword_.find(tmpstr);

					pNewKeyword->pushBack(&((*findkey).second));
				}
			} else {
				// キーワードがあるなら出現位置だけ保存する
#if 1
				((*findkey).second).pushBackPosition(&(*tgt), lenMax);
#else
				((*findkey).second).pushBackPosition(tgt, lenMax);
#endif
			}

		}

		//if ( Local::getMaxAgKeyword() ) {
		if ( ModTrue ) {
			// キーワードは最大一致しか使用しない
   		break;
		}
	}
}

// FUNCTION pubic
//	Executer::getWordTypeRx
//		-- 品詞列の正規表現結果を取得する
//
// NOTES
//
// ARGUMENTS
//	ModUnicodeRegularExpression* rx_
//		正規表現マッチングクラス
//	ModUnicodeChar* typeString_
//		文字化された品詞 ID
//	unsigned int len_
//		マッチング長さ
//	Common::Bitset& wordFlag_
//		キーワード候補を表現するビットセット
//	ModBoolean flag_ = ModTrue
//		設定するフラグ
//
// RETURN
//
// EXCEPTIONS
//
void
Executer::getWordTypeRx(Type::RegularExpression& rxdata_,
			const ModUnicodeChar* typeString_,
			unsigned int start_,
			unsigned int len_,
			Bitset& result_,
			ModBoolean flag_)
{
	Local::RxResult rxresult;

	// 正規表現の結果を作成する
	Local::makeRxResult(rxdata_, typeString_ + start_, len_, rxresult);

	// マッチング結果を参照しながらフラグを設定する
	Local::RxResult::Iterator it  = rxresult.begin();
	Local::RxResult::Iterator fin = rxresult.end();
	for ( ; it != fin; ++it ) {

		int startPos = (int)((*it)._start - typeString_);
		int jMax = startPos + (*it)._len;

		for ( int j = startPos ; j < jMax; ++j )
			result_[j] = flag_;
	}
}

// FUNCTION pubic
// 	Executer::getWordTextRx
//		-- 形態素表記の正規表現結果を取得する
//
// NOTES
//
// ARGUMENTS
//	ModUnicodeRegularExpression* rx_
//		正規表現マッチクラス
//	ModVector<Common::Word>::ConstIterator wordIt_
//		処理対象になっている形態素の開始位置
//	unsigned int startPos_
//		処理開始位置
//	unsigned int endPos_
//		処理終了位置
//	Common::Bitset& keyword_
//		キーワードを表現するビットセット
//	ModBoolean flag_
//		設定するフラグ
//
// RETURN
//
// EXCEPTIONS
//
void
Executer::getWordTextRx(Type::RegularExpression& rxdata_,
			ModVector<Morph>::ConstIterator wordIt_,
			unsigned int start_,
			unsigned int len_,
			Bitset& keyword_,
			ModBoolean flag_)
{
	typedef ModVector<Morph>::ConstIterator MorphIter;

	MorphIter startWord = wordIt_ + start_;
	MorphIter endWord = startWord + len_;

	// 正規化済の文字列を作成する
	ModUnicodeOstrStream buf;
	const ModUnicodeChar* normstr;
	ModSize normlen = 0;
	ModUnicodeString normtmp;
	if ( Local::isNorm(startWord, endWord) ) {
		// 正規化文字があったので文字列を作成する
		MorphIter tmpWord = startWord;
		for ( ; tmpWord != endWord; ++tmpWord ) {
			buf << (*tmpWord).getNorm();
		}
		normtmp = buf.getString();
		normstr = normtmp;
		normlen = normtmp.getLength();
	} else {
		// 正規化文字は無いので文字列は作成せず、文字列長さを求める
		normstr = (*startWord).getOrgPos();
		MorphIter tmpWord = startWord;
		for ( ; tmpWord != endWord; ++tmpWord ) {
			normlen += (*tmpWord).getOrgLen();
		}
	}

	// マッチング結果を参照しながらフラグを立てる
	MorphIter tgtPos = startWord;
	Local::RxResult rxresult;
	Local::makeRxResult(rxdata_, normstr, normlen, rxresult);
	{
		Local::RxResult::Iterator it  = rxresult.begin();
		Local::RxResult::Iterator fin = rxresult.end();
		for ( ; it != fin; ++it ) {

			// マッチ位置が含まれる形態素を取得する

			// 毎回 startWord から検索を開始しているが、本来なら tgtPos から開始すべき。
			// 第三引数から検索した文字数を減算すればよいが、急ぎのため、ほっとく。
			MorphIter findStart
				= Algorithm::findStringPosInMorph(startWord, endWord,
								(ModSize)((*it)._start - normstr));
			MorphIter findEnd
				= Algorithm::findStringPosInMorph(findStart, endWord,
								 (*it)._len);

			// 検索された形態素に対してフラグを設定する
			for ( ; findStart < findEnd; ++findStart ) {
				keyword_[start_ + findStart - startWord] = flag_;
			}

			// 検索対称位置を findEnd に飛ばす
			tgtPos = findEnd; // ++ される分
		}
	}
}

// FUNCTION pubic
// Executer::getWordTextRxForAll
//		-- 形態素表記の正規表現結果を取得する
//
// NOTES
//	正規表現文字列全てを対象とする
//
// ARGUMENTS
//	ModUnicodeRegularExpression* rx_
//		正規表現マッチクラス
//	ModVector<Common::Word>::ConstIterator startMorph_,
//		処理対象になっている形態素の開始位置
//	ModVector<Common::Word>::ConstIterator endMorph_,
//		処理対象になっている形態素の終了位置
//	const ModUnicodeChar* target_
//		正規化済文書先頭位置
//	unsigned int start_
//		処理開始位置
//	unsigned int len_
//		処理対象長さ
//	Common::Bitset& keyword_
//		キーワードを表現するビットセット
//	ModBoolean flag_
//		設定するフラグ
//
// RETURN
//	void
//
// EXCEPTIONS
//
void
Executer::getWordTextRxForAll(Type::RegularExpression& rxdata_,
			ModVector<Morph>::ConstIterator startMorph_,
			ModVector<Morph>::ConstIterator endMorph_,
			const ModUnicodeChar* targetstr_,
			unsigned int start_,
			unsigned int len_,
			Bitset& keyword_,
			ModBoolean flag_)
{
	typedef ModVector<Morph>::ConstIterator MorphIter;

	// マッチング結果を参照しながらフラグを立てる
	Local::RxResult rxresult;
	Local::makeRxResult(rxdata_, targetstr_ + start_, len_, rxresult);
	{
		Local::RxResult::Iterator it  = rxresult.begin();
		Local::RxResult::Iterator fin = rxresult.end();
		for ( ; it != fin; ++it ) {

			// マッチ位置が含まれる形態素を取得する

			// 毎回 startWord から検索を開始しているが、本来なら tgtPos から開始すべき。
			// 第三引数から検索した文字数を減算すればよいが、急ぎのため、ほっとく。
			MorphIter findStart
				= ModLowerBound(startMorph_, endMorph_, (*it)._start,
								IncludeCharFunction());

			MorphIter findEnd
				= ModLowerBound(startMorph_, endMorph_, (*it)._start + (*it)._len,
								IncludeCharFunction());
			if ( findEnd == endMorph_ ) {
				findEnd = endMorph_ - 1;
			}

			// 検索された形態素に対してフラグを設定する
			for ( ; findStart < findEnd; ++findStart ) {
				keyword_[findStart - startMorph_] = flag_;
			}
		}
	}
}

//
// Copyright (c) 2004-2009, 2023 Ricoh Company and Ltd.
// All rights reserved.
//
