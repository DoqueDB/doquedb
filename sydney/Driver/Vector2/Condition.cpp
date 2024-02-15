// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Condition.cpp -- 
// 
// Copyright (c) 2005, 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Vector2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Vector2/Condition.h"
#include "Vector2/Types.h"

#include "Common/Data.h"
#include "Common/DataType.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/IntegerData.h"
#include "Common/Integer64Data.h"
#include "Common/DoubleData.h"

#include "LogicalFile/TreeNodeInterface.h"

#include "Exception/Unexpected.h"

#include "ModAlgorithm.h"

_SYDNEY_BEGIN
_SYDNEY_VECTOR2_BEGIN

//
//	FUNCTION public
//	Vector2::Condition::Condition -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		木構造の検索条件オブジェクトへのポインタ
//	LogicalFile::TreeNodeInterface::Type iParent_
//		親ノードのタイプ
//	int iHierarchy_
//		Rootからの段数
//
//	RETURN
//
//	EXCEPTIONS
//
Condition::Condition(const LogicalFile::TreeNodeInterface* pCondition_,
					 LogicalFile::TreeNodeInterface::Type iParent_,
					 int iHierarchy_)
	: m_pNode(pCondition_), m_iParent(iParent_), m_iHierarchy(iHierarchy_), 
	  m_bSimple(true), m_bAll(false), m_bZero(false),
	  m_uiMin(IllegalKey), m_uiMax(IllegalKey)
{
	// m_vecMin, m_vecMaxの初期化はコストがかかるので、
	// 必要になるまで初期化しない。
}

//
//	FUNCTION public
//	Vector2::Condition::~Condition -- デストラクタ
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
Condition::~Condition()
{
}

//
//	FUNCTION public
//	Vector2::Condition::parseRoot -- 検索条件を解析する
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	bool
//		与えられた検索条件が解析できればtrue
//		できなければfalse
//
//	EXCEPTIONS
//
bool
Condition::parseRoot()
{
	using namespace LogicalFile;

	bool result = false;
	if (isOperatorLeaf(m_pNode))
		result = parseOperatorLeaf(m_pNode);
	else if (m_pNode->getType() == TreeNodeInterface::And)
		result = parseAndNode();
	else if (m_pNode->getType() == TreeNodeInterface::Or)
		result = parseOrNode();

	return result;
}

//
//	FUNCTION public
//	Vector2::Condition::getConditionSize -- 解析された検索条件の数を返す
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	int
//		検索条件の数
//
//	EXCEPTIONS
//
int
Condition::getConditionSize() const
{
	if (m_bSimple)
		return 1;
	else
	{
		if (m_vecMin.getSize() != m_vecMax.getSize())
			_SYDNEY_THROW0(Exception::Unexpected);
		
		return static_cast<int>(m_vecMin.getSize());
	}
}

//
//	FUNCTION public
//	Vector2::Condition::getMin -- 解析結果の最小値を取得する
//
//	NOTES
//
//	ARGUMENTS
//	int pos_
//		値の位置
//
//	RETURN
//	ModUInt32
//		最小値
//
//	EXCEPTIONS
//
ModUInt32
Condition::getMin(int pos_) const
{
	if (m_bSimple)
		return m_uiMin;
	else
		return m_vecMin.at(static_cast<ModSize>(pos_));
}

//
//	FUNCTION public
//	Vector2::Condition::getMax -- 解析結果の最大値を取得する
//
//	NOTES
//
//	ARGUMENTS
//	int pos_
//		値の位置
//
//	RETURN
//	ModUInt32
//		最大値
//
//	EXCEPTIONS
//
ModUInt32
Condition::getMax(int pos_) const
{
	if (m_bSimple)
		return m_uiMax;
	else
		return m_vecMax.at(static_cast<ModSize>(pos_));
}

//
//	FUNCTION private
//	Vector2::Condition::parseOrNode -- OR ノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	bool
//		与えられた検索条件が解析できればtrue
//		できなければfalse
//
//	EXCEPTIONS
//
bool
Condition::parseOrNode()
{
	using namespace LogicalFile;

	const int size = static_cast<int>(m_pNode->getOperandSize());
	if (size == 0)
		return false;
	
	for (int n = 0; n != size; ++n)
	{
		// 各ノードを解析する

		const TreeNodeInterface* pChild = m_pNode->getOperandAt(n);

		if (isOperatorLeaf(pChild))
		{
			if (parseOperatorLeaf(pChild) == false)
				return false;
		}
		else if (pChild->getType() == TreeNodeInterface::And)
		{
			if (m_iHierarchy == 2)
				// AND/ORノードは2階層まで
				return false;

			Condition cAndNode(pChild, TreeNodeInterface::Or, m_iHierarchy + 1);
			if (cAndNode.parseAndNode() == false)
				return false;

			// マージする
			mergeOrCondition(cAndNode.getMin(), cAndNode.getMax());
		}
		else
			// ORノードからORノードは呼べない。
			return false;

		if (m_iParent == TreeNodeInterface::Undefined && m_bAll)
			// 全件取得はVectorで取得するよりRecordから直接の方が速い。
			return false;
	}

	checkCondition();
	
	return true;
}

//
//	FUNCTION private
//	Vector2::Condition::parseAndNode -- AND ノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	bool
//		与えられた検索条件が解析できればtrue
//		できなければfalse
//
//	EXCEPTIONS
//
bool
Condition::parseAndNode()
{
	using namespace LogicalFile;
	
	const int size = static_cast<int>(m_pNode->getOperandSize());
	if (size == 0)
		return false;

	// 条件の初期化
	// ANDノードは、全件取得状態からマージごとに範囲を狭くする。
	// ちなみに、リーフとORノードは、0件取得状態からマージごとに広くする。
	m_uiMin = 0;
	m_uiMax = IllegalKey;
	
	for (int n = 0; n != size; ++n)
	{
		// 各ノードを解析する
		
		const TreeNodeInterface* pChild = m_pNode->getOperandAt(n);
		
		if (isOperatorLeaf(pChild))
		{
			if (parseOperatorLeaf(pChild) == false)
				return false;
		}
		else if (pChild->getType() == TreeNodeInterface::Or)
		{
			if (m_iHierarchy == 2)
				// AND/ORノードは2階層まで
				return false;

			Condition cOrNode(pChild, TreeNodeInterface::And, m_iHierarchy + 1);
			if (cOrNode.parseOrNode() == false)
				return false;

			// 解析中のANDノードと今解析したORノードは、
			// 両方とも複数条件になる場合がある。
			// また、AND条件でマージするため、
			// 条件を一つずつ取得してマージすることもできない。
			// なので、Conditionオブジェクトごと渡す。
			mergeAndCondition(cOrNode);
		}
		else
			// ANDノードからANDノードは呼べない。
			return false;
	}

	checkCondition();

	if (m_bSimple && m_uiMin ==0 && m_uiMax == IllegalKey)
		// 全件取得はRecordから直接取得した方が速い。
		return false;

	return true;
}

//
//	FUNCTION private
//	Vector2::Condition::parseOperatorLeaf -- 関係演算子リーフを解析してマージする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*	pLeaf_
//		検索条件
//
//	RETURN
//	bool
//		与えられた検索条件が解析できればtrue
//		できなければfalse
//
//	EXCEPTIONS
//
bool
Condition::parseOperatorLeaf(const LogicalFile::TreeNodeInterface* pLeaf_)
{
	using namespace LogicalFile;

	if (pLeaf_->getOperandSize() != 2)
		return false;

	TreeNodeInterface::Type eMatch = pLeaf_->getType();
	const TreeNodeInterface* pField = pLeaf_->getOperandAt(0);
	const TreeNodeInterface* pValue	= pLeaf_->getOperandAt(1);

	if (pValue->getType() == TreeNodeInterface::Field &&
		(pField->getType() == TreeNodeInterface::ConstantValue ||
		 pField->getType() == TreeNodeInterface::Variable))
	{
		// Value was set to pField and Field was set to pValue.
		// These data is set to pCondition_ in order of SQL's string.
		alternateTerm(pField, pValue, eMatch);
	}
	else if (pField->getType() != TreeNodeInterface::Field ||
			 (pValue->getType() != TreeNodeInterface::ConstantValue &&
			  pValue->getType() != TreeNodeInterface::Variable))
	{
		return false;
	}

	// Check Field
	if (ModUnicodeCharTrait::toInt(pField->getValue()) != 0)
	{
		// Fieldからintを直接取得できないので、getValueを経由する。
		// キーフィールド以外は検索できない
		return false;
	}

	// Check Value
	const Common::Data* pData = pValue->getData();
	if (pData == 0)
	{
		// ROWID->ObjectID変換で使ってない

		int i = ModUnicodeCharTrait::toInt(pValue->getValue());
		if (i >= 0 && pLeaf_->getType() == TreeNodeInterface::Equals)
		{
			// Schemaで使っている
			m_uiMin = static_cast<ModUInt32>(i);
			m_uiMax = static_cast<ModUInt32>(i);
			return true;
		}
		return false;
	}

	// 条件を解析する
	ModInt64 min = IllegalKey;
	ModInt64 max = IllegalKey;
	if (pData->getType() == Common::DataType::Null)
	{
		// [YET] switch文がNull以外の場合と二つにわかれている。
		//  しかし、まとめると修正範囲が多くなってしまう。
		switch (eMatch)
		{
		case TreeNodeInterface::Equals:
		case TreeNodeInterface::GreaterThan:
		case TreeNodeInterface::GreaterThanEquals:
		case TreeNodeInterface::LessThan:
		case TreeNodeInterface::LessThanEquals:
			// nullとの比較は0件ヒット
			break;
		default:
			// NotEqualsはサポートしない
			return false;
		}
	}
	else
	{
		bool integer = false;
		ModInt64 v = round(*pData, integer);
		
		switch (eMatch)
		{
		case TreeNodeInterface::Equals:
			if (v == -1 || integer == false)
				v = IllegalKey;
			min = v;
			max = v;
			break;
		case TreeNodeInterface::GreaterThan:
			min = (v == IllegalKey) ? v : v + 1;
			max = IllegalKey;
			break;
		case TreeNodeInterface::GreaterThanEquals:
			min = (v == IllegalKey || integer && v != -1) ? v : v + 1;
			max = IllegalKey;
			break;
		case TreeNodeInterface::LessThan:
			if (integer)
				--v;
			if (v < 0)
			{
				min = IllegalKey;
				max = IllegalKey;
			}
			else
			{
				min = 0;
				max = (v == IllegalKey - 1) ? IllegalKey : v;
			}
			break;
		case TreeNodeInterface::LessThanEquals:
			if (v == -1)
			{
				min = IllegalKey;
				max = IllegalKey;
			}
			else
			{
				min = 0;
				max = (v == IllegalKey - 1) ? IllegalKey : v;
			}
			break;
		default:
			// NotEqualsはサポートしない
			return false;
		}
	}
	
	// 条件をマージする

	const ModUInt32 uiMin = static_cast<ModUInt32>(min);
	const ModUInt32 uiMax = static_cast<ModUInt32>(max);	
	const int type = m_pNode->getType();
	if (type == TreeNodeInterface::And)
		// ANDノードの場合
		mergeAndCondition(uiMin, uiMax);
	else if (type == TreeNodeInterface::Or)
		// ORノードの場合
		mergeOrCondition(uiMin, uiMax);
	else
	{
		// ROOTの場合
		if (uiMin == 0 && uiMax == IllegalKey)
			// 全件取得条件
			return false;
		m_uiMin = uiMin;
		m_uiMax = uiMax;
	}

	return true;
}

//
//	FUNCTION private
//	Vector2::Condition::alternateTerm -- Alternate two terms.
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pFirst_
//		First term
//	const LogicalFile::TreeNodeInterface* pSecond_
//		Second term
//	LogicalFile::TreeNodeInterface::Type& eMatch_
//		Comparison operator
//
//	RETURN
//
//	EXCEPTIONS
//
void
Condition::alternateTerm(const LogicalFile::TreeNodeInterface*& pFirst_,
						 const LogicalFile::TreeNodeInterface*& pSecond_,
						 LogicalFile::TreeNodeInterface::Type& eMatch_)
{
	using namespace LogicalFile;
	
	const TreeNodeInterface* p = pFirst_;
	pFirst_ = pSecond_;
	pSecond_ = p;

	switch (eMatch_)
	{
	case TreeNodeInterface::Equals:
		break;
	case TreeNodeInterface::GreaterThan:
		eMatch_ = TreeNodeInterface::LessThan;
		break;
	case TreeNodeInterface::GreaterThanEquals:
		eMatch_ = TreeNodeInterface::LessThanEquals;
		break;
	case TreeNodeInterface::LessThan:
		eMatch_ = TreeNodeInterface::GreaterThan;
		break;
	case TreeNodeInterface::LessThanEquals:
		eMatch_ = TreeNodeInterface::GreaterThanEquals;
		break;
	default:
		break;
	}
}

//
//	FUNCTION private
//	Vector2::Condition::mergeOrCondition -- OR条件でマージする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 min
//		新しい条件の下限の値
//	ModUInt32 max
//		新しい条件の上限の値
//
//	RETURN
//
//	EXCEPTIONS
//
void
Condition::mergeOrCondition(ModUInt32 min,
							ModUInt32 max)
{
	if (m_bAll)
		// 全件取得条件がすでにある。
		return;

	if (max == IllegalKey)
	{
		if (min == 0)
		{
			// 全件取得条件
			m_bAll = true;
			m_bSimple = true;
			m_uiMin = 0;
			m_uiMax = IllegalKey;
			return;
		}
		else if (min == IllegalKey)
			// 0件取得条件
			return;
	}


	// マージする
	if (m_bSimple)
	{
		if (m_uiMin == IllegalKey)
		{
			// 今までの条件は0件取得条件だった。
			m_uiMin = min;
			m_uiMax = max;
		}
		else
		{
			if (m_uiMin > min)
			{
				// 新条件の最小値の方が小さい
				if (m_uiMin-1 <= max)
				{
					// 重なった
					m_uiMin = min;
					m_uiMax = (m_uiMax >= max) ? m_uiMax : max;
				}
				else
				{
					// 重ならなかった
					m_bSimple = false;
					m_vecMin.pushBack(min);
					m_vecMax.pushBack(max);
					m_vecMin.pushBack(m_uiMin);
					m_vecMax.pushBack(m_uiMax);
				}
			}
			else
			{
				// 今までの条件の最小値の方が小さいか同じ
				if (m_uiMax == IllegalKey || min <= m_uiMax+1)
					// 重なった
					m_uiMax = (m_uiMax >= max) ? m_uiMax : max;
				else
				{
					// 重ならなかった
					m_bSimple = false;
					m_vecMin.pushBack(m_uiMin);
					m_vecMax.pushBack(m_uiMax);
					m_vecMin.pushBack(min);
					m_vecMax.pushBack(max);
				}
			}
		}

		if (m_bSimple && m_uiMin == 0 && m_uiMax == IllegalKey)
			// 全件取得条件になった
			m_bAll = true;
	}
	else
	{
		// 2分探索で、新しい条件と重なる条件を探す。
	
		const ModUInt32 size = m_vecMin.getSize();
		if (size != m_vecMax.getSize())
			_SYDNEY_THROW0(Exception::Unexpected);
		bool lower = false;
		bool upper = false;
		ModUInt32 i = binsearch(min, lower);
		// minが最大条件より大きいのでmaxも明らかに大きい。
		const ModUInt32 j = (i == size) ? i : binsearch(max, upper, i, size-1);

		if (lower == false && i != 0 && min == m_vecMax.at(i-1) + 1)
		{
			// m_vecMax.at(i-1)は最大条件の上限ではないので
			// m_vecMax.at(i-1) + 1 で IllegalKey + 1 を計算することはない。

			// minは条件i-1に含まれていないが、隣接するので含まれることにする
			--i;
			lower = true;
		}

		if (upper == false && j != size && max == m_vecMin.at(j) - 1)
		{
			// m_vecMin.at(j)が0なら必ずupper==trueになるので
			// m_vecMin.at(j) - 1 で 0 - 1 を計算することはない。

			// maxは条件jに含まれていないが、隣接するので含まれることにする
			upper = true;
		}
		

		// 条件をマージする
		// upper=trueの時 j<size なので、イテレータにj+1を使っても大丈夫
		if (lower)
		{
			if (upper)
			{
				// 条件iと条件jをマージする場合

				m_vecMin.erase(m_vecMin.begin()+i+1,m_vecMin.begin()+j+1);
				m_vecMax.erase(m_vecMax.begin()+i,m_vecMax.begin()+j);
			}
			else
			{
				// 条件iの上限を更新する場合

				if (i == j)
					_SYDNEY_THROW0(Exception::Unexpected);

				m_vecMin.erase(m_vecMin.begin()+i+1,m_vecMin.begin()+j);
				m_vecMax.erase(m_vecMax.begin()+i,m_vecMax.begin()+j);
				m_vecMax.insert(m_vecMax.begin()+i, max);
			}
		}
		else
		{
			if (upper)
			{
				// 条件jの下限を更新する場合
			
				m_vecMin.erase(m_vecMin.begin()+i,m_vecMin.begin()+j+1);
				m_vecMin.insert(m_vecMin.begin()+i, min);
				m_vecMax.erase(m_vecMax.begin()+i,m_vecMax.begin()+j);
			}
			else
			{
				// 条件iの上限と下限を更新する場合

				m_vecMin.erase(m_vecMin.begin()+i,m_vecMin.begin()+j);
				m_vecMin.insert(m_vecMin.begin()+i, min);
				m_vecMax.erase(m_vecMax.begin()+i,m_vecMax.begin()+j);
				m_vecMax.insert(m_vecMax.begin()+i, max);
			}
		}

		if (m_vecMin.getSize() == 1 && m_vecMax.getSize() == 1 &&
			m_vecMin.at(0) == 0 && m_vecMax.at(0) == IllegalKey)
		{
			// 全件取得条件になった
			m_bAll = true;
			m_bSimple = true;
			m_uiMin = 0;
			m_uiMax = IllegalKey;
		}
	}
}

//
//	FUNCTION private
//	Vector2::Condition::mergeAndCondition -- AND条件でマージする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 min
//		新しい条件の下限の値
//	ModUInt32 max
//		新しい条件の上限の値
//
//	RETURN
//
//	EXCEPTIONS
//
void
Condition::mergeAndCondition(ModUInt32 min,
							 ModUInt32 max)
{
	if (m_bZero)
		// 0件取得条件がすでにある、
		return;

	if (max == IllegalKey)
	{
		if (min == 0)
			// 全件取得条件
			return;
		else if (min == IllegalKey)
		{
			// 0件取得条件
			m_bZero = true;
			m_bSimple = true;
			m_uiMin = IllegalKey;
			m_uiMax = IllegalKey;
			return;
		}
	}
	

	// マージする
	if (m_bSimple)
	{
		m_uiMin = (min > m_uiMin) ? min : m_uiMin;
		m_uiMax = (max < m_uiMax) ? max : m_uiMax;

		if (m_uiMin > m_uiMax)
		{
			// 0件取得条件になった
			m_bZero = true;
			m_uiMin = IllegalKey;
			m_uiMax = IllegalKey;
		}
	}
	else
	{
		const ModUInt32 size = m_vecMin.getSize();
		if (size != m_vecMax.getSize())
			_SYDNEY_THROW0(Exception::Unexpected);
		bool lower = false;
		bool upper = false;
		const ModUInt32 i = binsearch(min, lower);
		// minが最大条件より大きいのでmaxも明らかに大きい。
		const ModUInt32 j = (i == size) ? i : binsearch(max, upper, i, size-1);

		// まずはupper
		if (upper)
		{
			// upper==trueのためj<sizeなので、j+1<=sizeも保証される
			m_vecMin.erase(m_vecMin.begin()+j+1, m_vecMin.end());
			m_vecMax.erase(m_vecMax.begin()+j, m_vecMax.end());
			m_vecMax.insert(m_vecMax.begin()+j, max);
		}
		else
		{
			m_vecMin.erase(m_vecMin.begin()+j, m_vecMin.end());
			m_vecMax.erase(m_vecMax.begin()+j, m_vecMax.end());
		}

		// 次はlower
		if (lower)
		{
			// lower==trueのためi<sizeなので、i+1<=sizeも保証される
			m_vecMin.erase(m_vecMin.begin(), m_vecMin.begin()+i+1);
			m_vecMin.pushFront(min);
			m_vecMax.erase(m_vecMax.begin(), m_vecMax.begin()+i);
		}
		else
		{
			m_vecMin.erase(m_vecMin.begin(), m_vecMin.begin()+i);
			m_vecMax.erase(m_vecMax.begin(), m_vecMax.begin()+i);
		}

		if (m_vecMin.getSize() == 0)
		{
			if (m_vecMax.getSize() != 0)
				_SYDNEY_THROW0(Exception::Unexpected);

			// 0件取得条件になった
			m_bZero = true;
			m_bSimple = true;
			m_uiMin = IllegalKey;
			m_uiMax = IllegalKey;
		}
	}
}

//
//	FUNCTION private
//	Vector2::Condition::mergeAndCondition
//		-- 条件数がわからない条件をAND条件でマージする
//
//	NOTES
//
//	ARGUMENTS
//	Condition& other_
//		マージする条件
//
//	RETURN
//
//	EXCEPTIONS
//
void
Condition::mergeAndCondition(Condition& other_)
{
	if (m_bZero)
		// 0件取得条件がすでにある、
		return;

	if (getConditionSize() >= other_.getConditionSize())
	{
		// otherのデータをthisにマージする

		if (other_.m_bSimple)
			mergeAndCondition(other_.m_uiMin, other_.m_uiMax);
		else
			mergeAndConditions(other_);
	}
	else
	{
		// thisのデータをotherにマージする

		if (m_bSimple)
			other_.mergeAndCondition(m_uiMin, m_uiMax);
		else
			other_.mergeAndConditions(*this);

		// other_のデータをthisにコピー
		m_bSimple = other_.m_bSimple;
		if (m_bSimple)
		{
			m_bZero = other_.m_bZero;
			m_uiMin = other_.m_uiMin;
			m_uiMax = other_.m_uiMax;
		}
		else
		{
			m_vecMin.swap(other_.m_vecMin);
			m_vecMax.swap(other_.m_vecMax);
		}
	}
}

//
//	FUNCTION private
//	Vector2::Condition::mergeAndConditions -- 複数の条件をAND条件でマージする
//
//	NOTES
//
//	ARGUMENTS
//	Condition& other_
//		マージする条件
//
//	RETURN
//
//	EXCEPTIONS
//
void
Condition::mergeAndConditions(Condition& other_)
{
	const ModUInt32 other_size = other_.m_vecMin.getSize();
	if (other_size != other_.m_vecMax.getSize())
		_SYDNEY_THROW0(Exception::Unexpected);

	ModUInt32 prev =0;
	for (ModUInt32 k = 0; k != other_size; ++k)
	{
		// マージする条件(other_)を一つずつ調べる
		
		const ModUInt32 size = m_vecMin.getSize();
		if (size != m_vecMax.getSize())
			_SYDNEY_THROW0(Exception::Unexpected);
		if (size == prev)
			// thisの条件は調べつくしたのでループを抜ける
			break;

		const ModUInt32 min = other_.m_vecMin.at(k);
		const ModUInt32 max = other_.m_vecMax.at(k);
		bool lower = false;
		bool upper = false;
		const ModUInt32 i = binsearch(min, lower, prev, size-1);
		if (i == size)
		{
			// minが最大条件より大きいので、
			// other_のこれ以降の条件を含むthisの条件は存在しない。
			break;
		}
		ModUInt32 j = binsearch(max, upper, i, size-1);

		// まずはupper
		if (upper)
		{
			if (m_vecMax.at(j) >= max + 2)
			{
				// 条件の重なっていない部分で新条件を作る
				m_vecMin.insert(m_vecMin.begin()+j+1, max+2);
				m_vecMax.insert(m_vecMax.begin()+j, max);
			}
			else if (m_vecMax.at(j) == max + 1)
			{
				// 新しい条件を作る範囲はないので更新するだけ
				
				// upeer==trueなのでj==sizeはない
				m_vecMax.erase(m_vecMax.begin()+j);
				m_vecMax.insert(m_vecMax.begin()+j, max);
			}

			// j番目は探索済み扱い
			++j;
		}

		// 次はlower
		if (lower)
		{
			// lower==trueなのでi==sizeはない
			m_vecMin.erase(m_vecMin.begin()+prev, m_vecMin.begin()+i+1);
			m_vecMin.insert(m_vecMin.begin()+prev, min);
			m_vecMax.erase(m_vecMax.begin()+prev, m_vecMax.begin()+i);
		}
		else
		{
			m_vecMin.erase(m_vecMin.begin()+prev, m_vecMin.begin()+i);
			m_vecMax.erase(m_vecMax.begin()+prev, m_vecMax.begin()+i);
		}

		// 探索済み条件位置の更新
		prev = j-(i-prev);
	}

	if (m_vecMin.getSize() != m_vecMax.getSize())
		_SYDNEY_THROW0(Exception::Unexpected);
	
	if (prev != m_vecMin.getSize())
	{
		// otherの条件と比較されなかったthisの条件を削除する
		m_vecMin.erase(m_vecMin.begin()+prev, m_vecMin.end());
		m_vecMax.erase(m_vecMax.begin()+prev, m_vecMax.end());
	}

	if (m_vecMin.getSize() == 0)
	{
		// 0件取得条件になった
		m_bZero = true;
		m_bSimple = true;
		m_uiMin = IllegalKey;
		m_uiMax = IllegalKey;
	}
}

//
//	FUNCTION private
//	Vector2::Condition::checkCondition -- 一つの条件で表せるか調べる
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Condition::checkCondition()
{
	if (m_bSimple ==false && m_vecMin.getSize() == 1)
	{
		if (m_vecMax.getSize() != 1)
			_SYDNEY_THROW0(Exception::Unexpected);

		// 一つの条件で表せる
		m_bSimple = true;
		m_uiMin = m_vecMin.at(0);
		m_uiMax = m_vecMax.at(0);
	}		
}

//
//	FUNCTION private static
//	Vector2::Condition::isOperatorLeaf -- 関係演算子ノードかどうか
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*	pCondition_
//		検索条件
//
//	RETURN
//	bool
//		関係演算子ノードならばtrue
//		そうでないならfalse
//
//	EXCEPTIONS
//
bool
Condition::isOperatorLeaf(const LogicalFile::TreeNodeInterface* pCondition_)
{
	using namespace LogicalFile;

	const int n = pCondition_->getType();
	// NotEqualsはサポートしない
	return n == TreeNodeInterface::Equals ||
		n == TreeNodeInterface::GreaterThan ||
		n == TreeNodeInterface::GreaterThanEquals ||
		n == TreeNodeInterface::LessThan ||
		n == TreeNodeInterface::LessThanEquals;
}

//
//	FUNCTION private static
//	Vector2::Condition::round -- 整数に丸める
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data*	pData_
//		関係演算子の右辺の値
//	bool& integer_
//		右辺の値が元々keyとして有効な整数かどうか
//
//	RETURN
//	ModInt64
//		丸められた値
//
//	EXCEPTIONS
//
ModInt64
Condition::round(const Common::Data& cData_,
				 bool& integer_)
{
	ModInt64 v;
	const int type = cData_.getType();
	if (type == Common::DataType::Integer)
	{
		integer_ = true;
		const Common::IntegerData& c
			= _SYDNEY_DYNAMIC_CAST(const Common::IntegerData&, cData_);
		v = static_cast<ModInt64>(c.getValue());

		if (v < 0)
			// 負数は、すべて-1。
			v = -1;
	}
	else if (type == Common::DataType::UnsignedInteger)
	{
		integer_ = true;
		const Common::UnsignedIntegerData& c
			= _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&,
								   cData_);
		v = static_cast<ModInt64>(c.getValue());
	}
	else if (type == Common::DataType::Integer64)
	{
		integer_ = true;
		const Common::Integer64Data& c
			= _SYDNEY_DYNAMIC_CAST(const Common::Integer64Data&, cData_);
		v = c.getValue();
		
		if (v < 0)
			// 負数は、すべて-1。
			v = -1;
		else if (v > IllegalKey)
			// Keyとして有効な整数の最大値以上は、すべてIllegalKey。
			v = IllegalKey;
	}
	else if (type == Common::DataType::Double)
	{
		const Common::DoubleData& c
			= _SYDNEY_DYNAMIC_CAST(const Common::DoubleData&, cData_);
		const double d = c.getValue();

		if (d < 0)
			// 負数は、すべて-1。
			v = -1;
		else if (d > IllegalKey)
			// Keyとして有効な整数の最大値以上は、すべてIllegalKey。
			v = IllegalKey;
		else
		{
			v = static_cast<ModInt64>(d);
			if (d == v)
				integer_ = true;
		}
	}
	else
	{		
		// これら以外は文字列として取り出してからintに変換する
		const int c = ModUnicodeCharTrait::toInt(cData_.getString());
		v = static_cast<ModInt64>(c);
		integer_ = true;

		if (v < 0)
			v = -1;
	}

	return v;
}

//
//	FUNCTION private
//	Vector2::Condition::binsearch -- 条件を二分探索する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 key_
//		探索対象の値
//	bool& range_
//		入力値は使わない。
//		出力値は、返り値と共にkey_と得られた条件の関係を表す。
//	ModUInt32 low_
//		探索を開始する時の下限の位置
//	ModUInt32 high_
//		探索を開始する時の上限の位置
//
//	RETURN
//	ModUInt32
//		得られた条件の位置を返す。
//		key_を含む条件があれば、range_=trueで、その位置を返す。
//		key_を含む条件がない場合は、
//		range_=falseで、key_より大きい右隣の条件の位置を返す。
//		最大条件より大きければ high_+1 を返す。
//		つまり 初期値のlow_ から 初期値のhigh_+1 の範囲で値を返す。
//
//	EXCEPTIONS
//
ModUInt32
Condition::binsearch(ModUInt32 key_,
					 bool& range_,
					 ModUInt32 low_,
					 ModUInt32 high_)
{
	const ModUInt32 size = m_vecMin.getSize();
	if (low_ > high_ || m_bSimple == true ||
		size == 0 || size != m_vecMax.getSize() ||
		low_ >= size || high_ >= size)
	{
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	range_ = false;
	ModUInt32 mid = 0;
	while (low_ <= high_)
	{
		mid = (low_ + high_)/2;
		
		if (key_ > m_vecMin.at(mid))
			low_ = mid + 1;
		else if (key_ < m_vecMin.at(mid))
		{
			if (mid == 0)
				// key_は最小条件より小さい
				return 0;
			high_ = mid - 1;
		}
		else
		{
			// key_と一致する下限を持つ条件を見つけた
			range_ = true;
			return mid;
		}
	}

	// 以下の条件でループを抜けるはず
	// high_ = low_ - 1 , m_vecMin.at(high_) < key_ < m_vecMin.at(low_)

	if (key_ > m_vecMax.at(high_))
		return low_;
	else
	{
		range_ = true;
		return high_;
	}
}

_SYDNEY_VECTOR2_END
_SYDNEY_END


//
//	Copyright (c) 2005, 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
