// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query.cpp --
// 
// Copyright (c) 2010, 2011, 2013, 2016, 2017, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/Query.h"

#include "FullText2/ListManager.h"
#include "FullText2/OpenOption.h"
#include "FullText2/SearchInformation.h"
#include "FullText2/Tokenizer.h"

#include "FullText2/AndLeafNode.h"
#include "FullText2/AtomicOrLeafNode.h"
#include "FullText2/HeadLeafNode.h"
#include "FullText2/LeafNode.h"
#include "FullText2/TailLeafNode.h"
#include "FullText2/WithinOrderedLeafNode.h"
#include "FullText2/WithinUnorderedLeafNode.h"
#include "FullText2/Parameter.h"

#include "FullText2/OperatorAddNode.h"
#include "FullText2/OperatorAndNode.h"
#include "FullText2/OperatorAndNotNode.h"
#include "FullText2/OperatorOrNode.h"
#include "FullText2/OperatorTermNodeAnd.h"
#include "FullText2/OperatorTermNodeOr.h"
#include "FullText2/OperatorTermNodeTf.h"
#include "FullText2/OperatorTermNodeSingle.h"
#include "FullText2/OperatorWeightNode.h"

#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Exception/WrongParameter.h"

#include "ModAutoPointer.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace {

	// tea構文のコマンド
		
	ModUnicodeString _cAndNot("#and-not");
	ModUnicodeString _cAnd("#and");
	ModUnicodeString _cOr("#or");

	ModUnicodeString _cScale("#scale");
	
	ModUnicodeString _cLocation("#location");
	ModUnicodeString _cEnd("#end");
	ModUnicodeString _cSyn("#syn");
	ModUnicodeString _cTerm("#term");
	ModUnicodeString _cWindow("#window");
	
	ModUnicodeString _cFreeText("#freetext");
	ModUnicodeString _cWordList("#wordlist");
	
	ModUnicodeString _cWord("#word");

	ModUnicodeString _cOrdered("o");
	ModUnicodeString _cUnordered("u");

	ModUnicodeString _cMulti("m");
	
	ModUnicodeString _cExactWord("e");
	ModUnicodeString _cSimpleWord("s");
	ModUnicodeString _cString("n");
	ModUnicodeString _cWordHead("h");
	ModUnicodeString _cWordTail("t");

	// マルチ言語モードを利用するかどうか
	FullText2::ParameterBoolean _cIsMulti("Inverted_MultiMatchMode", true);

	// 一度にDF値を求める単語の数の最大値
	// この上限数分 OpenMP で一気にDF値を求める
	//
	// この数値を大きくすると高速にはなるが、バッファメモリを消費してしまう。
	// 最悪値で、1単語のバッファサイズは 64KB x 分散数(disribute=10なら10倍)
	// distribute=10で上限が100の場合、64KB x 10 x 100 = 60MB 必要
	
	FullText2::ParameterInteger
	_cDFCalculatingLimit("FullText2_DocumentFrequencyCalculatingLimit", 100,
						 false);
	
	//
	//	文字列を Query::ScaleData に変換する
	//
	//	2.0#1.0 -> scale = 2.0, geta = 1.0 
	//
	Query::ScaleData _toScaleData(const ModUnicodeString& s_)
	{
		Query::ScaleData r;
		const ModUnicodeChar* p = s_.search('#');
		if (p)
		{
			const ModUnicodeChar* b = s_;
			ModUnicodeString scale(b, static_cast<ModSize>(p - b));
			r.m_dblScale = ModUnicodeCharTrait::toDouble(scale);

			++p;
			ModUnicodeString geta(p);
			r.m_dblGeta = ModUnicodeCharTrait::toDouble(geta);
		}
		else
		{
			r.m_dblScale = ModUnicodeCharTrait::toDouble(s_);
		}
		return r;
	}
}

//
//	FUNCTION public
//	FullText2::Query::Query -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	bool bScore_
//		スコア計算が必要かどうか
//	bool bTTfList_
//		TF値リストが必要かどうか
//	bool bTeaString_
//		位置情報のためのtea構文文字列が必要かどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Query::Query(bool bScore_, bool bTfList_, bool bTeaString_)
	: m_pRoot(0),
	  m_eMultiMethod(CombineMethod::None),
	  m_bScoreCalculator(bScore_), m_bQueryTermFrequency(false),
	  m_bDocumentFrequency(false), m_bTotalTermFrequency(false),
	  m_pTerm(0), m_pPool1(0), m_pPool2(0),
	  m_iMatchMode(Utility::ModTermElement::voidMatch),
	  m_bTfList(bTfList_), m_bLocation(false), m_bTeaString(bTeaString_),
	  m_bEstimate(false), m_uiEstimateCount(0)
{
}

//
//	FUNCTION public
//	FullText2::Query::~Query -- デストラクタ
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
Query::~Query()
{
	delete m_pRoot;
	delete m_pTerm;
	delete m_pPool1;
	delete m_pPool2;
}

//
//	FUNCTION public
//	FullText2::Query::getScoreCalculator -- スコア計算器を得る
//
//	NOTES
//	得られたスコア計算器のインスタンスは呼び出し側で解放すること
//
//	ARUGMENTS
//	const ModUnicodeString& cCalculator_
//		スコア計算器を表す文字列。ただし、SQL文でスコア計算器が
//		指定されている場合には、引数の指定は無視される。
//
//	RETURN
//	FullText2::ScoreCalculator*
//		スコア計算器
//
//	EXCEPTIONS
//
ScoreCalculator*
Query::getScoreCalculator(const ModUnicodeString& cCalculator_)
{
	// SQL文でスコア計算器が指定されている場合はそれを優先する
	// 関数的には、getScoreCombinerと逆の実装になっている
	
	if (m_cCalculator.getLength() == 0)
		return OpenOption::createScoreCalculator(cCalculator_);
	return OpenOption::createScoreCalculator(m_cCalculator);
}

//
//	FUNCTION public
//	FullText2::Query::getScoreCombiner -- スコア合成器を得る
//
//	NOTES
//	得られたスコア合成器のインスタンスは呼び出し側で解放すること
//
//	ARUGMENTS
//	const ModUnicodeString& cCombiner_
//		スコア合成器を表す文字列。空文字列の場合はデフォルトのものを返す
//
//	RETURN
//	FullText2::ScoreCombiner*
//		スコア合成器
//
//	EXCEPTIONS
//
ScoreCombiner*
Query::getScoreCombiner(const ModUnicodeString& cCombiner_)
{
	if (cCombiner_.getLength())
		return OpenOption::createScoreCombiner(cCombiner_);
	return OpenOption::createScoreCombiner(m_cCombiner);
}

//
//	FUNCTION public
//	FullText2::Query::setExpandDocument
//		-- 拡張文書を設定する
//
//	NOTES
//
//	ARGUMETNS
//	const ModVector<ModUnicodeString>& vecDocument_
//		拡張文書
//	const ModVector<ModLanguageSet>& vecLang_
//		拡張文書の言語情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Query::setExpandDocument(const ModVector<ModUnicodeString>& vecDocument_,
						 const ModVector<ModLanguageSet>& vecLang_)
{
	m_vecDocument = vecDocument_;
	m_vecLanguage = vecLang_;
}

//
//	FUNCTION public
//	FullText2::Query::parse -- tea構文をパースし、検索ノードを作成する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	ModVector<FullText2::ListManager*>& vecpManager_
//		リスト管理クラス
//	const ModUnicodeChar*& pTea_
//		tea構文
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Query::parse(SearchInformation& cSearchInfo_,
			 ModVector<ListManager*>& vecpManager_,
			 const ModUnicodeChar*& pTea_)
{
	if (m_bScoreCalculator)
	{
		// スコア計算器が必要とするデータ
		ModVector<ScoreCalculator::Argument> vecScoreArg;
		
		// スコア計算器が必要とする引数を確認する
		ModAutoPointer<ScoreCalculator> pCalc
			= getScoreCalculator(ModUnicodeString());
		pCalc->initialize(vecScoreArg);

		ModVector<ScoreCalculator::Argument>::Iterator i
			= vecScoreArg.begin();
		for (; i != vecScoreArg.end(); ++i)
		{
			switch ((*i).m_eType)
			{
			case ScoreCalculator::Argument::QueryTermFrequency:
				m_bQueryTermFrequency = true;
				break;
			case ScoreCalculator::Argument::DocumentFrequency:
				m_bDocumentFrequency = true;
				break;
			case ScoreCalculator::Argument::TotalTermFrequency:
				m_bTotalTermFrequency = true;
				break;
			}
		}
	}

	// デフォルトの一致条件を設定する
	switch (cSearchInfo_.getIndexingType())
	{
	case IndexingType::Dual:
		m_iMatchMode = Utility::ModTermElement::multiMatch;
		break;
	case IndexingType::Word:
		m_iMatchMode = Utility::ModTermElement::exactMatch;
		break;
	case IndexingType::Ngram:
		m_iMatchMode = Utility::ModTermElement::stringMatch;
		break;
	}

	// パースする
	m_pRoot = parseImpl(cSearchInfo_, vecpManager_, pTea_);

	if (m_pTerm == 0)
	{
		// 自然文検索ではない場合、ここで文書頻度を求める必要あり
		// 自然文検索の場合は、検索語選択時にすでに求めている

		if (m_eMultiMethod == CombineMethod::None ||
			m_eMultiMethod == CombineMethod::Tf)
		{
			getDocumentFrequency(cSearchInfo_);
		}
		else
		{
			// 子のSearchInfoを１つ１つ実行する
			ModVector<ScaleData>::Iterator i = m_vecMultiScale.begin();
			for (; i != m_vecMultiScale.end(); ++i)
			{
				SearchInformation& cSearchInfo
					= cSearchInfo_.getElement((*i).m_iField);
				getDocumentFrequency(cSearchInfo);
			}
		}
	}
	else if (m_bTfList)
	{
		// null のエントリがあったら、例外とする

		ModVector<OperatorTermNode*>::Iterator i
			= m_vecTermNodeForTfList.begin();
		for (; i != m_vecTermNodeForTfList.end(); ++i)
		{
			if ((*i) == 0)
				// サポート外
				_TRMEISTER_THROW0(Exception::NotSupported);
		}
	}
}

//
//	FUNCTION public
//	FullText2::Query::getEstimateCount
//		-- 検索結果件数を見積もる
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	ModVector<FullText2::ListManager*>& vecpManager_
//		リスト管理クラス
//	const ModUnicodeChar*& pTea_
//		tea構文
//
//	RETURN
//	ModSize
//		見積もった検索結果件数
//
//	EXCEPTIONS
//
ModSize
Query::getEstimateCount(SearchInformation& cSearchInfo_,
						ModVector<ListManager*>& vecpManager_,
						const ModUnicodeChar*& pTea_)
{
	// デフォルトの一致条件を設定する
	switch (cSearchInfo_.getIndexingType())
	{
	case IndexingType::Dual:
		m_iMatchMode = Utility::ModTermElement::multiMatch;
		break;
	case IndexingType::Word:
		m_iMatchMode = Utility::ModTermElement::exactMatch;
		break;
	case IndexingType::Ngram:
		m_iMatchMode = Utility::ModTermElement::stringMatch;
		break;
	}

	// 検索結果件数見積もりのためのパース
	m_bEstimate = true;
	m_uiEstimateCount = 0;

	// パースする
	OperatorNode* node = parseImpl(cSearchInfo_, vecpManager_, pTea_);

	if (node)
	{
		// freetext や wordlist の場合は 0 が返る
		
		try
		{
			// 見積もり件数を得る
			m_uiEstimateCount = node->getEstimateCount(cSearchInfo_);
		}
		catch (...)
		{
			delete node;
			_SYDNEY_RETHROW;
		}
		delete node;
	}

	return m_uiEstimateCount;
}

//
//	FUNCTION public
//	FullText2::Query::parseForLocation
//		-- 位置情報取得のために、tea構文をパースする
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	ListManager* pManager_
//		リスト管理クラス
//	const ModUnicodeChar*& pTea_
//		tea構文
//
//	RETURN
//	FullText2::Query::TermNodeMap&
//		位置情報取得のためのノードが格納されているマップ
//
//	EXCEPTIONS
//
Query::TermNodeMap&
Query::parseForLocation(SearchInformation& cSearchInfo_,
						ListManager* pManager_,
						const ModUnicodeChar*& pTea_)
{
	// 引数 pTea_ は ( を指していること。
	if (*pTea_ != '(')
		_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
	++pTea_;

	// 位置情報取得
	m_bLocation = true;

	// 現在のマップをクリアする
	m_mapTermLeafNode.erase(m_mapTermLeafNode.begin(), m_mapTermLeafNode.end());

	// パースする
	ModVector<ListManager*> v;
	v.pushBack(pManager_);
	parseImpl(cSearchInfo_, v, pTea_);

	m_bLocation = false;

	return m_mapTermLeafNode;
}

//
//	FUNCTION public
//	FullText2::Query::getWord -- 関連語を取得する
//
//	NOTES
//
//	ARGUMENTS
//	Common::LargeVector<Common::WordData>& vecWord_
//		関連語の配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Query::getWord(Common::LargeVector<Common::WordData>& vecWord_)
{
	vecWord_.clear();

	Utility::ModTermPool::Iterator i;

	// 問い合わせ文
	for (i = m_pPool1->begin(); i != m_pPool1->end(); ++i)
	{
		// 選択値が 0 の場合は、条件に含めない
		if ((*i).getTsv() == 0) continue;

		Common::WordData c((*i).getOriginalString());
		c.setLanguage((*i).getLangSpec());
		c.setCategory(
			static_cast<Common::WordData::Category::Value>((*i).getType()));
		c.setScale(((*i).getScale() == 0) ? 1.0 : (*i).getScale());
		c.setDocumentFrequency(static_cast<int>((*i).getDf()));

		vecWord_.pushBack(c);
	}

	if (vecWord_.getSize() == 0)
	{
		// 1つも条件に加われなかったので、すべてを加える
		for (i = m_pPool1->begin(); i != m_pPool1->end(); ++i)
		{
			Common::WordData c((*i).getOriginalString());
			c.setLanguage((*i).getLangSpec());
			c.setCategory(
				static_cast<Common::WordData::Category::Value>((*i).getType()));
			c.setScale(((*i).getScale() == 0) ? 1.0 : (*i).getScale());
			c.setDocumentFrequency(static_cast<int>((*i).getDf()));

			vecWord_.pushBack(c);
		}
	}

	// シード文書
	for (i = m_pPool2->begin(); i != m_pPool2->end(); ++i)
	{
		// 選択値が 0 の場合は、条件に含めない
		if ((*i).getTsv() == 0) continue;

		Common::WordData c((*i).getOriginalString());
		c.setLanguage((*i).getLangSpec());
		c.setCategory(
			static_cast<Common::WordData::Category::Value>((*i).getType()));
		c.setScale(((*i).getScale() == 0) ? 1.0 : (*i).getScale());
		c.setDocumentFrequency(static_cast<int>((*i).getDf()));

		vecWord_.pushBack(c);
	}
}

//
//	FUNCTION public static
//	FullText2::Query::getToken -- トークンを切り出す
//
//	NOTES
//
//	ARGUMETNS
//	ModUnicodeOstrStream& s_
//		切り出されたトークン
//	const ModUnicodeChar*& p_
//		パースするtea構文
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Query::getToken(ModUnicodeOstrStream& s_, const ModUnicodeChar*& p_)
{
	s_.clear();
	
	while (*p_ != 0)
	{
		if (*p_ == '\\')
		{
			++p_;
			if (*p_ != 0)
			{
				s_ << *p_;
				++p_;
			}
			continue;
		}
		
		if (*p_ == '[' || *p_ == ']' || *p_ == '(' || *p_ == ')' || *p_ == ',')
		{
			++p_;
			break;
		}

		s_ << *p_;
		++p_;
	}
}

//
//	FUNCTION public static
//	FullText2::Query::getToken -- トークンを切り出す
//
//	NOTES
//
//	ARGUMETNS
//	ModUnicodeString& s_
//		切り出されたトークン
//	const ModUnicodeChar*& p_
//		パースするtea構文
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Query::getToken(ModUnicodeString& s_, const ModUnicodeChar*& p_)
{
	s_ = "";
	
	while (*p_ != 0)
	{
		if (*p_ == '\\')
		{
			++p_;
			if (*p_ != 0)
			{
				s_.append(*p_);
				++p_;
			}
			continue;
		}
		
		if (*p_ == '[' || *p_ == ']' || *p_ == '(' || *p_ == ')' || *p_ == ',')
		{
			++p_;
			break;
		}

		s_.append(*p_);
		++p_;
	}
}

//
//	FUNCTION public
//	FullText2::Query::parseIntArray
//		-- 文字列からint型の配列を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<int>& v_
//		パース結果を格納する配列
//	const ModUnicodeChar*& p_
//		パースする文字列へのポインタ
//
//	RETURN
//	bool
//		パースできた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Query::parseIntArray(ModVector<int>& v_,
					 const ModUnicodeChar*& p_)
{
	if (*p_ != 0)
	{
		if (*p_ == '[')
		{
			++p_;
			while (*p_ != 0)
			{
				if (*p_ == ',')
				{
					++p_;
					break;
				}

				ModUnicodeString s;
				getToken(s, p_);
				v_.pushBack(ModUnicodeCharTrait::toInt(s));
			}
		}
		else if (*p_ == ',')
		{
			++p_;
		}
		else
		{
			ModUnicodeString s;
			getToken(s, p_);
			v_.pushBack(ModUnicodeCharTrait::toInt(s));
		}
	}

	return true;
}

//
//	FUNCTION public
//	FullText2::Query::parseDoubleArray
//		-- 文字列からdouble型の配列を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<double>& v_
//		パース結果を格納する配列
//	const ModUnicodeChar*& p_
//		パースする文字列へのポインタ
//
//	RETURN
//	bool
//		パースできた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Query::parseDoubleArray(ModVector<double>& v_,
						const ModUnicodeChar*& p_)
{
	if (*p_ != 0)
	{
		if (*p_ == '[')
		{
			++p_;
			while (*p_ != 0)
			{
				if (*p_ == ',')
				{
					++p_;
					break;
				}

				ModUnicodeString s;
				getToken(s, p_);
				v_.pushBack(ModUnicodeCharTrait::toDouble(s));
			}
		}
		else if (*p_ == ',')
		{
			++p_;
		}
		else
		{
			ModUnicodeString s;
			getToken(s, p_);
			v_.pushBack(ModUnicodeCharTrait::toDouble(s));
		}
	}

	return true;
}

//
//	FUNCTION public
//	FullText2::Query::parseScaleArray
//		-- 文字列からscaleとgetaの配列を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<ScaleData>& v_
//		パース結果を格納する配列
//	const ModUnicodeChar*& p_
//		パースする文字列へのポインタ
//
//	RETURN
//	bool
//		パースできた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Query::parseScaleArray(ModVector<ScaleData>& v_,
					   const ModUnicodeChar*& p_)
{
	//	ScaleData の m_iField は -1 となる

	if (*p_ != 0)
	{
		if (*p_ == '[')
		{
			++p_;
			while (*p_ != 0)
			{
				if (*p_ == ',')
				{
					++p_;
					break;
				}

				ModUnicodeString s;
				getToken(s, p_);
				v_.pushBack(_toScaleData(s));
			}
		}
		else if (*p_ == ',')
		{
			++p_;
		}
		else
		{
			ModUnicodeString s;
			getToken(s, p_);
			v_.pushBack(_toScaleData(s));
		}
	}

	return true;
}

//
//	FUNCTION private
//	FullText2::Query::parseImpl -- tea構文をパースする
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	ModVector<FullText2::ListManager*>& vecpManager_
//		リスト管理クラス
//	const ModUnicodeChar*& pTea_
//		tea構文
//
//	RETURN
// 	FullText2::OperatorNode*
//		パース結果
//
//	EXCEPTIONS
//
OperatorNode*
Query::parseImpl(SearchInformation& cSearchInfo_,
				 ModVector<ListManager*>& vecpManager_,
				 const ModUnicodeChar*& pTea_)
{
	ModAutoPointer<OperatorNode> ret;
	
	// 例えば、#term[...](...) の ...) の部分をパースする。
	// パース後は、) の次の文字を指している。

	int c = 0;
	while (*pTea_ == '(')
	{
		// 括弧は読み飛ばす
		++c;
		++pTea_;
	}

	if (*pTea_ != '#')
	{
		// 先頭が '#' ではないので、エラー
		_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
	}

	if (_cAndNot.compare(pTea_, _cAndNot.getLength()) == 0)
	{
		// #and-not(A,B)
		
		pTea_ += _cAndNot.getLength();
		if (*pTea_ == '(')
			++pTea_;	// '(' の分
		else
			_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);

		// 第一要素
		ModAutoPointer<OperatorNode> node1 = parseImpl(cSearchInfo_,
													   vecpManager_,
													   pTea_);

		if (*pTea_ == ',')
			++pTea_;	// ',' の分
		else
			_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);

		bool loc = m_bLocation;
		m_bLocation = false;
		
		// 第二要素
		ModAutoPointer<OperatorNode> node2 = parseImpl(cSearchInfo_,
													   vecpManager_,
													   pTea_);

		m_bLocation = loc;

		if (*pTea_ == ')')
			++pTea_;	// ')' の分
		else
			_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);

		if (!m_bLocation)
			ret = new OperatorAndNotNode(node1.release(), node2.release());
	}
	else if (_cAnd.compare(pTea_, _cAnd.getLength()) == 0)
	{
		// #and[Max](....)
		
		pTea_ += _cAnd.getLength();
		ModAutoPointer<LogicalOperatorNode> node
			= (m_bLocation == false) ? new OperatorAndNode() : 0;
		parseLogicalOperator(cSearchInfo_, vecpManager_, pTea_, node.get());
		ret = node.release();	// テンプレート引数が異なるので代入できない
	}
	else if (_cOr.compare(pTea_, _cOr.getLength()) == 0)
	{
		// #or[Max](....)
		
		pTea_ += _cOr.getLength();
		ModAutoPointer<LogicalOperatorNode> node
			= (m_bLocation == false) ? new OperatorOrNode() : 0;
		parseLogicalOperator(cSearchInfo_, vecpManager_, pTea_, node.get());
		ret = node.release();	// テンプレート引数が異なるので代入できない
	}
	else if (_cScale.compare(pTea_, _cScale.getLength()) == 0)
	{
		// #scale[1.5](....)
			
		pTea_ += _cScale.getLength();
			
		if (*pTea_ == '[')
			++pTea_;	// '[' の分
		else
			_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);

		ModUnicodeString s;
		getToken(s, pTea_);

		// doubleに変換する
		DocumentScore scale = ModUnicodeCharTrait::toDouble(s);

		// (....) を変換する
		ModAutoPointer<OperatorNode> node
			= parseImpl(cSearchInfo_, vecpManager_, pTea_);

		// 重みを処理するノードを作成する
		if (!m_bLocation)
			ret = new OperatorWeightNode(scale, node.release());
	}
	else if (_cFreeText.compare(pTea_, _cFreeText.getLength()) == 0)
	{
		// #freetext[m,ja,0.3,30](....)

		pTea_ += _cFreeText.getLength();
		ret = parseFreeText(cSearchInfo_, vecpManager_, pTea_);

		if (m_cConditionForLocation.getLength())
			m_cConditionForLocation.append(")");
	}
	else if (_cWordList.compare(pTea_, _cWordList.getLength()) == 0)
	{
		// #wordlist[10](#word[m,ja,Helpful,1.0,1](...),#word(...),...)

		pTea_ += _cWordList.getLength();
		ret = parseWordList(cSearchInfo_, vecpManager_, pTea_);
		
		if (m_cConditionForLocation.getLength())
			m_cConditionForLocation.append(")");
	}
	else
	{
		// これ以外は LeafNode を作成するもの

		ret = parseTermNode(cSearchInfo_, vecpManager_, pTea_, true,
							ModUnicodeString());
	}

	while (c)
	{
		if (*pTea_ != ')')
			_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
		++pTea_;
		--c;
	}

	return ret.release();
}

//
//	FUNCTION private
//	FullText2::Query::parseTermNode
//		-- 検索語ノード部分をパースする
//
//	NOTES
//
//	ARGUMETNS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	ModVector<FullText2::ListManager*>& vecpManager_
//		リスト管理クラス
//	const ModUnicodeChar*& pTea_
//		tea構文
//	bool bAddTermNode_
//		SearchInformationに作成したTremNodeを追加するかどうか
//	const ModUnicodeString& cCalculator_
//		スコア計算器指定
//
//	RETURN
//	FullText2::OperatorTermNode*
//		検索語ノード
//
//	EXCEPTIONS
//
OperatorTermNode*
Query::parseTermNode(SearchInformation& cSearchInfo_,
					 ModVector<ListManager*>& vecpManager_,
					 const ModUnicodeChar*& pTea_,
					 bool bAddTermNode_,
					 const ModUnicodeString& cCalculator_)
{
	//【注意】
	// bAddTermNode_ が true でも
	// m_bDocumentFrequency または m_bTotalTermFrequency のどちらかが true
	// でない場合には登録しない
	
	const ModUnicodeChar* p = pTea_;
	ModAutoPointer<OperatorTermNode> node;

	if (m_bLocation)
	{
		// 位置情報取得のためのパース

		parseLeafNode(vecpManager_[0], pTea_);	// 戻り値を解放する必要はない
		return 0;
	}
	else if (m_vecMultiScale.getSize() == 1)
	{
		// 単独カラムの場合

		ListManager* pManager = vecpManager_[m_vecMultiScale[0].m_iField];
		ModAutoPointer<LeafNode> leaf = parseLeafNode(pManager, pTea_);

		ModUnicodeString tea(p, static_cast<ModSize>(pTea_ - p));
		
		node = new OperatorTermNodeSingle(tea, leaf.release());
		node->setScoreCalculator(getScoreCalculator(cCalculator_));
		
		// スコア計算に必要な引数を登録する
	
		if (bAddTermNode_ && (m_bDocumentFrequency || m_bTotalTermFrequency))
			cSearchInfo_.addTermNode(tea, node);

	}
	else
	{
		// 複数カラムに跨った検索の場合
		
		OperatorTermNodeArray* array = 0;

		ModVector<ScaleData>::Iterator i = m_vecMultiScale.begin();
		for (; i != m_vecMultiScale.end(); ++i)
		{
			pTea_ = p;

			ListManager* pManager = vecpManager_[(*i).m_iField];
			if (pManager == 0)
				continue;
			
			ModAutoPointer<LeafNode> leaf = parseLeafNode(pManager, pTea_);

			ModUnicodeString tea(p, static_cast<ModSize>(pTea_ - p));
		
			if (array == 0)
			{
				// 必要なノードのインスタンスを確保する
				
				switch (m_eMultiMethod)
				{
				case CombineMethod::Tf:
					{
						ModAutoPointer<OperatorTermNodeTf> tmp
							= new OperatorTermNodeTf(tea);
						tmp->setScoreCalculator(
							getScoreCalculator(cCalculator_));

						array = tmp.get();
						node = tmp.release();
					}
					break;
				case CombineMethod::ScoreOr:
					{
						ModAutoPointer<OperatorTermNodeOr> tmp
							= new OperatorTermNodeOr(
								tea,
								getScoreCombiner(
									(m_cMultiCombiner.getLength() != 0) ?
									m_cMultiCombiner : m_cCombiner));

						array = tmp.get();
						node = tmp.release();
					}
					break;
				case CombineMethod::ScoreAnd:
					{
						ModAutoPointer<OperatorTermNodeAnd> tmp
							= new OperatorTermNodeAnd(
								tea,
								getScoreCombiner(
									(m_cMultiCombiner.getLength() != 0) ?
									m_cMultiCombiner : m_cCombiner));

						array = tmp.get();
						node = tmp.release();
					}
					break;
				}
			}

			// OperatorTermにする
			ModAutoPointer<OperatorTermNodeSingle> term
				= new OperatorTermNodeSingle(tea, leaf.release());

			// TermNodeを設定する
			array->pushBack((*i).m_iField, term.release(),
							(*i).m_dblScale, (*i).m_dblGeta);

			if (m_eMultiMethod != CombineMethod::Tf)
			{
				// スコア計算器を設定する
				term->setScoreCalculator(getScoreCalculator(cCalculator_));

				if (bAddTermNode_ &&
					(m_bDocumentFrequency || m_bTotalTermFrequency))
				{
					// スコア計算に必要な引数を登録する
				
					SearchInformation& cSearchInfo
						= cSearchInfo_.getElement((*i).m_iField);
					cSearchInfo.addTermNode(tea, term);
				}
			}
		}

		// スコア計算に必要な引数を登録する
		
		if (m_eMultiMethod == CombineMethod::Tf &&
			bAddTermNode_ == true &&
			(m_bDocumentFrequency || m_bTotalTermFrequency))
		{
			ModUnicodeString tea(p, static_cast<ModSize>(pTea_ - p));
			cSearchInfo_.addTermNode(tea, node);
		}
	}
			
	return node.release();
}

//
//	FUNCTION private
//	FullText2::Query::parseLogicalOperator -- 論理演算子共通の処理
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	ModVector<FullText2::ListManager*>& vecpManager_
//		リスト管理クラス
//	const ModUnicodeChar*& pTea_
//		tea構文
//	FullText2::LogicalOperatorNode* pNode_
//		論理演算子のノード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Query::parseLogicalOperator(SearchInformation& cSearchInfo_,
							ModVector<ListManager*>& vecpManager_,
							const ModUnicodeChar*& pTea_,
							LogicalOperatorNode* pNode_)
{
	ModUnicodeString combiner;

	if (*pTea_ == '[')
	{
		// スコア合成器の指定がある
		++pTea_;
		getToken(combiner, pTea_);
	}

	if (*pTea_ != '(')
	{
		// 先頭が '(' ではないので、エラー
		_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
	}
	++pTea_;

	// スコア合成器を設定する
	if (pNode_) pNode_->setScoreCombiner(getScoreCombiner(combiner));

	while (*pTea_ != 0)
	{
		OperatorNode* element = parseImpl(cSearchInfo_, vecpManager_, pTea_);
		if (pNode_) pNode_->pushBack(element);

		if (*pTea_ == ')')
		{
			++pTea_;
			break;
		}

		if (*pTea_ != ',')
		{
			// セパレータが ',' ではないので、エラー
			_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
		}
		
		++pTea_;
	}
}

//
//	FUNCTION private
// 	FullText2::Query::parseLeafNode -- LeafNode を作成する
//
//	NOTES
//
//	ARGUMETNS
//	FullText2::ListManager* pManager_
//		リスト管理クラス
//	const ModUnicodeChar*& pTea_
//		tea構文
//
//	RETURN
//	FullText2::LeafNode*
//		パース結果
//
//	EXCEPTIONS
//
LeafNode*
Query::parseLeafNode(ListManager* pManager_,
					 const ModUnicodeChar*& pTea_,
					 TermValue* pTermValue_)
{
	LeafNode* ret = 0;

	// 例えば #term[...](...) をパースする
	// パース後は、) の次の文字を指している

	if (*pTea_ != '#')
	{
		// 先頭が '#' ではないので、エラー
		_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
	}

	if (_cLocation.compare(pTea_, _cLocation.getLength()) == 0)
	{
		// #location[1](....)
		
		pTea_ += _cLocation.getLength();

		if (*pTea_ == '[')
			++pTea_;	// '[' の分
		else
			_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);

		ModUnicodeString s;
		getToken(s, pTea_);

		// ModSizeに変換する
		ModSize loc = ModUnicodeCharTrait::toUInt(s);

		// (....) を変換する
		LeafNode* node = parseUnaryLeafNode(pManager_, pTea_);

		// 先頭からの位置を処理するノードを作成する
		if (!m_bLocation)
			ret = new HeadLeafNode(node, loc);
	}
	else if (_cEnd.compare(pTea_, _cEnd.getLength()) == 0)
	{
		// #end[0](....)
		
		pTea_ += _cEnd.getLength();

		if (*pTea_ == '[')
			++pTea_;	// '[' の分
		else
			_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);

		ModUnicodeString s;
		getToken(s, pTea_);

		// ModSizeに変換する
		ModSize loc = ModUnicodeCharTrait::toUInt(s);

		// (....) を変換する
		LeafNode* node = parseUnaryLeafNode(pManager_, pTea_);

		// 終端からの位置を処理するノードを作成する
		if (!m_bLocation)
			ret = new TailLeafNode(node, loc);
	}
	else if (_cSyn.compare(pTea_, _cSyn.getLength()) == 0)
	{
		// #syn(...)

		pTea_ += _cSyn.getLength();
		
		ModAutoPointer<AtomicOrLeafNode> node = new AtomicOrLeafNode();

		ModAutoPointer<TermValue> p = 0;
		if (m_bLocation)
			p = new TermValue;
		
		parseArrayLeafNode(pManager_, pTea_, node.get(), p.get());

		if (m_bLocation)
		{
			m_mapTermLeafNode.insert(*p.get(), node.release());
		}
		else
		{
			ret = node.release();
		}
	}
	else if (_cTerm.compare(pTea_, _cTerm.getLength()) == 0)
	{
		// #term[m,,ja](...)

		pTea_ += _cTerm.getLength();
		
		TermValue* t = pTermValue_;
		
		ModAutoPointer<TermValue> p = 0;
		if (t == 0 && m_bLocation)
		{
			p = new TermValue;
			t = p.get();
		}
		
		ret = parseTerm(pManager_, pTea_, t);
		
		if (p.get())
		{
			m_mapTermLeafNode.insert(*t, ret);
			ret = 0;
		}
	}
	else if (_cWindow.compare(pTea_, _cWindow.getLength()) == 0)
	{
		// #window[1,100,o](...)

		pTea_ += _cWindow.getLength();

		if (*pTea_ == '[')
			++pTea_;	// '[' の分
		else
			_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);

		ModSize upper = 0;
		ModSize lower = 0;
		bool ordered = true;

		// オプションを解析する

		ModUnicodeString s;
		getToken(s, pTea_);
		if (*pTea_ != '(')
		{
			// #window[1,100 以上が決定
			lower = ModUnicodeCharTrait::toUInt(s);
			getToken(s, pTea_);
			upper = ModUnicodeCharTrait::toUInt(s);
				
			if (*pTea_ != '(')
			{
				// #window[1,100,o] の形
				getToken(s, pTea_);

				if (_cOrdered.compare(s) == 0)
				{
					ordered = true;
				}
				else if (_cUnordered.compare(s) == 0)
				{
					ordered = false;
				}
				else
				{
					_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
				}
			}
		}
		else
		{
			// #window[100] の形
			
			upper = ModUnicodeCharTrait::toUInt(s);
		}

		ModAutoPointer<ArrayLeafNode> node = 0;
		if (m_bLocation == false)
		{
			if (ordered == true)
				node = new WithinOrderedLeafNode(lower, upper);
			else
				node = new WithinUnorderedLeafNode(lower, upper);
		}
		
		parseArrayLeafNode(pManager_, pTea_, node.get());

		ret = node.release();
	}
	else
	{
		// エラー
		_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
	}

	return ret;
}

//
//	FUNCTION private
//	FullText2::Query::parseTerm -- 検索語ノードをパースする
//
//	NOTES
//
//	ARUGMENTS
// 	FullText2::ListManager* pManager_
//		リスト管理クラス
//	const ModUnicodeChar*& pTea_
//		tea構文
//	FullText2::Query::TermValue* pTermValue_
//		検索語データ
//
//	RETURN
//	LeafNode*
//		検索語ノード
//
//	EXCEPTIONS
//
LeafNode*
Query::parseTerm(ListManager* pManager_,
				 const ModUnicodeChar*& pTea_,
				 TermValue* pTermValue_)
{
	// #term[m,,ja](...) の [m,,ja](...) の部分をパースする
	
	if (*pTea_ == '[')
		++pTea_;	// '[' の分
	else
		_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);

	ModUnicodeString cValue;
	ModLanguageSet cLang;

	// 一致条件
	ModUnicodeString s;
	getToken(s, pTea_);
	MatchMode::Value eMatchMode = getMatchMode(s);

	if (*pTea_ != '(')
	{
		// スコア計算器(今は空しか許していない)
		getToken(s, pTea_);
		if (s.getLength() != 0)
			_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
	}

	if (*pTea_ != '(')
	{
		// 言語
		getToken(s, pTea_);
		cLang = s;
	}

	if (*pTea_ != '(')
		_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
	++pTea_;

	// 検索語
	getToken(cValue, pTea_);

	if (eMatchMode == MatchMode::MultiLanguage)
	{
		if (_cIsMulti.get() == true)
		{
			// 多言語対応モード
			// 文字列にCJKが含まれているときは文字列検索
			// 含まれていないときは厳格単語検索

			eMatchMode = MatchMode::ExactWord;

			const ModUnicodeChar* p = cValue;
			while (*p != 0)
			{
				if (ModUnicodeCharTrait::isAlphabet(*p) == ModTrue ||
					ModUnicodeCharTrait::isDigit(*p) == ModTrue ||
					ModUnicodeCharTrait::isSymbol(*p) == ModTrue ||
					ModUnicodeCharTrait::isSpace(*p) == ModTrue)
				{
					++p;
					continue;
				}
				else
				{
					eMatchMode = MatchMode::String;
					break;
				}
			}
		}
		else
		{
			// 文字列検索
			eMatchMode = MatchMode::String;
		}
	}

	// LeafNodeを作成する
	LeafNode* node = pManager_->getTokenizer()->createLeafNode(*pManager_,
															   cValue,
															   cLang,
															   eMatchMode);

	if (node == 0)
	{
		// ヒットしない条件だったので、ダミーのLeafNodeを割り当てる

		node = LeafNode::createEmptyLeafNode();
	}

	if (pTermValue_ && cValue.getLength())
	{
		Utility::SearchTermData d;
		d.setTerm(cValue);
		d.setLanguage(cLang);
		switch (eMatchMode)
		{
		case MatchMode::WordHead:
			d.setMatchMode(Utility::SearchTermData::MatchMode::WordHead);
			break;
		case MatchMode::WordTail:
			d.setMatchMode(Utility::SearchTermData::MatchMode::WordTail);
			break;
		case MatchMode::SimpleWord:
			d.setMatchMode(Utility::SearchTermData::MatchMode::SimpleWord);
			break;
		case MatchMode::ExactWord:
			d.setMatchMode(Utility::SearchTermData::MatchMode::ExactWord);
			break;
		case MatchMode::String:
		default:
			d.setMatchMode(Utility::SearchTermData::MatchMode::String);
			break;
		}
		
		pTermValue_->pushBack(d);
	}

	return node;
}

//
//	FUNCTION private
//	FullText2::Query::parseUnaryLeafNode -- (A)をパースする
//
//	NOTES
//
//	ARGUMENTS
// 	FullText2::ListManager* pManager_
//		リスト管理クラス
//	const ModUnicodeChar*& pTea_
//		tea構文
//
//	RETURN
//	FullText2::LeafNode*
//		要素
//
//	EXCEPTIONS
//
LeafNode*
Query::parseUnaryLeafNode(ListManager* pManager_,
						  const ModUnicodeChar*& pTea_)
{
	LeafNode* ret = 0;
	
	// ここには、#hogehoge[...](...) の (...) の部分が来る

	if (*pTea_ != '(')
	{
		// 先頭が '(' ではないので、エラー
		_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
	}
	++pTea_;

	if (*pTea_ != 0)
	{
		ret = parseLeafNode(pManager_, pTea_);

		if (*pTea_ == ')')
		{
			++pTea_;
		}
		else
		{
			// ')' ではないので、エラー
			_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
		}
	}

	return ret;
}

//
//	FUNCTION private
//	FullText2::Query::parseArrayLeafNode -- (A,B,C,...)をパースする
//
//	NOTES
//
//	ARGUMENTS
// 	FullText2::ListManager* pManager_
//		リスト管理クラス
//	const ModUnicodeChar*& pTea_
//		tea構文
//	FullText2::ArrayLeafNode* node_
//		配列ノード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Query::parseArrayLeafNode(ListManager* pManager_,
						  const ModUnicodeChar*& pTea_,
						  ArrayLeafNode* node_,
						  TermValue* pTermValue_)
{
	// ここには、#hogehoge[...](A,B,C,...) の (A,B,C,...) の部分が来る

	if (*pTea_ != '(')
	{
		// 先頭が '(' ではないので、エラー
		_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
	}
	++pTea_;

	while (*pTea_ != 0)
	{
		LeafNode* element = parseLeafNode(pManager_, pTea_, pTermValue_);
		if (node_) node_->pushBack(element);

		if (*pTea_ == ')')
		{
			++pTea_;
			break;
		}

		if (*pTea_ != ',')
		{
			// セパレータが ',' ではないので、エラー
			_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
		}
		
		++pTea_;
	}
}

//
//	FUNCTION private
//	FullText2::Query::parseFreeText -- 自然文検索をパースする
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	ModVector<FullText2::ListManager*>& vecpManager_
//		リスト管理クラス
//	const ModUnicodeChar*& pTea_
//		tea構文
//
//	RETURN
//	FullText2::OperatorNode*
//		検索ノード
//
//	EXCEPTIONS
//
OperatorNode*
Query::parseFreeText(SearchInformation& cSearchInfo_,
					 ModVector<ListManager*>& vecpManager_,
					 const ModUnicodeChar*& pTea_)
{
	// 自然文から単語を抽出する -- プールも確保される
	makePoolFreeText(pTea_);

	// DFを取得する
	getDocumentFrequency(cSearchInfo_, vecpManager_, *m_pPool1);

	if (m_bEstimate)
	{
		// 検索結果件数を見積もる
		// シード文書はあっても無視する

		m_uiEstimateCount = estimateCountFromPool(cSearchInfo_, *m_pPool1);

		return 0;
	}

	if (m_vecDocument.getSize())
	{
		// シード文書がある

		expandPool(cSearchInfo_, vecpManager_, *m_pPool1, *m_pPool2);
	}

	OperatorNode* ret = 0;

	// 抽出した単語はすべて OR でつなげればいい

	ModAutoPointer<OperatorOrNode> orNode = new OperatorOrNode();
	orNode->setScoreCombiner(getScoreCombiner(ModUnicodeString()));

	// 1つ以上の条件を設定したか
	bool isAdd = false;

	Utility::ModTermPool::Iterator i;
	for (i = m_pPool1->begin(); i != m_pPool1->end(); ++i)
	{
		// 選択値が 0 の場合は、条件に含めない
		if ((*i).getTsv() == 0) continue;

		// 検索ノードを作成する
		orNode->pushBack(makeOperatorNode(cSearchInfo_, vecpManager_, i));
		isAdd = true;
	}

	if (isAdd == false)
	{
		// 1つも条件に加わらなかったので、すべてを加える
		
		for (i = m_pPool1->begin(); i != m_pPool1->end(); ++i)
		{
			// 検索ノードを作成する
			orNode->pushBack(makeOperatorNode(cSearchInfo_, vecpManager_, i));
		}
	}

	for (i = m_pPool2->begin(); i != m_pPool2->end(); ++i)
	{
		// 選択値が 0 の場合は、条件に含めない
		if ((*i).getTsv() == 0) continue;

		// 検索ノードを作成する
		orNode->pushBack(makeOperatorNode(cSearchInfo_, vecpManager_, i));
	}

	ret = orNode.release();

	return ret;
}

//
//	FUNCTION private
//	FullText2::Query::parseWordList -- 単語リストをパースする
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	ModVector<FullText2::ListManager*>& vecpManager_
//		リスト管理クラス
//	const ModUnicodeChar*& pTea_
//		tea構文
//
//	RETURN
//	FullText2::OperatorNode*
//		検索ノード
//
//	EXCEPTIONS
//
OperatorNode*
Query::parseWordList(SearchInformation& cSearchInfo_,
					 ModVector<ListManager*>& vecpManager_,
					 const ModUnicodeChar*& pTea_)
{
	// 単語リストから単語を抽出する -- プールも確保される
	makePoolWordList(pTea_);

	// DFを取得する
	getDocumentFrequency(cSearchInfo_, vecpManager_, *m_pPool1);

	if (m_bEstimate)
	{
		// 検索結果件数を見積もる
		// シード文書はあっても無視する

		m_uiEstimateCount = estimateCountFromPool(cSearchInfo_, *m_pPool1);

		return 0;
	}

	if (m_vecDocument.getSize())
	{
		// シード文書がある

		expandPool(cSearchInfo_, vecpManager_, *m_pPool1, *m_pPool2);
	}

	OperatorNode* ret = 0;

	ModAutoPointer<OperatorAndNode> andNode;
	ModAutoPointer<OperatorOrNode> orNode;

	// カテゴリが Essential なものは andNode に push し、
	// それ以外のものは、orNode に push する
	
	Utility::ModTermPool::Iterator i;
	for (i = m_pPool1->begin(); i != m_pPool1->end(); ++i)
	{
		// 選択値が 0 の場合は、条件に含めない
		if ((*i).getTsv() == 0) continue;

		// 検索ノードを作成する
		ModAutoPointer<OperatorNode> node
			= makeOperatorNode(cSearchInfo_, vecpManager_, i);

		if ((*i).getType() == Common::WordData::Category::Essential ||
			(*i).getType() == Common::WordData::Category::EssentialRelated)
		{
			// 必須語なので、andNodeにpushする
			
			if (andNode.get() == 0)
			{
				andNode = new OperatorAndNode();
				andNode->setScoreCombiner(getScoreCombiner(ModUnicodeString()));
			}

			andNode->pushBack(node.release());
		}
		else
		{
			// 必須語ではないので、orNodeにpushする
			if (orNode.get() == 0)
			{
				orNode = new OperatorOrNode();
				orNode->setScoreCombiner(getScoreCombiner(ModUnicodeString()));
			}

			orNode->pushBack(node.release());
		}
	}

	for (i = m_pPool2->begin(); i != m_pPool2->end(); ++i)
	{
		// 選択値が 0 の場合は、条件に含めない
		if ((*i).getTsv() == 0) continue;

		// シード文書の単語はすべて必須語ではない
		if (orNode.get() == 0)
		{
			orNode = new OperatorOrNode();
			orNode->setScoreCombiner(getScoreCombiner(ModUnicodeString()));
		}

		// 検索ノードを作成し、orNodeにpushする
		orNode->pushBack(makeOperatorNode(cSearchInfo_, vecpManager_, i));
	}

	if (andNode.get())
	{
		if (orNode.get())
		{
			// 必須語とそうでない語があるので、OperatorAddNodeを挟む

			ModAutoPointer<OperatorAddNode> addNode = new OperatorAddNode();
			addNode->setNode(andNode.release(), orNode.release());
			addNode->setScoreCombiner(getScoreCombiner(ModUnicodeString()));
			ret = addNode.release();
		}
		else
		{
			ret = andNode.release();
		}
	}
	else
	{
		if (orNode.get() == 0)
		{
			// １つも単語がないので、とりあえず空のノードを作成する
			orNode = new OperatorOrNode();
			orNode->setScoreCombiner(getScoreCombiner(ModUnicodeString()));
		}
		
		ret = orNode.release();
	}

	return ret;
}

//
//	FUNCTION private
//	FullText2::Query::parseWord -- 単語をパースする
//
//	NOTES
//
//	ARUGMENTS
//	const ModUnicodeChar*& pTea_
//		tea構文
//	Utility::ModTermElement& term
//		中身を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Query::parseWord(const ModUnicodeChar*& pTea_,
				 Utility::ModTermElement& term)
{
	// #word[m,ja,Helpful,1.0,0](...) が来る

	if (_cWord.compare(pTea_, _cWord.getLength()) != 0)
		_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);

	pTea_ += _cWord.getLength();

	if (*pTea_ != '[')
		_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
	++pTea_;

	// 一致条件
	ModUnicodeString s;
	getToken(s, pTea_);
	term.setMatchMode(getMatchModeForTerm(s));

	// 言語
	getToken(s, pTea_);
	term.setLangSpec(s);

	// カテゴリ
	getToken(s, pTea_);
	term.setType(Common::WordData::toCategory(s));

	// スケール
	getToken(s, pTea_);
	term.setScale(ModUnicodeCharTrait::toDouble(s));

	// 文書頻度
	getToken(s, pTea_);
	term.setDf(ModUnicodeCharTrait::toDouble(s));
	if (term.getDf() != 0)
	{
		// 複合索引時のスコア合成方法が None または Tf の場合のみ指定可能
		if (m_eMultiMethod != CombineMethod::None &&
			m_eMultiMethod != CombineMethod::Tf)
			_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
	}

	// その他のデフォルト値
	term.setTwv(1);

	if (*pTea_ != '(')
		_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
	++pTea_;

	// 単語
	getToken(s, pTea_);
	term.setString(m_pTerm->getNormalizedString(term.getLangSpec(), s));
	term.setOriginalString(s);
}

//
//	FUNCTION private
//	FullText2::Query::makePoolFreeText -- 自然文から単語の抽出する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar*& pTea_
//		tea構文
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Query::makePoolFreeText(const ModUnicodeChar*& pTea_)
{
	// #freetext[m,ja,0.3,30](...) の [m,ja,0.3,30](...) が来る

	if (*pTea_ == '[')
		++pTea_;	// '[' の分
	else
		_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);

	ModUnicodeString s;
	ModLanguageSet cLang;

	// 一致条件
	getToken(s, pTea_);
	m_iMatchMode = getMatchModeForTerm(s);

	if (*pTea_ != '(')
	{
		// 言語
		getToken(s, pTea_);
		cLang = s;
	}

	if (*pTea_ != '(')
	{
		// paramScale1
		getToken(s, pTea_);
		if (s.getLength())
		{
			double scale = ModUnicodeCharTrait::toDouble(s);
			m_pTerm->paramScale1 = scale;
		}
	}

	if (*pTea_ != '(')
	{
		// maxTerm1
		getToken(s, pTea_);
		if (s.getLength())
		{
			ModSize maxTerm = ModUnicodeCharTrait::toInt(s);
			m_pTerm->maxTerm1 = maxTerm;
		}
	}

	if (*pTea_ != '(')
		_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
	++pTea_;

	// 自然文
	ModUnicodeOstrStream cValue;
	getToken(cValue, pTea_);

	// プールを作成する
	m_pPool1 = new Utility::ModTermPool(m_pTerm->maxTerm1);
	m_pPool2 = new Utility::ModTermPool(m_pTerm->maxTerm2);

	// 自然文から pool を作成
	m_pTerm->poolTerm(cValue.getString(), cLang, m_iMatchMode, *m_pPool1);

	// すべてのカテゴリーをHelpfulにする
	Utility::ModTermPool::Iterator i = m_pPool1->begin();
	for (; i != m_pPool1->end(); ++i)
	{
		(*i).setType(Common::WordData::Category::Helpful);
	}
}

//
//	FUNCTION private
//	FullText2::Query::makePoolWordList -- 単語リストから単語の抽出する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar*& pTea_
//		tea構文
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Query::makePoolWordList(const ModUnicodeChar*& pTea_)
{
	// #wordlist[10](#word[..],#word[..],...) の
	// [10](#word[..],#word[..],...) が来る

	if (*pTea_ == '[')
		++pTea_;	// '[' の分
	else
		_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);

	// 単語数
	ModUnicodeString s;
	getToken(s, pTea_);
	ModSize wordCount = static_cast<ModSize>(ModUnicodeCharTrait::toInt(s));
	m_pTerm->maxTerm1 = (m_pTerm->maxTerm1 < wordCount) ?
		wordCount : m_pTerm->maxTerm1;

	if (*pTea_ != '(')
		_TRMEISTER_THROW1(Exception::WrongParameter, pTea_);
	++pTea_;

	// プールを確保する
	m_pPool1 = new Utility::ModTermPool(m_pTerm->maxTerm1);
	m_pPool2 = new Utility::ModTermPool(m_pTerm->maxTerm2);
	
	int pos = 0;

	while (*pTea_ != 0)
	{
		Utility::ModTermElement term;
		parseWord(pTea_, term);
		term.setPosition(pos);
		++pos;

		m_pPool1->insertTerm(term);

		if (*pTea_ == ',')
			++pTea_;
		else if (*pTea_ == ')')
			break;
	}

	// TfListのための配列を確保する
	if (m_bTfList)
	{
		m_vecTermNodeForTfList.assign(pos, 0);
	}

	m_pTerm->validatePool(*m_pPool1);
}

//
//	FUNCTION private
//	FullText2::Query::makeOperatorNode -- 検索ノードを作成する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	ModVector<ListManager&>& vecpManager_
//		リスト管理クラス
//	Utility::ModTermPool::Iterator& ite_
//		検索語プールのイテレータ
//
//	RETURN
//	OperatorNode*
//		検索ノード
//
//	EXCEPTIONS
//
OperatorNode*
Query::makeOperatorNode(SearchInformation& cSearchInfo_,
						ModVector<ListManager*>& vecpManager_,
						Utility::ModTermPool::Iterator& ite_)
{
	// 検索ノードを得る
	const ModUnicodeString& tea = (*ite_).getFormula();
	const ModUnicodeChar* p = tea;
	ModAutoPointer<OperatorTermNode> tmp
		= parseTermNode(cSearchInfo_, vecpManager_, p, true,
						(*ite_).getCalculator());
	
	if (m_bTeaString)
	{
		// 位置情報取得のための検索文が必要
		if (m_cConditionForLocation.getLength() == 0)
		{
			m_cConditionForLocation.append("(#or(");
		}
		else
		{
			m_cConditionForLocation.append(",");
		}
		m_cConditionForLocation.append((*ite_).getFormula());
	}
	
	// 位置を設定する
	if (m_bTfList && (*ite_).getPosition() != -1)
	{
		m_vecTermNodeForTfList[(*ite_).getPosition()] = tmp;
	}
	
	ModAutoPointer<OperatorNode> ret = tmp.release();

	// スケールを計算する
	double scaleValue = (m_cCalculator.getLength() == 0) ?
		(((*ite_).getScale() ? (*ite_).getScale() : 1)
		 * ((*ite_).getWeight() ? (*ite_).getWeight() : 1)) :
		((*ite_).getScale() ? (*ite_).getScale() : 1);

	if (scaleValue != 1)
	{
		// スケールが 1 ではないので、重みノードを挟む
		ret = new OperatorWeightNode(scaleValue, ret.release());
	}
	
	return ret.release();
}

//
//	FUNCTION private
//	FullText2::Query::expandPool -- シード文書で検索語プールを拡張する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	ModVector<FullText2::ListManager*>& vecpManager_
//		リスト管理クラス
//	Utility::ModTermPool& pool
//		初期検索語の検索語プール
//	Utility::ModTermPool& pool2
//		シード文書の検索語プール
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Query::expandPool(SearchInformation& cSearchInfo_,
				  ModVector<ListManager*>& vecpManager_,
				  Utility::ModTermPool& pool,
				  Utility::ModTermPool& pool2)
{
	// シード文書から検索語をマップに格納
	
	Utility::ModTermMap map;

	ModVector<ModUnicodeString>::Iterator d = m_vecDocument.begin();
	ModVector<ModLanguageSet>::Iterator l = m_vecLanguage.begin();
	ModSize id = 1;
	for (; d != m_vecDocument.end(); ++d, ++l, ++id)
	{
		m_pTerm->mapTerm(*d, *l, m_iMatchMode, id, map);
	}

	// 重みをつける
	double collectionSize = cSearchInfo_.getTotalDocumentFrequency();
	m_pTerm->weightTerm(map, pool, collectionSize);

	// 拡張語の候補用プール
	Utility::ModTermPool cand(m_pTerm->maxCandidate);
	// 拡張語の候補をプール
	m_pTerm->poolTerm(map, cand);

	// 各候補にDFを設定
	getDocumentFrequency(cSearchInfo_, vecpManager_, cand);
	
	// 候補群から拡張検索後を選択
	m_pTerm->selectTerm(map, cand, pool2, collectionSize);

	// すべてのカテゴリーをHelpfulRelatedにする
	Utility::ModTermPool::Iterator i = pool2.begin();
	for (; i != pool2.end(); ++i)
	{
		(*i).setType(Common::WordData::Category::HelpfulRelated);
	}
}

//
//	FUNCTION private
//	FullText2::Query::getDoucmentFrequency -- DFを取得する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	ModVector<FullText2::ListManager*>& vecpManager_
//		リスト管理クラス
//	Utility::ModTermPool& cPool_
//		検索語プール
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Query::getDocumentFrequency(SearchInformation& cSearchInfo_,
							ModVector<ListManager*>& vecpManager_,
							Utility::ModTermPool& pool)
{
	int n = 0;
	
	// 検索語プールの検索語を SearchInformation に登録する
	Utility::ModTermPool::Iterator s = pool.begin();
	Utility::ModTermPool::Iterator i = s;
	while (i != pool.end())
	{
		if ((*i).getDf() != 0)
		{
			// すでに求められているので、検索対象にしない
			++i; 	// 次へ
			continue;
		}

		// tea構文を得る
		const ModUnicodeString& tea = (*i).getFormula();

		// tea構文を検索ノードに変換する
		const ModUnicodeChar* p = tea;
		OperatorTermNode* node
			= parseTermNode(cSearchInfo_, vecpManager_, p, false,
							ModUnicodeString());
		
		// 検索ノードを設定する
		ModPair<PoolTermMap::Iterator, ModBoolean> r
			= m_mapPoolTermNode.insert(tea, node);
		if (r.second == ModTrue)
		{
			// 登録できたので、設定する
			cSearchInfo_.addTermNode(tea, node);
			++n;
		}

		++i;	// 次へ

		if (n == _cDFCalculatingLimit.get())
		{
			// 上限に達したので、文書頻度を求める
			getDocumentFrequency(cSearchInfo_);

			// PoolにDF値を設定する
			setDocumentFrequencyForTermPool(cSearchInfo_, s, i);

			// 変数をクリアする
			n = 0;
			s = i;
		}
	}
		
	// 残りを処理する
	getDocumentFrequency(cSearchInfo_);

	// PoolにDF値を設定する
	setDocumentFrequencyForTermPool(cSearchInfo_, s, i);
}

//
//	FUNCTION private
//	FullText2::Query::getDocumentFrequency -- DFを取得する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		
//
//	RETURN
//	なし
//
//	EXCPTIONS
//
void
Query::getDocumentFrequency(SearchInformation& cSearchInfo_)
{
	if (m_bEstimate)
		// 見積り
		cSearchInfo_.estimateDocumentFrequency();
	else
		// 正確な値
		cSearchInfo_.setUpDocumentFrequency(m_bTotalTermFrequency);
}

//
//	FUNCTION private
//	FullText2::Query::setDocumentFrequencyForTermPool
//		-- Utility::ModTermPoolにDF値を設定する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	Utility::ModTermPool::Iterator s_
//		始端
//	Utility::ModTermPool::Iterator e_
//		終端
//
// 	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Query::setDocumentFrequencyForTermPool(SearchInformation& cSearchInfo_,
									   Utility::ModTermPool::Iterator s_,
									   Utility::ModTermPool::Iterator e_)
{
	Utility::ModTermPool::Iterator j = s_;
	for (; j < e_; ++j)
	{
		// tea構文を得る
		const ModUnicodeString& tea = (*j).getFormula();

		if ((*j).getDf() != 0)
		{
			// DF値が与えられているので検索は実行しなかったが、
			// スコア計算のために情報のエントリは必要
			//
			// 総文書内頻度を指定するインターフェースがまだないので、
			// 念のため文書頻度を設定しておく
			
			cSearchInfo_.setDocumentFrequency(tea,
											  (*j).getDf(),
											  (*j).getDf());
		}
		else
		{
			// マップを検索する
			SearchInformation::MapValue* v = cSearchInfo_.findTermNode(tea);
		
			if (v == 0)
				// ありえない
				_TRMEISTER_THROW0(Exception::BadArgument);

			(*j).setDf(v->m_dblDocumentFrequency);
		}

		if (m_bQueryTermFrequency)
		{
			// 検索文内頻度が必要

			// 検索文内の頻度がすでに設定されていても
			// シード文書内の頻度をさらに加える
			
			cSearchInfo_.setQueryTermFrequency(
				tea,
				((*j).getTf() <= 1) ? 1.0 : (*j).getTf());
		}
	}

	// DFを求めたので、ノードを削除する
	PoolTermMap::Iterator ii = m_mapPoolTermNode.begin();
	PoolTermMap::Iterator ee = m_mapPoolTermNode.end();
	for (; ii != ee; ++ii)
	{
		delete (*ii).second;
	}
	m_mapPoolTermNode.erase(m_mapPoolTermNode.begin(),
							m_mapPoolTermNode.end());
}

//
//	FUNCTION private static
//	FullText2::Query::getMatchMode -- 一致条件を得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& mode_
//		一致条件の文字列表記
//
//	RETURN
//	FullText2::MatchMode::Value
//		一致条件
//
//	EXCEPTIONS
//
MatchMode::Value
Query::getMatchMode(const ModUnicodeString& mode_)
{
	MatchMode::Value eMatchMode;
	
	if (_cMulti.compare(mode_) == 0)
	{
		eMatchMode = MatchMode::MultiLanguage;
	}
	else if (_cString.compare(mode_) == 0)
	{
		eMatchMode = MatchMode::String;
	}
	else if (_cExactWord.compare(mode_) == 0)
	{
		eMatchMode = MatchMode::ExactWord;
	}
	else if (_cWordHead.compare(mode_) == 0)
	{
		eMatchMode = MatchMode::WordHead;
	}
	else if (_cWordTail.compare(mode_) == 0)
	{
		eMatchMode = MatchMode::WordTail;
	}
	else if (_cSimpleWord.compare(mode_) == 0)
	{
		eMatchMode = MatchMode::SimpleWord;
	}
	else
	{
		_TRMEISTER_THROW1(Exception::WrongParameter, mode_);
	}

	return eMatchMode;
}

//
//	FUNCTION private static
//	FullText2::Query::getMatchMode -- 一致条件を得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& mode_
//		一致条件の文字列表記
//
//	RETURN
//	FullText2::MatchMode::Value
//		一致条件
//
//	EXCEPTIONS
//
int
Query::getMatchModeForTerm(const ModUnicodeString& mode_)
{
	int mode = Utility::ModTermElement::voidMatch;
	switch (getMatchMode(mode_))
	{
	case MatchMode::MultiLanguage:
		mode = Utility::ModTermElement::multiMatch;
		break;
	case MatchMode::String:
		mode = Utility::ModTermElement::stringMatch;
		break;
	case MatchMode::WordHead:
		mode = Utility::ModTermElement::headMatch;
		break;
	case MatchMode::WordTail:
		mode = Utility::ModTermElement::tailMatch;
		break;
	case MatchMode::SimpleWord:
		mode = Utility::ModTermElement::simpleMatch;
		break;
	case MatchMode::ExactWord:
		mode = Utility::ModTermElement::exactMatch;
		break;
	}
	return mode;
}

//
//	FUNCTION private
//	FullText2::Query::estimateCountFromPool
//		-- 検索語プールから検索結果件数を見積もる
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	Utility::ModTermPool& cPool_
//		検索語プール
//
//	RETURN
//	ModSize
//		検索件数見積
//
//	EXCEPTIONS
//
ModSize
Query::estimateCountFromPool(SearchInformation& cSearchInfo_,
							 Utility::ModTermPool& cPool_)
{
	ModSize totalCount = cSearchInfo_.getDocumentCount();
	double docCount = static_cast<double>(totalCount);
	double ratio = 0;

	// 必須語があることは少ないので、まずは必須語がないと仮定して、
	// すべての語で OR 条件の処理をしながら、必須語の有無をチェックする
	// 必須語があったら、その後は、必須語のみで AND 条件の処理を行う

	bool bEssential = false;

	Utility::ModTermPool::Iterator i;
	for (i = cPool_.begin(); i != cPool_.end(); ++i)
	{
		// 選択値が 0 の場合は、条件に含めない
		if ((*i).getTsv() == 0) continue;

		if ((*i).getType() == Common::WordData::Category::Essential ||
			(*i).getType() == Common::WordData::Category::EssentialRelated)
		{
			// 必須語があった

			if (bEssential == false)
			{
				// 初めてなので、ratio をクリアする
				
				ratio = 1.0;
				bEssential = true;
			}
			
			double r = static_cast<double>((*i).getDf()) / docCount;
			ratio *= r;
		}

		if (bEssential == false)
		{
			// 必須語はまだない

			double r = static_cast<double>((*i).getDf()) / docCount;
			ratio += (1.0 - ratio) * r;
		}
	}

	// 割合が求まったので、件数にする
	
	ModSize count = static_cast<ModSize>(docCount * ratio);
	return (count < totalCount) ? count : totalCount;
}

//
//	Copyright (c) 2010, 2011, 2013, 2016, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
