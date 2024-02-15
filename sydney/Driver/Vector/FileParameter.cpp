// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileParameter.cpp -- ファイルパラメータクラスの実装ファイル
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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
const char srcFile[] = __FILE__;
const char moduleName[] = "Vector";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "ModAutoPointer.h"
#include "Common/Assert.h"
#include "Common/Message.h"
#include "Exception/BadArgument.h"
#include "Exception/Unexpected.h"
#include "FileCommon/DataManager.h"
#include "FileCommon/FileOption.h"
#include "LogicalFile/FileID.h"
#include "PhysicalFile/File.h"
#include "Vector/FileParameter.h"
#include "Vector/File.h"

_SYDNEY_USING
_SYDNEY_VECTOR_USING

namespace
{
	//  CONST
	//  $$$::DefaultPhysicalPageSize -- 
	//    物理ページの大きさのデフォルト値
	//
	//  NOTES
	//  物理ページの大きさのデフォルト値。
	//	実際にベクタファイルが利用できるサイズとは異なる。
	//
	const ModSize DefaultPhysicalPageSize = 4 << 10;

	//  CONST
	//  $$$::CountAreaSize -- 
	//    オブジェクトカウント領域のサイズ
	//
	//  NOTES
	//  オブジェクトカウント領域のサイズ。
	//
	const ModSize CountAreaSize = sizeof(ModUInt32);
}

//	FUNCTION public
//	Vector::FileParameter::FileParameter -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//  引数の解釈が必要なので、代入によって
//  ほとんどのメンバ変数に値を与えている。
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
FileParameter::FileParameter(const LogicalFile::FileID& rFileID_)
	: m_ulDataSize(0)
	, m_ulBitMapAreaOffset(CountAreaSize)
	, m_pFieldInfoArrayWithOuterFieldID(0)
	, m_cFileID(rFileID_)
	, m_cIDNumber(m_cFileID)
{
	ModSize ulPageSize;
	int iTmp;
	if (rFileID_.getInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key), iTmp)) {
		// デフォルトではないPageSizeが指定された場合
		ulPageSize = iTmp << 10; 	// Kbyte -> byte
	}
	else
	{
		//	本来、ページサイズが設定されていない場合には、内部でページサイズを
		//	設定する必要があるが、Vectorは設定されていなかった。
		//	もうしょうがないので、Vectorのファイルバージョンをあげて対応する
		
		if (getVersion() >= File::Version2)
		{
			ulPageSize = FileCommon::FileOption::PageSize::getDefault();
			setPageSize(ulPageSize >> 10);
		}
		else
		{
			ulPageSize = DefaultPhysicalPageSize;
		}
	}

	// ファイルオプションに Temporary が設定されているかどうかを調べる
	bool bTemporary = rFileID_.getBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Temporary::Key));

	// ファイルオプションに ReadOnly が設定されているかどうかを調べる
	bool bReadOnly = rFileID_.getBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::ReadOnly::Key));

	// 物理ファイルに関する情報のセット
	// パスの存在確認は File::create() で行う
	ModUnicodeString usstrPhysicalFilePath = 
		rFileID_.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::Area::Key, 0));

	// 物理ファイル格納戦略
	m_cStorageStrategy.m_PhysicalFileType = 
		PhysicalFile::PageManageType;

	m_cStorageStrategy.m_VersionFileInfo._pageSize = ulPageSize;

	m_cStorageStrategy.m_VersionFileInfo._path._masterData =
		usstrPhysicalFilePath;

	if (!bTemporary) {
		m_cStorageStrategy.m_VersionFileInfo._path._versionLog =
		m_cStorageStrategy.m_VersionFileInfo._path._syncLog    =
			usstrPhysicalFilePath;
	}

	m_cStorageStrategy.m_VersionFileInfo._sizeMax._masterData =
	m_cStorageStrategy.m_VersionFileInfo._sizeMax._versionLog =
	m_cStorageStrategy.m_VersionFileInfo._sizeMax._syncLog    =
		PhysicalFile::ConstValue::DefaultFileMaxSize;

	m_cStorageStrategy.m_VersionFileInfo._extensionSize._masterData =
	m_cStorageStrategy.m_VersionFileInfo._extensionSize._versionLog =
	m_cStorageStrategy.m_VersionFileInfo._extensionSize._syncLog    =
		PhysicalFile::ConstValue::DefaultFileExtensionSize;

	bool bMounted;
	if ( rFileID_.getBoolean( _SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key), bMounted ) ) {
		m_cStorageStrategy.m_VersionFileInfo._mounted = bMounted;
	} else {
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// 物理ファイルバッファリング戦略
	m_cBufferingStrategy.m_VersionFileInfo._category =
		bTemporary ? Buffer::Pool::Category::Temporary :
	   (bReadOnly  ? Buffer::Pool::Category::ReadOnly :
					 Buffer::Pool::Category::Normal);

    // m_pFieldInfoArrayWithOuterFieldIDを初期化
	initializeFieldInfoArray(rFileID_);
}

//	FUNCTION public
//	Vector::FileParameter::~FileParameter -- デストラクタ
//
//	NOTES
//  デストラクタ。
//	m_pFieldInfoArrayWithOuterFieldIDを delete [] によって解放している。
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
	if (m_pFieldInfoArrayWithOuterFieldID != 0) {
		delete [] m_pFieldInfoArrayWithOuterFieldID, m_pFieldInfoArrayWithOuterFieldID = 0;
	}
}

// ファイルIDへの参照を返す
const LogicalFile::FileID& 
FileParameter::getFileOption() const
{
	return m_cFileID;
}

//	FUNCTION public
//	Vector::FileParameter::getDirectoryPath -- 
//	  レコードファイル格納先ディレクトリパスを取得
//
//	NOTES
//	レコードファイル格納先ディレクトリパスを取得する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUnicodeString&
//
//	EXCEPTIONS
//	なし
//
const ModUnicodeString&
FileParameter::getDirectoryPath() const
{
	return m_cStorageStrategy.m_VersionFileInfo._path._masterData;
}

// ベクタファイル格納先ディレクトリパスを設定(File::move()が使用)
void
FileParameter::setDirectoryPath(const ModUnicodeString& cPath_)
{
	m_cStorageStrategy.m_VersionFileInfo._path._masterData = cPath_;
	if (m_cBufferingStrategy.m_VersionFileInfo._category != Buffer::Pool::Category::Temporary)
	{
		m_cStorageStrategy.m_VersionFileInfo._path._versionLog =
		m_cStorageStrategy.m_VersionFileInfo._path._syncLog    = cPath_;
	}

#if 0 // 必要?
	// FileIDの変更
	LogicalFile::FileID& fileid = const_cast<LogicalFile::FileID&>(m_cFileID);
	fileid.setString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
						 FileCommon::FileOption::Area::Key, 0), cPath_);
#endif
}

//	FUNCTION public
//	Vector::FileParameter::getStorageStrategy --
//	  物理ファイル格納戦略を取得
//
//	NOTES
//	物理ファイル格納戦略を取得する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const PhysicalFile::File::StorageStrategy& const
//	  物理ファイル格納戦略への参照
//
//	EXCEPTIONS
//	なし
//
const PhysicalFile::File::StorageStrategy&
FileParameter::getStorageStrategy() const
{
	return m_cStorageStrategy;
}

//	FUNCTION public
//	Vector::FileParameter::getBufferingStrategy --
//	  物理ファイルバッファリング戦略を取得
//
//	NOTES
//	物理ファイルバッファリング戦略を取得する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const PhysicalFile::File::BufferingStrategy& const
//	物理ファイルバッファリング戦略への参照
//
//	EXCEPTIONS
//	なし
//
const PhysicalFile::File::BufferingStrategy&
FileParameter::getBufferingStrategy() const
{
	return m_cBufferingStrategy;
}

//	FUNCTION public
//	Vector::FileParameter::getPhysicalPageSize --
//		1物理ページのサイズを取得
//
//	NOTES
//	1物理ページのサイズを取得する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	1物理ページのサイズ
//
//	EXCEPTIONS
//	なし
//
PhysicalFile::PageSize
FileParameter::getPhysicalPageSize() const
{
	return m_cStorageStrategy.m_VersionFileInfo._pageSize;
}

//	FUNCTION public
//	Vector::FileParameter::getBlocksPerPage --
//	  1物理ページあたりのブロック数を取得
//
//	NOTES
//	1物理ページあたりのブロック数を取得。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//	1物理ページあたりのブロック数
//
//	EXCEPTIONS
//	なし
//
ModUInt32
FileParameter::getBlocksPerPage() const
{
	return m_ulBlocksPerPage;
}

//	FUNCTION public
//	Vector::FileParameter::getBlockSize --
//	  1ブロックのサイズを取得
//	NOTES
//	1ブロックのサイズを取得。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//	  1ブロックのサイズ
//
//	EXCEPTIONS
//	なし
//
ModSize
FileParameter::getBlockSize() const
{
	return m_ulBlockSize;
}

//	FUNCTION public
//	Vector::FileParameter::getBitMapAreaOffset --
//		ビットマップ領域へのオフセット(byte)を取得
//
//	NOTES
//	ビットマップ領域へのオフセット(byte)を取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//
//	EXCEPTIONS
//	なし
//
ModSize
FileParameter::getBitMapAreaOffset() const
{
	return m_ulBitMapAreaOffset;
}

#ifdef OBSOLETE
//	FUNCTION public
//	Vector::FileParameter::getBitMapAreaSize --
//	物理ページごとのビットマップ領域のサイズを取得
//
//	NOTES
//	物理ページごとのビットマップ領域のサイズを取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//
//	EXCEPTIONS
//	なし
//
ModSize
FileParameter::getBitMapAreaSize() const
{
	return m_ulBitMapAreaSize;
}
#endif

//	FUNCTION public
//	Vector::FileParameter::getBlockAreaOffset --
//		ブロック領域へのオフセット(byte)を取得
//
//	NOTES
//	ブロック領域へのオフセット(byte)を取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//
//	EXCEPTIONS
//	なし
//
ModSize
FileParameter::getBlockAreaOffset() const
{
	return m_ulBlockAreaOffset;
}

//	FUNCTION public
//	Vector::FileParameter::getOuterFieldNumber --
//	フィールド数を取得  
//
//	NOTES
//  外部用の(i.e. VectorKeyをも数えた)
//	フィールド数を取得する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//	  外部用のフィールド数。
//
//	EXCEPTIONS
//		なし
//
ModUInt32
FileParameter::getOuterFieldNumber() const
{
	return m_ulOuterFieldNumber;
}

//	FUNCTION public
//	Vector::FileParameter::getInnerFieldNumber --
//	フィールド数を取得
//  
//	NOTES
//  内部用の(i.e. VectorKeyを数えない)
//	フィールド数を取得する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//	  内部用のフィールド数。
//
//	EXCEPTIONS
//		なし
//
ModUInt32
FileParameter::getInnerFieldNumber() const
{
	return m_ulOuterFieldNumber-1;
}

//	FUNCTION public
//	Vector::FileParameter::getDataTypeForOuterFieldID --
//	  任意のフィールドのデータ型を取得
//
//	NOTES
//	任意のフィールドのデータ型を取得。
//  フィールドIDは外側での意味。
//
//	ARGUMENTS
//	ModUInt32 ulOuterFieldID_
//	  外部用の意味でのフィールドID。
//
//	RETURN
//	Common::DataType::Type
//	  データの型を表す列挙子。
//
//	EXCEPTIONS
//	なし
//
Common::DataType::Type
FileParameter::getDataTypeForOuterFieldID (ModUInt32 ulOuterFieldID_) const
{
	return m_pFieldInfoArrayWithOuterFieldID[ulOuterFieldID_].m_iDataType;
}

//	FUNCTION public
//	Vector::FileParameter::getDataTypeForInnerFieldID --
//	  任意のフィールドのデータ型を取得
//
//	NOTES
//	任意のフィールドのデータ型を取得。
//  フィールドIDは内側での意味。
//
//	ARGUMENTS
//	ModUInt32 ulInnerFieldID_
//	  内部用の意味でのフィールドID。
//
//	RETURN
//	Common::DataType::Type
//	  データの型を表す列挙子。
//
//	EXCEPTIONS
//	なし
//
Common::DataType::Type
FileParameter::getDataTypeForInnerFieldID
	(ModUInt32 ulInnerFieldID_) const
{
	return m_pFieldInfoArrayWithOuterFieldID[ulInnerFieldID_+1].m_iDataType;
}

#ifdef OBSOLETE
//	FUNCTION public
//	Vector::FileParameter::getInnerFieldSize --
//	  フィールドをファイルに書き込む場合に必要なサイズを取得
//
//	NOTES
//	フィールドをファイルに書き込む場合に必要なサイズを取得。
//  フィールドIDは内側での意味。
//	外側用は作っても意味をなさないので作らない。
//
//	ARGUMENTS
//	ModUInt32 ulInnerFieldID_
//	  内部用の意味でのフィールドID。
//
//	RETURN
//	ModSize
//	  該当のフィールドの大きさ
//
//	EXCEPTIONS
//	なし
//
ModSize
FileParameter::getInnerFieldSize(ModUInt32 ulInnerFieldID_) const
{
	return m_pFieldInfoArrayWithOuterFieldID[ulInnerFieldID_+1].m_ulFieldSize;
}
#endif

//	FUNCTION public
//	Vector::FileParameter::getInnerFieldOffset --
//	  フィールドのブロック先頭からのオフセットを取得
//
//	NOTES
//	フィールドのブロック先頭からのオフセットを取得。
//  フィールドIDは内側での意味。
//	外側用は作っても意味をなさないので作らない。
//
//	ARGUMENTS
//	ModUInt32 ulInnerFieldID_
//	  内部用の意味でのフィールドID。
//
//	RETURN
//	ModOffset
//	  該当のフィールドのオフセット
//
//	EXCEPTIONS
//	なし
//
ModOffset
FileParameter::getInnerFieldOffset(ModUInt32 ulInnerFieldID_) const
{
	return m_pFieldInfoArrayWithOuterFieldID[ulInnerFieldID_+1].m_ulFieldOffset;
}

//	FUNCTION public
//	Vector::FileParameter::equals -- 同値性の検査
//
//	NOTES
//	ファイルパラメータ同士の同値性の検査。
//	ディレクトリパス名だけ比較する。
//
//	ARGUMENTS
//	const FileParameter& rOtherFileParameter_
//	  他のファイルパラメータへの参照。
//
//	RETURN
//	bool
//	  true:  等しい。
//	  false: 等しくない。
//
//	EXCEPTIONS
//	なし
//
bool
FileParameter::equals(const FileParameter& rOtherFileParameter_) const
{
	// 比較結果を返す(文字列の比較結果は ModBooleanであり 
	// C++ のbool値ではないので、値を置き換える必要がある)。
	return (this->getDirectoryPath() 
			== rOtherFileParameter_.getDirectoryPath())
			? true : false;
}

//	FUNCTION private
//	Vector::FileParameter::initializeBlockParameters
//	  ページ当たりのブロック数などに関する情報を設定する
//
//	NOTES
//	  ulDataSize_の値は実際にファイルを開いて見ないと分からないので、
//	  この関数はFile::open()の際に実行される。
//
//	ARGUMENTS
//  ModSize ulDataSize_
//	  物理ページのうち実際にベクタファイルが利用できる領域の大きさ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileParameter::initializeBlockParameters(ModSize ulDataSize_)
{
	if (m_ulDataSize == 0)
	{
	    // 物理ページの利用可能な領域のサイズをセット
		m_ulDataSize = ulDataSize_;
		// ページ辺りのブロック数をセット
		m_ulBlocksPerPage = 8*(m_ulDataSize-CountAreaSize)/(8*m_ulBlockSize+1);
		// ブロック領域のオフセットをセット
		m_ulBlockAreaOffset = m_ulDataSize - m_ulBlockSize*m_ulBlocksPerPage;
		// ビットマップ領域の大きさをセット
		m_ulBitMapAreaSize = (m_ulBlocksPerPage+7)/8;
	}
}

//
// PRIVATE FUNCTION
//

//	FUNCTION private
//	Vector::FileParameter::initializeFieldInfoArray -- 
//	  フィールド情報をベクタに読み込む
//
//	NOTES
//	フィールド情報をベクタに読み込む。
//	フィールド数が0の場合は無駄な処理になるが、害はない。
//
//	ARGUMENTS
//  LogicalFile::FileID& rFileOption_
//	  ファイルオプションへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//  Unexpected
//	  ファイルオプションに不適切な値が含まれていた場合
//
void
FileParameter::initializeFieldInfoArray(const LogicalFile::FileID& rFileID_)
{
	// ここでフィールド数とはオブジェクトIDを含むフィールドの総数である
	m_ulOuterFieldNumber = rFileID_.getInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::FieldNumber::Key));

	// フィールド情報を格納するための配列を確保
	m_pFieldInfoArrayWithOuterFieldID = new FieldInfo[m_ulOuterFieldNumber];

	// バリューを保持する変数
	//- ModUnicodeString cFieldHintValue;

	//#0のセット()
	m_pFieldInfoArrayWithOuterFieldID[0].m_iDataType = 
		Common::DataType::UnsignedInteger;
	//ダミーの値を入れる
	ModUInt32 ulDummy = 0;
	m_pFieldInfoArrayWithOuterFieldID[0].m_ulFieldSize	 = ulDummy;
	m_pFieldInfoArrayWithOuterFieldID[0].m_ulFieldOffset = ulDummy;

	// フィールド情報を取得
	Common::DataType::Type iType;
	int			iTypeDummy;
	ModSize		ulSize;
	bool		bFindType;
	//- bool	bFindFieldHint;

	// レコードオブジェクトの書き込みに必要なサイズを空にしておく
	m_ulBlockSize = 0;
	for (ModUInt32 i = 1; i < m_ulOuterFieldNumber; ++i)
	{
		// フィールドの情報をえるためのキー値を作成
		// (format は内部で ModString::clear を呼び出している)
		//- _SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::FieldHint::Key, i);

		// フィールド型を取得
		bFindType = rFileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::FieldType::Key, i), iTypeDummy);
		iType = static_cast<Common::DataType::Type>(iTypeDummy);
		if (!bFindType) {
			// フィールド型にデフォルト値はない。設定されていなければ例外送出
			SydErrorMessage << "FIELD[" << i << "] : must be set FieldType"
							<< ModEndl;
			throw Exception::Unexpected(moduleName, srcFile, __LINE__);
		}

		// FieldHint
		//- bFindFieldHint = rFileID_.getString(cFieldHint, cFieldHintValue);

		FileCommon::DataManager::DataType type;
		type._name = iType;
		ModAutoPointer<Common::Data>
			pObj = FileCommon::DataManager::createCommonData(type);
		// フィールド長の検査とデフォルト値の設定
		if (pObj->isFixedSize())
		{
			ulSize = pObj->getDumpSize();
		}
		else
		{
			SydErrorMessage	<< "Field[" << i << "] : not supported." << ModEndl;
			throw Exception::Unexpected(moduleName, srcFile, __LINE__);
		}
		m_pFieldInfoArrayWithOuterFieldID[i].m_iDataType		= iType;
		m_pFieldInfoArrayWithOuterFieldID[i].m_ulFieldSize		= ulSize;
		m_pFieldInfoArrayWithOuterFieldID[i].m_ulFieldOffset	= m_ulBlockSize;
		m_ulBlockSize += ulSize;
	}
}

//	FUNCTION public
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileParameter::setMounted()
{
	m_cFileID.setBoolean( _SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key), true );

	m_cStorageStrategy.m_VersionFileInfo._mounted = true;
}

//	FUNCTION public
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileParameter::unsetMounted()
{
	m_cFileID.setBoolean( _SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key), false );

	m_cStorageStrategy.m_VersionFileInfo._mounted = false;
}

#ifdef OBSOLETE
bool
FileParameter::
isMounted() const
{
	return m_cStorageStrategy.m_VersionFileInfo._mounted;
}
#endif

//	FUNCTION public
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileParameter::setVersion(int iVersion_)
{
	m_cFileID.setInteger( _SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Version::Key), iVersion_);
}

//
//	FUNCTION public
//	Vector::FileParameter::getVersion -- ファイルバージョンを得る
//
int
FileParameter::getVersion() const
{
	int v;
	if (m_cFileID.getInteger(
			_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Version::Key), v)
		== false)
		v = File::CurrentVersion;
	return v;
}

//
//	FUNCTION public
//	Vector::FileParameter -- ページサイズを設定する
//
void
FileParameter::setPageSize(ModSize pageSize_)
{
	m_cFileID.setInteger( _SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key), static_cast<int>(pageSize_));
}

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
