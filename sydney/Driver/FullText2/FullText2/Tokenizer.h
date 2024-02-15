// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Tokenizer.h --
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

#ifndef __SYDNEY_FULLTEXT2_TOKENIZER_H
#define __SYDNEY_FULLTEXT2_TOKENIZER_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"
#include "FullText2/FeatureList.h"
#include "FullText2/SmartLocationList.h"

#include "ModLanguageSet.h"
#include "ModMap.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

#include "ModNLP.h"

_SYDNEY_BEGIN

namespace Utility {
	class ModTermResource;
}

_SYDNEY_FULLTEXT2_BEGIN

class Blocker;
class FullTextFile;
class ListManager;
class LeafNode;
class Query;

//
//	TYPEDEF
//	FullText2::SmartLocationListMap -- 位置情報リストのマップ
//
//	NOTES
//
typedef ModMap<ModUnicodeString, SmartLocationList,
			   ModLess<ModUnicodeString> >	SmartLocationListMap;

//
//	CLASS
//	FullText2::Tokenizer -- トークナイザーの基底クラス
//
//	NOTES
//
class Tokenizer
{
public:
	//
	//	CLASS
	//	FullText2::Tokenizer::AutoPointer -- 自動ポインタ
	//
	//	NOTES
	//
	class AutoPointer
	{
	public:
		// コンストラクタ
		AutoPointer(Tokenizer* p_ = 0)
			: m_bOwner(p_ ? true : false), m_pPointer(p_) {}
		// コピーコンストラクタ
		AutoPointer(const AutoPointer& c_)
			: m_bOwner(c_.m_bOwner), m_pPointer(c_.release()) {}
		// デストラクタ
		~AutoPointer() { expunge(); }

		// アクセッサ
		Tokenizer* get() const
			{ return m_pPointer; }
		Tokenizer* release() const
			{ m_bOwner = false; return m_pPointer; }
		Tokenizer& operator *() const
			{ return *get(); }
		Tokenizer* operator ->() const
			{ return get(); }
		operator Tokenizer* ()
			{ return get(); }
		operator const Tokenizer* () const
			{ return get(); }

		// 代入演算子
		AutoPointer& operator =(const AutoPointer& c_)
			{
				expunge();
				m_bOwner = c_.m_bOwner;
				m_pPointer = c_.release();
				return *this;
			}
		AutoPointer& operator =(Tokenizer* p_)
			{
				expunge();
				m_bOwner = p_ ? true : false;
				m_pPointer = p_;
				return *this;
			}
		
	private:
		// 解放する
		void expunge()
			{
				if (m_bOwner == false)
					return;

				m_pPointer->release();
				m_bOwner = false;
				m_pPointer = 0;
			}
		
		mutable bool	m_bOwner;
		Tokenizer*		m_pPointer;
	};
	
	// コンストラクタ
	Tokenizer(FullTextFile& cFile_, UNA::ModNlpAnalyzer* pAnalyzer_);
	// デストラクタ
	virtual ~Tokenizer();

	// トークナイズパラメータをチェックし、正規化して返す
	static ModUnicodeString check(IndexingType::Value eType_,
								  const ModUnicodeString& cstrParameter_);
	// 下請け
	static ModUnicodeString check(const ModUnicodeChar* pParameter_);

	// トークナイザーを得る
	static Tokenizer* createTokenizer(FullTextFile& cFile_,
									  UNA::ModNlpAnalyzer* pAnalyzer_,
									  const ModUnicodeString& cstrParameter_);

	// 特徴語取得のための情報を付加する(ここでは常にNotSupported例外発生)
	virtual
	void setFeatureParameter(ModSize uiFeatureSize_, // 特徴語の数
							 const Utility::ModTermResource* pTermResource_);

	// トークナイズする(通常用)
	void tokenize(const ModUnicodeString& cTarget_,	// 対象の文字列
				  const ModLanguageSet& cLang_,		// 対象の言語
				  SmartLocationListMap& cResult_,	// 結果
				  ModSize& uiSize_,					// 正規化後のサイズ
				  ModSize& uiOriginalSize_);		// 正規化前のサイズ			
	// トークナイズする(セクション用)
	void tokenize(const ModVector<ModUnicodeString>& vecTarget_, // 対象文字列
				  const ModVector<ModLanguageSet>& vecLang_,	 // 対象言語
				  SmartLocationListMap& cResult_,			// 結果
				  ModVector<ModSize>& vecSize_,				// 正規化後のサイズ
				  ModVector<ModSize>& vecOriginalSize_);	// 正規化前のサイズ

	// 特徴語リストを得る(ここでは常にNotSupported例外発生)
	virtual void getFeatureList(FeatureList& vecFeature_);

	// 検索用のLeafNodeを作成する
	LeafNode* createLeafNode(ListManager& cManager_,
							 const ModUnicodeString& cTerm_,
							 const ModLanguageSet& cLang_,
							 MatchMode::Value eMatchMode_);

	// target_ の先頭が src_ と一致するか比較する
	// src_ と一致した次の文字が ':' か ' ' か '\0' の場合は一致
	// それ以外の場合は不一致となる
	// 一致した場合は、target_ は一致した位置まで進められる
	static bool compare(const ModUnicodeChar*& target_,
						const ModUnicodeChar* src_);

	// 解放する
	void release();

protected:
	// ブロック化器を設定する
	void setBlocker(const ModUnicodeChar* pParameter_);
	
	// 検索用のLeafNodeを作成する
	virtual LeafNode* createLeafNodeImpl(ListManager& cManager_,
										 const ModUnicodeString& cTerm_,
										 const ModLanguageSet& cLang_,
										 MatchMode::Value eMatchMode_) = 0;

	// 正規化する
	virtual
	void normalize(const ModUnicodeString& cTarget_,	// 対象の文字列
				   const ModLanguageSet& cLang_,		// 対象の言語
				   ModSize uiStartPosition_,			// 対象文字列の先頭位置
				   SmartLocationListMap& cResult_,		// 結果(単語境界のみ)
				   ModSize& uiSize_,					// 正規化後のサイズ
				   ModSize& uiOriginalSize_) = 0;		// 正規化前のサイズ
	// トークナイズする
	virtual	void tokenize(SmartLocationListMap& cResult_) = 0;	// 結果

	// 初期化する
	virtual	void initialize() = 0;

	// 全文ファイルクラス
	FullTextFile& m_cFile;
	// UNA
	UNA::ModNlpAnalyzer* m_pAnalyzer;
	// ブロック化器
	Blocker* m_pBlocker;

	//
	// 高速化のためのキャッシュ
	//
	
	// 索引タイプ
	IndexingType::Value	m_eIndexingType;
	// 正規化するかどうか
	bool				m_bNormalized;
	// 位置情報が格納されているかどうか
	bool				m_bNoLocation;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_TOKENIZER_H

//
//	Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
