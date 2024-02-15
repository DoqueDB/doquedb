// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ScoreCalculator.h --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_SCORECALCULATOR_H
#define __SYDNEY_FULLTEXT2_SCORECALCULATOR_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"

#include "ModVector.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::ScoreCalculator -- スコア計算器
//
//	NOTES
//
class ScoreCalculator
{
	//【注意】
	// 外部スコア計算器の実装時に、このヘッダーファイルを
	// インクルードする必要がある
	// ただし、外部スコア計算器の実装時に SyDrvFts2 をリンクさせたくないので、
	// このクラスは、純粋仮想関数のみとする
	
public:
	//
	// 引数を格納する構造体
	//
	// スコアの計算は、結局すべてのデータを double にキャストして行うので、
	// データ種別によらず、すべてのデータのデータ型は double とする
	//
	struct Argument
	{
		// データ種別
		enum Type {
			// 不定
			Unknown					= 0,
			
			//
			// 全体には関係ないが、文書ごとに変化する
			//
			// 文書内頻度
			TermFrequency 			= (1 << 0),
			// 文書長
			DocumentLength			= (1 << 1),

			//
			// 全体にも関係しないし、文書ごとにも変化しない
			//
			// 検索文内頻度
			QueryTermFrequency		= (1 << 8),

			//
			// 全体に関係するが、文書ごとには変化はない
			//
			// 文書頻度(ヒット数)
			DocumentFrequency		= (1 << 16),
			// 総文書長
			TotalDocumentLength		= (1 << 17),
			// 総文書内頻度
			TotalTermFrequency		= (1 << 18),
			// 平均文書長
			AverageDocumentLength	= (1 << 19),
			// 総文書数
			TotalDocumentFrequency	= (1 << 20)
		};

		// コンストラクタ
		Argument() : m_eType(Argument::Unknown), m_dblValue(0.0) {}
		// コンストラクタ
		Argument(Type eType_) : m_eType(eType_), m_dblValue(0.0) {}

		// データ種別
		Type		m_eType;
		// データ
		double		m_dblValue;
	};
	
	// コンストラクタ
	ScoreCalculator() {}
	// デストラクタ
	virtual ~ScoreCalculator() {}
	
	// 必要な引数を得る
	virtual void initialize(ModVector<Argument>& arg_) = 0;

	// 前準備を行う
	// TF項の計算部分のうち、文書単位で変化しない部分を事前に計算しておく
	virtual void prepare(const ModVector<Argument>& arg_) = 0;

	//【注意】
	// スコア計算は、TF項 * IDF項によって行われる
	// TF項は、文書ごとに値が再設定され呼び出されるが、
	// IDF項は、検索処理中同じ数値を返すことが想定されており、
	// 検索中に一度しか呼び出されない。(検索ノード内でキャッシュされる)

	// TF項を計算する
	virtual double firstStep(const ModVector<Argument>& arg_) = 0;
	// IDF項を計算する
	virtual double secondStep(const ModVector<Argument>& arg_) = 0;

	// コピーを取得する
	virtual ScoreCalculator* copy() = 0;

};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#ifdef WIN32
#ifdef SYD_EXTERNAL_CALCULATOR
#define SYD_CALCULATOR_FUNCTION	__declspec(dllexport)
#else
#define SYD_CALCULATOR_FUNCTION
#endif
#else
#define SYD_CALCULATOR_FUNCTION
#endif

//
//	FUNCTION global
//	DBGetScoreCalculator -- 外部スコア計算器を得る
//
//	NOTES
//	外部スコア計算器を得るエントリ関数
//	外部スコア計算器を提供する場合は、このエントリ関数を実装する必要がある
//
//	ARGUMENTS
// 	const ModUnicodeChar* parameter_
//		パラメータ
//
//	RETURN
//	FullText2::ScoreCalculator*
//		スコア計算器へのポインタ
//
//	EXCEPTIONS
//
extern "C" SYD_CALCULATOR_FUNCTION
_SYDNEY::FullText2::ScoreCalculator*
DBGetScoreCalculator(const ModUnicodeChar* parameter_);

//
//	FUNCTION global
//	DBReleaseScoreCalculator -- 外部スコア計算器を解放する
//
//	NOTES
//	外部スコア計算器を提供する場合には、このエントリ関数を実装する必要がある
//
//	ARGUMENTS
//	FullText2::ScoreCalculator* pCalculator_
//		解放する外部スコア計算器
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
extern "C" SYD_CALCULATOR_FUNCTION
void
DBReleaseScoreCalculator(_SYDNEY::FullText2::ScoreCalculator* pCalculator_);

#endif //__SYDNEY_FULLTEXT2_SCORECALCULATOR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
