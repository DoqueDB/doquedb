// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TreeNode.h -- ファイルドライバーに渡す条件関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_TREENODE_H
#define	__SYDNEY_SCHEMA_TREENODE_H

#include "LogicalFile/TreeNodeInterface.h"

#include "Schema/Module.h"
#include "Schema/Field.h"
#include "Schema/Object.h"
#include "Common/DataArrayData.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

namespace TreeNode
{
	//	CLASS
	//	Schema::TreeNode::Base --
	//		条件を表す木構造のノードを表すクラスのベースクラス
	//
	//	NOTES

	class Base : public LogicalFile::TreeNodeInterface
	{
	public:
// LogicalFile::TreeNodeInterface::
//		Type				getType() const;	// 型を得る
//
//		virtual ModUnicodeString
//							getValue() const;	// 文字列で値を得る
//
//		virtual ModSize 	getOptionSize() const;
//												// オプションの個数を得る
//		virtual const LogicalFile::TreeNodeInterface*
//							getOptionAt(ModInt32 iPosition_) const;
//												// 指定番めのオプションを得る
//		virtual ModSize		getOperandSize() const;
//												// オペランドの個数を得る
//		virtual const LogicalFile::TreeNodeInterface*
//							getOperandAt(ModInt32 iPosition_) const;
//												// 指定番めのオペランドを得る

#ifndef SYD_COVERAGE
		// Debug用
		virtual ModUnicodeString
							toString() const {return getValue();}
#endif

	protected:
		// サブクラス以外がこのクラスを生成することは許さないのでここに置く
		Base(LogicalFile::TreeNodeInterface::Type eType_);
												// コンストラクタ
	private:
	};

	//	CLASS
	//	Schema::TreeNode::Field --
	//		ファイルに与える条件を表す木構造でフィールドを表すクラス
	//
	//	NOTES

	class Field : public Base
	{
	public:
		Field(Schema::Field::Position iPosition_)
			: Base(LogicalFile::TreeNodeInterface::Field),
			  m_iPosition(iPosition_) { }		// コンストラクター

		virtual ModUnicodeString
							getValue() const;	// 文字列で値を得る
#ifndef SYD_COVERAGE
		// Debug用
		ModUnicodeString	toString() const {return getValue();}
#endif
		
	private:
		Schema::Field::Position	m_iPosition;	// フィールドの位置
	};

	//	CLASS
	//	Schema::TreeNode::Value --
	//		ファイルに与える条件を表す木構造でIDの値を表すクラス
	//
	//	NOTES

	class Value : public Base
	{
	public:
		Value(Schema::Object::ID::Value iID_)
			: Base(LogicalFile::TreeNodeInterface::ConstantValue),
			  m_iID(iID_), m_cstrValue() { }	// コンストラクター
		Value(const ModUnicodeString& cstrValue_)
			: Base(LogicalFile::TreeNodeInterface::ConstantValue),
			  m_iID(Schema::Object::ID::Invalid),
			  m_cstrValue(cstrValue_) { }		// コンストラクター

		virtual ModUnicodeString
							getValue() const;	// 文字列で値を得る
#ifndef SYD_COVERAGE
		// Debug用
		ModUnicodeString	toString() const {return getValue();}
#endif
	private:
		Schema::Object::ID::Value	m_iID;		// データの値(IDのみ)
		mutable ModUnicodeString	m_cstrValue; // 文字列になったデータの値
												// getValueでキャッシュとして
												// セットすることもあるので
												// mutable
	};

	//	CLASS
	//	Schema::TreeNode::Variable --
	//		ファイルに与える条件を表す木構造でデータを表すクラス
	//
	//	NOTES

	class Variable : public Base
	{
	public:
		Variable(Common::Data::Pointer pData_)
			: Base(LogicalFile::TreeNodeInterface::Variable),
			  m_pData(pData_) { }				// コンストラクター

		virtual ModUnicodeString
							getValue() const;	// 文字列で値を得る
		virtual const Common::Data*
							getData() const;	// Common::Dataで値を得る
		void				setData(Common::Data::Pointer pData_);
#ifndef SYD_COVERAGE
		// Debug用
		ModUnicodeString	toString() const {return getValue();}
#endif

	private:
		Common::Data::Pointer m_pData;	// データの値
	};

	//	CLASS
	//	Schema::TreeNode::Pair --
	//		ファイルに与える条件を表す木構造でペアを表すクラス
	//
	//	NOTES

	class Pair : public Base
	{
	public:
		Pair(Base& cNode1_, Base& cNode2_)
			: Base(LogicalFile::TreeNodeInterface::Pair),
			  m_cNode1(cNode1_), m_cNode2(cNode2_) { }
												// コンストラクター

		virtual ModSize		getOperandSize() const;
												// オペランドの個数を得る
		virtual const LogicalFile::TreeNodeInterface*
							getOperandAt(ModInt32 iPosition_) const;
												// 指定番めのオペランドを得る
												// (0-base)
#ifndef SYD_COVERAGE
		// Debug用
		ModUnicodeString	toString() const;
#endif
	private:
		Base&				m_cNode1;
		Base&				m_cNode2;
	};

	//	CLASS
	//	Schema::TreeNode::List --
	//		ファイルに与える条件を表す木構造でリストを表すクラス
	//
	//	NOTES

	class List : public Base
	{
	public:
		List()
			: Base(LogicalFile::TreeNodeInterface::List),
			  m_vecNodes() { }
												// コンストラクター
		~List()
		{
			while (!m_vecNodes.isEmpty()) {
				delete m_vecNodes.getBack();
				m_vecNodes.popBack();
			}
		}

		virtual ModSize		getOperandSize() const;
												// オペランドの個数を得る
		virtual const LogicalFile::TreeNodeInterface*
							getOperandAt(ModInt32 iPosition_) const;
												// 指定番めのオペランドを得る
												// (0-base)

		void				addNode(Base* pNode_); // ノードを加える

#ifndef SYD_COVERAGE
		// Debug用
		ModUnicodeString	toString() const;
#endif
	private:
		ModVector<Base*>	m_vecNodes;
	};

	//	CLASS
	//	Schema::TreeNode::And --
	//		ファイルに与える条件を表す木構造でAndを表すクラス
	//
	//	NOTES

	class And : public Base
	{
	public:
		And()
			: Base(LogicalFile::TreeNodeInterface::And),
			  m_vecNodes() { }
												// コンストラクター
		~And()
		{
			while (!m_vecNodes.isEmpty()) {
				delete m_vecNodes.getBack();
				m_vecNodes.popBack();
			}
		}

		virtual ModSize		getOperandSize() const;
												// オペランドの個数を得る
		virtual const LogicalFile::TreeNodeInterface*
							getOperandAt(ModInt32 iPosition_) const;
												// 指定番めのオペランドを得る
												// (0-base)

		void				addNode(Base* pNode_); // ノードを加える

#ifndef SYD_COVERAGE
		// Debug用
		ModUnicodeString	toString() const;
#endif
	private:
		ModVector<Base*>	m_vecNodes;
	};

	//	CLASS
	//	Schema::TreeNode::Fetch --
	//		ファイルに与える条件を表す木構造でFetch演算子を表すクラス
	//
	//	NOTES

	class Fetch : public Base
	{
	public:
		Fetch(Base& cNode_)
			: Base(LogicalFile::TreeNodeInterface::Fetch),
			  m_cNode(cNode_) { }				// コンストラクター

		virtual ModSize		getOptionSize() const;
												// オプションの個数を得る
		virtual const LogicalFile::TreeNodeInterface*
							getOptionAt(ModInt32 iPosition_) const;
												// 指定番めのオプションを得る
												// (0-base)
		virtual ModSize		getOperandSize() const;
												// オペランドの個数を得る
		virtual const LogicalFile::TreeNodeInterface*
							getOperandAt(ModInt32 iPosition_) const;
												// 指定番めのオペランドを得る
#ifndef SYD_COVERAGE
		// Debug用
		ModUnicodeString	toString() const;
#endif
	private:
		Base&				m_cNode;			// Fetchの条件フィールド
	};

	//	CLASS
	//	Schema::TreeNode::Equals --
	//		ファイルに与える条件を表す木構造でEquals演算子を表すクラス
	//
	//	NOTES

	class Equals : public Base
	{
	public:
		Equals(Base& cNode1_, Base& cNode2_)
			: Base(LogicalFile::TreeNodeInterface::Equals),
			  m_cOperand(cNode1_, cNode2_) { }	// コンストラクター

		virtual ModSize		getOperandSize() const;
												// オペランドの個数を得る
		virtual const LogicalFile::TreeNodeInterface*
							getOperandAt(ModInt32 iPosition_) const;
												// 指定番めのオペランドを得る
												// (0-base)
#ifndef SYD_COVERAGE
		// Debug用
		ModUnicodeString	toString() const;
#endif
	private:
		TreeNode::Pair		m_cOperand;			// Equalsの条件に使うフィールド
	};

	//	CLASS
	//	Schema::TreeNode::EqualsToNull --
	//		ファイルに与える条件を表す木構造でEqualsToNull演算子を表すクラス
	//
	//	NOTES

	class EqualsToNull : public Base
	{
	public:
		EqualsToNull(Base& cNode_)
			: Base(LogicalFile::TreeNodeInterface::EqualsToNull),
			  m_cOperand(cNode_) { }	// コンストラクター

		virtual ModSize		getOperandSize() const;
												// オペランドの個数を得る
		virtual const LogicalFile::TreeNodeInterface*
							getOperandAt(ModInt32 iPosition_) const;
												// 指定番めのオペランドを得る
												// (0-base)
#ifndef SYD_COVERAGE
		// Debug用
		ModUnicodeString	toString() const;
#endif
	private:
		TreeNode::Base&		m_cOperand;			// EqualsToNullの条件に使うフィールド
	};

} // namespace TreeNode

////////////////////////////
// Schema::TreeNode::Base //
////////////////////////////

//	FUNCTION protected
//	Schema::TreeNode::Base::Base -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		LogicalFile::TreeNodeInterface::Type eType_
//			ノードの種別を表す値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
TreeNode::Base::
Base(LogicalFile::TreeNodeInterface::Type eType_)
	: LogicalFile::TreeNodeInterface(eType_)
{ }

////////////////////////////
// Schema::TreeNode::Pair //
////////////////////////////

//	FUNCTION public
//	Schema::TreeNode::Pair::getOperandSize --
//		ペアを表すノードのオペランド数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		固定値2が返る
//
//	EXCEPTIONS

inline
ModSize
TreeNode::Pair::
getOperandSize() const
{
	return static_cast<ModSize>(2);
}

//	FUNCTION public
//	Schema::TreeNode::Pair::getOperandAt --
//		ペアを表すノードの指定した位置のオペランドを得る
//
//	NOTES
//
//	ARGUMENTS
//		ModInt32 iPosition_
//			この位置のオペランドを得る
//
//	RETURN
//		0なら最初の、それ以外なら最後のノードが返る
//
//	EXCEPTIONS

inline
const LogicalFile::TreeNodeInterface*
TreeNode::Pair::
getOperandAt(ModInt32 iPosition_) const
{
	return (iPosition_) ? &m_cNode2 : &m_cNode1;
}

////////////////////////////
// Schema::TreeNode::List //
////////////////////////////

//	FUNCTION public
//	Schema::TreeNode::List::getOperandSize --
//		リストを表すノードのオペランド数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		リストのノード数
//
//	EXCEPTIONS

inline
ModSize
TreeNode::List::
getOperandSize() const
{
	return m_vecNodes.getSize();
}

//	FUNCTION public
//	Schema::TreeNode::List::getOperandAt --
//		リストを表すノードの指定した位置のオペランドを得る
//
//	NOTES
//
//	ARGUMENTS
//		ModInt32 iPosition_
//			この位置のオペランドを得る
//
//	RETURN
//		指定された位置のノード
//
//	EXCEPTIONS

inline
const LogicalFile::TreeNodeInterface*
TreeNode::List::
getOperandAt(ModInt32 iPosition_) const
{
	return m_vecNodes[iPosition_];
}

//	FUNCTION public
//	Schema::TreeNode::List::addNode --
//		リストを表すノードにオペランドを追加する
//
//	NOTES
//
//	ARGUMENTS
//		TreeNode::Base* pNode_
//			追加するオペランド
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
TreeNode::List::
addNode(Base* pNode_)
{
	m_vecNodes.pushBack(pNode_);
}

////////////////////////////
// Schema::TreeNode::And //
////////////////////////////

//	FUNCTION public
//	Schema::TreeNode::And::getOperandSize --
//		リストを表すノードのオペランド数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		リストのノード数
//
//	EXCEPTIONS

inline
ModSize
TreeNode::And::
getOperandSize() const
{
	return m_vecNodes.getSize();
}

//	FUNCTION public
//	Schema::TreeNode::And::getOperandAt --
//		リストを表すノードの指定した位置のオペランドを得る
//
//	NOTES
//
//	ARGUMENTS
//		ModInt32 iPosition_
//			この位置のオペランドを得る
//
//	RETURN
//		指定された位置のノード
//
//	EXCEPTIONS

inline
const LogicalFile::TreeNodeInterface*
TreeNode::And::
getOperandAt(ModInt32 iPosition_) const
{
	return m_vecNodes[iPosition_];
}

//	FUNCTION public
//	Schema::TreeNode::And::addNode --
//		リストを表すノードにオペランドを追加する
//
//	NOTES
//
//	ARGUMENTS
//		TreeNode::Base* pNode_
//			追加するオペランド
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
TreeNode::And::
addNode(Base* pNode_)
{
	m_vecNodes.pushBack(pNode_);
}

/////////////////////////////
// Schema::TreeNode::Fetch //
/////////////////////////////

//	FUNCTION public
//	Schema::TreeNode::Fetch::getOptionSize --
//		Fetchを表すノードのオプションの数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		固定値2
//
//	EXCEPTIONS

inline
ModSize
TreeNode::Fetch::
getOptionSize() const
{
	return static_cast<ModSize>(2);
}

//	FUNCTION public
//	Schema::TreeNode::Fetch::getOptionAt --
//		Fetchを表すノードの指定した位置のオプションを得る
//
//	NOTES
//
//	ARGUMENTS
//		ModInt32 iPosition_
//			この位置のオプションを得る
//
//	RETURN
//		引数に関係なくコンストラクターに指定されたノードのアドレスを返す
//
//	EXCEPTIONS

inline
const LogicalFile::TreeNodeInterface*
TreeNode::Fetch::
getOptionAt(ModInt32 iPosition_) const
{
	return &m_cNode;
}

//	FUNCTION public
//	Schema::TreeNode::Fetch::getOperandSize --
//		ノードのオペランド数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		固定値1が返る
//
//	EXCEPTIONS

inline
ModSize
TreeNode::Fetch::
getOperandSize() const
{
	return static_cast<ModSize>(1);
}

//	FUNCTION public
//	Schema::TreeNode::Fetch::getOperandAt --
//		ペアを表すノードの指定した位置のオペランドを得る
//
//	NOTES
//
//	ARGUMENTS
//		ModInt32 iPosition_
//			この位置のオペランドを得る
//
//	RETURN
//		自身を返す
//
//	EXCEPTIONS

inline
const LogicalFile::TreeNodeInterface*
TreeNode::Fetch::
getOperandAt(ModInt32 iPosition_) const
{
	return this;
}

/////////////////////////////
// Schema::TreeNode::Equals //
/////////////////////////////

//	FUNCTION public
//	Schema::TreeNode::Equals::getOperandSize --
//		Equalsを表すノードのオペランドの数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		固定値2
//
//	EXCEPTIONS

inline
ModSize
TreeNode::Equals::
getOperandSize() const
{
	return static_cast<ModSize>(2);
}

//	FUNCTION public
//	Schema::TreeNode::Equals::getOperandAt --
//		Equalsを表すノードの指定した位置のオペランドを得る
//
//	NOTES
//
//	ARGUMENTS
//		ModInt32 iPosition_
//			この位置のオペランドを得る
//
//	RETURN
//		コンストラクターに指定されたノードの指定位置のオペランドを返す
//
//	EXCEPTIONS

inline
const LogicalFile::TreeNodeInterface*
TreeNode::Equals::
getOperandAt(ModInt32 iPosition_) const
{
	return m_cOperand.getOperandAt(iPosition_);
}

////////////////////////////////////
// Schema::TreeNode::EqualsToNull //
////////////////////////////////////

//	FUNCTION public
//	Schema::TreeNode::EqualsToNull::getOperandSize --
//		Equalsを表すノードのオペランドの数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		固定値1
//
//	EXCEPTIONS

inline
ModSize
TreeNode::EqualsToNull::
getOperandSize() const
{
	return static_cast<ModSize>(1);
}

//	FUNCTION public
//	Schema::TreeNode::EqualsToNull::getOperandAt --
//		Equalsを表すノードの指定した位置のオペランドを得る
//
//	NOTES
//
//	ARGUMENTS
//		ModInt32 iPosition_
//			この位置のオペランドを得る
//
//	RETURN
//		コンストラクターに指定されたノードの指定位置のオペランドを返す
//
//	EXCEPTIONS

inline
const LogicalFile::TreeNodeInterface*
TreeNode::EqualsToNull::
getOperandAt(ModInt32 iPosition_) const
{
	return (iPosition_ == 0) ? &m_cOperand : 0;
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_TREENODE_H

//
// Copyright (c) 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
