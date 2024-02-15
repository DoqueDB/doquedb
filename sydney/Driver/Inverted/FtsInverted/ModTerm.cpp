//
// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
//	ModTerm.cpp -- ModTerm の実装
// 
// Copyright (c) 2000, 2004, 2005, 2006, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
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
#include "ModOsDriver.h"	// log, pow, sqrt
#include "ModTerm.h"
#include "ModUnicodeOstrStream.h"

#define LOG(x)	 ModOsDriver::log(x)
#define SQRT(x)	ModOsDriver::sqrt(x)
#define POW(x,y) ModOsDriver::pow(x,y)
#define EXP(x)	 POW(2.718281828,x)

typedef ModInvertedSearchResultScore	RankingResult;
typedef ModInvertedDocumentID			DocumentID;
typedef ModInvertedDocumentScore		DocumentScore;
typedef ModHasher<DocumentID>			DocumentIDHasher;
typedef ModHashMap<DocumentID,DocumentScore,DocumentIDHasher> ModTermDocIDMap;


namespace {

	// リソースファイルのファイル名

	const ModUnicodeString _ParameterDict("parameter.utf8");
	const ModUnicodeString _StopDict("stopDict.utf8");
	const ModUnicodeString _StopExpansionDict("stopExpansion.utf8");
	const ModUnicodeString _StopTypeDict("stopType.utf8");

	const ModUnicodeString _PatternDict("patternDict.utf8");
	const ModUnicodeString _TermTypeTable("termType.utf8");

#ifdef SYD_USE_UNA_V10
	ModUnicodeString _ModNlpNormOnly("0");
	ModUnicodeString _ModNlpNormRet("4");
	ModUnicodeString _ModNlpNormRetDiv("6");
	ModUnicodeString _ModNlpNormRetStemDiv("5");

#ifdef SYD_USE_UNA_V12
	ModUnicodeString _Stem("stem");
	ModUnicodeString _Carriage("carriage");
	ModUnicodeString _Compound("compound");
#endif

	ModUnicodeString _DoNorm("donorm");
	ModUnicodeString _True("true");
	ModUnicodeString _False("false");
#endif
}

//
// FUNCTION public
// ModTermResource::ModTermResource -- 検索語処理器リソースのコンストラクタ
//
// NOTES
//	 検索語処理器リソースのコンストラクタ。
//	 各リソースファイルを基に以下の辞書を生成しそのポインタをセット。
//	 --------------------------------------------------------------
//		属性				 説明			 リソースファイル
//	 --------------------------------------------------------------
//	 - parameterDict		パラメタ辞書	(dir/paramter.utf8)
//	 - stopDict				禁止辞書		(dir/stopDict.utf8)
//	 - stopExpansionDict	禁止拡張辞書	(dir/stopExpansion.utf8)
//	 - stopTypeDict			禁止タイプ辞書	(dir/stopType.utf8)
//	 - patternDict			パタン辞書		(dir/patternDict.utf8)
//	 - termTypeTable		検索語タイプ表	(dir/termType.utf8)
//	 --------------------------------------------------------------
//
// ARGUMENTS
//	 const ModUnicodeString& dir	リソースの格納ディレクトリ
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
ModTermResource::ModTermResource(
	const ModUnicodeString& dir)	// リソースの格納ディレクトリ
{
	// 初期化
	stopDict			= 0;
	parameterDict		= 0;
	stopExpansionDict	= 0;
	stopTypeDict		= 0;
	patternDict			= 0;
	termTypeTable		= 0;

	try {
		// 読み込み
		parameterDict		 = new ModTermParameterFile(dir + _ParameterDict);
		stopDict					= new ModTermWordFile(dir + _StopDict);
		stopExpansionDict = new ModTermWordFile(dir + _StopExpansionDict);
		stopTypeDict			= new ModTermTypeFile(dir + _StopTypeDict);

		// リソースのバージョン設定
		double value;
		if(parameterDict->getValue("version", value) == ModTrue) {
			version = (int)value;
		} else {
			version = 0;
		}

		if(version == 0) {
			patternDict		= new ModTermPatternFile(dir + _PatternDict);
		} else {
			termTypeTable	= new ModTermTypeTable(dir + _TermTypeTable);
		}

		// 例外発生
	} catch (ModException& exception){
		// これまで作成したリソースを解放する
		if(stopDict != 0) {
			delete stopDict;
		}
		if(parameterDict != 0) {
			delete parameterDict;
		}
		if(stopExpansionDict != 0) {
			delete stopExpansionDict;
		}
		if(stopTypeDict != 0) {
			delete stopTypeDict;
		}
		if(version == 0 && patternDict != 0) {
			delete patternDict;
		}
		if(version != 0 && termTypeTable != 0) {
			delete termTypeTable;
		}
		// 再スロー
		ModRethrow(exception);
	}
}

//
// FUNCTION public
// ModTermResource::~ModTermResource-- デストラクタ
//
// NOTES
//	 デストラクタ。 各リソースを消去する。
//
// ARGUMENTS
//	 なし
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
ModTermResource::~ModTermResource()
{
	delete stopDict;
	delete parameterDict;
	delete stopExpansionDict;
	delete stopTypeDict;
	if(version == 0) {
		delete patternDict;
	} else {
		delete termTypeTable;
	}
}

// 区切り文字
static const ModUnicodeChar sepField('\t');
static const ModUnicodeChar sepRecord('\n');
#ifdef SYD_USE_UNA_V10
namespace {
	const ModUnicodeString _SepField("\t");
	const ModUnicodeString _SepRecord("\n");
}
#endif

//
// FUNCTION public
// ModTerm::ModTerm -- 検索語処理器コンストラクタ
//
// NOTES
//	 検索語処理器コンストラクタ。
//
//	 リソース中のパラメタファイルを基に各パラメタについて以下を行う。
//	 - 値域の検査
//	 - パラメタ値の設定
//	 パラメタ値がパラメタファイル中に見付からない場合はデフォルト値が
//	 設定される。
//
// ARGUMENTS
//	 const ModTermResource* _resource		リソース
//	 ModNlpAnalyzer*	_analyzer			自然言語解析器
//	 ModBoolean			_isOwner			オーナーか
//	 ModSize _collectionSize				登録文書の総数
//	 double _averageLength					登録文書の平均長
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
ModTerm::ModTerm(
	const ModTermResource* _resource,		// リソース
#ifdef SYD_USE_UNA_V10
	UNA::ModNlpAnalyzer* _analyzer,				// 自然言語解析器
#else
	ModNlpAnalyzer* _analyzer,				// 自然言語解析器
#endif
	ModBoolean _isOwner,					// オーナーか
	ModSize _collectionSize,				// 登録文書の総数
	double _averageLength,					// 登録文書の平均長
	ModSize maxWordLen_
	)
	: resource(_resource),
		analyzer(_analyzer),
		isOwner(_isOwner),
		collectionSize(_collectionSize),
		averageLength(_averageLength)
{
	double value;

	//
	// 主要パラメタ
	//
	// 初期検索語数の上限
	if(resource->parameterDict->getValue("maxTerm1", value) == ModTrue) {
		maxTerm1 = (ModSize)value;
	} else {
		maxTerm1 = 10;
	}

	// 拡張検索語数の上限
	if(resource->parameterDict->getValue("maxTerm2", value) == ModTrue) {
		maxTerm2 = (ModSize)value;
	} else {
		maxTerm2 = 10;
	}

	// 拡張検索語数の下限
	if(resource->parameterDict->getValue("minTerm2", value) == ModTrue) {
		minTerm2 = (ModSize)value;
	} else {
		minTerm2 = 2;
	}

	// 検索語候補数の上限
	if(resource->parameterDict->getValue("maxCandidate", value) == ModTrue) {
		maxCandidate = (ModSize)value;
	} else {
		maxCandidate = 500;
	}

	// シード文書数の上限
	if(resource->parameterDict->getValue("maxSeed", value) == ModTrue) {
		maxSeed = (ModSize)value;
	} else {
		maxSeed = 10;
	}

	//
	// 初期検索語候補の生成パタメタ
	//

	// 単独語生成の有無
	if(resource->parameterDict->getValue("useUniGram1", value) == ModTrue) {
		useUniGram1 = value ? ModTrue : ModFalse;
	} else {
		useUniGram1 = ModTrue;	// 生成する
	}

	// 隣接語生成の有無
	if(resource->parameterDict->getValue("useBiGram1", value) == ModTrue) {
		useBiGram1 = value ? ModTrue : ModFalse;
	} else {
		useBiGram1 = ModTrue;	// 生成する
	}

	// 禁止辞書使用の有無
	if(resource->parameterDict->getValue("useStopDict1", value) == ModTrue) {
		useStopDict1 = value ? ModTrue : ModFalse;
	} else {
		useStopDict1 = ModTrue;	// 使用する
	}

	// 正規化処理の有無
	if(resource->parameterDict->getValue("useNormalizer1", value) == ModTrue) {
		useNormalizer1 = value ? ModTrue : ModFalse;
	} else {
		useNormalizer1 = ModTrue; // 正規化あり
	}

	// 失敗回避の有無
	if(resource->parameterDict->getValue("failsafe1", value) == ModTrue) {
		failsafe1 = value ? ModTrue : ModFalse;
	} else {
		failsafe1 = ModTrue; // 回避する
	}

	// 検索語単位
#ifdef SYD_USE_UNA_V10
	if (resource->parameterDict->getValue("termUnit1", value) == ModTrue) {
		ModSize n = (ModSize)value;
		ModUnicodeOstrStream s;
		s << n;
		termUnit1 = s.getString();
	} else {
		// 正規化、ステミング、短単位、改行処理
		termUnit1 = _ModNlpNormRetStemDiv;
	}
#else
	if(resource->parameterDict->getValue("termUnit1", value) == ModTrue) {
		ModSize n = (ModSize)value;
		termUnit1 = (ModTermUnit)n;
	} else {
#if 0 // 2003/2/5に変更
		termUnit1 = ModNlpNormStemDiv; // 正規化、ステミング、短単位
#else
		termUnit1 = ModNlpNormRetStemDiv; // 正規化、ステミング、短単位、改行処理
#endif
	}
#endif

	// 検索要求の最大長 (形態素数)
	if(resource->parameterDict->getValue("maxText1", value) == ModTrue) {
		maxText1 = (ModSize)value;
	} else {
		maxText1 = 0; // 無制限
	}

	// 検索語生成法
	if(resource->parameterDict->getValue("termMethod1", value) == ModTrue) {
		termMethod1 = (ModTermMethod)value;
	} else {
		termMethod1 = defaultMethod;
	}

	// 検索語生成法
	if(resource->parameterDict->getValue("maxTermLength1", value) == ModTrue) {
		maxTermLength1 = (ModSize)value;
	} else {
		maxTermLength1 = 4;
	}

	//
	// 初期検索語のパラメタ
	//

	// スケールパラメタ
	if(resource->parameterDict->getValue("paramScale1", value) == ModTrue) {
		paramScale1 = value;
	} else {
		paramScale1 = 0.5;
	}

	// 重みパラメタ (ALPHA, k4)
	if(resource->parameterDict->getValue("paramWeight1", value) == ModTrue) {
		paramWeight1 = value;
	} else {
		paramWeight1 = 0.2;
	}

	// 重みパラメタ調整用 (RHO)
	if(resource->parameterDict->getValue("adaptWeight1", value) == ModTrue) {
		adaptWeight1 = value;
	} else {
		adaptWeight1 = 0;
	}

	// 文書スコアパラメタ (KAPPA, k1)
	if(resource->parameterDict->getValue("paramScore1", value) == ModTrue) {
		paramScore1 = value;
	} else {
		paramScore1 = 1.0;
	}

	// 文書長パラメタ (LAMBDA, b)
	if(resource->parameterDict->getValue("paramLength1", value) == ModTrue) {
		paramLength1 = value;
	} else {
		paramLength1 = 0.25;
	}

	// 隣接語の近接パラメタ
	if(resource->parameterDict->getValue("paramProximity1", value) == ModTrue) {
		paramProximity1 = (int)value;
	} else {
		paramProximity1 = 0;
	}

	// 隣接語の結合パラメタ (PSI)
	if(resource->parameterDict->getValue("paramCombineBiGram1", value) == ModTrue) {
		paramCombineBiGram1 = value;
	} else {
		paramCombineBiGram1 = 0.3;
	}

	//
	// シード検索語の生成パラメタ
	//

	// 単独語生成の有無
	if(resource->parameterDict->getValue("useUniGram2", value) == ModTrue) {
		useUniGram2 = value ? ModTrue : ModFalse;
	} else {
		useUniGram2 = ModTrue;	// 生成する
	}

	// 隣接語生成の有無
	if(resource->parameterDict->getValue("useBiGram2", value) == ModTrue) {
		useBiGram2 = value ? ModTrue : ModFalse;
	} else {
		useBiGram2 = ModTrue;	// 生成する
	}

	// 禁止辞書使用の有無
	if(resource->parameterDict->getValue("useStopDict2", value) == ModTrue) {
		useStopDict2 = value ? ModTrue : ModFalse;
	} else {
		useStopDict2 = ModTrue;	// 使用する
	}

	// 正規化処理の有無
	if(resource->parameterDict->getValue("useNormalizer2", value) == ModTrue) {
		useNormalizer2 = value ? ModTrue : ModFalse;
	} else {
		useNormalizer2 = ModTrue; // 正規化あり
	}

	// 失敗回避の有無
	if(resource->parameterDict->getValue("failsafe2", value) == ModTrue) {
		failsafe2 = value ? ModTrue : ModFalse;
	} else {
		failsafe2 = ModFalse; // 回避しない
	}

	// 検索語単位
#ifdef SYD_USE_UNA_V10
	if (resource->parameterDict->getValue("termUnit2", value) == ModTrue) {
		ModSize n = (ModSize)value;
		ModUnicodeOstrStream s;
		s << n;
		termUnit2 = s.getString();
	} else {
		// 正規化、長単位、改行処理
		termUnit2 = _ModNlpNormRet;
	}
#else
	if(resource->parameterDict->getValue("termUnit2", value) == ModTrue) {
		ModSize n = (ModSize)value;
		termUnit2 = (ModTermUnit)n;
	} else {
#if 0 // 2003/2/5に変更
		termUnit2 = ModNlpNormOnly;	// 正規化、長単位
#else
		termUnit2 = ModNlpNormRet;	 // 正規化、長単位、改行処理
#endif
	}
#endif

	// 各シード文書の最大長 (形態素数)
	if(resource->parameterDict->getValue("maxText2", value) == ModTrue) {
		maxText2 = (ModSize)value;
	} else {
		maxText2 = 5000;
	}

	//
	// 拡張語候補の選択パラメタ
	//

	// 出現シード文書数の下限
	if(resource->parameterDict->getValue("minSeedDf2", value) == ModTrue) {
		minSeedDf2 = (int)value;
	} else {
		minSeedDf2 = 1;	// 出現シード文書数が1以下ならば選択しない
	}

	// 一文字語の使用の有無
	if(resource->parameterDict->getValue("useSingleChar2", value) == ModTrue) {
		useSingleChar2 = value ? ModTrue : ModFalse;
	} else {
		useSingleChar2 = ModFalse;	// 使用しない
	}

	// 単独語の使用の有無
	if(resource->parameterDict->getValue("useUniGramExpansion2", value) == ModTrue) {
		useUniGramExpansion2 = value ? ModTrue : ModFalse;
	} else {
		useUniGramExpansion2 = ModTrue;	// 使用する
	}

	// 隣接語の使用の有無
	if(resource->parameterDict->getValue("useBiGramExpansion2", value) == ModTrue) {
		useBiGramExpansion2 = value ? ModTrue : ModFalse;
	} else {
		useBiGramExpansion2 = ModFalse;	// 使用しない
	}

	//
	// 拡張語の検索パラメタ
	//

	// スケールパラメタ
	if(resource->parameterDict->getValue("paramScale2", value) == ModTrue) {
		paramScale2 = value;
	} else {
		paramScale2 = 0.5;
	}

	// 重みパラメタ (ALPHA, k4)
	if(resource->parameterDict->getValue("paramWeight2", value) == ModTrue) {
		paramWeight2 = value;
	} else {
		paramWeight2 = 0.2;
	}

	// 文書スコアパラメタ (KAPPA, k1)
	if(resource->parameterDict->getValue("paramScore2", value) == ModTrue) {
		paramScore2 = value;
	} else {
		paramScore2 = 1.0;
	}

	// 文書長パラメタ (LAMBDA, b)
	if(resource->parameterDict->getValue("paramLength2", value) == ModTrue) {
		paramLength2 = value;
	} else {
		paramLength2 = 0.25;
	}

	// 隣接語の近接パラメタ
	if(resource->parameterDict->getValue("paramProximity2", value) == ModTrue) {
		paramProximity2 = (int)value;
	} else {
		paramProximity2 = 0;
	}

	// 単独語の結合パラメタ (XI)
	if(resource->parameterDict->getValue("paramCombineUniGram2", value) == ModTrue) {
		paramCombineUniGram2 = value;
	} else {
		paramCombineUniGram2 = 0.2;
	}

	// 隣接語の結合パラメタ (PSI * XI)
	if(resource->parameterDict->getValue("paramCombineBiGram2", value) == ModTrue) {
		paramCombineBiGram2 = value;
	} else {
		paramCombineBiGram2 = 0.06;
	}

	//
	// 重みの混合パラメタ
	//

	// 初期単独語の重み混合パラメタ (CHI1)
	if(resource->parameterDict->getValue("paramMixUniGram1", value) == ModTrue) {
		paramMixUniGram1 = value;
	} else {
		paramMixUniGram1 = 0.8;
	}

	// 初期隣接語の重み混合パラメタ (CHI2)
	if(resource->parameterDict->getValue("paramMixBiGram1", value) == ModTrue) {
		paramMixBiGram1 = value;
	} else {
		paramMixBiGram1 = 0.6;
	}

	// 拡張単独語の重み混合パラメタ (CHI3)
	if(resource->parameterDict->getValue("paramMixUniGram2", value) == ModTrue) {
		paramMixUniGram2 = value;
	} else {
		paramMixUniGram2 = 0.9;
	}

	// 拡張隣接語の重み混合パラメタ (CHI2)
	if(resource->parameterDict->getValue("paramMixBiGram2", value) == ModTrue) {
		paramMixBiGram2 = value;
	} else {
		paramMixBiGram2 = 0.6;
	}

	//
	// マージランクのパラメタ
	//

	// 初期検索結果の最大マージランク
	if(resource->parameterDict->getValue("maxRank1", value) == ModTrue) {
		maxRank1 = (ModSize)value;
	} else {
		maxRank1 = 0;	// マージランクを行わない
	}

	// 拡張検索結果の最大マージランク
	if(resource->parameterDict->getValue("maxRank2", value) == ModTrue) {
		maxRank2 = (ModSize)value;
	} else {
		maxRank2 = 2000;	// 最終的な検索結果 1000 × 2
	}

	// ランクの混合パラメタ (BETA)
	if(resource->parameterDict->getValue("paramMixRank", value) == ModTrue) {
		paramMixRank = value;
	} else {
		paramMixRank = 0.6;
	}

	//
	// 検索語の重要度(スケール)指定のパラメタ
	//

	// 追加された検索語に対するスケール値
	if(resource->parameterDict->getValue("scaleUndefined", value) == ModTrue) {
		scaleUndefined = value;
	} else {
		scaleUndefined = 1.0;
	}
	// 重要な初期検索語に対するスケール値
	if(resource->parameterDict->getValue("scaleImportant1", value) == ModTrue) {
		scaleImportant1 = value;
	} else {
		scaleImportant1 = 2.0;
	}
	// 重要な拡張検索語に対するスケール値
	if(resource->parameterDict->getValue("scaleImportant2", value) == ModTrue) {
		scaleImportant2 = value;
	} else {
		scaleImportant2 = 1.0;
	}

#ifdef SYD_USE_UNA_V10
	// UNAのパラメータを設定する
#if 0
	m_cUnaParam.insert(_NormMode, _ModNlpNormOnly);		// ModNlpNormOnly
	m_cUnaParam.insert(_GetOrg, _False);
	m_cUnaParam.insert(_GetVec, _True);
#endif
	m_cUnaParam.insert(_DoNorm, _True);
//	m_cUnaParam.insert("sep1", _SepField);
//	m_cUnaParam.insert("sep2", _SepRecord);
	ModUnicodeOstrStream str;
	str << maxWordLen_;
	m_cUnaParam.insert("maxwordlen", str.getString());	// MaxWordLen
#endif
}

//
//	FUNCTION public
//	ModTerm::~ModTerm -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ModTerm::~ModTerm()
{
	if (isOwner)
	{
#ifdef SYD_USE_UNA_V10
		analyzer->releaseResource();
#endif
		delete analyzer;
	}
}

//
// FUNCTION static
// _nextTerm -- レコード頭位置の獲得
//
// NOTES
//	 対象文字列中の次のレコード頭位置を返す。
//
// ARGUMENTS
//	 - string		対象文字列
//	 - offset		現在のレコード頭の位置
//
// RETURN
//	 ModSize 次のレコード頭の位置
//
// EXCEPTIONS
//
static ModSize _nextTerm(
	ModUnicodeString string,	// 対象文字列
	ModSize				offset)	// 現在のレコード頭の位置
{
	while(offset < string.getLength()) {
		if(string[offset++] == sepRecord) break;
	}
	return offset;
}

//
// FUNCTION public
// ModTerm::poolTerm -- 検索語をプール
//
// NOTES
//	 テキストから検索語群を生成し検索語プールに登録する。
//	 検索語プール中の検索語には以下がセットされる。
//	 - 語形 (string)
//	 - タイプ (type, パタン種別の値)
//	 - 重みづけ値 (twv, パタン重みの値)
//	 - 選択値 (tsv, 出現頻度と同一値)
//	 - 出現頻度 (tf)
//
// ARGUMENTS
//	 const ModUnicodeString& text,		テキスト
//	 const ModBoolean	 useUniGram		単独語生成の有無
//	 const ModBoolean	 useBiGram		隣接語生成の有無
//	 const ModBoolean	 useStopDict	禁止辞書使用の有無
//	 const ModBoolean	 useNormalizer	正規化処理の有無
//	 const ModTermUnit	termUnit		検索語単位
//	 const ModSize			maxText		テキストの最大長 (形態素数,0なら無制限)
//	 ModTermPool&			 pool		検索語プール
//	 ModTermTable*			table		検索語テーブル
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
void
ModTerm::poolTerm(
	const ModUnicodeString& text,		// テキスト
#ifdef V1_6
	const ModLanguageSet& langSpec,		// 言語指定
#endif
	const ModBoolean	useUniGram,		// 単独語生成の有無
	const ModBoolean	useBiGram,		// 隣接語生成の有無
	const ModBoolean	useStopDict,	// 禁止辞書使用の有無
	const ModBoolean	useNormalizer,	// 正規化処理の有無
#ifdef SYD_USE_UNA_V10
	const ModUnicodeString&	termUnit,	// 検索語単位
#else
	const ModTermUnit	termUnit,		// 検索語単位
#endif
	const ModSize		maxText,	// テキストの最大長(形態素数,0なら無制限)
	ModTermPool&		pool,		// 検索語プール
	ModTermTable*		table)		// 検索語テーブル
{
	ModUnicodeString preGram;			// 前置語の語形 (正規化あり)
	ModUnicodeString preOriginal;		// 前置語の語形 (正規化なし)
										// - version0 未対応
										// - version1 対応済
	ModTermType			preType;		// 前置語のパタン種別
	double				preWeight;		// 前置語のパタン重み
	ModSize analyzedLength = 0;			// 解析した語数

	ModUnicodeString uniGram;			// 単独語の語形

	// 高速化バージョンへ
	if(resource->version == 1) {
		goto version1;
	}
	// 以下はパタン辞書を用いた旧版

#ifdef SYD_USE_UNA_V10
	parseNormMode(termUnit);
	m_cUnaParam[_DoNorm] = (useNormalizer == ModTrue) ? _True : _False;
	analyzer->prepare(m_cUnaParam);
	analyzer->set(text,langSpec);

#else
	// 対象テキストの設定
#ifdef V1_6
	analyzer->set(text, termUnit, langSpec);
#else
	analyzer->set(text, termUnit);
#endif
#endif

	if (patternSet.getSize() == 0)
	{
		resource->patternDict->getPattern(patternSet);
	}

	{
	ModUnicodeString analysis;			// 自然言語解析結果 (文字列)
	analysis.reallocate(4096);
#ifdef SYD_USE_UNA_V10
	// 解析ブロック毎に
	while(analyzer->getBlock(analysis, sepField, sepRecord)){
#else
	// 解析ブロック毎に
	while(analyzer->getBlock(analysis, sepField, sepRecord, useNormalizer)
					== ModTrue) {
#endif
		// 前置語をリセット (解析ブロックに跨る検索語は生成しない)
		preGram.clear();

		// レコード毎に
		ModSize offset = 0;
		while(offset < analysis.getLength()) {

			// 検索語の検出成功を表すフラグ
			ModBoolean found = ModFalse;

			// パタン毎に
			for(ModVector<ModTermPattern>::Iterator
					pattern = patternSet.begin();
				pattern != patternSet.end(); ++pattern) {

				// 隣接語を生成しない場合は前置/後置語パタンをスキップし高速化
				if(useBiGram != ModTrue) {
					if((*pattern).getID() == 3) continue;
					if((*pattern).getID() == 4) continue;
				}
				// パタン照合に成功
				if((*pattern).match(analysis, offset) == ModTrue) {
					// 区切語
					if((*pattern).getID() == 1) {
						// 前置語はリセットしない
						goto there;
					}
					else
					// 禁止単独語
					if((*pattern).getID() == 2) {
						// 前置語をリセットして次のレコードへ
						preGram.clear();
						goto there;
					}
					else
					// 後置語
					if((*pattern).getID() == 3) {

						// 前置語があれば隣接語の生成
						if(preGram.getLength() != 0) {

							// 検索語文字列の獲得
							uniGram = (*pattern).replace();
							// 禁止辞書の検査
							if(useStopDict == ModTrue) {
								if(resource->stopDict->isFound(uniGram)
								== ModTrue) {
									// 前置語をリセットして次のレコードへ
									preGram.clear();
									goto there;
								}
							}
							// 隣接語を合成
							ModTermElement biTerm(
											preGram,
											preType,
											preWeight,
											uniGram,
											(*pattern).getType(),
											(*pattern).getWeight());
#ifdef V1_6
							biTerm.setLangSpec(langSpec);
#endif
							// 検索語プールへの登録
							pool.insertTerm(biTerm);

							// 検索語テーブルへの登録
							if(table != 0) {

								 // 隣接語の登録
								 table->insertTerm(biTerm, 2);

								 // 後置語の登録 (useUniGramにかかわらず)
								 ModTermElement uniTerm(
									 uniGram,					// 検索語文字列
									 (*pattern).getType(),		// パタン種別
									 (*pattern).getWeight());	// パタン重み
#ifdef V1_6
								 uniTerm.setLangSpec(langSpec);
#endif
								 table->insertTerm(uniTerm, 1);
							}

							// 前置語のリセット
							preGram.clear();

							// 隣接語となった時のみ単独語との重複を禁止
							// (例えば名詞.接尾辞は単独語にもマッチする)
							goto there;
						}
					}
					else
					// 前置語
					if((*pattern).getID() == 4) {

						// 検索語文字列の獲得
						preGram = (*pattern).replace();

						// 禁止辞書の検査
						if(useStopDict == ModTrue) {
							if(resource->stopDict->isFound(preGram) == ModTrue)
							{
								// 前置語をリセットして次のレコードへ
								preGram.clear();
								goto there;
							}
						}

						// 検索語テーブルへ前置語を登録 (useUniGramにかかわらず)
						if(table != 0) {
							ModTermElement uniTerm(
								preGram,					// 検索語文字列
								(*pattern).getType(),		// パタン種別
								(*pattern).getWeight());	// パタン重み
#ifdef V1_6
							uniTerm.setLangSpec(langSpec);
#endif
							table->insertTerm(uniTerm, 1);
						}

						// 前置語の更新
						preType	 = (*pattern).getType();
						preWeight = (*pattern).getWeight();

						// 前置語となった時は単独語との重複を禁止
						// (例えば名詞.接頭辞は単独語にもなる)
						found = ModTrue;
						goto there;
					}
					else
					// 単独語
					if((*pattern).getID() == 5) {

						// 検索語文字列の獲得
						uniGram = (*pattern).replace();

						// 禁止辞書の検査
						if(useStopDict == ModTrue) {
							if(resource->stopDict->isFound(uniGram) == ModTrue)
							{
								// 前置語をリセットして次のレコードへ
								preGram.clear();
								goto there;
							}
						}

						// 前置語があれば隣接語の生成
						if(useBiGram == ModTrue && preGram.getLength() != 0) {

							ModTermElement biTerm(
											preGram,
											preType,
											preWeight,
											uniGram,
											(*pattern).getType(),
											(*pattern).getWeight());
#ifdef V1_6
							biTerm.setLangSpec(langSpec);
#endif

							// 検索語プールへの登録
							pool.insertTerm(biTerm);

							// 検索語テーブルへの登録
							if(table != 0) {
							 table->insertTerm(biTerm, 2);
							}
						}

						// 単独語の生成
						ModTermElement uniTerm(
							uniGram,				// 検索語文字列
							(*pattern).getType(),	// パタン種別
							(*pattern).getWeight());	// パタン重み
#ifdef V1_6
						uniTerm.setLangSpec(langSpec);
#endif
						// 検索語の登録
						if(useUniGram == ModTrue) {

							pool.insertTerm(uniTerm);
						}
						
						// 検索語テーブルにはuseUniGramに関わらず登録
						if(table != 0) {
							table->insertTerm(uniTerm, 1);
						}

						// 前置語の更新
						preGram	 = uniGram;
						preType	 = (*pattern).getType();
						preWeight = (*pattern).getWeight();
						found = ModTrue;

						// 別のパタン適用を停止 (相関語停止を前提)
						goto there;
					}
#if 0
					// 相関語の処理を停止
					else
					// 禁止相関語
					if((*pattern).getID() == 6) {
						// 以降の相関語パタンの適用を行わない
						break;
					}
					else
					// 相関語
					if((*pattern).getID() == 7 && useBiGram == ModTrue) {

						// 検索語の語形は、係受け関係にある２つの検索語を
						// 半角spaceで区切り連結した文字列。
						uniGram = (*pattern).replace();

						// 検索語の生成
						ModTermElement coTerm(
							uniGram,
							(*pattern).getType(), // 相関語固有の種別とする
							(*pattern).getWeight());
#ifdef V1_6
						coTerm.setLangSpec(langSpec);
#endif

						// 検索語の登録
						pool.insertTerm(coTerm);

						// 以降の相関語パタンの適用を行わない
						break;
					}
#endif
				}
			}
			// 検索語が見付からない場合
			if(found == ModFalse) {
				preGram.clear();
			}

there:
			// 次のレコード頭に進める	
			offset = _nextTerm(analysis, offset);

			// 解析した語数の総計
			analyzedLength++;
		}

		// 最大解析長に達したら終了。
		if(maxText != 0 && analyzedLength > maxText) {
			break;
		}
	}

	// パタン辞書を用いた旧版の終り
	return;
	}
	
	// 検索語タイプ表を用いた高速化バージョン
version1:
	preGram.clear();
	preOriginal.clear();

	ModVector<ModUnicodeString> formVector;	// 形態素語形ベクトル
	ModVector<ModUnicodeString> ostrVector;	// 形態素語形ベクトル(元表記)
	ModVector<int>				posVector;  // 形態素品詞ベクトル

	// 対象テキストの設定
#ifdef SYD_USE_UNA_V10
	parseNormMode(termUnit);
	m_cUnaParam[_DoNorm] = (useNormalizer == ModTrue) ? _True : _False;
#if 0
	m_cUnaParam[_GetOrg] = _True;
	m_cUnaParam[_GetVec] = _True;
#endif
	analyzer->prepare(m_cUnaParam);
	analyzer->set(text, langSpec);
#else
#ifdef V1_6
	analyzer->set(text, termUnit, langSpec);
#else
	analyzer->set(text, termUnit);
#endif
#endif

	// ダミー検索語
	ModTermElement dummyTerm("", 0, 0);
#ifdef V1_6
	dummyTerm.setLangSpec(langSpec);
#endif

	// 解析ブロック毎に
#ifdef SYD_USE_UNA_V10
	while (analyzer->getBlock(formVector, ostrVector, posVector) == ModTrue) {
#else
	while(analyzer->getBlock(formVector, ostrVector, posVector, useNormalizer)
		  == ModTrue) {
#endif

			ModVector<ModUnicodeString>::Iterator form = formVector.begin();
			ModVector<ModUnicodeString>::Iterator ostr = ostrVector.begin();
			ModVector<int>::Iterator			  pos  = posVector.begin();

			// 検索語毎に
			for(; pos != posVector.end();
				++pos, ++form, ++ostr, ++analyzedLength) {

				// 品詞コードから検索語タイプへ
				ModTermType type = (* (resource->termTypeTable))[*pos];

				// 禁止語
				if(type == TERM_STOP) {

					// 前置語をリセットして次の検索語へ
					preGram.clear();
					goto dummy;

				}

				// 区切語
				if(type == TERM_DELIM) {

					// 前置語はリセットせず次の検索語へ
					continue;

				}

				// 数詞
				if(type == TERM_NUMBER) {

#if 0				// １文字以外は用いない
					if(form->getLength() != 1) {
						preGram.clear();
						continue;
					}
					if(preGram.getLength() != 0) {
						type = TERM_SUFFIX; // 接尾に
					} else {
						type = TERM_PREFIX; // 接頭に
					}

#else
					// 全て用いない。前置語をリセットして次の検索語へ
					preGram.clear();
					goto dummy;
#endif

				// １文字のアルファベット
				} else if(type == TERM_ALPHABET	&& form->getLength() == 1) {

#if 1
					if(preGram.getLength() != 0) {
						type = TERM_SUFFIX; // 接尾に
					} else {
						type = TERM_PREFIX; // 接頭に
					}

#else
					// 全て用いない。前置語をリセットして次の検索語へ
					preGram.clear();
					goto dummy;
#endif

				// １文字の記号
				} else if(type == TERM_SYMBOL && form->getLength() == 1) {

					// 空白文字は区切り語 - 前置語はリセットせず次の検索語へ
					if(form->compare(" ", 1) == 0) {
						continue;
					}

					// それ以外は用いない。前置語をリセットして次の検索語へ
					preGram.clear();
					goto dummy;

				}

				// 後置語
				if(type == TERM_NOUN_S || type == TERM_SUFFIX) {

					// 前置語があれば隣接語の生成 (useBiGramのチェックは済み)
					// たとえ禁止語でも後接側ならば隣接語を生成する
					if(preGram.getLength() != 0) {

						// 隣接語を合成
						ModTermElement biTerm(
							 preGram, preType, preWeight, *form, type, 1);

						// 元表記のセット
						preOriginal.append(ModTermElement::termSeparator);
						preOriginal.append(*ostr);
						biTerm.setOriginalString(preOriginal);

#ifdef V1_6
						biTerm.setLangSpec(langSpec);
#endif

						// 検索語プールへの登録
						pool.insertTerm(biTerm);

						// 検索語テーブルへの登録
						if(table != 0) {
							table->insertTerm(biTerm, 2);
						}

						// 前置語をリセットして次の検索語へ
						preGram.clear();
						goto there2;
					}

					// 隣接語を構成しなかった場合 (前置語は空なのでリセット不要)
					//
					// (1) 接尾ならば次の検索語へ
					//
					if(type == TERM_SUFFIX) {
						goto there2;
					}
					//
					// (2) 名詞.接尾ならば単独語としても登録
					//

					// 体言性の前置語
					} else if(type == TERM_NOUN_P) {

						// 隣接語を構成できるなら (たとえ禁止語でも)
						if(useBiGram == ModTrue && (pos+1) != posVector.end()
						&& (* (resource->termTypeTable))[*(pos+1)] != TERM_STOP) {
							preGram		 = *form;
							preOriginal = *ostr;
							preType		 = type;
							preWeight	 = 1;
							goto there2;
					}
					// そうでなければ単独語

				// 前置語
				} else if(type == TERM_PREFIX) {

					// 前置語をリセット
					preGram.clear();

					// 禁止辞書の検査 (2003/1/23追加)
					if(useStopDict == ModTrue
					&& resource->stopDict->isFound(*form) == ModTrue) {
						goto dummy;
					}

					if(useBiGram == ModTrue) {
						preGram		 = *form;
						preOriginal = *ostr;
						preType		 = type;
						preWeight	 = 1;
					}
					goto there2;
				}

				// 単独語
				{

				// 前置語があれば隣接語の生成 (useBiGramのチェックは済み)
				// 禁止語でも後接側ならば隣接語を生成する
				if(preGram.getLength() != 0) {

					ModTermElement biTerm(
					preGram, preType, preWeight, *form, type, 1);

					// 元表記のセット
					preOriginal.append(ModTermElement::termSeparator);
					preOriginal.append(*ostr);
					biTerm.setOriginalString(preOriginal);

#ifdef V1_6
					biTerm.setLangSpec(langSpec);
#endif

					// 検索語プールへの登録
					pool.insertTerm(biTerm);

					// 検索語テーブルへの登録
					if(table != 0) {
						table->insertTerm(biTerm, 2);
					}
				}

				// 前置語をリセット
				preGram.clear();

#if 0			// 禁止語の場合は隣接語を構成しないことにした 2003/1/25
				// そのため禁止辞書の検索後に移動

				// 前置語の更新 (※)
				if(useBiGram == ModTrue) {
					preGram		 = *form;
					preOriginal = *ostr;
					preType		 = type;
					preWeight	 = 1;
				}
#endif

				// 禁止辞書の検査
				if(useStopDict == ModTrue
					&& resource->stopDict->isFound(*form) == ModTrue) {
					goto dummy;
				}

				// 前置語の更新 (上記※から移動)
				if(useBiGram == ModTrue) {
					preGram		 = *form;
					preOriginal = *ostr;
					preType		 = type;
					preWeight	 = 1;
				}

				// 単独語の生成
				ModTermElement uniTerm(*form, type, 1);

				// 元表記のセット
				uniTerm.setOriginalString(*ostr);

#ifdef V1_6
				uniTerm.setLangSpec(langSpec);
#endif

				// 検索語の登録
				if(useUniGram == ModTrue) {
					pool.insertTerm(uniTerm);
				}

				// 検索語テーブルへ登録 (useUniGramにかかわらず)
				if(table != 0) {
					table->insertTerm(uniTerm, 1);
				}

				continue;
			}

			// 検索語テーブルへ登録 (useUniGramにかかわらず)
there2:		if(table != 0) {
				ModTermElement uniTerm(*form, type, 1);
				uniTerm.setOriginalString(*ostr);
#ifdef V1_6
				uniTerm.setLangSpec(langSpec);
#endif
				table->insertTerm(uniTerm, 1);

			}
			continue;

			// 検索語テーブルへダミーを登録
dummy:		if(table != 0) {
				table->insertTerm(dummyTerm, 1);
			}
		}

		// 最大解析長に達したら終了。
		if(maxText != 0 && analyzedLength > maxText) {
			break;
		}
	}
}

//
// FUNCTION public
// ModTerm::poolTerm -- 初期検索語をプール
//
// NOTES
//	 検索語生成法 termMethod1 の値に応じて初期検索語の生成法を
//	 切替える。
//
// ARGUMENTS
//	 const ModUnicodeString& text			検索要求
//	 ModTermPool&						pool			検索語プール
//	 ModTermTable*					 table		 検索語テーブル
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
void
ModTerm::poolTerm(
	const ModUnicodeString& text,		 // 検索要求
#ifdef V1_6
	const ModLanguageSet& langSpec,	 // 言語指定
#endif
	ModTermPool&						pool,		 // 検索語プール
	ModTermTable*					 table)		// 検索語テーブル
{
	if(termMethod1 == mgramMethod) {
		poolTermMgram(
		text,
#ifdef V1_6
		langSpec,
#endif
		pool);
	} else if(termMethod1 == cgramMethod) {
		poolTermCgram(
		text,
#ifdef V1_6
		langSpec,
#endif
		pool);
	} else {
		poolTermDefault(
		text,
#ifdef V1_6
		langSpec,
#endif
		pool,
		table);
	}
}

//
// FUNCTION public
// ModTerm::poolTermDefault -- 初期検索語をプール(デフォルト)
//
// NOTES
//	 検索要求から検索語群を生成し検索語プールに登録する。
//	 検索語プール中の各初期検索語には以下がセットされる。
//	 - 重み(weight)
//	 - スケール(scale)
//	 - 出現頻度(tf)
//	 - 重みづけ値(twv)
//	 - 選択値 (tsv, 出現頻度と同一値)
//	 - 初期検索パラメタ群
//
// ARGUMENTS
//	 const ModUnicodeString& text			検索要求
//	 ModTermPool&						pool			検索語プール
//	 ModTermTable*					 table		 検索語テーブル
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
void
ModTerm::poolTermDefault(
	const ModUnicodeString& text,		 // 検索要求
#ifdef V1_6
	const ModLanguageSet& langSpec,	 // 言語指定
#endif
	ModTermPool&						pool,		 // 検索語プール
	ModTermTable*					 table)		// 検索語テーブル
{
	// 検索要求を検査
	if(text.getLength() == 0) {
		return;
	}

	// 検索語候補 (出現頻度による選択精度を高めるため大きなプールを用いる)
	ModTermPool candidate(maxCandidate);

	// 検索語をプール
	this->poolTerm(
		text,
#ifdef V1_6
		langSpec,
#endif
		useUniGram1,
		useBiGram1,
		useStopDict1,
		useNormalizer1,
		termUnit1,
		maxText1,
		candidate,
		table);

	// 検索語が１つも得られない事態を回避
	if(candidate.getSize() == 0 && failsafe1 == ModTrue) {
		// 禁止辞書を使用しない
		this->poolTerm(
			text,
#ifdef V1_6
			langSpec,
#endif
			useUniGram1,
			useBiGram1,
			ModFalse,
			useNormalizer1,
			termUnit1,
			maxText1,
			candidate,
			table);
		// それでも得られない場合はテキストを１つの検索語とする
		if(candidate.getSize() == 0) {
			ModTermElement term(text);
#ifdef V1_6
			term.setLangSpec(langSpec);
#endif
			candidate.insertTerm(term);
			if(table != 0) {
				table->insertTerm(term);
			}
		}
	}

	// 検索語テーブルの文書切替え
	if(table != 0) {
		table->switchDocument();
	}

	// 初期検索語の選択
	ModTermPool::Iterator t;
	for(t = candidate.begin(); t != candidate.end(); ++t) {

		// 選択値のセット
		const ModUnicodeString& form = t->getString();
		double tsv = SQRT(form.getLength()) * t->getTf();
		t->setTsv(tsv);

		// 初期検索語プールに登録
		pool.insertTerm(t->getTerm());
	}

	// 初期検索パラメタのデフォルト
	double paramWeight				= paramWeight1;
	double paramScore				 = paramScore1;
	double paramLength				= paramLength1;
	int		paramProximity		 = paramProximity1;
	double paramScale				 = paramScale1;
	double paramCombineBiGram = paramCombineBiGram1;

	// 初期検索パラメタの調整
	if(adaptWeight1 != 0) {
		paramWeight = adaptWeight1 / (pool.getNumUniGram() - adaptWeight1);
	}
	if(pool.getNumBiGram() != 0) {
#if 0
		paramCombineBiGram /= pool.getNumBiGram();
#else
		;
#endif
	}

	// 各初期検索語について
	for(t = pool.begin(); t != pool.end(); ++t) {

		// 重みをセット (デフォルト値を使用)
		t->setWeight(0);

		// スケールをセット

#if 1 // nextModel
		double scale = t->getTwv() * t->getTf() / (t->getTf() + paramScale);
#else // nextModel2
		double scale = t->getTwv() * t->getTf();
#endif

		// 隣接語の結合パラメタを加味
		if(t->isBiGram() == ModTrue) {
			scale *= paramCombineBiGram;
		}
		t->setScale(scale);

		// 初期検索パラメタをセット
		t->setParamWeight(paramWeight);
		t->setParamScore(paramScore);
		t->setParamLength(paramLength);
		if(t->isBiGram()) {
			t->setParamProximity(paramProximity);
		}
	}

	// 全て同一スケールであればリセット
	ModBoolean resetScale = ModFalse;
	if(pool.getSize() == 0) {
		;
	} else if(pool.getSize() == 1) {
		resetScale = ModTrue;
	} else {
		t = pool.begin();
		double scale = t->getScale();
		for(++t; t != pool.end(); ++t) {
			if(scale != t->getScale()) {
				break;
			}
		}
		if(t == pool.end()) {
			resetScale = ModTrue;
		}
	}
	if(resetScale == ModTrue) {
		for(t = pool.begin(); t != pool.end(); ++t) {
			t->setScale(0);
		}
	}

#if 0	// 高速化のため停止
	// 選択値(出現頻度と同じ値)の順にソート(必須ではない)
	pool.sortByTsv();
#endif
}

void ModTerm::parseNormMode(const ModUnicodeString& termUnit)
{
	m_cUnaParam[_Stem] = _False;
	m_cUnaParam[_Carriage] = _False;
	m_cUnaParam[_Compound] = _False;
	
	if (termUnit == _ModNlpNormOnly)
	{
		// nothing to do
	}
	else if (termUnit == _ModNlpNormRet)
	{
		m_cUnaParam[_Carriage] = _True;
	}
	else if (termUnit == _ModNlpNormRetStemDiv)
	{
		m_cUnaParam[_Stem] = _True;
		m_cUnaParam[_Carriage] = _True;
		m_cUnaParam[_Compound] = _True;	
	}
	else if (termUnit == _ModNlpNormRetDiv)
	{
		m_cUnaParam[_Carriage] = _True;
		m_cUnaParam[_Compound] = _True;	
	}
}

//
// FUNCTION public
// ModTerm::poolTermMgram -- 初期検索語をプール(形態素n-gram)
//
// NOTES
//	 形態素n-gramの検索語を生成する。
//
// ARGUMENTS
//	 const ModUnicodeString&	text			検索要求
//	 ModTermPool&				pool			検索語プール
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
void
ModTerm::poolTermMgram(
	const ModUnicodeString& text,		 // 検索要求
#ifdef V1_6
	const ModLanguageSet& langSpec,	 // 言語指定
#endif
	ModTermPool&						pool)		 // 検索語プール
{
	// 検索要求を検査
	if(text.getLength() == 0) {
		return;
	}

	// 検索要求内の形態素列を得る
	ModVector<ModUnicodeString> mvec;	// 検索要求内の形態素列
	ModVector<ModUnicodeString> bvec;	// ブロック内の形態素列
	ModVector<int> pvec;							 // ブロック内の品詞番号列

#ifdef SYD_USE_UNA_V10
	parseNormMode(termUnit1);
	m_cUnaParam[_DoNorm] = _True;// _False;
	analyzer->prepare( m_cUnaParam);
	analyzer->set(text,langSpec);
#else
#ifdef V1_6
	analyzer->set(text, termUnit1, langSpec);
#else
	analyzer->set(text, termUnit1);
#endif
#endif

	// 正規化しないので元表記のセットはしない
#ifdef SYD_USE_UNA_V10
	ModVector<ModUnicodeString> dummy;
	while (analyzer->getBlock(bvec, dummy, pvec) == ModTrue) {
#else
	while(analyzer->getBlock(bvec, pvec, ModFalse) == ModTrue) {
#endif
		ModVector<ModUnicodeString>::Iterator b;
		for(b=bvec.begin(); b!=bvec.end(); ++b) {
			mvec.pushBack(*b);
		}
		// 最大長の制限
		if(mvec.getSize() > maxText1) {
			break;
		}
	}

	// 形態素n-gramを生成
	ModVector<ModUnicodeString>::Iterator x;
	ModVector<ModUnicodeString>::Iterator y;
	for(x=mvec.begin(); x != mvec.end(); ++x) {
		if(x->getLength() > 1) {
			ModTermElement t(*x);
#ifdef V1_6
			t.setLangSpec(langSpec);
#endif
			pool.insertTerm(t);
		} else {
			// 空白から始まる検索語を除く
			ModUnicodeString& s = *x;
			if(ModUnicodeCharTrait::isSpace(s[0]) == ModTrue) {
				continue;
			}
		}
		ModUnicodeString& mgram = *x;
		ModSize length = 1;
		for(y = x+1; y != mvec.end() && length < maxTermLength1; ++y) {
			mgram += *y;
			// 空白で終る検索語を除く
			if(y->getLength() == 1) {
				ModUnicodeString& s = *y;
				if(ModUnicodeCharTrait::isSpace(s[0]) == ModTrue) {
					continue;
				}
			}
			ModTermElement t(mgram);
#ifdef V1_6
			t.setLangSpec(langSpec);
#endif
			pool.insertTerm(t);
			length++;
		}
	}

	// 初期検索パラメタをセット
	for(ModTermPool::Iterator t = pool.begin(); t != pool.end(); ++t) {
		t->setWeight(0);
		t->setScale(0);
		t->setParamWeight(paramWeight1);
		t->setParamScore(paramScore1);
		t->setParamLength(paramLength1);
	}
}

//
// FUNCTION public
// ModTerm::poolTermCgram -- 初期検索語をプール(文字n-gram)
//
// NOTES
//	 文字n-gramの検索語を生成する。
//
// ARGUMENTS
//	 const ModUnicodeString& text			検索要求
//	 ModTermPool&						pool			検索語プール
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
void
ModTerm::poolTermCgram(
	const ModUnicodeString& text,		 // 検索要求
#ifdef V1_6
	const ModLanguageSet& langSpec,	 // 言語指定
#endif
	ModTermPool&						pool)		 // 検索語プール
{
	// 検索要求を検査
	if(text.getLength() == 0) {
		return;
	}

	if(text.getLength() == 1) {
		ModTermElement t(text);
#ifdef V1_6
		t.setLangSpec(langSpec);
#endif
		pool.insertTerm(t);
	} else {
		for(ModSize start=0; start < text.getLength() - 1 && start < maxText1;
				++start) {
			// 空白から始まる検索語を除く
			if(ModUnicodeCharTrait::isSpace(text[start]) == ModTrue) {
				continue;
			}
			ModUnicodeString ngram(text[start]);
			for(ModSize length = 1;
					length < text.getLength() - start && length < maxTermLength1;
					++length) {
				ngram.append(text[start + length]);
				// 空白で終る検索語を除く
				if(ModUnicodeCharTrait::isSpace(text[start + length]) == ModTrue) {
					continue;
				}
				ModTermElement t(ngram);
#ifdef V1_6
				t.setLangSpec(langSpec);
#endif
				pool.insertTerm(t);
			}
		}
	}

	// 初期検索パラメタをセット
	for(ModTermPool::Iterator t = pool.begin(); t != pool.end(); ++t) {
		t->setWeight(0);
		t->setScale(0);
		t->setParamWeight(paramWeight1);
		t->setParamScore(paramScore1);
		t->setParamLength(paramLength1);
	}
}

//
// FUNCTION public
// ModTerm::poolTerm -- 拡張検索語の候補をプール
//
// NOTES
//	 検索語マップから拡張検索語の候補を選択し検索語プールに登録する。
//
// ARGUMENTS
//	 ModTermMap&	 map			検索語マップ
//	 ModTermPool&	pool		 検索語プール (拡張語の候補群)
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
void
ModTerm::poolTerm(
	ModTermMap&	 map,	 // 検索語マップ
	ModTermPool&	pool)	// 検索語プール (拡張語の候補群)
{
	 // 検索語マップ中の検索語毎に
	for(ModTermMap::Iterator m = map.begin(); m != map.end(); ++m) {

		// 出現シード文書数の検査
		ModSize r = ((*m).second).getSize();
		if(r <= minSeedDf2) {
			continue;
		}

		// 代表検索語
		ModTermElement& term = ((*m).second).getTerm();

		// 単独語の検査
		if(term.isBiGram() != ModTrue && useUniGramExpansion2 == ModFalse) {
			continue;
		}

		// 隣接語の検査
		if(term.isBiGram() == ModTrue && useBiGramExpansion2 == ModFalse) {
			continue;
		}

		// 禁止タイプ辞書の検査
		if(resource->stopTypeDict->isFound(term.getType())) {
			continue;
		}

		// 禁止拡張辞書の検査
		const ModUnicodeString& form = term.getString();
		if(resource->stopExpansionDict->isFound(form)) {
			continue;
		}

		// 一文字語の検査
		if(form.getLength() == 1 && useSingleChar2 == ModFalse) {
			continue;
		}

		// 平均出現頻度をセット
		double tf = 0;
		ModTermPosting::Iterator p = ((*m).second).begin();
		while(p != ((*m).second).end()) {
			tf += (double)((*p).second);
			++p;
		}
		tf /= r; // 常に r > 0
		term.setTf(tf);

		// 出現シード文書数をセット
		term.setSdf(r);

		// 選択値(出現シード文書数)をセット
		term.setTsv(r);

		// 拡張語候補として登録
		pool.insertTerm(term);
	}
}

//
// FUNCTION public
// ModTerm::mapTerm -- 検索語をマップ
//
// NOTES
//	 シード文書から検索語群を生成し検索語マップに登録する。
//	 マップのシード文書数を更新(インクリメント)する。
//
// ARGUMENTS
//	 const ModUnicodeString&	doc			シード文書
//	 const ModSize				docID		シード文書番号
//	 ModTermMap&				map			検索語マップ
//	 ModTermTable*				table		検索語テーブル
//
// RETURN
//	 なし
//
// EXCEPTIONS
//

void
ModTerm::mapTerm(
	const ModUnicodeString&		 doc,	// シード文書
#ifdef V1_6
	const ModLanguageSet& langSpec,		// 言語指定
#endif
	const ModSize				 docID,	// シード文書番号
	ModTermMap&					 map,	// 検索語マップ
	ModTermTable*				 table)	// 検索語テーブル
{
	// シード文書中の検索語をプール
	ModTermPool pool(maxCandidate);
	this->poolTerm(
		doc,
#ifdef V1_6
		langSpec,
#endif
		useUniGram2,
		useBiGram2,
		useStopDict2,
		useNormalizer2,
		termUnit2,
		maxText2,
		pool,
		table);
	// 検索語が１つも得られない事態を回避
	if(pool.getSize() == 0 && failsafe2 == ModTrue) {
		// 禁止辞書を使用しない
		this->poolTerm(
			doc,
#ifdef V1_6
			langSpec,
#endif
			useUniGram2,
			useBiGram2,
			ModFalse,
			useNormalizer2,
			termUnit2,
			maxText2,
			pool,
			table);
	}

	// テーブルの文書切替え
	if(table != 0) {
		table->switchDocument();
	}

	// 検索語をマップに登録
	for(ModTermPool::Iterator t = pool.begin(); t != pool.end(); ++t) {
		map.insertTerm(docID, t->getTerm());
	}

	// シード文書数を更新
	map.setNumDocs(map.getNumDocs() + 1);
}

//
// FUNCTION public
// ModTerm::mapTerm -- 検索語をマップ (混合適合性フィードバック用)
//
// NOTES
//	 シード文書から検索語群を生成し検索語マップに登録する。
//	 ただし支持集合中の検索語のみをマップに登録する。
//	 マップのシード文書数を更新(インクリメント)する。
//
// ARGUMENTS
//	 const ModUnicodeString&	doc			シード文書
//	 const ModSize				docID		シード文書番号
//	 ModTermMap&				map			検索語マップ
//	 ModTermPool&				support		検索語プール(支持集合)
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
void
ModTerm::mapTerm(
	const ModUnicodeString&	 doc,	// シード文書
#ifdef V1_6
	const ModLanguageSet& langSpec,	// 言語指定
#endif
	const ModSize			 docID,	// シード文書番号
	ModTermMap&				 map,	// 検索語マップ
	ModTermPool&			support)// 検索語プール(支持集合)
{
	// シード文書中の検索語をプール
	ModTermPool pool(maxCandidate);
	this->poolTerm(
		doc,
#ifdef V1_6
		langSpec,
#endif
		useUniGram2,
		useBiGram2,
		useStopDict2,
		useNormalizer2,
		termUnit2,
		maxText2,
		pool);

	// 検索語が１つも得られない事態を回避
	if(pool.getSize() == 0 && failsafe2 == ModTrue) {
		// 禁止辞書を使用しない
		this->poolTerm(
			doc,
#ifdef V1_6
			langSpec,
#endif
			useUniGram2,
			useBiGram2,
			ModFalse,
			useNormalizer2,
			termUnit2,
			maxText2,
			pool);
	}

	// 検索語をマップに登録
	for(ModTermPool::Iterator t = pool.begin(); t != pool.end(); ++t) {
		// 支持集合中の検索語のみ
		if(support.map.find(t->getString()) != support.map.end()) {
			map.insertTerm(docID, t->getTerm());
		}
	}

	// シード文書数を更新
	map.setNumDocs(map.getNumDocs() + 1);
}

//
// FUNCTION public
// ModTerm::weightTerm -- 初期検索語を重み付け
//
// NOTES
//	 検索語マップに表現されたフィードバック情報を基に、
//	 検索語プール中の各初期検索語について以下を行う。
//	 - 重み(weight)
//	 - スケール(scale)
//	 - シード文書での平均出現頻度(tf)
//	 - 選択値 (tsv)
//	 - 拡張検索パラメタ群
//
//	 さらに
//	 - 全ての初期検索語を検索語マップからの消去
//	 - 選択値が零の初期検索語を検索語プールから消去
//	 - 検索語マップの信頼度(confidence)をセット
//
// ARGUMENTS
//	 ModTermMap&	 map			検索語マップ
//	 ModTermPool&	pool		 検索語プール (初期検索語群)
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
void
ModTerm::weightTerm(
	ModTermMap&	 map,	 // 検索語マップ
	ModTermPool&	pool)	// 検索語プール (初期検索語群)
{
	// シード文書数と登録文書数
	const ModSize R = map.getNumDocs();
	const ModSize N = collectionSize;
	if(R == 0 || N == 0) {
		return;
	}

	// 初期検索語パラメタのデフォルト
	double paramWeight				= paramWeight1;
	double paramScore				 = paramScore1;
	double paramLength				= paramLength1;
	int		paramProximity		 = paramProximity1;
	double paramScale				 = paramScale1;
	double paramCombineBiGram = paramCombineBiGram1;

	// 混合パラメタのデフォルト
	double paramMixUniGram		= paramMixUniGram1;
	double paramMixBiGram		 = paramMixBiGram1;

	// 初期検索語パラメタの調整
	if(adaptWeight1 != 0) {
		paramWeight = adaptWeight1 / (pool.getNumUniGram() - adaptWeight1);
	}
	if(pool.getNumBiGram() != 0) {
		paramCombineBiGram /= pool.getNumBiGram();
	}

	// 重み正規化定数
	const double maxWeight1 = LOG(paramWeight * N + 1);
	const double maxWeight2 = LOG(((R+0.5)/0.5)/(0.5/N));
	if(maxWeight1 <= 0 || maxWeight2 <= 0) {
		return;
	}

	// 検索語毎に
	ModTermPool::Iterator t;
	for(t = pool.begin(); t != pool.end(); ++t) {

		// 文書頻度 (初期検索結果の分析時にセット済)
		double n = t->getDf();
		if(n == 0) {
			t->setTsv(0);
			continue;
		}

		// 拡張検索パラメタのセット
		t->setParamWeight(paramWeight);
		t->setParamScore(paramScore);
		t->setParamLength(paramLength);
		if(t->isBiGram()) {
			t->setParamProximity(paramProximity);
		}

		// 出現シード文書数(r)と平均出現頻度(tf)を求める
		double r	= 0.5;
		double tf = 0.5;

		// シード文書から生成しなかった場合
		if((t->isBiGram() == ModTrue && useBiGram2	== ModFalse)
		 ||(t->isBiGram() != ModTrue && useUniGram2 == ModFalse)) {

			// 選択値のみセットしその他は初期検索時のまま
			t->setTsv(0.1);
			continue;

		// シード文書から生成した場合
		} else {

			ModTermMap::Iterator m = map.findTerm(t->getTerm());
			if(m != map.end()) {

				// 出現シード文書数
				r = (double)(((*m).second).getSize());

				// 平均出現頻度
				ModTermPosting::Iterator p = ((*m).second).begin();
				while(p != ((*m).second).end()) {
					tf += (double)((*p).second);
					++p;
				}
				tf /= r; // 常に r > 0

				// マップから消去
				map.erase(m);
			}
		}
		t->setSdf(r);
		t->setTf(tf);

		// 初期検索時の重み
		double w1 = LOG((paramWeight * N) / n + 1) / maxWeight1;
		if(w1 < 0) w1 = 0;

		// シード文書での重み
		// [NOTE] temp1は、非シード文書で、ある検索語を含む文書数を表し、
		//  temp2は、非シード文書で、ある検索語を含まない文書数を表しているが、
		//  シード文書は検索対象集合から収集されることが想定されていたと思われる。
		//  シード文書が検索対象集合から収集されない場合や、
		//  nolocationオプションで、nとrのカウント方法が異なる場合は、
		//  temp1, temp2がそれぞれ負になる可能性がある。
		double temp1 = n-r;
		if (temp1 < 0) temp1 = 0;
		double temp2 = N-n-R+r;
		if (temp2 < 0) temp2 = 0;
		double w2 = LOG(((r+0.5)/(R-r+0.5))/((temp1+0.5)/(temp2+0.5))) / maxWeight2;
		if(w2 < 0) w2 = 0;

		// 重みの混合パラメタ
		double paramMix = t->isBiGram()==ModTrue ? paramMixBiGram : paramMixUniGram;

		// 重みを混合してセット
		double weight = (1 - paramMix) * w1 + paramMix * w2;
		t->setWeight(weight);

		// 選択値をセット
		double tsv =	weight * (r/R - n/N);
		if(tsv < 0) tsv = 0;
		t->setTsv(tsv);

		// 平均出現頻度を用いてスケールを計算
		double scale = t->getTwv() * t->getTf() / (t->getTf() + paramScale);

		// 隣接語の結合パラメタを加味
		if(t->isBiGram() == ModTrue) {
			scale *= paramCombineBiGram;
		}
		t->setScale(scale);
	}

#if 0 // 2003/1/30
	// 選択値(0)の検索語を消去 (検索語は選択値順にソートされる)
	pool.eraseTerm(0);
#endif

	// マップの信頼度 (選択値の平均) をセット
	double averageTsv(0);
	ModSize count(0);
	for(t = pool.begin(); t != pool.end(); ++t) {
		averageTsv += t->getTsv();
		++count;
	}
	if(count != 0) {
		averageTsv /= count;
	}
	map.setConfidence(averageTsv);
}

//
// FUNCTION public
// ModTerm::selectTerm -- 拡張語を選択
//
// NOTES
//	 検索語プール中の各拡張語候補から拡張語を選択し検索語プールに登録する。
//	 各拡張語には以下がセットされる。
//	 - 重み(weight)
//	 - スケール(scale)
//	 - シード文書での平均出現頻度(tf)
//	 - 選択値 (tsv)
//	 - 拡張検索パラメタ群
//
// ARGUMENTS
//	 ModTermMap&	 map,				検索語マップ
//	 ModTermPool&	candidate	 拡張語の候補群
//	 ModTermPool&	pool				拡張語群
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
void
ModTerm::selectTerm(
	ModTermMap&	 map,			 // 検索語マップ
	ModTermPool&	candidate, // 拡張語の候補群
	ModTermPool&	pool)			// 拡張語群
{
	// シード文書数と登録文書数
	const ModSize R = map.getNumDocs();
	const ModSize N = collectionSize;
	if(R == 0 || N == 0) {
		return;
	}

	// 拡張語数の決定
	if(maxTerm2 > minTerm2) {
		// [NOTE] maxTerm2には、EXTPAND (FROM ... ORDER BY WORD ... LIMIT ...)
		//  のLIMIT値が設定されている。
		//  参照 ModTerm(), SearchCapsule::initializeTerm(),
		//   OpenOption::parseExpand()
		ModSize numTerm2 = minTerm2;
		numTerm2 += (ModSize)((maxTerm2 - minTerm2) * map.getConfidence());
		pool.reSize(numTerm2);
	}

	// 拡張語パラメタのデフォルト
	double paramWeight			= paramWeight2;
	double paramScore			= paramScore2;
	double paramLength			= paramLength2;
	double paramScale			= paramScale2;
	int		paramProximity		= paramProximity2;

	double paramCombineUniGram	= paramCombineUniGram2;
	double paramCombineBiGram	= paramCombineBiGram2;

	double paramMixUniGram		= paramMixUniGram2;
	double paramMixBiGram		= paramMixBiGram2;

	// 重み正規化定数
	const double maxWeight1 = LOG(paramWeight * N + 1);
	const double maxWeight2 = LOG(((R+0.5)/0.5)/(0.5/N));
	if(maxWeight1 <= 0 || maxWeight2 <= 0) {
		return;
	}

	// 候補語毎に
	ModTermPool::Iterator t;
	for(t = candidate.begin(); t != candidate.end(); ++t) {

		// 文書頻度 (外部でセット済)
		double n = t->getDf();
		if(n == 0) {
			t->setTsv(0);
			continue;
		}

		// 出現シード文書数(r)を求める
		double r	= t->getSdf();

		// コレクション全体での重み
		double w1 = LOG((paramWeight * N) / n + 1) / maxWeight1;
		if(w1 < 0) w1 = 0;

		// シード文書での重み
		// [NOTE] temp1,temp2の説明はweighTermを参照。
		double temp1 = n-r;
		if (temp1 < 0) temp1 = 0;
		double temp2 = N-n-R+r;
		if (temp2 < 0) temp2 = 0;
		double w2 = LOG(((r+0.5)/(R-r+0.5))/((temp1+0.5)/(temp2+0.5))) / maxWeight2;
		if(w2 < 0) w2 = 0;

		// 重みの混合パラメタ
		double paramMix
			= t->isBiGram() == ModTrue ? paramMixBiGram : paramMixUniGram;

		// 重みを混合してセット
		double weight = (1 - paramMix) * w1 + paramMix * w2;
		t->setWeight(weight);

		// 選択値を計算
		double tsv =	weight * (r/R - n/N);
		if(tsv < 0) tsv = 0;

		// 選択値をセット
		t->setTsv(tsv);
		// 登録
		pool.insertTerm(t->getTerm());
	}

	// 拡張語群が決定した後のパラメタ調整 - 現状では特になし

	// 拡張語毎に
	for(t = pool.begin(); t != pool.end(); ++t) {

		// 拡張検索パラメタのセット
		t->setParamWeight(paramWeight);
		t->setParamScore(paramScore);
		t->setParamLength(paramLength);
		if(t->isBiGram()) {
			t->setParamProximity(paramProximity);
		}

		// 平均出現頻度を用いてスケールを計算してセット
		double scale = t->getTwv() * t->getTf() / (t->getTf() + paramScale);

		// 結合パラメタを加味 (検索語数によらず)
		if(t->isBiGram() == ModTrue) {
			scale *= paramCombineBiGram;
		} else {
			scale *= paramCombineUniGram;
		}
		t->setScale(scale);
	}

	// 選択値の順にソート (必須)
	pool.sortByTsv();
}

//
// FUNCTION public
// ModTerm::analyzeResult -- 検索結果の分析
//
// NOTES
//	 検索結果の分析。
//	 現状では検索語の文書頻度(df)のセットのみ。
//
// ARGUMENTS
//	 ModTermElement&				term			検索語
//	 RankingResult&					result			検索結果(未ソート)
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
void
ModTerm::analyzeResult(
		ModTermElement&				term,	// 検索語
		ModInvertedSearchResult		*result)// 検索結果(未ソート)
{
	// 文書頻度をセット
	term.setDf(result->getSize());

	// ※ 文書スコアの調整について
	//		RankingResult (ModInvertedRankingResult) は以下を要素とする。
	//			文書番号、文書スコアの組
	//		これを
	//			文書番号、文書スコア、文書長の組、文書内頻度
	//		を要素とするよう変更することで文書スコア調整が実現可能となる。
}

//
// FUNCTION public
// ModTerm::mergeResult -- 検索結果のマージ
//
// NOTES
//	 ２つの検索結果をマージする。
//
// ARGUMENTS
//	 RankingResult&					result1	検索結果(文書番号順)
//	 RankingResult&					result2	検索結果(文書番号順)
//	 RankingResult&					result3	マージ結果(文書番号順)
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
void
ModTerm::mergeResult(
		ModInvertedSearchResult *x,	// 検索結果(文書番号順)
		ModInvertedSearchResult *y,	// 検索結果(文書番号順)
		ModInvertedSearchResult *z)	// マージ結果(文書番号順)
{
	// マージ実行
	x->setUnion(y,z);
}

//
// FUNCTION public
// ModTerm::mergeRank -- マージランク
//
// NOTES
//	 ２つの検索結果をマージし各ランクでスコアを計算する。
//
// ARGUMENTS
//	 RankingResult&	result1	検索結果(初期検索)
//	 RankingResult&	result2	検索結果(拡張検索)
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
void
ModTerm::mergeRank(
	ModInvertedSearchResult *x, //	検索結果(初期検索)
	ModInvertedSearchResult *y) //	検索結果(拡張検索)
{
	// 最大マージ数が0ならば何もしない
	if(maxRank1 == 0) {
		return;
	}

	// 初期検索結果の文書番号とそのランクをマップに登録
	ModTermDocIDMap map;
	ModSize rank1 = 1;
	ModSize i;
	for( i = 0; i < x->getSize(); ++i) {
		// 上位 maxRank1 文書まで
		if(rank1 > maxRank1) {
			break;
		}
		map.insert(x->getDocID(i), DocumentScore(rank1));
		++rank1;
	}

	// 拡張検索結果のスコアを変更
	ModSize rank2 = 1;
	for( i = 0; i < y->getSize(); ++i) {
		// 上位 maxRank2 文書まで
		if(rank2 > maxRank2) {
			break;
		}
		// 初期検索結果での順位 rank1 を取得
		rank1 = maxRank1 + 1;
		ModTermDocIDMap::Iterator iter = map.find(y->getDocID(i));
		if(iter != map.end()) {
			rank1 = (ModSize)((*iter).second);
		}
		// 混合ランクの逆数をスコアに
		DocumentScore score
			= 1.0 / (paramMixRank * rank2 + (1-paramMixRank) * rank1);

		y->setScore(i,score);
		// 次の文書
		++rank2;
	}
	// 再ランキング
	y->sort(0,i,_SYDNEY::Inverted::SortParameter::ScoreAsc);
}

//
// FUNCTION public
// ModTerm::validatePool -- 検索語プールの有効化
//
// NOTES
//	 - 検索語タイプに応じたスケール調整
//	 - 新語(スケールがゼロの語)に対するスケール設定
//	 - その他の検索語属性の設定
//
//
// ARGUMENTS
//	 ModTermPool&	pool	検索語プール
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
void
ModTerm::validatePool(
	ModTermPool&	pool)				// 検索語プール
{
	for(ModTermPool::Iterator t = pool.begin(); t != pool.end(); ++t) {

		// 初期検索に必要なパラメタ
		// - TSVはプールへの挿入時に1となっている
		// - 常に単独語として使用し paramProximity は設定しない
		t->setWeight(0);
			t->setParamWeight(paramWeight1);
			t->setParamScore(paramScore1);
			t->setParamLength(paramLength1);

		// 正規化済み語形
		if (useNormalizer1 == ModTrue)
		{
			ModUnicodeString normalized = getNormalizedString(
#ifdef V1_6
				t->getLangSpec(),
#endif
				t->getString());
				t->setString(normalized);
		}

		// 検索語タイプに応じたスケール設定
		ModTermType type = t->getType();
		switch(t->getType()) {
			case ModTermCategoryUndefined :
				t->setScale(t->getScale() * scaleUndefined);
				t->setType(ModTermCategoryHelpful);
				break;
			case ModTermCategoryEssential :
			case ModTermCategoryImportant :
				t->setScale(t->getScale() * scaleImportant1);
				break;
			case ModTermCategoryHelpful	 :
				// スケールはそのまま
				break;
			case ModTermCategoryEssentialRelated :
			case ModTermCategoryImportantRelated :
				t->setScale(t->getScale() * scaleImportant2);
				break;
			case ModTermCategoryHelpfulRelated	 :
				// スケールはそのまま
				t->setType(ModTermCategoryHelpful);
				break;
			default :
				break;
		}
	}
}

//
// FUNCTION public
// ModTerm::getNormalizedString -- 正規化文字列の取得
//
// NOTES
//	 - 入力文字列を正規化して得られた文字列を単語に分解しないで返す
//
// ARGUMENTS
//	const ModLanguageSet&	langSpec	言語指定
//	const ModUnicodeString& string	 入力文字列
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
ModUnicodeString
ModTerm::getNormalizedString(
#ifdef V1_6
	const ModLanguageSet&	langSpec,	// 言語指定
#endif
	const ModUnicodeString& string)	 // 入力文字列
{
	// 対象文字列の設定
#ifdef SYD_USE_UNA_V10
#if 0
	m_cUnaParam[_NormMode] = _ModNlpNormOnly;
#endif
	m_cUnaParam[_DoNorm] = _True;
	analyzer->prepare( m_cUnaParam);
	analyzer->set(string,langSpec);
#else
#ifdef V1_6
	analyzer->set(string, ModNlpNormOnly, langSpec);
#else
	analyzer->set(string, ModNlpNormOnly);
#endif
#endif

	// 形態素毎に
	ModUnicodeString result;
	ModUnicodeString normalized;
#ifdef SYD_USE_UNA_V10
#ifdef SYD_USE_UNA_V12
	while (analyzer->getWord(normalized) == ModTrue) {
#else
	ModUnicodeString dummy1;
	int dummy2;
	while (analyzer->getWord(normalized, dummy1, dummy2) == ModTrue) {
#endif
#else
	while (analyzer->getWord(normalized, ModTrue) == ModTrue) {
#endif
		 result += normalized;
	}
	return result;
}

//
// Copyright (c) 2000, 2004, 2005, 2006, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
