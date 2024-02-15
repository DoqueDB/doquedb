// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
//  ModTermPattern.cpp -- ModTermPattern の実装
//             -- ModTermPatternFile の実装
// 
// Copyright (c) 2000, 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "ModTermPattern.h"
#include "ModTermException.h"

//
// FUNCTION public
// ModTermPatternFile::ModTermPatternFile -- コンストラクタ
//
// NOTES
//   コンストラクタ。
//   指定されたファイルからパタンを読込みベクトルにセットする。
//
// ARGUMENTS
//   const ModUnicodeString& path  ファイルパス
//
// RETURN
//   なし
//
// EXCEPTIONS
//   なし
//
ModTermPatternFile::ModTermPatternFile(
	const ModUnicodeString& path,
	const char recordSep)
{
	// パタン辞書を読込む
	ModTermStringFile stringFile(path, recordSep);

	ModTermPatternResource* pattern;

	ModTermStringFile::Iterator iterator;
	iterator = stringFile.begin();
	while(iterator != stringFile.end()) {

		// 読込んだレコードからModTermPatternResourceを生成
		try {
			pattern = new ModTermPatternResource(*iterator);
      
			// 例外発生
		} catch (ModException& exception){
			// これまで作成したパターンを削除する
			ModVector<ModTermPatternResource*>::Iterator p = this->begin();
			while(p != this->end()) {
				delete *p;
				p++;
			}
			ModRethrow(exception);
		}

		// 生成したTermPatternを追加
		this->pushBack(pattern);
		iterator++;
	}
}

//
// FUNCTION public
// ModTermPatternFile::~ModTermPatternFile -- デストラクタ
//
// NOTES
//   デストラクタ。
//   ベクトル要素(パタンへのポインタ)毎にdeleteする。
//
// ARGUMENTS
//   なし
//
// RETURN
//   なし
//
// EXCEPTIONS
//   なし
//
ModTermPatternFile::~ModTermPatternFile() {
	ModVector<ModTermPatternResource*>::Iterator p = this->begin();
	while(p != this->end()) {
		delete *p;
		p++;
	}
}

//
//	FUNCTION public
//	ModTermPatternFile::getPattern -- パターン配列を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<ModTermPattern>& pattern_
//		パターン配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ModTermPatternFile::getPattern(ModVector<ModTermPattern>& pattern_)
{
	pattern_.assign(getSize());
	ModVector<ModTermPattern>::Iterator i = pattern_.begin();
	Iterator j = begin();
	for (; j != end(); ++i, ++j)
	{
		(*i).setResource(*j);
	}
}

//
// FUNCTION public
// ModTermPatternResource::ModTermPatternResource -- コンストラクタ
//
// NOTES
//   コンストラクタ。
//
//   引数に渡されたレコードから次の各フィールド
//     id, type, weight, replacement, pattern
//   を取り出し、各属性にセットする。
//
// ARGUMENTS
//   const ModUnicodeString& record レコード
//   const ModUnicodeChar fieldSep  フィールドセパレータ)
//
// RETURN
//   なし
//
// EXCEPTIONS
//
ModTermPatternResource::ModTermPatternResource(
	const ModUnicodeString& record,
	const ModUnicodeChar fieldSep)
{
	ModUnicodeString tmp;
	ModSize count = 0;

	//
	// id を取り出す
	//
	while(1) {
		if(record[count] == fieldSep) {
			break;
		} else  {
			tmp+=record[count];
		}
		count++;
	}
	id = ModUnicodeCharTrait::toInt((const ModUnicodeChar*)tmp,10);

	// バッファをクリア
	tmp.clear();

	//
	// type を取り出す
	//
	count++;
	while(1) {
		if(record[count] == fieldSep) {
			break;
		} else  {
			tmp+=record[count];
		}
		count++;
	}
	type = ModUnicodeCharTrait::toInt((const ModUnicodeChar*)tmp,10);

	// バッファをクリア
	tmp.clear();

	//
	// weight を取り出す
	//
	count++;
	while(1) {
		if(record[count] == fieldSep) {
			break;
		} else  {
			tmp+=record[count];
		}
		count++;
	}
	weight = ModUnicodeCharTrait::toFloat((const ModUnicodeChar*)tmp);

	// バッファをクリア
	tmp.clear();

	//
	// replace を取り出す
	//
	count++;
	while(1) {
		if(record[count] == fieldSep) {
			break;
		} else  {
			tmp+=record[count];
		}
		count++;
	}
	replacement = tmp;

	// バッファをクリア
	tmp.clear();

	//
	// pattern を取り出す
	//
	// patternは fieldSepを含む事があるので
	// recordの最後まで読込む
	//
	count++;
	for(;count < record.getLength(); count++) {
		tmp+=record[count];
	}
	pattern = tmp;

	// バッファをクリア
	tmp.clear();

}

//
// FUNCTION public
// ModTermPattern::match -- パタン照合
//
// NOTES
//   対象文字列の照合開始位置に対しパタン(正規表現)の照合を行う。
//   offsetの値域はチェックはしていない。呼び出し側で管理すべし。
//
// ARGUMENTS
//   const ModUnicodeString& string 対象文字列
//   ModSize offset                 照合開始位置 (先頭からの文字数)
//
// RETURN
//   照合に成功した場合はModTrue、それ以外はModFalse。
//
// EXCEPTIONS
//   なし
//
ModBoolean
ModTermPattern::match(
	const ModUnicodeString& string,
	const ModSize offset)
{
	if (isCompile == ModFalse)
	{
		// rxHandleの取得
		try {
			handle.compile((const ModUnicodeChar*)resource->getPattern());
		} catch ( ModException& exception ) {
#ifdef DEBUG
			ModUnicodeString pattern(resource->getPattern());
			ModDebugMessage 
				<< "pattern compile failed - "
				<< pattern.getString(ModKanjiCode::literalCode) 
				<< "CODE[" << exception.getErrorNumber() << "]" << ModEndl;
#endif // DEBUG
			ModThrowTermError(ModTermErrorRxCompile);
		}
		isCompile = ModTrue;
	}
		
	return (handle.walk((const ModUnicodeChar*)string + offset, ModSize(0))
			!= 0) ? ModTrue : ModFalse;
}

//
// FUNCTION public
// ModTermPattern::replace -- 照合結果の置換
//
// NOTES
//   照合結果の置換関数。
//   ModTermPattern::match()実行後、本メソッドを行う事により、対象文字列の
//   正規表現にマッチした部分をreplacementに従い置換した文字列を生成し返す。
//
//   このメソッドで置換可能なのは、最初にマッチした箇所のみである。
//   先頭アンカー(^)を含む正規表現はマッチする個所は１つなので問題はない。
//
// ARGUMENTS
//   なし
//
// RETURN
//   置換文字列
//
// EXCEPTIONS
//   なし
//
ModUnicodeString
ModTermPattern::replace()
{
	// マッチした部分をreplacementを使って置換する
	ModUnicodeString newString;
	ModSize count=0;
	const ModUnicodeString& replacement = resource->getReplacement();
	
	while(count < replacement.getLength()) {
		// replacementを一文字ずつ見ていく
		if(replacement.at(count) != '\\') {
			// '\'以外の文字：そのまま書出す
			newString+=replacement.at(count);

		} else {
			// '\'を見つけた
			// \1 \2 .. の可能性あり
			if(ModUnicodeCharTrait::isDigit(replacement.at(count+1))==ModTrue) {
				// \1 \2 .. であった
				ModUnicodeString fukuNum;
				fukuNum += replacement.at(count+1);

				// 副表現 \x (x = fukuNum) の文字列を取り出す
				ModUnicodeChar* fukuBegin = handle.matchBegin(
					0,ModUnicodeCharTrait::toInt(fukuNum));
				ModUnicodeChar* fukuEnd = handle.matchEnd(
					0,ModUnicodeCharTrait::toInt(fukuNum));
				ModUnicodeString fuku(
					fukuBegin,
					static_cast<ModSize>(fukuEnd - fukuBegin));

				// 副表現の文字列を書出す(\x を 副表現文字列に置換える)
				newString+=fuku;
				count++;
			} else {
				// \1 \2 .. ではない：そのまま書出す
				newString+=replacement.at(count);
			}
		}
		count++;
	}
	return newString;
}

//
// Copyright (c) 2000, 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
