// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.cpp --
// 
// Copyright (c) 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Array";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Array/FileID.h"
#include "Array/Parameter.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "FileCommon/FileOption.h"
#include "FileCommon/HintArray.h"
#include "FileCommon/IDNumber.h"
#include "Version/File.h"

#include "ModTypes.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_ARRAY_USING

namespace
{
	//
	//	基本ページサイズ(kbyte)
	//
	ParameterInteger _cBasicPageSize("Array_BasicPageSize", 4);

	//
	//	最大ページサイズ(kbyte)
	//
	ParameterInteger _cMaxPageSize("Array_MaxPageSize", 64);

	//
	//	異表記正規化方法
	//
	ParameterInteger _cNormalizingMethod("Execution_LikeNormalizedString", 1);

	//
	//	最大タプルサイズ(byte)
	//
	ModUInt32 _MaxTupleSize = FileID::MAX_SIZE * sizeof(ModUInt32);

	//
	//	Assumed tupple count
	//
	//	This value is estimated by the following way.
	//	* Assumed data volume is 1 million.
	//	* Assumed page space efficiency is 70%.
	//	* Assumed step count of tree structure is 3.
	//	* Assumed tupple count is X.
	//	(X * (page space efficiency))^(step count) < (data volume)
	//	=> Maximum X is about 143.
	//	=> X is rouded up to 200 for an allowance.
	//
	ModUInt32 _AssumedTupleCount = 200;

	//
	//	normalized のヒント
	//
	ModUnicodeString _Normalized("normalized");
}

//
//	CONST
//	Array::FileID::FieldCount --
//
const int FileID::FieldCount = FileID::FieldPosition::RowID + 1;

//
//	FUNCTION public
//	Array::FileID::FileID -- コンストラクタ
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
	: LogicalFile::FileID(cLogicalFileID_),
	  m_uiKeySize(0), m_uiTupleSize(0), m_bFixed(false),
	  m_eNormalizingMethod(Utility::CharTrait::NormalizingMethod::Unknown),
	  m_iNormalized(-1)
{
}

//
//	FUNCTION public
//	Array::FileID::~FileID -- デストラクタ
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
//	Array::FileID::create -- ファイルIDの内容を作成する
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

	// Normalized
	setNormalized(cstrHint, cHintArray);

	//
	//	システムパラメータから得る
	//

	// PageSize
	ModSize pageSize = _cBasicPageSize.get();	// 基本ページサイズ
	if (pageSize < (FileCommon::FileOption::PageSize::getDefault() >> 10))
		pageSize = (FileCommon::FileOption::PageSize::getDefault() >> 10);
	ModSize size = getTupleSize() * _AssumedTupleCount / 1024;
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
}

//
//	FUNCTION public
//	Array::FileID::getPageSize -- ページサイズを得る
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
//	Array::FileID::getLockName -- ロック名を得る
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
//	Array::FileID::isReadOnly -- 読み取り専用か
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
//	Array::FileID::isTemporary -- 一時か
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
//	Array::FileID::isMounted -- マウントされているか
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
//	Array::FileID::setMounted -- マウントされているかを設定する
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
//	Array::FileID::isNormalized -- 異表記正規化ありか
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
//	Array::FileID::getNormalizingMethod -- 異表記正規化方法を得る
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
//	Array::FileID::getPath -- パス名を得る
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
//	Array::FileID::setPath -- パス名を設定する
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
//	Array::FileID::isFixed -- すべてのフィールドが固定長かどうか
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
//	Array::FileID::getKeyType --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
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
//	Array::FileID::getKeySize --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModSize
//		[byte]
//		The size is multiples of 'sizeof(ModUInt32)'.
//		See loadFieldInformation() and getFieldSize().
//
//	EXCEPTIONS
//
ModSize
FileID::getKeySize() const
{
	loadFieldInformation();
	return m_uiKeySize;
}

//
//	FUNCTION public
//	Array::FileID::getTupleSize -- タプルの最大サイズを得る
//
//	NOTES
//	It is equal to the size of Leaf's entry in the DataTree.
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		タプルの最大サイズ [byte]
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
//	Array::FileID::normalize -- 正規化する
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
	; _SYDNEY_ASSERT(startPosition_ >= 0);
	
	// Check the count of the cOutput_'s fields.
	// The cInput_'s fields before the startPosition_ is ignored.
	if (cOutput_.getCount() != (cInput_.getCount() - startPosition_))
	{
		cOutput_.clear();
		cOutput_.reserve(cInput_.getCount() - startPosition_);
	}

	// Set the data.
	bool bNormalized = isNormalized();
	int j = 0;
	for (int i = startPosition_; i < cInput_.getCount(); ++i, ++j)
	{
		Common::Data::Pointer pData = cInput_.getElement(i);
		if (bNormalized == true)
		{
			normalizeOneData(pData);
		}
		cOutput_.setElement(j, pData);
	}
}

//
//	FUNCTION public
//	Array::FileID::normalizeOneData --
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data::Pointer pData_
//
//	RETURN
//
//	EXCEPTIONS
//
void
FileID::normalizeOneData(Common::Data::Pointer& pData_) const
{
	if (pData_->getType() == Common::DataType::String)
	{
		// [NOTE] pData_は、たとえばnormalize()で渡されたconstなcInput_
		//  の一部なので、コピーしたうえで正規化データを上書きする必要がある。
		// [YET] 正規化で文字列が変更されたことを確認してから
		//  copyしても遅くないのでは？
		Common::Data::Pointer pCopyData = pData_->copy();
		Common::StringData* pStringData
			= _SYDNEY_DYNAMIC_CAST(Common::StringData*, pCopyData.get());
		Utility::CharTrait::normalize(
			pStringData, getNormalizingMethod());
		pData_ = pCopyData;
	}
}

//
//	FUNCTION private
//	Array::FileID::readHint -- ヒントを読む
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
//	Array::FileID::loadFieldInformation -- フィールド情報をロードする
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
		// The expected data is Array and RowID.
		
		// Check the number of fields.
		int fieldnum = getInteger(
			_SYDNEY_FILE_PARAMETER_KEY(
				FileCommon::FileOption::FieldNumber::Key));
		if (fieldnum != FieldCount)
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		
		ModSize total = 0;
		for (int i = 0; i < FieldCount; ++i)
		{
			// Get each field information.
			bool isFixed = false;
			bool isArray = false;
			Data::Type::Value eType = getFieldType(i, isFixed, isArray);
			ModSize length = getFieldSize(i, eType, isArray);
			
			// Check the total.
			total += length;
			if (total > _MaxTupleSize)
			{
				_SYDNEY_THROW0(Exception::NotSupported);
			}
			
			if (i == FieldPosition::Array)
			{
				if (isArray == false)
				{
					_SYDNEY_THROW0(Exception::NotSupported);
				}
				
				// Set the Value field.
				m_eKeyType = eType;
				m_uiKeySize = length;
				// Set Fixed.
				m_bFixed = isFixed;
			}
			else // i == FieldPosition::RowID
			{
				if (isArray == true || eType != Data::Type::UnsignedInteger)
				{
					_SYDNEY_THROW0(Exception::NotSupported);
				}
			}
		}

		// Set Tuple size.
		m_uiTupleSize = total;
	}
}

//
//	FUNCTION private
//	Array::FileID::getFieldType -- フィールドの型を得る
//
//	NOTES
//
//	ARGUMENTS
//	int iPosition_
//		フィールド位置
//
//	RETURN
//	Array::Data::Type::Value
//		フィールドの型
//	bool& isFixed
//		固定長フィールドかどうか
//
//	EXCEPTIONS
//
Data::Type::Value
FileID::getFieldType(int iPosition_, bool& isFixed_, bool& isArray_) const
{
	return Data::getFieldType(*this, iPosition_, isFixed_, isArray_);
}

//
//	FUNCTION private
//	Array::FileID::getFieldSize -- フィールドのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	int iPosition_
//		フィールド位置
//	Array::Data::Type::Value eType_
//		データ型
//
//	RETURN
//	ModSize
//		フィールドのサイズ(byte)
//		When the field is variable, the size is maximum in the range.
//
//	EXCEPTIONS
//
ModSize
FileID::getFieldSize(int iPosition_, Data::Type::Value eType_,
					 bool isArray_) const
{
	return Data::getFieldSize(*this, iPosition_, eType_, isArray_)
		* sizeof(ModUInt32);
}

//
//	FUNCTION private
//	Array::FileID::setNormalized
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
//	Copyright (c) 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
