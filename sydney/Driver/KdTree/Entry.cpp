// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Entry.cpp --
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
#include "KdTree/Entry.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
}

//
//	FUNCTION public
//	KdTree::Entry::getSize -- 全体のバイト数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		全体のバイト数
//
//	EXCEPTIONS
//
int
Entry::getSize() const
{
	return calcSize(m_iDimensionSize);
}

//
//	FUNCTION public
//	KdTree::Entry::operator const char* ()
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const char*
//		先頭のアドレス
//
//	EXCEPTIONS
//
Entry::operator const char* () const
{
	return syd_reinterpret_cast<const char*>(this);
}

//
//	FUNCTION public
//	KdTree::Entry::operator char* ()
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	char*
//		先頭のアドレス
//
//	EXCEPTIONS
//
Entry::operator char* ()
{
	return syd_reinterpret_cast<char*>(this);
}

//
//	FUNCTION public static
//	KdTree::Entry::calcSize -- サイズを求める
//
//	NOTES
//
//	ARGUMENTS
//	int dsize_
//		次元数
//
//	RETURN
//	int
//		サイズ
//
//	EXCEPTIONS
//
int
Entry::calcSize(int dsize_)
{
	// 8バイト境界にする
	
	return (sizeof(ModUInt32) + sizeof(int) + sizeof(float) * dsize_ + 7)
		/ 8 * 8;
}

//
//	FUNCTION public
//	KdTree::Entry::calcDistance -- 各次元の差の二乗の和(この√が距離)
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::Entry* p
//		距離計算の対象
//
//	RETURN
//	double
//		距離の二乗
//
//	EXCEPTIONS
//
double
Entry::calcDistance(const Entry* p) const
{
	double d = 0.0;
	const float* v1 = m_pValue;
	const float* v2 = p->m_pValue;
	int dim = m_iDimensionSize;

	for (int i = 0; i < dim; ++i, ++v1, ++v2)
	{
		d += (*v1 - *v2) * (*v1 - *v2);
	}
	
	return d;
}

//
//	FUNCTION public
//	KdTree::Entry::getMaxDifferenceDimension
//		-- 一番差の大きな次元を求める
//
//	NOTES
//
//	ARGUMENTS
// 	const KdTree::Entry* p
//		対象
//
//	RETURN
//	int
//		次元
//
//	EXCEPTIONS
//
int
Entry::getMaxDifferenceDimension(const Entry* p) const
{
	float max = 0.0;
	int maxDim = -1;
	const float* v1 = m_pValue;
	const float* v2 = p->m_pValue;
	int dim = m_iDimensionSize;
	
	for (int i = 0; i < dim; ++i, ++v1, ++v2)
	{
		float d = (*v1 - *v2) * (*v1 - *v2);
		if (d > max)
		{
			max = d;
			maxDim = i;
		}
	}

	return maxDim;
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
