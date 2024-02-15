// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedBlockedNgramTokenizer.cpp -- ブロック化 Ngram 分割器の実装
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#include "UNA_UNIFY_TAG.h"
    
#include "ModAlgorithm.h"
#include "ModCharString.h"
#include "ModUnicodeString.h"
#include "ModOstrStream.h"

#include "ModInvertedException.h"
#include "ModInvertedBlockedNgramTokenizer.h"
#include "ModInvertedUnicodeCharBlocker.h"

//
// CONST
// ModInvertedBlockedNgramTokenizer::tokenizerName -- 分割器の名称
//
// NOTES
// ModInvertedBlockedNgramTokenizer 分割器の名称を表す
//
/*static*/
const char ModInvertedBlockedNgramTokenizer::tokenizerName[] = "BNG";

//
// CONST
// ModInvertedBlockedNgramTokenizer::blockName -- 文字種ラベル
//
// NOTES
// 文字種を表すラベルである。
// 
/*static*/ const char* 
ModInvertedBlockedNgramTokenizer::blockName[]
	= { "OTH", "ASC", "SYM", "DIG", "ALP", "HIR", "KAT",
		"GRK", "RUS", "KEI", "KAN", "HAN", "GAI" };


//
// FUNCTION
// ModInvertedBlockedNgramTokenizer::setPair -- 有効文字組の設定
//
// NOTES
// 有効文字組（トークナイズにおいて抽出すべき文字種からなる文字組）を
// 設定する。
//
// ARGUMENTS
// const ModSize first
//		１文字目の文字種
// const ModSize second
//		２文字目の文字種
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedBlockedNgramTokenizer::setPair(const ModSize first,
										  const ModSize second)
{
	validPair[first*blockNum + second] = ModTrue;
}

//
// FUNCTION
// ModInvertedBlockedNgramTokenizer::checkPair -- 有効文字組の検査
//
// NOTES
// 有効文字組か否かを検査する。
//
// ARGUMENTS
// const ModSize first
//		１文字目の文字種
// const ModSize second
//		２文字目の文字種
//
// RETURN
// 有効文字組であれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedBlockedNgramTokenizer::checkPair(const ModSize first,
											const ModSize second) const
{
	return validPair[first*blockNum + second];
}

//
// FUNCTION
// ModInvertedBlockedNgramTokenizer::ModInvertedBlockedNgramTokenizer
//		-- コンストラクタ
//
// NOTES
// ブロック化 Ngram 分割器をコンストラクトする。
//
// ARGUMENTS
// const ModCharString& description_
//		パラメータ記述
// ModInvertedFile* file_
//		転置ファイル
// const ModBoolean normalizing_
//		正規化指示
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModInvertedBlockedNgramTokenizer::ModInvertedBlockedNgramTokenizer(
	const ModCharString& description_,
	ModInvertedFile* file_,
	const ModBoolean normalizing_)
	: blocker(0), ModInvertedNgramTokenizer(0, 0, file_, ModFalse)
{
	// normalizer の生成の都合から NgramTokenizer の正規化指定には false を
	// 渡し、ここで正規化指定を直接設定する
	normalizing = normalizing_;
	normRscID = getResourceID(description_, normalizerResourceID);

	try {
		blocker = ModInvertedUnicodeCharBlocker::create();
		blockNum = blocker->getBlockNum();

		parse(description_);

	} catch (ModException& e) {
		delete blocker;
		// normalizer は NgramTokenizer のデストラクタで delete される
		// delete normalizer;	
		ModRethrow(e);
	}
}

//
// FUNCTION
// ModInvertedBlockedNgramTokenizer::~ModInvertedBlockedNgramTokenizer
//		-- デストラクタ
//
// NOTES
// ブロック化 Ngram 分割器をデストラクトする。
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
ModInvertedBlockedNgramTokenizer::~ModInvertedBlockedNgramTokenizer()
{
	delete blocker;
}

//
// FUNCTION
// ModInvertedBlockedNgramTokenizer::set -- 処理対象テキストのセット
//
// NOTES
// 処理対象のテキストをセットする
//
// ARGUMENTS
// const ModUnicodeString& target_
//		処理対象テキスト
// const TokenizeMode mode_
//		処理モード
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
ModInvertedBlockedNgramTokenizer::set(const ModUnicodeString& target_,
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

		tailOffset = 0;			// 末尾部分（文書の先頭に近い側）
		headOffset = 0;			// 先頭部分（文書の末尾に近い側)
		nextOffset = 0;

		if (targetLength == 0) {
			// 空文字列の場合 -- つぎの yield ですぐに false が返るようにする
			isShortWord = ModTrue;
			currentLength = 0;
			return;
		}

		isShortWord = ModFalse;

		// 先頭の文字種だけはここで判断しておく必要がある
		// - はじめに切り出す文字列の長さが決まらない
		tailBlock = blocker->getBlock((*target)[tailOffset]);
		headBlock = tailBlock;
		maxLength = blockMaxLength[tailBlock];
		minLength = blockMinLength[tailBlock];
		isNewBlock = ModTrue;
		pairValid = ModFalse;
		currentLength
			= ((mode&baseModeMask) == document) ? minLength : maxLength;

		ModBoolean change(ModFalse);

		// 先頭位置を進めておく
		if (currentLength > targetLength) {
			// 長さが足りない場合
			while (++headOffset < targetLength) {
				headBlock = blocker->getBlock((*target)[headOffset]);
				if (headBlock != tailBlock) {
					// 末尾に達する前に文字種が変わった
					currentLength = headOffset;
					currentMaxLength = headOffset;
					change = ModTrue;
					break;
				}
			}
			if (change == ModFalse) {
				// 末尾までが同一文字種だった場合
				currentMaxLength = targetLength;
				if ((mode&baseModeMask) != document) {
					if (minLength > targetLength) {
						// MIN よりも検索文字列が短ければショートワード処理
						isShortWord = ModTrue;
					}
					currentLength = currentMaxLength;
				}
			}
		} else {
			while (++headOffset < currentLength) {
				headBlock = blocker->getBlock((*target)[headOffset]);
				if (headBlock != tailBlock) {
					// 現在長に達する前に文字種が変わった
					currentLength = headOffset;
					currentMaxLength = headOffset;
					change = ModTrue;
					break;
				}
			}
			if (change == ModFalse) {
				// 現在長までが同一文字種だった場合
				// 現在最大長をとりあえず最大長にしておく
				currentMaxLength = maxLength;
				ModSize tmpOffset(headOffset);
				while (tmpOffset < maxLength) {
					if (tmpOffset == targetLength ||
						blocker->getBlock((*target)[tmpOffset]) != tailBlock) {
						// 最大長に達する前に文字種が変わる
						currentMaxLength = tmpOffset;
						break;
					}
					++tmpOffset;
				} // ここでチェックした文字種はあとでチェックし直すので無駄
			}
		}

		if (currentMaxLength == 1 &&
			checkPair(tailBlock, headBlock) == ModTrue &&
			targetLength > 1) {
			currentMaxLength = 2;
			pairValid = ModTrue;
		}

	} catch (ModException& exception) {
		ModErrorMessage << "set failed: target=" << *target_
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
// ModInvertedTokenizer::tokenizeSub -- テキスト分割の下請
//
// NOTES
// 渡されたテキスト（文字列）をモードにしたがって分割する。
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
//		ショートワード
// ModUnicodeString& from_
//		展開すべき表記の先頭
// ModUnicodeString& to_
//		展開すべき表記の末尾
//
// RETURN
// ショートワード処理が不要ならば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// 下位からの例外はそのまま再送出する
//
ModBoolean
ModInvertedBlockedNgramTokenizer::tokenizeSub(
	const ModUnicodeString& target_,
	const TokenizeMode mode_,
	ModInvertedLocationListMap& result_,
	ModSize& tokenizedEnd_,
	ModUnicodeString& shortWord_,
	ModUnicodeString& from_,
	ModUnicodeString& to_
#ifdef V1_6
	, const ModLanguageSet& langSet_
#endif // V1_6
	)
{
	try {
		// 古い結果をクリアする
		result_.erase(result_.begin(), result_.end());
		pairValid = ModFalse;

#ifdef V1_6
		ModVector<ModLanguageSet> langSets;
		langSets.pushBack(langSet_);
#endif // V1_6

		// まず分割してみる
		ModInvertedTokenizer::tokenize(target_, mode_, result_, tokenizedEnd_,
									   0, 0
#ifdef V1_6
									   , &langSets
#endif // V1_6
									   , 0, 0);

		if (isShortWord == ModTrue && headOffset > 0) {
			// short word の処理 -- from, to をセットする
			// short word は同一文字種で構成される
			// ただし、空文字列は除外する
			; ModAssert(result_.getSize() == 0);
			; ModAssert(currentLength > 0);

			tokenizedEnd_ = tailOffset + 1;

			target->copy(shortWord_, tailOffset, currentLength);

			ModUnicodeChar beginChar, endChar;
			blocker->getBlockRegion(headBlock, beginChar, endChar);

			from_ = shortWord_;
			from_ += beginChar;
			to_ = shortWord_;
			to_ += endChar;
#ifdef DEBUG0
			ModDebugMessage << "SHORT:" << shortWord_ << ","
							<< from_ << "," << to_ << ":" << headBlock << ' '
							<< beginChar << ' ' << ModHex << int(beginChar) << ' '
							<< endChar << ' ' << ModHex << int(endChar)
							<< ModEndl;
#endif
			return ModFalse;
		}

		// 通常の処理 -- そのまま返ればよい
		; ModAssert(tokenizedEnd_ == headOffset);
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
// ModInvertedBlockedNgramTokenizer::yield -- 新たなトークンの出力
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
ModInvertedBlockedNgramTokenizer::yield(ModUnicodeString& token,
										Occurrence& occurrence)
{
	ModSize tmpBlock;

	if (currentLength == 0 || isShortWord == ModTrue) {
		return ModFalse;
	} else if (currentLength < currentMaxLength) {
		target->copy(token, tailOffset, currentLength);
		if (pairValid == ModTrue) {
			; ModAssert(currentMaxLength == 2 && currentLength == 1);
		}
		++currentLength;
		++headOffset;
		occurrence.setOffset(tailOffset);
		isNewBlock = ModFalse;
		return ModTrue;
	}
	else if ((mode&baseModeMask) != document && isNewBlock == ModFalse &&
			 currentLength < maxLength && headOffset == targetLength) {
		return ModFalse;
	}
	else {
		target->copy(token, tailOffset, currentLength);
		occurrence.setOffset(tailOffset);
		if (pairValid == ModTrue && currentLength == 2) {
			; ModAssert(currentMaxLength == 2);
		}

		if (currentLength == currentMaxLength && tailBlock != headBlock) {
			// 最大長になっているので、文字種が変わる
			if (((mode&baseModeMask) == document)) {
				++tailOffset;
			} else {
				// ※ query モードで異文字種組を利用するにはここを修正する
				//    必要がある
				if (pairValid == ModTrue) {
					++tailOffset;
				} else {
					tailOffset += currentLength;
				}
			}
			isNewBlock = ModFalse;
			tmpBlock = blocker->getBlock((*target)[tailOffset]);
			if (tailBlock != tmpBlock) {
				tailBlock = tmpBlock;
				isNewBlock = ModTrue;
			}
			maxLength = blockMaxLength[tailBlock];
			minLength = blockMinLength[tailBlock];
			if (tailOffset >= targetLength) {
				// 末尾に達した -- 現在のトークンは出力する
				currentLength = 0;
				return ModTrue;
			}
		} else {
			isNewBlock = ModFalse;
			++tailOffset;
			tmpBlock = blocker->getBlock((*target)[tailOffset]);
			if (tailBlock != tmpBlock) {
				tailBlock = tmpBlock;
				isNewBlock = ModTrue;
			}
			maxLength = blockMaxLength[tailBlock];
			minLength = blockMinLength[tailBlock];
			if (tailOffset >= targetLength) {
				// 末尾に達した -- 現在のトークンは出力する
				currentLength = 0;
				return ModTrue;
			}
		}
		pairValid = ModFalse;

	retry:
		// 文字種の変り目は 1 にしないといけない
		currentLength =
			((mode&baseModeMask) == document) ?
			(isNewBlock == ModTrue) ? 1 : minLength : maxLength;

		headOffset = tailOffset;
		ModBoolean change(ModFalse);

		// 先頭位置を進めておく
		if (tailOffset + currentLength > targetLength) {
			// 長さが足りない場合
			while (++headOffset < targetLength) {
				headBlock = blocker->getBlock((*target)[headOffset]);
				if (headBlock != tailBlock) {
					// 末尾に達する前に文字種が変わった
					currentLength = headOffset - tailOffset;
					currentMaxLength = headOffset - tailOffset;
					change = ModTrue;
					break;
				}
			}
			if (change == ModFalse) {
				// 末尾までが同一文字種だった場合
				currentMaxLength = targetLength - tailOffset;
				currentLength = currentMaxLength;
			}
		} else {
			while (++headOffset < tailOffset + currentLength) {
				headBlock = blocker->getBlock((*target)[headOffset]);
				if (headBlock != tailBlock) {
					// 現在長に達する前に文字種が変わった
					currentLength = headOffset - tailOffset;
					currentMaxLength = headOffset - tailOffset;
					if ((mode&baseModeMask) != document &&
						isNewBlock == ModFalse) {
						// 同一文字種で短いものなので、検索時は不要
						// ただし、異文字種ペアをとる場合にはさらに修正が必要
						tailOffset = headOffset;
						tmpBlock = blocker->getBlock((*target)[tailOffset]);
						if (tailBlock != tmpBlock) {
							tailBlock = tmpBlock;
							isNewBlock = ModTrue;
						}
						maxLength = blockMaxLength[tailBlock];
						minLength = blockMinLength[tailBlock];
						isNewBlock = ModTrue;
						goto retry;
					}
					change = ModTrue;
					break;
				}
			}
			if (change == ModFalse) {
				// 現在長までが同一文字種だった場合
				// 現在最大長をとりあえず最大長にしておく
				currentMaxLength = maxLength;
				ModSize tmpOffset(headOffset);
				// headOffset は文字種のチェックをしていない
				while (tmpOffset < tailOffset + maxLength) {
					if (tmpOffset == targetLength ||
						blocker->getBlock((*target)[tmpOffset]) != tailBlock) {
						// 最大長に達する前に文字種が変わる
						currentMaxLength = tmpOffset - tailOffset;
						break;
					}
					++tmpOffset;
				} // ここでチェックした文字種はあとでチェックし直すので無駄
			}
		}

		if (currentMaxLength == 1 &&
			checkPair(blocker->getBlock((*target)[tailOffset]),
					  blocker->getBlock((*target)[headOffset])) == ModTrue &&
			headOffset < targetLength) {
			; ModAssert(headOffset == tailOffset + 1);
			currentMaxLength = 2;
			pairValid = ModTrue;
		}
		return ModTrue;
	}
}

//
// FUNCTION
// ModInvertedBlockedNgramTokenizer::parse -- 記述文字列の解析
//
// NOTES
// 記述文字列を解析、それを対応したメンバに設定する。
// 記述法は以下の通り。
//
//		<記述> ::= <文字種判定器>:<文字種記述>* <文字種連接記述>* [<異表記正規化器リソース指定>]
//		<文字種判定器> ::= JAP
//		<文字種記述> ::= <文字種>:<切り出し文字数> | 
//						 <文字種>:<最小切り出し文字数>:<最大切り出し文字数>
//		<文字種> ::= ALL | ALP | DIG | SYM | ASC | HIR | KAT | KAN |GRK | RUS |
//					 KEI | HAN | GAI | OTH
//				なお	ALL		全ての文字種
//						ALP		全角アルファベット
//						DIG		全角数字
//						SYM		全角記号
//						ASC		アスキー
//						HIR		全角ひらがな
//						KAT		全角カタカナ
//						KAN		全角漢字
//						GRK		全角ギリシャ文字
//						RUS		全角ロシア文字
//						KEI		全角罫線素
//						HAN		半角カタカナ
//						GAI		全角外字
//						OTH		上記以外（EXC文字など）
//		<切り出し文字数> ::= 1以上8以下の整数
//		<最小切り出し文字数> ::= 1以上8以下の整数
//		<最大切り出し文字数> ::= 1以上8以下の整数
//				ただし、<最小切り出し文字数>以上であること
//		<文字種連接記述> ::= <文字種>:<文字種>
//				ただし、前後の文字種が同一であるものは無視される
//		<異表記正規化器リソース指定> ::= <異表記正規化器リソースキー>:<リソース番号>
//		<異表記正規化器リソースキー> ::= @NORMRSCID
//		<リソース番号> ::= 0以上の整数
//
// デフォルトでは、すべての文字種について 2 文字単位に切り出し、
// リソース番号は 0 とする。
// 異表記正規化器リソースキーは ModInvertedTokenizer.cpp で定義されている。
// 異表記正規化器リソース指定は異表記正規化時にのみ有効。
// なお、記述順に値は設定される。
// 記述例を以下に示す。
//
//		eg.1 "JAP:ALL:2 KAT:3 KAN:1:2"
//				カタカナは 3 文字単位、漢字は 1 文字および 2 文字単位、
//				それ以外は 2 文字単位に切り出す。
//				異表記正規化器リソース番号はデフォルト通り 0。
//		eg.2 "JAP:ALL:1 KAN:HIR HIR:KAN"
//				すべての文字種について 1 文字単位に切り出す。漢字／ひらがな
//				およびひらがな／漢字の連続も切り出す。
//		eg.3 "JAP:@NORMRSC:5"
//				デフォルト通り、全ての文字種について 2 文字単位に切り出す。
//				異表記正規化器リソース番号は 5。
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
ModInvertedBlockedNgramTokenizer::parse(const ModCharString& description)
{
	// 初期化 - デフォルトでは min/max = 2 で、文字組は使用しない
	blockMinLength.clear();
	blockMinLength.insert(blockMinLength.begin(), blockNum, 2);
	blockMaxLength.clear();
	blockMaxLength.insert(blockMaxLength.begin(), blockNum, 2);
	validPair.clear();
	validPair.insert(validPair.begin(), blockNum*blockNum, ModFalse);

	// blocker 名称を切り出す -- 当面は JAP 固定とする
	ModSize	length(description.getLength());
	if (description.compare("JAP", 3) == 0) {
		if (length == 3) {
			return;
		}
		; ModAssert(length > 3);
		if (description[3] != ':' && description[3] != ' ') {
			ModErrorMessage << "invalid blocker name: "
							<< description << ModEndl;
			ModThrowInvertedFileError(
				ModInvertedErrorInvalidTokenizerParameterDescription);
		}
	} else {
		ModErrorMessage << "invalid blocker name: " << description << ModEndl;
		ModThrowInvertedFileError(
			ModInvertedErrorInvalidTokenizerParameterDescription);
	}
	// 残りのパラメータを解析する

	ModSize	index(4);			// length("JAP") + 1
	ModCharString buffer;		// 切り出した記述
	ModCharString token[3];		// bufferから切り出した単語

	while (index <= length) {
		// スペースを記述の区切りと見なす．
		if (description[index] == ' ' || index == length) {

			if (buffer.getLength() == 0 || buffer[0] == '@') {
				// トークンの長さが 0, あるいは @ で始まってれば無視
				++index;
				continue;
			}

			// <文字種記述> <文字種連接記述> の処理
			ModInt32 setCharType1(ModSizeMax);	// 文字種1
			ModInt32 setCharType2(ModSizeMax);	// 文字種2 <文字種連接記述>の場合のみ
			ModInt32 minValue(ModSizeMax);		// 最小切り出し文字数(切り出し文字数)
			ModInt32 maxValue(ModSizeMax);		// 最大切り出し文字数

			ModSize countWord(divideIntoToken(token, buffer));
			if (countWord < 2) {
				// 少なくとも２トークンは指定されているはず
				ModErrorMessage << "invalid token: "
								<< buffer << ModEndl;
				ModThrowInvertedFileError(
					ModInvertedErrorInvalidTokenizerParameterDescription);
			}

			// 文字種1取得
			setCharType1 = getCharType(token[0]);
			if (setCharType1 == ModSizeMax) {	
				// 1文字目は必ず文字種のためこれはエラー
				ModErrorMessage << "invalid charType: "
								<< token[0] << ModEndl;
				ModThrowInvertedFileError(
					ModInvertedErrorInvalidTokenizerParameterDescription);
			}

			// 文字種2取得
			setCharType2 = getCharType(token[1]);
			if (setCharType2 == ModSizeMax) {
				// 文字数の指定
				minValue = token[1].toInt();
				if (minValue < 1 || 8 < minValue) {
					// minValue が不正な値
					ModErrorMessage << "invalid minValue: "
									<< token[1] << ModEndl;
					ModThrowInvertedFileError(
						ModInvertedErrorInvalidTokenizerParameterDescription);
				}

				if (countWord == 3) {
					// maxValue が指定されている
					maxValue = token[2].toInt();
					if (maxValue < minValue || 8 < maxValue) {
						// maxValue が不正な値
						ModErrorMessage << "invalid maxValue: "
										<< token[2] << ModEndl;
						ModThrowInvertedFileError(
							ModInvertedErrorInvalidTokenizerParameterDescription);
					}
				} else {
					// maxValue が指定されていない
					maxValue = minValue;
				}
			} else {
				// 文字種連接の指定
				if (countWord != 2) {
					// 余計なものが指定されている
					ModErrorMessage << "invalid charPair: " << buffer
									<< ModEndl;
					ModThrowInvertedFileError(
						ModInvertedErrorInvalidTokenizerParameterDescription);
				}
			}

			// 実際に値をセット
			setArgument(setCharType1, setCharType2, minValue, maxValue);

			// 処理の後変数をクリアする必要あり．
			buffer.clear();
		}
		else {
			buffer += description[index];
		}
		++index;
	}

#ifdef DEBUG0
	{
		for (ModSize x(0); x < blockNum; ++x) {
			ModDebugMessage << x << ' ' << blockName[x] << ' '
							<< blockMinLength[x] << ' ' << blockMaxLength[x]
							<< ModEndl;
		}
	}
#endif
}

//
// FUNCTION
// ModInvertedBlockedNgramTokenizer::getCharType
//
// NOTES
// 文字種(文字列)の指す文字種番号(ModInvertedJapanizer.h参照)を返す．
//	
// ARGUMENTS
// const ModString& stringCharType)
//		文字種文字列
//
// RETURN
// 文字種番号
//		blockNum の場合、すべての文字種を意味する
//		ModSizeMax の場合、存在しない文字種を意味する
//
// EXCEPTIONS
// ModInvertedErrorInvalidTokenizerParameterDescription
//		パラメータ記述が不正である
//
int
ModInvertedBlockedNgramTokenizer::getCharType(
	const ModCharString& stringCharType)
{
	for (int i = 0; i < static_cast<int>(blockNum); ++i) {
		if (stringCharType.compare(blockName[i]) == 0) {
			return i;
		}
	}
	if (stringCharType.compare("ALL") == 0) {
		// 全ての文字種(blockNumを返すことにする)
		return blockNum;
	}

	// 文字種以外の場合
	return ModSizeMax;
}


//
// FUNCTION
// ModInvertedBlockedNgramTokenizer::divideIntoToken
//
// NOTES
// 文字列から区切り文字':'で区切られた単語を取出す．
//
// ARGUMENTS
// ModCharString* word
//		切り出した単語の格納先
// const ModCharString& description
//		元の文字列
//
// RETURN
// 切り出した文字数(最大3単語まで)
//
// EXCEPTIONS
// ModInvertedErrorInvalidTokenizerParameterDescription
//		パラメータ記述が不正である
//
ModSize
ModInvertedBlockedNgramTokenizer::divideIntoToken(
	ModCharString* word,
	const ModCharString& description)
{
	ModSize	count(0);
	ModSize	index(0);
	ModSize length(description.getLength());

	word[0].clear();
	while (index < length) {	// 単語切り出し
		if (description[index] == ':') {
			if (++count >= 3) {
				//	記述エラー 単語が3以上
				ModErrorMessage << "too many tokens: " << description << ModEndl;
				ModThrowInvertedFileError(
					ModInvertedErrorInvalidTokenizerParameterDescription);
			}		
			word[count].clear();
		}
		else {
			word[count] += description[index];
			
		}
		++index;
	}
	return ++count;
}

//
// FUNCTION
// ModInvertedBlockedNgramTokenizer::setArgument
//
// NOTES
//	切り出し文字数(minValueおよびmaxValue),最小切り出し文字数(minValue)
//	最大切り出し文字数(maxValue) 文字種連接記述(validPair)のセット
//
// ARGUMENTS
// const ModSize 	setCharType1
//		文字種1
// const ModSize	setCharType2
//		文字種2 （<文字種連接記述>の場合のみ使用）
// const ModSize	minValue
//		最小切り出し文字数(切り出し文字数)
// const ModSize	maxValue
//		最大切り出し文字数
//
// RETURN
// なし
//
// EXCEPTIONS
// ModInvertedErrorInvalidTokenizerArgument
//		パラメータ記述が不正である
//
void
ModInvertedBlockedNgramTokenizer::setArgument(
	const ModSize 	setCharType1,
	const ModSize	setCharType2,
	const ModSize	minValue,
	const ModSize	maxValue)
{
	ModSize index, index2;

	if (setCharType2 != ModSizeMax) {
		// <文字種連接記述>
		if (setCharType1 == blockNum) {
			// ALL
			for (index = 0; index < blockNum; ++index) {
				if (setCharType2 == blockNum) {
					for (index2 = 0; index2 < blockNum; ++index2) {
						if (index != index2) {
							setPair(index, index2);
						}
					}
				}
				else if (index != setCharType2) {
					; ModAssert(setCharType2 < blockNum);
					setPair(index, setCharType2);
				}
			}
		} else {
			if (setCharType2 == blockNum) {
				for (index2 = 0; index2 < blockNum; ++index2) {
					if (setCharType1 != index2) {
						setPair(setCharType1, index2);
					}
				}
			}
			else if (setCharType1 != setCharType2) {
				; ModAssert(setCharType2 < blockNum);
				setPair(setCharType1, setCharType2);
			}
		}
	}
	else {
		// <文字種記述>
		; ModAssert(minValue > 0 && maxValue > 0);
		if (setCharType1 == blockNum) {
			// ALL
			for (index = 0; index < blockNum; index++) {
				blockMinLength[index] = minValue;
				blockMaxLength[index] = maxValue;
			}
		}
		else {
			blockMinLength[setCharType1] = minValue;
			blockMaxLength[setCharType1] = maxValue;
		}
	}
}

//
// FUNCTION
// ModInvertedBlockedNgramTokenizer::getDescription -- 分割器記述の取得
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
ModInvertedBlockedNgramTokenizer::getDescription(
	ModCharString& description_,
	const ModBoolean withName_) const
{
	description_.clear();
	if (withName_ == ModTrue) {
		description_.append(tokenizerName);
		description_.append(':');
	}

	ModOstrStream stream;
	stream << "JAP" << ':';		// とりあえず JAP は固定
	for (ModSize i(0); i < blockNum; ++i) {
		if (i != 0)
			stream << ' ';
		stream << blockName[i] << ':'
			   << blockMinLength[i] << ':' << blockMaxLength[i];
		for (ModSize j(0); j < blockNum; ++j) {
			if (checkPair(i, j) == ModTrue) {
				stream << ' ' << blockName[i] << ':' << blockName[j];
			}
		}
	}
	if (normalizing == ModTrue || normRscID != 0) {
		stream << ' ' << normalizerResourceID << ':' << normRscID;
	}
	description_.append(stream.getString());
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
