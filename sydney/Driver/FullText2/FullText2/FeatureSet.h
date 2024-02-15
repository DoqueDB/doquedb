// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FeatureSet.h -- 特徴語列を表現するクラス
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_FEATURESET_H
#define __SYDNEY_FULLTEXT2_FEATURESET_H

#include "FullText2/Module.h"
#include "FullText2/FeatureList.h"

#include "Os/Memory.h"

#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::FeatureSet
//
//	NOTES
//
class FeatureSet
{
	// 	フォーマット
	//
	//           |<------- 4 byte 境界に丸められる ------->|
	//  +--------+--------+--------+-----------------------+--
	//  |4byte   |4byte   |2byte   |2byte x length         |
	//	+--------+--------+--------+-----------------------+--
	//	|特徴語数|  重み  |特徴語長| 特徴語文字列(可変長)  | …
	//  +--------+--------+--------+-----------------------+--
	//
	//【注意】
	//	本クラスはそのままDirectAreaにダンプされるので、継承したり virtual 関数
	// 	を追加したりしてはいけない。

public:
	//
	//	CLASS
	//	FullText2::FeatureSet::Feature -- 1つの特徴語情報を表すクラス
	//
	struct Feature
	{
		float				m_fWeight;		// 重み
		unsigned short		m_usLength;		// 特徴語長
		ModUnicodeChar		m_pBuffer[1];	// 特徴語文字列(可変長)

		// サイズを求める
		ModSize getSize() const
		{
			return getSize(static_cast<ModSize>(m_usLength));
		}
		static ModSize getSize(const FeatureElement& feature_)
		{
			return getSize(feature_.term.getLength());
		}
		static ModSize getSize(ModSize uiLength_)
		{
			return (uiLength_ * sizeof(ModUnicodeChar)
					+ sizeof(unsigned short) + sizeof(ModUInt32) - 1)
				/ sizeof(ModUInt32) * sizeof(ModUInt32) + sizeof(float);
		}

		// 特徴語の文字列長を得る
		ModSize getLength() const { return m_usLength; }
		// 特徴語の文字列を得る(nullターミネートはされていない)
		const ModUnicodeChar* getString() const { return m_pBuffer; }

		// 重みを得る
		float getWeight() const { return m_fWeight; }
		
		// 大小比較を行う
		int compareTo(const Feature& other_) const
		{
			ModSize l = (m_usLength < other_.m_usLength) ?
				m_usLength : other_.m_usLength;
			const ModUnicodeChar* s = m_pBuffer;
			const ModUnicodeChar* d = other_.m_pBuffer;
			for (ModSize i = 0; i < l; ++i, ++s, ++d)
			{
				if (*s != *d)
					return static_cast<int>(*s - *d);
			}
			return (static_cast<int>(m_usLength)
					- static_cast<int>(other_.m_usLength));
		}

		// ポインターを進める
		const Feature* next() const
		{
			const char* p = syd_reinterpret_cast<const char*>(this);
			p += getSize();
			return syd_reinterpret_cast<const Feature*>(p);
		}
		Feature* next()
		{
			char* p = syd_reinterpret_cast<char*>(this);
			p += getSize();
			return syd_reinterpret_cast<Feature*>(p);
		}

		// ダンプする
		static char* dump(char* p,
						  const FeatureElement& feature_)
		{
			Feature* f = syd_reinterpret_cast<Feature*>(p);
			f->m_fWeight = static_cast<float>(feature_.scale);
			const ModUnicodeString& s = feature_.term;
			f->m_usLength = static_cast<unsigned short>(s.getLength());
			Os::Memory::copy(f->m_pBuffer, s.operator const ModUnicodeChar*(),
							 f->m_usLength * sizeof(ModUnicodeChar));
			return p + getSize(f->m_usLength);
		}
	};
	
	// コンストラクタ
	FeatureSet() {}
	// デストラクタ
	~FeatureSet() {}

	// 内積を得る
	float innerProduct(const FeatureSet& other_) const;
	// 正規化する
	void normalize();
	// 特徴語数を得る
	ModSize getCount() const { return m_uiCount; }
	// 全体のサイズを得る(byte)
	ModSize getSize() const;

	// 空の特徴語列を設定する
	void clear() { m_uiCount = 0; }

	// コピーする
	//【注意】
	//	コピーするサイズ分の容量は確保されていることが前提
	void copy(const FeatureSet* pFeatureSet_)
	{
		ModSize size = pFeatureSet_->getSize();
		Os::Memory::copy(this, pFeatureSet_, size);
	}

	// 先頭の特徴語を得る
	const Feature* first() const { return m_pFeature; }
	
	// サイズを得る
	static ModSize getSize(const FeatureList& feature_,
						   ModSize maxSize_ = 0);
	// ダンプする
	static void dump(char* buf,
					 const FeatureList& feature_,
					 ModSize maxSize_ = 0);

private:
	// 特徴語数
	ModUInt32	m_uiCount;
	// 特徴語
	Feature		m_pFeature[1];
};

//
//	CLASS
//	FullText2::FeatureSetPointer -- FeatureSetのポインターを管理するクラス
//
//	NOTES
//
class FeatureSetPointer
{
public:
	// コンストラクタ
	FeatureSetPointer(FeatureSet* pFeatureSet_ = 0)
		: m_pFeatureSet(pFeatureSet_),
		  m_bOwner(pFeatureSet_ == 0 ? false : true) {}
	// デストラクタ
	~FeatureSetPointer()
		{ this->free(); }
	// コピーコンストラクタ
	FeatureSetPointer(const FeatureSetPointer& r_)
		{
			m_bOwner = r_.isOwner();
			m_pFeatureSet = const_cast<FeatureSetPointer&>(r_).release();
		}

	// 代入演算子
	FeatureSetPointer& operator = (const FeatureSetPointer& r_)
		{
			if (get() != r_.get() || r_.isOwner())
			{
				this->free();
				m_bOwner = r_.isOwner();
				m_pFeatureSet = r_.release();
			}
			return *this;
		}
	FeatureSetPointer& operator = (FeatureSet* p_)
		{
			this->free();
			m_bOwner = true;
			m_pFeatureSet = p_;
			return *this;
		}

	// 参照
	FeatureSet& operator * () const
		{ return *get(); }
	FeatureSet* operator -> () const
		{ return get(); }

	// アドレス取得
	FeatureSet* get() const
		{ return const_cast<FeatureSet*>(m_pFeatureSet); }
	
	// 所有権放棄
	FeatureSet* release() const
		{
			m_bOwner = false;
			return const_cast<FeatureSet*>(m_pFeatureSet);
		}

	// 所有権の有無
	bool isOwner() const
		{ return m_bOwner; }

	// 領域を確保して内容をコピーする
	void copy(const FeatureSet* p_);

	// メモリを開放する
	void free();
	
private:
	FeatureSet*		m_pFeatureSet;	// ポインタ
	mutable bool	m_bOwner;		// 所有権
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_FEATURESET_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
