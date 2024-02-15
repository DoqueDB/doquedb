// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.cpp --
// 
// Copyright (c) 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "FileCommon/HintArray.h"
#include "FileCommon/FileOption.h"
#include "FileCommon/IDNumber.h"
#include "FileCommon/OpenOption.h"

#include "Exception/BadArgument.h"

#include "Common/Message.h"
#include "Common/ObjectPointer.h"
#include "Common/IntegerArrayData.h"
#include "Version/File.h"
#include "Inverted/IntermediateFileID.h"
#include "Inverted/SortParameter.h"
#include "Inverted/FileIDNumber.h"
#include "Inverted/FieldType.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

namespace
{
	//
	//	delayed のヒント
	//
	ModUnicodeString _Delayed("delayed");

	//
	//	invertedのヒント
	//
	ModUnicodeString _Inverted("inverted");

	//

	namespace Parameter {
		namespace KeyNumber {
			enum Value {
				Delayed = LogicalFile::FileID::DriverNumber::FullText,//Inverted,
				Sectionized,
				Normalizing,
				SchemaIndexID,
				InvertedFileID,
				SectionInfoFileID,
				DelayProcFileID,
				InvFilePairNum,
				Language,
				ScoreModifier,
				OtherFieldNum,
				OtherFieldType,
				ValueNum
			};
		}
	}
}


//
//	FUNCTION public
//	Inverted::IntermediateFileID::IntermediateFileID -- コンストラクタ
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
//	なし
//
IntermediateFileID::IntermediateFileID(const LogicalFile::FileID& cFileID_)
	: LogicalFileID(cFileID_)
{
}

//
//	FUNCTION public
//	Inverted::IntermediateFileID::~IntermediateFileID -- デストラクタ
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
//	なし
//
IntermediateFileID::~IntermediateFileID()
{
}

//
//	FUNCTION public
//	Inverted::IntermediateFileID::isMounted -- マウントされているか
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
IntermediateFileID::isMounted() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
							FileCommon::FileOption::Mounted::Key));
}

//
//	FUNCTION public
//	Inverted::IntermediateFileID::setMounted -- マウントされているかを設定する
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
IntermediateFileID::setMounted(bool bFlag_)
{
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(
						FileCommon::FileOption::Mounted::Key), bFlag_);
}

//
//	FUNCTION public
//	Inverted::IntermediateFileID::isTemporary -- 一時データベースかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		一時データベースの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
IntermediateFileID::isTemporary() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
							FileCommon::FileOption::Temporary::Key));
}

//
//	FUNCTION public
//	Inverted::IntermediateFileID::isReadOnly -- ReadOnlyかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		ReadOnlyの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
IntermediateFileID::isReadOnly() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
							FileCommon::FileOption::ReadOnly::Key));
}

//
//
//	FUNCTION public
//	Inverted::IntermediateFileID::getLockName -- ロック名を得る
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
IntermediateFileID::getLockName() const
{
	if (m_cLockName.getDatabasePart() == ~static_cast<Lock::Name::Part>(0))
	{
		m_cLockName = FileCommon::IDNumber(*this).getLockName();
	}
	return m_cLockName;
}

//
//	FUNCTION public
//	Inverted::IntermediateFileID::getPath -- パス名を得る
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
IntermediateFileID::getPath() const
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
//	Inverted::IntermediateFileID::setPath -- パス名を設定する
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
IntermediateFileID::setPath(const Os::Path& cPath_)
{
	m_cPath = cPath_;
	setString(_SYDNEY_FILE_PARAMETER_KEY(
		FileCommon::FileOption::Area::Key), cPath_);
}

//
//	FUNCTION public
//	Inverted::IntermediateFileID::getInverted -- 大転置のファイルIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID&
IntermediateFileID::getInverted() const
{
	return setCurrentValue(getInvertedFileID());
}

//
//	FUNCTION public
//	Inverted::IntermediateFileID::getInsert0 -- 挿入用転置0のファイルIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID&
IntermediateFileID::getInsert0()
{
	return setCurrentValue(getDelayProcFileID(FileIDNumber::_Ins0));
}

//
//	FUNCTION public
//	Inverted::IntermediateFileID::getInsert1 -- 挿入用転置1のファイルIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID&
IntermediateFileID::getInsert1()
{
	return setCurrentValue(getDelayProcFileID(FileIDNumber::_Ins1));
}

//
//	FUNCTION public
//	Inverted::IntermediateFileID::getExpunge0 -- 削除用転置0のファイルIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID&
IntermediateFileID::getExpunge0()
{
	return setCurrentValue(getDelayProcFileID(FileIDNumber::_Exp0));
}

//
//	FUNCTION public
//	Inverted::IntermediateFileID::getExpunge1 -- 削除用転置1のファイルIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID&
IntermediateFileID::getExpunge1()
{
	return setCurrentValue(getDelayProcFileID(FileIDNumber::_Exp1));
}

//
//	FUNCTION public
//	Inverted::IntermediateFileID::setInverted -- 大転置のファイルIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IntermediateFileID::setInverted(const LogicalFile::FileID& cFileID_)
{
	getInvertedFileID() = cFileID_;
}

//
//	FUNCTION public
//	Inverted::IntermediateFileID::setInsert0 -- 小転置のファイルIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//

void
IntermediateFileID::setInverted(const LogicalFile::FileID& cFileID_,int id_)
{
	getDelayProcFileID(id_) = cFileID_;
}
//
//	FUNCTION private
//	Inverted::IntermediateFileID::readHint -- ヒントを読む
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
IntermediateFileID::readHint(ModUnicodeString& cstrHint_,
				 FileCommon::HintArray& cHintArray_,
				 const ModUnicodeString& cstrKey_,
				 ModUnicodeString& cstrValue_)
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
//	Inverted::IntermediateFileID::getInvertedFileID -- 大転置のFileIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID&
IntermediateFileID::getInvertedFileID() const
{
	Common::ObjectPointer<LogicalFile::FileID> pFileID
		= getFileID(_SYDNEY_FILE_PARAMETER_KEY(
			Parameter::KeyNumber::InvertedFileID));
	return *pFileID;
}

//
//	FUNCTION private
//	Inverted::IntermediateFileID::getDelayProcFileID -- 小転置のファイルIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	int iIndex_
//		取得する小転置の配列上の位置
//
//	RETURN
//	LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID&
IntermediateFileID::getDelayProcFileID(int iIndex_)
{
	Common::ObjectPointer<LogicalFile::FileID> pFileID
		= getInvertedFileID().getFileID(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
			Parameter::KeyNumber::DelayProcFileID, iIndex_));
	return *pFileID;
}


//
//	FUNCTION public
//	Inverted::IntermediateFileID::setCurrentValue -- 子FileIDに親の現在値を設定する
//
//	NOTES
//	スキーマ等が設定してくれるFileID中の値を、子のFileIDにも反映する。
//
//	ARGUMENTS
//	LogicalFile::FileID& cFileID_
//		設定するファイルID
//
//	RETURN
//	LogicalFile::FileID&
//		値が設定された引数のファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID &
IntermediateFileID::setCurrentValue(LogicalFile::FileID& cFileID_) const
{
	// マウントされているかどうか
	cFileID_.setBoolean(_SYDNEY_FILE_PARAMETER_KEY(
		FileCommon::FileOption::Mounted::Key), isMounted());
	// ReadOnlyかどうか
	cFileID_.setBoolean(_SYDNEY_FILE_PARAMETER_KEY(
		FileCommon::FileOption::ReadOnly::Key), isReadOnly());
	// データベースID
	cFileID_.setInteger(_SYDNEY_FILE_PARAMETER_KEY(
		FileCommon::FileOption::DatabaseID::Key),
						getInteger(_SYDNEY_FILE_PARAMETER_KEY(
							FileCommon::FileOption::DatabaseID::Key)));

	return cFileID_;
}

//
//	Copyright (c) 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
