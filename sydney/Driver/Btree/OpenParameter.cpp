// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenParameter.cpp -- Ｂ＋木ファイルオープンパラメータクラスの実現ファイル
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2006, 2023 Ricoh Company, Ltd.
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
const char	moduleName[] = "Btree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/IntegerData.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/Integer64Data.h"
#include "Common/UnsignedInteger64Data.h"
#include "Common/StringData.h"
#include "Common/FloatData.h"
#include "Common/DoubleData.h"
#include "Common/DateData.h"
#include "Common/DateTimeData.h"
#include "Common/NullData.h"
#include "Common/Message.h"
#include "Common/Assert.h"
#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"

#include "LogicalFile/TreeNodeInterface.h"
#include "LogicalFile/OpenOption.h"

#include "Btree/OpenParameter.h"
#include "Btree/OpenOption.h"

#include "FileCommon/DataManager.h"

#include "ModString.h"

_SYDNEY_USING

using namespace Btree;

//////////////////////////////////////////////////
//
//	PUBLIC CONST
//
//////////////////////////////////////////////////

//
//	FUNCTION public
//	Btree::OpenParameter::OpenParameter -- コンストラクタ
//
//	NOTES
//	コンストラクタ
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
OpenParameter::OpenParameter()
	: Common::Object(),
	  m_TargetFieldIndexArray(0),
	  m_TargetFieldIndexArrayAllocateSize(0)
{
}

//
//	FUNCTION public
//	Btree::OpenParameter::OpenParameter -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const Btree::FileParameter&		cFileParameter_
//		Ｂ＋木ファイルパラメータへの参照
//	const LogicalFile::OpenOption&	cOpenOption_
//		Ｂ＋木ファイルオープンオプションへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//		( Btree::OpenParameter::set )
//	NotSupported
//		未サポート
//		( Btree::OpenParameter::set )
//
OpenParameter::OpenParameter(const FileParameter&			cFileParameter_,
							 const LogicalFile::OpenOption&	cOpenOption_)
	: Common::Object()
{
	set(cFileParameter_, cOpenOption_);
}

//
//	FUNCTION public
//	Btree::OpenParameter::OpenParameter -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const Btree::FileParameter&	FileParam_
//		Ｂ＋木ファイルパラメータへの参照
//	const Btree::OpenParameter&	Original_
//		コピー元オープンパラメータへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
OpenParameter::OpenParameter(const FileParameter&	FileParam_,
							 const OpenParameter&	Original_)
	: Common::Object(),
	  m_iOpenMode(Original_.m_iOpenMode),
	  m_bEstimate(Original_.m_bEstimate),
	  m_bGetByBitSet(Original_.m_bGetByBitSet),
	  m_bFieldSelect(Original_.m_bFieldSelect),
	  m_cTargetFieldIndexArray(Original_.m_cTargetFieldIndexArray),
	  m_TargetFieldIndexArray(0),
	  m_TargetFieldNum(0),
	  m_TargetFieldIndexArrayAllocateSize(0),
	  m_cSearchCondition(Original_.m_cSearchCondition),
	  m_iSortKeyType(Original_.m_iSortKeyType),
	  m_bSortReverse(Original_.m_bSortReverse),
	  m_iReadSubMode(Original_.m_iReadSubMode),
	  m_cFetchFieldIndexArray(Original_.m_cFetchFieldIndexArray)
{
	if (this->m_bFieldSelect)
	{
		this->setTargetFields(FileParam_);
	}
	else
	{
		this->m_SelectObjectID = true;
		this->m_ExistTargetFieldInKey = true;
		this->m_ExistTargetFieldInValue = true;
	}
}

//
//	FUNCTION public
//	Btree::OpenParameter::~OpenParameter -- デストラクタ
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
//	なし
//
OpenParameter::~OpenParameter()
{
	this->clearTargetFields();
}

//
//	FUNCTION public
//	Btree::OpenParameter::set --
//		Ｂ＋木ファイルオープンパラメータを設定する
//
//	NOTES
//	Ｂ＋木ファイルオープンパラメータを設定する。
//
//	ARGUMENTS
//	const Btree::FileParameter&		cFileParameter_
//		Ｂ＋木ファイルパラメータへの参照
//	const LogicalFile::OpenOption&	cOpenOption_
//		Ｂ＋木ファイルオープンオプションへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//		( Btree::OpenParameter::checkSortKey )
//		( Btree::OpenParameter::setSearchCondition )
//	NotSupported
//		未サポート
//		( Btree::OpenParameter::setSearchCondition )
//
void
OpenParameter::set(const FileParameter&				cFileParameter_,
				   const LogicalFile::OpenOption&	cOpenOption_)
{
	int					iParameterValue;

	//
	// オープンモード
	//

	if (cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key), iParameterValue) == false)
	{
		//
		// オープンモードは省略不可
		//

		SydErrorMessage << "Unset open mode." << ModEndl;

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	if (iParameterValue == FileCommon::OpenOption::OpenMode::Read)
	{
		m_iOpenMode = FileCommon::OpenMode::Read;
	}
	else if (iParameterValue == FileCommon::OpenOption::OpenMode::Search)
	{
		m_iOpenMode = FileCommon::OpenMode::Search;
	}
	else if (iParameterValue == FileCommon::OpenOption::OpenMode::Update ||
			 iParameterValue == FileCommon::OpenOption::OpenMode::Batch)
	{
		m_iOpenMode = FileCommon::OpenMode::Update;
	}
	else if (iParameterValue == FileCommon::OpenOption::OpenMode::Initialize)
	{
		m_iOpenMode = FileCommon::OpenMode::Initialize;
	}
	else
	{
		//
		// 不正なオープンモード
		//

		SydErrorMessage
			<< "Illegal open mode (=\"" << iParameterValue
			<< "\")." << ModEndl;

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	//
	// 見積りフラグ
	//

	if (cOpenOption_.getBoolean(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::Estimate::Key), m_bEstimate) == false)
	{
		//
		// 見積りフラグが省略されている（未設定）場合は、
		// 見積りのためのオープンではない。
		//
		m_bEstimate = false;
	}

	//
	// ビットセットフラグ
	// ※ このフラグが true の場合には、
	//    Btree::File::getData が返すオブジェクトは
	//    ビットセットとなる。
	//

	if ( cOpenOption_.getBoolean( _SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::GetByBitSet::Key), m_bGetByBitSet ) == false )
	{
		//
		// ビットセットフラグが省略されている（未設定）場合は、
		// 通常のオブジェクトを返す。
		//
		m_bGetByBitSet = false;
	}

	//
	// フィールド選択フラグ
	// ※ このフラグが true の場合には、
	//    Btree::File::getData が返すオブジェクトは
	//    挿入されているオブジェクトのうち、ある特定の
	//    フィールドのみで構成されたものとなる。
	//

	if (cOpenOption_.getBoolean(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::FieldSelect::Key), m_bFieldSelect) == false)
	{
		//
		// フィールド選択フラグが省略されている（未設定）場合は、
		// 特定のフィールドを選択のではなく、オブジェクト全体を
		// 返す
		m_bFieldSelect = false;
	}

	if (m_bFieldSelect)
	{
		// フィールド選択…

		//
		// 処理対象フィールドインデックス配列
		//

		iParameterValue = -1;
		if ( cOpenOption_.getInteger( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::TargetFieldNumber::Key), iParameterValue) == false || iParameterValue < 1 )
		{
			//
			// フィールド選択フラグに true を設定しておいて
			// 処理対象フィールドがないのはおかしい
			//

			SydErrorMessage << "Unset select field." << ModEndl;

			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}

		const int iArraySize = iParameterValue;
		for (int iArrayIndex = 0 ; iArrayIndex < iArraySize; ++iArrayIndex)
		{
			iParameterValue = -1;
			if ( cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(FileCommon::OpenOption::TargetFieldIndex::Key, iArrayIndex), iParameterValue)
				== false ||
				iParameterValue < 0 ||
				iParameterValue > cFileParameter_.m_FieldNum )
			{
				//
				// 不正なフィールドインデックス
				//

				SydErrorMessage
					<< "Illegal field index (=" << iParameterValue
					<< ":number of fields="
					<< cFileParameter_.m_FieldNum << ")."
					<< ModEndl;

				throw Exception::BadArgument(moduleName,
											 srcFile,
											 __LINE__);
			}

			m_cTargetFieldIndexArray.setElement( iArrayIndex, iParameterValue);

		} // end for iArrayIndex

		this->setTargetFields(cFileParameter_);

	}
	else
	{
		this->m_TargetFieldNum = 0;
		this->m_TargetFieldIndexArray = 0;
		this->m_TargetFieldIndexArrayAllocateSize = 0;

		this->m_SelectObjectID = true;
		this->m_ExistTargetFieldInKey = true;
		this->m_ExistTargetFieldInValue = true;
	}

	// 検索条件パラメータを設定する
	setSearchCondition(cFileParameter_, cOpenOption_);

	//
	// ソートキー
	//

	m_iSortKeyType = KeyField;
	iParameterValue = -1;
	if (cOpenOption_.getInteger(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::SortKey::Key), iParameterValue))
	{
		if (iParameterValue == Btree::OpenOption::SortKey::ObjectID)
		{
			m_iSortKeyType = ObjectID;
		}
		else if (iParameterValue != Btree::OpenOption::SortKey::KeyField)
		{
			//
			// 不正なソートキー
			//

			SydErrorMessage << "Illegal sort key." << ModEndl;

			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}
	}

	// 検索条件とソートキーの整合性をチェックする
	// ※ 整合性が合っていなければ、checkSortKey が例外をなげる
	checkSortKey();

	//
	// オブジェクト取得ソート順
	//
	if (cOpenOption_.getBoolean(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::SortReverse::Key), m_bSortReverse) == false)
	{
		m_bSortReverse = false;
	}

	//
	// オブジェクト取得サブモード
	//

	m_iReadSubMode = ScanRead;
	iParameterValue = -1;
	if (cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::ReadSubMode::Key), iParameterValue))
	{
		if (iParameterValue == FileCommon::OpenOption::ReadSubMode::Fetch)
		{
			m_iReadSubMode = FetchRead;
		}
		else if (iParameterValue != FileCommon::OpenOption::ReadSubMode::Scan)
		{
			//
			// 不正なオブジェクト取得サブモード
			//

			SydErrorMessage
				<< "Illegal read sub mode (=\""
				<< iParameterValue << "\")." << ModEndl;

			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}
	}

	if (m_iReadSubMode == FetchRead)
	{
		// Fetchモード...

		//
		// Fetch 対象フィールドインデックス
		//

		iParameterValue = -1;

		if (cOpenOption_.getInteger(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::FetchFieldNumber::Key), iParameterValue)
			== false ||
			iParameterValue < 1)
		{
			//
			// Fetch 対象フィールドが指定されていない
			//

			SydErrorMessage << "Unset fetch field." << ModEndl;

			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}

		const int iArraySize = iParameterValue;
		for (int iArrayIndex = 0 ; iArrayIndex < iArraySize; ++iArrayIndex)
		{
			iParameterValue = -1;
			if (cOpenOption_.getInteger(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::FetchFieldIndex::Key, iArrayIndex), iParameterValue)
				== false ||
				iParameterValue < 0 ||
				iParameterValue >= cFileParameter_.m_FieldNum)
			{
				//
				// 不正なフィールドインデックス
				//

				SydErrorMessage
					<< "Illegal fetch field index (="
					<< iParameterValue
					<< ")."
					<< ModEndl;

				throw Exception::BadArgument(moduleName,
											 srcFile,
											 __LINE__);
			}

			m_cFetchFieldIndexArray.setElement( iArrayIndex, iParameterValue);
		}
	}
}

//////////////////////////////////////////////////
//
//	PRIVATE CONST
//
//////////////////////////////////////////////////

//
//	FUNCTION private
//	Btree::OpenParameter::setTargetFields --
//		処理対象フィールドインデックス配列を設定する
//
//	NOTES
//	処理対象フィールドインデックス配列を設定する。
//
//	ARGUMENTS
//	const Btree::FileParameter&	FileParam_
//		Ｂ＋木ファイルパラメータへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
OpenParameter::setTargetFields(const FileParameter&	FileParam_)
{
	; _SYDNEY_ASSERT(this->m_bFieldSelect);

	this->m_TargetFieldNum = this->m_cTargetFieldIndexArray.getCount();

	this->m_TargetFieldIndexArrayAllocateSize =
		sizeof(int) * this->m_TargetFieldNum;

	this->m_TargetFieldIndexArray =
		static_cast<int*>(
			ModDefaultManager::allocate(
				this->m_TargetFieldIndexArrayAllocateSize));

	this->m_SelectObjectID = false;
	this->m_ExistTargetFieldInKey = false;
	this->m_ExistTargetFieldInValue = false;

	for (int i = 0; i < this->m_TargetFieldNum; i++)
	{
		int	targetFieldIndex =
			this->m_cTargetFieldIndexArray.getElement(i);

		*(this->m_TargetFieldIndexArray + i) = targetFieldIndex;

		if (targetFieldIndex == 0)
		{
			this->m_SelectObjectID = true;
		}
		else if (targetFieldIndex < FileParam_.m_TopValueFieldIndex)
		{
			this->m_ExistTargetFieldInKey = true;
		}
		else
		{
			this->m_ExistTargetFieldInValue = true;
		}
	}
}

//
//	FUNCTION private
//	Btree::OpenParameter::clearTargetFields --
//		処理対象フィールドインデックス配列を解放する
//
//	NOTES
//	処理対象フィールドインデックス配列を解放する。
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
OpenParameter::clearTargetFields()
{
	if (this->m_TargetFieldIndexArrayAllocateSize > 0)
	{
		ModDefaultManager::free(this->m_TargetFieldIndexArray,
								this->m_TargetFieldIndexArrayAllocateSize);
	}
}

//
//	FUNCTION private
//	Btree::OpenParameter::setSearchCondition --
//		検索条件パラメータを設定する
//
//	NOTES
//	検索条件パラメータを設定する。
//
//	ARGUMENTS
//	const Btree::FileParameter&		cFileParameter_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//	const LogicalFile::OpenOption&	cOpenOption_
//		Ｂ＋木ファイルオープンオプションオブジェクトへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//		( Btree::OpenParameter::getCompareOperator )
//	NotSupported
//		未サポート
//		( Btree::OpenParameter::getSearchCondition )
//
void
OpenParameter::setSearchCondition(const FileParameter&				cFileParameter_,
								  const LogicalFile::OpenOption&	cOpenOption_)
{
	int					iParameterValue;
	ModUnicodeString	cstrParameterValue;

	iParameterValue = 0;
	cOpenOption_.getInteger(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::SearchFieldNumber::Key), iParameterValue);
	if (iParameterValue == 0)
	{
		// getSearchParameter() の時点で既に検索結果が'φ'だとされている。
		m_cSearchCondition.m_VoidSearch = true;
	}
	else
	{
		m_cSearchCondition.m_VoidSearch = false;
		bool	likeSearch = false;

		const int iArraySize = iParameterValue;
		for (int iArrayIndex = 0 ; iArrayIndex < iArraySize; ++iArrayIndex)
		{
			//
			// 検索対象キーフィールドインデックス
			//
			iParameterValue = -1;
			if (cOpenOption_.getInteger(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::SearchFieldIndex::Key, iArrayIndex), iParameterValue)
				== false ||
				iParameterValue < 0 ||
				iParameterValue > cFileParameter_.m_KeyNum)
			{
				//
				// 不正なキーフィールドインデックス
				//

				SydErrorMessage
					<< "Illegal search key field index (=" << iParameterValue
					<< ")." << ModEndl;

				throw Exception::BadArgument(moduleName, srcFile, __LINE__);
			}

			int	iSearchFieldIndex = iParameterValue;

			m_cSearchCondition.m_cFieldIndexArray.setElement( iArrayIndex, iSearchFieldIndex);

			//
			// 検索開始条件
			//

			//
			// ここでgetSearchConditionを呼んでしまうと、
			// Common::DataType::Date型のキーに対して
			// 『キー値がヌルのオブジェクトを検索』する場合、
			// 空文字列でCommon::DateData::setValue()を呼び出してしまい、
			// Exception::BadArgumentが送出されてしまうので。
			// Common::DataType::Time型のキーでも同じはず。
			//
			/*
			cstrParameterValue = "";
			if (cOpenOption_.getString(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::SearchStart::Key, iArrayIndex), cstrParameterValue) == false) {
				//
				// 不正な検索開始条件
				//

				// 長さ 0 の文字列でも検索可能とする

				SydErrorMessage
					<< "Unset search condition (start value)." << ModEndl;

				throw Exception::BadArgument(moduleName, srcFile, __LINE__);
			}

			Common::Data*	pStartCondition = getSearchCondition(cFileParameter_,
																 iSearchFieldIndex,
																 cstrParameterValue);
			m_cSearchCondition.m_cStartArray.setElement( iArrayIndex, pStartCondition);
			*/

			//
			// 検索開始比較演算子
			//
			iParameterValue = -1;
			if (cOpenOption_.getInteger(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::Ope::StartKey, iArrayIndex), iParameterValue)
				== false ||
				iParameterValue < 0)
			{
				//
				// 不正な検索開始比較演算子
				//

				SydErrorMessage
					<< "Unset search condition (start compare operator)."
					<< ModEndl;

				throw Exception::BadArgument(moduleName, srcFile, __LINE__);
			}

			LogicalFile::TreeNodeInterface::Type	iStartOperator =
				OpenParameter::getCompareOperator(iParameterValue);
			m_cSearchCondition.m_cStartOpeArray.setElement( iArrayIndex, (int)iStartOperator);

			if (iStartOperator == LogicalFile::TreeNodeInterface::Like)
			{
				likeSearch = true;
			}

			if (iStartOperator == LogicalFile::TreeNodeInterface::EqualsToNull)
			{
				m_cSearchCondition.m_cStartArray.setElement( iArrayIndex, Common::NullData::getInstance());
			}
			else
			{
				cstrParameterValue = "";
				if (cOpenOption_.getString(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::SearchStart::Key ,iArrayIndex), cstrParameterValue) == false)
				{
					//
					// 不正な検索開始条件
					//

					// 長さ 0 の文字列でも検索可能とする

					SydErrorMessage
						<< "Unset search condition (start value)." << ModEndl;

					throw Exception::BadArgument(moduleName,
												 srcFile,
												 __LINE__);
				}

				Common::Data*	pStartCondition =
					OpenParameter::getSearchCondition(cFileParameter_,
													  iSearchFieldIndex,
													  cstrParameterValue);

				m_cSearchCondition.m_cStartArray.setElement( iArrayIndex, pStartCondition);
			}

			//
			// 検索終了条件
			//
			cstrParameterValue = "";
			if (cOpenOption_.getString(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::SearchStop::Key, iArrayIndex), cstrParameterValue))
			{
				if (cstrParameterValue.getLength() == 0)
				{
					//
					// 不正な検索終了条件
					//

					SydErrorMessage
						<< "Unset search condition (stop value)." << ModEndl;

					throw Exception::BadArgument(moduleName,
												 srcFile,
												 __LINE__);
				}

				Common::Data*	pStopCondition =
					OpenParameter::getSearchCondition(cFileParameter_,
													  iSearchFieldIndex,
													  cstrParameterValue);

				m_cSearchCondition.m_cStopArray.setElement( iArrayIndex, pStopCondition);

				//
				// 検索終了比較演算子
				//
				iParameterValue = -1;
				if (cOpenOption_.getInteger(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(Btree::OpenOption::Ope::StopKey,iArrayIndex), iParameterValue)
					== false ||
					iParameterValue < 0)
				{
					//
					// 不正な検索終了比較演算子
					//

					SydErrorMessage
						<< "Unset search condition (stop compare operator)."
							<< ModEndl;

					throw Exception::BadArgument(moduleName, srcFile, __LINE__);
				}

				LogicalFile::TreeNodeInterface::Type	iStopOperator = OpenParameter::getCompareOperator(iParameterValue);

				m_cSearchCondition.m_cStopOpeArray.setElement( iArrayIndex, (int)iStopOperator );
			}
			else
			{
				m_cSearchCondition.m_cStopOpeArray.setElement( iArrayIndex, (int)LogicalFile::TreeNodeInterface::Undefined );
			}

		} // end for iArrayIndex

		if (likeSearch)
		{
			this->m_cSearchCondition.m_SetEscape = false;

			ModUnicodeString	escape;

			if (cOpenOption_.getString(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(Btree::OpenOption::Escape::Key), escape))
			{
				this->m_cSearchCondition.m_SetEscape = true;
				this->m_cSearchCondition.m_Escape = escape[0];
			}
		}
	}
}

//
//	FUNCTION private
//	Btree::OpenParameter::checkSortKey --
//		検索条件とソートキーの整合性をチェックする
//
//	NOTES
//	自身に設定された検索条件とソートキーの整合性をチェックする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//	※ 整合性が合わない場合は例外を投げるため。
//
//	EXCEPTIONS
//	BadArgument
//		検索条件とソートキーの整合性が合わない
//		※ オブジェクト ID に対する検索条件が指定されているのに
//		   ソートキーにキーフィールドが指定されていたり、
//		   または、その逆の場合。
//
void
OpenParameter::checkSortKey() const
{
	if (m_cSearchCondition.m_cFieldIndexArray.getCount() > 0)
	{
		int	iTopSearchFieldIndex =
			m_cSearchCondition.m_cFieldIndexArray.getElement(0);

		SortKeyType	iSearchSortKeyType =
			(iTopSearchFieldIndex == 0) ? ObjectID : KeyField;

		if (iSearchSortKeyType != this->m_iSortKeyType)
		{
			//
			// 検索条件とソートキーの整合性が合わない
			//

			SydErrorMessage << "Illegal sort key." << ModEndl;

			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}
	}
}

//
//	FUNCTION private
//	Btree::OpenParameter::getSearchCondition --
//		検索条件となるコモンデータを返す
//
//	NOTES
//	検索条件となるコモンデータを生成し、返す。
//
//	ARGUMENTS
//	const Btree::FileParameter&	cFileParameter_
//		Ｂ＋木ファイルパラメータオブジェクトへの参照
//	const int					iSearchFieldIndex_
//		検索対象フィールドのオブジェクト内でのインデックス
//	const ModUnicodeString&		cstrValueString_
//		検索条件値の文字列表現
//
//	RETURN
//	Common::Data*
//		検索条件となるコモンデータへのポインタ
//
//	EXCEPTIONS
//	NotSupported
//		Ｂ＋木ファイルでサポートしていないフィールドデータ型
//
// static
Common::Data*
OpenParameter::getSearchCondition(
	const FileParameter&	cFileParameter_,
	const int				iSearchFieldIndex_,
	const ModUnicodeString&	cstrValueString_)
{
	; _SYDNEY_ASSERT(
		iSearchFieldIndex_ < cFileParameter_.m_FieldNum);

	Common::DataType::Type	iSearchFieldDataType =
		*(cFileParameter_.m_FieldTypeArray + iSearchFieldIndex_);

	if (iSearchFieldDataType == Common::DataType::Binary)
	{
		//
		// Common::BinaryData フィールドに対して検索はできない。
		// ※ getSearchParameter でチェックしているので
		//    ここにはこないはず。
		//

		SydErrorMessage << "Can not search by binary field." << ModEndl;

		throw Exception::NotSupported(moduleName, srcFile, __LINE__);
	}

	return
		FileCommon::DataManager::createCommonData(
			iSearchFieldDataType,
			const_cast<ModUnicodeString&>(cstrValueString_));
}

//
//	FUNCTION private
//	Btree::OpenParameter::getCompareOperator -- 検索条件の比較演算子を返す
//
//	NOTES
//	検索条件の比較演算子を返す。
//
//	ARGUMENTS
//	const ModUnicodeString&	iOperatorValue_
//		検索条件の比較演算子の文字列表現
//
//	RETURN
//	LogicalFile::TreeNodeInterface::Type
//		検索条件の比較演算子
//
//	EXCEPTIONS
//	BadArgument
//		不正な比較演算子の文字列表現
//
// static
LogicalFile::TreeNodeInterface::Type
OpenParameter::getCompareOperator(
	int	iOperatorValue_)
{
	if (iOperatorValue_ == OpenOption::Ope::Equals)
	{
		return LogicalFile::TreeNodeInterface::Equals;
	}
	else if (iOperatorValue_ == OpenOption::Ope::GreaterThan)
	{
		return LogicalFile::TreeNodeInterface::GreaterThan;
	}
	else if (iOperatorValue_ == OpenOption::Ope::GreaterThanEquals)
		{
		return LogicalFile::TreeNodeInterface::GreaterThanEquals;
	}
	else if (iOperatorValue_ == OpenOption::Ope::LessThan)
	{
		return LogicalFile::TreeNodeInterface::LessThan;
	}
	else if (iOperatorValue_ == OpenOption::Ope::LessThanEquals)
	{
		return LogicalFile::TreeNodeInterface::LessThanEquals;
	}
	else if (iOperatorValue_ == OpenOption::Ope::EqualsToNull)
	{
		return LogicalFile::TreeNodeInterface::EqualsToNull;
	}
	else if (iOperatorValue_ == OpenOption::Ope::Like)
	{
		return LogicalFile::TreeNodeInterface::Like;
	}
	else
	{
		//
		// 不正な比較演算子の文字列表現
		//

		SydErrorMessage
			<< "Unknown compare operator (=\""
			<< iOperatorValue_
			<< "\")."
			<< ModEndl;

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}
}

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
