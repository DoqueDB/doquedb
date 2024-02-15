// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Term.cpp -- 質問処理ライブラリのラッパークラス
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Utility";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Utility/Term.h"

#include "Common/UnicodeString.h"
#include "Common/WordData.h"

#include "LogicalFile/FileID.h"

#include "Utility/TermResourceManager.h"

#include "Exception/BadArgument.h"

#include "FullText2/KeyID.h"	// FileIDの中身を確認するために必要

#include "ModAutoPointer.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeString.h"

#define _PARAMETER_KEY(key_)		LogicalFile::Parameter::Key((key_), 0)

_TRMEISTER_USING
_TRMEISTER_UTILITY_USING

namespace
{
	// UNAのリソース番号
	ModUnicodeString _cUNA("@UNARSCID:");
	// NORMのリソース番号
	ModUnicodeString _cNORM("@NORMRSCID:");
	// TERMのリソース番号
	ModUnicodeString _cTERM("@TERMRSCID:");


	namespace _UnaParam
	{
		ModUnicodeString _cDoNorm("donorm");
		ModUnicodeString _cCompound("compound");
		ModUnicodeString _cStem("stem");
		ModUnicodeString _cCarriage("carriage");
		ModUnicodeString _cSpace("space");

		ModUnicodeString _cTrue("true");
		ModUnicodeString _cFalse("false");
		ModUnicodeString _cTwo("2");
		ModUnicodeString _cZero("0");
	}

	// 検索語セパレータ
	ModUnicodeChar _TermSeparator(Common::UnicodeChar::usSpace);
}

//
//	FUNCTION public
//	Utility::Term::Term -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		全文索引のファイルID
//	ModSize uiCollectionSize_
//		登録件数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Term::Term(const LogicalFile::FileID& cFileID_,
		   ModSize uiCollectionSize_)
	: m_pTerm(0), m_pPool1(0), m_pPool2(0), m_pCand2(0), m_pMap(0),
	  m_iTermResourceID(0), m_iUnaResourceID(0),
	  m_bNormalized(false), m_bStemming(false),
	  m_bIgnoreCarriage(false), m_bDeleteSpace(false),
	  m_uiLimit(0), m_uiRelatedLimit(0),
	  m_dblCollectionSize(uiCollectionSize_),
	  m_uiWordLimit(0), m_dblScaleParameter(0)
{
	setParameter(cFileID_);
}

//
//	FUNCTION public
//	Utility::Term::~Term -- デストラクタ
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
Term::~Term()
{
	clear();
}

//
//	FUNCTION public
//	Utility::Term::setExtractor -- 質問処理器パラメータを設定する
//
//	NOTES
//	contains のオプションとして extractor が指定されている場合、
//	本メソッドに extractor オプションの文字列を渡すこと
//
//	ARGUMENTS
//	const ModUnicodeString& cExtractor_
//		質問処理器パラメータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Term::setExtractor(const ModUnicodeString& cExtractor_)
{
	// @TERMRSCID:?
	const ModUnicodeChar* p = cExtractor_.search(_cTERM, ModFalse);
	if (p)
	{
		p += _cTERM.getLength();
		m_iTermResourceID = ModUnicodeCharTrait::toInt(p);
	}

	// @UNARSCID:?
	p = cExtractor_.search(_cUNA, ModFalse);
	if (p)
	{
		p += _cUNA.getLength();
		m_iUnaResourceID = ModUnicodeCharTrait::toInt(p);
	}
}

//
//	FUNCTION public
//	Utility::Term::makePool -- 自然文からプールを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cFreeText_
//		自然文
//	const ModLanguageSet& cLang_
//		言語
//	
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Term::makePool(const ModUnicodeString& cFreeText_,
			   const ModLanguageSet& cLang_)
{
	ModTerm* pTerm = getTerm();
	
	// 自然文から pool を作成
	pTerm->poolTerm(cFreeText_,
					cLang_.getSize() ? cLang_ : m_cLang,
					ModTermElement::voidMatch,	// ここでは利用されない
					*m_pPool1);

	// この場合、すべてのカテゴリをHelpfulにする
	for (ModTermPool::Iterator i = m_pPool1->begin(); i != m_pPool1->end(); ++i)
	{
		(*i).setType(Common::WordData::Category::Helpful);
	}
}

//
//	FUNCTION public
//	Utility::Term::makePool -- 単語リストをプールに設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<Common::WordData>& vecWords_
//		単語リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Term::makePool(const ModVector<Common::WordData>& vecWords_)
{
	ModTerm* pTerm = getTerm();
	
	int pos = 0;
	ModVector<Common::WordData>::ConstIterator i = vecWords_.begin();
	for (; i != vecWords_.end(); ++i)
	{
		ModTermElement term;

		// 一致条件
		term.setMatchMode(ModTermElement::voidMatch);
		// 言語
		term.setLangSpec((*i).getLanguage().getSize() ?
						 (*i).getLanguage() : m_cLang);
		// カテゴリ
		term.setType((*i).getCategory());
		// スケール
		term.setScale((*i).getScale());
		// 文書頻度
		term.setDf((*i).getDocumentFrequency());
		// 選択率
		term.setTwv(1);
		// 単語
		term.setString(pTerm->getNormalizedString((*i).getLanguage(),
												  (*i).getTerm()));
		term.setOriginalString((*i).getTerm());
		// 位置
		term.setPosition(pos);

		// プールに挿入する
		m_pPool1->insertTerm(term);
	}

	// プールの有効化
	pTerm->validatePool(*m_pPool1);
}

//
//	FUNCTION public
//	Utility::Term::getCandidate -- 候補を得る
//
//	NOTES
//	特徴語かどうか選択するために、候補を取得しDF値を求めてもらう
//
//	ARGUMENTS
//	ModVector<Common::WordData>& vecCandidate_
//		候補
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Term::getCandidate(ModVector<Common::WordData>& vecCandidate_)
{
	vecCandidate_.clear();
	
	ModTermPool::Iterator i = m_pPool1->begin();
	for (; i != m_pPool1->end(); ++i)
	{
		if ((*i).getDf() != 0)
			// すでに求められているので新たに求める必要なし
			continue;

		if ((*i).getString().getLength() == 0)
			// 正規化の結果、空文字列になった
			continue;
		
		Common::WordData cData;

		cData.setTerm((*i).getString());	// mapのキー
		cData.setLanguage((*i).getLangSpec());
		cData.setCategory(
			static_cast<Common::WordData::Category::Value>((*i).getType()));

		vecCandidate_.pushBack(cData);
	}
}

//
//	FUNCTION public
//	Utility::Term::setDocumentFrequency -- DF値を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<Common::WordData>& vecWords_
//		単語リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Term::setDocumentFrequency(const ModVector<Common::WordData>& vecWords_)
{
	ModVector<Common::WordData>::ConstIterator i = vecWords_.begin();
	for (; i != vecWords_.end(); ++i)
	{
		m_pPool1->setDf((*i).getTerm(), (*i).getDocumentFrequency());
	}
}

//
//	FUNCTION public
//	Utility::Term::expandPool -- 関連文書から単語を拡張する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<ModUnicodeString>& vecRelevance_
//		関連文書
//	const ModVector<ModLanguageSet>& vecLang_
//		関連文書の言語
//	ModSize 
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Term::expandPool(const ModVector<ModUnicodeString>& vecRelevance_,
				 const ModVector<ModLanguageSet>& vecLang_)
{
	if (vecLang_.getSize() != 0 &&
		vecRelevance_.getSize() != vecLang_.getSize())
		_TRMEISTER_THROW0(Exception::BadArgument);

	// シード文書から検索語をマップに格納
	m_pMap = new ModTermMap;
	ModVector<ModUnicodeString>::ConstIterator d = vecRelevance_.begin();
	ModVector<ModLanguageSet>::ConstIterator l = vecLang_.begin();
	ModSize id = 1;
	ModLanguageSet lang = m_cLang;
	for (; d != vecRelevance_.end(); ++d)
	{
		if (l != vecLang_.end())
			lang = *l;

		getTerm()->mapTerm(*d, lang, ModTermElement::voidMatch, id, *m_pMap);
	}

	// 初期検索語に重みを付ける
	getTerm()->weightTerm(*m_pMap, *m_pPool1, m_dblCollectionSize);

	// 拡張語の候補用プール
	m_pCand2 = new ModTermPool(getTerm()->maxCandidate);
	// 拡張語の候補をプール
	getTerm()->poolTerm(*m_pMap, *m_pCand2);
}

//
//	FUNCTION public
//	Utility::Term::getRelatedCandidate -- 関連文書の候補を得る
//
//	NOTES
//	特徴語かどうか選択するために、候補を取得しDF値を求めてもらう
//
//	ARGUMENTS
//	ModVector<Common::WordData>& vecCandidate_
//		候補
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Term::getRelatedCandidate(ModVector<Common::WordData>& vecCandidate_)
{
	vecCandidate_.clear();

	if (m_pCand2 == 0)
		return;
	
	ModTermPool::Iterator i = m_pCand2->begin();
	for (; i != m_pCand2->end(); ++i)
	{
		if ((*i).getDf() != 0)
			// すでに求められているので新たに求める必要なし
			continue;
		
		Common::WordData cData;

		cData.setTerm((*i).getString());	// mapのキー
		cData.setLanguage((*i).getLangSpec());
		cData.setCategory(Common::WordData::Category::HelpfulRelated);

		vecCandidate_.pushBack(cData);
	}
}

//
//	FUNCTION public
//	Utility::Term::setRelatedDocumentFrequency -- 関連文書のDF値を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<Common::WordData>& vecWords_
//		単語リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Term::setRelatedDocumentFrequency(const ModVector<Common::WordData>& vecWords_)
{
	if (m_pCand2 == 0) return;
	
	ModVector<Common::WordData>::ConstIterator i = vecWords_.begin();
	for (; i != vecWords_.end(); ++i)
	{
		m_pCand2->setDf((*i).getTerm(), (*i).getDocumentFrequency());
	}

	// 候補群から拡張検索語を選択
	getTerm()->selectTerm(*m_pMap, *m_pCand2, *m_pPool2, m_dblCollectionSize);

	// 拡張検索語のカテゴリーを設定する
	ModTermPool::Iterator j = m_pPool2->begin();
	for (; j != m_pPool2->end(); ++j)
	{
		(*j).setType(Common::WordData::Category::HelpfulRelated);
	}
}

//
//	FUNCTION public
//	Utility::Term::getSelection -- 最終結果の選択された単語を取得する
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<Common::WordData>& vecWords
//		最終結果
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Term::getSelection(ModVector<Common::WordData>& vecWords_)
{
	vecWords_.clear();
	ModTermPool::Iterator i;
	
	for (i = m_pPool1->begin(); i != m_pPool1->end(); ++i)
	{
		// 選択値が 0 の場合は、含めない
		if ((*i).getTsv() == 0) continue;

		Common::WordData c((*i).getOriginalString());
		c.setLanguage((*i).getLangSpec());
		c.setCategory(
			static_cast<Common::WordData::Category::Value>((*i).getType()));
		c.setScale(((*i).getScale() == 0) ? 1.0 : (*i).getScale());
		c.setDocumentFrequency(static_cast<int>((*i).getDf()));

		vecWords_.pushBack(c);
	}

	if (vecWords_.getSize() == 0)
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

			vecWords_.pushBack(c);
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

		vecWords_.pushBack(c);
	}
}

//
//	FUNCTION public
//	Utility::Term::clear -- 内部状態をクリアする
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
Term::clear()
{
	delete m_pTerm, m_pTerm = 0;
	delete m_pPool1, m_pPool1 = 0;
	delete m_pPool2, m_pPool2 = 0;
	delete m_pCand2, m_pCand2 = 0;
	delete m_pMap, m_pMap = 0;
}

//
//	FUNCTION public
//	Utility::Term::getFormula -- 引数のWordDataの単語のcontainsの検索式を得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::WordData& cWord_
//		ワードデータ
//
//	RETURN
//	ModUnicodeString
//		contains の検索式
//
//	EXCEPTIONS
//
ModUnicodeString
Term::getFormula(const Common::WordData& cWord_) const
{
	//【注意】	Common::WordData だけでは情報が不足していて検索式が作れない
	//			そのため、本クラスに検索式を作るメソッドを追加した

	ModUnicodeOstrStream s;

	// ワードデータの単語を得る
	const ModUnicodeString& term = cWord_.getTerm();

	// セパレータの処理を設定する
	int proximity = 0;
	switch (cWord_.getCategory())
	{
	case Common::WordData::Category::EssentialRelated:
	case Common::WordData::Category::ImportantRelated:
	case Common::WordData::Category::HelpfulRelated:
	case Common::WordData::Category::ProhibitiveRelated:
		// 関連語
		proximity = m_pTerm->paramProximity2;
		break;
	default:
		// その他は初期語とする
		proximity = m_pTerm->paramProximity1;
	}

	// within を使うかチェックする
	bool bWithin = false;
	if (proximity != 0 && term.search(_TermSeparator) != 0)
	{
		// パラメータが within を使うとなっており、かつ、
		// セパレータが文字列中に存在するので、within を使う
		
		bWithin = true;
	}

	if (bWithin)
	{
		// 隣接語は within にする
		s << "within(";
	}

	s << "'";

	const ModUnicodeChar* b = term;
	const ModUnicodeChar* p = b;
	while (*p != 0)
	{
		if (*p == Common::UnicodeChar::usQuate)
		{
			// ' -> '' にするために、ここで加える
			
			s << Common::UnicodeChar::usQuate;
		}
		
		if (*p == _TermSeparator)
		{
			if (proximity == 0)
			{
				// within を使わない場合

				if (p == b || *(p+1) == 0)
				{
					// 先頭または末尾のセパレータを削除
					;
				}
				else
				{
					// 文字列中のセパレータ
				
					if (ModUnicodeCharTrait::isAlphabet(*(p-1))
						&& ModUnicodeCharTrait::isAlphabet(*(p+1)))
					{
						// 英文字 + セパレータ + 英文字 の場合は
						// セパレータを残す
						s << *p;
					}
					else if (ModUnicodeCharTrait::isDigit(*(p-1))
							 && ModUnicodeCharTrait::isDigit(*(p+1)))
					{
						// 数字 + セパレータ + 数字 の場合は
						// セパレータを残す
						s << *p;
					}
					else
					{
						// それ以外はセパレータを削除
						;
					}
				}
			}
			else
			{
				// within を使う場合

				s << "' '";
			}
		}
		else
		{
			s << *p;
		}

		++p;
	}

	s << "'";

	if (bWithin)
	{
		if (proximity > 0)
		{
			// 順序指定
			s << " symmetric upper " << proximity << ")";
		}
		else
		{
			// 順序不定
			s << "asymmetric upper " << -proximity << ")";
		}
	}

	return ModUnicodeString(s.getString());
}

//
//	FUNCTION private
//	Utility::Term::setParameter -- 全文索引のファイルIDから必要な情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		全文索引のファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Term::setParameter(const LogicalFile::FileID& cFileID_)
{
	//【注意】必ず全文索引のFileIDを渡すこと

	// バージョン確認
	int iVersion;
	if (cFileID_.getInteger(
			_PARAMETER_KEY(LogicalFile::FileID::KeyNumber::Version),
			iVersion) == false)
		iVersion = 0;
	if (iVersion < FullText2::VersionNum::Version4)
		_SYDNEY_THROW0(Exception::BadArgument);

	// トークナイズパラメータを得て、UNAのリソース番号を調べる
	ModUnicodeString cValue;
	if (cFileID_.getString(_PARAMETER_KEY(FullText2::KeyID::TokenizeParameter),
						   cValue) == false)
		_SYDNEY_THROW0(Exception::BadArgument);
	const ModUnicodeChar* p = cValue.search(_cUNA, ModFalse);
	if (p) p += _cUNA.getLength();
	else
	{
		p = cValue.search(_cNORM, ModFalse);
		if (p) p += _cNORM.getLength();
	}
	if (p)
	{
		m_iUnaResourceID = ModUnicodeCharTrait::toInt(p);
	}

	// エクストラクターを得て、Termのリソース番号を調べる
	if (cFileID_.getString(_PARAMETER_KEY(FullText2::KeyID::Extractor),
						   cValue) == true)
	{
		p = cValue.search(_cTERM, ModFalse);
		if (p)
		{
			p += _cTERM.getLength();
			m_iTermResourceID = ModUnicodeCharTrait::toInt(p);
		}
	}

	//
	// 異表記正規化関係
	//
	m_bNormalized = cFileID_.getBoolean(
		_PARAMETER_KEY(FullText2::KeyID::IsNormalized));
	if (m_bNormalized)
	{
		bool v;
		
		m_bStemming = true;
		if (cFileID_.getBoolean(
				_PARAMETER_KEY(FullText2::KeyID::IsStemming), v) == true)
		{
			m_bStemming = v;
		}
		
		m_bDeleteSpace = true;
		if (cFileID_.getBoolean(
				_PARAMETER_KEY(FullText2::KeyID::IsDeleteSpace), v) == true)
		{
			m_bDeleteSpace = v;
		}

		m_bIgnoreCarriage = true;
		if (cFileID_.getBoolean(
				_PARAMETER_KEY(FullText2::KeyID::IsCarriage), v) == true)
		{
			m_bIgnoreCarriage = v;
		}
	}

	// 言語
	if (cFileID_.getString(_PARAMETER_KEY(FullText2::KeyID::Language),
						   cValue) == true)
	{
		m_cLang = cValue;
	}
}

//
//	FUNCTION private
//	Utility::Term::getTerm -- 質問処理ライブラリを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	質問処理ライブラリ
//
//	EXCEPTIONS
//
ModTerm*
Term::getTerm()
{
	if (m_pTerm == 0)
	{
		//
		// まだ作られていないので、質問処理器を作成する
		//

		// UNAのアナライザーを得る
		ModAutoPointer<UNA::ModNlpAnalyzer> anal
			= Una::Manager::getModNlpAnalyzer(m_iUnaResourceID);

		// UNAのパラメータを得る
		UNA::ModNlpAnalyzer::Parameters param;
		makeUnaParameter(param);

		// 質問処理器のリソースを得る
		const ModTermResource* resource
			= Utility::TermResourceManager::get(m_iTermResourceID);

		// 質問処理器を作成する
		m_pTerm = new ModTerm(resource, anal.release(), param);

		// 上限を設定する
		m_pTerm->maxTerm1 = (m_pTerm->maxTerm1 > m_uiLimit) ?
			m_pTerm->maxTerm1 : m_uiLimit;
		m_pTerm->maxTerm2 = (m_pTerm->maxTerm2 > m_uiRelatedLimit) ?
			m_pTerm->maxTerm2 : m_uiRelatedLimit;

		// 検索語数と重みパラメータを設定する
		if (m_uiWordLimit != 0)
			m_pTerm->maxTerm1 = m_uiWordLimit;
		if (m_dblScaleParameter != 0)
			m_pTerm->paramScale1 = m_dblScaleParameter;

		// プールを用意する
		m_pPool1 = new ModTermPool(m_pTerm->maxTerm1);
		m_pPool2 = new ModTermPool(m_pTerm->maxTerm2);
	}
	
	return m_pTerm;
}

//
//	FUNCTION private
//	Term::FullTextFile::makeUnaParameter
//		-- UNAの解析器用のパラメータを作成する
//
//	NOTES
//
//	ARGUMENTS
//	UNA::ModNlpAnalyzer::Parameters& p;
//		UNA解析器用のパラメータ
//
// 	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Term::makeUnaParameter(UNA::ModNlpAnalyzer::Parameters& p)
{
	p.insert(_UnaParam::_cDoNorm,
			 m_bNormalized ? _UnaParam::_cTrue : _UnaParam::_cFalse);
	p.insert(_UnaParam::_cCompound, _UnaParam::_cTrue);
	p.insert(_UnaParam::_cStem,
			 m_bStemming ? _UnaParam::_cTrue : _UnaParam::_cFalse);
	p.insert(_UnaParam::_cCarriage,
			 m_bIgnoreCarriage ? _UnaParam::_cTrue : _UnaParam::_cFalse);

	//
	// 空白の処理は 0 が AsIs (リソースのデフォルト) で、
	// 2 が DeleteSpace である
	//
	p.insert(_UnaParam::_cSpace,
			 m_bDeleteSpace ? _UnaParam::_cTwo : _UnaParam::_cZero);
}

//
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
