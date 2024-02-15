// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FeatureSet.cpp --
// 
// Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Inverted/FeatureSet.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

namespace
{
}

//
//	FUNCTION public
//	Inverted::FeatureSet::innerProduct -- 内積を求める
//
//	NOTES
//
//	ARGUMENTS
//	const Inverted::FeatureSet& other_
//		引数
//
//	RETURN
//	float
//		内積値
//
//	EXCEPTIONS
//
float
FeatureSet::innerProduct(const FeatureSet& other_) const
{
	//【注意】
	//	特徴語列はすべて特徴語文字列の昇順に並んでいるものとする

	float v = 0.0;
	const Feature* s = m_pFeature;
	const Feature* o = other_.m_pFeature;

	ModUInt32 si = 0;
	ModUInt32 oi = 0;
	
	while (si < m_uiCount && oi < other_.m_uiCount)
	{
		int c = s->compareTo(*o);
		if (c == 0)
		{
			// 同じ特徴語なので、内積を求める
			v += (s->m_fWeight * o->m_fWeight);

			s = s->next();
			o = o->next();
			++si;
			++oi;
		}
		else if (c < 0)
		{
			// 自身の方が小さいので自分だけを次にする
			s = s->next();
			++si;
		}
		else
		{
			// 相手の方が小さいので相手だけを次にする
			o = o->next();
			++oi;
		}
	}

	return v;
}

//
//	FUNCTION public
//	Inverted::FeatureSet::normalize -- 特徴量を正規化する
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
FeatureSet::normalize()
{
	double norm = 0.0;
	
	Feature* s = m_pFeature;
	ModUInt32 i = 0;
	for (; i < m_uiCount; s = s->next(), ++i)
	{
		norm += (static_cast<double>(s->m_fWeight)
				 * static_cast<double>(s->m_fWeight));
	}

	norm = ModOsDriver::sqrt(norm);

	if (norm != 0.0)
	{
		s = m_pFeature;
		for (i = 0; i < m_uiCount; s = s->next(), ++i)
		{
			s->m_fWeight /= static_cast<float>(norm);
		}
	}
}

//
//	FUNCTION public
//	Inverted::FeatureSet::getSize -- 全体のサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		全体のサイズ
//
//	EXCEPTIONS
//
ModSize
FeatureSet::getSize() const
{
	ModSize s = 0;
	
	ModUInt32 i = 0;
	const Feature* p = m_pFeature;

	for (; i < m_uiCount; p = p->next(), ++i)
	{
		s += p->getSize();
	}

	s += sizeof(ModUInt32);	// m_uiCount分

	return s;
}

//
//	FUNCTION public
//	Inverted::FeatureSet::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModInvertedFeatureList& feature_
//		特徴語配列
//	ModSize maxSize_
//		最大長(default 0)
//
//	RETURN
//	ModSize
//		求めたサイズ
//
//	EXCEPTIONS
//
//static
ModSize
FeatureSet::getSize(const ModInvertedFeatureList& feature_,
					ModSize maxSize_)
{
	ModSize s = sizeof(ModUInt32);	// m_uiCount分
	
	ModInvertedFeatureList::ConstIterator i = feature_.begin();
	ModInvertedFeatureList::ConstIterator e = feature_.end();
	for (; i != e; ++i)
	{
		ModSize ss = Feature::getSize((*i).first.getLength());
		if (maxSize_ != 0 && (s + ss) > maxSize_)
			break;
		s += ss;
	}

	return s;
}

//
//	FUNCTION public
//	Inverted::FeatureSet::dump -- メモリにダンプする
//
//	NOTES
//
//	ARGUMENTS
//	char* buf_
//		ダンプするメモリ
//	const ModInvertedFeatureList& feature_
//		ダンプする特徴語配列
//	ModSize maxSize_
//		最大サイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
//static
void
FeatureSet::dump(char* buf_,
				 const ModInvertedFeatureList& feature_,
				 ModSize maxSize_)
{
	// maxSize_分のところまでダンプする
	
	ModInvertedFeatureMap fmap;
	ModSize size = sizeof(ModUInt32);
	FeatureSet* f = syd_reinterpret_cast<FeatureSet*>(buf_);
	f->m_uiCount = 0;

	ModInvertedFeatureList::ConstIterator i = feature_.begin();
	ModInvertedFeatureList::ConstIterator e = feature_.end();
	for (; i != e; ++i)
	{
		ModSize s = Feature::getSize((*i).first.getLength());
		if (maxSize_ != 0 && (size + s) > maxSize_)
			break;

		// 特徴語の昇順に格納するために、マップに登録する
		fmap.insert(*i);
		size += s;
		++f->m_uiCount;
	}

	char* p = buf_ + sizeof(ModUInt32);	// m_uiCount分
	
	ModInvertedFeatureMap::Iterator ii = fmap.begin();
	ModInvertedFeatureMap::Iterator ee = fmap.end();
	for (; ii != ee; ++ii)
	{
		// ダンプする
		p = Feature::dump(p, *ii);
	}
}

//
//	FUNCTION public
//	Inverted::FeatureSetPointer::copy -- 領域を確保して内容をコピーする
//
//	NOTES
//
//	ARGUMENTS
//	const FeatureSet* p_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FeatureSetPointer::copy(const FeatureSet* p_)
{
	free();
	
	if (p_ != 0)
	{
		m_pFeatureSet = (FeatureSet*)operator new (p_->getSize());
		m_pFeatureSet->copy(p_);
		m_bOwner = true;
	}
}

//
//	FUNCTION public
//	Inverted::FeatureSetPointer::free -- メモリを開放する
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
FeatureSetPointer::free()
{
	if (isOwner())
	{
		operator delete (m_pFeatureSet);
		m_pFeatureSet = 0;
		m_bOwner = false;
	}
}

//
//	Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

