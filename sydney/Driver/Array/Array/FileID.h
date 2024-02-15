// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.h -- LogicalFile::FileIDのラッパークラス
// 
// Copyright (c) 2006, 2007, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ARRAY_FILEID_H
#define __SYDNEY_ARRAY_FILEID_H

#include "Array/Module.h"
#include "Array/Data.h"				//Data::Type::Value

#include "Common/StringData.h"
#include "Lock/Name.h"				//Lock::FileName
#include "LogicalFile/FileID.h"
#include "Os/Path.h"
#include "Utility/CharTrait.h"

class ModUnicodeString;

_SYDNEY_BEGIN

namespace Common
{
	class DataArrayData;
}
namespace FileCommon
{
	class HintArray;
}

_SYDNEY_ARRAY_BEGIN

//
//	CLASS
//	Array::FileID --
//
//	NOTES
//
class FileID : public LogicalFile::FileID
{
public:
	struct KeyID
	{
		enum Value
		{
			// 異表記正規化
			Normalized = LogicalFile::FileID::DriverNumber::Array,
			// キーがnot nullか
			NotNull,
			// 異表記正規化方法
			NormalizingMethod
		};
	};

	// 最大行サイズ(ModUInt32単位) [unit size]
	enum { MAX_SIZE = 1250 };

	// The count of fields
	static const int FieldCount;
	
	// The position of each field in the point of upper module.
	struct FieldPosition
	{
		enum Value
		{
			Array = 0,
			RowID
		};
	};

	// コンストラクタ
	FileID(const LogicalFile::FileID& cLogicalFileID_);
	// デストラクタ
	virtual ~FileID();

	// ファイルIDの内容を作成する
	void create();

	// ページサイズ
	int getPageSize() const;

	// LockNameを得る
	const Lock::FileName& getLockName() const;

	// 読み取り専用か
	bool isReadOnly() const;
	// 一時か
	bool isTemporary() const;
	// マウントされているか
	bool isMounted() const;
	void setMounted(bool bFlag_);

	// 異表記正規化ありか
	bool isNormalized() const;
	// 異表記正規化方法を得る
	Utility::CharTrait::NormalizingMethod::Value
	getNormalizingMethod() const;

	// パス名を得る
	const Os::Path& getPath() const;
	void setPath(const Os::Path& cPath_);

	// すべてのフィールドがFixedかどうか
	bool isFixed() const;

	// Get the type of key field
	Data::Type::Value getKeyType() const;
	// Get the size of key field [byte]
	ModSize getKeySize() const;

	// Get the maximum tuple size [byte]
	ModSize getTupleSize() const;

	// 正規化する
	void normalize(Common::DataArrayData& cOutput_,
				   const Common::DataArrayData& cInput_,
				   int startPotision = 0) const;
	void normalizeOneData(Common::Data::Pointer& pData_) const;

private:
	// Field情報をロードする
	void loadFieldInformation() const;

	// FieldTypeを得る
	Data::Type::Value getFieldType(int iPosition_,
								   bool& isFixed_, bool& isArray_) const;
	// FieldSizeを得る
	ModSize getFieldSize(int iPosition_,
						 Data::Type::Value eType_, bool isArray_) const;

	// ヒントを解釈し、格納されている文字列を得る
	bool readHint(ModUnicodeString& cstrHint_,
				  FileCommon::HintArray& cHintArray_,
				  const ModUnicodeString& cstrKey_,
				  ModUnicodeString& cstrValue_);

	// normalizedヒントを設定する
	void setNormalized(ModUnicodeString& cstrHint_,
					   FileCommon::HintArray& cHintArray_);
	
	// 以下はFileIDの中にあるが、スピードを考え同じ値をメンバーとして持つもの

	// The type of Value field
	mutable Data::Type::Value m_eKeyType;
	// The size of Value field [byte]
	mutable ModSize m_uiKeySize;
	// The total of the all fields [byte]
	// When variable field is included, the total is maximum length.
	mutable ModSize m_uiTupleSize;
	// Is key field fixed?
	mutable bool m_bFixed;
	// Fileのロック名
	mutable Lock::FileName m_cLockName;
	// パス名
	mutable Os::Path m_cPath;
	// 異表記正規化方法
	mutable Utility::CharTrait::NormalizingMethod::Value m_eNormalizingMethod;
	// 異表記正規化するかどうか
	mutable int m_iNormalized;
};

_SYDNEY_ARRAY_END
_SYDNEY_END

#endif //__SYDNEY_ARRAY_FILEID_H

//
//	Copyright (c) 2006, 2007, 2009, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
