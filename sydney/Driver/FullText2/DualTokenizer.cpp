// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DualTokenizer.cpp
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
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
#include "FullText2/DualTokenizer.h"

#include "FullText2/DummyListIterator.h"
#include "FullText2/FeatureList.h"
#include "FullText2/FullTextFile.h"
#include "FullText2/ListManager.h"
#include "FullText2/Parameter.h"
#include "FullText2/WordLeafNode.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "ModAlgorithm.h"
#include "ModOsDriver.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

#define LOG(x)	 ModOsDriver::Math::log(x)

namespace
{
	//
	//	VARIABLE
	//	_$$::_MaxOccurrenceCost -- 生起コストの最大値
	//
	ParameterInteger
	_MaxOccurrenceCost("FullText2_MaxOccurrenceCost", 500, false);
	
	//
	//	VARIABLE
	//	_$$::_AlphabetOccurrenceCostFactor -- 英文字列の生起コストの係数
	//
	ParameterInteger
	_AlphabetOccurrenceCostFactor("FullText2_AlphabetOccurrenceCostFactor",
								  25, false);

	//
	//	VARIABLE
	//	_$$::_ExpandLimit -- 検索語を展開する上限
	//
	ParameterInteger
	_ExpandLimit("FullText2_QueryExpandLimit", 200, false);
}

//
//	FUNCTION public
//	FullText2::DualTokenizer::DualTokenizer -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::FullTextFile& cFile_
//		全文索引ファイル
//	UNA::ModNlpAnalyzer* pAnalyzer_
//		UNAアナライザー
//	const ModUnicodeChar* pParameter_
//		トークナイズパラメータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
DualTokenizer::DualTokenizer(FullTextFile& cFile_,
							 UNA::ModNlpAnalyzer* pAnalyzer_,
							 const ModUnicodeChar* pParameter_)
	: NgramTokenizer(cFile_, pAnalyzer_, 0),
	  m_uiFeatureSize(0), m_pTermResource(0)
{
	if (m_eIndexingType != IndexingType::Word)
	{
		// 単語単位索引ではない場合には、ブロック化器が必要
		
		if (pParameter_) setBlocker(pParameter_);
	}
}

//
//	FUNCTION public
//	FullText2::DualTokenizer::~DualTokenizer -- デストラクタ
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
DualTokenizer::~DualTokenizer()
{
}

//
//	FUNCTION public
//	FullText2::DualTokenizer::getFeatureList
//		-- 特徴語リストを取得する
//
//	NOTES
//
//	ARGUEMNTS
//	FullText2::FeatureList& vecFeature_
//	   	特徴語リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DualTokenizer::getFeatureList(FeatureList& vecFeature_)
{
	if (m_uiFeatureSize == 0)
		// 特徴語を取得するモードではない
		return;
	
	vecFeature_.clear();
	vecFeature_.reserve(m_mapFeatureCandidate.getSize());

	FeatureMap::Iterator i0 = m_mapFeatureCandidate.begin();
	FeatureMap::Iterator e0 = m_mapFeatureCandidate.end();
	for (; i0 != e0; ++i0)
	{
		// 特徴語の重みは、log(TF+1)*単語生起コスト
		//
		// [NOTE] ここでは重みを正規化しない。
		// 参照 FullText::LogicalInterface::makeFeatureFieldData()
		
		vecFeature_.pushBack(
			FeatureElement(
				(*i0).first,
				LOG((*i0).second.first + 1) * (*i0).second.second));
	}

	// 重みの降順にソート
	ModSort(vecFeature_.begin(), vecFeature_.end(), FeatureElement::Greator());

	// ここで上位n件を特徴語とする。
	// m_uiFeatureSizeは、システムパラメータとして、index作成時に指定する
	// defaultは10
	// 特徴語の重みが等しい場合は、途中でcutしない
	
	FeatureList::Iterator b1 = vecFeature_.begin();
	FeatureList::Iterator e1 = vecFeature_.end();
	FeatureList::Iterator i1;
	
	for (i1 = b1; i1 != e1; ++i1)
	{
		// 特徴語の重みが前後で等しい場合は、指定された特徴語数(n)に達した場合
		// でもループからぬけない
		// ただし、特徴語数の２倍を超えたらぬける
		
		if ((i1 + 1) < e1 && (*(i1 + 1)).scale == (*i1).scale
			&& (i1 - b1) < static_cast<int>(m_uiFeatureSize) * 2)
			continue;
		if ((i1 - b1) >= static_cast<int>(m_uiFeatureSize))
			break;
	}
	
	// 残りの特長語を削除する
	if (i1 != e1)
		vecFeature_.erase(i1, e1);
}

//
//	FUNCTION protected
//	FullText2::DualTokenizer::createLeafNodeImpl -- 検索用のLeafNodeを作成する
//
//	NOTES
//
//	ARGUMETNS
//	FullText2::ListManager& cManager_
//		リストマネージャー
//	const ModUnicodeString& cTerm_
//		検索キーワード
//	const ModLanguageSet& cLang_
//		言語
//	FullText2::MatchMode::Value eMatchMode_
//		一致モード
//
//	RETURN
//	FullText2::LeafNode*
//		検索ノード。ただし、
//		正規化の結果、空文字列になったり、検索にヒットしないようなものの場合、
//		null を返す
//
//	EXCEPTIONS
//
LeafNode*
DualTokenizer::createLeafNodeImpl(ListManager& cManager_,
								  const ModUnicodeString& cTerm_,
								  const ModLanguageSet& cLang_,
								  MatchMode::Value eMatchMode_)
{
	// サポートしている一致モードを確認する
	if ((m_eIndexingType == IndexingType::Ngram &&
		 eMatchMode_ != MatchMode::String) ||
		(m_eIndexingType == IndexingType::Word &&
		 eMatchMode_ != MatchMode::ExactWord &&
		 eMatchMode_ != MatchMode::WordHead))

		// 索引種別がN-gramで、かつ、文字列検索じゃないときか、
		// 索引種別がWordで、かつ、完全単語境界一致検索でも先頭単語境界一致検索
		// でもないときは、エラー
		
		_TRMEISTER_THROW0(Exception::NotSupported);

	LeafNode* ret = 0;

	if (m_eIndexingType == IndexingType::Word)
	{
		// Word索引

		ret = createLeafNodeForWord(cManager_,
									cTerm_,
									cLang_,
									eMatchMode_);
	}
	else
	{
		// DUAL索引またはN-gram索引
		
		if (m_bNormalized == false && eMatchMode_ != MatchMode::ExactWord)
		{
			// 正規化もしないし、完全単語単位検索でもないので、
			// UNAを利用することもない

			ret = createOneLeafNode(cManager_, cTerm_);

			if (eMatchMode_ != MatchMode::String)
			{
				// 文字列検索ではないので、単語境界のイテレータを得る

				ListIterator* sep = 0;
				if (cManager_.reset(ModUnicodeString(),
									ListManager::AccessMode::Search) == true)
					sep = cManager_.getIterator();
				else
					sep = new DummyListIterator;	// 空のイテレータ

				// 単語境界一致用のノードに変更する
			
				WordLeafNode* word = new WordLeafNode(ret, sep);
			
				if (eMatchMode_ == MatchMode::SimpleWord)
				{
					word->pushWordPosition(0);
					word->pushWordPosition(cTerm_.getLength());
				}
				else if (eMatchMode_ == MatchMode::WordHead)
				{
					word->pushWordPosition(0);
				}
				else if (eMatchMode_ == MatchMode::WordTail)
				{
					word->pushWordPosition(cTerm_.getLength());
				}

				ret = word;
			}
		}
		else
		{
			// UNAを利用する必要がある

			ret = createLeafNodeForDual(cManager_,
										cTerm_,
										cLang_,
										eMatchMode_);
		}
	}

	return ret;
}

//
//	FUNCTION protected
//	FullText2::DualTokenizer::normalize -- 正規化する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cTarget_
//		対象文字列
//	const ModLanguageSet& cLang_
//		対象文字列の言語
//	ModSize uiStartPosition_
//		対象文字列の先頭位置
//	FullText2::SmartLocationListMap& cResult_
//		トークナイズ結果
//			WORD索引の場合はすべての結果
//			DUAL索引の場合は単語境界のみ
//			NGRAM索引の場合は空っぽ
//	ModSize& uiSize_
//		正規化後のサイズ
//	ModSize& uiOriginalSize_
//		正規化前のサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DualTokenizer::normalize(const ModUnicodeString& cTarget_,
						 const ModLanguageSet& cLang_,
						 ModSize uiStartPosition_,
						 SmartLocationListMap& cResult_,
						 ModSize& uiSize_,
						 ModSize& uiOriginalSize_)
{
	// DUAL索引なら、空文字列のエントリをまず加える
	if (m_eIndexingType == IndexingType::Dual)
	{
		ModUnicodeString empty;
		SmartLocationListMap::Iterator i = cResult_.find(empty);
		if (i == cResult_.end())
		{
			// 見つからないので、新規に挿入
			SmartLocationList tmp(m_cFile.getLocationCoder(empty),
								  0, m_bNoLocation);
			ModPair<SmartLocationListMap::Iterator, ModBoolean> r
				= cResult_.insert(empty, tmp);

			// コピーを1回減らすため、挿入してから位置を追加する
			(*r.first).second.pushBack(1);
		}
	}

	// UNAを利用して正規化する

	if (m_uiFeatureSize != 0)
	{
		// 特徴語取得用

		normalizeFeature(cTarget_, cLang_, uiStartPosition_,
						 cResult_, uiSize_, uiOriginalSize_);
	}
	else if (m_bNormalized || m_eIndexingType != IndexingType::Ngram)
	{
		// 通常用

		normalizeNormal(cTarget_, cLang_, uiStartPosition_,
						cResult_, uiSize_, uiOriginalSize_);
	}
	else
	{
		// 正規化もしないし、単語切り出さないので、UNAを使う必要はない

		m_cstrNormalized += cTarget_;
		uiOriginalSize_ = cTarget_.getLength();
		uiSize_ = uiOriginalSize_;
	}
}

//
//	FUNCTION protected
//	FullText2::DualTokenizer::tokenize -- トークナイズする
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SmartLocationListMap& cResult_
//		トークナイズ結果
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DualTokenizer::tokenize(SmartLocationListMap& cResult_)
{
	// 単語単位索引以外の場合には NgramTokeinzer のトークナイズを実行する
	// 単語単位索引の場合には normalize でトークナイズは終了しているので、
	// なにもしない
	
	if (m_eIndexingType != IndexingType::Word)
	{
		// 単語単位索引以外
		
		NgramTokenizer::tokenize(cResult_);
	}
}

//
//	FUNCTION protected
//	FullText2::DualTokenizer::initialize -- 初期化する
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
void
DualTokenizer::initialize()
{
	NgramTokenizer::initialize();
	m_mapFeatureCandidate.erase(m_mapFeatureCandidate.begin(),
								m_mapFeatureCandidate.end());
}

//
//	FUNCTION private
//	FullText2::DualTokenizer::normalizeFeature -- 正規化する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cTarget_
//		対象文字列
//	const ModLanguageSet& cLang_
//		対象文字列の言語
//	ModSize uiStartPosition_
//		対象文字列の先頭位置
//	FullText2::SmartLocationListMap& cResult_
//		トークナイズ結果
//	ModSize& uiSize_
//		正規化後のサイズ
//	ModSize& uiOriginalSize_
//		正規化前のサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DualTokenizer::normalizeFeature(const ModUnicodeString& cTarget_,
								const ModLanguageSet& cLang_,
								ModSize uiStartPosition_,
								SmartLocationListMap& cResult_,
								ModSize& uiSize_,
								ModSize& uiOriginalSize_)
{
	// 位置
	ModSize uiPosition = 0;

	// 正規化前のサイズ
	uiOriginalSize_ = cTarget_.getLength();

	if (m_eIndexingType != IndexingType::Word)
	{
		// 正規化文字列を取っておくので、領域を確保する
		m_cstrNormalized.reallocate(m_cstrNormalized.getLength() +
									uiOriginalSize_);
	}

	// 対象文字列をUNAにセットする
	m_pAnalyzer->set(cTarget_, cLang_);
	
 	ModVector<ModUnicodeString> formVector;	// 形態素語形配列
	ModVector<ModUnicodeString> ostrVector;	// 形態素語形配列(元表記)
	ModVector<int>				posVector;  // 形態素品詞配列
	ModVector<int>				costVector;	// 単語コスト配列
	ModVector<int>				uposVector;	// UNA統合品詞配列

	while (m_pAnalyzer->getBlock(formVector, ostrVector,
								 posVector, costVector, uposVector) == ModTrue)
	{
		// getBlock はある解析単位の解析結果がまとめて配列で返される
		// 検索語のように展開されるわけではない
		
		ModVector<ModUnicodeString>::Iterator form = formVector.begin();
		ModVector<int>::Iterator pos  = posVector.begin();
		ModVector<int>::Iterator cost = costVector.begin();
		ModVector<int>::Iterator upos = uposVector.begin();

		// 形態素がある限り処理を続ける
		for (; form != formVector.end(); ++form, ++pos, ++cost, ++upos)
		{
			if ((*form).getLength() == 0)
				continue;
			
			// 形態素を特徴語候補に設定
			setFeatureCandidate(*form, *upos, *cost, *pos);

			if (m_eIndexingType == IndexingType::Word)
			{
				// WORD索引なので、正規化時にトークナイズする

				SmartLocationListMap::Iterator i = cResult_.find(*form);
				if (i == cResult_.end())
				{
					// 見つからないので、新規に挿入
					SmartLocationList tmp(m_cFile.getLocationCoder(*form),
										  (*form).getLength(),
										  m_bNoLocation);
					ModPair<SmartLocationListMap::Iterator, ModBoolean> r
						= cResult_.insert(*form, tmp);

					// コピーを1回減らすため、挿入してから位置を追加する
					(*r.first).second.pushBack(uiPosition +
											   uiStartPosition_ + 1);
				}
				else
				{
					// 見つかったので、位置を追加
					(*i).second.pushBack(uiPosition +
										 uiStartPosition_ + 1);
				}

				++uiPosition;
			}
			else
			{
				// 正規化後の文字列
				m_cstrNormalized += (*form);
				// 正規化後のサイズ
				uiPosition += (*form).getLength();
				
				if (m_eIndexingType == IndexingType::Dual)
				{
					// DUAL索引なので、ここでは単語境界の索引単位の位置を
					// 登録する

					ModUnicodeString empty;
					SmartLocationListMap::Iterator i = cResult_.find(empty);
					; _SYDNEY_ASSERT(i != cResult_.end());

					// 単語の後ろの境界を登録
					(*i).second.pushBack(uiPosition + uiStartPosition_ + 1);
				}
			}
		}
	}

	// 正規化後のサイズを設定する
	uiSize_ = uiPosition;
}

//
//	FUNCTION private
//	FullText2::DualTokenizer::normalizeNormal -- 正規化する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cTarget_
//		対象文字列
//	const ModLanguageSet& cLang_
//		対象文字列の言語
//	ModSize uiStartPosition_
//		対象文字列の先頭位置
//	FullText2::SmartLocationListMap& cResult_
//		トークナイズ結果
//	ModSize& uiSize_
//		正規化後のサイズ
//	ModSize& uiOriginalSize_
//		正規化前のサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DualTokenizer::normalizeNormal(const ModUnicodeString& cTarget_,
							   const ModLanguageSet& cLang_,
							   ModSize uiStartPosition_,
							   SmartLocationListMap& cResult_,
							   ModSize& uiSize_,
							   ModSize& uiOriginalSize_)
{
	// 位置
	ModSize uiPosition = 0;

	// 正規化前のサイズ
	uiOriginalSize_ = cTarget_.getLength();

	if (m_eIndexingType != IndexingType::Word)
	{
		// 正規化文字列を取っておくので、領域を確保する
		m_cstrNormalized.reallocate(m_cstrNormalized.getLength() +
									uiOriginalSize_);
	}

	// 対象文字列をUNAにセットする
	m_pAnalyzer->set(cTarget_, cLang_);

	ModUnicodeString	morph;	// 正規化後の形態素

	while (m_pAnalyzer->getWord(morph) == ModTrue)
	{
		// 形態素がある限り処理を続ける

		if (morph.getLength() == 0)
			continue;
		
		if (m_eIndexingType == IndexingType::Word)
		{
			// WORD索引なので、正規化時にトークナイズする

			SmartLocationListMap::Iterator i = cResult_.find(morph);
			if (i == cResult_.end())
			{
				// 見つからないので、新規に挿入
				SmartLocationList tmp(m_cFile.getLocationCoder(morph),
									  morph.getLength(),
									  m_bNoLocation);
				ModPair<SmartLocationListMap::Iterator, ModBoolean> r
					= cResult_.insert(morph, tmp);

				// コピーを1回減らすため、挿入してから位置を追加する
				(*r.first).second.pushBack(uiPosition +
										   uiStartPosition_ + 1);
			}
			else
			{
				// 見つかったので、位置を追加
				(*i).second.pushBack(uiPosition +
									 uiStartPosition_ + 1);
			}

			++uiPosition;
		}
		else
		{
			// 正規化後の文字列
			m_cstrNormalized += morph;
			// 正規化後のサイズ
			uiPosition += morph.getLength();
				
			if (m_eIndexingType == IndexingType::Dual)
			{
				// DUAL索引なので、ここでは単語境界の索引単位の位置を
				// 登録する

				ModUnicodeString empty;
				SmartLocationListMap::Iterator i = cResult_.find(empty);
				; _SYDNEY_ASSERT(i != cResult_.end());

				// 単語の後ろの境界を登録
				(*i).second.pushBack(uiPosition + uiStartPosition_ + 1);
			}
		}
	}

	// 正規化後のサイズを設定する
	uiSize_ = uiPosition;
}

//
//	FUNCTION private
//	FullText2::DualTokenizer::createLeafNodeForDual
//		-- 検索用のLeafNodeを作成する - DUAL索引用
//
//	NOTES
//	形態素 = 単語
//
//	ARGUMETNS
//	FullText2::ListManager& cManager_
//		リストマネージャー
//	const ModUnicodeString& cTerm_
//		検索キーワード
//	const ModLanguageSet& cLang_
//		言語
//	FullText2::MatchMode::Value eMatchMode_
//		一致モード
//
//	RETURN
//	FullText2::LeafNode*
//	   	検索ノード
//
//	EXCEPTIONS
//
LeafNode*
DualTokenizer::createLeafNodeForDual(ListManager& cManager_,
									 const ModUnicodeString& cTerm_,
									 const ModLanguageSet& cLang_,
									 MatchMode::Value eMatchMode_)
{
	// 対象文字列をUNAにセットする
	m_pAnalyzer->set(cTerm_, cLang_);

	ModVector<ModUnicodeString> expanded;

	ModSize location = 0;
	OrderedDistanceLeafNode* ordered = 0;

	// 単語境界をあらわす転置リストを得る
	ModAutoPointer<ListIterator> sep;
	if (eMatchMode_ != MatchMode::String)
	{
		if (cManager_.reset(ModUnicodeString(),
							ListManager::AccessMode::Search) == true)
			sep = cManager_.getIterator();
		else
			sep = new DummyListIterator;	// 空のイテレータ
	}

	// 展開された検索文字列を格納するための配列
	ModVector<ModUnicodeString> vecToken;
	// 展開された検索文字列ごとの単語境界
	ModVector<ModVector<ModSize> > vecBoundary;

	// 展開された単語は展開されて追加される
	// しかし、あまり多くなるとメモリ確保に失敗する場合もあるので、
	// 多くなりすぎた場合には、OrderedDistaceLeafNode でつなぐ
	//
	// パターンとしては以下のものがある
	// 1. NormalLeafNode -- 展開がない場合
	// 2. AtomicOrLeafNode -- 展開が多すぎない場合
	// 3. OrderedDistanceLeafNode -- 展開が多すぎる場合
	//
	// ※ NormalLeafNode のところは WordLeafNode、SimpleLeafNode、
	//    ShortLeafNodeCompatible の場合もある

	bool empty = true;
	
	while (getExpandWords(m_pAnalyzer, m_bNormalized, expanded) == ModTrue)
	{
		if (expanded.getSize() == 0 || (*expanded.begin()).getLength() == 0)
			// 展開されて、先頭が空文字列だけど、二番目に何かあるってことは
			// ないよね？
			continue;

		// 展開された単語を追加する
		if (appendExpandedString(expanded, vecToken, vecBoundary) == false)
		{
			// 追加できなかったので、OrderedDistanceLeafNode にする

			if (ordered == 0)
			{
				ordered = new OrderedDistanceLeafNode();
			}
			
			AtomicOrLeafNode* atomicor = new AtomicOrLeafNode();
			atomicor->reserve(vecToken.getSize());

			ModVector<ModUnicodeString>::Iterator
				i = vecToken.begin();
			ModVector<ModVector<ModSize> >::Iterator
				j = vecBoundary.begin();
			for (; i < vecToken.end(); ++i, ++j)
			{
				// 検索ノードを作成する
				LeafNode* leaf = createOneLeafNode(cManager_, *i);
				
				// 単語単位検索だったら、WordLeafNode にする
				if (eMatchMode_ == MatchMode::ExactWord)
				{
					WordLeafNode* w
						= new WordLeafNode(leaf, sep->copy());
					w->setWordPosition(*j);
					leaf = w;
				}
				else if (eMatchMode_ == MatchMode::SimpleWord)
				{
					WordLeafNode* w
						= new WordLeafNode(leaf, sep->copy());
					w->pushWordPosition(0);
					eMatchMode_ = MatchMode::WordTail;
					leaf = w;
				}
				else if (eMatchMode_ == MatchMode::WordHead)
				{
					WordLeafNode* w
						= new WordLeafNode(leaf, sep->copy());
					w->pushWordPosition(0);
					eMatchMode_ = MatchMode::String;
					leaf = w;
				}
				
				atomicor->pushBack(leaf);
			}

			ordered->pushBack(atomicor);

			vecToken.clear();
			vecBoundary.clear();
			// あらためて追加する
			appendExpandedString(expanded, vecToken, vecBoundary);
		}
	}

	// 最後のトークンを処理する

	AtomicOrLeafNode* atomicor = 0;
	LeafNode* leaf = 0;

	ModVector<ModUnicodeString>::Iterator
		i = vecToken.begin();
	ModVector<ModVector<ModSize> >::Iterator
		j = vecBoundary.begin();
	for (; i < vecToken.end(); ++i, ++j)
	{
		// 他の条件を含んでいる条件なら読み飛ばす
		if (ordered == 0 && isContains(vecToken, i, eMatchMode_))
			continue;
		
		// 検索ノードを作成する
		LeafNode* tmp = createOneLeafNode(cManager_, *i);

		// 単語単位検索だったら、WordLeafNode にする
		if (eMatchMode_ != MatchMode::String)
		{
			WordLeafNode* w = new WordLeafNode(tmp, sep->copy());
			tmp = w;
			
			if (eMatchMode_ == MatchMode::ExactWord)
			{
				w->setWordPosition(*j);
			}
			else if (eMatchMode_ == MatchMode::SimpleWord)
			{
				w->pushWordPosition(0);
				w->pushWordPosition((*i).getLength());
			}
			else if (eMatchMode_ == MatchMode::WordHead)
			{
				w->pushWordPosition(0);
			}
			else if (eMatchMode_ == MatchMode::WordTail)
			{
				w->pushWordPosition((*i).getLength());
			}
		}

		if (leaf == 0)
		{
			leaf = tmp;
		}
		else if (atomicor == 0)
		{
			// 複数ノードなので、AtomicOrにする
			
			atomicor = new AtomicOrLeafNode();
			atomicor->reserve(vecToken.getSize());
			atomicor->pushBack(leaf);
			leaf = atomicor;
		}
			
		if (atomicor) atomicor->pushBack(tmp);
	}

	LeafNode* ret = 0;

	if (ordered)
	{
		ordered->pushBack(leaf);
		ret = ordered;
	}
	else
	{
		ret = leaf;
	}
	
	return ret;
}

//
//	FUNCTION private
//	FullText2::DualTokenizer::createLeafNodeForWord
//		-- 検索用のLeafNodeを作成する - WORD索引用
//
//	NOTES
//	形態素 = 単語
//
//	ARGUMETNS
//	FullText2::ListManager& cManager_
//		リストマネージャー
//	const ModUnicodeString& cTerm_
//		検索キーワード
//	const ModLanguageSet& cLang_
//		言語
//	FullText2::MatchMode::Value eMatchMode_
//		一致モード
//
//	RETURN
//	FullText2::LeafNode*
//	   	検索ノード
//
//	EXCEPTIONS
//
LeafNode*
DualTokenizer::createLeafNodeForWord(ListManager& cManager_,
									 const ModUnicodeString& cTerm_,
									 const ModLanguageSet& cLang_,
									 MatchMode::Value eMatchMode_)
{
	// 対象文字列をUNAにセットする
	m_pAnalyzer->set(cTerm_, cLang_);

	ModVector<ModUnicodeString> expanded;

	ModSize location = 0;
	LeafNode* ret = 0;
	OrderedDistanceLeafNode* ordered = 0;

	// 展開された単語を格納するための配列
	ModVector<LeafNode::LocationVector> vecToken;

	// 展開された単語は展開されて追加される
	// しかし、あまり多くなるとメモリ確保に失敗する場合もあるので、
	// 多くなりすぎた場合には、OrderedDistaceLeafNode でつなぐ
	//
	// パターンとしては以下のものがある
	// 1. NormalLeafNode -- WordHeadでもなく、展開がない場合
	// 2. ShortLeafNode -- WordHeadでかつ、形態素が１つでかつ、展開がない場合
	// 3. OrderedDistanceLeafNode -- WordHeadでかつ、形態素が複数でかつ、
	//								 展開がない場合
	// 4. AtomicOrLeafNode -- 展開が多すぎない場合
	//						  要素としては上の1,2,3がある
	// 5. OrderedDistanceLeafNode -- 展開が多すぎる場合
	//								 要素としては上の1,2,3,4がある
	//
	// ※ NormalLeafNode のところは SimpleLeafNode の場合もある
	
	while (getExpandWords(m_pAnalyzer, m_bNormalized, expanded) == ModTrue)
	{
		if (expanded.getSize() == 0 || (*expanded.begin()).getLength() == 0)
			// 展開されて、先頭が空文字列だけど、二番目に何かあるってことは
			// ないよね？
			continue;

		// 展開された単語を追加する
		if (appendExpandedWord(expanded, vecToken) == false)
		{
			// 追加できなかったので、OrderedDistanceLeafNode にする

			if (ordered == 0)
			{
				ordered = new OrderedDistanceLeafNode();
			}
			
			AtomicOrLeafNode* atomicor = new AtomicOrLeafNode();
			atomicor->reserve(vecToken.getSize());

			ModVector<LeafNode::LocationVector>::Iterator
				i = vecToken.begin();
			for (; i < vecToken.end(); ++i)
			{
				LeafNode* leaf
					= LeafNode::createNormalLeafNode(cManager_, *i);
				atomicor->pushBack(leaf);
			}

			ordered->pushBack(atomicor);

			vecToken.clear();
			// あらためて追加する
			appendExpandedWord(expanded, vecToken);
		}
	}

	// 最後のトークンを処理する

	AtomicOrLeafNode* atomicor = 0;
	LeafNode* leaf = 0;

	if (vecToken.getSize() > 1)
	{
		// 直前のトークンは展開されていたので、
		// AtomicOrLeafNode にする
				
		atomicor = new AtomicOrLeafNode();
		atomicor->reserve(vecToken.getSize());
	}

	ModVector<LeafNode::LocationVector>::Iterator
		i = vecToken.begin();
	for (; i < vecToken.end(); ++i)
	{
		LeafNode* shortWord = 0;
		LeafNode* node = 0;
		OrderedDistanceLeafNode* ordered2 = 0;
		
		if (eMatchMode_ == MatchMode::WordHead)
		{
			// 単語前方一致なので、最後のトークンは
			// ショートワードにする必要がある

			LeafNode::LocationVector::Iterator j = (*i).end();
			--j;

			shortWord = LeafNode::createShortLeafNode(cManager_,
													  (*j).second,
													  1,
													  0,
													  false);

			// 最後の要素を削除する
			(*i).popBack();
		}

		if ((*i).getSize())
		{
			// 普通の LeafNode にする
				
			node = LeafNode::createNormalLeafNode(cManager_,
												  *i);
		}

		if (shortWord != 0 && node != 0)
		{
			// 単語前方一致なので、通常のノードとショートワードのノードを
			// 連接ノードで結合する
			
			ordered2 = new OrderedDistanceLeafNode();

			ordered2->pushBack(node);
			ordered2->pushBack(shortWord);
		}

		if (ordered2)
			leaf = ordered2;
		else if (node)
			leaf = node;
		else
			leaf = shortWord;

		if (atomicor) atomicor->pushBack(leaf);
	}

	if (ordered)
	{
		if (atomicor)
			ordered->pushBack(atomicor);
		else
			ordered->pushBack(leaf);
		ret = ordered;
	}
	else if (atomicor)
	{
		ret = atomicor;
	}
	else
	{
		ret = leaf;
	}
	
	return ret;
}

//
//	FUNCTION private
//	FullText2::DualTokenizer::setFeatureCandidate
//		-- 特徴語候補を設定する
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
DualTokenizer::setFeatureCandidate(const ModUnicodeString& cstrMorpheme_,
								   int iUnifiedPOS_,
								   int iCost_,
								   int iPOS_)
{
	// 名詞が特徴語候補
	// ただし、日本語中の英語が UNA_UNKNOWN になるので、それも考慮する。
	
	if (iUnifiedPOS_ & UNA_NOUN || iUnifiedPOS_ == UNA_UNKNOWN)
	{
		ModSize uiMorphemeSize = cstrMorpheme_.getLength();

		Utility::ModTermType iTermType = TERM_ALPHABET;
		if (m_pTermResource->termTypeTable != 0)
		{
			iTermType = (*m_pTermResource->termTypeTable)[iPOS_];
		}
		
		// [NOTE] 変換テーブルがない場合、全ての未定義語を英文字列とみなす。
		
		if (uiMorphemeSize > 1 &&
			(iUnifiedPOS_ & UNA_NOUN || iTermType == TERM_ALPHABET))
		{
			// 2文字以上、かつ、名詞または英文字、の場合
			
			FeatureMap::Iterator i =
				m_mapFeatureCandidate.find(cstrMorpheme_);
			if (i == m_mapFeatureCandidate.end())
			{
				// 初めてなので、1のTFとコストを登録
				
				// TF項とのバランスを考えてコストを補正
				int iCost = 0;
				if (iUnifiedPOS_ & UNA_NOUN)
				{
					iCost = ModMin(_MaxOccurrenceCost.get(), iCost_);
				}
				else
				{
					// 英文字列の場合
					// 文字列長に依存
					iCost = static_cast<int>(_AlphabetOccurrenceCostFactor.get()
											 * LOG(uiMorphemeSize));
				}
				
				// 登録
				m_mapFeatureCandidate.insert(cstrMorpheme_,
											 FeatureCandidateElement(1, iCost));
			}
			else
			{
				// TFをインクリメント
				++((*i).second.first);
			}
		}
	}
}

//
//	FUNCTION privae
//	FullText2::DualTokenizer::appendExpandedString
//	   	-- 展開された形態素を追加する -- DUAL索引用
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<ModUnicodeString>& expaned_
//		展開された形態素
//	ModVector<ModUnicodeString>& vecToken_
//		検索文字列
//	ModVector<ModVector<ModSize> >& vecBoundary_
//		単語境界のある位置
//
//	RETURN
//	bool
//		追加できた場合は true 、展開の上限に達した場合は false
//
//	EXCEPTIONS
//
bool
DualTokenizer::
appendExpandedString(const ModVector<ModUnicodeString>& expanded_,
					 ModVector<ModUnicodeString>& vecToken_,
					 ModVector<ModVector<ModSize> >& vecBoundary_)
{
	if (expanded_.getSize() * vecToken_.getSize()
		> static_cast<ModSize>(_ExpandLimit.get()))
		// 上限を超えるので false を返す
		return false;

	if (vecToken_.getSize() == 0)
		vecToken_.pushBack(ModUnicodeString());
	if (vecBoundary_.getSize() == 0)
	{
		vecBoundary_.pushBack(ModVector<ModSize>());
		(*vecBoundary_.begin()).pushBack(0);
	}

	int n = expanded_.getSize();
	int org = vecToken_.getSize();
	
	// reserve しているので、自分自身の要素をコピーしても問題ない
	
	vecToken_.reserve(vecToken_.getSize() * n);
	ModVector<ModUnicodeString>::Iterator b = vecToken_.begin();
	ModVector<ModUnicodeString>::Iterator e = vecToken_.end();
	
	vecBoundary_.reserve(vecBoundary_.getSize() * n);
	ModVector<ModVector<ModSize> >::Iterator bb
		= vecBoundary_.begin();
	ModVector<ModVector<ModSize> >::Iterator ee
		= vecBoundary_.end();
	
	for (int k = 1; k < n; ++k)
	{
		vecToken_.insert(vecToken_.end(), b, e);
		vecBoundary_.insert(vecBoundary_.end(), bb, ee);
	}

	// expanded_ の内容をそれぞれの要素に追加する
	
	ModVector<ModUnicodeString>::ConstIterator i = expanded_.begin();
	ModVector<ModUnicodeString>::ConstIterator ie = expanded_.end();
	for (; i != ie; ++i)
	{
		ModSize len = (*i).getLength();
		for (int j = 0; j < org; ++j, ++b, ++bb)
		{
			(*b).append(*i);
			ModVector<ModSize>::Iterator t = (*bb).end();
			--t;	// 必ず先頭の１つは登録されている
			(*bb).pushBack(*t + len);
		}
	}

	return true;
}

//
//	FUNCTION privae
//	FullText2::DualTokenizer::appendExpandedWord
//	   	-- 展開された形態素を追加する -- WORD索引用
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<ModUnicodeString>& expaned_
//		展開された形態素
//	ModVector<LeafNode::LocationVector>& vecToken_
//		トークン
//
//	RETURN
//	bool
//		追加できた場合は true 、展開の上限に達した場合は false
//
//	EXCEPTIONS
//
bool
DualTokenizer::
appendExpandedWord(const ModVector<ModUnicodeString>& expanded_,
				   ModVector<LeafNode::LocationVector>& vecToken_)
{
	if (expanded_.getSize() * vecToken_.getSize() >
		static_cast<ModSize>(_ExpandLimit.get()))
		// 上限を超えるので false を返す
		return false;

	if (vecToken_.getSize() == 0)
		vecToken_.pushBack(LeafNode::LocationVector());

	int n = expanded_.getSize();
	int org = vecToken_.getSize();
	
	// reserve しているので、自分自身の要素をコピーしても問題ない
	
	vecToken_.reserve(vecToken_.getSize() * n);
	ModVector<LeafNode::LocationVector>::Iterator
		b = vecToken_.begin();
	ModVector<LeafNode::LocationVector>::Iterator
		e = vecToken_.end();
	
	for (int k = 1; k < n; ++k)
	{
		vecToken_.insert(vecToken_.end(), b, e);
	}

	// expanded_ の内容をそれぞれの要素に追加する

	ModVector<ModUnicodeString>::ConstIterator i = expanded_.begin();
	ModVector<ModUnicodeString>::ConstIterator ie = expanded_.end();
	ModSize loc = (*b).getSize();
	for (; i != ie; ++i)
	{
		for (int j = 0; j < org; ++j, ++b)
		{
			(*b).pushBack(LeafNode::LocationPair(loc, *i));
		}
	}

	return true;
}

//
//	FUNCTION private
//	FullText2::DualTokenizer::getExpandWords -- 展開語を取得する
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<ModUnicodeString>& vecExpanded_
//		展開語
//
//	RETURN
//	ModBoolean
//		結果が存在する場合はModTrue、それ以外の場合はModFalse
//
//	EXCEPTIONS
//
ModBoolean
DualTokenizer::getExpandWords(UNA::ModNlpAnalyzer* pAnalyzer_,
							  bool bNormalized_,
							  ModVector<ModUnicodeString>& vecExpanded_)
{
	ModBoolean r;
	ModUnicodeString dummy1;
	int dummy2;
	
	if (bNormalized_)
	{
		r = pAnalyzer_->getExpandWords(vecExpanded_, dummy1, dummy2);
	}
	else
	{
		ModUnicodeString token;
		vecExpanded_.clear();
		r = pAnalyzer_->getWord(token);
		if (r == ModTrue)
			vecExpanded_.pushBack(token);
	}

	return r;
}

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
