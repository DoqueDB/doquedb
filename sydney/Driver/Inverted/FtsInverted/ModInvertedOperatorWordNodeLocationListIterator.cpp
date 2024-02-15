// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedOperatorWordNodeLocationListIterator.cpp -- WordNode用文書内出現位置リストの反復子
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2006, 2008, 2010, 2023 Ricoh Company, Ltd.
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

#ifdef V1_4	 // 単語単位検索

#include "ModInvertedOperatorWordNodeLocationListIterator.h"
#include "ModAutoPointer.h"

//
// FUNCTION
// ModInvertedOrderedOperatorWindowLocationListIterator::initialize -- 初期化
//
// NOTES
// WordNode(単語検索用ノード)用位置反復子を生成する
//
// ARGUMENTS
// LocationIterator* _termLoc
//		子ノードの位置反復子
// LocationIterator* _emptyStringLoc
//		空集合ノードの位置反復子
// ModInvertedTermMatchMode mode
//		マッチモード
// ModSize _wordLength
//		もととなった検索語の長さ
// ModInvertedSmartLocationList* emptyLoationList
//		検索語内区切り文字位置情報(exactWordModeで使用する)
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedOperatorWordNodeLocationListIterator::initialize(
	LocationIterator* termLoc_,
	LocationIterator* emptyStringLoc_)
{
	termLoc = termLoc_;
	emptyStringLoc = emptyStringLoc_;
#ifdef USE_LOWER
	isEndStatus = ModFalse;
#endif
	
	// tokenBoundaryが0でもsimpleWordModeへの変換は行わないようにした
	// rawNextで tokenBoundary をチェックする

	// マッチする最初の位置に移動
	rawNext();

#ifdef DEBUG
	if ((matchMode == ModInvertedTermExactWordMode
#ifndef MOD_DIST // APPMODE
		 || matchMode == ModInvertedTermApproximateMode
#endif
		) && tokenBoundary != 0) {
		ModInvertedLocationListIterator* p = tokenBoundary->begin();

		ModDebugMessage << "Boundaries {";
		while(p->isEnd() == ModFalse) {
			ModDebugMessage << p->getLocation() << " ";
			p->next();
		}
		ModDebugMessage << "}" << ModEndl;
		delete p;
	} else {
		ModDebugMessage << "Boundaries {no  Boundaries(shortWord)}";
	}
#endif // DEBUG
}

//
// FUNCTION
// ModInvertedOrderedOperatorWindowLocationListIterator::rawNextExact
//		-- 次の位置に進む
//
// NOTES
// 現在位置から、距離の制約条件を満たす次の位置に自分を進める。
// 現在位置が距離の制約条件を満たしている場合は動かない。
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
#ifdef USE_LOWER
void
ModInvertedOperatorWordNodeLocationListIterator::rawNextExact()
{
	if (tokenBoundary == 0) {
		// exactWordModeで検索語内単語境界情報がセットされていない
		// これはショートワードのケース
		// この場合はsimpleWordModeと同じように先頭と終端のみチェックする
		// だけで良い
		rawNextSimple();
		return;
	}

	ModSize termCurrentLoc;

	while (termLoc->isEnd() == ModFalse) {

		termCurrentLoc = termLoc->getLocation();

		if (emptyStringLoc->isEnd() == ModTrue) {
			// 最後に達したのでリセットする
			emptyStringLoc->reset();
		}

		if (emptyStringLoc->find(termCurrentLoc) == ModTrue) {
			// 現在の位置と区切り文字の位置が一致した - マッチする可能性がある

			ModSize termEnd(termCurrentLoc + wordLength);
			ModBoolean boundaryCheck(ModTrue);

			// つぎの while で (termCurrentLoc - 1) を毎回するのは無駄
			--termCurrentLoc;

			try {
				if (boundary == 0)
					boundary = tokenBoundary->begin();
				else
					boundary->reset();

				while (boundary->isEnd() == ModFalse) {
					if (emptyStringLoc->getLocation() != 
						termCurrentLoc + boundary->getLocation()/* - 1*/) {
						// 境界が一致しない
						boundaryCheck = ModFalse;
						break;
					}

					boundary->next();
					if(boundary->isEnd() == ModTrue) {
						break;
					}
					emptyStringLoc->next();

					if (emptyStringLoc->isEnd() == ModTrue &&
						boundary->isEnd() == ModFalse) {
						// emptyStringLoc が終了したのに boundary が終了して
						// いなければ、照合しない（両方とも終了していれば OK）
						isEndStatus = ModTrue;
						return;
					}
				}
			} catch (ModException& e) {
				ModRethrow(e);
			}

			if (boundaryCheck == ModTrue) {
				// OK
				return;
			}
		}

		// 現在の位置は一致しない事がわかったので次に移動する
		termLoc->next();
	}
	isEndStatus = ModTrue;
}
#else // USE_LOWER
void
ModInvertedOperatorWordNodeLocationListIterator::rawNextExact()
{
	if (tokenBoundary == 0) {
		// exactWordModeで検索語内単語境界情報がセットされていない
		// これはショートワードのケース
		// この場合はsimpleWordModeと同じように先頭と終端のみチェックする
		// だけで良い
		rawNextSimple();
		return;
	}

	ModSize termCurrentLoc;

	// exactWord/simpleWord 共通の処理
	while (termLoc->isEnd() == ModFalse) {

		termCurrentLoc = termLoc->getLocation();

		if (emptyStringLoc->isEnd() == ModTrue ||
			emptyStringLoc->getLocation() > termCurrentLoc) {
			// 区切文字の位置が現在の位置を超えている場合はリセットする
#ifdef DEBUG
			ModDebugMessage << "WordNodeLocationListIterator"
							<< "emptySringLocationListIterator need to reset"
							<< ModEndl;
#endif //  DEBUG
			emptyStringLoc->reset();
		}

		// 現在の位置と一致するまで、区切り文字の位置を移動する
		while (emptyStringLoc->getLocation() < termCurrentLoc) {
			emptyStringLoc->next();
			if (emptyStringLoc->isEnd() == ModTrue) {
				break;
			}
		}

		// 現在の位置と区切り文字の位置が一致した - マッチする可能性がある

		if (emptyStringLoc->isEnd() == ModFalse &&
			emptyStringLoc->getLocation() == termCurrentLoc) {

			ModSize termEnd(termCurrentLoc + wordLength);

			// exactWordModeの場合
			// 		検索語内の区切文字の位置と同じところに区切り文字が
			//		ある必要がある

			if (boundary == 0)
				boundary = tokenBoundary->begin();
			else
				boundary->reset();

			ModSize boundaryLoc;
			ModBoolean boundaryCheck(ModTrue);

			while (emptyStringLoc->getLocation() < termEnd) {

				// 検索語内の区切り文字位置情報(先頭からのオフセット)
				// を取り出す。
				boundaryLoc = boundary->getLocation();

				if (emptyStringLoc->getLocation() != 
					termCurrentLoc + boundaryLoc -1) { 
					// termCurrentLocは条件を満たさない
					boundaryCheck = ModFalse;
#ifdef DEBUG
					ModDebugMessage << "{emp,bound} = {" 
									<< emptyStringLoc->getLocation()
									<< ","
									<< termCurrentLoc + boundaryLoc - 1
									<< "["
									<< boundaryLoc
									<< "]"
									<< "} not match"
									<< ModEndl;
#endif
					break;
				}

#ifdef DEBUG
				ModDebugMessage << "{emp,bound} = {" 
								<< emptyStringLoc->getLocation()
								<< ","
								<< boundary->getLocation() + termCurrentLoc - 1
								<< "["
								<< boundary->getLocation()
								<< "]"
								<< "} next"
								<< ModEndl;
#endif

				boundary->next();
				if (boundary->isEnd() == ModTrue) {
					boundaryCheck = ModFalse;
					break;
				}
				emptyStringLoc->next();
				if (emptyStringLoc->isEnd() == ModTrue) {
					boundaryCheck = ModFalse;
					break;
				}
			}

			if (boundaryCheck == ModTrue &&
				emptyStringLoc->getLocation() == termEnd) {
#ifdef DEBUG
				ModDebugMessage << "{emp,bound} = {" 
								<< emptyStringLoc->getLocation()
								<< ","
								<< boundary->getLocation() + termCurrentLoc-1
								<< "["
								<< boundary->getLocation()
								<< "]"
								<< "} match ? check is there more Boundaries"
								<< ModEndl;
#endif
				// 一つ進めて endになる事を確認
				boundary->next();
				if (boundary->isEnd() == ModTrue) {
#ifdef DEBUG
					ModDebugMessage << "last Boundaries check OK"
									<< ModEndl;
#endif // DEBUG
					return;
				}
#ifdef DEBUG
				else {
					ModDebugMessage << "more Boundaries"
									<< ModEndl;
				}
#endif // DEBUG
			}
		}

		// 現在の位置は一致しない事がわかったので次に移動する
		termLoc->next();
	}
}
#endif // USE_LOWER

//
// FUNCTION
// ModInvertedOrderedOperatorWindowLocationListIterator::rawNextSimple
//		-- 次の位置に進む
//
// NOTES
// 現在位置から、距離の制約条件を満たす次の位置に自分を進める。
// 現在位置が距離の制約条件を満たしている場合は動かない。
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
#ifdef USE_LOWER
void
ModInvertedOperatorWordNodeLocationListIterator::rawNextSimple() 
{
	ModSize termCurrentLoc;

	while (termLoc->isEnd() == ModFalse) {

		termCurrentLoc = termLoc->getLocation();

		if (emptyStringLoc->isEnd() == ModTrue) {
			// 最後に達したのでリセットする
			emptyStringLoc->reset();
		}

		if (emptyStringLoc->find(termCurrentLoc) == ModTrue) {
			// 現在の位置と区切り文字の位置が一致した - マッチする可能性がある

			ModSize termEnd(termCurrentLoc + wordLength);

			// 区切り文字を検索語の終端位置まで移動する
			if (emptyStringLoc->lowerBound(termEnd) == ModTrue &&
				emptyStringLoc->getLocation() == termEnd) {
				// 一致した
				return;
			}
			// 現在の位置は条件を満たさない。次の位置をチェック
		}

		// 現在の位置は一致しない事がわかったので次に移動する
		termLoc->next();
	}
	isEndStatus = ModTrue;
}
#else // USE_LOWER
void
ModInvertedOperatorWordNodeLocationListIterator::rawNextSimple() 
{
	ModSize termCurrentLoc;

	// exactWord/simpleWord 共通の処理
	while (termLoc->isEnd() == ModFalse) {

		termCurrentLoc = termLoc->getLocation();

		if (emptyStringLoc->isEnd() == ModTrue ||
			emptyStringLoc->getLocation() > termCurrentLoc) {
			// 区切文字の位置が現在の位置を超えている場合はリセットする
#ifdef DEBUG
			ModDebugMessage << "WordNodeLocationListIterator"
							<< "emptySringLocationListIterator need to reset"
							<< ModEndl;
#endif //  DEBUG
			emptyStringLoc->reset();
		}

		// 現在の位置と一致するまで、区切り文字の位置を移動する
		while (emptyStringLoc->getLocation() < termCurrentLoc) {
			emptyStringLoc->next();
			if (emptyStringLoc->isEnd() == ModTrue) {
				break;
			}
		}

		// 現在の位置と区切り文字の位置が一致した - マッチする可能性がある

		if (emptyStringLoc->isEnd() == ModFalse &&
			emptyStringLoc->getLocation() == termCurrentLoc) {

			ModSize termEnd(termCurrentLoc + wordLength);

			// simpleWordModeの場合
			// 		検索語の終端の位置に区切り文字がある必要がある

			// 区切り文字を検索語の終端位置まで移動する
			while (emptyStringLoc->getLocation() < termEnd) {
				emptyStringLoc->next();
				if (emptyStringLoc->isEnd() == ModTrue) {
					break;
				}
			}

			if (emptyStringLoc->isEnd() == ModFalse &&
				emptyStringLoc->getLocation() == termEnd) {
				// 一致した
#ifdef DEBUG
				ModDebugMessage << "empLoc = {" 
								<< emptyStringLoc->getLocation()
								<< "} "
								<< "termEndPos = {"
								<< termCurrentLoc + wordLength
								<< "} match"
								<< ModEndl;
#endif
				return;
			}
			// ここに来るのは一致しないケース。
			// 現在の位置は条件を満たさない。次の位置をチェック
#ifdef DEBUG
			ModDebugMessage << "empLoc = {" 
							<< emptyStringLoc->getLocation()
							<< "} "
							<< "termEndPos = {"
							<< termCurrentLoc + wordLength
							<< "} not match"
							<< ModEndl;
#endif
		}

		// 現在の位置は一致しない事がわかったので次に移動する
		termLoc->next();
	}
}
#endif // USE_LOWER

//
// FUNCTION
// ModInvertedOrderedOperatorWindowLocationListIterator::rawNextWordHead
//		-- 次の位置に進む
//
// NOTES
// 現在位置から、距離の制約条件を満たす次の位置に自分を進める。
// 現在位置が距離の制約条件を満たしている場合は動かない。
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
void
ModInvertedOperatorWordNodeLocationListIterator::rawNextWordHead()
{
	ModSize termCurrentLoc;

	while (termLoc->isEnd() == ModFalse) {

		termCurrentLoc = termLoc->getLocation();

		if (emptyStringLoc->isEnd() == ModTrue) {
			// 最後に達したのでリセットする
			emptyStringLoc->reset();
		}

		if (emptyStringLoc->find(termCurrentLoc) == ModTrue)
			// 一致した
			return;

		// 現在の位置は一致しない事がわかったので次に移動する
		termLoc->next();
	}
	isEndStatus = ModTrue;
}

//
// FUNCTION
// ModInvertedOrderedOperatorWindowLocationListIterator::rawNextWordTail
//		-- 次の位置に進む
//
// NOTES
// 現在位置から、距離の制約条件を満たす次の位置に自分を進める。
// 現在位置が距離の制約条件を満たしている場合は動かない。
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
void
ModInvertedOperatorWordNodeLocationListIterator::rawNextWordTail()
{
	ModSize termCurrentLoc;

	while (termLoc->isEnd() == ModFalse) {

		termCurrentLoc = termLoc->getEndLocation();

		if (emptyStringLoc->isEnd() == ModTrue) {
			// 最後に達したのでリセットする
			emptyStringLoc->reset();
		}

		if (emptyStringLoc->find(termCurrentLoc) == ModTrue)
			// 一致した
			return;

		// 現在の位置は一致しない事がわかったので次に移動する
		termLoc->next();
	}
	isEndStatus = ModTrue;
}

#ifndef MOD_DIST // APPMODE
//
// FUNCTION
// ModInvertedOrderedOperatorWindowLocationListIterator::rawNextApproximate
//		-- 次の位置に進む
//
// NOTES
// 現在位置から、距離の制約条件を満たす次の位置に自分を進める。
// 現在位置が距離の制約条件を満たしている場合は動かない。
//
// これはappMode(ApproximateMode)の時に呼ばれる。
// AppModeの照合位置は単語単位検索ではなく、文字列としての照合の位置を返す。
// (=termLoc の位置を返せばよい)
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
#ifdef USE_LOWER
void
ModInvertedOperatorWordNodeLocationListIterator::rawNextApproximate()
{
	// termLocの位置を次に進める
	if(termLoc->isEnd() == ModTrue) {
		isEndStatus = ModTrue;
	}
}
#endif
#endif

//
// FUNCTION
// ModInvertedOperatorWordNodeLocationListIterator::getCurrentMatchType
//
// NOTES
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
ModInvertedTermMatchMode 
ModInvertedOperatorWordNodeLocationListIterator::getCurrentMatchType()
{
#ifdef MOD_DIST // APPMODE
	return matchMode;
#else
	if (matchMode != ModInvertedTermApproximateMode) {
		// appMode以外の場合はiterator にセットされているmatchModeを返す
		return matchMode;
	}

	// ここに来る時点でiteratorは文字列としてマッチしているいちにある
	ModInvertedTermMatchMode
		currentMatchMode(ModInvertedTermStringMode);

	// 現在の照合位置を取得
	ModSize termCurrentLoc = termLoc->getLocation();
	ModSize termEnd = termCurrentLoc + wordLength;

	if (tokenBoundary != 0) {
		// tokenBoundaryが0の場合はhead/tailのみのチェックでよい

		if (emptyStringLoc->isEnd() == ModTrue ||
			emptyStringLoc->getLocation() > termCurrentLoc) {
			// 区切文字の位置が現在の位置を超えている場合はリセットする
			emptyStringLoc->reset();
		}

		// 現在の位置と一致するまで、区切り文字の位置を移動する
		if (emptyStringLoc->lowerBound(termCurrentLoc) == ModFalse) {
			// 検索後の先頭の位置以降に単語境界がない
			// 当然termEndの位置にも単語境界はない 文字列としてマッチ

			return currentMatchMode;
		}

		if (emptyStringLoc->getLocation() == termCurrentLoc) {
			// headがマッチ
			currentMatchMode = static_cast<ModInvertedTermMatchMode>(
				currentMatchMode | ModInvertedTermWordHead);

			ModBoolean boundaryCheck(ModTrue);

			--termCurrentLoc;

			try {
				if (boundary == 0)
					boundary = tokenBoundary->begin();
				else
					boundary->reset();

				while (boundary->isEnd() == ModFalse) {
					if (emptyStringLoc->getLocation() !=
						termCurrentLoc + boundary->getLocation()) {
						// 境界が一致しない
						boundaryCheck = ModFalse;
						break;
					}

					boundary->next();
					emptyStringLoc->next();

					if (emptyStringLoc->isEnd() == ModTrue &&
									boundary->isEnd() == ModFalse) {
						// emptyStringLoc が終了したのに boundary が終了して
						// いなければ、照合しない（両方とも終了していれば OK）
						boundaryCheck = ModFalse;
						break;
					}
				}

			} catch (ModException& e) {
				ModRethrow(e);
			}

			if (boundaryCheck == ModTrue) {
				// OK
				// 厳格一致
				currentMatchMode = ModInvertedTermExactWordMode;
			} else {
				// ここの時点ではheadのみ一致することが確認できている
				// 厳格一致ではない
				// tailの確認
				if (emptyStringLoc->isEnd() == ModTrue ||
					emptyStringLoc->getLocation() > termEnd) {
					emptyStringLoc->reset();
				}

				if(emptyStringLoc->lowerBound(termEnd) == ModTrue) {
					if(emptyStringLoc->getLocation() == termEnd) {
						//  tailも一致
						currentMatchMode =static_cast<ModInvertedTermMatchMode>(
							currentMatchMode | ModInvertedTermWordTail);
					}
				}
			}
		} else {
			// headがマッチしていないので後はwordTailの可能性だけが残っている
			if (emptyStringLoc->isEnd() == ModTrue ||
				emptyStringLoc->getLocation() > termEnd) {
				emptyStringLoc->reset();
			}

			if(emptyStringLoc->lowerBound(termEnd) == ModTrue) {
				if(emptyStringLoc->getLocation() == termEnd) {
					//  tail一致
					currentMatchMode =static_cast<ModInvertedTermMatchMode>(
							currentMatchMode | ModInvertedTermWordTail);

				}
			}
		}

	} else {
		// tokneBoundaryがない head/tailのみの確認でOK

		if (emptyStringLoc->isEnd() == ModTrue ||
			emptyStringLoc->getLocation() > termCurrentLoc) {
			// 区切文字の位置が現在の位置を超えている場合はリセットする
			emptyStringLoc->reset();
		}

		// 現在の位置と一致するまで、区切り文字の位置を移動する
		if (emptyStringLoc->lowerBound(termCurrentLoc) == ModFalse) {
			// 検索後の先頭の位置以降に単語境界がない
			// 当然termEndの位置にも単語境界はない 文字列としてマッチ

			// stringMode
			return currentMatchMode;
		}

		if (emptyStringLoc->getLocation() == termCurrentLoc) {
			// headがマッチ
			currentMatchMode = static_cast<ModInvertedTermMatchMode>(
				currentMatchMode | ModInvertedTermWordHead);
		}

		if (emptyStringLoc->getLocation() > termEnd) {
			// 区切文字の位置がEndの位置を超えている場合はリセットする
			emptyStringLoc->reset();
		}

		// 現在の位置と一致するまで、区切り文字の位置を移動する
		if (emptyStringLoc->lowerBound(termEnd) == ModTrue) {
			if(emptyStringLoc->getLocation() == termEnd) {
				// tailがマッチ
				currentMatchMode = static_cast<ModInvertedTermMatchMode>(
						currentMatchMode | ModInvertedTermWordTail);
			}
		}

		// このケースはtokenBoundaryがない = shortWordなので
		// 検索語ないに単語境界はない
		// head,tail両方満たしている場合は exactWordModeを返す
		if((currentMatchMode & ModInvertedTermWordHead) != 0 
				&& (currentMatchMode & ModInvertedTermWordTail) != 0) {
			// simpleWordMode
			currentMatchMode = ModInvertedTermExactWordMode;
		}

	}
	return currentMatchMode;
#endif
}

#endif // V1_4	単語単位検索

//
// Copyright (c) 2000, 2001, 2002, 2005, 2006, 2008, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
