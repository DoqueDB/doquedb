// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileParameter.cpp -- Ｂ＋木ファイルパラメータクラスの実現ファイル
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#include "Common/Assert.h"
#include "Common/Message.h"

#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"

#include "Version/File.h"

#include "LogicalFile/ObjectID.h"

#include "FileCommon/DataManager.h"
#include "FileCommon/IDNumber.h"

#include "Btree/FileParameter.h"
#include "Btree/FileOption.h"
#include "Btree/File.h"
#include "Btree/TreeFile.h"
#include "Btree/ValueFile.h"
#include "Btree/NodePageHeader.h"
#include "Btree/KeyInformation.h"
#include "Btree/Hint.h"
#include "Btree/NullBitmap.h"

#include "ModString.h"

_SYDNEY_USING
_SYDNEY_BTREE_USING

//////////////////////////////////////////////////
//
//	PUBLIC CONST
//
//////////////////////////////////////////////////

//
//	CONST public
//	Btree::FileParameter::VariableFieldInsideThreshold
//		可変長フィールドを外置きオブジェクトに記録するかどうかの閾値
//
//	NOTES
//	可変長フィールドを外置きオブジェクトに記録するかどうかの閾値。
//	（可変長フィールド最大長[byte]）
//	利用者が可変長フィールド最大長を指定していて、この閾値以下の場合には、
//	可変長フィールドの値は代表オブジェクトに直接記録される。
//
// static
const Os::Memory::Size
FileParameter::VariableFieldInsideThreshold = 8;

//////////////////////////////////////////////////
//
//	PRIVATE CONST
//
//////////////////////////////////////////////////

//
//	CONST private
//	Btree::FileParameter::LocalLimit
//		データメンバの各配列をヒープ領域に確保するかどうかの閾値
//
//	NOTES
//	データメンバの各配列をヒープ領域に確保するかどうかの閾値。
//	（フィールド数）
//	オブジェクトを構成するフィールド数がこの閾値以下の場合には、
//	ローカルな配列を使用し、閾値を超える場合には、
//	データメンバの各配列用の領域をヒープ領域に確保する。
//
// static
const int
FileParameter::LocalLimit = 10;

//////////////////////////////////////////////////
//
//	PUBLIC METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION public
//	Btree::FileParameter::FileParameter -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	const LogicalFile::FileID&	FileID_
//		Ｂ＋木ファイル用論理ファイルIDオブジェクトへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//
FileParameter::FileParameter(const LogicalFile::FileID&	FileID_)
{
	this->initialize(FileID_);
}

//
//	FUNCTION public
//	Btree::FileParameter::~FileParameter -- デストラクタ
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
FileParameter::~FileParameter()
{
	this->terminate();
}

//
//	FUNCTION public
//	Btree::FileParameter::initialize -- ファイルパラメータを初期化する
//
//	NOTES
//	ファイルパラメータを初期化する。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	FileID_
//		Ｂ＋木ファイル用論理ファイルIDオブジェクトへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//
void
FileParameter::initialize(const LogicalFile::FileID&	FileID_)
{
	// ファイルパスを設定する
	ModUnicodeString	btreeFilePath;
	this->setBtreeFilePath(FileID_, btreeFilePath);

	// 物理ページサイズを設定する
	this->setPhysicalPageSize(FileID_);

	// 一時ファイルかどうかを設定する
	bool	isTemp = this->setTemporary(FileID_);

	// 読み込み専用かどうかを設定する
	bool	readOnly = this->setReadOnly(FileID_);

	// マウントされているかどうかを設定する
	bool	mounted = this->setMounted(FileID_);

	// バッファリング戦略を設定する
	this->setBufferingStrategy(isTemp, readOnly);

	// ユニークタイプを設定する
	this->setUniqueType(FileID_);

	// フィールドパラメータを設定する
	this->setFieldParam(FileID_);

	// ファイルヒントを解析する
	this->analyzeFileHint(FileID_);

	// ツリーファイル格納戦略を設定する
	this->setTreeFileStorageStrategy(btreeFilePath, isTemp, mounted);

	// バリューファイル格納戦略を設定する
	this->setValueFileStorageStrategy(btreeFilePath, isTemp, mounted);

	// 論理ファイルIDから一意な整数値を得るためのオブジェクトを生成
	{// 自身にも設定する(∵IDNumber は FileID を参照でしか保持しない)
	FileCommon::IDNumber cIDNumber(FileID_);
	this->setInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::DatabaseID::Key), cIDNumber.getDatabaseID());
	this->setInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::TableID::Key), cIDNumber.getTableID());
	this->setInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::FileObjectID::Key), cIDNumber.getFileObjectID());
	}
	this->m_IDNumber = new FileCommon::IDNumber(*this);

	// バージョン情報を設定する
	int version;
	if (FileID_.getInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Version::Key), version) == true)
		setInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Version::Key), version);
}

//
//	FUNCTION public
//	Btree::FileParameter::terminate -- ファイルパラメータの後処理をする
//
//	NOTES
//	ファイルパラメータの後処理をする。
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
FileParameter::terminate()
{
	// 論理ファイルIDから一意な整数値を得るためのオブジェクトを破棄
	delete this->m_IDNumber;

	if (this->m_FieldNum > FileParameter::LocalLimit)
	{
		// フィールドタイプ配列

		; _SYDNEY_ASSERT(this->m_FieldTypeArray !=
						 this->m_FieldTypeLocalArray);
		; _SYDNEY_ASSERT(this->m_FieldTypeArrayAllocateSize > 0);

		ModDefaultManager::free(this->m_FieldTypeArray,
								this->m_FieldTypeArrayAllocateSize);

		// 固定長フィールドフラグ配列

		; _SYDNEY_ASSERT(this->m_IsFixedFieldArray !=
						 this->m_IsFixedFieldLocalArray);
		; _SYDNEY_ASSERT(this->m_IsFixedFieldArrayAllocateSize > 0);

		ModDefaultManager::free(this->m_IsFixedFieldArray,
								this->m_IsFixedFieldArrayAllocateSize);

		// フィールド最大長配列

		; _SYDNEY_ASSERT(this->m_FieldMaxLengthArray !=
						 this->m_FieldMaxLengthLocalArray);
		; _SYDNEY_ASSERT(this->m_FieldMaxLengthArrayAllocateSize > 0);

		ModDefaultManager::free(this->m_FieldMaxLengthArray,
								this->m_FieldMaxLengthArrayAllocateSize);

		// 外置きフィールドフラグ配列

		; _SYDNEY_ASSERT(this->m_FieldOutsideArray !=
						 this->m_FieldOutsideLocalArray);
		; _SYDNEY_ASSERT(this->m_FieldOutsideArrayAllocateSize > 0);

		ModDefaultManager::free(this->m_FieldOutsideArray,
								this->m_FieldOutsideArrayAllocateSize);

		// 配列フィールドフラグ配列

		; _SYDNEY_ASSERT(this->m_IsArrayFieldArray !=
						 this->m_IsArrayFieldLocalArray);
		; _SYDNEY_ASSERT(this->m_IsArrayFieldArrayAllocateSize > 0);

		ModDefaultManager::free(this->m_IsArrayFieldArray,
								this->m_IsArrayFieldArrayAllocateSize);

		// 要素タイプ配列（配列フィールド用）

		; _SYDNEY_ASSERT(this->m_ElementTypeArray !=
						 this->m_ElementTypeLocalArray);
		; _SYDNEY_ASSERT(this->m_ElementTypeArrayAllocateSize > 0);

		ModDefaultManager::free(this->m_ElementTypeArray,
								this->m_ElementTypeArrayAllocateSize);

		// 最大要素数配列（配列フィールド用）

		; _SYDNEY_ASSERT(this->m_ElementMaxNumArray !=
						 this->m_ElementMaxNumLocalArray);
		; _SYDNEY_ASSERT(this->m_ElementMaxNumArrayAllocateSize > 0);

		ModDefaultManager::free(this->m_ElementMaxNumArray,
								this->m_ElementMaxNumArrayAllocateSize);

		// 固定長要素フラグ配列

		; _SYDNEY_ASSERT(this->m_IsFixedElementArray !=
						 this->m_IsFixedElementLocalArray);
		; _SYDNEY_ASSERT(this->m_IsFixedElementArrayAllocateSize > 0);

		ModDefaultManager::free(this->m_IsFixedElementArray,
								this->m_IsFixedElementArrayAllocateSize);

		// 要素最大長配列（配列フィールド用）

		; _SYDNEY_ASSERT(this->m_ElementMaxLengthArray !=
						 this->m_ElementMaxLengthLocalArray);
		; _SYDNEY_ASSERT(this->m_ElementMaxLengthArrayAllocateSize > 0);

		ModDefaultManager::free(this->m_ElementMaxLengthArray,
								this->m_ElementMaxLengthArrayAllocateSize);

		// キーフィールドソート順配列

		; _SYDNEY_ASSERT(this->m_KeyFieldSortOrderArray !=
						 this->m_KeyFieldSortOrderLocalArray);
		; _SYDNEY_ASSERT(this->m_KeyFieldSortOrderArrayAllocateSize > 0);

		ModDefaultManager::free(this->m_KeyFieldSortOrderArray,
								this->m_KeyFieldSortOrderArrayAllocateSize);

		// キーフィールド比較結果との乗数配列

		; _SYDNEY_ASSERT(this->m_MultiNumberArray !=
						 this->m_MultiNumberLocalArray);
		; _SYDNEY_ASSERT(this->m_MultiNumberArrayAllocateSize > 0);

		ModDefaultManager::free(this->m_MultiNumberArray,
								this->m_MultiNumberArrayAllocateSize);
	}
}

//
//	FUNCTION public
//	Btree::FileParameter::getFieldArchiveSize --
//		フィールド値の記録サイズを返す
//
//	NOTES
//	代表オブジェクトに記録する、フィールド値の記録サイズを返す。
//
//	ARGUMENTS
//	const int	FieldIndex_
//		フィールドインデックス
//
//	RETURN
//	Os::Memory::Size
//		フィールド値の記録サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
Os::Memory::Size
FileParameter::getFieldArchiveSize(const int	FieldIndex_) const
{
	Os::Memory::Size	fieldSize = 0;

	if (*(this->m_IsArrayFieldArray + FieldIndex_))
	{
		// 配列フィールド…

		// 外置きオブジェクトのオブジェクトID(6byte)

		fieldSize = File::ObjectIDArchiveSize;
	}
	else if (*(this->m_IsFixedFieldArray + FieldIndex_))
	{
		// 固定長フィールド…

		// フィールドデータの記録サイズを加算する
		fieldSize =
			FileCommon::DataManager::getFixedCommonDataArchiveSize(
				*(this->m_FieldTypeArray + FieldIndex_));
	}
	else
	{
		// 可変長フィールド…

		if (*(this->m_FieldOutsideArray + FieldIndex_))
		{
			// 外置き…

			// 外置きオブジェクトのオブジェクトID(6byte)

			fieldSize = File::ObjectIDArchiveSize;
		}
		else
		{
			// 外置きではない…

			; _SYDNEY_ASSERT(
				*(this->m_FieldMaxLengthArray + FieldIndex_) !=
				File::UnlimitedFieldLen);

			//
			// 外置きではない可変長フィールドは、
			// 代表オブジェクト内で、
			// 下図のような物理構成となっている。
			//
			//     ┌───────────┐
			//     │　　フィールド長　　　│ 1[byte]
			//     ├───────────┤
			//     │　　フィールド値　　　│ *
			//     └───────────┘
			//

			//
			// ※ 外置きではない可変長フィールドでは、
			// 　 フィールド値を記録する領域として、
			// 　 フィールド最大長の分だけ確保してある。
			//

			// フィールド長(1byte)
			// フィールド最大長(*byte)
			fieldSize =
				File::InsideVarFieldLenArchiveSize +
				*(this->m_FieldMaxLengthArray + FieldIndex_);
		}
	}

	return fieldSize;
}

//
//	FUNCTION public
//	Btree::FileParameter::changeBtreeFilePath --
//		ファイル格納先ディレクトリを変更する
//
//	NOTES
//	ファイル格納先ディレクトリを変更する。
//
//	ARGUMENTS
//	const ModUnicodeString&	Path_
//		変更後のファイル格納先ディレクトリパス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileParameter::changeBtreeFilePath(const ModUnicodeString&	Path_)
{
	this->setString( _SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0), Path_);

	bool	isTemp =
		(this->m_BufferingStrategy.m_VersionFileInfo._category ==
		 Buffer::Pool::Category::Temporary);

	bool	mounted = this->getBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key));

	// ツリーファイル格納戦略を設定する
	this->setTreeFileStorageStrategy(Path_, isTemp, mounted);

	// バリューファイル格納戦略を設定する
	this->setValueFileStorageStrategy(Path_, isTemp, mounted);
}

//////////////////////////////////////////////////
//
//	PRIVATE METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION private
//	Btree::FileParameter::setBtreeFilePath --
//		ファイル格納先ディレクトリパスを設定する
//
//	NOTES
//	ファイル格納先ディレクトリパスを設定する。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	FileID_
//		Ｂ＋木ファイル用論理ファイルIDオブジェクトへの参照
//	ModUnicodeString&			Path_
//		ファイル格納先ディレクトリパスへの参照
//
//	RETURN
//	
//
//	EXCEPTIONS
//	[YET!]
//
void
FileParameter::setBtreeFilePath(const LogicalFile::FileID&	FileID_,
								ModUnicodeString&			Path_)
{
	// B+木ファイルは、Area[0]しか使用しない。

	if (FileID_.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
							  FileCommon::FileOption::Area::Key, 0),
						  Path_) == false ||
		!Path_.getLength())
	{
		// ディレクトリが正しく設定されていない…

		SydErrorMessage << "unset Area[0] path." << ModEndl;

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	// 自身にも設定する
	setString( _SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				   FileCommon::FileOption::Area::Key, 0), Path_);

	// しかし、Kernel/SchemaモジュールがArea[1]以降にも
	// 指定することがある。
	// なので、指定されているのならば自身にも設定する。
	// ファイルIDに設定されていれば良く、B+木ファイル側でArea[1]以降を認識する必要はない。
	for (int i = 1; ; ++i) {
		ModUnicodeString	v;
		if (FileID_.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
								  FileCommon::FileOption::Area::Key, i),
							  v) == false)
			break;

		// 自身にも設定する
		setString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
					  FileCommon::FileOption::Area::Key, i), v);
	}
}

//
//	FUNCTION private
//	Btree::FileParameter::setPhysicalPageSize --
//		物理ページサイズを設定する
//
//	NOTES
//	物理ページサイズを設定する。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	FileID_
//		Ｂ＋木ファイル用論理ファイルIDオブジェクトへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
FileParameter::setPhysicalPageSize(const LogicalFile::FileID&	FileID_)
{
	int			paramValue;

	//
	// 利用者が論理ファイルIDに設定する値は“キロバイト単位”。
	//

	if (FileID_.getInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key), paramValue) == false)
	{
		// デフォルトは4K

		this->m_PhysicalPageSize = 4096;
	}
	else
	{
		if (paramValue == 0)
		{
			// 物理ページサイズが正しく設定されていない…

			SydErrorMessage << "illegal page size (= 0)." << ModEndl;

			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}

		// 自身にも設定する
		//this->setInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key) ,paramValue);

		this->m_PhysicalPageSize =
			static_cast<Os::Memory::Size>(paramValue);

		// “キロバイト”→“バイト”
		this->m_PhysicalPageSize <<= 10;
	}

	this->m_PhysicalPageSize =
		Version::File::verifyPageSize(this->m_PhysicalPageSize);

	// “バイト”→“キロバイト”変換して、自身にも設定する
	this->setInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key), this->m_PhysicalPageSize >> 10);
}

//
//	FUNCTION private
//	Btree::FileParameter::setTemporary --
//		一時ファイルかどうかを設定する
//
//	NOTES
//	一時ファイルかどうかを設定する。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	FileID_
//		Ｂ＋木ファイル用論理ファイルIDオブジェクトへの参照
//
//	RETURN
//	bool
//		一時ファイルかどうか
//			true  : 一時ファイル
//			false : 一時ファイルではない
//
//	EXCEPTIONS
//	[YET!]
//
bool
FileParameter::setTemporary(const LogicalFile::FileID&	FileID_)
{
	bool	paramValue;

	if (FileID_.getBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Temporary::Key), paramValue) == false)
	{
		paramValue = false;
	}

	// 自身にも設定する
	this->setBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Temporary::Key), paramValue);

	return paramValue;
}

//
//	FUNCTION private
//	Btree::FileParameter::setReadOnly --
//		読み込み専用かどうかを設定する
//
//	NOTES
//	読み込み専用かどうかを設定する。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	FileID_
//		Ｂ＋木ファイル用論理ファイルIDオブジェクトへの参照
//
//	RETURN
//	bool
//		読み込み専用かどうか
//			true  : 読み込み専用
//			false : 読み込み専用ではない
//
//	EXCEPTIONS
//	[YET!]
//
bool
FileParameter::setReadOnly(const LogicalFile::FileID&	FileID_)
{
	bool	paramValue;

	if (FileID_.getBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::ReadOnly::Key), paramValue) == false)
	{
		paramValue = false;
	}

	// 自身にも設定する
	this->setBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::ReadOnly::Key), paramValue);

	return paramValue;
}

//
//	FUNCTION private
//	Btree::FileParameter::setMounted --
//		マウントされているかどうかを設定する
//
//	NOTES
//	マウントされているかどうかを設定する。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	FileID_
//		Ｂ＋木ファイル用論理ファイルIDオブジェクトへの参照
//
//	RETURN
//	bool
//		マウントされているかどうか
//			true  : マウントされている
//			false : マウントされていない
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//
bool
FileParameter::setMounted(const LogicalFile::FileID&	FileID_)
{
	bool	paramValue;

	if (FileID_.getBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key), paramValue) == false)
	{
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	// 自身にも設定する
	this->setBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key), paramValue);

	return paramValue;
}

//
//	FUNCTION private
//	Btree::FileParameter::setTreeFileStorageStrategy --
//		ツリーファイル格納戦略を設定する
//
//	NOTES
//	ツリーファイル格納戦略を設定する。
//
//	ARGUMENTS
//		const ModUnicodeString&	Path_
//			ファイル格納先ディレクトリパスへの参照
//
//			引数を const Os::Path& にすると、
//			引数に ModUnicodeString を与えたとき、
//			内部で代入するときの都合 2 回コピーが起きてしまうので、
//			引数は const ModUnicodeString& にする
//
//	const bool				IsTemp_
//		一時ファイルかどうか
//			true  : 一時ファイル
//			false : 一時ファイルではない
//	const bool				Mounted_
//		マウントされているかどうか
//			true  : マウントされている
//			false : マウントされていない
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
FileParameter::setTreeFileStorageStrategy(
	const ModUnicodeString&	Path_,
	const bool				IsTemp_,
	const bool				Mounted_)
{
	; _SYDNEY_ASSERT(this->m_PhysicalPageSize > 0);

	this->m_TreeFileStorageStrategy.m_PhysicalFileType =
		PhysicalFile::AreaManageType;

	this->m_TreeFileStorageStrategy.m_PageUseRate = 100;

	this->m_TreeFileStorageStrategy.m_VersionFileInfo._mounted = Mounted_;

	this->m_TreeFileStorageStrategy.m_VersionFileInfo._pageSize =
		this->m_PhysicalPageSize;

	Os::Path& path =
		(m_TreeFileStorageStrategy.m_VersionFileInfo._path._masterData = Path_);
	path.addPart(TreeFile::DirectoryName);

	if (IsTemp_ == false)
	{
		this->m_TreeFileStorageStrategy.m_VersionFileInfo._path._versionLog = path;

		this->m_TreeFileStorageStrategy.m_VersionFileInfo._path._syncLog = path;
	}

	this->m_TreeFileStorageStrategy.m_VersionFileInfo._sizeMax._masterData =
		PhysicalFile::ConstValue::DefaultFileMaxSize;

	this->m_TreeFileStorageStrategy.m_VersionFileInfo._sizeMax._versionLog =
		PhysicalFile::ConstValue::DefaultFileMaxSize;

	this->m_TreeFileStorageStrategy.m_VersionFileInfo._sizeMax._syncLog =
		PhysicalFile::ConstValue::DefaultFileMaxSize;

	this->m_TreeFileStorageStrategy.m_VersionFileInfo._extensionSize._masterData =
		PhysicalFile::ConstValue::DefaultFileExtensionSize;

	this->m_TreeFileStorageStrategy.m_VersionFileInfo._extensionSize._versionLog =
		PhysicalFile::ConstValue::DefaultFileExtensionSize;

	this->m_TreeFileStorageStrategy.m_VersionFileInfo._extensionSize._syncLog =
		PhysicalFile::ConstValue::DefaultFileExtensionSize;
}

//
//	FUNCTION private
//	Btree::FileParameter::setValueFileStorageStrategy --
//		バリューファイル格納戦略を設定する
//
//	NOTES
//	バリューファイル格納戦略を設定する。
//
//	ARGUMENTS
//		const ModUnicodeString&	Path_
//			ファイル格納先ディレクトリパスへの参照
//
//			引数を const Os::Path& にすると、
//			引数に ModUnicodeString を与えたとき、
//			内部で代入するときの都合 2 回コピーが起きてしまうので、
//			引数は const ModUnicodeString& にする
//
//	const bool				IsTemp_
//		一時ファイルかどうか
//			true  : 一時ファイル
//			false : 一時ファイルではない
//	const bool				Mounted_
//		マウントされているかどうか
//			true  : マウントされている
//			false : マウントされていない
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
FileParameter::setValueFileStorageStrategy(
	const ModUnicodeString&	Path_,
	const bool				IsTemp_,
	const bool				Mounted_)
{
	; _SYDNEY_ASSERT(this->m_PhysicalPageSize > 0);

	this->m_ValueFileStorageStrategy.m_PhysicalFileType =
		PhysicalFile::AreaManageType;

	this->m_ValueFileStorageStrategy.m_PageUseRate = 100;

	this->m_ValueFileStorageStrategy.m_VersionFileInfo._mounted = Mounted_;

	this->m_ValueFileStorageStrategy.m_VersionFileInfo._pageSize =
		this->m_PhysicalPageSize;

	Os::Path& path =
	(m_ValueFileStorageStrategy.m_VersionFileInfo._path._masterData = Path_);
	path.addPart(ValueFile::DirectoryName);

	if (IsTemp_ == false)
	{
		this->m_ValueFileStorageStrategy.m_VersionFileInfo._path._versionLog = path;

		this->m_ValueFileStorageStrategy.m_VersionFileInfo._path._syncLog = path;
	}

	this->m_ValueFileStorageStrategy.m_VersionFileInfo._sizeMax._masterData =
		PhysicalFile::ConstValue::DefaultFileMaxSize;

	this->m_ValueFileStorageStrategy.m_VersionFileInfo._sizeMax._versionLog =
		PhysicalFile::ConstValue::DefaultFileMaxSize;

	this->m_ValueFileStorageStrategy.m_VersionFileInfo._sizeMax._syncLog =
		PhysicalFile::ConstValue::DefaultFileMaxSize;

	this->m_ValueFileStorageStrategy.m_VersionFileInfo._extensionSize._masterData =
		PhysicalFile::ConstValue::DefaultFileExtensionSize;

	this->m_ValueFileStorageStrategy.m_VersionFileInfo._extensionSize._versionLog =
		PhysicalFile::ConstValue::DefaultFileExtensionSize;

	this->m_ValueFileStorageStrategy.m_VersionFileInfo._extensionSize._syncLog =
		PhysicalFile::ConstValue::DefaultFileExtensionSize;
}

//
//	FUNCTION private
//	Btree::FileParameter::setBufferingStrategy --
//		バッファリング戦略を設定する
//
//	NOTES
//	バッファリング戦略を設定する。
//
//	ARGUMENTS
//	const bool	IsTemp_
//		一時ファイルかどうか
//			true  : 一時ファイル
//			false : 一時ファイルではない
//	const bool	ReadOnly_
//		読み込み専用かどうか
//			true  : 読み込み専用
//			false : 読み込み専用ではない
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
FileParameter::setBufferingStrategy(const bool	IsTemp_,
									const bool	ReadOnly_)
{
	if (IsTemp_)
	{
		this->m_BufferingStrategy.m_VersionFileInfo._category =
			Buffer::Pool::Category::Temporary;
	}
	else if (ReadOnly_)
	{
		this->m_BufferingStrategy.m_VersionFileInfo._category =
			Buffer::Pool::Category::ReadOnly;
	}
	else
	{
		this->m_BufferingStrategy.m_VersionFileInfo._category =
			Buffer::Pool::Category::Normal;
	}
}

//
//	FUNCTION private
//	Btree::FileParameter::setUniqueType -- ユニークタイプを設定する
//
//	NOTES
//	ユニークタイプを設定する。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	FileID_
//		Ｂ＋木ファイル用論理ファイルIDオブジェクトへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
FileParameter::setUniqueType(const LogicalFile::FileID&	FileID_)
{
	int	paramValue;

	if (FileID_.getInteger(_SYDNEY_BTREE_FILE_PARAMETER_KEY(FileCommon::FileOption::Unique::Key), paramValue) == false)
	{
		this->m_UniqueType = UniqueType::NotUnique;
	}
	else
	{
		if (paramValue == FileCommon::FileOption::Unique::Object)
		{
			this->m_UniqueType = UniqueType::Object;
		}
		else if (paramValue == FileCommon::FileOption::Unique::KeyField)
		{
			this->m_UniqueType = UniqueType::Key;
		}
		else
		{
			SydErrorMessage
				<< "unknown unique type (= \""
				<< paramValue
				<< "\" )."
				<< ModEndl;

			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}

		// 自身にも設定する
		this->setInteger(_SYDNEY_BTREE_FILE_PARAMETER_KEY(FileCommon::FileOption::Unique::Key), paramValue);//???
	}
}

//
//	FUNCTION private
//	Btree::FileParameter::setFieldParam -- フィールドパラメータを設定する
//
//	NOTES
//	フィールドパラメータを設定する。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	FileID_
//		Ｂ＋木ファイル用論理ファイルIDオブジェクトへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
FileParameter::setFieldParam(const LogicalFile::FileID&	FileID_)
{
	this->m_ExistOutsideFieldInKey = false;

	this->m_ExistOutsideFieldInValue = false;

	this->m_ExistVariableFieldInKey = false;

	this->m_ExistVariableFieldInValue = false;

	this->m_ExistArrayFieldInValue = false;

	this->m_OutsideKeyFieldTotalLengthMax = 0;//

	// フィールド数などを設定する
	this->setFieldNum(FileID_);

	// 各配列へのポインタを設定する
	this->setArrayPointer();

	for (int i = 0; i < this->m_FieldNum; i++)
	{
		Common::DataType::Type	fieldType = this->getFieldType(FileID_,
															   i);

		*(this->m_FieldTypeArray + i) = fieldType;

		*(this->m_IsFixedFieldArray + i) =
			(FileCommon::DataManager::isVariable(fieldType) == false || fieldType == Common::DataType::Array);

		if (*(this->m_IsFixedFieldArray + i) == false)
		{
			this->setVariableFieldParam(FileID_, i);
		}

		*(this->m_IsArrayFieldArray + i) = (fieldType == Common::DataType::Array);

		if (*(this->m_IsArrayFieldArray + i))
		{
			this->setArrayFieldParam(FileID_, i);
		}

		if (i > 0 && i < this->m_TopValueFieldIndex)
		{
			this->setSortParam(FileID_, i);
		}
		else
		{
			*(this->m_MultiNumberArray + i) = 1;
		}
	}

	this->setObjectForm();

	this->setKeyPerNode(FileID_);
}

//
//	FUNCTION private
//	Btree::FileParameter::setFieldNum -- フィールド数などを設定する
//
//	NOTES
//	フィールド数などを設定する。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	FileID_
//		Ｂ＋木ファイル用論理ファイルIDオブジェクトへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
FileParameter::setFieldNum(const LogicalFile::FileID&	FileID_)
{
	int	paramValue;

	//
	// フィールド数
	//

	if (FileID_.getInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::FieldNumber::Key), paramValue) == false)
	{
		// フィールド数が省略されている…

		// フィールド数は省略付加
		SydErrorMessage << "illegal number of fields." << ModEndl;

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}
	else
	{
		if (paramValue < 2)
		{
			// オブジェクトには、
			//     ・オブジェクトIDフィールド
			//     ・1つ以上のキーフィールド
			// が含まれなければならない。

			SydErrorMessage
				<< "illegal number of fields ( ="
				<< paramValue
				<< " )."
				<< ModEndl;

			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}

		// 自身にも設定する
		this->setInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::FieldNumber::Key), paramValue);
		this->m_FieldNum = paramValue;
	}

	//
	// キーフィールド数
	//

	if (FileID_.getInteger(_SYDNEY_BTREE_FILE_PARAMETER_KEY(FileCommon::FileOption::KeyFieldNumber::Key), paramValue)
		== false)
	{
		// キーフィールド数が省略されている…

		// キーフィールド数は省略付加
		SydErrorMessage << "illegal number of key fields." << ModEndl;

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}
	else
	{
		if (paramValue < 1 || paramValue >= this->m_FieldNum)
		{
			SydErrorMessage
				<< "illegal number of key fields ( ="
				<< paramValue
				<< " )."
				<< ModEndl;

			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}

		// 自身にも設定する
		this->setInteger(_SYDNEY_BTREE_FILE_PARAMETER_KEY(FileCommon::FileOption::KeyFieldNumber::Key) ,paramValue);//???

		this->m_KeyNum = paramValue;
	}

	//
	// バリューフィールド数
	//

	this->m_ValueNum = this->m_FieldNum - this->m_KeyNum - 1;
	// this->m_FieldNumには、                           ~~~~~
	// オブジェクトIDフィールドの分も含まれているので。

	this->m_TopValueFieldIndex = this->m_KeyNum + 1;
}

//
//	FUNCTION private
//	Btree::FileParameter::getFieldType -- フィールドデータタイプを返す
//
//	NOTES
//	フィールドデータタイプを返す。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	FileID_
//		Ｂ＋木ファイル用論理ファイルIDオブジェクトへの参照
//	const int					FieldIndex_
//		フィールドインデックス
//
//	RETURN
//	Common::DataType::Type
//		フィールドデータタイプ
//
//	EXCEPTIONS
//	[YET!]
//
Common::DataType::Type
FileParameter::getFieldType(const LogicalFile::FileID&	FileID_,
							const int					FieldIndex_)
{
	int			paramValue;

	if (FileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::FieldType::Key, FieldIndex_), paramValue) == false)
	{
		SydErrorMessage
			<< "field type not found. field index = "
			<< FieldIndex_
			<< "."
			<< ModEndl;

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	Common::DataType::Type	fieldType =
		static_cast<Common::DataType::Type>(paramValue);

	if (FieldIndex_ == 0)
	{
		if (fieldType != LogicalFile::ObjectID().getType())
		{
			SydErrorMessage << "illegal object id field type." << ModEndl;

			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}
	}
	else if (this->isSupportFieldType(fieldType) == false)
	{
		SydErrorMessage
			<< "not support field type. field index = "
			<< FieldIndex_
			<< ", field type = "
			<< paramValue
			<< ModEndl;

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	if (FieldIndex_ < this->m_TopValueFieldIndex)
	{
		if (fieldType == Common::DataType::Array ||
			fieldType == Common::DataType::Binary)
		{
			SydErrorMessage
				<< "illegal key field type. field index = "
				<< FieldIndex_
				<< ", key field type = "
				<< paramValue
				<< ModEndl;

			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}
	}

	// 
	this->setInteger( _SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::FieldType::Key, FieldIndex_), paramValue);

	return fieldType;
}

//
//	FUNCTION private
//	Btree::FileParameter::setVariableFieldParam --
//		可変長フィールドパラメータを設定する
//
//	NOTES
//	可変長フィールドパラメータを設定する。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	FileID_
//		Ｂ＋木ファイル用論理ファイルIDオブジェクトへの参照
//	const int					FieldIndex_
//		フィールドインデックス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
FileParameter::setVariableFieldParam(const LogicalFile::FileID&	FileID_,
									 const int					FieldIndex_)
{
	int			paramValue;

	if (FileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::FieldLength::Key, FieldIndex_), paramValue) == false)
	{
		// 無制限可変長

		*(this->m_FieldMaxLengthArray + FieldIndex_) =
			File::UnlimitedFieldLen;

		*(this->m_FieldOutsideArray + FieldIndex_) = true;
	}
	else
	{
		this->setInteger( _SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::FieldLength::Key, FieldIndex_), paramValue);

		Os::Memory::Size	maxLen =
			static_cast<Os::Memory::Size>(paramValue);

		*(this->m_FieldMaxLengthArray + FieldIndex_) = maxLen;

		*(this->m_FieldOutsideArray + FieldIndex_) =
			(maxLen > FileParameter::VariableFieldInsideThreshold);
	}

	if (FieldIndex_ < this->m_TopValueFieldIndex)
	{
		if (this->m_ExistVariableFieldInKey == false)
		{
			this->m_ExistVariableFieldInKey = true;
		}

		if (*(this->m_FieldOutsideArray + FieldIndex_) ) 
		{
			if (this->m_ExistOutsideFieldInKey == false)
			{
				this->m_ExistOutsideFieldInKey = true;
			}

			if (m_OutsideKeyFieldTotalLengthMax != static_cast<Os::Memory::Size>(-1))	// 無制限か？
			{
				// OutsideKeyField長の合計の最大値
				if ( *(this->m_FieldMaxLengthArray + FieldIndex_) == File::UnlimitedFieldLen ) {
					m_OutsideKeyFieldTotalLengthMax = static_cast<Os::Memory::Size>(-1);// どれか一つでも無制限長なら合計も無制限
				} else {
					m_OutsideKeyFieldTotalLengthMax += *(this->m_FieldMaxLengthArray + FieldIndex_);
				}
			}
		}
	}
	else
	{
		if (this->m_ExistVariableFieldInValue == false)
		{
			this->m_ExistVariableFieldInValue = true;
		}

		if (*(this->m_FieldOutsideArray + FieldIndex_) &&
			this->m_ExistOutsideFieldInValue == false)
		{
			this->m_ExistOutsideFieldInValue = true;
		}
	}
}

//
//	FUNCTION private
//	Btree::FileParameter::setArrayFieldParam --
//		配列フィールドパラメータを設定する
//
//	NOTES
//	配列フィールドパラメータを設定する。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	FileID_
//		Ｂ＋木ファイル用論理ファイルIDオブジェクトへの参照
//	const int					FieldIndex_
//		フィールドインデックス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
FileParameter::setArrayFieldParam(const LogicalFile::FileID&	FileID_,
								  const int					FieldIndex_)
{
	; _SYDNEY_ASSERT(FieldIndex_ >= this->m_TopValueFieldIndex);

	int			paramValue;

	if (FileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::ElementType::Key, FieldIndex_), paramValue) == false)
	{
		SydErrorMessage << "element type not found." << ModEndl;

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	// 
	this->setInteger( _SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::ElementType::Key, FieldIndex_), paramValue);

	Common::DataType::Type	elementType = static_cast<Common::DataType::Type>(paramValue);

	*(this->m_ElementTypeArray + FieldIndex_) = elementType;

	if (FileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::FieldLength::Key, FieldIndex_), paramValue) == false)
	{
		// 無制限要素数…
		*(this->m_ElementMaxNumArray + FieldIndex_) = static_cast<int>(File::UnlimitedFieldLen);
	}
	else
	{
		*(this->m_ElementMaxNumArray + FieldIndex_) = paramValue;

		// 
		this->setInteger( _SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::FieldLength::Key, FieldIndex_), paramValue);
	}

	*(this->m_IsFixedElementArray + FieldIndex_) = (FileCommon::DataManager::isVariable(elementType) == false);

	if (*(this->m_IsFixedElementArray + FieldIndex_) == false)
	{
		// 可変長要素…
		if (FileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::ElementLength::Key, FieldIndex_), paramValue) == false)
		{
			// 無制限可変長…
			*(this->m_ElementMaxLengthArray + FieldIndex_) = File::UnlimitedFieldLen;
		}
		else
		{
			this->setInteger( _SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::ElementLength::Key, FieldIndex_), paramValue);

			*(this->m_ElementMaxLengthArray + FieldIndex_) = static_cast<Os::Memory::Size>(paramValue);
		}
	}

	// ※ 配列は外置き固定
	*(this->m_FieldOutsideArray + FieldIndex_) = true;

	if (this->m_ExistArrayFieldInValue == false)
	{
		this->m_ExistArrayFieldInValue = true;
	}

	if (this->m_ExistOutsideFieldInValue == false)
	{
		this->m_ExistOutsideFieldInValue = true;
	}
}

//
//	FUNCTION private
//	Btree::FileParameter::setSortParam --
//		フィールドソートパラメータを設定する
//
//	NOTES
//	フィールドソートパラメータを設定する。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	FileID_
//		Ｂ＋木ファイル用論理ファイルIDオブジェクトへの参照
//	const int					FieldIndex_
//		フィールドインデックス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
FileParameter::setSortParam(const LogicalFile::FileID&	FileID_,
							const int					FieldIndex_)
{
	; _SYDNEY_ASSERT(FieldIndex_ > 0 &&
					 FieldIndex_ < this->m_TopValueFieldIndex);

	bool		paramValue;

	if (FileID_.getBoolean(_SYDNEY_BTREE_FILE_PARAMETER_FORMAT_KEY(Btree::FileOption::FieldSortOrder::Key, FieldIndex_), paramValue) == false)
	{
		paramValue = false;
	}
	else
	{
		this->setBoolean( _SYDNEY_BTREE_FILE_PARAMETER_FORMAT_KEY(Btree::FileOption::FieldSortOrder::Key, FieldIndex_), paramValue );//???
	}

	if (paramValue)
	{
		*(this->m_KeyFieldSortOrderArray + FieldIndex_) = SortOrder::Descending;
		*(this->m_MultiNumberArray + FieldIndex_) = -1;
	}
	else
	{
		*(this->m_KeyFieldSortOrderArray + FieldIndex_) = SortOrder::Ascending;
		*(this->m_MultiNumberArray + FieldIndex_) = 1;
	}
}

//
//	FUNCTION private
//	Btree::FileParameter::setObjectForm --
//		オブジェクトの記録フォーマットを設定する
//
//	NOTES
//	オブジェクトの記録フォーマットを設定する。
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
FileParameter::setObjectForm()
{
	//
	// キー値をキー情報内に記録するかどうかを設定する。
	// ※ キーフィールドのいずれかが可変長フィールドの場合には、
	// 　 キーオブジェクトに記録する。
	//

	Os::Memory::Size	keySize = this->getFixedKeyRealSize();

	bool	keyInfoContainsObject =
		(this->m_ExistVariableFieldInKey == false &&
		 keySize <= KeyPosType::KeyPosThreshold);

	this->m_KeyPosType =
		keyInfoContainsObject ?
			KeyPosType::KeyInfo : KeyPosType::KeyObject;

	//
	// キー値をキー情報内に記録するのであれば、
	// キーオブジェクトのフォーマットは不要である。
	//

	if (this->m_KeyPosType == KeyPosType::KeyInfo)
	{
		// キー値をキー情報内に記録する…

		this->m_KeySize = keySize;

		this->m_DirectKeyObjectSize = 0;
	}
	else
	{
		// キー値をキーオブジェクトに記録する…
		// （キー値をキー情報の外に記録する…）

		this->m_KeySize = 0;

		this->resetPageSizeForLeafKey();
	}

	this->resetPageSizeForValue();
}

//
//	FUNCTION private
//	Btree::FileParameter::getFixedKeyRealSize --
//		固定長キー値の記録サイズを返す
//
//	NOTES
//	固定長キー値の記録サイズを返す。
//	可変長キーフィールドが存在する場合には、0を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Os::Memory::Size
//		固定長キー値の記録サイズ [byte]
//
//	EXCEPTIONS
//	[YET!]
//
Os::Memory::Size
FileParameter::getFixedKeyRealSize() const
{
	if (this->m_ExistVariableFieldInKey)
	{
		return 0;
	}

	Os::Memory::Size	keyRealSize = 0;

	for (int i = 1; i < this->m_TopValueFieldIndex; i++)
	{
		Common::DataType::Type	fieldType = *(this->m_FieldTypeArray + i);

		; _SYDNEY_ASSERT(
			fieldType != Common::DataType::Array &&
			FileCommon::DataManager::isVariable(fieldType) == false);

		keyRealSize +=
			FileCommon::DataManager::getFixedCommonDataArchiveSize(fieldType);
	}

	return keyRealSize;
}

//
//	FUNCTION private
//	Btree::FileParameter::resetPageSizeForLeafKey --
//		リーフページのキーオブジェクトのために物理ページサイズを更新する
//
//	NOTES
//	リーフページを構成する1物理ページに、
//	キーオブジェクトが1つ以上記録可能なように、
//	必要ならば物理ページサイズを更新する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	ARGUMENTS
//	[YET!]
//
void
FileParameter::resetPageSizeForLeafKey()
{
	Os::Memory::Size	pageDataSize =
		PhysicalFile::File::getPageDataSize(PhysicalFile::AreaManageType,
											this->m_PhysicalPageSize,
											2);
		                                // ~~~ ① リーフページヘッダ
		                                //     ② 代表キーオブジェクト

	Os::Memory::Size	leafPageHeaderSize =
		NodePageHeader::getArchiveSize(true); // リーフページ

	// 「リーフページヘッダ」の分を引く
	Os::Memory::Size	leafPageFreeSizeMax =
		pageDataSize - leafPageHeaderSize;

	this->m_DirectKeyObjectSize = this->getDirectKeyObjectSize();

	bool	changePageSize = false;

	while (this->m_DirectKeyObjectSize > leafPageFreeSizeMax)
	{
		//
		// 物理ページサイズを大きくしてみる。
		//

		changePageSize = true;

		// ページサイズを2倍にして、それを矯正する。
		resizePhysicalPageSize();

		pageDataSize =
			PhysicalFile::File::getPageDataSize(PhysicalFile::AreaManageType,
												this->m_PhysicalPageSize,
												2);

		leafPageFreeSizeMax = pageDataSize - leafPageHeaderSize;
	}

	if (changePageSize)
	{
		// “バイト”→“キロバイト”変換して、自身にも設定する
		this->setInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key), this->m_PhysicalPageSize >> 10);
	}
}

//
//	FUNCTION private
//	Btree::FileParameter::resetPageSizeForValue --
//		バリューオブジェクトのために物理ページサイズを更新する
//
//	NOTES
//	1バリューページに、バリューオブジェクトが1つ以上記録可能なように、
//	必要ならば物理ページサイズを更新する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	ARGUMENTS
//	[YET!]
//	
void
FileParameter::resetPageSizeForValue()
{
	Os::Memory::Size	valuePageFreeSizeMax =
		PhysicalFile::File::getPageDataSize(PhysicalFile::AreaManageType,
											this->m_PhysicalPageSize,
											1);
		                                // ~~~ ① 代表バリューオブジェクト

	this->m_DirectValueObjectSize = this->getDirectValueObjectSize();

	bool	changePageSize = false;

	while (this->m_DirectValueObjectSize > valuePageFreeSizeMax)
	{
		//
		// 物理ページサイズを大きくしてみる。
		//

		changePageSize = true;

		// ページサイズを2倍にして、それを矯正する。
		resizePhysicalPageSize();

		valuePageFreeSizeMax =
			PhysicalFile::File::getPageDataSize(PhysicalFile::AreaManageType,
												this->m_PhysicalPageSize,
												1);
	}

	if (changePageSize)
	{
		// “バイト”→“キロバイト”変換して、自身にも設定する
		this->setInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key), this->m_PhysicalPageSize >> 10);
	}
}

//
//	FUNCTION private
//	Btree::FileParameter::getDirectKeyObjectSize --
//		代表キーオブジェクトの記録サイズを返す
//
//	NOTES
//	代表キーオブジェクトの記録サイズを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Os::Memory::Size
//		代表キーオブジェクトの記録サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
Os::Memory::Size
FileParameter::getDirectKeyObjectSize() const
{
	// どんな代表キーオブジェクトでも、
	// オブジェクトタイプは必ず先頭に記録する。
	Os::Memory::Size	keyObjectSize = File::ObjectTypeArchiveSize;

	// ヌルビットマップサイズ分を加算する
	keyObjectSize += NullBitmap::getSize(this->m_KeyNum);

	//
	// 各キーフィールドの記録サイズを加算する
	//
	for (int i = 0; i < this->m_KeyNum; ++i)
	{
		keyObjectSize += this->getFieldArchiveSize(i + 1);
	}

	return keyObjectSize;
}

//
//	FUNCTION private
//	Btree::FileParameter::getDirectValueObjectSize --
//		代表バリューオブジェクトの記録サイズを返す
//
//	NOTES
//	代表バリューオブジェクトの記録サイズを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Os::Memory::Size
//		代表バリューオブジェクトの記録サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
Os::Memory::Size
FileParameter::getDirectValueObjectSize() const
{
	// どんな代表バリューオブジェクトでも、
	// オブジェクトタイプは必ず先頭に記録する。
	Os::Memory::Size	valueObjectSize = File::ObjectTypeArchiveSize;

	// 「リーフページの物理ページ識別子」と
	// 「キー情報のインデックス」の分を加算する
	valueObjectSize += File::PageIDArchiveSize;
	valueObjectSize += File::ModUInt32ArchiveSize;

	// ヌルビットマップサイズ分を加算する
	valueObjectSize += NullBitmap::getSize(this->m_ValueNum);

	//
	// 各バリューフィールドの記録サイズを加算する
	//

	int	fieldIndex = this->m_TopValueFieldIndex;

	for (int i = 0; i < this->m_ValueNum; i++, fieldIndex++)
	{
		valueObjectSize += this->getFieldArchiveSize(fieldIndex);
	}

	return valueObjectSize;
}

//
//	FUNCTION private
//	Btree::FileParameter::analyzeFileHint -- ファイルヒントを解析する
//
//	NOTES
//	ファイルヒントを解析する。
//
//	m_KeyPerNode を参照するので、setFieldParam() の後に呼び出す必要がある。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	FileID_
//		Ｂ＋木ファイル用論理ファイルIDオブジェクトへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正なヒントの値が設定されていた
//
void
FileParameter::analyzeFileHint(const LogicalFile::FileID&	FileID_)
{
	ModUnicodeString fileHint;
	(void) FileID_.getString(
		_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::FileHint::Key),
		fileHint);

	// デフォルト値を設定する。
	int divrate = Hint::NodeKeyDivideRate::Default;
	int chkrate = Hint::NodeMergeCheckRate::Default;
	int exerate = Hint::NodeMergeExecuteRate::Default;

	if (fileHint.getLength()) {
		// Hint のパース
		FileCommon::HintArray	hintArray(fileHint);

		//
		// ノード内のキー分割率
		//
		if (FileParameter::getFileHintValue(hintArray, fileHint, Hint::NodeKeyDivideRate::Key, divrate))
		{
			// ヒントに設定されていた…

			// ちゃんと0〜100[%]が指定されているかチェックする。
			if (divrate < 0 || 100 < divrate)
			{
				SydErrorMessage << "Illegal node key divide rate. (file hint)" << ModEndl;

				throw Exception::BadArgument(moduleName, srcFile, __LINE__);
			}
		}

		//
		// ノードマージチェックの閾値（利用者は率で指定）
		//
		if (FileParameter::getFileHintValue(hintArray, fileHint, Hint::NodeMergeCheckRate::Key, chkrate))
		{
			// ヒントに設定されていた…

			// ちゃんと0〜100[%]が指定されているかチェックする。
			if (chkrate < 0 || 100 < chkrate)
			{
				SydErrorMessage << "Illegal node merge check rate. (file hint)" << ModEndl;

				throw Exception::BadArgument(moduleName, srcFile, __LINE__);
			}
		}

		//
		// ノードマージ実行の閾値（利用者は率で指定）
		//
		if (FileParameter::getFileHintValue(hintArray, fileHint, Hint::NodeMergeExecuteRate::Key, exerate))
		{
			// ヒントに設定されていた…

			// ちゃんと0〜100[%]が指定されているかチェックする。
			if (exerate < 0 || 100 < exerate)
			{
				SydErrorMessage << "Illegal node merge execute rate. (file hint)" << ModEndl;

				throw Exception::BadArgument(moduleName, srcFile, __LINE__);
			}
		}
	}
	this->m_NodeKeyDivideRate = divrate;
	this->m_NodeMergeCheckThreshold = static_cast<ModUInt32>(this->m_KeyPerNode / 100.0 * chkrate);
	this->m_NodeMergeExecuteThreshold = static_cast<ModUInt32>(this->m_KeyPerNode / 100.0 * exerate);
}

//
//	FUNCTION private
//	Btree::FileParameter::getFileHintValue --
//		ファイルヒントに設定されている整数値を得る
//
//	NOTES
//	ファイルヒントに設定されている整数値を得る。
//
//	ARGUMENTS
//	ModUnicodeString&	FileHint_
//		ファイルヒント文字列への参照
//	const char*	const	HintKey_
//		ファイルヒントキーへのポインタ
//	int&				HintValue_
//		ファイルヒントの値への参照
//
//	RETURN
//	bool
//		引数HintKey_で示されるファイルヒントの値が設定されているかどうか
//			true  : ファイルヒントの値が設定されている
//			false : ファイルヒントの値が設定されていない
//
//	EXCEPTIONS
//	[YET!]
//
// static
bool
FileParameter::getFileHintValue(const FileCommon::HintArray& hintArray_,
								const ModUnicodeString& FileHint_,
								const char* const	HintKey_,
								int&				HintValue_)
{
	ModSize	elementNum = hintArray_.getSize();

	if ( elementNum ) {

		ModUnicodeString	hintKey(HintKey_);

		ModSize	hintKeyLength = hintKey.getLength();

		for (ModSize i = 0; i < elementNum; i++)
		{
			FileCommon::HintElement*	hintElement = hintArray_.at(i);

			const ModUnicodeChar*	hintKeyBuff = (const ModUnicodeChar*)hintKey;

			if (hintElement->CompareToKey(FileHint_, (const ModUnicodeChar*)hintKey, hintKeyLength))
			{
				ModAutoPointer<ModUnicodeString> hintValue = hintElement->getValue(FileHint_);

				const ModUnicodeChar*	hintValueBuff = (const ModUnicodeChar*)(*hintValue);

				HintValue_ = ModUnicodeCharTrait::toInt(hintValueBuff);

				return true;
			}
		}
	}
	return false;
}

//
//	FUNCTION private
//	Btree::FileParameter::setArrayPointer --
//		各配列へのポインタを設定する
//
//	NOTES
//	各配列へのポインタを設定する。
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
FileParameter::setArrayPointer()
{
	; _SYDNEY_ASSERT(this->m_FieldNum >= 2);

	if (this->m_FieldNum > FileParameter::LocalLimit)
	{
		// フィールドタイプ配列

		this->m_FieldTypeArrayAllocateSize =
			sizeof(Common::DataType::Type) * this->m_FieldNum;

		this->m_FieldTypeArray =
			static_cast<Common::DataType::Type*>(
				ModDefaultManager::allocate(
					this->m_FieldTypeArrayAllocateSize));

		ModOsDriver::Memory::reset(this->m_FieldTypeArray,
								   this->m_FieldTypeArrayAllocateSize);

		// 固定長フィールドフラグ配列

		this->m_IsFixedFieldArrayAllocateSize =
			sizeof(bool) * this->m_FieldNum;

		this->m_IsFixedFieldArray =
			static_cast<bool*>(
				ModDefaultManager::allocate(
					this->m_IsFixedFieldArrayAllocateSize));

		ModOsDriver::Memory::reset(this->m_IsFixedFieldArray,
								   this->m_IsFixedFieldArrayAllocateSize);

		// フィールド最大長配列

		this->m_FieldMaxLengthArrayAllocateSize =
			sizeof(Os::Memory::Size) * this->m_FieldNum;

		this->m_FieldMaxLengthArray =
			static_cast<Os::Memory::Size*>(
				ModDefaultManager::allocate(
					this->m_FieldMaxLengthArrayAllocateSize));

		ModOsDriver::Memory::reset(this->m_FieldMaxLengthArray,
								   this->m_FieldMaxLengthArrayAllocateSize);

		// 外置きフィールドフラグ配列

		this->m_FieldOutsideArrayAllocateSize =
			sizeof(bool) * this->m_FieldNum;

		this->m_FieldOutsideArray =
			static_cast<bool*>(
				ModDefaultManager::allocate(
					this->m_FieldOutsideArrayAllocateSize));

		ModOsDriver::Memory::reset(this->m_FieldOutsideArray,
								   this->m_FieldOutsideArrayAllocateSize);

		// 配列フィールドフラグ配列

		this->m_IsArrayFieldArrayAllocateSize =
			sizeof(bool) * this->m_FieldNum;

		this->m_IsArrayFieldArray =
			static_cast<bool*>(
				ModDefaultManager::allocate(
					this->m_IsArrayFieldArrayAllocateSize));

		ModOsDriver::Memory::reset(this->m_IsArrayFieldArray,
								   this->m_IsArrayFieldArrayAllocateSize);

		// 要素タイプ配列（配列フィールド用）

		this->m_ElementTypeArrayAllocateSize =
			sizeof(Common::DataType::Type) * this->m_FieldNum;

		this->m_ElementTypeArray =
			static_cast<Common::DataType::Type*>(
				ModDefaultManager::allocate(
					this->m_ElementTypeArrayAllocateSize));

		ModOsDriver::Memory::reset(this->m_ElementTypeArray,
								   this->m_ElementTypeArrayAllocateSize);

		// 最大要素数配列（配列フィールド用）

		this->m_ElementMaxNumArrayAllocateSize =
			sizeof(int) * this->m_FieldNum;

		this->m_ElementMaxNumArray =
			static_cast<int*>(
				ModDefaultManager::allocate(
					this->m_ElementMaxNumArrayAllocateSize));

		ModOsDriver::Memory::reset(this->m_ElementMaxNumArray,
								   this->m_ElementMaxNumArrayAllocateSize);

		// 固定長要素フラグ配列

		this->m_IsFixedElementArrayAllocateSize =
			sizeof(bool) * this->m_FieldNum;

		this->m_IsFixedElementArray =
			static_cast<bool*>(
				ModDefaultManager::allocate(
					this->m_IsFixedElementArrayAllocateSize));

		ModOsDriver::Memory::reset(
			this->m_IsFixedElementArray,
			this->m_IsFixedElementArrayAllocateSize);

		// 要素最大長配列（配列フィールド用）

		this->m_ElementMaxLengthArrayAllocateSize =
			sizeof(Os::Memory::Size) * this->m_FieldNum;

		this->m_ElementMaxLengthArray =
			static_cast<Os::Memory::Size*>(
				ModDefaultManager::allocate(
					this->m_ElementMaxLengthArrayAllocateSize));

		ModOsDriver::Memory::reset(
			this->m_ElementMaxLengthArray,
			this->m_ElementMaxLengthArrayAllocateSize);

		// キーフィールドソート順配列

		this->m_KeyFieldSortOrderArrayAllocateSize =
			sizeof(SortOrder::Value) * this->m_FieldNum;

		this->m_KeyFieldSortOrderArray =
			static_cast<SortOrder::Value*>(
				ModDefaultManager::allocate(
					this->m_KeyFieldSortOrderArrayAllocateSize));

		ModOsDriver::Memory::reset(
			this->m_KeyFieldSortOrderArray,
			this->m_KeyFieldSortOrderArrayAllocateSize);

		// キーフィールド比較結果との乗数配列

		this->m_MultiNumberArrayAllocateSize =
			sizeof(int) * this->m_FieldNum;

		this->m_MultiNumberArray =
			static_cast<int*>(
				ModDefaultManager::allocate(
					this->m_MultiNumberArrayAllocateSize));

		ModOsDriver::Memory::reset(this->m_MultiNumberArray,
								   this->m_MultiNumberArrayAllocateSize);
	}
	else
	{
		this->initializeLocalArray();

		// フィールドタイプ配列

		this->m_FieldTypeArrayAllocateSize = 0;

		this->m_FieldTypeArray = this->m_FieldTypeLocalArray;

		// 固定長フィールドフラグ配列

		this->m_IsFixedFieldArrayAllocateSize = 0;

		this->m_IsFixedFieldArray = this->m_IsFixedFieldLocalArray;

		// フィールド最大長配列

		this->m_FieldMaxLengthArrayAllocateSize = 0;

		this->m_FieldMaxLengthArray = this->m_FieldMaxLengthLocalArray;

		// 外置きフィールドフラグ配列

		this->m_FieldOutsideArrayAllocateSize = 0;

		this->m_FieldOutsideArray = this->m_FieldOutsideLocalArray;

		// 配列フィールドフラグ配列

		this->m_IsArrayFieldArrayAllocateSize = 0;

		this->m_IsArrayFieldArray = this->m_IsArrayFieldLocalArray;

		// 要素タイプ配列（配列フィールド用）

		this->m_ElementTypeArrayAllocateSize = 0;

		this->m_ElementTypeArray = this->m_ElementTypeLocalArray;

		// 最大要素数配列（配列フィールド用）

		this->m_ElementMaxNumArrayAllocateSize = 0;

		this->m_ElementMaxNumArray = this->m_ElementMaxNumLocalArray;

		// 固定長要素フラグ配列（配列フィールド用）

		this->m_IsFixedElementArrayAllocateSize = 0;

		this->m_IsFixedElementArray = this->m_IsFixedElementLocalArray;

		// 要素最大長配列（配列フィールド用）

		this->m_ElementMaxLengthArrayAllocateSize = 0;

		this->m_ElementMaxLengthArray = this->m_ElementMaxLengthLocalArray;

		// キーフィールドソート順配列

		this->m_KeyFieldSortOrderArrayAllocateSize = 0;

		this->m_KeyFieldSortOrderArray = this->m_KeyFieldSortOrderLocalArray;

		// キーフィールド比較結果との乗数配列

		this->m_MultiNumberArrayAllocateSize = 0;

		this->m_MultiNumberArray = this->m_MultiNumberLocalArray;
	}
}

//
//	FUNCTION private
//	Btree::FileParameter::initializeLocalArray --
//		ローカルは配列を初期化する
//
//	NOTES
//	ローカルは配列を初期化する。
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
FileParameter::initializeLocalArray()
{
	for (int i = 0; i < FileParameter::LocalLimit; i++)
	{
		// フィールドタイプ配列

		this->m_FieldTypeLocalArray[i] = Common::DataType::Undefined;

		// 固定長フィールドフラグ配列

		this->m_IsFixedFieldLocalArray[i] = false;

		// フィールド最大長配列

		this->m_FieldMaxLengthLocalArray[i] = 0;

		// 外置きフィールドフラグ配列

		this->m_FieldOutsideLocalArray[i] = false;

		// 配列フィールドフラグ配列

		this->m_IsArrayFieldLocalArray[i] = false;

		// 要素タイプ配列（配列フィールド用）

		this->m_ElementTypeLocalArray[i] = Common::DataType::Undefined;

		// 最大要素数配列（配列フィールド用）

		this->m_ElementMaxNumLocalArray[i] = 0;

		// 固定長要素フラグ配列

		this->m_IsFixedElementLocalArray[i] = false;

		// 要素最大長配列（配列フィールド用）

		this->m_ElementMaxLengthLocalArray[i] = 0;

		// キーフィールドソート順配列

		this->m_KeyFieldSortOrderLocalArray[i] = SortOrder::Undefined;

		// キーフィールド比較結果との乗数配列

		this->m_MultiNumberLocalArray[i] = 0;
	}
}

//
//	FUNCTION private
//	Btree::File::Parameter::setKeyPerNode -- 次数を設定する
//
//	NOTES
//	次数（1ノード辺りのキー数）を設定する。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	FileID_
//		Ｂ＋木ファイル用論理ファイルIDオブジェクトへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
FileParameter::setKeyPerNode(const LogicalFile::FileID&	FileID_)
{
	int	paramValue;

	if (FileID_.getInteger(_SYDNEY_BTREE_FILE_PARAMETER_KEY(Btree::FileOption::KeyObjectPerNode::Key), paramValue) == false)
	{
		// ファイルIDでは、次数が指定されていない…

		//
		// では、デフォルトの次数を設定しましょう。
		//
		this->setDefaultKeyPerNode();	// this->m_KeyPerNode を設定
	}
	else
	{
		if (paramValue < 2)
		{
			// 『2次のB+木』以上じゃないと、木構造ができない。

			SydErrorMessage
				<< "illegal number of key per node ( ="
				<< paramValue
				<< " )."
				<< ModEndl;

			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}

		this->m_KeyPerNode =
			static_cast<ModUInt32>(paramValue);
	}

	Os::Memory::Size	pageDataSize =
		PhysicalFile::File::getPageDataSize(PhysicalFile::AreaManageType,
											this->m_PhysicalPageSize,
											2);
		//                                 ~~~ ① リーフページヘッダ
		//                                     ② キーテーブル

	Os::Memory::Size	leafPageHeaderSize =
		NodePageHeader::getArchiveSize(true); // リーフページ

	//
	// キー値をキー情報内に記録するのであれば、
	// キー値の記録サイズを得る。
	//

	// キーテーブル（キー情報の列）の記録サイズを求める
	Os::Memory::Size	keyTableSize =
		(KeyInformation::getSize(true, // リーフページ
								 this->m_KeyPosType,
								 this->m_KeyNum) +
		 this->m_KeySize) *
		this->m_KeyPerNode;

	bool	changePageSize = false;

	while (keyTableSize > pageDataSize - leafPageHeaderSize)
	{
		//
		// 物理ページサイズを大きくしてみる。
		//

		changePageSize = true;

		// ページサイズを2倍にして、それを矯正する。
		resizePhysicalPageSize();

		pageDataSize =
			PhysicalFile::File::getPageDataSize(PhysicalFile::AreaManageType,
												this->m_PhysicalPageSize,
												2);
	}

	if (changePageSize)
	{
		// “バイト”→“キロバイト”変換して、自身にも設定する
		this->setInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key), this->m_PhysicalPageSize >> 10);
	}
}

//
//	FUNCTION private
//	Btree::FileParameter::setDefaultKeyPerNode --
//		デフォルトの次数を設定する
//
//	NOTES
//	デフォルトの次数を設定する。
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
FileParameter::setDefaultKeyPerNode()
{
	//
	// 『2次のB+木』以上じゃないと、木構造ができないので、
	// もしそうなってしまうのなら、物理ページサイズを大きくして、
	// 『2次のB+木』以上にする。
	//

	this->m_KeyPerNode = 0;

	try
	{
		bool	changePageSize = false;

		while ((this->m_KeyPerNode = this->getDefaultKeyPerNode()) < 2)
		{
			// ページサイズを2倍にして、それを矯正する。
			this->m_PhysicalPageSize = Version::File::verifyPageSize(this->m_PhysicalPageSize << 1);

			changePageSize = true;
		}

		if (changePageSize)
		{
			// 物理ページサイズを大きくした…

			// “バイト”→“キロバイト”変換して、自身にも設定する
			this->setInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key), this->m_PhysicalPageSize >> 10);
		}
	}
	catch (...)
	{
		//
		// ここで例外が投げられるのは、
		// ページサイズをこれ以上大きくできない場合のみ。
		// なので、NotSupportedに翻訳して送出する。
		//

		throw Exception::NotSupported(moduleName, srcFile, __LINE__);
	}

	; _SYDNEY_ASSERT(this->m_KeyPerNode >= 2);
}

//
//	FUNCTION 
//	Btree::FileParameter::getDefaultKeyPerNode -- 
//		次数のデフォルト値を返す
//
//	NOTES
//	次数のデフォルト値を返す。
//
//	キー値をキー情報内に記録するタイプのファイルにおいては、
//	キー情報がページ内に収まる最大数を次数にする。
//
//	キー値をキーオブジェクトに記録するタイプのファイルにおいては、
//	旧：１キーオブジェクト１物理領域を消費すると仮定し、ページ消費サイズが最小になる次数を決定
//		概算では、keyInfoSize + m_DirectKeyObjectSizeを１ページ分の物理領域の大きさを超えないぎりぎりの大きさにする
//		ただしそのとき 可変長キー（や配列）など外置きのキーの場合、m_DirectKeyObjectSize には、
//		そのキーを指すオブジェクトID分のみが加算されている。物理ページで余った領域は、外置きのキーに消費される。
//		問題点：
//		外置きキーを含むキーの場合、求められる次数が必要以上に大きくなる。従ってキーテーブルをサーチする際に、
//		外置きキーへのアクセスのために多くの物理ページへアタッチされてしまう。
//
//	新：物理ページの領域のうち、ページヘッダやキーテーブルなどで消費された部分の残りに、
//		外置き可変長キー（配列も含む）部分が入る。次数が大きすぎる場合、消費される部分が大きくなり
//		外置き可変長キーが同一ページに載らない可能性が高くなる。
//		よって、キー値をキーオブジェクトに記録するタイプのうち、外置き可変長キーを含む場合は、
//		ページヘッダやキーテーブルなどで消費される部分は物理ページの５０％を超えない様にする。
//		＃50%という数値自体には大した根拠はない。
//
//		外置き可変長キーを含むかどうかは、m_ExistVariableFieldInKey と m_ExistOutsideFieldInKey とで判断でき、
//		それは setVariableFieldParam() で既に初期化済みなので、この関数の時点で参照可能
//
//		分割操作について：
//		次数で分割するのではなく、外置きのキーも含めて、複数のページを使用する様になったら分割する。
//		注意：複数ページに跨ったら分割する場合、キーテーブルに存在するキー数が２以上であること。
//		      （複数ページが必要なほど大きな外置きキーの場合も同様。）
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		次数のデフォルト値
//
//	EXCEPTIONS
//	なし
//
ModUInt32
FileParameter::getDefaultKeyPerNode() const
{
	Os::Memory::Size	leafPageHeaderSize =
		NodePageHeader::getArchiveSize(true); // リーフページ

	//
	// キー値をキー情報内に記録するのであれば、
	// キー値の記録サイズを得る。
	//

	Os::Memory::Size	keyInfoSize =
		KeyInformation::getSize(true, // リーフページ
								this->m_KeyPosType,
								this->m_KeyNum) +
		this->m_KeySize;

	ModUInt32	keyPerNode = 10;//次数（返り値）

	if (this->m_KeyPosType == KeyPosType::KeyInfo)
	{
		// キー値をキー情報内に記録するタイプのファイル…

		Os::Memory::Size	pageDataSize =
			PhysicalFile::File::getPageDataSize(
				PhysicalFile::AreaManageType,
				this->m_PhysicalPageSize,
				2);
			// ~~~ ① リーフページヘッダ
			//     ② キーテーブル
			//     物理ページに存在する物理エリアの個数

		pageDataSize -= leafPageHeaderSize;

		// １０（以上）の位の次数を求める
		while (true)
		{
			Os::Memory::Size	keyTableSize = keyInfoSize * keyPerNode;

			// 物理ページに収まる最大次数
			if (pageDataSize < keyTableSize)
			{
				keyPerNode -= 10;

				break;
			}

			keyPerNode += 10;
		}

		// １の位の次数を求める
		for (int i = 0; i < 10; ++i , ++keyPerNode)
		{
			Os::Memory::Size	keyTableSize = keyInfoSize * keyPerNode;

			// 物理ページに収まる最大次数
			if (pageDataSize < keyTableSize)
			{
				--keyPerNode;

				break;
			}
		}
	}
	else
	{
		// キー値をキーオブジェクトに記録するタイプのファイル…

		// 無限長の外置き可変長キーフィールドを含む場合は、テーブル等で消費される部分は物理ページの５０％を超えない。
		const Os::Memory::Size pageSize = ( this->m_ExistVariableFieldInKey
		                                 && this->m_ExistOutsideFieldInKey
		                                 && (m_OutsideKeyFieldTotalLengthMax == static_cast<Os::Memory::Size>(-1)) )
		                                 ? (this->m_PhysicalPageSize / 2) : this->m_PhysicalPageSize;

		// １０（以上）の位の次数を求める
		while (true)
		{
			Os::Memory::Size	pageDataSize =
				PhysicalFile::File::getPageDataSize(
					PhysicalFile::AreaManageType,
					pageSize,
					keyPerNode + 2);
					//         ~~~ ① リーフページヘッダ
					//             ② キーテーブル
					//             ③ キーオブジェクト（複数）
					//             物理ページに存在する物理エリアの個数

			Os::Memory::Size	keyTableSize =
				keyInfoSize * keyPerNode;

			Os::Memory::Size	directObjectTotalSize =
					this->m_DirectKeyObjectSize * keyPerNode;
					//注意：可変長キー（や配列）の場合、m_DirectKeyObjectSize には
					//      そのキーを指すオブジェクトID分のみが加算されている。

			Os::Memory::Size	useSize =
				leafPageHeaderSize + keyTableSize + directObjectTotalSize;

			// 長さの制限がある外置き可変長キーフィールドを含む場合は、
			// 高次の次数を決定する条件のときにのみ、その部分も含めて判断する。
			if (m_OutsideKeyFieldTotalLengthMax != static_cast<Os::Memory::Size>(-1)) {
				useSize += m_OutsideKeyFieldTotalLengthMax * keyPerNode;
			}

			// 物理ページに収まる最大次数
			if (pageDataSize < useSize)
			{
				keyPerNode -= 10;

				break;
			}

			keyPerNode += 10;
		}

		// １の位の次数を求める
		for (int i = 0; i < 10; ++i, ++keyPerNode)
		{
			Os::Memory::Size	pageDataSize =
				PhysicalFile::File::getPageDataSize(
					PhysicalFile::AreaManageType,
					pageSize,
					keyPerNode + 2);
					//         ~~~ ① リーフページヘッダ
					//             ② キーテーブル
					//             ③ キーオブジェクト（複数）
					//             物理ページに存在する物理エリアの個数

			Os::Memory::Size	keyTableSize =
				keyInfoSize * keyPerNode;

			Os::Memory::Size	directObjectTotalSize =
					this->m_DirectKeyObjectSize * keyPerNode;
					//注意：可変長キー（や配列）の場合、m_DirectKeyObjectSize には
					//      そのキーを指すオブジェクトID分のみが加算されている。

			Os::Memory::Size	useSize =
				leafPageHeaderSize + keyTableSize + directObjectTotalSize;

			// 物理ページに収まる最大次数
			if (pageDataSize < useSize)
			{
				--keyPerNode;

				break;
			}
		}
	}

	return keyPerNode;
}

//	FUNCTION private
//	Btree::FileParameter::isSupportFieldType --
//		サポートしているフィールドデータタイプかどうかを知らせる
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataType::Type	FieldType_
//		フィールドデータタイプ
//
//	RETURN
//	bool
//		サポートしているフィールドデータタイプかどうか
//			true  : サポート
//			false : 未サポート
//
//	EXCEPTIONS
//	なし

bool
FileParameter::isSupportFieldType(Common::DataType::Type FieldType_) const
{
	// ※ Decimal 型はカーネル側でまだ実装されていない

	return FieldType_ == Common::DataType::Integer ||
		FieldType_ == Common::DataType::UnsignedInteger ||
		FieldType_ == Common::DataType::Integer64 ||
		FieldType_ == Common::DataType::UnsignedInteger64 ||
		FieldType_ == Common::DataType::String ||
		FieldType_ == Common::DataType::Float ||
		FieldType_ == Common::DataType::Double ||
		FieldType_ == Common::DataType::Date ||
		FieldType_ == Common::DataType::DateTime ||
		FieldType_ == Common::DataType::Binary ||
		FieldType_ == Common::DataType::Array ||
		FieldType_ == Common::DataType::ObjectID;
}

//	FUNCTION private
//	Btree::FileParameter::resizePhysicalPageSize -- ページサイズを2倍にする
//
//	NOTES
//		ページサイズを2倍にして、それを矯正する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
FileParameter::resizePhysicalPageSize()
{
	try {
		// ページサイズを2倍にして、それを矯正する。

		m_PhysicalPageSize =
			Version::File::verifyPageSize(m_PhysicalPageSize << 1);

	} catch (...) {

		// ここで例外が投げられるのは、
		// ページサイズをこれ以上大きくできない場合のみ。
		// なので、NotSupportedに翻訳して送出する。

		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
