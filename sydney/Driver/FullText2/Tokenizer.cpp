// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Tokenizer.cpp
// 
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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
#include "FullText2/Tokenizer.h"

#include "FullText2/DualTokenizer.h"
#include "FullText2/FullTextFile.h"
#include "FullText2/NgramTokenizer.h"
#include "FullText2/GeneralBlocker.h"
#include "FullText2/JapaneseBlocker1.h"
#include "FullText2/JapaneseBlocker2.h"
#include "FullText2/JapaneseBlocker3.h"

#include "Exception/NotSupported.h"
#include "Exception/SQLSyntaxError.h"

#include "ModUnicodeCharTrait.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//
	//	VARIABLE local -- 各トークナイザーのパラメータ名
	//	_$$::_cDual			
	//	_$$::_cBlockedNgram
	//	_$$::_cNgram
	//
	ModUnicodeString _cDual("DUAL");
	ModUnicodeString _cBlockedNgram("BNG");
	ModUnicodeString _cNgram("NGR");

	//
	//	VARIABLE local -- 各ブロック化器のパラメータ名
	//	_$$::_cJapanese2
	//	_$$::_cJapanese1
	//
	ModUnicodeString _cJapanese3("JAP3");
	ModUnicodeString _cJapanese2("JAP2");
	ModUnicodeString _cJapanese1("JAP1");
	ModUnicodeString _cJapanese("JAP");
}

//
//	FUNCTION public
//	FullText2::Tokenizer::Tokenizer -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::FullTextFile& cFile_
//		全文ファイルクラス
//	UNA::ModNlpAnalyzer* pAnalyzer_
//		UNAアナライザー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Tokenizer::Tokenizer(FullTextFile& cFile_, UNA::ModNlpAnalyzer* pAnalyzer_)
	: m_cFile(cFile_), m_pAnalyzer(pAnalyzer_), m_pBlocker(0)
{
	// 高速化のためキャッシュする
	m_eIndexingType = cFile_.getIndexingType();
	m_bNormalized = cFile_.isNormalized();
	m_bNoLocation = cFile_.isNoLocation();
}

//
//	FUNCTION public
//	FullText2::Tokenizer::~Tokenizer -- デストラクタ
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
Tokenizer::~Tokenizer()
{
	if (m_pAnalyzer) delete m_pAnalyzer;
	if (m_pBlocker) delete m_pBlocker;
}

//
//	FUNCTION public static
//	FullText2::Tokenizer::check
//		-- トークナイズパラメータをチェックし、正規化して返す
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::IndexingType::Value eType_
//		索引タイプ
//	const ModUnicodeString& cstrParameter_
//		トークナイズパラメータ
//
//	RETURN
//	ModUnicodeString
//		正規化されたトークナイズパラメータ
//
//	EXCEPTIONS
//
ModUnicodeString
Tokenizer::check(IndexingType::Value eType_,
				 const ModUnicodeString& cstrParameter_)
{
	//【注意】cstrParameter_ の英字はすべて大文字であることが前提

	ModUnicodeString r;
	const ModUnicodeChar* p = cstrParameter_;

	if (eType_ == IndexingType::Word)
	{
		// 単語単位索引の場合、選択できるトークナイザ—は DUAL のみ

		if (compare(p, _cDual))
		{
			// 単語単位索引の場合、DUALと指定されていれば、後はなんでもいい
			r.append(_cDual).append(':').append(p);
		}
		else
		{
			// 見つからない
			_TRMEISTER_THROW1(Exception::SQLSyntaxError, cstrParameter_);
		}
	}
	else
	{
		// 文字列索引の場合、選択できるトークナイザ—は DUAL, BNG, NGR
		// ただし、実装上、BNG と NGR の違いはない(ブロック化器が違うだけ)

		if (compare(p, _cDual))
		{
			r.append(_cDual).append(':').append(DualTokenizer::check(p));
		}
		else if (compare(p, _cBlockedNgram))
		{
			r.append(_cBlockedNgram)
				.append(':').append(NgramTokenizer::check(p));
		}
		else if (compare(p, _cNgram))
		{
			r.append(_cNgram).append(':').append(NgramTokenizer::check(p));
		}
		else
		{
			// 見つからない
			_TRMEISTER_THROW1(Exception::SQLSyntaxError, cstrParameter_);
		}
	}

	return r;
}

//
//	FUNCTION public static
//	FullText2::Tokenizer::check -- パラメータをチェックする
//
//	NOTES
//
//	ARGUMETNS
//	const ModUnicodeChar* pParameter_
//		パラメータ
//
//	RETURN
//	ModUnicodeStirng
//		正規化されたパラメータ
//
//	EXCEPTIONS
//
ModUnicodeString
Tokenizer::check(const ModUnicodeChar* pParameter_)
{
	ModUnicodeString r;
	ModAutoPointer<Blocker> pBlocker;
	
	if (compare(pParameter_, _cJapanese3) == true)
	{
		// 新日本語ブロック化器v3
		r.append(_cJapanese3).append(':');
		pBlocker = new JapaneseBlocker3();
	}
	else if (compare(pParameter_, _cJapanese2) == true)
	{
		// 新日本語ブロック化器v2
		r.append(_cJapanese2).append(':');
		pBlocker = new JapaneseBlocker2();
	}
	else if (compare(pParameter_, _cJapanese1) == true)
	{
		// 旧日本語ブロック化器
		// 未テストのためサポート外
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	else if (compare(pParameter_, _cJapanese) == true)
	{
		// デフォルトは新日本語ブロック化器v3
		r.append(_cJapanese3).append(':');
		pBlocker = new JapaneseBlocker3();
	}
	else
	{
		// 文字種を確認しないブロック化器
		pBlocker = new GeneralBlocker();
	}

	const ModUnicodeChar* p = pParameter_;
	pBlocker->parse(pParameter_);
	r.append(p);

	return r; 
}

//
//	FUNCTION public static
//	FullText2::Tokenizer::createTokenizer -- トークナイザーを得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::FullTextFile& cFile_
//		全文ファイルクラス
//	UNA::ModNlpAnalyzer* pAnalyzer_
//		UNAアナライザー
//	const ModUnicodeString& cstrParameter_
//		トークナイズパラメータ
//
//	RETURN
//	FullText2::Tokenizer*
//		トークナイザー。存在しない場合は 0 を返す。
//		得られたポインターのメモリーは呼び出し側で解放すること。
//
//	EXCEPTIONS
//
Tokenizer*
Tokenizer::createTokenizer(FullTextFile& cFile_,
						   UNA::ModNlpAnalyzer* pAnalyzer_,
						   const ModUnicodeString& cstrParameter_)
{
	//	FullText2::FullTextFile から呼び出されることを想定している
	
	Tokenizer* tokenizer = 0;
	const ModUnicodeChar* p = cstrParameter_;

	// 索引タイプ
	IndexingType::Value eType = cFile_.getIndexingType();
	
	// パラメータの先頭文字列から、該当するトークナイザーを確保する

	if (eType == IndexingType::Word)
	{
		// 単語単位索引の場合、選択できるトークナイザーは DUAL のみ
		
		if (compare(p, _cDual))
			// DUAL
			tokenizer = new DualTokenizer(cFile_, pAnalyzer_, p);
		else
			// 見つからない
			_TRMEISTER_THROW1(Exception::SQLSyntaxError, cstrParameter_);
	}
	else
	{
		// 文字列索引の場合、選択できるトークナイザーは DUAL, BNG, NGR
		// ただし、実装上、BNG と NGR の違いはない(ブロック化器が違うだけ)
		
		if (compare(p, _cDual))
			// DUAL
			tokenizer = new DualTokenizer(cFile_, pAnalyzer_, p);
		else if (compare(p, _cBlockedNgram))
			// BNG
			tokenizer = new NgramTokenizer(cFile_, pAnalyzer_, p);
		else if (compare(p, _cNgram))
			// NGR
			tokenizer = new NgramTokenizer(cFile_, pAnalyzer_, p);
		else
			// 見つからない
			_TRMEISTER_THROW1(Exception::SQLSyntaxError, cstrParameter_);
	}

	return tokenizer;
}

//
//	FUNCTION public
//	FullText2::Tokenizer::setFeatureParameter
//		-- 特徴語取得のための情報を付加する
//
//	NOTES
//	サポートするトークナイザーで上書きする
//
//	ARGUEMNTS
//	ModSize uiFeatureSize_
//		取得する特徴語の数
//	const Utility::ModTermResource* pTermResource_
//		特徴語を取得するために必要なTermResource
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::NotSupported
//		サポートしていない
//
void
Tokenizer::setFeatureParameter(ModSize iFeatureSize_,
							   const Utility::ModTermResource* pTermResource_)
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	FullText2::Tokenizer::tokenize -- トークナイズする
//
//	NOTES
//	通常索引用
//
//	ARGUMENTS
//	const ModUnicodeString& cTarget_
//		対象文字列
//	const ModLanguageSet& cLang_
//		対象文字列の言語
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
Tokenizer::tokenize(const ModUnicodeString& cTarget_,
					const ModLanguageSet& cLang_,
					SmartLocationListMap& cResult_,
					ModSize& uiSize_,
					ModSize& uiOriginalSize_)
{
	initialize();
	normalize(cTarget_, cLang_, 0, cResult_, uiSize_, uiOriginalSize_);
	tokenize(cResult_);
}

//
//	FUNCTION public
//	FullText2::Tokenizer::tokenize -- トークナイズする
//
//	NOTES
//	セクション検索用
//
//	ARGUMENTS
//	const ModVector<ModUnicodeString>& vecTarget_
//		対象文字列
//	const ModVector<ModLanguageSet>& vecLang_
//		対象文字列の言語
//	FullText2::SmartLocationListMap& cResult_
//		トークナイズ結果
//	ModVector<ModSize>& vecSize_
//		正規化後のサイズ
//	ModVector<ModSize>& vecOriginalSize_
//		正規化前のサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Tokenizer::tokenize(const ModVector<ModUnicodeString>& vecTarget_,
					const ModVector<ModLanguageSet>& vecLang_,
					SmartLocationListMap& cResult_,
					ModVector<ModSize>& vecSize_,
					ModVector<ModSize>& vecOriginalSize_)
{
	vecSize_.clear();
	vecSize_.reserve(vecTarget_.getSize());

	initialize();
	ModSize start = 0;
	ModVector<ModUnicodeString>::ConstIterator i = vecTarget_.begin();
	ModVector<ModLanguageSet>::ConstIterator j = vecLang_.begin();
	for (; i < vecTarget_.end(); ++i)
	{
		ModSize size = 0;
		ModSize originalSize = 0;
		normalize(*i, *j, start, cResult_, size, originalSize);
		start += size;
		
		vecSize_.pushBack(size);
		vecOriginalSize_.pushBack(originalSize);

		if (vecLang_.getSize() > 1)
			++j;
	}
	tokenize(cResult_);
}

//
//	FUNCTION public
//	FullText2::Tokenizer::getFeatureList
//		-- 特徴語リストを取得する
//
//	NOTES
//	基底クラスでは何もしない
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
Tokenizer::getFeatureList(FeatureList& vecFeature_)
{
}

//
//	FUNCTION public
//	FullText2::Tokenizer::createLeafNode -- 検索用のLeafNodeを作成する
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
//	   	検索ノード
//
//	EXCEPTIONS
//
LeafNode*
Tokenizer::createLeafNode(ListManager& cManager_,
						  const ModUnicodeString& cTerm_,
						  const ModLanguageSet& cLang_,
						  MatchMode::Value eMatchMode_)
{
	return createLeafNodeImpl(cManager_,
							  cTerm_,
							  cLang_,
							  eMatchMode_);
}

//
//	FUNCTION public static
//	FullText2::Tokenizer::compare -- target_ の先頭が src_ と一致するか比較する
//
//	NOTES
//	target_ の先頭から調べて src_ と一致し、かつ、その次の文字が
//	':' か ' ' か '\0' の場合一致しているとみなす
//	一致した場合には、target_ は一致した位置まで進められる
//
//	ARGUMENTS
//	const ModUnicodeChar*& target_
//		走査対象の文字列
//	const ModUnicodeChar* src_
//		検索文字列
//
//	RETURN
//	bool
//		一致した場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Tokenizer::compare(const ModUnicodeChar*& target_,
				   const ModUnicodeChar* src_)
{
	bool ret = false;
	
	const ModUnicodeChar* t = target_;
	const ModUnicodeChar* s = src_;

	for (; *t != 0 && *s != 0 && *t == *s; ++t, ++s);

	if (*s == 0 && (*t == ':' || *t == ' ' || *t == 0))
	{
		// 一致した
		ret = true;

		// 次の位置に進める
		if (*t == ':') ++t;
		else while (*t != 0 && *t == ' ') ++t;
		target_ = t;
	}

	return ret;
}

//
//	FUNCTION public
//	FullText2::Tokenizer::release -- 解放する
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Tokenizer::release()
{
	m_cFile.pushTokenizer(this);
}

//
//	FUNCTION protected
//	FullText2::Tokenizer::setBlocker -- ブロック化器を設定する
//
//	NOTES
//
//	ARGUMETNS
//	const ModUnicodeChar* pParameter_
//		パラメータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Tokenizer::setBlocker(const ModUnicodeChar* pParameter_)
{
	if (m_pBlocker) delete m_pBlocker, m_pBlocker = 0;

	try
	{
		if (compare(pParameter_, _cJapanese3) == true)
			// 新日本語ブロック化器v3
			m_pBlocker = new JapaneseBlocker3();
		else if (compare(pParameter_, _cJapanese2) == true)
			// 新日本語ブロック化器v2
			m_pBlocker = new JapaneseBlocker2();
		else if (compare(pParameter_, _cJapanese1) == true)
			// 旧日本語ブロック化器
			// 未テストのためサポート外
			_SYDNEY_THROW0(Exception::NotSupported);
		else if (compare(pParameter_, _cJapanese) == true)
			// デフォルトは新日本語ブロック化器v2
			m_pBlocker = new JapaneseBlocker2();
		else
			// 文字種を確認しないブロック化器
			m_pBlocker = new GeneralBlocker();
		
		m_pBlocker->parse(pParameter_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		if (m_pBlocker) delete m_pBlocker, m_pBlocker = 0;
		_TRMEISTER_RETHROW;
	}
}

//
//	Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
