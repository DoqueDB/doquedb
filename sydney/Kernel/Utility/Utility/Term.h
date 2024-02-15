// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Term.h -- 質問処理ライブラリのラッパークラス
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

#ifndef	__TRMEISTER_UTILITY_TERM_H
#define	__TRMEISTER_UTILITY_TERM_H

#include "Utility/Module.h"

#include "Common/WordData.h"

#include "LogicalFile/FileID.h"

#include "Utility/ModTerm.h"
#include "Utility/ModTermElement.h"
#include "Utility/UNA.h"

#include "ModLanguageSet.h"
#include "ModUnicodeString.h"
#include "ModVector.h"
#include "ModTypes.h"

class ModTermResource;

_TRMEISTER_BEGIN
_TRMEISTER_UTILITY_BEGIN

//
//	CLASS
//	Utility::Term -- 質問処理ライブラリのラッパークラス
//
//	NOTES
//

class Term
{
public:
	// コンストラクタ
	Term(const LogicalFile::FileID& cFileID_, ModSize uiCollectionSize_);
	// デストラクタ
	virtual ~Term();

	// 質問処理器パラメータを設定する(オプション)
	void setExtractor(const ModUnicodeString& cExtractor);
	// リミットを設定する(オプション)
	void setLimit(ModSize uiLimit_)
		{ m_uiLimit = uiLimit_; }
	void setRelatedLimit(ModSize uiLimit_)
		{ m_uiRelatedLimit = uiLimit_; }
	// 検索に使用する単語数を指定する
	void setWordLimit(ModSize uiLimit_)
		{ m_uiWordLimit = uiLimit_; }
	// 重みパラメータを指定する
	void setScaleParameter(double param_)
		{ m_dblScaleParameter = param_; }

	// 自然文からプールを作成する
	void makePool(const ModUnicodeString& cFreeText_,
				  const ModLanguageSet& cLang_);
	// 単語リストをプールに挿入する
	void makePool(const ModVector<Common::WordData>& vecWords_);

	// 候補を得る
	void getCandidate(ModVector<Common::WordData>& vecCandidate_);

	// DF値を設定する
	void setDocumentFrequency(const ModVector<Common::WordData>& vecWords_);
	
	// 関連文書から単語を拡張する
	void expandPool(const ModVector<ModUnicodeString>& vecRelevance_,
					const ModVector<ModLanguageSet>& vecLang_);

	// 関連文書の候補を得る
	void getRelatedCandidate(ModVector<Common::WordData>& vecCandidate_);


	// 関連文書のDF値を設定する
	void setRelatedDocumentFrequency(const ModVector<Common::WordData>&
									 vecWords_);

	// 最終結果を得る
	void getSelection(ModVector<Common::WordData>& vecWords_);

	// 内部状態をクリアする
	void clear();

	// 引数の単語の検索式を得る
	ModUnicodeString getFormula(const Common::WordData& cWord_) const;

private:
	// 全文索引のファイルIDから情報を抜き出す
	void setParameter(const LogicalFile::FileID& cFileID_);

	// UNA解析器用のパラメータを作成する
	void makeUnaParameter(UNA::ModNlpAnalyzer::Parameters& param);

	// 質問処理ライブラリを得る
	ModTerm* getTerm();
	
	// 質問処理ライブラリ
	ModTerm* m_pTerm;
	
	// プール
	ModTermPool* m_pPool1;
	ModTermPool* m_pPool2;
	ModTermPool* m_pCand2;
	
	// マップ
	ModTermMap* m_pMap;

	// 質問処理のリソース番号
	int m_iTermResourceID;
	// 言語処理のリソース番号
	int m_iUnaResourceID;

	// 正規化するかどうか
	bool m_bNormalized;
	// ステミングするかどうか
	bool m_bStemming;
	// 改行を無視するかどうか
	bool m_bIgnoreCarriage;
	// スペースを削除するかどうか
	bool m_bDeleteSpace;

	// デフォルトの言語
	ModLanguageSet m_cLang;

	// 上限
	ModSize m_uiLimit;
	ModSize m_uiRelatedLimit;

	// 登録件数
	double m_dblCollectionSize;

	// 検索語に使う単語数
	ModSize m_uiWordLimit;
	// 重みパラメータ
	double m_dblScaleParameter;
};

_TRMEISTER_UTILITY_END
_TRMEISTER_END

#endif	// __TRMEISTER_UTILITY_TERM_H

//
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
