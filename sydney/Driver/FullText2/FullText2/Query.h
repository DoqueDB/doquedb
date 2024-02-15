// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query.h -- 
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_QUERY_H
#define __SYDNEY_FULLTEXT2_QUERY_H

#include "FullText2/Module.h"
#include "FullText2/OperatorNode.h"

#include "FullText2/ScoreCalculator.h"
#include "FullText2/LeafNode.h"

#include "Common/LargeVector.h"
#include "Common/WordData.h"

#include "Utility/SearchTermData.h"
#include "Utility/ModTerm.h"
#include "Utility/ModTermElement.h"

#include "ModAutoPointer.h"
#include "ModHashMap.h"
#include "ModMap.h"
#include "ModPair.h"
#include "ModVector.h"
#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class ArrayLeafNode;
class LeafNode;
class ListManager;
class LogicalOperatorNode;
class OperatorTermNode;
class SearchInformation;
class ScoreCalculator;
class ScoreCombiner;

//
//	CLASS
//	FullText2::Query -- ノードを生成するクラス
//
//	NOTES
//
class Query
{
public:
	// 検索語情報の配列
	typedef ModVector<Utility::SearchTermData>	TermValue;
	// TermValueのLess
	struct TermValueLess
	{
		ModBoolean operator()(const TermValue& a, const TermValue& b) const
			{
				TermValue::ConstIterator ia = a.begin();
				TermValue::ConstIterator ib = b.begin();
				for (; ia != a.end() && ib != b.end(); ++ia, ++ib) {
					int c = compare(*ia, *ib);
					if (c == 0)
						continue;
					return (c < 0) ? ModTrue : ModFalse;
				}
				return (a.getSize() < b.getSize()) ? ModTrue : ModFalse;
			}

		int compare(const Utility::SearchTermData& a,
					const Utility::SearchTermData& b) const
			{
				int c = a.getTerm().compare(b.getTerm());
				if (c == 0) {
					if (a.getMatchMode() == b.getMatchMode())
					{
						return (a.getLanguage() == b.getLanguage()) ? 0
							: ((a.getLanguage() < b.getLanguage()) ? -1 : 1);
					}
					return (a.getMatchMode() < b.getMatchMode()) ? -1 : 1;
				}
				return c;
			}
	};
	
	// 検索語ノードのマップ
	typedef ModMap<TermValue, LeafNode*, TermValueLess>	TermNodeMap;

	//
	//	複合索引のスコア合成方法
	//
	struct CombineMethod
	{
		enum Value
		{
			None,		// 合成しない
			
			Tf,			// TF値の和を使ってTF項を計算する
			ScoreOr,	// スコアを合成する(結果集合は論理和)
			ScoreAnd	// スコアを合成する(結果集合は論理積)
		};
	};

	// フィールドごとのスケール値を表すクラス
	struct ScaleData
	{
		ScaleData()
			: m_iField(-1), m_dblScale(0), m_dblGeta(0) {}
		ScaleData(int f, double s, double g)
			: m_iField(f), m_dblScale(s), m_dblGeta(g) {}
		
		int m_iField;
		double m_dblScale;
		double m_dblGeta;
	};
	
	// コンストラクタ
	Query(bool bScore_, bool bTfList_, bool bTeaString_);
	// デストラクタ
	virtual ~Query();
	// コピーコンストラクタ
	Query(const Query& src_);

	//
	//	スコア計算のためのメソッド
	//

	// スコア計算器を得る
	ScoreCalculator* getScoreCalculator(const ModUnicodeString& cCalculator_);
	// スコア合成器を得る
	ScoreCombiner* getScoreCombiner(const ModUnicodeString& cCombiner_);
	
	// スコア計算器を表す文字列を設定する
	void setScoreCalculator(const ModUnicodeString& cCalculator_)
		{ m_cCalculator = cCalculator_; }
	// スコア合成器を表す文字列を設定する
	void setScoreCombiner(const ModUnicodeString& cCombiner_)
		{ m_cCombiner = cCombiner_; }

	// 質問処理器を設定する
	void setTerm(Utility::ModTerm* pTerm_)
		{ m_pTerm = pTerm_; }
	
	//
	//	概念検索のためのメソッド
	//

	// 拡張文書を登録する
	void setExpandDocument(const ModVector<ModUnicodeString>& vecDocument_,
						   const ModVector<ModLanguageSet>& vecLang_);

	// tea構文をパースする
	//
	// #contains[...](...) の (...) の部分をパースする。
	// 引数 pTea_ は ( を指していること。
	// パース後は、) の次の文字を指している。
	//
	void parse(SearchInformation& cSearchInfo_,
			   ModVector<ListManager*>& vecpManager_,
			   const ModUnicodeChar*& pTea_);

	// ヒット件数を見積もる
	ModSize getEstimateCount(SearchInformation& cSearchInfo_,
							 ModVector<ListManager*>& vecpManager_,
							 const ModUnicodeChar*& pTea_);

	//
	// 位置情報取得用のノードを得るためにtea構文をパースする
	// 得られたノードは速やかにコピーすること(マップのみのコピー)
	//
	TermNodeMap& parseForLocation(SearchInformation& cSearchInfo_,
								  ListManager* pManager_,
								  const ModUnicodeChar*& pTea_);

	// スコア合成時のパラメータを登録する
	void setCombineParameter(CombineMethod::Value eMethod_,
							 const ModUnicodeString& cCombiner_,
							 const ModVector<ScaleData>& vecScale_)
		{
			m_eMultiMethod = eMethod_;
			m_cMultiCombiner = cCombiner_;
			m_vecMultiScale = vecScale_;
		}

	// 検索ノードを取得する
	OperatorNode* getRootNode() { return m_pRoot; }
	
	// 関連語を取得する
	void getWord(Common::LargeVector<Common::WordData>& vecWord_);

	// 位置情報取得のためのtea構文を得る
	const ModUnicodeString& getConditionForLocation() const
		{ return m_cConditionForLocation; }

	// 検索語ノードを取得する
	ModVector<OperatorTermNode*>& getTermNodeForTfList()
		{ return m_vecTermNodeForTfList; }		// TF値リスト用

	// トークンを切り出す
	static void getToken(ModUnicodeOstrStream& s_, const ModUnicodeChar*& p_);
	static void getToken(ModUnicodeString& s_, const ModUnicodeChar*& p_);

	// 文字列からint型を得る
	static bool parseIntArray(ModVector<int>& v_,
							  const ModUnicodeChar*& p_);
	// 文字列からdouble型を得る
	static bool parseDoubleArray(ModVector<double>& v_,
								 const ModUnicodeChar*& p_);
	// 文字列からscaleとgetaを得る
	static bool parseScaleArray(ModVector<ScaleData>& v_,
								const ModUnicodeChar*& p_);

private:
	// tea構文をパースする
	OperatorNode* parseImpl(SearchInformation& cSearchInfo_,
							ModVector<ListManager*>& vecpManager_,
							const ModUnicodeChar*& pTea_);
	// tea構文をパースする
	OperatorTermNode* parseTermNode(SearchInformation& cSearchInfo_,
									ModVector<ListManager*>& vecpManager_,
									const ModUnicodeChar*& pTea_,
									bool bAddTermNode_,
									const ModUnicodeString& cCalculator_);
	// 論理演算子共通の処理
	void parseLogicalOperator(SearchInformation& cSearchInfo_,
							  ModVector<ListManager*>& vecpManager_,
							  const ModUnicodeChar*& pTea_,
							  LogicalOperatorNode* pNode_);
	
	// LeafNodeを作成する
	LeafNode* parseLeafNode(ListManager* pManager_,
							const ModUnicodeChar*& pTea_,
							TermValue* pTermValue_ = 0);
	// 検索キーワードノードを作成する
	LeafNode* parseTerm(ListManager* pManager_,
						const ModUnicodeChar*& pTea_,
						TermValue* pTermValue_ = 0);
	// 単一要素を子ノードに持つLeafNode共通の処理を行う
	LeafNode* parseUnaryLeafNode(ListManager* pManager_,
								 const ModUnicodeChar*& pTea_);
	// 配列を子ノードに持てるLeafNode共通の処理を行う
	void parseArrayLeafNode(ListManager* pManager_,
							const ModUnicodeChar*& pTea_,
							ArrayLeafNode* pNode_,
							TermValue* pTermValue_ = 0);

	// 自然文検索をパースする
	OperatorNode* parseFreeText(SearchInformation& cSearchInfo_,
								ModVector<ListManager*>& vecpManager_,
								const ModUnicodeChar*& pTea_);
	// 単語リストをパースする
	OperatorNode* parseWordList(SearchInformation& cSearchInfo_,
								ModVector<ListManager*>& vecpManager_,
								const ModUnicodeChar*& pTea_);
	// 単語をパースする
	void parseWord(const ModUnicodeChar*& pTea_,
				   Utility::ModTermElement& term);
	
	// 自然文から単語を抽出する
	void makePoolFreeText(const ModUnicodeChar*& pTea_);
	// 単語リストから単語を取得する
	void makePoolWordList(const ModUnicodeChar*& pTea_);
	
	// 
	// 検索ノードを作成する
	OperatorNode* makeOperatorNode(SearchInformation& cSearchInfo_,
								   ModVector<ListManager*>& vecpManager_,
								   Utility::ModTermPool::Iterator& ite_);

	// シード文書で検索語プールを拡張する
	void expandPool(SearchInformation& cSearchInfo_,
					ModVector<ListManager*>& vecpManager_,
					Utility::ModTermPool& pool,
					Utility::ModTermPool& pool2);

	// DFを取得する
	void getDocumentFrequency(SearchInformation& cSearchInfo_,
							  ModVector<ListManager*>& vecpManager_,
							  Utility::ModTermPool& cPool_);
	// DFを取得する
	void getDocumentFrequency(SearchInformation& cSearchInfo_);

	// DFをTermPoolに設定する
	void setDocumentFrequencyForTermPool(SearchInformation& cSearchInfo_,
										 Utility::ModTermPool::Iterator s_,
										 Utility::ModTermPool::Iterator e_);

	// 一致条件を得る
	static MatchMode::Value getMatchMode(const ModUnicodeString& mode_);
	static int getMatchModeForTerm(const ModUnicodeString& mode_);
	
	// 検索結果件数を見積もる
	ModSize estimateCountFromPool(SearchInformation& cSearchInfo_,
								  Utility::ModTermPool& cPool_);
	
	// ルートノード
	OperatorNode* m_pRoot;

	// デフォルトのスコア計算器
	ModUnicodeString m_cCalculator;
	// デフォルトのスコア合成器
	ModUnicodeString m_cCombiner;

	// 複合索引のスコア合成方法
	CombineMethod::Value m_eMultiMethod;
	// 複合索引のスコア合成器
	ModUnicodeString m_cMultiCombiner;
	// 複合索引のスケール
	ModVector<ScaleData> m_vecMultiScale;

	// スコア計算がいるか
	bool m_bScoreCalculator;
	// 検索文内頻度がいるか
	bool m_bQueryTermFrequency;
	// 文書頻度がいるか
	bool m_bDocumentFrequency;
	// 総文書内頻度がいるか
	bool m_bTotalTermFrequency;

	// 拡張文書
	ModVector<ModUnicodeString> m_vecDocument;
	ModVector<ModLanguageSet> m_vecLanguage;

	// 質問処理器
	Utility::ModTerm* m_pTerm;
	// 検索語プール
	Utility::ModTermPool* m_pPool1;
	Utility::ModTermPool* m_pPool2;

	// 検索語プール用の終端ノード
	typedef ModMap<ModUnicodeString, OperatorTermNode*,
				   ModLess<ModUnicodeString> >	PoolTermMap;
	PoolTermMap m_mapPoolTermNode;

	// 自然文検索時の一致条件(ModTermElementのenum値)
	int m_iMatchMode;

	// TFリスト用の検索語ノード
	ModVector<OperatorTermNode*> m_vecTermNodeForTfList;
	// 位置情報取得用の検索語ノード
	TermNodeMap m_mapTermLeafNode;

	// 位置情報のためのtea構文
	ModUnicodeString m_cConditionForLocation;
	
	// 検索語ノードが必要かどうか
	bool m_bTfList;
	bool m_bLocation;
	bool m_bTeaString;

	// 検索結果件数見積もりかどうか
	bool m_bEstimate;
	// 見積もった件数
	ModSize m_uiEstimateCount;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_QUERY_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
