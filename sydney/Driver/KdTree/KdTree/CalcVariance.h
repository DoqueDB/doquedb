// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CalcVariance.h -- 
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

#ifndef __SYDNEY_KDTREE_CALCVARIANCE_H
#define __SYDNEY_KDTREE_CALCVARIANCE_H

#include "KdTree/Module.h"
#include "Utility/OpenMP.h"

#include "Common/LargeVector.h"

#include "ModVector.h"
#include "ModPair.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

class Entry;

//
//	CLASS
//	KdTree::CalcVariance -- 分散のもっとも大きい次元を求める
//
//	NOTES
//
class CalcVariance : public Utility::OpenMP
{
public:
	// コンストラクタ
	CalcVariance(const Common::LargeVector<Entry*>::ConstIterator& b_,
				 const Common::LargeVector<Entry*>::ConstIterator& e_);
	// デストラクタ
	virtual ~CalcVariance();

	// もっとも分散値の大きな次元を得る
	int getMaxVarianceDimension() { return m_iMaxDimension; }

	// 準備
	void prepare();
	// 並列実行
	void parallel();
	// 後処理
	void dispose();

private:
	// 自スレッド分の計算対象範囲を得る
	void getRange(Common::LargeVector<Entry*>::ConstIterator& b_,
				  Common::LargeVector<Entry*>::ConstIterator& e_);

	// 計算結果を登録する
	void setVariance(const ModVector<ModPair<double, double> >& vecSubData_);
	
	// エントリ
	Common::LargeVector<Entry*>::ConstIterator m_b;
	Common::LargeVector<Entry*>::ConstIterator m_e;
	
	// 次元数
	int m_iDimensionSize;
	// 計算対象数
	int m_iCount;
	
	// 計算結果
	ModVector<ModPair<double, double> > m_vecData;
	// 最も分散の大きな次元
	int m_iMaxDimension;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_CALCVARIANCE_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
