// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CalcVariance.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "KdTree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "KdTree/CalcVariance.h"

#include "KdTree/Entry.h"

#include "Os/AutoCriticalSection.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
}

//
//	FUNCTION public
//	KdTree::CalcVariance::CalcVariance -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Common::LargeVector<KdTree::Entry*>::ConstIterator& b_
//		計算範囲の先頭
//	const Common::LargeVector<KdTree::Entry*>::ConstIterator& e_
//		計算範囲の末尾
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
CalcVariance::
CalcVariance(const Common::LargeVector<KdTree::Entry*>::ConstIterator& b_,
			 const Common::LargeVector<KdTree::Entry*>::ConstIterator& e_)
	: m_b(b_), m_e(e_), m_iDimensionSize(0)
{
	m_iCount = (m_e - m_b);
	if (m_iCount)
		m_iDimensionSize = (*m_b)->getDimensionSize();
}

//
//	FUCNTION public
//	KdTree::CalcVariance::~CalcVariance -- デストラクタ
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
CalcVariance::~CalcVariance()
{
}

//
//	FUNCTION public
//	KdTree::CalcVariance::prepare -- 準備する
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
CalcVariance::prepare()
{
	m_vecData.assign(m_iDimensionSize, ModPair<double, double>(0.0, 0.0));
}

//
//	FUNCTION public
//	KdTree::CalcVariance::parallel -- 並列実行
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
CalcVariance::parallel()
{
	// 自スレッド分の計算範囲を得る
	Common::LargeVector<Entry*>::ConstIterator b;
	Common::LargeVector<Entry*>::ConstIterator e;
	getRange(b, e);

	// 次元分のデータ領域を確保する
	ModVector<ModPair<double, double> > vecData;
	vecData.assign(m_iDimensionSize, ModPair<double, double>(0.0, 0.0));

	Common::LargeVector<Entry*>::ConstIterator i = b;
	for (; i != e; ++i)
	{
		int d = 0;
		ModVector<ModPair<double, double> >::Iterator j = vecData.begin();
		for (; j != vecData.end(); ++j, ++d)
		{
			float v = (*i)->getValue(d);

			// 二乗の和
			(*j).first += (v * v);
			// 和
			(*j).second += v;
		}
	}

	// 結果を設定する
	setVariance(vecData);
}

//
//	FUNCTION public
//	KdTree::CalcVariance::dispose -- 後処理
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
CalcVariance::dispose()
{
	// 最大値
	double m = 0.0;

	int d = 0;
	ModVector<ModPair<double, double> >::Iterator i = m_vecData.begin();
	for (; i != m_vecData.end(); ++i, ++d)
	{
		// 分散は、(平均値 - 値) ^ 2 の平均であるので、
		// 二乗の平均値 - 平均値の二乗となる

		double a1 = (*i).first / m_iCount;
		double a2 = (*i).second / m_iCount;

		double s = a1 - (a2 * a2);

		if (s > m)
		{
			m_iMaxDimension = d;
			m = s;
		}
	}
}

//
//	FUNCTION private
//	KdTree::CalcVariance::getRange -- 自スレッド分の計算対象範囲を得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::LargeVector<KdTree::Entry*>::ConstIterator& b_
//		自スレッド分の計算範囲の先頭
//	Common::LargeVector<KdTree::Entry*>::ConstIterator& e_
//		自スレッド分の計算範囲の終端
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CalcVariance::getRange(Common::LargeVector<Entry*>::ConstIterator& b_,
					   Common::LargeVector<Entry*>::ConstIterator& e_)
{
	// スレッド数
	int num = getThreadSize();
	// 自スレッドのスレッド番号(0からの連番)
	int t = getThreadNum();

	b_ = m_b;
	e_ = m_b;

	// スレッド数で等分する
	// ただし、1スレッドの割り当てが10以上になるように割り当てる

	int n = (m_iCount + (num - 1)) / num;	// 切り上げる
	if (n < 10) n = 10;	// 10以上にする
	
	b_ += (t * n) > m_iCount ? m_iCount : (t * n);
	e_ += ((t + 1) * n) > m_iCount ? m_iCount : ((t + 1) * n);
}

//
//	FUNCTION public
//	KdTree::CalcVariance::setVariance -- 計算結果を登録する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<ModPair<double, double> >& vecSubData_
//		1スレッド分のデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CalcVariance::
setVariance(const ModVector<ModPair<double, double> >& vecSubData_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	ModVector<ModPair<double, double> >::Iterator i = m_vecData.begin();
	ModVector<ModPair<double, double> >::ConstIterator j = vecSubData_.begin();
	for (; i != m_vecData.end(); ++i, ++j)
	{
		(*i).first += (*j).first;
		(*i).second += (*j).second;
	}
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
