// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AccessFile.h -- 再構成や整合性検査で論理ファイルを読み書きするためのクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_ACCESSFILE_H
#define	__SYDNEY_SCHEMA_ACCESSFILE_H

#include "Schema/Module.h"
#include "Schema/File.h"
#include "Schema/OpenOption.h"
#include "Schema/TreeNode.h"

#include "Common/Object.h"
#include "Common/BitSet.h"

#include "Admin/Verification.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace Common {
	class DataArrayData;
	class IntegerArrayData;
}
namespace Trans {
	class Transaction;
}
namespace Lock {
	class Name;
}
namespace LogicalFile {
	class AutoLogicalFile;
}

_SYDNEY_SCHEMA_BEGIN

class Field;
namespace TreeNode {
	class Base;
	class Field;
	class Variable;
}

//	CLASS
//	Schema::IDSet -- 重複のない要素を格納するModVector
//
//	NOTES
//		IDをBitSetで保持することで重複チェックを高速にした

class IDSet : public ModVector<unsigned int>
{
public:
	typedef unsigned int			ValueType;
	typedef ModVector<ValueType>	Vector;

	IDSet() : Vector() {}
	IDSet(const Vector& vecData_) : Vector(vecData_)
	{
		ModSize n = vecData_.getSize();
		for (ModSize i = 0; i < n; ++i) {
			m_cBitSet.set(vecData_[i]);
		}
	}
	IDSet(const ModVector<Schema::File*>& vecData_) : Vector()
	{
		ModSize n = vecData_.getSize();
		reserve(n);
		for (ModSize i = 0; i < n; ++i) {
			pushBack(vecData_[i]->getID());
			m_cBitSet.set(vecData_[i]->getID());
		}
	}

	void pushBack(const ValueType& value)
	{
		Vector::pushBack(value);
		m_cBitSet.set(value);
	}
	void erase(const ValueType& value)
	{
		// Vectorは変更しない
		m_cBitSet.reset(value);
	}
	// すべての要素が含まれるか
	bool containsAll(const IDSet& cOther_) const
	{
		// BitSetの引き算をする -> b1 - b2 <=> (b1 | b2) ^ b2
		Common::BitSet cDiff(cOther_.m_cBitSet);
		cDiff -= m_cBitSet;

		// cOther_にあるものがすべてthisにあるなら結果は0のはず
		return cDiff.none();
	}
	// 少なくとも1つの要素が含まれるか
	bool containsAny(const IDSet& cOther_) const
	{
		// BitSetの&をとる
		Common::BitSet cDiff(cOther_.m_cBitSet);
		cDiff &= m_cBitSet;

		// 重なりがあれば結果は0でないはず
		return cDiff.any();
	}
	// 指定したIDを持つ要素が含まれるか
	bool contains(const ValueType& value) const
	{
		return m_cBitSet.test(value);
	}
	// 要素が空でないか
	bool any() const
	{
		return m_cBitSet.any();
	}

private:
	Common::BitSet m_cBitSet;
};

//	CLASS
//	Schema::AccessFile -- 論理ファイルを読み書きするための情報を表すクラス
//
//	NOTES
//		verifyおよび再構成後の反映でタプルを一意に識別して取得するために用いる

class AccessFile : public Common::Object
{
public:
	struct OpenMode {
		enum Value {
			None,
			Fetch,
			Search,
			Scan,
			ValueNum
		};
	};

	AccessFile(Trans::Transaction& cTrans_, const File& cFile_);
	virtual ~AccessFile();

	// 更新操作用のファイルを得る
	LogicalFile::AutoLogicalFile& getUpdateFile();
	// 検索用のファイルを得る
	LogicalFile::AutoLogicalFile& getSearchFile();

	// 指定したデータで識別されるタプルを得るためにファイルをオープンする
	void					openSearchFile(Common::DataArrayData* pKey_,
										   const Common::Data::Pointer* pValue_ = 0);
	// 更新操作のためにファイルをオープンする
	void					openUpdateFile(bool bNoUpdate_ = false, bool bBatch_ = false, bool bNoKeyUpdate_ = false);

	// 索引ファイルのときそのキーがついている列の集合を得る
	const IDSet&			getKeyColumnIDs();
	// 索引ファイルのときそのキーの値を保持するフィールドを表すスキーマ情報の集合を得る
	const ModVector<Field*>& getKeyFields();
	// 索引ファイル以外のときそのバリューの値を保持するフィールドを表すスキーマ情報の集合を得る
	const ModVector<Field*>& getValueFields();
	// 索引ファイルのときそのバリューの値(=RowID)を保持するフィールドを表すスキーマ情報を得る
	Field*					getValueField();

	// ファイルから値を取得するフィールドの位置リストを得る
	const ModVector<Field::Position>&
							getProjection();

	// ファイルからopenSearchFileで指定した条件と引数で指定した値で特定されるタプルの値を得る
	const Common::DataArrayData*
							getData(const Common::Data* pKey_ = 0);

	// ファイル中にopenSearchFileで指定した条件と引数で指定した値で特定されるタプルがあるかを得る
	bool					isExists(const Common::Data* pData_);

	// ファイル中のタプルを特定するのにキーだけではなくバリューもいるかを得る
	bool					isValueNeeded();

	// 検査対象のファイル種別を取得する
	File::Category::Value	getCategory();

	// ロック名を得る
	const Lock::Name&		getLockName() const;

	// 整合性を検査する
	void					verify(Admin::Verification::Treatment::Value uiTreatment_,
								   Admin::Verification::Progress& cProgress_);
	// ファイルのタプル数を得る
	ModInt64				getCount();

protected:
	// 定数に対応するTreeNodeInterfaceを作る
	// ファイルによって異なることがあるのでvirtual
	virtual TreeNode::Base*	createVariable(const Common::Data::Pointer& pVariable_);

	// フィールドをどのように扱うか
	virtual bool			isKey(Field* pField_) const;
	virtual bool			isGetData(Field* pField_) const;
	virtual bool			isObjectID(Field* pField_) const;

private:
	// ファイル中のタプルを特定するための条件を作る
	TreeNode::Base*			getIdentifyCondition(ModVector<TreeNode::Field*>& vecFields,
												 ModVector<TreeNode::Base*>& vecVariables);

	// 取得するフィールドやキーのフィールドのリストを作る
	void					setFields();

	// FetchでgetSearchParameterする
	bool					setFetchMode(LogicalFile::OpenOption& cOpenOption_);
	// SearchでgetSearchParameterする
	bool					setSearchMode(Common::DataArrayData* pKey_,
										  const Common::Data::Pointer* pValue_,
										  LogicalFile::OpenOption& cOpenOption_);
	// getProjectionParameterする
	bool					setProjection(LogicalFile::OpenOption& cOpenOption_);

	// 検索用のファイルに渡すオープンオプションを得る
	LogicalFile::OpenOption&		getSearchOpenOption();
	// 更新操作用のファイルに渡すオープンオプションを得る
	LogicalFile::OpenOption&		getUpdateOpenOption();

	// フィールドに対応したCommon::Dataを作る
	Common::Data*			createData(Field* pField_) const;

	//--------------------------------------
	// アクセス先のファイルについて不変な情報
	//--------------------------------------
	LogicalFile::AutoLogicalFile* m_pUpdateFile;// 更新用のファイル
	LogicalFile::AutoLogicalFile* m_pSearchFile;		// 検索用のファイル
	OpenMode::Value			m_eSearchMode;		// 検索の種類を表す
	IDSet*					m_pKeyColumnIDs;
	ModVector<Field*>*		m_pKeyFields;
	ModVector<Field*>*		m_pValueFields;
	Field*					m_pTupleIDField;
	ModVector<Field::Position>* m_pProjection;
	Common::DataArrayData*	m_pTupleData;

	TreeNode::Base*			m_pCondition;		// 一意に絞る条件
	ModVector<TreeNode::Field*>* m_pNodeFields;
	ModVector<TreeNode::Base*>* m_pNodeVariables;

	bool					m_bValueNeeded;

	LogicalFile::OpenOption*		m_pUpdateOpenOption;
	LogicalFile::OpenOption*		m_pSearchOpenOption;

	//--------------------------------------
	// 内部処理用の変数
	//--------------------------------------
	Trans::Transaction&		m_cTrans;
	const File&				m_cFile;
	Lock::Name*				m_pLockName;
	bool					m_bNoUpdate;		// 更新用ファイルをNoUpdateでopenしているか
	bool					m_bNoKeyUpdate;		// 更新用ファイルをNoKeyUpdateでopenしているか
	bool					m_bBatch;			// 更新用ファイルをBatchでopenしているか
};

//	FUNCTION public
//	Schema::AccessFile::getUpdateFile -- 更新操作用のファイルオブジェクト
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		更新操作用のファイルオブジェクトに対応するAuotAttachFileの参照
//
//	EXCEPTIONS
//		なし

inline
LogicalFile::AutoLogicalFile&
AccessFile::
getUpdateFile()
{
	return *m_pUpdateFile;
}

//	FUNCTION public
//	Schema::AccessFile::getSearchFile -- 検索操作用のファイルオブジェクト
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		検索操作用のファイルオブジェクトに対応するAuotAttachFileの参照
//
//	EXCEPTIONS
//		なし

inline
LogicalFile::AutoLogicalFile&
AccessFile::
getSearchFile()
{
	return *m_pSearchFile;
}

//	FUNCTION public
//	Schema::AccessFile::isValueNeeded -- タプルを特定するのにバリューが必要かを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//
//	EXCEPTIONS
//		なし

inline
bool
AccessFile::
isValueNeeded()
{
	return m_bValueNeeded;
}

//	FUNCTION public
//	Schema::AccessFile::getCategory -- 検査対象のファイル種別を取得する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//
//	EXCEPTIONS
//		なし

inline
File::Category::Value
AccessFile::
getCategory()
{
	return m_cFile.getCategory();
}

//	FUNCTION public
//	Schema::AccessFile::getLockName -- ファイルのロック名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ロック名
//
//	EXCEPTIONS
//		なし

inline
const Lock::Name&
AccessFile::
getLockName() const
{
	return *m_pLockName;
}

//	FUNCTION public
//	Schema::AccessFile::isKey -- フィールドをキーとして扱うか
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field* pField
//			検査対象のフィールド
//
//	RETURN
//		true ... キーとして扱う
//		false... キーとして扱わない
//
//	EXCEPTIONS
//		なし

inline
bool
AccessFile::
isKey(Field* pField_) const
{
	return pField_->isKey();
}

//	FUNCTION public
//	Schema::AccessFile::isGetData -- フィールドを取得するデータとして扱うか
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field* pField
//			検査対象のフィールド
//
//	RETURN
//		true ... 取得するデータとして扱う
//		false... 取得するデータとして扱わない
//
//	EXCEPTIONS
//		なし

inline
bool
AccessFile::
isGetData(Field* pField_) const
{
	return pField_->isData() && pField_->isGetable();
}

//	FUNCTION public
//	Schema::AccessFile::isObjectID -- フィールドをObjectIDとして扱うか
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field* pField
//			検査対象のフィールド
//
//	RETURN
//		true ... ObjectIDとして扱う
//		false... ObjectIDとして扱わない
//
//	EXCEPTIONS
//		なし

inline
bool
AccessFile::
isObjectID(Field* pField_) const
{
	return pField_->isObjectID() && pField_->isGetable();
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_ACCESSFILE_H

//
// Copyright (c) 2001, 2002, 2004, 2005, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
