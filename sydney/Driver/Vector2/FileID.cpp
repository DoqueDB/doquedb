// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.h -- LogicalFile::FileIDのラッパークラス
// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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
#include "Vector2/FileID.h"

#include "Common/Assert.h"
#include "FileCommon/FileOption.h"
#include "FileCommon/IDNumber.h"
#include "Version/Page.h"
#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"

_SYDNEY_BEGIN
_SYDNEY_VECTOR2_BEGIN

namespace
{
	//
	//	基本ページサイズ(kbyte)
	//	Btree2,Lobでは各モジュールのParameterクラスを使っていたがintで十分
	//
	int _BasicPageSize = 16;
}

//
//	FUNCTION public
//	Vector2::FileID::FileID -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//	[?] この時点ではフィールドの型(m_vecFieldType)はわからない？
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
	: LogicalFileID(cLogicalFileID_), m_uiFieldCount(0), m_uiFieldSize(0)
{
}

//
//	FUNCTION public
//	Vector2::FileID::~FileID -- デストラクタ
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
//	Vector2::FileID::create -- ファイルIDの内容を作成する
//
//	NOTES
//	フィールド情報のチェック
//	ページサイズ(ヘッダーとデータ)の設定
//	バージョン(VectorかVector2か)の設定
//	マウントされていることを示すフラグを立てる
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
	//	システムパラメータから得る
	//

	// PageSize
	ModSize pageSize = _BasicPageSize << 10;	// 基本ページサイズ
	if (pageSize < FileCommon::FileOption::PageSize::getDefault())
		pageSize = FileCommon::FileOption::PageSize::getDefault();
	setInteger(
		_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key),
		pageSize >> 10);

	//
	//	その他
	//

	// マウント
	setMounted(true);
	
	// Version
	setInteger(
		_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Version::Key),
		CurrentVersion);
}

//
//	FUNCTION public
//	Vector2::FileID::getPageSize -- ページサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	OS::Memory::Size
//		ページサイズ
//
//	EXCEPTIONS
//
Os::Memory::Size
FileID::getPageSize() const
{
	return static_cast<ModSize>(
		getInteger(_SYDNEY_FILE_PARAMETER_KEY(
					   FileCommon::FileOption::PageSize::Key))) << 10;
}

//
//	FUNCTION public
//	Vector2::FileID::getPageDataSize -- ページデータサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Os::Memory::Size
//		ページデータサイズ
//
//	EXCEPTIONS
//
Os::Memory::Size
FileID::getPageDataSize() const
{
	return Version::Page::getContentSize(getPageSize());
}

//
//	FUNCTION public
//	Vector2::FileID::getLockName -- ロック名を得る
//
//	NOTES
//	VectorFileのコンストラクタでVersion::File::attach()する際に必要
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
	if (m_cLockName.getDatabasePart()
		== ~static_cast<Lock::Name::Part>(0))
	{
		m_cLockName = FileCommon::IDNumber(*this).getLockName();
	}
	return m_cLockName;
}

//
//	FUNCTION public
//	Vector2::FileID::isReadOnly -- 読み取り専用か
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
//	Vector2::FileID::isTemporary -- 一時か
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
//	Vector2::FileID::isMounted -- マウントされているか
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
//	Vector2::FileID::setMounted -- マウントされているかを設定する
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
//	Vector2::FileID::getPath -- パス名を得る
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
//	Vector2::FileID::setPath -- パス名を設定する
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
//	Vector2::FileID::getFieldType -- フィールドの型を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Data::Type::Value*
//		フィールドの型の配列
//@@	const ModVector<Data::Type::Value>&
//@@		フィールドの型の配列
//
//	EXCEPTIONS
//
const Data::Type::Value*
FileID::getFieldType() const
{
	loadFieldInformation();
	return m_pFieldType;
}

//
//	FUNCTION public
//	Vector2::FileID::getFieldCount -- フィールド数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModSize
//		フィールド
//
//	EXCEPTIONS
//
const ModSize
FileID::getFieldCount() const
{
	loadFieldInformation();
	return m_uiFieldCount;
}

//
//	FUNCTION public
//	Vector2::FileID::getFieldSize -- 合計フィールドサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModSize
//		フィールド
//
//	EXCEPTIONS
//
const ModSize
FileID::getFieldSize() const
{
	loadFieldInformation();
	return m_uiFieldSize;
}

//
//	FUNCTION public static
//	Vector2::FileID::checkVersion -- Vector2のバージョンかチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cLogicalFileID_
//		ファイルID
//
//	RETURN
//	bool
//		バージョン情報がない、または指定のバージョン以上の場合はtrue、
//		それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::checkVersion(const LogicalFile::FileID& cLogicalFileID_)
{
	int iVersion;
	if (cLogicalFileID_.getInteger(
		_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Version::Key),
		iVersion) == false)
		return true;
	return (iVersion >= Version3) ? true : false;
}

//
//	FUNCTION public
//	Vector2::FileID::getCountFieldNumber
//		-- 全件数が入っているフィールド番号を得る
//
//	NOTES
//	実際は全件数( count(*) )が入っているフィールドは存在しないので、
//	全フィールドの直後にそのような値が入っているフィールドを仮想する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		フィールド番号
//
//	EXCEPTIONS
//
ModSize
FileID::getCountFieldNumber() const
{
	loadFieldInformation();
	return m_uiFieldCount + 1;
	//@@return m_vecFieldType.getSize() + 1;
}

//
//	FUNCTION private
//	Vector2::FileID::loadFieldInformation -- フィールド情報をロードする
//
//	NOTES
//	フィールドの型とそれらの合計サイズを設定する。
//	使用可能な型かどうかも、getFieldType()経由のData::getFieldType()で
//	チェックされる。
//	フィールドサイズの合計が設定されていたら、すでにロード済み。
//
//	[!] キーの型はチェックしていない
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//		フィールド数の超過
//		キーの数
//
void
FileID::loadFieldInformation() const
{
	if (m_uiFieldSize == 0)
	{
		// Schema::SystemFileSub::_SetVectorFileIDで初期値が設定される

		// フィールド数は、キーの数(1つ)を含んでいる
		int fieldnum = getInteger(_SYDNEY_FILE_PARAMETER_KEY(
			FileCommon::FileOption::FieldNumber::Key));
		// キーの数(1つ)は設定はされていない。
		//int keynum = getInteger(_SYDNEY_FILE_PARAMETER_KEY(
		//	FileCommon::FileOption::KeyFieldNumber::Key));
		int keynum = 1;
		
		if (fieldnum > Data::MaxFieldCount)
		{
			// フィールド数の最大数を超えている
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		// キーの数は設定されていないのでチェック不要
		//if (keynum != 1)
		//{
		//	//ベクターのキーは1つのみ
		//	_SYDNEY_THROW0(Exception::NotSupported);
		//}
		if (fieldnum <= keynum)
		{
			// キー以外のフィールドが指定されていない
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		//@@ Vectorはキーを書き込まないのでキーを除いた分の配列を確保する
		//@@m_vecFieldType.reserve(fieldnum - keynum);

		ModSize totalLength = 0;
		// キーはフィールドに含めない
		// [?] キーの型もチェックした方がいい？
		for (int i = keynum; i != fieldnum; ++i)
		{
			// 利用可能な型かどうかをチェックし、サイズを得る
			ModSize fieldSize;
			Data::Type::Value eType = getFieldType(i, fieldSize);

			totalLength += fieldSize;
			m_uiFieldCount++;
			m_pFieldType[i - keynum] = eType;
			//@@m_vecFieldType.pushBack(eType);
		}

		// 合計サイズ
		m_uiFieldSize = totalLength;
	}
}

//
//	FUNCTION private
//	Vector2::FileID::getFieldType -- フィールドの型を得る
//
//	NOTES
//	フィールドの型とそのサイズが得られる。
//	使用可能な型かどうかもチェックされる。
//
//	ARGUMENTS
//	int iPosition_
//		フィールド位置
//	ModSize& uiFieldSize_
//		フィールドのサイズを入れる参照
//
//	RETURN
//	Vector2::Data::Type::Value
//		フィールドの型
//
//	EXCEPTIONS
//

Data::Type::Value
FileID::getFieldType(int iPosition_, ModSize& uiFieldSize_) const
{
	return Data::getFieldType(*this, iPosition_, uiFieldSize_);
}

_SYDNEY_VECTOR2_END
_SYDNEY_END

//
//	Copyright (c) 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
