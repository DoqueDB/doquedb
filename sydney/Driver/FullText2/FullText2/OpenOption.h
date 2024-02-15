// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenOption.h --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_OPENOPTION_H
#define __SYDNEY_FULLTEXT2_OPENOPTION_H

#include "FullText2/Module.h"
#include "FullText2/FileID.h"

#include "LogicalFile/OpenOption.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "Common/BitSet.h"
#include "Common/WordData.h"

#include "ModVector.h"
#include "ModLanguageSet.h"
#include "ModUnicodeOstrStream.h"

namespace UNA {
	class ModNlpAnalyzer;
}

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class ScoreCombiner;
class ScoreCalculator;

//
//	CLASS
//	FullText2::OpenOption -- 転置ファイルドライバーのOpenOption
//
//	NOTES
//
class OpenOption
{
public:
	struct KeyID
	{
		enum Value
		{
			// 検索文(String)
			Condition = LogicalFile::OpenOption::DriverNumber::FullText,

			// 検索種別(Integer : Type::Value)
			SearchType,

			//
			// 整合性検査のための検索条件
			//
			// RowID(Integer)
			RowID,
			// セクション数(Integer)
			SectionCount,
			// セクションの文字列(String, Array)
			SectionValue,
			// セクションの言語(String, Array)
			SectionLang,

			// 検索語数
			TermCount,
			// 粗いKWICのサイズ
			KwicSize,

			// 結果取得の上限値(Integer)
			Limit,
			// 結果取得の取得開始位置(Integer)
			Offset,

			// ソート指定(Integer : SortParamter::Value)
			SortSpec,

			// 関数指定(Integer Function::Value, Array)
			Function,
			// 関数引数(String, Array)
			FunctionArgument,

			// 検索対象フィールド数(Integer)
			SearchFieldCount,
			// 検索対象フィールド(Integer, Array)
			SearchField
		};
	};

	// 検索タイプ
	struct Type
	{
		enum Value
		{
			None,
			
			Normal,		// 通常の検索
			FreeText,	// ModTermを利用する検索(Word含む)
			Equal		// 整合性検査のためのequal検索
		};
	};

	// ソートパラメータ
	struct SortParameter
	{
		enum Value
		{
			None,
			
			ScoreDesc,		// スコア降順
			ScoreAsc,		// スコア昇順
			RowIdDesc,		// ROWID降順
			RowIdAsc,		// ROWID昇順
			WordScaleDesc,	// スケール降順
			WordScaleAsc,	// スケール昇順
			WordDfDesc,		// DF降順
			WordDfAsc,		// DF昇順
			TermNoDesc,		// 単語番号順降順
			TermNoAsc		// 単語番号順昇順
		};
	};

	// 関数指定
	struct Function
	{
		enum Value
		{
			None = 0,			// 関数なし

			RowID,				// ROWID(関数ではないが)
			
			Score,				// スコア
			Section,			// セクション
			Word,				// 単語
			WordDf,				// 単語.文書頻度
			WordScale,			// 単語.重み
			AverageLength,		// 平均文書長
			AverageCharLength,	// 平均文書長(DUAL, N-GRAM)
			AverageWordCount,	// 平均単語数(WORD)
			Tf,					// 文書内頻度
			Existence,			// 文書内有無
			Count,				// 文書頻度
			ClusterID,			// クラスターID
			FeatureValue,		// 特徴語リスト
			RoughKwicPosition,	// 粗いKWICの開始位置

			ValueNum
		};
	};
			

	// コンストラクタ
	OpenOption(LogicalFile::OpenOption& cLogicalOpenOption_);
	// デストラクタ
	virtual ~OpenOption();

	// TreeNodeInterfaceをパースする
	bool parse(const FileID& cFileID_,
			   const LogicalFile::TreeNodeInterface* pCondition_);

	// 検索タイプを得る
	Type::Value getSearchType() const;
	
	// 検索文を得る
	ModUnicodeString getCondition() const;
	
	// 検索語数を得る
	int getTermCount() const;

	// RowIDを得る
	ModUInt32 getRowID() const;
	// セクションの文字列データを得る
	ModVector<ModUnicodeString> getSectionValue() const;
	// セクションの言語情報を得る
	ModVector<ModLanguageSet> getSectionLanguage() const;

	// 取得するフィールド数を得る
	int getProjectionFieldCount() const;
	// 取得するフィールド番号を得る
	int getProjectionFieldNumber(int n_) const;
	// 取得するフィールドに適用する関数を得る
	Function::Value getProjectionFunction(int n_) const;
	// 取得するフィールドに適用する関数の引数を得る
	ModUnicodeString getProjectionFunctionArgument(int n_) const;

	// 更新対象のフィールド数を得る
	int getUpdateFieldCount() const;
	// 更新対象のフィールド番号を得る
	int getUpdateFieldNumber(int n_) const;

	// 検索フィールド数を得る
	int getSearchFieldCount() const;
	// 検索対象フィールドを得る
	int getSearchFieldNumber(int n_) const;

	// ソート条件を得る
	SortParameter::Value getSortParameter() const;

	// LIMITを得る
	ModSize getLimit() const;
	// OFFSETを得る
	ModSize getOffset() const;

	// 見積もりモードかどうか
	bool isEstimate() const;
	// ビットセットによる取得かどうか
	bool isGetByBitSet() const;

	// 検索結果の絞り込み用のビットセットを取得する
	const Common::BitSet* getBitSetForNarrowing() const;
	// 絞り込んだ集合でランキング検索する用のビットセットを取得する
	const Common::BitSet* getBitSetForRanking() const;

	// スコア合成器を作成する
	static ScoreCombiner*
	createScoreCombiner(const ModUnicodeString& cCombiner_);
	// スコア計算器を作成する
	static ScoreCalculator*
	createScoreCalculator(const ModUnicodeString& cCalculator_);

	// スコア調整方法を得る
	static AdjustMethod::Value getAdjustMethod(const ModUnicodeString& method_);

	// 正しいスコア合成器か確認する
	static bool checkScoreCombiner(const ModUnicodeString& cCombiner_);

private:
	//
	//	STRUCT
	//	FullText2::OpenOption::Term
	//
	struct Term
	{
		Term() : m_bRegrex(false), m_bFront(false), m_bBack(false) {}
		Term(const ModUnicodeChar* p, ModSize l, bool b, bool front, bool back)
			: m_pszValue(p), m_uiLength(l), m_bRegrex(b),
			  m_bFront(front), m_bBack(back) {}
		const ModUnicodeChar*	m_pszValue;
		ModSize					m_uiLength;
		bool					m_bRegrex;
		bool					m_bFront;
		bool					m_bBack;
	};

	//
	//	STRUCT
	//	FullText2::OpenOption::Entry
	//
	struct Entry
	{
		Entry() : m_uiPosition(0), m_pNext(0) {}
		~Entry() { delete m_pNext; }
		ModUnicodeString	m_cstrValue;
		ModSize				m_uiPosition;
		Entry*				m_pNext;
	};

	//
	//	ENUM
	//	FullText2::OpenOption::TermType
	//
	enum TermType
	{
		Undefined,
		Word,
		String,
		Error
	};

	// contains演算子の検索条件を設定する
	bool setContains(const LogicalFile::TreeNodeInterface* pCondition_);
	// 普通の検索条件を設定する
	bool setNormal(const LogicalFile::TreeNodeInterface* pCondition_);
	// 整合性検査時のequal検索を設定する
	bool setEqual(const LogicalFile::TreeNodeInterface* pCondition_);

	// Expandデータを得る
	bool parseExpand(const LogicalFile::TreeNodeInterface* pCondition_,
					 int& iLimit_);

	// 1つの検索条件をtea構文に変換する
	bool convertNormal(ModUnicodeOstrStream& cStream_,
					   const LogicalFile::TreeNodeInterface* pCondition_,
					   int& iTermCount_);
	// likeを変換する
	bool convertLike(ModUnicodeOstrStream& cStream_,
					 const LogicalFile::TreeNodeInterface* pCondition_,
					 int& iTermCount_);
	// equalを変換する
	bool convertEqual(ModUnicodeOstrStream& cStream_,
					  const LogicalFile::TreeNodeInterface* pCondition_,
					  int& iTermCount_);
	// 検索語を変換する
	bool convertValue(ModUnicodeOstrStream& cStream_,
					  const ModUnicodeString& cstrValue_,
					  const ModUnicodeString& cstrEscape_,
					  const ModUnicodeString& cstrLanguage_,
					  int& iTermCount_);

	// '_'を#distanceに変換する
	bool convertDistance(ModUnicodeOstrStream& cStream_,
						 const ModUnicodeChar* pValue_,
						 ModSize uiLength_,
						 ModUnicodeChar escape_,
						 const ModUnicodeString& cLang_,
						 int& f_, int& e_);

	// '%'または'/'で文字を分割する
	TermType separate(const ModUnicodeChar* pszValue_,
					  ModUnicodeChar cEscape_, ModVector<Term>& vecTerm_);

	// contains を変換する
	bool convertContains(ModUnicodeOstrStream& cStream_,
						 const LogicalFile::TreeNodeInterface* pCondition_,
						 int& iTermCount_);

	// patternを変換する
	bool convertPattern(ModUnicodeOstrStream& cStream_,
						const LogicalFile::TreeNodeInterface* pCondition_,
						ModUnicodeChar match_);

	// freetextを変換する
	bool convertFreeText(ModUnicodeOstrStream& cStream_,
						 const LogicalFile::TreeNodeInterface* pCondition_,
						 int& iTermCount_);

	// wordlistを変換する
	bool convertWordList(ModUnicodeOstrStream& cStream_,
						 const LogicalFile::TreeNodeInterface* pCondition_,
						 int& iTermCount_);
	
	// wordを変換する
	bool convertWord(ModUnicodeOstrStream& cStream_,
					 const LogicalFile::TreeNodeInterface* pCondition_,
					 int& iTermCount_);

	// 単語単位検索可能か？
	bool isWord() { return m_eIndexType & IndexingType::Word; }
	// 文字列検索可能か？
	bool isString() { return m_eIndexType & IndexingType::Ngram; }

	// tea構文ではエスケープしなければならない文字か？
	bool isEscapeChar(ModUnicodeChar c);
	// tea構文でエスケープしなければならない文字をエスケープして文字を追加する
	void appendPattern(ModUnicodeOstrStream& cStream_,
					   const ModUnicodeString& cstrPattern_);

	// タプルIDデータを設定する
	bool setTupleID(const LogicalFile::TreeNodeInterface* pValue_,
					int& iRowID_);
	// 文字列データを設定する
	bool setStringData(const LogicalFile::TreeNodeInterface* pValue_,
					   ModVector<ModUnicodeString>& vecStrValue_);
	// 言語データを設定する
	bool setLanguageData(const LogicalFile::TreeNodeInterface* pValue_,
						 ModVector<ModLanguageSet>& vecLanguageSet_);
	// 同義語を展開する
	bool getExpandSynonym(const LogicalFile::TreeNodeInterface* pNode_,
						  ModVector<ModUnicodeString>& vecExp_);

	// 同じフィールドに対する条件だけか確認する
	int checkField(const LogicalFile::TreeNodeInterface* pNode_);

	// LogicalFile::OpenOption
	LogicalFile::OpenOption& m_cOpenOption;
	
	// 索引タイプ
	IndexingType::Value m_eIndexType;
	// 位置情報リストを使えるか
	bool m_bNolocation;
	// デフォルト言語
	ModUnicodeString m_cDefaultLanguage;
	// キーの数
	int m_iKeyCount;
	// 言語指定フィールド番号
	int m_iLanguageField;
	// ROWIDフィールド番号
	int m_iRowIDField;
	// ClusterLimit指定可能かどうか
	bool m_bClustering;
	// リソース番号
	ModUInt32 m_uiResourceID;

	// equal検索か
	bool m_bEqual;

	// EXPAND_SYNONYMで展開された単語
	ModUnicodeString m_cExpandWord;
	// UNA
	UNA::ModNlpAnalyzer* m_pAnalyzer;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_OPENOPTION_H

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
