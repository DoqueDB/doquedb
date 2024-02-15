// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.cpp --
// 
// Copyright (c) 2003, 2005, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Lob";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Lob/FileID.h"
#include "Lob/Parameter.h"
#include "FileCommon/FileOption.h"
#include "FileCommon/IDNumber.h"
#include "FileCommon/HintArray.h"
#include "Common/Message.h"
#include "Common/UnicodeString.h"
#include "Common/DataType.h"
#include "Exception/BadArgument.h"
#include "Version/File.h"

_SYDNEY_USING
_SYDNEY_LOB_USING

namespace
{
	//
	//	ページサイズ(kbyte)
	//
	ParameterInteger _cPageSize("Lob_FilePageSize", 8);

	//
	//	バージョン
	//
	int _iVersion = 0;
	
	//
	//	compressed のヒント
	//
	ModUnicodeString _Compressed("compressed");
}

//
//	FUNCTION public
//	Lob::FileID::FileID -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const LogicalFile::FileID& cLogicalFileID_
//		論理ファイルインターフェースのFileID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
FileID::FileID(const LogicalFile::FileID& cLogicalFileID_)
	: LogicalFileID(cLogicalFileID_), m_eFileType(FileType::Unknown),
	  m_iCompressed(-1)
{
}

//
//	FUNCTION public
//	Lob::FileID::~FileID -- デストラクタ
//
//	NOTES
//	デストラクタ。
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
FileID::~FileID()
{
}

//
//	FUNCTION public
//	Lob::FileID::create -- ファイルIDの内容を作成する
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
void
FileID::create()
{
	// FileIDの内容をチェックする
	check();
	
	//
	//	ヒントを解釈し値を設定する
	//
	ModUnicodeString cstrHint;
	getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				  FileCommon::FileOption::FieldHint::Key, 1),
			  cstrHint);
	FileCommon::HintArray cHintArray(cstrHint);

	// Compressed
	setCompressed(cstrHint, cHintArray);

	//
	//	システムパラメータから得る
	//

	ModUInt32 size;

	// FilePageSize
	size = Version::File::verifyPageSize(_cPageSize.get() << 10);
	if (size < FileCommon::FileOption::PageSize::getDefault())
		size = FileCommon::FileOption::PageSize::getDefault();
	setInteger(
			_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key),
			size >> 10);

	// Version
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Version::Key),
			   _iVersion);

	// マウント
	setMounted(true);
}

//
//	FUNCTION public
//	Lob::FileID::getPageSize -- ページサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		その他のページのページサイズ
//
//	EXCEPTIONS
//
int
FileID::getPageSize() const
{
	return getInteger(
			 _SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key))
				 << 10;
}

//
//	FUNCTION public
//	Lob::FileID::getLockName -- ロック名を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Lock::FileName&
//		ロック名
//
//	EXCEPTIONS
//
const Lock::FileName&
FileID::getLockName() const
{
	if (m_cLockName.getDatabasePart() == ~static_cast<Lock::Name::Part>(0))
	{
		m_cLockName = FileCommon::IDNumber(*this).getLockName();
	}
	return m_cLockName;
}

//
//	FUNCTION public
//	Lob::FileID::isReadOnly -- 読み取り専用か
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		読み取り専用ならtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isReadOnly() const
{
	return getBoolean(
			_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::ReadOnly::Key));
}

//
//	FUNCTION public
//	Lob::FileID::isTemporary -- 一時か
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		一時ならtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isTemporary() const
{
	return getBoolean(
			_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Temporary::Key));
}

//
//	FUNCTION public
//	Lob::FileID::isMounted -- マウントされているか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		マウントされている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isMounted() const
{
	return getBoolean(
			_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key));
}

//
//	FUNCTION public
//	Lob::FileID::setMounted -- マウントされているかを設定する
//
//	NOTES
//
//	ARGUMENTS
//	bool bFlag_
//		マウントされている場合はtrue、それ以外の場合はfalseを指定
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::setMounted(bool bFlag_)
{
	setBoolean(
		_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key),
		bFlag_);
}

//
//	FUNCTION public
//	Lob::FileID::isCompressed -- 圧縮かどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		圧縮の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isCompressed() const
{
	if (m_iCompressed == -1)
	{
		bool b = getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Compressed));
		m_iCompressed = (b == true) ? 1 : 0;
	}
	return (m_iCompressed == 1) ? true : false;
}

//
//	FUNCTION public
//	Lob::FileID::getPath -- パス名を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Os::Path&
//		パス名
//
//	EXCEPTIONS
//
const Os::Path&
FileID::getPath() const
{
	if (m_cPath.getLength() == 0)
	{
		getString(
			_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Area::Key),
			m_cPath);
	}
	return m_cPath;
}

//
//	FUNCTION public
//	Lob::FileID::setPath -- パス名を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Os::Path& cPath_
//		パス名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::setPath(const Os::Path& cPath_)
{
	setString(
		_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Area::Key), cPath_);
	m_cPath = cPath_;
}

//
//	FUNCTION public static
//	Lob::FileID::checkVersion -- バージョンをチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cLogicalFileID_
//		ファイルID
//
//	RETURN
//	bool
//		現在のバージョンと一致した場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::checkVersion(const LogicalFile::FileID& cLogicalFileID_)
{
	if (cLogicalFileID_.getInteger(_SYDNEY_FILE_PARAMETER_KEY(
		FileCommon::FileOption::Version::Key))
		== _iVersion)
		return true;
	return false;
}

//
//	FUNCTION public
//	Lob::FileID::getFileType -- ファイルタイプを得る
//
//	NOTES
//	ファイルタイプはFieldType[1]の型で決まる
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Lob::FileID::FileType::Value
//		ファイルタイプ
//
//	EXCEPTIONS
//
FileID::FileType::Value
FileID::getFileType() const
{
	if (m_eFileType == FileType::Unknown)
	{
		int type = getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
								FileCommon::FileOption::FieldType::Key, 1));
		switch (type)
		{
		case Common::DataType::Binary:
			m_eFileType = FileType::BLOB;
			break;
		case Common::DataType::String:
			m_eFileType = FileType::NCLOB;
			break;
		default:
			;
		}
	}
	return m_eFileType;
}

//
//	FUNCTION private
//	Lob::FileID::check -- FileIDの内容が正しいかチェックする
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
void
FileID::check()
{
	//
	//	LOBファイルで許されるフィールドは
	//		0 - ObjectID
	//		1 - Binary or String
	//	である。そうなっているかチェックする。
	//

	// フィールド数チェック
	int num = getInteger(_SYDNEY_FILE_PARAMETER_KEY(
							FileCommon::FileOption::FieldNumber::Key));
	if (num != 2)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// フィールドタイプをチェック
	int type = getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
							FileCommon::FileOption::FieldType::Key, 0));
	if (type != Common::DataType::ObjectID)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	type = getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
							FileCommon::FileOption::FieldType::Key, 1));
	if (type != Common::DataType::Binary
		&& type != Common::DataType::String)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}
}

//
//	FUNCTION private
//	Lob::FileID::setCompressed
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeString& cstrHint_
//		ヒント文字列
//	FileCommon::HintArray& cHintArray_
//		ヒント配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::setCompressed(ModUnicodeString& cstrHint_,
					  FileCommon::HintArray& cHintArray_)
{
	ModUnicodeString cstrValue;
	bool bCompressed = false;

	if (readHint(cstrHint_, cHintArray_, _Compressed, cstrValue) == true)
	{
		if (cstrValue.compare(_TRMEISTER_U_STRING("true"), ModFalse) == 0
			|| cstrValue.getLength() == 0)
		{
			bCompressed = true;
		}
		else if (cstrValue.compare(_TRMEISTER_U_STRING("false"), ModFalse) == 0)
		{
			bCompressed = false;
		}
		else
		{
			// エラー
			SydErrorMessage << "Illegal compressed hint. "
							<< cstrValue << ModEndl;
			_SYDNEY_THROW0(Exception::BadArgument);
		}
	}

	// FileIDに設定する
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Compressed), bCompressed);
}

//
//	FUNCTION private
//	Btree2::FileID::readHint -- ヒントを読む
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeString& cstrHint_
//		ヒント文字列
//	FileCommon::HintArray& cHintArray_
//		ヒント配列
//	const ModUncodeString& cstrKey_
//		キー
//	ModUnicodeString& cstrValue_
//		ヒントの値
//
//	RETURN
//	bool
//		ヒントに存在した場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::readHint(ModUnicodeString& cstrHint_,
				 FileCommon::HintArray& cHintArray_,
				 const ModUnicodeString& cstrKey_, ModUnicodeString& cstrValue_)
{
	FileCommon::HintArray::Iterator i = cHintArray_.begin();
	for (; i != cHintArray_.end(); ++i)
	{
		if ((*i)->CompareToKey(cstrHint_,
							   cstrKey_, cstrKey_.getLength()) == true)
		{
			// 見つかった
			if ((*i)->hasValue() == true)
			{
				ModAutoPointer<ModUnicodeString> p = (*i)->getValue(cstrHint_);
				cstrValue_ = *p;
			}
			return true;
		}
	}
	return false;
}

//
//	Copyright (c) 2003, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
