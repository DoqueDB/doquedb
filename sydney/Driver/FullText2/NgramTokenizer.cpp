// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NgramTokenizer.cpp
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
#include "FullText2/NgramTokenizer.h"

#include "FullText2/AtomicOrLeafNode.h"
#include "FullText2/Blocker.h"
#include "FullText2/FullTextFile.h"
#include "FullText2/LeafNode.h"
#include "FullText2/NormalShortLeafNode.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "ModNLP.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::NgramTokenizer::NgramTokenizer -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::FullTextFile& cFile_
//	   	全文索引ファイル
//	UNA::ModNlpAnalyzer* pAnalyzer_
//		UNAアナライザー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
NgramTokenizer::NgramTokenizer(FullTextFile& cFile_,
							   UNA::ModNlpAnalyzer* pAnalyzer_,
							   const ModUnicodeChar* pParameter_)
	: Tokenizer(cFile_, pAnalyzer_)
{
	if (pParameter_) setBlocker(pParameter_);
}

//
//	FUNCTION public
//	FullText2::NgramTokenizer::~NgramTokenizer -- デストラクタ
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
NgramTokenizer::~NgramTokenizer()
{
}

//
//	FUNCTION protected
//	FullText2::NgramTokenizer::createLeafNodeImpl
//		-- 検索用のノードを作成する
//
//	NOTES
//
//	ARGUMENTS
//	FulLText2::ListManager& cManager_
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
NgramTokenizer::createLeafNodeImpl(ListManager& cManager_,
								   const ModUnicodeString& cTerm_,
								   const ModLanguageSet& cLang_,
								   MatchMode::Value eMatchMode_)
{
	// サポートしている一致モードは String のみ
	if (eMatchMode_ != MatchMode::String)
		_TRMEISTER_THROW0(Exception::NotSupported);

	ModVector<ModUnicodeString> vecResult;
	
	if (m_bNormalized)
	{
		// 正規化する
		
		// 検索文字列をUNAにセットする
		m_pAnalyzer->set(cTerm_, cLang_);

		// 検索なので、展開して取得する
		m_pAnalyzer->getExpandBuf(vecResult);
	}
	else
	{
		// 正規化しないので、そのままコピーする
		
		vecResult.pushBack(cTerm_);
	}

	
	// ノードにする

	LeafNode* root = 0;
	AtomicOrLeafNode* atomic = 0;

	ModVector<ModUnicodeString>::Iterator i = vecResult.begin();
	for (; i < vecResult.end(); ++i)
	{
		// 他の条件を含んでいる条件なら読み飛ばす
		if (isContains(vecResult, i, MatchMode::String))
			continue;
		
		// LeafNodeを作成する
		LeafNode* tmp = createOneLeafNode(cManager_, *i);

		if (root == 0)
		{
			root = tmp;
		}
		else if (atomic == 0)
		{
			// 複数ノードなので、AtomicOrにする
			
			atomic = new AtomicOrLeafNode();
			atomic->reserve(vecResult.getSize());
			atomic->pushBack(root);
			root = atomic;
		}
		
		if (atomic != 0)
		{
			atomic->pushBack(tmp);
		}
	}

	return root;
}

//
//	FUNCTION protected
//	FullText2::NgramTokenizer::normalize -- 正規化する
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
//		トークナイズ結果(空っぽ)
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
NgramTokenizer::normalize(const ModUnicodeString& cTarget_,
						  const ModLanguageSet& cLang_,
						  ModSize uiStartPosition_,
						  SmartLocationListMap& cResult_,
						  ModSize& uiSize_,
						  ModSize& uiOriginalSize_)
{
	// 正規化前のサイズは cTarget の文字列長に等しい
	uiOriginalSize_ = cTarget_.getLength();
	
	if (m_bNormalized)
	{
		uiSize_ = 0;
		
		// 正規化する

		// 対象文字列をUNAにセットする
		m_pAnalyzer->set(cTarget_, cLang_);

		ModUnicodeString	result;	// 正規化後の文字列

		// 正規化後の文字列を得る
		while (m_pAnalyzer->getNormalizeBuf(result) == ModTrue)
		{
			// 結果がある限り処理を続ける

			if (result.getLength() == 0)
				continue;
		
			// 正規化後の文字列
			m_cstrNormalized += result;
			// 正規化後のサイズ
			uiSize_ += result.getLength();
		}
	}
	else
	{
		// 正規化しないのでそのまま追加

		m_cstrNormalized += cTarget_;
		uiSize_ = uiOriginalSize_;
	}
}

//
//	FUNCTION protected
//	FullText2::NgramTokenizer::tokenize -- トークナイズする
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
NgramTokenizer::tokenize(SmartLocationListMap& cResult_)
{
	// ターゲットの文字列を設定する
	m_pBlocker->set(m_cstrNormalized, false);

	// トークンを取得する
	ModUnicodeString token;
	ModSize offset;
	bool dummy;	// 登録なので、ショートにはならない
	ModSize dummy2;
	
	while (m_pBlocker->yield(token, offset, dummy, dummy2) == true)
	{
		// 検索してから挿入すると、二回検索することになるので、
		// 検索せずに挿入する

		ModPair<SmartLocationListMap::Iterator, ModBoolean> r
			= cResult_.insert(token, SmartLocationList());

		if (r.second == ModTrue)
		{
			// 初めての挿入だったので、ちゃんとした SmartLocationList で上書き
			(*r.first).second
				= SmartLocationList(m_cFile.getLocationCoder(token),
									token.getLength(),
									m_bNoLocation);
		}

		// 位置情報を追加する
		(*r.first).second.pushBack(offset + 1);	// 1 origin 
	}
}

//
//	FUNCTION protected
//	FullText2::NgramTokenizer::initialize -- 初期化する
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
NgramTokenizer::initialize()
{
	// ターゲットの文字列をクリアする
	m_cstrNormalized.clear();
}

//
//	FUNCTION protected
//	FullText2::NgramTokenizer::createOneLeafNode
//		-- 検索用のノードを１つ作成する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::ListManager& cManager_
//		リストマネージャー
//	const ModUnicodeString& cTerm_
//		検索キーワード
//
//	RETURN
//	FullText2::LeafNode*
//		検索ノード
//
//	EXCEPTIONS
//
LeafNode*
NgramTokenizer::createOneLeafNode(ListManager& cManager_,
								  const ModUnicodeString& cTerm_)
{
	// ターゲットの文字列を設定する
	m_pBlocker->set(cTerm_, true);	// 検索用

	// トークンを取得する
	ModUnicodeString token;
	ModSize offset;
	bool isShort = false;
	ModSize minLength = 0;

	LeafNode::LocationVector cToken;
	cToken.reserve(cTerm_.getLength());

	while (m_pBlocker->yield(token, offset, isShort, minLength) == true)
	{
		if (isShort == true)
			// ショートになるのは最後だけなので
			break;

		cToken.pushBack(LeafNode::LocationPair(offset, token));
	}

	// LeafNodeを作成する
	LeafNode* ret = 0;
		
	if (isShort == true)
	{
		// ショート
		int len = static_cast<int>(token.getLength());
		ret = LeafNode::createShortLeafNode(cManager_,
											token,
											len,
											minLength,
											m_pBlocker->isPrefixProcessing());
	}

	if (cToken.getSize())
	{
		// 通常
		LeafNode* normal = LeafNode::createNormalLeafNode(cManager_,
														  cToken);

		if (ret)
		{
			// ショートもあったので、NormalShortLeafNodeにする
			NormalShortLeafNode* ns = new NormalShortLeafNode();

			ns->setNormal(normal);
			ns->setShort(offset, ret);

			ret = ns;
		}
		else
		{
			ret = normal;
		}
	}

	return ret;
}

//
//	FUNCTION protected
//	FullText2::NgramTokenizer::isContains
//	   	-- 包含している文字列が存在するかどうか
//	
//	NOTES
//
//	ARGUMENTS
//	ModVector<ModUnicodeString>& vecToken_
//		展開した文字列
//	ModVector<ModUnicodeString>::Iterator i_
//		包含している文字列が存在するかどうか確認する文字列へのイテレータ
//	FullText2::MatchMode::Value eMatchMode_
//		マッチモード(文字列検索とか、厳格単語単位検索とか)
//
//	RETURN
//	bool
//		引数 i_ を含む条件が見つかった場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
NgramTokenizer::isContains(ModVector<ModUnicodeString>& vecToken_,
						   ModVector<ModUnicodeString>::Iterator i_,
						   MatchMode::Value eMatchMode_)
{
	if (eMatchMode_ == MatchMode::ExactWord ||
		eMatchMode_ == MatchMode::SimpleWord)
		// 単語単位検索の場合は含まれていても検索する
		return false;
	
	ModVector<ModUnicodeString>::Iterator b = vecToken_.begin();
	ModVector<ModUnicodeString>::Iterator e = vecToken_.end();
	for (; b != e; ++b)
	{
		if (b == i_)
			continue;

		if ((*b).getLength() < (*i_).getLength())
		{
			ModUnicodeChar* s = (*i_).search(*b);
			if (s == 0)
				continue;
			
			// i_ の中に b が含まれている

			switch (eMatchMode_)
			{
			case MatchMode::WordHead:
				if (s == (*i_).operator const ModUnicodeChar*())
					// 先頭のポインタなので、含む
					return true;
				break;
			case MatchMode::WordTail:
				s += (*b).getLength();
				if (s == ((*i_).operator const ModUnicodeChar*()
						  + (*i_).getLength()))
					// 最後のポインタなので、含む
					return true;
				break;
			default:
				// 含む
				return true;
			}
		}
	}

	return false;
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
