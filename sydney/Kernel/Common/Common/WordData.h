// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// WordData.h -- 全文索引から得られる単語を表すクラス
// 
// Copyright (c) 2004, 2010, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_WORDDATA_H
#define __TRMEISTER_COMMON_WORDDATA_H

#include "Common/Module.h"
#include "Common/Data.h"

#include "ModUnicodeString.h"
#include "ModLanguageSet.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//
//	CLASS
//	Common::WordData -- 全文索引から得られる単語を表すクラス
//
//	NOTES
//

class SYD_COMMON_FUNCTION WordData : public Data
{
public:
	//
	//	ENUM
	//	Common::WordData::Category::Value
	//
	struct Category
	{
		enum Value
		{
			Undefined = 0,				// 未定義
			
			Essential,					// 必須
			Important,					// 重要
			Helpful,					// 有用
			EssentialRelated,			// 必須関連語
			ImportantRelated,			// 重要関連語
			HelpfulRelated,				// 有用関連語
			Prohibitive,				// 禁止
			ProhibitiveRelated,			// 禁止関連語

			ValueNum
		};
	};
	
	// コンストラクタ
	WordData();
	// コンストラクタ
	explicit WordData(const ModUnicodeString& cTerm_);
	// コピーコンストラクタ
	WordData(const WordData& cWordData_);
	// デストラクタ
	virtual ~WordData();

	// 単語
	const ModUnicodeString& getTerm() const
		{ return m_cTerm; }
	void setTerm(const ModUnicodeString& cTerm_)
		{ m_cTerm = cTerm_; }

	// 言語
	const ModLanguageSet& getLanguage() const
		{ return m_cLanguage; }
	void setLanguage(const ModLanguageSet& cLanguage_)
		{ m_cLanguage = cLanguage_; }

	// カテゴリー
	Category::Value getCategory() const
		{ return m_eCategory; }
	void setCategory(Category::Value eCategory_)
		{ m_eCategory = eCategory_; }

	// スケール
	double getScale() const
		{ return m_dblScale; }
	void setScale(double scale_)
		{ m_dblScale = scale_; }

	// 文書頻度
	int getDocumentFrequency() const
		{ return m_iDocumentFrequency; }
	void setDocumentFrequency(int df_)
		{ m_iDocumentFrequency = df_; }

	// 代入演算子
	WordData& operator= (const WordData& cWordData_);
	
	// シリアル化する
//	Common::Data
//	virtual void
//	serialize(ModArchive& archiver);

	// コピーする
//	Common::Data
//	virtual Pointer
//	copy() const;
	// キャストする
//	Common::Data
//	virtual Pointer
//	cast(DataType::Type type) const;
//	virtual Pointer
//	cast(const Data& target) const;

	// 文字列の形式で値を得る
//	Common::Data
//	virtual ModUnicodeString
//	getString() const;

	// 等しいか調べる
	virtual bool
	equals(const Data* r) const;
	// 大小比較を行う
//	Common::Data
//	virtual int
//	compareTo(const Data* r) const;

	// 代入を行う
//	Common::Data
//	virtual bool
//	assign(const Data* r);
	// 四則演算を行う
//	Common::Data
//	virtual bool
//	operateWith(DataOperation::Type op, const Data* r, Pointer& result) const;
	// 単項演算を行う
//	Common::Data
//	virtual bool
//	operateWith(DataOperation::Type op, Pointer& result) const;
//	virtual bool
//	operateWith(DataOperation::Type op);

	// クラスIDを得る
//	Common::Data
//	virtual int
//	getClassID() const;

	//ハッシュコードを取り出す
	virtual ModSize hashCode() const;

	// 文字列カテゴリをenumに変換する
	static Category::Value toCategory(const ModUnicodeString& category);

protected:
	// シリアル化する(自分自身が NULL 値でない)
	virtual void
	serialize_NotNull(ModArchive& archiver);

	// コピーする(自分自身が NULL 値でない)
	virtual Pointer
	copy_NotNull() const;

	// 文字列の形式で値を得る(自分自身が NULL 値でない)
	virtual ModUnicodeString
	getString_NotNull() const;

	// 等しいか調べる(キャストなし)
	virtual bool
	equals_NoCast(const Data& r) const;

	// 大小比較を行う(キャストなし)
//	Common::Data
	virtual int
	compareTo_NoCast(const Data& r) const;

	// 代入を行う(キャストなし)
	virtual bool
	assign_NoCast(const Data& r);

	// 四則演算を行う(キャストなし)
//	Common::Data
	virtual bool
	operateWith_NoCast(DataOperation::Type op, const Data& r);

	// クラスIDを得る(自分自身が NULL 値でない)
	virtual int
	getClassID_NotNull() const;

private:
	// 単語
	ModUnicodeString m_cTerm;
	// 言語
	ModLanguageSet m_cLanguage;
	// カテゴリー
	Category::Value m_eCategory;
	// スケール
	double m_dblScale;
	// 文書頻度
	int m_iDocumentFrequency;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_WORDDATA_H

//
// Copyright (c) 2004, 2010, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
