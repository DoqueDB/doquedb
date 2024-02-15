// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.cpp --
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2014, 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Btree2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Btree2/FileID.h"
#include "Btree2/Parameter.h"

#include "FileCommon/FileOption.h"
#include "FileCommon/IDNumber.h"
#include "FileCommon/HintArray.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/UnicodeString.h"
#include "Common/StringData.h"

#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"

#include "Version/File.h"

_SYDNEY_USING
_SYDNEY_BTREE2_USING

namespace
{
	//
	//	基本ページサイズ(kbyte)
	//
	ParameterInteger _cBasicPageSize("Btree2_BasicPageSize", 4);

	//
	//	最大ページサイズ(kbyte)
	//
	ParameterInteger _cMaxPageSize("Btree2_MaxPageSize", 64);

	//
	//	異表記正規化方法
	//
	ParameterInteger _cNormalizingMethod("Execution_LikeNormalizedString", 1);

	//
	//	最大タプルサイズ(byte)
	//
	ModUInt32 _MaxTupleSize = FileID::MAX_SIZE * sizeof(ModUInt32);

	//
	//	not null のヒント
	//
	ModUnicodeString _NotNull("not null");

	//
	//	normalized のヒント
	//
	ModUnicodeString _Normalized("normalized");
}

//
//	FUNCTION public
//	Btree2::FileID::FileID -- コンストラクタ
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
	  m_eNormalizingMethod(Utility::CharTrait::NormalizingMethod::Unknown),
	  m_iNormalized(-1), m_bLastRowID(false)
{
}

//
//	FUNCTION public
//	Btree2::FileID::~FileID -- デストラクタ
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
//	Btree2::FileID::create -- ファイルIDの内容を作成する
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
	//	Set the condition of null.
	//
	setNull();

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

	// Normalized
	setNormalized(cstrHint, cHintArray);

	//
	//	システムパラメータから得る
	//

	// PageSize
	ModSize pageSize = _cBasicPageSize.get();	// 基本ページサイズ
	if (pageSize < (FileCommon::FileOption::PageSize::getDefault() >> 10))
		pageSize = (FileCommon::FileOption::PageSize::getDefault() >> 10);
	ModSize size = getTupleSize() * 200 / 1024;
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
//	Btree2::FileID::getPageSize -- ページサイズを得る
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
//	Btree2::FileID::getLockName -- ロック名を得る
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
//	Btree2::FileID::isReadOnly -- 読み取り専用か
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
//	Btree2::FileID::isTemporary -- 一時か
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
//	Btree2::FileID::isMounted -- マウントされているか
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
//	Btree2::FileID::setMounted -- マウントされているかを設定する
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
//	Btree2::FileID::isNormalized -- 異表記正規化ありか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		異表記正規化ありの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isNormalized() const
{
	if (m_iNormalized == -1)
	{
		m_iNormalized =
			(getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Normalized)) ? 1 : 0);
	}
	return (m_iNormalized == 1);
}

//
//	FUNCTION public
//	Btree2::FileID::getNormalizingMethod -- 異表記正規化方法を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::StringData::NormalizingMethod::Value
//		異表記正規化方法
//
//	EXCEPTIONS
//
Utility::CharTrait::NormalizingMethod::Value
FileID::getNormalizingMethod() const
{
	if (m_eNormalizingMethod == Utility::CharTrait::NormalizingMethod::Unknown)
	{
		// デフォルトはBuiltIn
		m_eNormalizingMethod = Utility::CharTrait::NormalizingMethod::BuiltIn;
	
		int v;
		if (getInteger(_SYDNEY_FILE_PARAMETER_KEY(
						   KeyID::NormalizingMethod), v) == true)
			m_eNormalizingMethod
				= static_cast<Utility::CharTrait::NormalizingMethod::Value>(v);
	}

	return m_eNormalizingMethod;
}

//
//	FUNCTION public
//	Btree2::FileID::getPath -- パス名を得る
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
//	Btree2::FileID::setPath -- パス名を設定する
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
//	Btree2::FileID::isNotNull -- キーがnull値を許さないか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		キーがnull値を許さない場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isNotNull() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::NotNull));
}

//
//	FUNCTION public
//	Btree2::FileID::isTopNull -- 
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		True means that the top of keys is nullable.
//
//	EXCEPTIONS
//
bool
FileID::isTopNull() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::TopNull));
}

//
//	FUNCTION public
//	Btree2::FileID::isFixed -- すべてのフィールドが固定長かどうか
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
//	Btree2::FileID::isFixed -- 指定されたフィールドがFixedかどうか
//
//	NOTES
//
//	ARGUMENTS
//	int iFieldNumber_
//		フィールド番号
//
//	RETURN
//	bool
//		そのフィールドがFixedだったらtrue、それ以外ならfalse
//
//	EXCEPTIONS
//
bool
FileID::isFixed(int iFieldNumber_) const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
						  FileCommon::FileOption::FieldFixed::Key,
						  iFieldNumber_ + 1));	// OID分を足す
}

//
//	FUNCTION public
//	Btree2::FileID::isUnique -- キー値でユニークかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		キー値でユニークの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isUnique() const
{
	loadFieldInformation();
	if (getFieldCount() != getRealKeyFieldCount())
		return true;
	return false;
}

//
//	FUNCTION public
//	Btree2::FileID::getKeyType -- キーフィールドの型を得る
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
const ModVector<Data::Type::Value>&
FileID::getKeyType() const
{
	loadFieldInformation();
	return m_vecKeyType;
}

const ModVector<int>& 
FileID::getKeyPosition() const
{
	loadFieldInformation();
	return m_vecKeyPosition;
}

//
//	FUNCTION public
//	Btree2::FileID::getValueType -- バリューフィールドの型を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModVector<Data::Type::Value>&
//		バリューフィールドの型
//
//	EXCEPTIONS
//
const ModVector<Data::Type::Value>&
FileID::getValueType() const
{
	loadFieldInformation();
	return m_vecValueType;
}

const ModVector<int>&
FileID::getValuePosition() const
{
	loadFieldInformation();
	return m_vecValuePosition;
}
//
//	FUNCTION public
//	Btree2::FileID::getFieldSize -- フィールドの最大長を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModVector<ModSize>&
//		フィールドの最大長
//
//	EXCEPTIONS
//
const ModVector<ModSize>&
FileID::getFieldSize() const
{
	loadFieldInformation();
	return m_vecFieldSize;
}

//
//	FUNCTION public
//	Btree2::FileID::getTupleSize -- タプルの最大サイズを得る
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
//	Btree2::FileID::checkVersion -- 指定されたバージョン以降かどうかチェックする
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
//	FUNCTION public static
//	Btree2::FileID::checkVersion -- Btree2のバージョンかチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cLogicalFileID_
//		ファイルID
//
//	RETURN
//	bool
//		該当するバージョンまたは、バージョン情報がない場合はtrue、
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
	return (iVersion >= Version2) ? true : false;
}

//
//	FUNCTION public
//	Btree2::FileID::normalize -- 正規化する
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData& cOutput_
//		正規化後データ
//	const Common::DataArrayData& cInput_
//		正規化前データ
//	int startPosition_
//		処理を行うデータ開始位置(default 0)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::normalize(Common::DataArrayData& cOutput_,
				  const Common::DataArrayData& cInput_,
				  int startPosition_) const
{
	bool bNormalized = isNormalized();
	if (cOutput_.getCount() != (cInput_.getCount() - startPosition_))
	{
		cOutput_.clear();
		cOutput_.reserve(cInput_.getCount() - startPosition_);
	}
	int j = 0;
	for (int i = startPosition_; i < cInput_.getCount(); ++i, ++j)
	{
		Common::Data::Pointer pData = cInput_.getElement(i);
		if (bNormalized == true)
		{
			Common::Data::Pointer pCopyData = pData->copy();
			
			if (pData->getType() == Common::DataType::String)
			{
				//  文字列の場合

				Common::StringData* pStringData
					= _SYDNEY_DYNAMIC_CAST(Common::StringData*,
										   pCopyData.get());
				Utility::CharTrait::normalize(
					pStringData, getNormalizingMethod());
			}
			else if (pData->getType() == Common::DataType::Array)
			{
				// 配列の場合、その要素を正規化する

				Common::DataArrayData* pArrayData
					= _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*,
										   pCopyData.get());

				int c = pArrayData->getCount();
				for (int k = 0; k < c; ++k)
				{
					Common::Data::Pointer p
						= pArrayData->getElement(k);

					if (p->getType() == Common::DataType::String)
					{
						// 要素が文字列なので

						Common::StringData* pStringData
							= _SYDNEY_DYNAMIC_CAST(Common::StringData*,
												   p.get());
						Utility::CharTrait::normalize(
							pStringData, getNormalizingMethod());
					}
				}
			}
			
			pData = pCopyData;
		}
		cOutput_.setElement(j, pData);
	}
}

//
//	FUNCTION public
//	Btree2::FileID::makeData -- データ取得用のDataArrayDataを作成する
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData& cTuple_
//		作成するデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::makeData(Common::DataArrayData& cTuple_) const
{
	int param[2] = {0, 0};
	const ModVector<Data::Type::Value>& keys = getKeyType();
	const ModVector<int>& positions = getKeyPosition();
	const ModVector<Data::Type::Value>& values = getValueType();
	const ModVector<int>& positions1 = getValuePosition();
	cTuple_.clear();
	cTuple_.reserve(keys.getSize() + values.getSize());

	ModVector<Data::Type::Value>::ConstIterator i = keys.begin();
	for (ModSize i=0; i < keys.getSize(); i++)
	{
		if (keys[i] == Data::Type::Decimal)
		{
			param[0] = getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileCommon::FileOption::FieldLength::Key, positions[i]));
			param[1] = getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileCommon::FileOption::FieldFraction::Key, positions[i]));
		}
		cTuple_.pushBack(Data::makeData(keys[i], param[0], param[1]));
	}

	for (ModSize j=0; j < values.getSize(); j++)
	{
		if (values[j] == Data::Type::Decimal)
		{
			param[0] = getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileCommon::FileOption::FieldLength::Key, positions1[j]));
			param[1] = getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileCommon::FileOption::FieldFraction::Key, positions1[j]));
		}
		cTuple_.pushBack(Data::makeData(values[j], param[0], param[1]));
	}
}

//
//	FUNCTION public
//	Btree2::FileID::getMinFieldNumber -- min()の仮想フィールド番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		min()の仮想フィールド番号
//
//	EXCEPTIONS
//
ModSize
FileID::getMinFieldNumber() const
{
	return getFieldCount();
}

//
//	FUNCTION public
//	Btree2::FileID::getMaxFieldNumber -- max()の仮想フィールド番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		max()の仮想フィールド番号
//
//	EXCEPTIONS
//
ModSize
FileID::getMaxFieldNumber() const
{
	return getFieldCount() + 1;
}

//
//	FUNCTION public
//	Btree2::FileID::isLastRowID -- 最後の要素がROWIDかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		最後の要素がROWIDの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isLastRowID() const
{
	loadFieldInformation();
	return m_bLastRowID;
}

//
//	FUNCTION public
//	Btree2::FileID::isUseHeader -- エントリヘッダーを利用するか否か
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		利用する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isUseHeader() const
{
	if (isUnique() && checkVersion(Version5))
		return true;
	return false;
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
//	FUNCTION private
//	Btree2::FileID::loadFieldInformation -- フィールド情報をロードする
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
		int keynum = getInteger(_SYDNEY_FILE_PARAMETER_KEY(
			FileCommon::FileOption::KeyFieldNumber::Key));

//		fieldnum -= 2;	// min(),max()の仮想列分を引く
		
		if ((fieldnum - 1) > MAX_FIELD_COUNT)	// OID分引く
		{
			// フィールド数の最大数を超えている
			_SYDNEY_THROW0(Exception::NotSupported);
		}

		// 本当のキーフィールドを設定する
		m_uiRealKeyField = static_cast<ModSize>(keynum);

		if (isNotNull() == false)
		{
			// not nullじゃない場合は、すべてのフィールドをキーにする
			// -> そうしないとキーでユニークにならない
			keynum = fieldnum - 1;
		}

		// 配列を確保する
		m_vecKeyType.reserve(keynum);
		m_vecKeyPosition.reserve(keynum);
		if (fieldnum - keynum - 1)
		{
			m_vecValueType.reserve(fieldnum - keynum - 1);
			m_vecValuePosition.reserve(fieldnum - keynum - 1);
		}
		m_vecFieldSize.reserve(fieldnum - 1);

		ModSize totalLength = 0;
		int fixedFieldNum = 0;
		bool v3 = checkVersion(Version3);
		bool v4 = checkVersion(Version4);

		for (int i = 0; i != fieldnum; ++i)
		{
			if (i == 0) continue;	// ObjectIDはBtree2では使用しない

			bool isFixed = false;
			Data::Type::Value eType = getFieldType(i, v3, v4, isFixed);
			if (isFixed == true) fixedFieldNum++;
			ModSize length = getFieldSize(i, eType);

			totalLength += length;
			if (totalLength > _MaxTupleSize)
			{
				_SYDNEY_THROW0(Exception::NotSupported);
			}

			if (i == (fieldnum - 1) && eType == Data::Type::UnsignedInteger)
			{
				// 最後の要素はunsigned integerである
				m_bLastRowID = true;
			}

			if (i <= keynum)
			{
				if (eType == Data::Type::IntegerArray
					|| eType == Data::Type::UnsignedIntegerArray)
					_SYDNEY_THROW0(Exception::NotSupported);

				// キー
				m_vecKeyType.pushBack(eType);
				m_vecKeyPosition.pushBack(i);
			}
			else
			{
				// バリュー
				m_vecValueType.pushBack(eType);
				m_vecValuePosition.pushBack(i);
			}
			// フィールドサイズ
			m_vecFieldSize.pushBack(length);
		}

		// すべてのフィールドが固定長か
		m_bFixed = (isNotNull() && fixedFieldNum == (fieldnum - 1))
			? true : false;

		// 合計サイズ
		m_uiTupleSize = totalLength;
	}
}

//
//	FUNCTION private
//	Btree2::FileID::getFieldType -- フィールドの型を得る
//
//	NOTES
//
//	ARGUMENTS
//	int iPosition_
//		フィールド位置
//	bool isVersion3_
//		The file version is 3 or later
//	bool isVersion4_
//		The file version is 4 or later
//
//	RETURN
//	Btree2::Data::Type::Value
//		フィールドの型
//	bool& isFixed
//		固定長フィールドかどうか
//
//	EXCEPTIONS
//
Data::Type::Value
FileID::getFieldType(int iPosition_, bool isVersion3_, bool isVersion4_,
					 bool& isFixed_) const
{
	// IMPOSSIBLE to use FileID::Version1, Vesion2, ... in Data class.
	// Because both of FileID and Data classes includs each other
	// if Data class includes FileID class.
	return Data::getFieldType(*this, iPosition_, isVersion3_, isVersion4_,
							  isFixed_);
}

//
//	FUNCTION private
//	Btree2::FileID::getFieldSize -- フィールドのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	int iPosition_
//		フィールド位置
//	Btree2::Data::Type::Value eType_
//		データ型
//
//	RETURN
//	ModSize
//		フィールドのサイズ(byte)
//
//	EXCEPTIONS
//
ModSize
FileID::getFieldSize(int iPosition_, Data::Type::Value eType_) const
{
	return Data::getFieldSize(*this, iPosition_, eType_) * sizeof(ModUInt32);
}

//
//	FUNCTION private
//	Btree2::FileID::setNull -- Set the condition of null.
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
FileID::setNull()
{
	int fieldnum = getInteger(_SYDNEY_FILE_PARAMETER_KEY(
		FileCommon::FileOption::FieldNumber::Key));
	; _TRMEISTER_ASSERT(fieldnum > 1);
	
	bool bNotNull = true;
	bool bTopNull = false;
	for (int i = 0; i != fieldnum; ++i)
	{
		if (i == 0) continue;	// ObjectIDはBtree2では使用しない

		if (bNotNull == true)	// When i == 1, bNotNull is always true.
		{
			// ヒントを解釈する
			ModUnicodeString cstrHint;
			getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
						  FileCommon::FileOption::FieldHint::Key, i),
					  cstrHint);
			FileCommon::HintArray cHintArray(cstrHint);
			ModUnicodeString cstrValue;
			if (readHint(cstrHint, cHintArray, _NotNull, cstrValue)
				== false)
			{
				if (i == 1)
				{
					bTopNull = true;
				}
				bNotNull = false;
			}
		}
	}
	
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::NotNull), bNotNull);
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::TopNull), bTopNull);
}

//
//	FUNCTION private
//	Btree2::FileID::setNormalized
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
FileID::setNormalized(ModUnicodeString& cstrHint_,
					  FileCommon::HintArray& cHintArray_)
{
	ModUnicodeString cstrValue;
	bool bNormalized = false;

	if (readHint(cstrHint_, cHintArray_, _Normalized, cstrValue) == true)
	{
		if (cstrValue.compare(_TRMEISTER_U_STRING("true"), ModFalse) == 0
			|| cstrValue.getLength() == 0)
		{
			bNormalized = true;
		}
		else if (cstrValue.compare(_TRMEISTER_U_STRING("false"), ModFalse) == 0)
		{
			bNormalized = false;
		}
		else
		{
			// エラー
			SydErrorMessage << "Illegal Normalized. " << cstrValue << ModEndl;
			_SYDNEY_THROW0(Exception::BadArgument);
		}
	}

	// FileIDに設定する
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Normalized), bNormalized);
	
	if (bNormalized)
	{
		// 異表記正規化方法を設定する
		setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::NormalizingMethod),
				   _cNormalizingMethod.get());
	}
}

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
