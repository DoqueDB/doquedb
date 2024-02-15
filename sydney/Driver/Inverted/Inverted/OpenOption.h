// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenOption.h --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_OPENOPTION_H
#define __SYDNEY_INVERTED_OPENOPTION_H

#include "Inverted/Module.h"
#include "Inverted/FileID.h"

#include "LogicalFile/OpenOption.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "Common/Object.h"
#include "Common/WordData.h"

#include "ModInvertedTypes.h"
#include "ModVector.h"
#include "ModLanguageSet.h"
#include "ModUnicodeOstrStream.h"

namespace UNA {
	class ModNlpAnalyzer;
}

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

//
//	CLASS
//	Inverted::OpenOption -- 転置ファイルドライバーのOpenOption
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
			ConditionString = LogicalFile::OpenOption::DriverNumber::Inverted,

			// 検索種別(Integer)
			SearchType,
			
			// RowID(Integer)
			RowID,
			// セクション数(Integer)
			SectionCount,
			// セクションの大きさbyte(Integer, Array)
			SectionSize,
			// セクションの言語(String, Array)
			SectionLang,

			// 検索語数
			TermCount,

			// 自然文言語情報(String)
			FreeTextLanguage,
			// スコア計算器(String)
			Calculator,
			// 平均文書長(Integer)
			AverageLength,
			// 文書頻度(Integer)
			DocumentFrequency,
			// Extractor(String)
			Extractor,
			// 拡張語の上限(Integer)
			ExpandLimit,

			// ワード数(Integer)
			WordCount,
			// ワードリストのパターン(String, Array)
			WordPattern,
			// ワードリストのカテゴリ(Integer, Array)
			WordCategory,
			// ワードリストのスケール(Double, Array)
			WordScale,
			// ワードリストの言語(String, Array)
			WordLanguage,

			// スコア調整方法(Integer)
			ScoreFunction,
			// クラスタリングのためのしきい値
			ClusteredLimit,
			// クラスタId
			ClusterId,
			// クラスタリングすることを示す(bool)
			Cluster,

			// ワードリストの一致条件(Integer, Array)
			WordMatchMode,
			
			// ユーザが指定したKWICの大きさ(Integer)
			KwicSize,

			// ワードリストで必須語が使われている(bool)
			EssentialWordList
		};
	};

	// 検索タイプ
	struct Type
	{
		enum Value
		{
			Normal,		// 通常の検索(Tea構文がある)
			Equal,		// 整合性検査のためのequal検索
			FreeText,	// 自然文検索
			WordList	// 単語リスト
		};
	};

	// スコア調整方法
	struct ScoreMethod
	{
		enum Value
		{
			Sum,		// 足す
			Multiply,	// 掛ける
			Replace,	// 置き換える

			Unknown		// 不明な調整方法
		};
	};

	// コンストラクタ
	OpenOption(const LogicalFile::OpenOption& cLogicalOpenOption_);
	// デストラクタ
	virtual ~OpenOption();

	// TreeNodeInterfaceをパースする
	bool parse(const LogicalFile::FileID& cFileID_,
			   bool isLanguage_,
			   bool isScoreField_,
			   const LogicalFile::TreeNodeInterface* pCondition_);

	// 検索タイプを得る
	Type::Value getSearchType() const;
	
	// 検索文を得る
	ModUnicodeString getConditionString() const;
	// 言語情報を得る
	ModLanguageSet getFreeTextLanguage() const;
	// スコア計算器を得る
	ModUnicodeString getCalculator() const;
	// 平均文書長を得る
	int getAverageLength() const;
	// 文書頻度を得る
	int getDocumentFrequency() const;
	// Extractorを得る
	ModUnicodeString getExtractor() const;
	// 拡張語の上限
	int getExpandLimit() const;
	
	// 検索語数を得る
	int getTermCount() const;

	// RowIDを得る
	ModUInt32 getRowID() const;
	// セクションの区切り位置を得る
	ModVector<ModSize> getSectionByteOffset() const;
	// 言語情報を得る
	ModVector<ModLanguageSet> getLanguageSet() const;

	// word数を得る
	int getWordCount() const;
	// wordのパターンを得る
	ModUnicodeString getWordPattern(int n) const;
	// wordのカテゴリを得る
	Common::WordData::Category::Value getWordCategory(int n) const;
	// wordのスケールを得る
	double getWordScale(int n) const;
	// wordの言語を得る
	ModUnicodeString getWordLanguage(int n) const;
	// wordの一致条件を得る
	int getWordMatchMode(int n) const;

	// スコア調整方法を得る
	ScoreMethod::Value getScoreMethod() const;

	// クラスタリング時の類似度のしきい値を得る
	float getClusteredLimit() const;

private:
	//
	//	STRUCT
	//	Inverted::OpenOption::Term
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
	//	Inverted::OpenOption::Entry
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
	//	Inverted::OpenOption::TermType
	//
	enum TermType
	{
		Undefined,
		Word,
		String,
		Error
	};

	// containsの検索条件を設定する
	bool setContains(const LogicalFile::TreeNodeInterface* pCondition_);
	// 普通の検索条件を設定する
	bool setNormal(const LogicalFile::TreeNodeInterface* pCondition_);
	// 整合性検査時のequal検索を設定する
	bool setEqual(const LogicalFile::TreeNodeInterface* pCondition_);

	// FreeTextデータを得る
	bool parseFreeText(const LogicalFile::TreeNodeInterface* pCondition_,
					   ModUnicodeString& cFreeText_,
					   ModUnicodeString& cLanguage_);

	// Expandデータを得る
	bool parseExpand(const LogicalFile::TreeNodeInterface* pCondition_,
					 int& iLimit_);

	// 1つの検索条件をtea構文に変換する
	bool convertNormal(const LogicalFile::TreeNodeInterface* pCondition_,
					   ModUnicodeString& cstrTea_,
					   int& iTermCount_);
	// likeを変換する
	bool convertLike(const LogicalFile::TreeNodeInterface* pCondition_,
					 ModUnicodeString& cstrTea_,
					 int& iTermCount_);
	// equalを変換する
	bool convertEqual(const LogicalFile::TreeNodeInterface* pCondition_,
					  ModUnicodeString& cstrTea_,
					  int& iTermCount_);
	// 検索語を変換する
	bool convertValue(const ModUnicodeString& cstrValue_,
					  const ModUnicodeString& cstrEscape_,
					  const ModUnicodeString& cstrLanguage_,
					  ModUnicodeString& cstrTea_,
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

	// contains operandを変換する
	bool convertContains(const LogicalFile::TreeNodeInterface* pCondition_,
						 ModUnicodeString& cstrTea_,
						 int& iTermCount_);

	// patternを変換する
	bool convertPattern(ModUnicodeChar match_,
						const LogicalFile::TreeNodeInterface* pCondition_,
						ModUnicodeString& cstrTea_);

	// wordを設定する
	bool setWord(const LogicalFile::TreeNodeInterface* pCondition_,
				 ModUnicodeString& pattern_,
				 int& category_,
				 double& scale_,
				 ModUnicodeString& language_,
				 int& matchMode_);

	// 単語単位検索可能か？
	bool isWord() { return m_eIndexType & ModInvertedFileWordIndexing; }
	// 文字列検索可能か？
	bool isString() { return m_eIndexType & ModInvertedFileNgramIndexing; }

	// tea構文ではエスケープしなければならない文字か？
	bool isEscapeChar(ModUnicodeChar c);

	// タプルIDデータを設定する
	bool setTupleID(const LogicalFile::TreeNodeInterface* pValue_,
					int& iRowID_);
	// 文字列データを設定する
	bool setStringData(const LogicalFile::TreeNodeInterface* pValue_,
					   ModUnicodeString& cValue_,
					   ModVector<ModSize>& vecSectionOffset_);
	// 言語データを設定する
	bool setLanguageData(const LogicalFile::TreeNodeInterface* pValue_,
						 ModVector<ModLanguageSet>& vecLanguageSet_);
	// 同義語を展開する
	bool getExpandSynonym(const LogicalFile::TreeNodeInterface* pNode_,
						  ModVector<ModUnicodeString>& vecExp_);


	// LogicalFile::OpenOption
	LogicalFile::OpenOption& m_cOpenOption;
	
	// 索引タイプ
	ModInvertedFileIndexingType m_eIndexType;
	// 位置情報リストを使えるか
	bool m_bNolocation;
	// 言語指定フィールド番号
	int m_iLanguageField;
	// ROWIDフィールド番号
	int m_iRowIDField;
	// デフォルト言語
	ModUnicodeString m_cDefaultLanguage;
	// ClusterLimit指定可能かどうか
	bool m_bClustering;
	// リソース番号
	ModUInt32 m_uiResourceID;

	// equal検索か
	bool m_bEqual;

	// 計算器
	ModUnicodeString m_cCalc;
	// 合成器
	ModUnicodeString m_cComb;

	// EXPAND_SYNONYMで展開された単語
	ModUnicodeString m_cExpandWord;
	// UNA
	UNA::ModNlpAnalyzer* m_pAnalyzer;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_OPENOPTION_H

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
