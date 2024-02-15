// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp -- 可変長レコードファイルクラスの実現ファイル
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2014, 2023 Ricoh Company, Ltd.
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

namespace
{
const char	srcFile[] = __FILE__;
const char	moduleName[] = "Record";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Record/File.h"

#include "Checkpoint/Database.h"
#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"

#include "FileCommon/FileOption.h"

#include "Exception/FakeError.h"
#include "Exception/BadArgument.h"
#include "Exception/FileAlreadyExisted.h"
#include "Exception/FileNotOpen.h"
#include "Exception/IllegalFileAccess.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "LogicalFile/Estimate.h"
#include "LogicalFile/TreeNodeInterface.h"
#include "LogicalFile/FileID.h"
#include "LogicalFile/OpenOption.h"

#include "Os/Path.h"
#include "PhysicalFile/Manager.h"

#include "Record/DirectFile.h"
#include "Record/FileInformation.h"
#include "Record/FreeAreaManager.h"
#include "Record/MetaData.h"
#include "Record/OpenOption.h"
#include "Record/OpenParameter.h"
#include "Record/PhysicalPosition.h"
#include "Record/TargetFields.h"
#include "Record/VariableFile.h"
#include "Record/UseInfo.h"
#include "Record/Message_VerifyFailed.h"
#include "Record/Message_DiscordObjectNum.h"
#include "Record/Message_ExistTopObject.h"
#include "Record/Message_ExistLastObject.h"
#include "Record/Message_VerifyPhysicalFileFinished.h"
#include "Record/Message_VerifyPhysicalFileStarted.h"
#include "Record/Debug.h"

_SYDNEY_USING

_SYDNEY_RECORD_USING

namespace
{
	const ModUnicodeString _cstrZero("0");

	namespace _ObjectID
	{
		const LogicalFile::ObjectID*
		_getID(const Common::Data* pKey_)
		{
			const LogicalFile::ObjectID* pID = 0;
			if (pKey_->getType() == Common::DataType::Array) {
				const Common::DataArrayData* pKey =
					_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pKey_);
				if (pKey->getElement(0)->getType() != LogicalFile::ObjectID().getType()) {
					_SYDNEY_THROW0(Exception::BadArgument);
				}
				pID = _SYDNEY_DYNAMIC_CAST(LogicalFile::ObjectID*, pKey->getElement(0).get());

			} else {
				if (pKey_->getType() != LogicalFile::ObjectID().getType()) {
					_SYDNEY_THROW0(Exception::BadArgument);
				}
				pID = _SYDNEY_DYNAMIC_CAST(const LogicalFile::ObjectID*, pKey_);
			}
			; _SYDNEY_ASSERT(pID);
			return pID;
		}
	}

	// スコープの前後でファイルを attach/detach する。
	class AutoFileAttacher : FileCommon::AutoObject
	{
	public:
		AutoFileAttacher(DirectFile* pDirectFile_ ,VariableFile* pVariableFile_)
			: m_cDirectFile(pDirectFile_)
			, m_cVariableFile(pVariableFile_)
		{
		}
		~AutoFileAttacher()
		{
			// m_cDirectFile, m_cVariableFile は自動的にデタッチ
		}
	private:
		AutoFile<DirectFile> m_cDirectFile;
		AutoFile<VariableFile> m_cVariableFile;
	};

	// スコープの最後にページを detachAll する。
	class AutoPageDetacher : FileCommon::AutoObject
	{
	public:
		AutoPageDetacher(DirectFile* pDirectFile_ ,VariableFile* pVariableFile_)
			: m_cDirectFile(pDirectFile_)
			, m_cVariableFile(pVariableFile_)
		{
		}
		~AutoPageDetacher()
		{
		}
		void succeeded() throw()
		{
			m_cDirectFile.succeeded();
			m_cVariableFile.succeeded();
		}
	private:
		AutoDetachPageAll<DirectFile> m_cDirectFile;
		AutoDetachPageAll<VariableFile> m_cVariableFile;
	};
}

//
// PUBLIC METHOD
//

//	FUNCTION public
//	Record::File::File -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID&	cFileOption_
//		可変長レコードファイルオプションオブジェクトへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS

File::
File(const LogicalFile::FileID&	cFileID_)
	: m_pTargetFields(0),
	  m_pDirectTargetFields(0),
	  m_pVariableTargetFields(0),
	  m_bTargetFieldsAllocated(false),
	  m_pDirectFile(0),
	  m_pVariableFile(0),
	  m_ullFetchObjectID(Tools::m_UndefinedObjectID),
	  m_pTransaction(0),
	  // メタデータをファイルオプションで初期化。
	  // レコードファイルがデストラクトされるまで
	  // 内容は変化しない。(ファイルIDだけはFile::createで変化する)
	  m_MetaData(cFileID_),
	  // オープンオプションを保持するオブジェクトを作成しておく
	  m_OpenParam(FileCommon::OpenMode::Read,
				  false, // ダミーの見積もりフラグ
				  true), // ダミーのソート順
	  m_DataPackage(m_MetaData)
{
	TRACEMSG("Record::File::constructor");
	MSGLIN( SydRecordTraceMessage << "Record::FileID: " << cFileID_.toString() << ModEndl );
}

//
//	FUNCTION public
//	Record::File::~File -- デストラクタ
//
//	NOTES
//	デストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
File::
~File()
{
	TRACEMSG("Record::File::destructor");
	delete m_pDirectFile, m_pDirectFile = 0;
	delete m_pVariableFile, m_pVariableFile = 0;

	if (m_bTargetFieldsAllocated) {
		delete m_pDirectTargetFields, m_pDirectTargetFields = 0;
		delete m_pVariableTargetFields, m_pVariableTargetFields = 0;
	} else {
		m_pDirectTargetFields = 0;
		m_pVariableTargetFields = 0;
	}
	if (m_pTargetFields != 0)
	{
		delete m_pTargetFields, m_pTargetFields = 0;
	}
}

//
//	FUNCTION public
//	Record::File::initializeManager -- 初期化処理
//
//	NOTES
//	初期化処理。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
//static
void
File::
initialize()
{
	; // do noting
}

//
//	FUNCTION public
//	Record::File::terminateManager -- 終了処理
//
//	NOTES
//	終了処理。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
//static
void
File::
terminate()
{
	; // do nothing
}

//
//	FUNCTION public
//	Record::File::getFileID -- ファイルIDを返す
//
//	NOTES
//	ファイルIDを返す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Logical::FileID&
//		自身がもつ論理ファイル ID オブジェクトへの参照
//
//	EXCEPTIONS
//
const LogicalFile::FileID&
File::getFileID() const
{
	TRACEMSG("Record::File::getFileID");
	return m_MetaData.m_FileID;
}

//
//	FUNCTION public
//	Record::File::getSize -- レコードファイルサイズを返す
//
//	NOTES
//	レコードファイルは、自身がもつ物理ファイルのファイルサイズを返す。
//	この関数を呼び出す場合、目的のレコードファイルが
//	オープンされていなければならない。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		レコードファイルサイズ [byte]
//
//	EXCEPTIONS
//	FileNotOpen
//		レコードファイルがオープンされていない
//
ModUInt64
File::
getSize() const
{
	TRACEMSG("Record::File::getSize");
	if (!m_pDirectFile || !m_pDirectFile->isAttached()) {
		_SYDNEY_THROW0(Exception::FileNotOpen);
	}

	if (!m_pVariableFile) {
		return m_pDirectFile->getSize();

	} else {
		; _SYDNEY_ASSERT(m_pVariableFile->isAttached());
		return m_pDirectFile->getSize() + m_pVariableFile->getSize();
	}
}

//
//	FUNCTION public
//	Record::File::getCount -- 挿入されているオブジェクト数を返す
//
//	NOTES
//	レコードファイルは、自身に挿入されているオブジェクトの総数を返す。
//	この関数を呼び出す場合、目的のレコードファイルが
//	オープンされていなければならない。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInt64
//		オブジェクトの総数
//
//	EXCEPTIONS
//	FileNotOpen
//		レコードファイルがオープンされていない
//
ModInt64
File::
getCount() const
{
	TRACEMSG("Record::File::getCount");
	//
	// レコードファイルがオープンされているかチェック
	//
	if (!m_pDirectFile || !m_pDirectFile->isAttached()) {
		_SYDNEY_THROW0(Exception::FileNotOpen);
	}

	AutoPageDetacher page( m_pDirectFile ,m_pVariableFile );// スコープの最後でdetachPageAllする

	// オブジェクト数は固定長用ファイルから得る
	ModInt64	cnt = m_pDirectFile->getCount();// attach あり

	page.succeeded();

	return cnt;
}

//
//	FUNCTION public
//	Record::File::getOverhead -- オブジェクト検索時のオーバヘッドを返す
//
//	NOTES
//	レコードファイルは、オブジェクト検索時のオーバヘッドの概算を秒数で返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	double
//		オブジェクト検索時のオーバヘッド [秒]
//
//	EXCEPTIONS
//
double
File::
getOverhead() const
{
	TRACEMSG("Record::File::getOverhead");
	if (!m_pDirectFile || !m_pDirectFile->isAttached()) {
		_SYDNEY_THROW0(Exception::FileNotOpen);
	}

	if (!m_pVariableFile) {
		return m_pDirectFile->getOverhead();

	} else {
		; _SYDNEY_ASSERT(m_pVariableFile->isAttached());
		return m_pDirectFile->getOverhead() + m_pVariableFile->getOverhead();
	}
}

//
//	FUNCTION public
//	Record::File::getProcessCost --
//		オブジェクトへアクセスする際のプロセスコストを返す
//
//	NOTES
//	レコードファイルは、自身に挿入されているオブジェクトへ
//	アクセスする際のプロセスコストを秒数で返す。
//	プロセスコストはオープンモードにより異なるため、
//	この関数を呼び出す場合、目的のレコードファイルが
//	オープンされていなければならない。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	double
//		プロセスコスト [秒]
//
//	EXCEPTIONS
//	FileNotOpen
//		レコードファイルがオープンされていない
//
double
File::
getProcessCost() const
{
	TRACEMSG("Record::File::getProcessCost");
	if (!m_pDirectFile || !m_pDirectFile->isAttached()) {
		_SYDNEY_THROW0(Exception::FileNotOpen);
	}

	double	pc = 0.0;

	AutoPageDetacher page( m_pDirectFile ,m_pVariableFile );// スコープの最後でdetachPageAllする

	if (!m_pVariableFile) {
		pc = m_pDirectFile->getProcessCost();

	} else {
		; _SYDNEY_ASSERT(m_pVariableFile->isAttached());
		pc = m_pDirectFile->getProcessCost()
			+ m_pVariableFile->getProcessCost(m_pDirectFile->getCount());// getCount()でattach
	}

	page.succeeded();

	return pc;
}

//
//	FUNCTION public
//	Record::File::getSearchParameter -- 検索オープンパラメータを設定する
//
//	NOTES
//	レコードファイルから get によりオブジェクトを取得する場合、
//	  ・オブジェクトを先頭から順次取得する
//	  ・fetch の引数でオブジェクト ID を指定し、特定のオブジェクトを取得する
//	以上、2 つの方法でオブジェクトを取得可能である。
//	このことから、レコードファイルで解析可能な検索条件（引数 pCondition_ ）は
//	Fetch ノードを唯一のノードとして持つ木構造のみである。
//	また、pCondition_ に 0 が指定されている場合、レコードファイルは
//	オブジェクトを先頭から順次取得と見なす。
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*	pCondition_
//		木構造の検索条件オブジェクトへのポインタ
//	LogicalFile::OpenOption&						cOpenOption_
//		レコードファイルオープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		引数 pCondition_ で示される検索が可能ならば true を、
//		そうでない場合には false を返す。
//
//	EXCEPTIONS
//	なし
//
bool
File::
getSearchParameter(
	const LogicalFile::TreeNodeInterface*	pCondition_,
	LogicalFile::OpenOption&						cOpenOption_) const
{
	TRACEMSG("Record::File::getSearchParameter");
	//検索条件と一致するすべてのオブジェクトをドライバ側で保持しない
	cOpenOption_.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::CacheAllObject::Key), false);
	if (pCondition_ == 0)
	{
		//
		// SCAN モード
		// （ get でオブジェクトを順次取得するモード）
		//

		// オープンオプション(参照引数)にオープンモードを設定
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), FileCommon::OpenOption::OpenMode::Read);

		return true;
	}

	if (pCondition_->getType() == LogicalFile::TreeNodeInterface::Fetch)
	{
		//
		// FETCH モード
		// （ fetch の引数でオブジェクト ID を指定するモード）
		//

		if (pCondition_->getOptionSize() != 2)
		{
			// 渡される要素は必ず２つ、ドライバは先頭要素だけ見ればよい
			_SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		// 第一要素がFetchされるカラムリスト
		const LogicalFile::TreeNodeInterface* pFetchedFields = pCondition_->getOptionAt(0);

		// リストは自身をオペランドとして扱う
		if (pFetchedFields->getOperandSize() != 1)
		{
			// オペランド数が 1 以外ということは「オブジェクト ID 以外で
			// Fetch が可能ですか？」と質問していることになります。
			// レコードファイルはオブジェクト ID でのみオブジェクトの Fetch
			// が可能なので、レコードファイルは「いいえ、できません。」
			// という意味の値を返せば良い。
			return false;
		}

		const LogicalFile::TreeNodeInterface* pField = pFetchedFields->getOperandAt(0);
		if (pField->getType() != LogicalFile::TreeNodeInterface::Field ||
			pField->getValue() != _cstrZero)
		{
			return false;
		}

		// オープンオプションに既にオープンモードが設定されていた場合、
		// それは "Read" でなければいけない。そうでなければ例外。
		int 	iValue;
		const bool bFind = cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), iValue);
		if (bFind && iValue != FileCommon::OpenOption::OpenMode::Read)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		// オープンオプション(参照引数)にオープンモードを設定
		if (!bFind)
		{
			cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), _SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::OpenMode::Read));
		}

		return true;
	}

	return false;
}

//
//	FUNCTION public
//	Record::File::getProjectionParameter -- プロジェクションオープンパラメータを設定する
//
//	NOTES
//	レコードファイルは、引数 cProjection_ で指定されている
//	1 つ以上のフィールド番号を読みとり、
//	オブジェクト取得時には、該当するフィールドのみで
//	オブジェクトを構成するようにオープンオプションを設定する。
//	例えば、4 つのフィールド（オブジェクト ID フィールド含む）
//	で構成されるオブジェクトを挿入するためのレコードファイルから
//	オブジェクトを取得する際に、第 2 、第 3 フィールドのみを
//	取得するのであれば、引数 cProjection_ は、下図のように設定する。
//
//	      cProjection_
//	   ┌───────┐
//	   │     ┌──┐ │
//	   │ [0] │  2 │ │
//	   │     ├──┤ │
//	   │ [1] │  3 │ │
//	   │     └──┘ │
//	   └───────┘
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	cProjection_
//		フィールド番号の配列オブジェクトへの参照
//	LogicalFile::OpenOption&				cOpenOption_
//		オープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		引数 cProjection_ で示されるフィールドでオブジェクトを構成可能ならば
//		true を、そうでない場合には false を返す。
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//
bool
File::
getProjectionParameter(
	const Common::IntegerArrayData&	cProjection_,
	LogicalFile::OpenOption&				cOpenOption_) const
{
	TRACEMSG("Record::File::getProjectionParameter");
	// オープンオプションに既にオープンモードが設定されていた場合、
	// それは "Read" でなければいけない。そうでなければ例外。
	int 	iValue;
	const bool bFind = cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), iValue);

	if (bFind && iValue != FileCommon::OpenOption::OpenMode::Read)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	if ( cOpenOption_.getBoolean( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::GetByBitSet::Key)) )
	{
		// レコードファイルは挿入されているオブジェクトを
		// ビットセットで返せない。
		return false;
	}

	// オープンオプション(参照引数)にオープンモードを設定
	if (!bFind)
	{
		cOpenOption_.setInteger( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), _SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::OpenMode::Read));
	}

	// フィールド選択指定がされている、という事を設定する
	cOpenOption_.setBoolean( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::FieldSelect::Key), true);

	// フィールド選択指定を設定する

	const int iFieldNum = cProjection_.getCount();
	cOpenOption_.setInteger( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::TargetFieldNumber::Key), iFieldNum);

	// オープンオプションに選択されているフィールドの番号を設定

	for (int i = 0; i < iFieldNum; ++i)
		cOpenOption_.setInteger(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				FileCommon::OpenOption::TargetFieldIndex::Key, i),
			cProjection_.getElement(i));

	return true;
}

//
//	FUNCTION public
//	Record::File::getUpdateParameter -- 更新オープンパラメータを設定する
//
//	NOTES
//	更新モードでのレコードファイルオープンオプションを設定する。
//	レコードファイルは、引数 cUpdateFields_ で指定されている
//	1 つ以上のフィールド番号を読みとり、
//	オブジェクト更新時には、該当するフィールドのみを
//	更新するようにオープンオプションを設定する。
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	cProjection_
//		フィールド番号の配列オブジェクトへの参照
//	LogicalFile::OpenOption&				cOpenOption_
//		オープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		レコードファイルの場合には、常に true を返す。
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//	[YET!]
//
bool
File::
getUpdateParameter(
	const Common::IntegerArrayData&	cProjection_,
	LogicalFile::OpenOption&				cOpenOption_) const
{
	TRACEMSG("Record::File::getUpdateParameter");
	// オープンオプションに既にオープンモードが設定されていた場合、
	// それは "Update" でなければいけない。そうでなければ例外。
	int 	iValue;
	const bool bFind = cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), iValue);
	if (bFind && iValue != FileCommon::OpenOption::OpenMode::Update)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// オープンオプション(参照引数)にオープンモードを設定
	if (!bFind)
	{
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), _SYDNEY_OPEN_PARAMETER_VALUE(FileCommon::OpenOption::OpenMode::Update));
	}

	// フィールド選択指定がされている、という事を設定する
	cOpenOption_.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::FieldSelect::Key), true);

	// フィールド選択指定を設定する
	const int iFieldNum = cProjection_.getCount();
	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::TargetFieldNumber::Key), iFieldNum);

	for (int i = 0; i < iFieldNum; ++i)
	{
		// 更新するフィールドの番号を取得。
		const int iPosition = cProjection_.getElement(i);
		if (iPosition == 0)
		{
			// 第0フィールド(オブジェクトID)は変更不能である、という仕様
			// なので false を返す
			return false;
		}

		// オープンオプションに選択されているフィールドの番号を設定

		cOpenOption_.setInteger(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				FileCommon::OpenOption::TargetFieldIndex::Key, i), iPosition);
	}

	return true;
}

//
//	FUNCTION public
//	Record::File::getSortParameter -- ソート順パラメータを設定する
//
//	NOTES
//	レコードファイルに挿入されているオブジェクトを取得する際、
//	引数 cKeys_ で指定されたフィールドと
//	引数 cOrders_ で指定されたソート順で取得可能ならば、
//	引数 cOpenOption_ に必要なパラメータを設定する。
//	レコードファイルはオブジェクト ID によりソートされているので、
//	オブジェクト ID フィールド以外のフィールドへのソート順の指定は
//	許可しない。従って、下図のような引数の場合に true を返し、
//	そうでなければfalseを返す。
//
//	    引数 cKeys_
//
//	    ┌  Common::IntegerArrayData オブジェクト   ┐
//	    │       ┌─────────────┐     │
//	    │   [0] │            0             │     │
//	    │       └─────────────┘     │
//	    └─────────────────────┘
//
//	    引数 cOrders_
//
//	    ┌  Common::IntegerArrayData オブジェクト   ┐
//	    │       ┌─────────────┐     │
//	    │   [0] │       0 または 1         │     │
//	    │       └─────────────┘     │
//	    └─────────────────────┘
//
//	ARGUMENTS
//	const Common::IntegerArrayData&	cKeys_
//		ソート順を指定するフィールドインデックスの列への参照
//	const Common::IntegerArrayData&	cOrders_
//		引数cKeys_で指定されたフィールドのソート順の列への参照
//		昇順ならば0を、降順ならば1を設定する。
//	LogicalFile::OpenOption&				cOpenOption_
//		レコードファイルオープンオプションオブジェクトへの参照
//
//	RETURN
//	bool
//		オブジェクトIDフィールドのみのソート順が
//		指定されている場合には true を、
//		オブジェクトIDフィールド以外のフィールドのソート順が
//		指定されている場合には false を返す。
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//
bool
File::
getSortParameter(
	const Common::IntegerArrayData&	cKeys_,
	const Common::IntegerArrayData&	cOrders_,
	LogicalFile::OpenOption&				cOpenOption_) const
{
	TRACEMSG("Record::File::getSortParameter");
	if (cKeys_.getCount() != cOrders_.getCount())
	{
		// 不正な引数
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	//
	// オブジェクト ID フィールドのみが指定されているかチェックする
	//
	if (cKeys_.getCount() == 1 && cKeys_.getElement(0) == 0)
	{
		//
		// オープンオプションに SortOrder パラメータを設定する
		//
		bool	iSortOrder = (cOrders_.getElement(0) != 0);
		cOpenOption_.setBoolean(_SYDNEY_RECORD_OPEN_PARAMETER_KEY(Record::OpenOption::SortOrder::Key), iSortOrder);

		return true;
	}

	return false;
}

//
//	FUNCTION public
//	Record::File::create -- レコードファイルを生成する
//
//	NOTES
//	レコードファイルは、自身がもつ物理ファイルを生成し、
//	生成した物理ファイルの初期化を行なう。
//
//	ARGUMENTS
//	const Trans::Transaction&	cTransaction_
//		トランザクション記述子
//
//	RETURN
//	const LogicalFile::FileID&
//		自身がもつ論理ファイル ID オブジェクトへの参照
//
//	EXCEPTIONS
//	FileAlreadyExisted
//		レコードファイルが既に存在する
//	[YET!]
//		ファイルオプションに設定されるべきパラメータが設定されていない
//	Unexpected
//		予想外のエラー
//	[YET!]
//
const LogicalFile::FileID&
File::
create(const Trans::Transaction& cTransaction_)
{
	TRACEMSG("Record::File::create");

	// FileIDを設定する
	m_MetaData.create();
	
	// 固定長/可変長用ファイルをアクセスするためのオブジェクトを作成する
	initializeFiles(cTransaction_);

	// ファイルの実体を作成する
	enum {
		None,
		Direct,
		Variable,
		ValueNum
	} eStatus = None;

	try {
		m_pDirectFile->create();
		eStatus = Direct;
		if (m_pVariableFile) {
			m_pVariableFile->create();
			eStatus = Variable;
		}

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		try {
			switch (eStatus) {
			case Variable:
				m_pVariableFile->destroy();
				// thru
			case Direct:
				m_pDirectFile->destroy();
				// thru
			case None:
			default:
				break;
			}
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{

			// 元に戻せなかったので、利用不可にする
			
			Checkpoint::Database::setAvailability(
				m_MetaData.getLockName(), false);
		}
		_SYDNEY_RETHROW;
	}

	//生成直後は mounted
	m_MetaData.m_FileID.setBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key), true);
	//ドライバのバージョン
	m_MetaData.m_FileID.setInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Version::Key), FileInformation::Vers::CurrentVersion);

	return m_MetaData.m_FileID;
}

//
//	FUNCTION public
//	Record::File::destroy -- レコードファイルを破棄する
//
//	NOTES
//	レコードファイルは、自身がもつ物理ファイルを破棄する。
//
//	ARGUMENTS
//	const Trans::Transaction&	cTransaction_
//		トランザクション記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::
destroy(const Trans::Transaction&	cTransaction_)
{
	TRACEMSG("Record::File::destroy");
	// 固定長/可変長用ファイルをアクセスするためのオブジェクトを作成する
	initializeFiles(cTransaction_);

	m_pDirectFile->destroy();
	if (m_pVariableFile) {
		m_pVariableFile->destroy();
	}
}

//
//	FUNCTION public
//	Record::File::isAccessible --
//		実体である OS ファイルが存在するか調べる
//
//	NOTES
//	レコードファイルはオープンされていても、
//	オープンされていなくても構わない。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		生成されているかどうか
//			true  : 生成されている
//			false : 生成されていない
//
//	EXCEPTIONS
//	[YET!]
//
bool
File::
isAccessible(bool force) const
{
	TRACEMSG("Record::File::isAccessible");

	if (!m_pDirectFile) {

		// openされていないので一時的なオブジェクトを作る
		// 0を指す参照を渡すが内部では使われないので大丈夫

		Trans::Transaction* trans = 0;
		return DirectFile(*trans, m_MetaData).isAccessible(force) &&
			(!m_MetaData.hasVariable() ||
			 VariableFile(*trans, m_MetaData).isAccessible(force));
	} else
		return m_pDirectFile->isAccessible(force) &&
			(!m_pVariableFile || m_pVariableFile->isAccessible(force));
}

//	FUNCTION public
//	Record::File::isMounted -- マウントされているか調べる
//
//	NOTES
//		レコードファイルはオープンされていなくてもかまわない
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			マウントされているか調べる
//			トランザクションのトランザクション記述子
//
//	RETURN
//		true
//			マウントされている
//		false
//			マウントされていない
//
//	EXCEPTIONS

bool
File::isMounted(const Trans::Transaction& trans) const
{
	return (!m_pDirectFile) ?
		(DirectFile(trans, m_MetaData).isMounted(trans) &&
		 (!m_MetaData.hasVariable() ||
		  VariableFile(trans, m_MetaData).isMounted(trans))) :
		(m_pDirectFile->isMounted(trans) &&
		 (!m_pVariableFile || m_pVariableFile->isMounted(trans)));
}

//
//	FUNCTION public
//	Record::File::open -- レコードファイルをオープンする
//
//	NOTES
//	レコードファイルをオープンする。
//
//	ARGUMENTS
//	const Trans::Transaction&	cTransaction_
//		トランザクション記述子
//	const LogicalFile::OpenOption&	cOpenOption_
//		オープンオプションオブジェクトへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//	NotSupported
//		既にオープンされている
//	[YET!]
//
void
File::
open(const Trans::Transaction&	cTransaction_,
	 const LogicalFile::OpenOption&		cOpenOption_)
{
	TRACEMSG("Record::File::open");
	MSGLIN( SydRecordTraceMessage << "Record::OpenOption: " << cOpenOption_.toString() << ModEndl );

	// オープンモードをデータメンバに保存
	int	iOpenModeValue = cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key));
	if (iOpenModeValue == FileCommon::OpenOption::OpenMode::Initialize)
	{
		;//nop 無視する
		return;
	}

	// 固定長/可変長用ファイルをアクセスするためのオブジェクトを作成する
	initializeFiles(cTransaction_);

	// 既にオープンされている？
	if (m_pDirectFile->isAttached()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	_SYDNEY_ASSERT(m_pTransaction == 0);

	// オープンモードを保存
	saveOpenOption(cOpenOption_ ,iOpenModeValue);

	// 物理ファイルをオープン
	m_pDirectFile->attachFile(&m_OpenParam);
	if (m_pVariableFile) {
		m_pVariableFile->attachFile(&m_OpenParam);
	}

	// トランザクション記述子を保持しておく
	m_pTransaction = &cTransaction_;
}

//
//	FUNCTION public
//	Record::File::close -- レコードファイルをクローズする
//
//	NOTES
//	レコードファイルをクローズする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::
close()
{
	TRACEMSG("Record::File::close");
	if (!m_pDirectFile) {
		// まだオープンされていない
		// またはすでにクローズされている
		return;
	}

	//
	// オープンされている場合は全てのリソースを解放する
	//

	delete m_pTargetFields, m_pTargetFields = 0;
	if (m_pVariableFile) {
		delete m_pDirectTargetFields, m_pDirectTargetFields = 0;
		delete m_pVariableTargetFields, m_pVariableTargetFields = 0;

		if (m_pTransaction == 0 || m_pTransaction->isNoVersion())
			m_pVariableFile->detachPageAll(true);

	} else {
		m_pDirectTargetFields = 0;
	}

	if (m_pTransaction == 0 || m_pTransaction->isNoVersion())
		m_pDirectFile->detachPageAll(true);

	delete m_pDirectFile, m_pDirectFile = 0;
	delete m_pVariableFile, m_pVariableFile = 0;

	m_pTransaction = 0;

	// 注意:
	// - クローズしてもファイルオプション(メタデータ)は変化しない。
	// - クローズするとオープンオプションは変化する。
}

//
//	FUNCTION public
//	Record::File::fetch -- 検索条件 (オブジェクトID) を設定する
//
//	NOTES
//	検索条件 (オブジェクトID) を設定する。
//	データは get で求める。
//
//	ARGUMENTS
//	const Common::Object*	pOption_
//		オプションオブジェクトへのポインタ（省略可）
//		レコードファイルの fetch へのオプションは、オブジェクト ID である。
//		この引数には、オブジェクト ID を値としてもつ
//		LogicalFile::ObjectID (= Common::ObjectIDData) クラスの
//		インスタンスオブジェクト、または、それを唯一の要素として持つ
//		Common::DataArrayData クラスのインスタンスオブジェクト
//		でなければならない。
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileNotOpen
//		レコードファイルがオープンされていない
//	IllegalFileAccess
//		不正な論理ファイルへのアクセス
//	[YET!]
//
void
File::fetch(const Common::DataArrayData* pOption_)
{
	TRACEMSG("Record::File::fetch");
	if (!m_pDirectFile || !m_pDirectFile->isAttached()) {
		// レコードファイルがオープンされていない
		_SYDNEY_THROW0(Exception::FileNotOpen);
	}

	if (m_OpenParam.m_iOpenMode != FileCommon::OpenMode::Read)
	{
		// オープンモードが間違っている
		_SYDNEY_THROW0(Exception::IllegalFileAccess);
	}

	// 指定されているオブジェクトIDを記録しておき、最初の get で
	// 反復子の生成を行う。
	// (指定方法が間違っている場合はオブジェクトIDの取りだし中に例外発生)
	m_ullFetchObjectID = convertFetchOptionToObjectID(pOption_);
}

//
//	FUNCTION public
//	Record::File::get -- 挿入されているオブジェクトを返す
//
//	NOTES
//	挿入されているオブジェクトを返す。
//	カーソルがさしているオブジェクトを返す。
//	fetch で指定されたオブジェクトIDが存在しないものであった場合 0 を返す。
//
//	ARGUMENTS
//	Common::DataArrayData* pTuple_
//		格納する配列
//
//	RETURN
//	bool
//		結果が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	FileNotOpen
//		レコードファイルがオープンされていない
//	IllegalFileAccess
//		不正な論理ファイルへのアクセス
//	BadArgument
//		不正な引数
//	[YET!]
//
bool
File::get(Common::DataArrayData* pTuple_)
{
	bool result = false;

	try
	{
		TRACEMSG("Record::File::get");
		if (!m_pDirectFile || !m_pDirectFile->isAttached()) {
			// オープンされていない
			_SYDNEY_THROW0(Exception::FileNotOpen);
		}

		//
		// オープンモードのチェック
		//
		if (m_OpenParam.m_iOpenMode != FileCommon::OpenMode::Read)
		{
			_SYDNEY_THROW0(Exception::IllegalFileAccess);
		}

		DirectFile::DataPackage& cData = m_DataPackage;
		cData.allocate(pTuple_);

		// ターゲットを固定長用と可変長用に分ける
		divideTargets();

		// スコープの最後でdetachPageAllする
		DirectFile* direct = m_pDirectFile;
		VariableFile* variable = m_pVariableFile;
		if (direct->getTransaction().isNoVersion() == false)
		{
			// 版を使用する検索の場合は、いちいちdetachしない
			direct = 0;
			variable = 0;
		}
		AutoPageDetacher page(direct, variable);

		// 固定長のデータを取得
		// 可変長がすべてNULLだったときに読む必要がないよう、
		// ターゲットはすべてのフィールドを渡す
		result = m_pDirectFile->read(m_ullFetchObjectID,
									 m_pTargetFields,
									 cData);

		if (result && m_pVariableFile && m_pVariableTargetFields->getSize()) {
			// 可変長も読み込む
			m_pVariableFile->prepareFreeAreaManager();
			m_pVariableFile->read(cData, m_pVariableTargetFields);
			if (m_pVariableFile->getTransaction().isNoVersion())
				m_pVariableFile->discardFreeAreaManager();
		}

		// 読み込んだらFetchの指定をクリアしておく
		m_ullFetchObjectID = Tools::m_UndefinedObjectID;

#ifdef DEBUG
		if (result) {
			SydRecordDebugMessage
				<< "File::get() result: "
				<< cData.get()->getString()
				<< ModEndl;
		}
#endif

		cData.free();
		page.succeeded();
	}
	catch (...)
	{
		SydMessage << m_MetaData.getDirectoryPath() << ModEndl;
		_SYDNEY_RETHROW;
	}

	// 読み込んだオブジェクトを返すデータ配列を作成
	return result;
}

//
//	FUNCTION public
//	Record::File::insert -- オブジェクトを挿入する
//
//	NOTES
//	オブジェクトを挿入する。
//
//	ARGUMENTS
//	Common::DataArrayData*	pTuple_
//		レコードファイルへ挿入するオブジェクトなどへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileNotOpen
//		レコードファイルがオープンされていない
//	IllegalFileAccess
//		不正な論理ファイルへのアクセス
//	[YET!]
//
void
File::insert(Common::DataArrayData* pTuple_)
{
	try
	{
		TRACEMSG("Record::File::insert");
		if (!m_pDirectFile || !m_pDirectFile->isAttached()) {
			// オープンされていない
			_SYDNEY_THROW0(Exception::FileNotOpen);
		}

		if ((m_OpenParam.m_iOpenMode != FileCommon::OpenMode::Update)
			&&
			(m_OpenParam.m_iOpenMode != FileCommon::OpenMode::Batch)) {
			// オープンモードが間違っている
			_SYDNEY_THROW0(Exception::IllegalFileAccess);
		}

		; _SYDNEY_ASSERT(pTuple_ != 0);

		if (pTuple_->getElementType() !=
			Common::DataType::Data ||
			pTuple_->getCount() < 1)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}

#ifdef OBSOLETE	// 呼び出し側がデータをセットしているのでここでは不要

		// 第一フィールドに空のObjectIDを入れる
		pTuple_->setElement(0, new LogicalFile::ObjectID());
#endif

		// 固定長用ファイルのためのデータを作る
		// 型の検査はこの中で行われる
		DirectFile::DataPackage& cData = m_DataPackage;
		cData.setData(pTuple_);

		// ObjectIDを格納するフィールドを得る
		LogicalFile::ObjectID* pID =
			_SYDNEY_DYNAMIC_CAST(LogicalFile::ObjectID*,
								 pTuple_->getElement(0).get());
		// 型の検査の後なので変換できないはずがない
		; _SYDNEY_ASSERT(pID);

		bool bIsBatch
			= (m_OpenParam.m_iOpenMode == FileCommon::OpenMode::Batch);
		// スコープの最後でdetachPageAllする
		AutoPageDetacher page( bIsBatch ? 0 : m_pDirectFile,
							   bIsBatch ? 0 : m_pVariableFile );

		// 挿入処理を行う
		Tools::ObjectID iDirectID = doInsert(cData);

		// オブジェクトIDを設定する
		pID->setValue(iDirectID);

		page.succeeded();

#ifdef DEBUG
		SydRecordDebugMessage
			<< "File::insert() result: "
			<< pTuple_->getString()
			<< ModEndl;
#endif
	}
	catch (...)
	{
		SydMessage << m_MetaData.getDirectoryPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Record::File::update -- オブジェクトを更新する
//
//	NOTES
//	オブジェクトを更新する。
//
//	ARGUMENTS
//	const Common::DataArrayData*	pKey_
//		更新するオブジェクトを指定するオブジェクトID
//		pObject_ の先頭の要素にも設定されている
//	Common::DataArrayData*	pObject_
//		更新するオブジェクトなどへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileNotOpen
//		レコードファイルがオープンされていない
//	IllegalFileAccess
//		不正な論理ファイルへのアクセス
//	[YET!]
//
void
File::update(const Common::DataArrayData* pKey_, Common::DataArrayData* pTuple_)
{
	try
	{
		TRACEMSG("Record::File::update");
		if (!m_pDirectFile || !m_pDirectFile->isAttached()) {
			// オープンされていない
			_SYDNEY_THROW0(Exception::FileNotOpen);
		}

		if (m_OpenParam.m_iOpenMode != FileCommon::OpenMode::Update) {
			// オープンモードが間違っている
			_SYDNEY_THROW0(Exception::IllegalFileAccess);
		}

		; _SYDNEY_ASSERT(pTuple_);

#ifdef DEBUG
		SydRecordDebugMessage
			<< "File::update() key: "
			<< pKey_->getString()
			<< " data: "
			<< pTuple_->getString()
			<< ModEndl;
#endif

		// キーはObjectIDである
		const LogicalFile::ObjectID* pID = _ObjectID::_getID(pKey_);

		// ターゲットを固定長用と可変長用に分ける
		// 同時に固定長用のターゲットからObjectIDを除外する
		divideTargets(true /* isUpdate */);

		// スコープの最後でdetachPageAllする
		AutoPageDetacher page( m_pDirectFile ,m_pVariableFile );

		// 更新前のnull bitmapと可変長オブジェクトIDを取得する
		DirectFile::DataPackage cOldObjectHeader(m_MetaData);
		m_pDirectFile->readObjectHeader(pID->getValue(), cOldObjectHeader);

		// 固定長部分について更新後のデータパッケージを作る
		// 型の検査はこの中で行われる
		DirectFile::DataPackage cNewDirectData(m_MetaData,
											   m_pDirectTargetFields);
		// 更新前のnull bitmapを入れてからデータを設定する
		cNewDirectData.setNullBitMap(cOldObjectHeader.getNullBitMap());
		cNewDirectData.setData(pTuple_);

		if (!m_pVariableFile) {
			// 可変長なしの場合

			doUpdate(pID->getValue(), cNewDirectData);

		} else {
			// 可変長ありの場合

			// 可変長部分のデータパッケージを作る
			DirectFile::DataPackage cNewVariableData(m_MetaData,
													 m_pVariableTargetFields,
													 pTuple_);
			// 可変データ部分のnull bitmap を設定する。
			cNewDirectData.mergeData(cNewVariableData);

			doUpdate(pID->getValue(), cOldObjectHeader,
					 cNewDirectData, cNewVariableData);
		}

		page.succeeded();
	}
	catch (...)
	{
		SydMessage << m_MetaData.getDirectoryPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Record::File::expunge -- オブジェクトを削除する
//
//	NOTES
//	オブジェクトを削除する。
//
//	ARGUMENTS
//	const Common::DataArrayData*	pKey_
//		削除するオブジェクトを指定するオブジェクトID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileNotOpen
//		レコードファイルがオープンされていない
//	IllegalFileAccess
//		不正な論理ファイルへのアクセス
//	[YET!]
//
void
File::expunge(const Common::DataArrayData* pKey_)
{
	try
	{
		TRACEMSG("Record::File::expunge");
		if (!m_pDirectFile || !m_pDirectFile->isAttached()) {
			// オープンされていない
			_SYDNEY_THROW0(Exception::FileNotOpen);
		}

		if (m_OpenParam.m_iOpenMode != FileCommon::OpenMode::Update) {
			// オープンモードが間違っている
			_SYDNEY_THROW0(Exception::IllegalFileAccess);
		}

#ifdef DEBUG
		SydRecordDebugMessage
			<< "File::expunge() key: "
			<< pKey_->getString()
			<< ModEndl;
#endif

		// キーはObjectIDである
		const LogicalFile::ObjectID* pID = _ObjectID::_getID(pKey_);

		AutoPageDetacher page( m_pDirectFile ,m_pVariableFile );// スコープの最後でdetachPageAllする

		// 削除対象の可変長オブジェクトIDを取得する
		Tools::ObjectID iVariableID =
			m_pDirectFile->readVariableID(pID->getValue());

		// 削除を行う
		doExpunge(pID->getValue(), iVariableID);

		page.succeeded();
	}
	catch (...)
	{
		SydMessage << m_MetaData.getDirectoryPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Record::File::mark -- 巻き戻しの位置を記録する
//
//	NOTES
//	巻き戻しの位置を記録する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
File::
mark()
{
	TRACEMSG("Record::File::mark");
	; _SYDNEY_ASSERT(m_pDirectFile);
	// markは代表オブジェクトファイルに対して行えばよい
	m_pDirectFile->mark();
}

//
//	FUNCTION public
//	Record::File::rewind -- 記録した位置に戻る
//
//	NOTES
//	巻き戻しで記録した位置に戻る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileNotOpen
//		レコードファイルがオープンされていない
//	[YET!]
//
void
File::
rewind()
{
	TRACEMSG("Record::File::rewind");
	; _SYDNEY_ASSERT(m_pDirectFile);

	AutoPageDetacher page( m_pDirectFile ,m_pVariableFile );// スコープの最後でdetachPageAllする

	// rewindは代表オブジェクトファイルに対して行えばよい
	m_pDirectFile->rewind();

	page.succeeded();
}

//
//	FUNCTION public
//	Record::File::reset -- カーソルをリセットする
//
//	NOTES
//	カーソルをリセットする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileNotOpen
//		レコードファイルがオープンされていない
//	[YET!]
//
void
File::
reset()
{
	TRACEMSG("Record::File::reset");
	; _SYDNEY_ASSERT(m_pDirectFile);
	// resetは代表オブジェクトファイルに対して行えばよい
	m_pDirectFile->reset();
}

//
//	FUNCTION public
//	Record::File::equals -- 比較
//
//	NOTES
//	自身と引数 pOther_ の比較を行ない、比較結果を返す。
//	※ 同一オブジェクトかをチェックするのではなく、
//	   それぞれがもつメンバが等しいか（同値か）をチェックする。
//  ※ すべての値を比較する訳ではないことに注意。
//  
//	ARGUMENTS
//	const Common::Object*	pOther_
//		比較対象オブジェクトへのポインタ
//
//	RETURN
//	bool
//		自身と引数 pOther_ が同値ならば true を、
//		そうでなければ false を返す。
//
//	EXCEPTIONS
//	なし
//
bool
File::
equals(const Common::Object*	pOther_) const
{
	TRACEMSG("Record::File::equals");
	//
	// 引数 pOther_ がヌルポインタならば等しくない
	//
	if (pOther_ == 0)
	{
		return false;
	}

	//
	// 引数 pOther_ が Record::File クラスの
	// インスタンスオブジェクトではないということは
	// 等しくない
	//
	const Record::File*	pOther = 0;
	if ((pOther = dynamic_cast<const Record::File*>(pOther_)) == 0)
	{
		return false;
	}

	// メタデータが異なっていれば、両者は異なっているとみなす
	// (「メタデータが異なっている」という判定は MetaData クラスの実装に
	//   依存している)
	if (!m_MetaData.equals(pOther->m_MetaData))
	{
		return false;
	}

	return true;
}

//	FUNCTION public
//	Record::File::sync -- レコードファイルの同期をとる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			レコードファイルの同期を取る
//			トランザクションのトランザクション記述子
//		bool&				incomplete
//			true
//				今回の同期処理でレコードファイルを持つ
//				オブジェクトの一部に処理し残しがある
//			false
//				今回の同期処理でレコードファイルを持つ
//				オブジェクトを完全に処理してきている
//
//				同期処理の結果、レコードファイルを処理し残したかを設定する
//		bool&				modified
//			true
//				今回の同期処理でレコードファイルを持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の同期処理でレコードファイルを持つ
//				オブジェクトはまだ更新されていない
//
//				同期処理の結果、レコードファイルが更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::sync(const Trans::Transaction& trans, bool& incomplete, bool& modified)
{
	try
	{
		TRACEMSG("Record::File::sync");
		// 固定長/可変長用ファイルをアクセスするためのオブジェクトを作成する
		initializeFiles(trans);

		if (m_pDirectFile)
			m_pDirectFile->sync(trans, incomplete, modified);
		if (m_pVariableFile)
			m_pVariableFile->sync(trans, incomplete, modified);
	}
	catch (...)
	{
		SydMessage << m_MetaData.getDirectoryPath() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

//
//	Utility
//

//
//	FUNCTION public
//	Record::File::move -- ファイルを移動する
//
//	NOTES
//  ファイルを移動する。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const Common::StringArrayData&	Area_
//		移動後のレコードファイル格納ディレクトリパス構造体への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::
move(const Trans::Transaction&		cTransaction_,
	 const Common::StringArrayData&	Area_)
{
	TRACEMSG("Record::File::move");
	// 固定長/可変長用ファイルをアクセスするためのオブジェクトを作成する
	initializeFiles(cTransaction_);

	// 現在のパスを記憶した上で新しいパスに設定する
	const Os::Path& cstrOldPath = m_MetaData.getDirectoryPath();
	const ModUnicodeString& cstrNewPath = Area_.getElement(0);

	// パスが異なるときのみ実行する
	if (Os::Path::compare(cstrOldPath, cstrNewPath)
		== Os::Path::CompareResult::Unrelated) {

		m_MetaData.setDirectoryPath(cstrNewPath);

		enum {
			None,
			DirectMoved,
			VariableMoved,
			ValueNum
		} eStatus = None;

		try {
			m_pDirectFile->move();
			eStatus = DirectMoved;
			if (m_pVariableFile) {
				m_pVariableFile->move();
				eStatus = VariableMoved;
			}

		_SYDNEY_FAKE_ERROR("Record::File::move",Exception::IllegalFileAccess(moduleName, srcFile, __LINE__));

		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			try {
				// パス指定を元に戻して移動しなおす
				m_MetaData.setDirectoryPath(cstrOldPath);
				switch (eStatus) {
				case VariableMoved:
					m_pVariableFile->move(true /* undo */);
					// thru
				case DirectMoved:
					m_pDirectFile->move(true /* undo */);
					// thru
				case None:
				default:
					break;
				}
			}
#ifdef NO_CATCH_ALL
			catch (Exception::Object&)
#else
			catch (...)
#endif
			{

				// 元に戻せなかったので、利用不可にする

				Checkpoint::Database::setAvailability(
					m_MetaData.getLockName(), false);
			}
			_SYDNEY_RETHROW;
		}
	}
}

// ラッチが不要なオペレーションを返す
LogicalFile::File::Operation::Value
File::
getNoLatchOperation()
{
	return Operation::Open
		| Operation::Close
		| Operation::Fetch;
}

//
//	FUNCTION public
//	Record::File::toString --
//		ファイルを識別するための文字列を返す
//
//	NOTES
//	ファイルを識別するための文字列を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		ファイルを識別するための文字列
//
//	EXCEPTIONS
//	[YET!]
//
ModUnicodeString
File::
toString() const
{
	TRACEMSG("Record::File::toString");
	return m_MetaData.getDirectoryPath();
}

//
//	FUNCTION public
//	Record::File::mount --
//		レコードファイルをマウントする
//
//	NOTES
//	レコードファイルをマウントする。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
const LogicalFile::FileID&
File::
mount(const Trans::Transaction&	cTransaction_)
{
	TRACEMSG("Record::File::mount");
	// 固定長/可変長用ファイルをアクセスするためのオブジェクトを作成する
	initializeFiles(cTransaction_);

	if (!isMounted(cTransaction_)) {

		// マウントされていなければ、マウントしてみる
		int st = 0;
		try {
			m_pDirectFile->mount();
			++st;//1
			if (m_pVariableFile)
				m_pVariableFile->mount();
			++st;//2
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			switch (st) {
			case 2:
				if (m_pVariableFile)
					m_pVariableFile->unmount();
				//fall thru
			case 1:
				m_pDirectFile->unmount();
				//fall thru
			default:
				;//nop
			}
			_SYDNEY_RETHROW;
		}

		// マウントされたことをファイル識別子に記録する

		m_MetaData.m_FileID.setBoolean(
			_SYDNEY_FILE_PARAMETER_KEY(
				FileCommon::FileOption::Mounted::Key), true);
	}

	return m_MetaData.m_FileID;
}

//
//	FUNCTION public
//	Record::File::unmount --
//		レコードファイルをアンマウントする
//
//	NOTES
//	レコードファイルをアンマウントする。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
const LogicalFile::FileID&
File::
unmount(const Trans::Transaction&	cTransaction_)
{
	TRACEMSG("Record::File::unmount");
	// 固定長/可変長用ファイルをアクセスするためのオブジェクトを作成する
	initializeFiles(cTransaction_);

	// とにかくアンマウントしてみる
	int st = 0;
	try {
		m_pDirectFile->unmount();
		++st;//1
		if (m_pVariableFile)
			m_pVariableFile->unmount();
		++st;//2
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		switch (st) {
		case 2:
			if (m_pVariableFile)
				m_pVariableFile->mount();
			//fall thru
		case 1:
			m_pDirectFile->mount();
			//fall thru
		default:
			;//nop
		}
		_SYDNEY_RETHROW;
	}

	; _SYDNEY_ASSERT(!isMounted(cTransaction_));

	// アンマウントされたことをファイル識別子に記録する

	m_MetaData.m_FileID.setBoolean(
		_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key),
		false);

	return m_MetaData.m_FileID;
}

//
//	FUNCTION public
//	Record::File::flush -- レコードファイルをフラッシュする
//
//	NOTES
//	レコードファイルをフラッシュする。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::
flush(const Trans::Transaction&	cTransaction_)
{
	TRACEMSG("Record::File::flush");
	// 固定長/可変長用ファイルをアクセスするためのオブジェクトを作成する
	initializeFiles(cTransaction_);

	m_pDirectFile->flush();
	if (m_pVariableFile) {
		m_pVariableFile->flush();
	}
}

//
//	FUNCTION public
//	Record::File::startBackup --
//		レコードファイルに対してバックアップ開始を通知する
//
//	NOTES
//	レコードファイルに対してバックアップ開始を通知する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const bool					Restorable_
//		版が最新版になるように変更可能とするかどうか
//			true  : バックアップされた内容をリストアしたとき、
//			        あるタイムスタンプの表す時点に開始された
//			        読取専用トランザクションの参照する版が
//			        最新版になるように変更可能にする。
//			false : バックアップされた内容をリストアしたとき、
//			        バックアップ開始時点に障害回復可能にする。
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::
startBackup(const Trans::Transaction&	cTransaction_,
			const bool				Restorable_)
{
	TRACEMSG("Record::File::startBackup");
	// 固定長/可変長用ファイルをアクセスするためのオブジェクトを作成する
	initializeFiles(cTransaction_);

	int st = 0;

	try {
		m_pDirectFile->startBackup(Restorable_);
		++st;//1
		if (m_pVariableFile) {
			m_pVariableFile->startBackup(Restorable_);
		}
		++st;//2
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		switch (st) {
		case 2:
			if (m_pVariableFile) {
				m_pVariableFile->endBackup();
			}
			//fall thru
		case 1:
			m_pDirectFile->endBackup();
			//fall thru
		case 0:
		default:
			;//nop
		}
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Record::File::endBackup --
//		レコードファイルに対してバックアップ終了を通知する
//
//	NOTES
//	レコードファイルに対してバックアップ終了を通知する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::
endBackup(const Trans::Transaction&	cTransaction_)
{
	TRACEMSG("Record::File::endBackup");
	// 固定長/可変長用ファイルをアクセスするためのオブジェクトを作成する
	initializeFiles(cTransaction_);

	m_pDirectFile->endBackup();
	if (m_pVariableFile) {
		m_pVariableFile->endBackup();
	}
}

//
//	FUNCTION public
//	Record::File::recover --
//		レコードファイルを障害回復する
//
//	NOTES
//	レコードファイルを障害回復する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const Trans::TimeStamp&		Point_
//		バージョンファイルを戻す時点のタイムスタンプ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::
recover(const Trans::Transaction&	cTransaction_,
		const Trans::TimeStamp&	Point_)
{
	TRACEMSG("Record::File::recover");
	// 固定長/可変長用ファイルをアクセスするためのオブジェクトを作成する
	initializeFiles(cTransaction_);

	m_pDirectFile->recover(Point_);
	if (m_pVariableFile) {
		m_pVariableFile->recover(Point_);
	}
}

//
//	FUNCTION public
//	Record::File::restore --
//		あるタイムスタンプの表す時点に開始された
//		読取専用トランザクションの参照する版が
//		最新版になるようにバージョンファイルを変更する
//
//	NOTES
//	あるタイムスタンプの表す時点に開始された
//	読取専用トランザクションの参照する版が
//	最新版になるようにバージョンファイルを変更する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const Trans::TimeStamp&		Point_
//		このタイムスタンプの表す時点に開始された
//		読取専用トランザクションの参照する版が
//		最新版になるようにバージョンファイルを変更する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::
restore(const Trans::Transaction&	cTransaction_,
		const Trans::TimeStamp&	Point_)
{
	TRACEMSG("Record::File::restore");
	// 固定長/可変長用ファイルをアクセスするためのオブジェクトを作成する
	initializeFiles(cTransaction_);

	m_pDirectFile->restore(Point_);
	if (m_pVariableFile) {
		m_pVariableFile->restore(Point_);
	}
}

//
//	FUNCTION public
//	Record::File::verify -- 整合性検査を行う
//
//	NOTES
//	整合性検査を行う。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const unsigned int				Treatment_
//		整合性検査の検査方法
//		const Admin::Verification::Treatment::Valueを
//		const unsigned intにキャストした値
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::
verify(const Trans::Transaction&		cTransaction_,
	   const unsigned int				iTreatment_,
	   Admin::Verification::Progress&	cProgress_)
{
	TRACEMSG("Record::File::verify");
	if (cProgress_.isGood() == false)
	{
		return;
	}

	// 固定長/可変長用ファイルをアクセスするためのオブジェクトを作成する
	initializeFiles(cTransaction_);

	AutoFileAttacher file( m_pDirectFile ,m_pVariableFile );

	{
	AutoPageDetacher page( m_pDirectFile ,m_pVariableFile );// スコープの最後でdetachPageAllする

	try
	{

		// 物理ファイルに対する整合性検査を行う
		_SYDNEY_VERIFY_INFO(cProgress_, m_MetaData.getDirectoryPath(), Message::VerifyPhysicalFileStarted(), iTreatment_);
		// 1件目の挿入時にVariableFileのcreateでエラーが発生すると、
		// DirectFileしか存在しない場合があるので、ここでチェックする
		VariableFile* vfile
			= (m_pVariableFile && m_pVariableFile->isMounted(cTransaction_)) ?
			m_pVariableFile : 0;
		// 物理ファイルに対する整合性検査を行う
		m_pDirectFile->verifyPhysicalFile(iTreatment_, cProgress_, vfile);

		_SYDNEY_VERIFY_INFO(cProgress_, m_MetaData.getDirectoryPath(), Message::VerifyPhysicalFileFinished(), iTreatment_);

		if (cProgress_.isGood()) {
			// ファイルの中身に対する整合性検査を行う
			m_pDirectFile->verifyContents(iTreatment_, cProgress_);
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		_SYDNEY_VERIFY_ABORTED(cProgress_,
							   m_MetaData.getDirectoryPath(),
							   Message::VerifyFailed());

		_SYDNEY_RETHROW;
	}
	}
}

//
// PRIVATE METHOD
//

//
//	FUNCTION private
//	Record::File::convertFetchOptionToObjectID --
//		fetch へのオプションからオブジェクト ID への変換
//
//	NOTES
//	この関数は、Record::File::fetch から呼ばれ、
//	引数 pOption_ を解析し、設定されているオブジェクト ID を返す。
//
//	ARGUMENTS
//	const Common::DataArrayData*	pOption_
//		Record::File::fetch へのオプション
//
//	RETURN
//	ModUInt64
//		オブジェクト ID
//		ただし、引数 pOption_ がヌルポインタの場合は、
//		Scan モードでのアクセスなので、UndefinedObjectID を返す。
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//	[YET!]

ModUInt64
File::convertFetchOptionToObjectID(const Common::DataArrayData* pOption_) const
{
	TRACEMSG("Record::File::convertFetchOpt");
	//
	// 必ず 1 要素がなければならない。
	//
	if (!pOption_ || pOption_->getCount() != 1)
		_SYDNEY_THROW0(Exception::BadArgument);

	//
	// 配列の要素が LogicalFile::ObjectID (= Common::ObjectIDData) クラスの
	// インスタンスオブジェクトでなければならない。
	//
	const Common::DataArrayData::Pointer& pElement = pOption_->getElement(0);
	if (pElement->getType() != Common::DataType::ObjectID)
		_SYDNEY_THROW0(Exception::BadArgument);

	const LogicalFile::ObjectID* pIDData =
		_SYDNEY_DYNAMIC_CAST(const LogicalFile::ObjectID*, pElement.get());
	; _SYDNEY_ASSERT(pIDData);

	return pIDData->getValue();
}

//	FUNCTION private
//	Record::File::doInsert -- 実際に挿入処理を行う
//
//	NOTES
//
//	ARGUMENTS
//	DirectFile::DataPackage& cData_
//		挿入するデータ
//
//	RETURN
//	Tools::ObjectID
//		挿入した代表オブジェクトのオブジェクトID
//
//	EXCEPTIONS

Tools::ObjectID
File::
doInsert(DirectFile::DataPackage& cData_)
{
	TRACEMSG("Record::File::doInsert");
	Tools::ObjectID iDirectID = Tools::m_UndefinedObjectID;

	// 管理情報を読み込む
	FileInformation& cFileInfo = m_pDirectFile->readFileInformationForUpdate();

	if (m_pVariableFile) {
		// 可変長の挿入を行う

		Tools::ObjectID iFreeVariableID = cFileInfo.getFirstFreeVariableObjectID();
		cData_.setVariableID(m_pVariableFile->insert(cData_, iFreeVariableID));
		cFileInfo.setFirstFreeVariableObjectID(iFreeVariableID);
	}
	// 固定長の挿入を行う
	iDirectID = m_pDirectFile->insert(cData_);

	// 管理情報を書き込む
	m_pDirectFile->syncFileInformation();

	return iDirectID;
}

//	FUNCTION private
//	Record::File::doUpdate -- 実際に更新処理を行う
//
//	NOTES
//
//	ARGUMENTS
//	Tools::ObjectID iObjectID_
//		更新するオブジェクトのオブジェクトID
//	Record::DirectFile::DataPackage& cOldObjectHeader_
//		更新前のnull bitmapと可変長ID
//	Record::DirectFile::DataPackage& cNewDirectData_
//	Record::DirectFile::DataPackage& cNewVariableData_
//		更新後のデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
File::
doUpdate(Tools::ObjectID iObjectID_,
		 DirectFile::DataPackage& cNewDirectData_)
{
	TRACEMSG("Record::File::doUpdate");
	// 可変長なし版

	// 管理情報を読み込む
	FileInformation& cFileInfo = m_pDirectFile->readFileInformationForUpdate();

	// 固定長オブジェクトを更新する
	m_pDirectFile->update(iObjectID_, cNewDirectData_, m_pDirectTargetFields);

	// 管理情報を書き込む
	m_pDirectFile->syncFileInformation();
}

void
File::
doUpdate(Tools::ObjectID iObjectID_,
		 DirectFile::DataPackage& cOldObjectHeader_,
		 DirectFile::DataPackage& cNewDirectData_,
		 DirectFile::DataPackage& cNewVariableData_)
{
	TRACEMSG("Record::File::doUpdate");
	// 可変長あり版
	; _SYDNEY_ASSERT(m_pVariableFile);

	// 管理情報を読み込む
	FileInformation& cFileInfo = m_pDirectFile->readFileInformationForUpdate();

	if (m_pVariableTargetFields->getSize()) {

		Tools::ObjectID iFreeVariableID = cFileInfo.getFirstFreeVariableObjectID();
		// 可変長オブジェクトを更新して得られたIDを更新後データに設定する
		// ★注意★
		// 可変長オブジェクトの更新は挿入+削除で行う
		cNewDirectData_.setVariableID(
			m_pVariableFile->update(cOldObjectHeader_, cNewVariableData_,
									m_pVariableTargetFields, iFreeVariableID));
		cFileInfo.setFirstFreeVariableObjectID(iFreeVariableID);

	} else {
		// 更新前のIDと同じ
		cNewDirectData_.setVariableID(cOldObjectHeader_.getVariableID());
	}

	// 固定長オブジェクトを更新する
	m_pDirectFile->update(iObjectID_, cNewDirectData_, m_pDirectTargetFields);

	// 管理情報を書き込む
	m_pDirectFile->syncFileInformation();
}

//	FUNCTION private
//	Record::File::doExpunge -- 実際に削除処理を行う
//
//	NOTES
//
//	ARGUMENTS
//	Tools::ObjectID iObjectID_
//		削除するオブジェクトのオブジェクトID
//	Tools::ObjectID iVariableID_
//		削除する可変長オブジェクトのオブジェクトID
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
File::
doExpunge(Tools::ObjectID iObjectID_, Tools::ObjectID iVariableID_)
{
	TRACEMSG("Record::File::doExpunge");
	// 管理情報を読み込む
	FileInformation& cFileInfo = m_pDirectFile->readFileInformationForUpdate();

	// 固定長オブジェクトを破棄する
	m_pDirectFile->expunge(iObjectID_);

	if (m_pVariableFile) {

		Tools::ObjectID iFreeVariableID = cFileInfo.getFirstFreeVariableObjectID();
		// 可変長も破棄する
		m_pVariableFile->expunge(iVariableID_, iFreeVariableID);
		cFileInfo.setFirstFreeVariableObjectID(iFreeVariableID);
	}

	// 管理情報を書き込む
	m_pDirectFile->syncFileInformation();
}

//
//	FUNCTION private
//	Record::File::saveOpenOption -- 
//		オープンオプションをデータメンバに保存する
//
//	NOTES
//	オープンオプションをメンバ変数、プロジェクション情報に格納する。
//
//	ARGUMENTS
//	const LogicalFile::OpenOption&	cOpenOption_
//		オープンオプションへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::
saveOpenOption(const LogicalFile::OpenOption& cOpenOption_ ,int& iOpenModeValue_)
{
	TRACEMSG("Record::File::saveOpenOption");
	// オープンモードをデータメンバに保存
	if (iOpenModeValue_ == FileCommon::OpenOption::OpenMode::Read)
	{
		m_OpenParam.m_iOpenMode = FileCommon::OpenMode::Read;
	}
	else if (iOpenModeValue_ == FileCommon::OpenOption::OpenMode::Update)
	{
		m_OpenParam.m_iOpenMode = FileCommon::OpenMode::Update;
	}
	else if (iOpenModeValue_ == FileCommon::OpenOption::OpenMode::Batch)
	{
		m_OpenParam.m_iOpenMode = FileCommon::OpenMode::Batch;
	}
	else
	{
		// 不正な引数
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// 見積り指定をデータメンバに保存
	if (! cOpenOption_.getBoolean(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::Estimate::Key), m_OpenParam.m_bEstimate))
	{
		// ユーザが値を設置していない時はデフォルト値を設定
		m_OpenParam.m_bEstimate = false;
	}

	// フィールド選択指定

	// 全フィールドを選択した状態になっているはず
	; _SYDNEY_ASSERT(m_pTargetFields == 0);

	bool bFieldSelect;
	if (! cOpenOption_.getBoolean(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::FieldSelect::Key), bFieldSelect))
	{
		// ユーザが値を設置していない時はデフォルト値を設定
		bFieldSelect = false;
	}

	// フィールド選択指定が true になっている時は選択されたフィールドの数と
	// 位置も指定されなければいけない。
	if (bFieldSelect)
	{
		int		iSelectedFieldCount			= 0;

		if (! cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::TargetFieldNumber::Key), iSelectedFieldCount))
		{
			// フィールド選択指定されているのに、いくつ指定したのかを
			// 表す値が設定されていない
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		// 指定されているフィールドの番号を格納する配列を準備
		m_pTargetFields = new TargetFields(iSelectedFieldCount);

		for (int i = 0; i < iSelectedFieldCount; ++i)
		{
			int iFieldID;

			bool bFind = cOpenOption_.getInteger(
				_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				FileCommon::OpenOption::TargetFieldIndex::Key, i), iFieldID);

			if (bFind)
			{
				// 選択されたフィールドをみつけた
				m_pTargetFields->addFieldNumber(iFieldID);
			}
		}

		if (m_pTargetFields->getSize() != iSelectedFieldCount)
		{
			// 実際に取得できた「選択されたフィールド」の数が指定された
			// 数よりも少ない
			delete m_pTargetFields, m_pTargetFields = 0;
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		if (m_OpenParam.m_iOpenMode == FileCommon::OpenMode::Read)
			// DataPackageに選択フィールドを設定する
			m_DataPackage.setTargetField(m_pTargetFields);
		
	}

	// ソートオーダー
	bool bSortOrder;
	if ( cOpenOption_.getBoolean(_SYDNEY_RECORD_OPEN_PARAMETER_KEY(Record::OpenOption::SortOrder::Key), bSortOrder) )
	{
		m_OpenParam.m_bSortOrder = bSortOrder;
	}
	else
	{
		// デフォルト値は false (昇順)
		m_OpenParam.m_bSortOrder = false;
	}
}

//	FUNCTION private
//	Record::File::divideTargets --
//		フィールドを固定長と可変長に分ける
//
//	NOTES
//
//	ARGUMENTS
//		bool bIsUpdate_ = false
//			更新のときはデータにオブジェクトIDが含まれないので
//			特別な処理が必要である
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::
divideTargets(bool bIsUpdate_)
{
	TRACEMSG("Record::File::divideTargets");
	if (!m_pDirectTargetFields) {
		if (m_pVariableFile) {
			// 可変長があるファイルのときは分ける
			; _SYDNEY_ASSERT(!m_pVariableTargetFields);

			m_pDirectTargetFields = new TargetFields(m_MetaData.getFieldNumber()
													 - m_MetaData.getVariableFieldNumber());
			m_pVariableTargetFields = new TargetFields(m_MetaData.getVariableFieldNumber());
			m_bTargetFieldsAllocated = true;

			TargetFields::divide(*m_pDirectTargetFields,
								 *m_pVariableTargetFields,
								 m_pTargetFields,
								 m_MetaData,
								 bIsUpdate_);

		} else {
			// 可変長がないときは同じものを指す
			m_pDirectTargetFields = m_pTargetFields;
			m_bTargetFieldsAllocated = false;
		}
	}
}

//	FUNCTION private
//	Record::File::initializeFiles --
//		固定長/可変長用のファイルを表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		const Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::
initializeFiles(const Trans::Transaction& cTrans_)
{
	TRACEMSG("Record::File::initializeFiles");

	// メタデータを初期化する
	m_MetaData.initialize();
	
	// 固定長用ファイルをアクセスするためのオブジェクトを作成する
	if (!m_pDirectFile) {
		m_pDirectFile = new DirectFile(cTrans_, m_MetaData);
	}
	if (m_MetaData.hasVariable()) {
		// 可変長用ファイルをアクセスするためのオブジェクトを作成する
		if (!m_pVariableFile) {
			m_pVariableFile = new VariableFile(cTrans_, m_MetaData);
		}
	}
}

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
