// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.cpp --
// 
// Copyright (c) 2005, 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Bitmap";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Bitmap/FileID.h"
#include "Bitmap/BitmapPage.h"
#include "Bitmap/Parameter.h"

#include "FileCommon/FileOption.h"
#include "FileCommon/IDNumber.h"
#include "FileCommon/HintArray.h"

#include "Common/Message.h"
#include "Common/UnicodeString.h"
#include "Common/StringData.h"
#include "Common/ObjectIDData.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"

#include "Version/File.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

namespace
{
	//
	//	基本ページサイズ(kbyte)
	//
	ParameterInteger _cBasicPageSize("Bitmap_BasicPageSize", 4);

	//
	//	基本ページサイズ for 圧縮版(kbyte)
	//
	ParameterInteger _cBasicCompressedPageSize("Bitmap_BasicCompressedPageSize", 4);

	//
	//	最大ページサイズ(kbyte)
	//
	ParameterInteger _cMaxPageSize("Bitmap_MaxPageSize", 64);

	//
	//	最大タプルサイズ(byte)
	//
	ModUInt32 _MaxTupleSize = FileID::MAX_SIZE * sizeof(ModUInt32);

	//
	//	最大フィールド数
	//
	int _MaxFieldCount = 2;

	//
	//	compressed のヒント
	//
	ModUnicodeString _Compressed("compressed");
}

//
//	FUNCTION public
//	Bitmap::FileID::FileID -- コンストラクタ
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
	: LogicalFileID(cLogicalFileID_), m_uiTupleSize(0),
	  m_bFixed(false), m_bArray(false), m_iCompressed(-1),
	  m_iPrecision(-1), m_iScale(-1)
{
}

//
//	FUNCTION public
//	Bitmap::FileID::~FileID -- デストラクタ
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
//	Bitmap::FileID::create -- ファイルIDの内容を作成する
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
	//
	//	FileIDからフィールド情報をロードする(チェックする)
	//
	loadFieldInformation();

	//
	//	ヒントを解釈し値を設定する
	//
	ModUnicodeString cstrHint;
	getString(_SYDNEY_FILE_PARAMETER_KEY(
				  FileCommon::FileOption::FileHint::Key),
			  cstrHint);
	FileCommon::HintArray cHintArray(cstrHint);

	// Compressed
	setCompressed(cstrHint, cHintArray);

	//
	//	システムパラメータから得る
	//

	// PageSize
	ModSize pageSize = _cBasicPageSize.get();	// 基本ページサイズ
	if (isCompressed() == true)
		// 圧縮機能付きの場合は _cBasicCompressedPageSize から
		pageSize = _cBasicCompressedPageSize.get();
	if (pageSize < (FileCommon::FileOption::PageSize::getDefault() >> 10))
		pageSize = (FileCommon::FileOption::PageSize::getDefault() >> 10);
	ModSize size = getTupleSize() * 50 / 1024;
	while (pageSize < size)
	{
		pageSize <<= 1;
		if (pageSize >= static_cast<ModSize>(_cMaxPageSize.get()))
		{
			pageSize = static_cast<ModSize>(_cMaxPageSize.get());
			break;
		}
	}
	pageSize = Version::File::verifyPageSize(pageSize << 10);
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(
						FileCommon::FileOption::PageSize::Key), pageSize >> 10);

	//
	//	その他
	//

	// マウント
	setMounted(true);
	
	// Version
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(
						FileCommon::FileOption::Version::Key),
			   CurrentVersion);

}

//
//	FUNCTION public
//	Bitmap::FileID::getPageSize -- ページサイズを得る
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
	return getInteger(_SYDNEY_FILE_PARAMETER_KEY(
							FileCommon::FileOption::PageSize::Key)) << 10;
}

//
//	FUNCTION public
//	Bitmap::FileID::getLockName -- ロック名を得る
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
//	Bitmap::FileID::isReadOnly -- 読み取り専用か
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
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
							FileCommon::FileOption::ReadOnly::Key));
}

//
//	FUNCTION public
//	Bitmap::FileID::isTemporary -- 一時か
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
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
							FileCommon::FileOption::Temporary::Key));
}

//
//	FUNCTION public
//	Bitmap::FileID::isMounted -- マウントされているか
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
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
							FileCommon::FileOption::Mounted::Key));
}

//
//	FUNCTION public
//	Bitmap::FileID::setMounted -- マウントされているかを設定する
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
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(
						FileCommon::FileOption::Mounted::Key), bFlag_);
}

//
//	FUNCTION public
//	Bitmap::FileID::getPath -- パス名を得る
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
		getString(_SYDNEY_FILE_PARAMETER_KEY(
					  FileCommon::FileOption::Area::Key),
				  m_cPath);
	}
	return m_cPath;
}

//
//	FUNCTION public
//	Bitmap::FileID::setPath -- パス名を設定する
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
	setString(_SYDNEY_FILE_PARAMETER_KEY(
						FileCommon::FileOption::Area::Key), cPath_);
	m_cPath = cPath_;
}

//
//	FUNCTION public
//	Bitmap::FileID::isFixed -- すべてのフィールドが固定長かどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		すべてのフィールドが固定長ならtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isFixed() const
{
	loadFieldInformation();
	return m_bFixed;
}

//
//	FUNCTION public
//	Bitmap::FileID::getKeyType -- キーフィールドの型を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModVector<Data::Type::Value>&
//		キーフィールドの型
//
//	EXCEPTIONS
//
Data::Type::Value
FileID::getKeyType() const
{
	loadFieldInformation();
	return m_eKeyType;
}

//
//	FUNCTION public
//	Bitmap::FileID::getTupleSize -- タプルの最大サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		タプルの最大サイズ
//
//	EXCEPTIONS
//
ModSize
FileID::getTupleSize() const
{
	loadFieldInformation();
	return m_uiTupleSize;
}

//
//	FUNCTION public
//	Bitmap::FileID::getKeySize -- キーの最大サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		キーの最大サイズ
//
//	EXCEPTIONS
//
ModSize
FileID::getKeySize() const
{
	return getTupleSize()
		- Data::UnsignedInteger::getSize(0) * sizeof(ModUInt32);
}

//
//	FUNCTION public
//	Bitmap::FileID::checkVersion -- 指定されたバージョン以降かどうかチェックする
//
//	NOTES
//
//	ARGUMENTS
//	int iVersion_
//		チェックするバージョン
//
//	RETURN
//	bool
//		iVersion_以上のバージョンだった場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::checkVersion(int iVersion_) const
{
	int v;
	if (getInteger(_SYDNEY_FILE_PARAMETER_KEY(
					   FileCommon::FileOption::Version::Key), v) == false)
		v = CurrentVersion;
	return (v >= iVersion_) ? true : false;
}

//
//	FUNCTION public
//	Bitmap::FileID::isArray -- キーが配列かどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		キーが配列の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isArray() const
{
	loadFieldInformation();
	return m_bArray;
}

//
//	FUNCTION public
//	Bitmap::FileID::isCompressed -- 圧縮するか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		圧縮する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isCompressed() const
{
	if (m_iCompressed == -1)
	{
		m_iCompressed =
			(getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Compressed)) ? 1 : 0);
	}
	return (m_iCompressed == 1);
}

//
//	FUNCTION public
//	Bitmap::FileID::createKeyData -- キーのデータ型を得る
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
Common::Data::Pointer
FileID::createKeyData() const
{
	return Data::makeData(getKeyType(), m_iPrecision, m_iScale);
}

//
//	FUNCTION public
//	Bitmap::FileID::createValueData -- バリューのデータ型を得る
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
Common::Data::Pointer
FileID::createValueData() const
{
	if (isCompressed())
		return Common::Data::Pointer(new Common::ObjectIDData);
	else
		return Common::Data::Pointer(new Common::UnsignedIntegerData);
}

//
//	FUNCTION private
//	Bitmap::FileID::loadFieldInformation -- フィールド情報をロードする
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
FileID::loadFieldInformation() const
{
	if (m_uiTupleSize == 0)
	{
		ModSize toalLength = 0;
		
		int fieldnum = getInteger(_SYDNEY_FILE_PARAMETER_KEY(
			FileCommon::FileOption::FieldNumber::Key));

		if (fieldnum > _MaxFieldCount)
		{
			// フィールド数の最大数を超えている
			_SYDNEY_THROW0(Exception::NotSupported);
		}

		ModSize totalLength = 0;

		m_bFixed = false;	// キーが固定長か？
		m_bArray = false;	// キーが配列か？

		// まずキーをチェックする
		bool v2 = checkVersion(Version2);
		m_eKeyType = getFieldType(0, v2, m_bFixed, m_bArray);	// キーの型
		m_iPrecision = -1;
		m_iScale = -1;
		ModSize length = getFieldSize(0, m_eKeyType, m_bArray,
									  m_iPrecision, m_iScale);
		totalLength += length;

		// 次にバリューをチェックする
		bool dummy;
		Data::Type::Value eType = getFieldType(1, v2, dummy, dummy);
		if (eType != Data::Type::UnsignedInteger)
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		int dummy1, dummy2;
		length = getFieldSize(1, eType, false, dummy1, dummy2);
		totalLength += length;
		
		if (totalLength > _MaxTupleSize)
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}

		// 合計サイズ
		m_uiTupleSize = totalLength;
	}
}

//
//	FUNCTION private
//	Bitmap::FileID::readHint -- ヒントを読む
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
//	FUNCTION private
//	Bitmap::FileID::getFieldType -- フィールドの型を得る
//
//	NOTES
//
//	ARGUMENTS
//	int iPosition_
//		フィールド位置
//
//	RETURN
//	Bitmap::Data::Type::Value
//		フィールドの型
//	bool isVersion2_
//		バージョン2以降かどうか
//	bool& isFixed
//		固定長フィールドかどうか
//	bool& isArray
//		配列かどうか
//
//	EXCEPTIONS
//
Data::Type::Value
FileID::getFieldType(int iPosition_, bool isVersion2_,
					 bool& isFixed_, bool& isArray_) const
{
	return Data::getFieldType(*this, iPosition_, isVersion2_,
							  isFixed_, isArray_);
}

//
//	FUNCTION private
//	Bitmap::FileID::getFieldSize -- フィールドのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	int iPosition_
//		フィールド位置
//	Bitmap::Data::Type::Value eType_
//		データ型
//	bool isArray_
//		配列かどうか
//
//	RETURN
//	ModSize
//		フィールドのサイズ(byte)
//
//	EXCEPTIONS
//
ModSize
FileID::getFieldSize(int iPosition_,
					 Data::Type::Value eType_, bool isArray_,
					 int& iPrecision_, int& iScale_) const
{
	return Data::getFieldSize(*this, iPosition_, eType_, isArray_,
							  iPrecision_, iScale_)
		* sizeof(ModUInt32);
}

//
//	FUNCTION private
//	Bitmap::FileID::setCompressed
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
			SydErrorMessage << "Illegal Compressed. " << cstrValue << ModEndl;
			_SYDNEY_THROW0(Exception::BadArgument);
		}
	}

	// FileIDに設定する
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Compressed), bCompressed);
}

//
//	Copyright (c) 2005, 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
