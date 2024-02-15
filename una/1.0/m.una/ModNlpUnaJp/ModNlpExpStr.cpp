//
// ModNlpExpStr.cpp -- Implementation of ModNlpExpStr class
// 
// Copyright (c) 2009,2010,2012, 2023 Ricoh Company, Ltd.
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

#include "ModList.h"
#include "UnaReinterpretCast.h"
#include "ModNlpUnaJp/ModNlpExpStr.h"

_UNA_USING
_UNA_UNAJP_USING

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION public
//      ModNlpExpStr::ModNlpExpStr -- constructor
//
// NOTES
//      default constructor of ModNlpExpStr
//
// ARGUMENTS
//		const ModNormalizer* const normalizer_
//			I:instance of ModNormalizer
//
// RETURN
//      none
//
// EXCEPTIONS
//      none
//
ModNlpExpStr::ModNlpExpStr(ModNormalizer* const normalizer_)
	: normalizer(normalizer_)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION public
//      ModNlpExpStr::~ModNlpExpStr -- destructor
//
// NOTES
//      default destructor of ModNlpExpStr
//
// ARGUMENTS
//      none
//
// RETURN
//      none
//
// EXCEPTIONS
//      none
//
ModNlpExpStr::~ModNlpExpStr()
{
}

// FUNCTION public
//		ModNlpExpStr::expandStrings -- get expanded character strings
//
// NOTES
// 		The searchExpandStrings function is called to search expanded 
//		strings patterns from expand data and get expanded character strings.
//
// ARGUMENTS
//		ModVector<ModUnicodeString>& formVector_
//			I:target strings
//		ModVector<ModUnicodeString>& expanded_
//			O:expanded strings patterns
//		ModSize maxExpPatternNum_
//			I:number of maximum expanded character string patterns
//		ModBoolean normFlag_
//			I:the flag of whether normalizer is on or off
//
// RETURN
//		Number of expanded strings paterns
//
// EXCEPTIONS
//
ModSize
ModNlpExpStr::expandStrings(ModVector<ModUnicodeString>& formVector_,
							ModVector<ModUnicodeString>& expanded_,
							ModSize maxExpPatternNum_,
							ModBoolean normFlag_)
{
	// search expanded strings from expand data
	return searchExpandStrings(formVector_, expanded_, maxExpPatternNum_, normFlag_);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION private
//      ModNlpExpStr::searchExpandStrings -- search expanded character string from expand data
//
// NOTES
//		Expanded strings patterns is searched from expand data and expanded character strings is obtained
//		by calling expandJoin function.
//
// ARGUMENTS
//		ModVector<ModUnicodeString>& inMrph
//			I:morpheme or character
//		ModVector<ModUnicodeString>& expanded_
//			O:expanded character strings
//		ModSize maxExpPatternNum_
//			I:number of maximum expanded character string patterns
//		ModBoolean normFlag_
//			I:the flag of whether normalizer is on or off
//
// RETURN
//		ModSize
//			number of expanded string patterns
//
// EXCEPTIONS
//		The exception from the subordinate position is returned as it is.
//
ModSize ModNlpExpStr::searchExpandStrings(
	ModVector<ModUnicodeString>& inMrph,
	ModVector<ModUnicodeString>& expanded_,
	ModSize maxExpPatternNum_,
	ModBoolean& normflag_)
{
	/* 表に格納する行objectを作成する */
	EXP_Table exp;

	/* 行objectを格納するベクタを作成する */
	ModVector<EXP_Table> expvector;

	/* 辞書引きカウンタ */
	ModSize count = 0;
	ModSize topPos = 0;

	/* 引数inMrphで与えられた形態素vector内に登録されている文字列の内最も長い文字列を登録する変数 */
	/* 初期値は空文字 */
	ModUnicodeString longStr;

	/* 最長文字列を検索する形態素集合のイテレータ inMrphIte を取得する */
	ModVector<ModUnicodeString>::Iterator inMrphIte = inMrph.begin();

	/* 検索結果した最長文字列を登録するvectorを初期化する */
	expanded_.clear();

	/* 辞書引きフラグ : 辞書にない場合を検出するためのフラグ */
	int onDicFlag = 0;

	/* 最長文字列サイズ */
	ModSize maxPos;
	ModSize maxSizeEntryPos;
	ModUnicodeString maxString;
	ModUnicodeString maxPattern;
	ModSize exp_sum(1);

	/* 最長文字列を検索する形態素集合のイテレータ*/
	ModVector<EXP_Table>::Iterator expvectorIte;

	/* 辞書引きされた展開文字列を格納するベクター */
	ModVector<ModUnicodeString> expStringVector;
	ModList< ModVector<ModUnicodeString> > expStringList;

	/* 形態素の個数分連結を繰り返し辞書引きする */
	while(topPos < inMrph.getSize()) {

		// 前の形態素組は確定したので表は削除する
		expvector.clear();

		/* 最長一致文字列の検索の実行flow:全ての形態素について辞書引き */
		for(longStr = "", count = topPos; count < inMrph.getSize(); count++){

			/* ひとつ形態素を取り出す：この形態素を先頭として以降連結する：連結しながら辞書引きする */
			longStr.append(inMrphIte[count]);

			if(normflag_ == ModTrue){
				/* 異表記正規化 */
			    normalizer->setExpStrMode(ModTrue);
				ModUnicodeString normStr = normalizer->normalizeBuf(longStr, 0, 0, ModNormalized);
				longStr = normStr;
			    normalizer->setExpStrMode(ModFalse);
			}

			// 入力文字列の()を展開パターンのデリミタと区別するために一時的に別の文字に置き換える
			longStr.replace(ModNormalizer::expandDelimiters[0], ModNormOpeningParenthesis);
			longStr.replace(ModNormalizer::expandDelimiters[2], ModNormClosingParenthesis);

			ModUnicodeString result;
			result.clear();

			/* 辞書引き */
			if((normalizer->getExpStr(longStr.getString(), result)) == ModTrue) {
				/* 辞書にある場合のフラグを立てる */
				exp.onDic = true;
			} else {
				/* 辞書にない場合のフラグを立てる */
				exp.onDic = false;
			}

			/* 表に検索した形態素を格納する */
			exp.onDicStr = longStr;
			/* 表に検索した形態素に対する展開パターンを格納する */
			exp.onDicPat = result;
			/* 表に頭の形態素格納順位を格納する */
			exp.topPos = topPos;
			/* 表に後ろの形態素格納順位を格納する */
			exp.count = count;

			expvector.pushBack(exp);
		}

		/* 最長文字列による展開パターンを検出する */
		/* 最長文字列を検索する形態素集合のイテレータにexpvectorの先頭を設定する */
		expvectorIte = expvector.begin();

		maxPos = 0;
		maxSizeEntryPos = 0;
		maxString = "";
		maxPattern = "";

		while(expvectorIte != expvector.end()){
			if (((EXP_Table)(*expvectorIte)).onDic) {
				/* 辞書引きフラグをonにする：今回ひとつ以上辞書登録されていたことを示す */
				onDicFlag ++;

				maxPos = ((EXP_Table)(*expvectorIte)).count;

				if(maxPos >= maxSizeEntryPos) {
					maxSizeEntryPos = maxPos;
					maxString = ((EXP_Table)(*expvectorIte)).onDicStr;
					maxPattern = ((EXP_Table)(*expvectorIte)).onDicPat;
				}
			}

			*expvectorIte++;
		}

		/* 展開語を登録する */
		if (onDicFlag < 1) { /* 辞書に一つも登録されていなかった場合 展開語がないので元の語をセット */
			inMrphIte = inMrph.begin();
			maxString = inMrphIte[topPos];
			maxSizeEntryPos = topPos;

			/* expStringVectorに格納 */
			expStringVector.clear();
			expStringVector.pushBack(maxString);

			/* listに格納 */
			expStringList.pushBack(expStringVector);

		} else {	/* 検索した最長文字列の展開語を登録する */
			/* 展開パターンを_expand_arrayに渡すフォームに整形する */
			maxPattern.replace(ModNormalizer::expandDelimiters[0], ModNormDefaultDelimiter0);
			maxPattern.replace(ModNormalizer::expandDelimiters[1], ModNormDefaultDelimiter1);
			maxPattern.replace(ModNormalizer::expandDelimiters[2], ModNormDefaultDelimiter2);

			ModUnicodeString* expandString = 0;

			// 一時的に別の文字に置き換えた()を元に戻す
			maxPattern.replace(ModNormOpeningParenthesis,ModNormalizer::expandDelimiters[0]);
			maxPattern.replace(ModNormClosingParenthesis,ModNormalizer::expandDelimiters[2]);

			ModSize exp_cnt = normalizer->_expand_array(maxPattern, expandString);

			expStringVector.clear();
			for (ModSize i = 0; i < exp_cnt; ++i){
				expStringVector.pushBack(expandString[i]);
			}

			delete [] expandString,expandString=0;

			/* listへの格納 */
			expStringList.pushBack(expStringVector);

			/* 展開パターンの総数を算出する */
			exp_sum *= exp_cnt;
			if(exp_sum >= maxExpPatternNum_){
				return 0;
			}
		}

		/* 次の検索位置をセットする */
		topPos = maxSizeEntryPos + 1;

		/* 辞書引きフラグをリセットする */
		onDicFlag = 0;
	}

	expandJoin(expStringList, expanded_);

	/* 最後にexpStringVector詰めた全要素を削除する */
	expStringVector.clear();

	/* listの全要素を破棄する */
	expStringList.clear();

	/* 展開パターンの総数を返す */
	return exp_sum;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION private
//      ModNlpExpStr::expandJoin --  Put a expanded string patterns together and make character string.
//
// NOTES
//		The expanded string pattern is combined and the expanded character string is generated.
//
// ARGUMENTS
//		ModVector<ModUnicodeString>& in_
//			I:list that stores expanded strings patterns
//		ModVector<ModUnicodeString>& expanded_
//			O:expanded character string
//
// RETURN
//		ModBoolean
//			ModTrue	 : the acquisition of the result succeeds
//			ModFalse : no input expanded string patterns
//
// EXCEPTIONS
//
ModBoolean ModNlpExpStr::expandJoin(
	ModList< ModVector<ModUnicodeString> > in_,
	ModVector<ModUnicodeString>& expanded_)
{
	ModVector<ModUnicodeString> fromToJoin;
	ModVector<ModUnicodeString> fromStr, toStr;
	ModSize count = 0;

	ModList< ModVector<ModUnicodeString> >::Iterator fromIte = in_.begin();
	ModVector<ModUnicodeString>::Iterator listStringIte = (*fromIte).begin();

	ModSize listNum = in_.getSize();
	if(listNum == 0){
		return ModFalse;
	} else if (listNum == 1){
		while (fromIte != in_.end()) {
			while (listStringIte != (*fromIte).end()){
				if((*fromIte).getSize() > 0){
					expanded_.pushBack((*listStringIte));
				}
				*listStringIte++;
			}
			*fromIte++;
		}
		return ModTrue;
	}

	while (listStringIte != (*fromIte).end()) {
		if ((*fromIte).getSize() > 0) {
			fromStr.pushBack((*listStringIte));
		}
		*listStringIte++;
	}

	ModUnicodeString str;
	while (fromIte != in_.end()) {
		ModList< ModVector<ModUnicodeString> >::Iterator to = fromIte + 1;
		ModVector<ModUnicodeString>::Iterator fromStrIte = fromStr.begin();
		toStr.clear();
		while (fromStrIte != fromStr.end()) {

			if ((*to).getSize() != 0) {
				ModVector<ModUnicodeString>::Iterator toListIte = (*to).begin();
				while (toListIte != (*to).end()) {
					if ((*to).getSize() > 0) {
						str =  (*fromStrIte)+(*toListIte);
						toStr.pushBack(str);
					}
					*toListIte++;
				}
			}
			*fromStrIte++;
		}

		ModVector<ModUnicodeString>::Iterator toStrIte = toStr.begin();
		fromStr.clear();
		while (toStrIte != toStr.end()) {
			fromStr.pushBack((*toStrIte));
			*toStrIte++;
		}
		
		/* 次の文字列 */
		*fromIte++;

		if ((++count) > (listNum-2)) {
			ModVector<ModUnicodeString>::Iterator outStrIte = fromStr.begin();

			while (outStrIte != fromStr.end()) {
				expanded_.pushBack((*outStrIte));
				*outStrIte++;
			}
			break;
		}
	}

	return ModTrue;
}
//
// Copyright (c) 2009,2010,2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
