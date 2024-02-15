// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AccessFile.cpp -- 再構成や整合性検査で論理ファイルを読み書きするためのクラスの定義
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "SyInclude.h"

#include "Schema/AccessFile.h"
#include "Schema/AutoLatch.h"
#include "Schema/Field.h"
#include "Schema/Column.h"
#include "Schema/File.h"
#include "Schema/Message.h"
#include "Schema/OpenOption.h"
#include "Schema/Parameter.h"
#include "Schema/TreeNode.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/DataInstance.h"
#include "Common/IntegerArrayData.h"
#include "Common/Message.h"

#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"

#include "FileCommon/OpenOption.h"

#include "FullText/OpenOption.h"

#include "LogicalFile/AutoLogicalFile.h"
#include "LogicalFile/FileDriver.h"
#include "LogicalFile/FileDriverManager.h"
#include "LogicalFile/OpenOption.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace {

	//	CONST local
	//	_cOpenModeKey --
	//
	//	NOTES

	const OpenOption::KeyType _cOpenModeKey(FileCommon::OpenOption::OpenMode::Key);

	//	CONST local
	//	_cOpenModeUpdate --
	//
	//	NOTES

	const OpenOption::OpenModeType _cOpenModeUpdate = _SYDNEY_SCHEMA_PARAMETER_VALUE(FileCommon::OpenOption::OpenMode::Update);

	namespace _File {

		// フィールドに対応する列IDを得る
		Schema::Object::ID::Value _getColumnID(Trans::Transaction& cTrans_, Field* pField_);
		// fetch可能なファイルか
		bool _isFetchable(const File& cFile_);
	}

} // namespace

///////////////////////
// _File
///////////////////////

//	FUNCTION local
//	_File::_getColumnID -- フィールドに対応する列IDを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Field* pField_
//			調べるフィールド
//
//	RETURN
//		列ID
//
//	EXCEPTIONS

Schema::Object::ID::Value
_File::_getColumnID(Trans::Transaction& cTrans_, Field* pField_)
{
	if (!pField_) {
		return Schema::Object::ID::Invalid;
	}
	if (pField_->getColumnID() != Schema::Object::ID::Invalid) {
		return pField_->getColumnID();
	}
	return _getColumnID(cTrans_, pField_->getSource(cTrans_));
}

//	FUNCTION local
//	_File::_isFetchable --
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::File& cFile_
//			調べるファイル
//
//	RETURN
//		true .. fetchできる
//		false.. fetchできない
//
//	EXCEPTIONS

inline
bool
_File::_isFetchable(const File& cFile_)
{
	switch (cFile_.getCategory()) {
	case File::Category::Record:
	case File::Category::Lob:
	case File::Category::Vector:
	case File::Category::Btree:
		return true;
	}
	return false;
}

//////////////////////////////////////////
// AccessFile
//////////////////////////////////////////

//	FUNCTION public
//	Schema::AccessFile::AccessFile --
//		コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::File& cFile_
//			再構成の対象となっているファイル
//
//	RETURN
//		なし
//
//	EXCEPTIONS

AccessFile::
AccessFile(Trans::Transaction& cTrans_, const File& cFile_)
	: m_cTrans(cTrans_), m_cFile(cFile_),
	  m_pKeyColumnIDs(0), m_pKeyFields(0), m_pValueFields(0),
	  m_pTupleIDField(0), m_pProjection(0), m_pTupleData(0),
	  m_pCondition(0), m_pNodeFields(0), m_pNodeVariables(0),
	  m_bValueNeeded(false),
	  m_pUpdateOpenOption(0), 
	  m_pSearchOpenOption(0),
	  m_pLockName(0),
	  m_bNoUpdate(true),
	  m_bNoKeyUpdate(false),
	  m_bBatch(false)
{
	LogicalFile::FileDriver* pDriver =
		LogicalFile::FileDriverManager::getDriver(cFile_.getDriverID());
	; _SYDNEY_ASSERT(pDriver);

	// 登録用
	m_pUpdateFile = new LogicalFile::AutoLogicalFile(*pDriver, cFile_.getFileID());
	// Redo/Undo時の検索用
	m_pSearchFile = new LogicalFile::AutoLogicalFile(*pDriver, cFile_.getFileID());
	m_eSearchMode = OpenMode::None;

	m_pLockName = new Lock::FileName(cFile_.getDatabaseID(), cFile_.getTableID(), cFile_.getID());

	m_bValueNeeded = (cFile_.getIndexID() != Schema::Object::ID::Invalid
					  && !(cFile_.isKeyUnique() && cFile_.isKeyNotNull(cTrans_)));
}

//	FUNCTION public
//	Schema::AccessFile::~AccessFile --
//		デストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

AccessFile::
~AccessFile()
{
	// LogicalFile::AutoLogicalFileのデストラクターでcloseが呼ばれるかもしれないのでラッチが必要
	{
	AutoLatch latch(m_cTrans, *m_pLockName);
	delete m_pUpdateFile, m_pUpdateFile = 0;
	delete m_pSearchFile, m_pSearchFile = 0;
	}
	delete m_pLockName, m_pLockName = 0;
	m_eSearchMode = OpenMode::None;

	delete m_pUpdateOpenOption, m_pUpdateOpenOption = 0;
	delete m_pSearchOpenOption, m_pSearchOpenOption = 0;

	delete m_pKeyColumnIDs, m_pKeyColumnIDs = 0;
	delete m_pKeyFields, m_pKeyFields = 0;
	delete m_pValueFields, m_pValueFields = 0;

	m_pTupleIDField = 0;
	delete m_pProjection, m_pProjection = 0;
	delete m_pTupleData, m_pTupleData = 0;

	delete m_pCondition, m_pCondition = 0;
	if (m_pNodeFields) {
		ModSize n = m_pNodeFields->getSize();
		for (ModSize i = 0; i < n; ++i) {
			delete (*m_pNodeFields)[i], (*m_pNodeFields)[i] = 0;
		}
		delete m_pNodeFields, m_pNodeFields = 0;
	}
	if (m_pNodeVariables) {
		ModSize n = m_pNodeVariables->getSize();
		for (ModSize i = 0; i < n; ++i) {
			delete (*m_pNodeVariables)[i], (*m_pNodeVariables)[i] = 0;
		}
		delete m_pNodeVariables, m_pNodeVariables = 0;
	}
}

//	FUNCTION public
//	Schema::AccessFile::getSearchOpenOption --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

LogicalFile::OpenOption&
AccessFile::
getSearchOpenOption()
{
	if (!m_pSearchOpenOption) {
		m_pSearchOpenOption = new LogicalFile::OpenOption;
	}
	return *m_pSearchOpenOption;
}

//	FUNCTION public
//	Schema::AccessFile::getUpdateOpenOption --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

LogicalFile::OpenOption&
AccessFile::
getUpdateOpenOption()
{
	if (!m_pUpdateOpenOption) {
		m_pUpdateOpenOption = new LogicalFile::OpenOption;
	}
	return *m_pUpdateOpenOption;
}

//	FUNCTION public
//	AccessFile::openSearchFile --
// 		検索用のファイルオープン
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataArrayData* pKey_
//			キーのデータ
//		Common::Data::Pointer* pValue_ = 0
//			ファイルがキーだけでユニークでない場合
//			バリューのデータも渡す
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AccessFile::
openSearchFile(Common::DataArrayData* pKey_,
			   const Common::Data::Pointer* pValue_)
{
	if (getUpdateFile().isOpened() && m_bBatch) {
		// UpdateFileがバッチモードでオープンされているときは
		// いったんクローズしないと正しく検索できない
		AutoLatch latch(m_cTrans, *m_pLockName);
		getUpdateFile().close();
	}

	LogicalFile::OpenOption& cOpenOption = getSearchOpenOption();

	if (m_eSearchMode == OpenMode::None) {
		; _SYDNEY_ASSERT(!getSearchFile().isOpened());

		if (!pKey_) {
			// キーが与えられないときはスキャンアクセス
			if (getSearchFile().getSearchParameter(0, cOpenOption)) {
				// 取得するフィールドをセットする
				setProjection(cOpenOption);

				// Scanの場合はここでopenしてよい
				// ★注意★
				// closeはendImportでのdeleteで行われる
				AutoLatch latch(m_cTrans, *m_pLockName);
				getSearchFile().open(m_cTrans, cOpenOption);
				m_eSearchMode = OpenMode::Scan;

			} else {
				// スキャンアクセスできないのにキーが与えられていない
				_SYDNEY_THROW0(Exception::BadArgument);
			}

		} else {
			; _SYDNEY_ASSERT(pKey_);

			// キーとバリューまたはキーでfetchするオープンオプションをセットする
			if (setFetchMode(cOpenOption)) {
				// 取得するフィールドをセットする
				setProjection(cOpenOption);

				// Fetchの場合はここでopenしてよい
				// ★注意★
				// closeはendImportでのdeleteで行われる
				AutoLatch latch(m_cTrans, *m_pLockName);
				getSearchFile().open(m_cTrans, cOpenOption);
				m_eSearchMode = OpenMode::Fetch;

			} else {
				// Fetchで取得できないのでキーでSearchした上でROWIDを比較する
				// → 毎回openし直す必要があるのでここではモード設定のみ
				m_eSearchMode = OpenMode::Search;
			}
		}
	}

	AutoLatch latch(m_cTrans, *m_pLockName);

	if (m_eSearchMode == OpenMode::Fetch) {
		// Fetchの場合はfetch()を一回呼び出す

		if (isValueNeeded()) {
			// バリューも条件に加える
			; _SYDNEY_ASSERT(pValue_);
			; _SYDNEY_ASSERT(pValue_->get());

			Common::DataArrayData cKey;
			cKey = *pKey_;				// ポインターのコピー
			cKey.pushBack(*pValue_);

			getSearchFile().fetch(&cKey);

		} else {
			getSearchFile().fetch(pKey_);
		}

	} else if (m_eSearchMode == OpenMode::Search) {

		// Searchの場合はキーデータをセットしてgetSearchParameterする

		if (getSearchFile().isOpened()) {
			getSearchFile().close();
			cOpenOption.clear();
		}

		if (setSearchMode(pKey_, pValue_, cOpenOption)) {
			// 取得するフィールドをセットする
			setProjection(cOpenOption);

			getSearchFile().open(m_cTrans, cOpenOption);

		} else {
			// FetchでもSearchでも取得できなければキーに問題があることを
			// オープンモードをセットしないことであらわす
			m_eSearchMode = OpenMode::None;
		}

	} else {
		; _SYDNEY_ASSERT(m_eSearchMode == OpenMode::Scan);
	}
}

//	FUNCTION public
//	AccessFile::openUpdateFile --
// 		更新用のファイルオープン
//
//	NOTES
//
//	ARGUMENTS
//		bool bNoUpdate_ = false
//			trueのときupdateは呼ばれないのでgetUpdateParameterしなくてよい
//		bool bBatch_ = false
//			trueのときバッチ的な登録を行うので特別な処理をする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AccessFile::
openUpdateFile(bool bNoUpdate_ /* = false */, bool bBatch_ /* = false */, bool bNoKeyUpdate_ /* = false */)
{
	if (getUpdateFile().isOpened()
		&& ((m_bNoUpdate && !bNoUpdate_) || (m_bBatch && !bBatch_) || (m_bNoKeyUpdate != bNoKeyUpdate_))) {
		// NoUpdate=trueでオープンされていて、NoUpdate=falseでのオープンが要求されたら
		// またはBatch=trueでオープンされていて、Batch=falseでのオープンが要求されたら
		// またはNoKeyUpdate=true/falseでオープンされていて、NoKeyUpdate=false/trueでのオープンが要求されたら
		// ここで一度closeしてオープンしなおす
		AutoLatch latch(m_cTrans, *m_pLockName);
		getUpdateFile().close();
	}
	if (!getUpdateFile().isOpened()) {
		AutoLatch latch(m_cTrans, *m_pLockName);

		LogicalFile::OpenOption& cOpenOption = getUpdateOpenOption();
		// 一度クリアする
		cOpenOption.clear();
		OpenOption::setOpenMode(cOpenOption, _cOpenModeKey, _cOpenModeUpdate);

		if (bBatch_ && getCategory() == File::Category::FullText) {
			// 全文ファイルの場合はバッチ的に登録できるときは特別のモードをセットする
			cOpenOption.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(FullText::OpenOption::CreateIndex::Key),
								   true);
		}

		if (!bNoUpdate_) {

			// 更新されるフィールドの位置情報
			ModVector<int> vecPositions;
			const ModVector<Field*>& vecFields
				= (bNoKeyUpdate_) ? getValueFields() : getKeyFields();

			ModSize n = vecFields.getSize();
			vecPositions.reserve(n);

			for (ModSize i = 0; i < n; ++i) {
				vecPositions.pushBack(vecFields[i]->getPosition());
			}

			bool bResult =
				getUpdateFile().getUpdateParameter(Common::IntegerArrayData(vecPositions),
												   cOpenOption);
			// UpdateParameterは失敗しないはず
			; _SYDNEY_ASSERT(bResult);
		}

		// openする
		getUpdateFile().open(m_cTrans, cOpenOption);
		m_bNoUpdate = bNoUpdate_;
		m_bBatch = bBatch_;
		m_bNoKeyUpdate = bNoKeyUpdate_;
	}
}

//	FUNCTION public
//	Schema::AccessFile::getKeyColumnIDs --
//		キーのフィールドに対応する列のIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		キーに対応する列のスキーマオブジェクトIDのベクター
//
//	EXCEPTIONS

const IDSet&
AccessFile::
getKeyColumnIDs()
{
	if (!m_pKeyColumnIDs) {
		setFields();
	}
	; _SYDNEY_ASSERT(m_pKeyColumnIDs);

	return *m_pKeyColumnIDs;
}

//	FUNCTION public
//	Schema::AccessFile::getKeyFields --
//		キーフィールドを得る
//
//	NOTES
//		Schema::Fileのものを使うと毎回Vectorを作り直すので
//		ここに作った
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		キーに対応する列のスキーマオブジェクトIDのベクター
//
//	EXCEPTIONS

const ModVector<Field*>&
AccessFile::
getKeyFields()
{
	if (!m_pKeyFields) {
		setFields();
	}
	; _SYDNEY_ASSERT(m_pKeyFields);

	return *m_pKeyFields;
}

//	FUNCTION public
//	Schema::AccessFile::getValueFields --
//		バリューフィールドを得る
//
//	NOTES
//		Schema::Fileのものを使うと毎回Vectorを作り直すので
//		ここに作った
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		バリューに対応する列のスキーマオブジェクトのベクター
//
//	EXCEPTIONS

const ModVector<Field*>&
AccessFile::
getValueFields()
{
	if (!m_pValueFields) {
		setFields();
	}
	; _SYDNEY_ASSERT(m_pValueFields);

	return *m_pValueFields;
}

//	FUNCTION public
//	Schema::AccessFile::getValueField --
//		バリューフィールドを得る
//
//	NOTES
//		isValueNeeded() == trueのときのみ意味がある
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		索引のバリュー、すなわちTupleIDを格納するフィールド
//
//	EXCEPTIONS

Field*
AccessFile::
getValueField()
{
	if (isValueNeeded()) {
		if (!m_pTupleIDField) {
			setFields();
		}
		return m_pTupleIDField;
	}
	return 0;
}

//	FUNCTION public
//	Schema::AccessFile::getProjection --
//		取得するフィールドの位置配列を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		取得するフィールドの位置配列
//
//	EXCEPTIONS

const ModVector<Field::Position>&
AccessFile::
getProjection()
{
	if (!m_pProjection) {
		setFields();
	}
	; _SYDNEY_ASSERT(m_pProjection);

	return *m_pProjection;
}

//	FUNCTION public
//	AccessFile::getData --
// 		検索した結果の中に指定した値と一致するタプルを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data* pData_
//			比較する値
//			この値が0なら一致するものではなく得られたものをそのまま返す
//
//	RETURN
//		0以外	一致するタプル
//		0		一致するタプルがない
//
//	EXCEPTIONS

const Common::DataArrayData*
AccessFile::
getData(const Common::Data* pData_ /* = 0 */)
{
	; _SYDNEY_ASSERT(m_pTupleData);

	bool bNotEmpty;
	{
	AutoLatch latch(m_cTrans, *m_pLockName);
	bNotEmpty = getSearchFile().getData(m_pTupleData);
	}

	while (bNotEmpty) {

		// キーだけでユニーク、またはOpenModeがFetchなら
		// 最初に得たものがそのもの

		if (!pData_ || !isValueNeeded() || m_eSearchMode == OpenMode::Fetch) {
#ifdef DEBUG
			SydSchemaDebugMessage
				<< "AccessFile::getData[" << m_cFile.getName() << "]: found=" << *m_pTupleData
				<< ModEndl;
#endif
			break;
		}

		// キーだけでSearchしているのでバリューと比較する
		; _SYDNEY_ASSERT(pData_);
		if (pData_->getType() == Common::DataType::Array) {
			if (m_pTupleData->equals(pData_)) {
#ifdef DEBUG
				SydSchemaDebugMessage
					<< "AccessFile::getData[" << m_cFile.getName() << "]: found(" << *pData_ << ")="
					<< *m_pTupleData
					<< ModEndl;
#endif
				break;
			}

		} else {
			if (m_pTupleData->getElement(0)->equals(pData_)) {
#ifdef DEBUG
				SydSchemaDebugMessage
					<< "AccessFile::getData[" << m_cFile.getName() << "]: found(" << *pData_ << ")="
					<< *m_pTupleData
					<< ModEndl;
#endif
				break;
			}
		}
		{
		AutoLatch latch(m_cTrans, *m_pLockName);
		bNotEmpty = getSearchFile().getData(m_pTupleData);
		}
	}
	return bNotEmpty ? m_pTupleData : 0;
}

//	FUNCTION public
//	AccessFile::isExists --
// 		検索した結果の中に指定した値と一致するタプルがあるかを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data* pData_
//			比較する値
//
//	RETURN
//		true...一致するタプルがある
//		false...一致するタプルがない
//
//	EXCEPTIONS

bool
AccessFile::
isExists(const Common::Data* pData_)
{
	if (m_eSearchMode != OpenMode::None) {
		AutoLatch latch(m_cTrans, *m_pLockName);
		const Common::DataArrayData* pTmpData = getData(pData_);
		return (pTmpData != 0);
	}
	else {
		// OpenでSearchModeが設定できなかったということは一致するタプルがないとみなしてよい
		return false;
	}
}

//	FUNCTION public
//	Schema::AccessFile::verify --
// 		ファイルの整合性を検査する
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Verification::Treatment::Value	eTreatment_
//			検査の結果見つかった障害に対する対応を表し、
//			Admin::Verification::Treatment::Value の論理和を指定する
//		Admin::Verification::Progress& cProgress_
//			検査結果を格納する変数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AccessFile::
verify(Admin::Verification::Treatment::Value uiTreatment_,
	   Admin::Verification::Progress& cProgress_)
{
	AutoLatch latch(m_cTrans, *m_pLockName);
	getSearchFile().verify(m_cTrans, uiTreatment_, cProgress_);
}

//	FUNCTION public
//	Schema::AccessFile::getCount --
// 		ファイルのタプル数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		タプル数
//
//	EXCEPTIONS

ModInt64
AccessFile::
getCount()
{
	AutoLatch latch(m_cTrans, *m_pLockName);
	return getSearchFile().getCount(m_cTrans);
}

//	FUNCTION protected
//	Schema::AccessFile::createVariable --
// 		定数に対応するTreeNodeInterfaceオブジェクトを作成する
//
//	NOTES
//		ファイルによって定数の表現が異なることがあるので
//		その違いをこの関数をオーバーライドすることで吸収する
//
//	ARGUMENTS
//		const Common::Data::Pointer& pVariable_
//			変換するData
//
//	RETURN
//		0以外	定数を表すTreeNode
//		0		引数がNullDataである場合
//
//	EXCEPTIONS

TreeNode::Base*
AccessFile::
createVariable(const Common::Data::Pointer& pVariable_)
{
	if (pVariable_->isNull())
		// NullDataのときはVariableをヌルポインターにする
		return 0;

	switch (pVariable_->getType()) {
	case Common::DataType::Array:
	{
		if (pVariable_->getElementType() == Common::DataType::Data) {
			ModAutoPointer<TreeNode::List> pList = new TreeNode::List;
			const Common::DataArrayData& cData =
				_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, *pVariable_);
			int n = cData.getCount();
			for (int i = 0; i < n; ++i) {
				pList->addNode(createVariable(cData.getElement(i)));
			}
			return pList.release();
		}
		// DataArrayData以外のArrayは想定していない
		; _SYDNEY_ASSERT(false);
	}
	break;
	default:
		break;
	}
	return new TreeNode::Variable(pVariable_);
}

//	FUNCTION private
//	Schema::AccessFile::getIdentifyCondition --
//		タプルを特定、または十分に絞り込む条件を作成する
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<TreeNode::Field*>& vecFields
//			条件を与えるキーフィールド
//		ModVector<TreeNode::Base*>& vecVariables
//			条件の引数となる変数
//
//	RETURN
//		作成した条件を表すTreeNodeInterface
//
//	EXCEPTIONS

// virtual
TreeNode::Base*
AccessFile::
getIdentifyCondition(ModVector<TreeNode::Field*>& vecFields,
					 ModVector<TreeNode::Base*>& vecVariables)
{
	; _SYDNEY_ASSERT(vecFields.getSize() == vecVariables.getSize());
	; _SYDNEY_ASSERT(vecFields.getSize() > 0);

	ModSize n = vecFields.getSize();
	if (n > 1) {
		ModAutoPointer<TreeNode::And> pAnd = new TreeNode::And();
		for (ModSize i = 0; i < n; ++i) {
			if (!vecVariables[i]) {
				// VariableがNULLのときは条件はEqualsToNull
				pAnd->addNode(new TreeNode::EqualsToNull(*(vecFields[i])));
			} else {
				pAnd->addNode(new TreeNode::Equals(*(vecFields[i]), *(vecVariables[i])));
			}
		}
		return pAnd.release();
	}

	; _SYDNEY_ASSERT(n == 1);

	if (!vecVariables[0]) {
		// VariableがNULLのときは条件はEqualsToNull
		return new TreeNode::EqualsToNull(*(vecFields[0]));
	} else {
		return new TreeNode::Equals(*(vecFields[0]), *(vecVariables[0]));
	}
}

//	FUNCTION private
//	Schema::AccessFile::setFields --
//		フィールド情報をセットする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
AccessFile::
setFields()
{
	if (!m_pKeyColumnIDs) {
		m_pKeyColumnIDs = new IDSet();
	} else {
		m_pKeyColumnIDs->clear();
	}
	if (!m_pKeyFields) {
		m_pKeyFields = new ModVector<Field*>();
	} else {
		m_pKeyFields->clear();
	}
	if (!m_pValueFields) {
		m_pValueFields = new ModVector<Field*>();
	} else {
		m_pValueFields->clear();
	}
	if (!m_pTupleData) {
		m_pTupleData = new Common::DataArrayData;
	} else {
		m_pTupleData->clear();
	}

	if (!m_pProjection) {
		m_pProjection = new ModVector<Field::Position>();
	} else {
		m_pProjection->clear();
	}

	const ModVector<Field*>& vecFields = m_cFile.getField(m_cTrans);
	ModSize n = vecFields.getSize();
	// reallocのコストを減らすために多めだがこのサイズでreserveする
	m_pKeyColumnIDs->reserve(n);
	m_pKeyFields->reserve(n);
	m_pValueFields->reserve(n);
	m_pTupleData->reserve(n);

	bool bIsIndexFile = (m_cFile.getIndexID() != Schema::Object::ID::Invalid);

	for (ModSize i = 0; i < n; ++i) {
		Field* pField = vecFields[i];
		if (pField->isPutable()) {
			if (isKey(pField)) {
				m_pKeyFields->pushBack(pField);
				Schema::Object::ID::Value iColumnID = _File::_getColumnID(m_cTrans, pField);
				; _SYDNEY_ASSERT(iColumnID != Schema::Object::ID::Invalid);
				m_pKeyColumnIDs->pushBack(iColumnID);

			} else if (isGetData(pField)) {

				// 取得対象のフィールド
				if (!bIsIndexFile)
					m_pValueFields->pushBack(pField);
				m_pProjection->pushBack(pField->getPosition());
				m_pTupleData->pushBack(createData(pField));
				if (!m_pTupleIDField) {
					if (pField->isTupleID(m_cTrans)) {
						m_pTupleIDField = pField;
					}
				}
			} else if (isObjectID(pField)) {

				// 索引でない場合、ObjectIDフィールドはキーにする
				if (!bIsIndexFile) {
					; _SYDNEY_ASSERT(m_pKeyFields->getSize() == 0);
					m_pKeyFields->pushBack(pField);
					Schema::Object::ID::Value iColumnID = _File::_getColumnID(m_cTrans, pField);
					if (iColumnID != Schema::Object::ID::Invalid)
						m_pKeyColumnIDs->pushBack(iColumnID);

					// 取得対象にもする
					m_pProjection->pushBack(pField->getPosition());
					m_pTupleData->pushBack(createData(pField));
				}
			}
		}
	}

#ifdef DEBUG
	if (bIsIndexFile) {
		// 索引を構成するファイルのデータフィールドはタプルID1つのみである
		; _SYDNEY_ASSERT(m_pTupleIDField);
		; _SYDNEY_ASSERT(m_pProjection->getSize() == 1);
	}
#endif
}

//	FUNCTION private
//	AccessFile::setFetchMode --
// 		キーまたはキーとバリューでフェッチするオープンモードをセットする
//
//	NOTES
//
//	ARGUMENTS
//		LogicalFile::OpenOption& cOpenOption_
//			オープンモードをセットする
//
//	RETURN
//		true...フェッチのオープンモードが正しくセットされた
//		false...フェッチはできない
//
//	EXCEPTIONS

bool
AccessFile::
setFetchMode(LogicalFile::OpenOption& cOpenOption_)
{
	; _SYDNEY_ASSERT(m_eSearchMode == OpenMode::None);

	if (_File::_isFetchable(m_cFile)) {
		TreeNode::List cFetchFields;

		// キーフィールドを得る
		const ModVector<Field*>& vecKeyFields = getKeyFields();
		ModSize n = vecKeyFields.getSize();
		for (ModSize i = 0; i < n; ++i) {
			cFetchFields.addNode(new TreeNode::Field(vecKeyFields[i]->getPosition()));
		}
		if (isValueNeeded()) {
			// タプルを特定するのにバリューが必要である
			// 現在はこのような場合は索引のファイルだけなので
			// ROWIDのフィールドを入れる
			cFetchFields.addNode(new TreeNode::Field(getValueField()->getPosition()));
		}

		// Fetch用のオプションを作る
		TreeNode::Fetch cFetch(cFetchFields);

		// Fetchできるかファイルドライバーに聞く
		return getSearchFile().getSearchParameter(&cFetch, cOpenOption_);
	}
	return false;
}

//	FUNCTION private
//	AccessFile::setSearchMode --
// 		キーまたはキーとバリューで検索するオープンモードをセットする
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataArrayData* pKey_
//			条件に用いるデータ
//		const Common::Data::Pointer* pValue_
//			キーだけでユニークにならない場合に追加するデータ
//		LogicalFile::OpenOption& cOpenOption_
//			オープンモードをセットする
//
//	RETURN
//		true...検索のオープンモードが正しくセットされた
//		false...検索はできない
//
//	EXCEPTIONS

bool
AccessFile::
setSearchMode(Common::DataArrayData* pKey_, const Common::Data::Pointer* pValue_, LogicalFile::OpenOption& cOpenOption_)
{
	; _SYDNEY_ASSERT(m_cFile.getIndexID() != Schema::Object::ID::Invalid);
	; _SYDNEY_ASSERT(m_eSearchMode == OpenMode::Search);
	; _SYDNEY_ASSERT(pKey_);
	; _SYDNEY_ASSERT(!isValueNeeded() || (pValue_ && pValue_->get()));

	// キーとなるフィールドの列を作る
	if (!m_pNodeFields) {
		m_pNodeFields = new ModVector<TreeNode::Field*>();
		const ModVector<Field*>& cKeyFields = getKeyFields();
		ModSize nKey = cKeyFields.getSize();
		m_pNodeFields->reserve(isValueNeeded() ? nKey + 1 : nKey);
		for (ModSize iKey = 0; iKey < nKey; ++iKey) {
			m_pNodeFields->pushBack(new TreeNode::Field(cKeyFields[iKey]->getPosition()));
		}
		if (isValueNeeded()) {
			// タプルを特定するのにバリューが必要である
			// 現在はこのような場合は索引のファイルだけなので
			// ROWIDのフィールドを使う
			; _SYDNEY_ASSERT(getValueField());
			m_pNodeFields->pushBack(new TreeNode::Field(getValueField()->getPosition()));
		}
	}
	// 条件を表すバリューの列を作る
	ModSize n = pKey_->getCount();
	if (!m_pNodeVariables) {
		m_pNodeVariables = new ModVector<TreeNode::Base*>(isValueNeeded() ? n + 1 : n);
	}
	// 指定された値にセットする
	for (ModSize i = 0; i < n; ++i) {
		if ((*m_pNodeVariables)[i]) delete (*m_pNodeVariables)[i];
		(*m_pNodeVariables)[i] = createVariable(pKey_->getElement(i));
	}
	if (isValueNeeded()) {
		// タプルを特定するのにバリューが必要である
		if ((*m_pNodeVariables)[n]) delete (*m_pNodeVariables)[n];
		(*m_pNodeVariables)[n] = new TreeNode::Variable(*pValue_);
	}
	; _SYDNEY_ASSERT(m_pNodeFields->getSize() == (isValueNeeded() ? n + 1 : n));
	; _SYDNEY_ASSERT(m_pNodeVariables->getSize() == (isValueNeeded() ? n + 1 : n));

	if (m_pCondition) delete m_pCondition;
	m_pCondition = getIdentifyCondition(*m_pNodeFields, *m_pNodeVariables);

/* TreeNodeの形を確認するときに生かす
#ifndef SYD_COVERAGE
	if (m_pCondition) {
		SydMessage << "AccessFile::getSearchParameter condition = " << m_pCondition->toString() << ModEndl;;
	}
#endif
*/

	return getSearchFile().getSearchParameter(m_pCondition, cOpenOption_);
}

//	FUNCTION private
//	AccessFile::setProjection --
// 		データフィールドの値を取得する設定をする
//
//	NOTES
//
//	ARGUMENTS
//		LogicalFile::OpenOption& cOpenOption_
//			オープンモードをセットする
//
//	RETURN
//		true...Projectionが正しくセットされた
//		false...Projectionはできない
//
//	EXCEPTIONS

bool
AccessFile::
setProjection(LogicalFile::OpenOption& cOpenOption_)
{
	if (!m_pProjection) {
		setFields();
	}
	TreeNode::List cProjection;
	ModSize n = m_pProjection->getSize();
	for (ModSize i = 0; i < n; ++i) {
		cProjection.addNode(new TreeNode::Field((*m_pProjection)[i]));
	}

	AutoLatch latch(m_cTrans, *m_pLockName);
	return getSearchFile().getProjectionParameter(&cProjection, cOpenOption_);
}

// FUNCTION public
//	Schema::AccessFile::createData -- フィールドに対応したCommon::Dataを作る
//
// NOTES
//
// ARGUMENTS
//	Field* pField_
//		このフィールドに対応したCommon::Dataを作る
//	
// RETURN
//	作成したCommon::Data
//
// EXCEPTIONS

Common::Data*
AccessFile::
createData(Field* pField_) const
{
	Column* pColumn = pField_->getRelatedColumn(m_cTrans);
	if (pColumn && !pColumn->isTupleID()) {
		return Common::DataInstance::create(pColumn->getType());
	} else {
		return Common::DataInstance::create(pField_->getType());
	}
}

//
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
