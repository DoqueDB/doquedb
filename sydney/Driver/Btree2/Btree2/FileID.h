// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.h -- LogicalFile::FileIDのラッパークラス
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2014, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE2_FILEID_H
#define __SYDNEY_BTREE2_FILEID_H

#include "Btree2/Module.h"
#include "Btree2/Data.h"
#include "Common/DataArrayData.h"
#include "Common/StringData.h"
#include "LogicalFile/FileID.h"
#include "Lock/Name.h"
#include "Buffer/Page.h"
#include "Trans/Transaction.h"
#include "Utility/CharTrait.h"

#include "ModCharString.h"

_SYDNEY_BEGIN

namespace FileCommon
{
	class HintArray;
}

_SYDNEY_BTREE2_BEGIN

//
//	TYPEDEF
//	Inverted::LogicalFileID --
//
//	NOTES
//	Inverted::FileIDが直接LogicalFile::FileIDを継承できないので、
//	このtypedefを間に挟む。VC6のバグ。
//
typedef LogicalFile::FileID LogicalFileID;

//
//	CLASS
//	Inverted::FileID -- 転置ファイルドライバーのFileID
//
//	NOTES
//	
//
class FileID : public LogicalFileID
{
public:
	struct KeyID
	{
		enum Value
		{
			// 以下は旧B木のもの(未使用)
			KeyObjectPerNode = LogicalFile::FileID::DriverNumber::Btree,
			FieldSortOrder,

			// 異表記正規化
			Normalized,
			// キーがnot nullか
			NotNull,
			// The top of the keys is nullable.
			TopNull,
			// 異表記正規化方法
			NormalizingMethod
		};
	};

	// 最大行サイズ(ModUInt32単位)
	enum { MAX_SIZE = 1250 };
	// 最大フィールド数
	enum { MAX_FIELD_COUNT = 8 };

	// バージョン
	enum
	{
		Version1 = 0,
		Version2,
		Version3,
		Version4,
		Version5,

		// バージョン数
		ValueNum,
		// 現在のバージョン
		CurrentVersion = ValueNum - 1
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

	// すべてのフィールドがnull値を許さないか
	bool isNotNull() const;
	// Is the top of the keys nullable?
	bool isTopNull() const;
	// すべてのフィールドがFixedかどうか
	bool isFixed() const;
	// 指定されたフィールドがFixedかどうか
	bool isFixed(int iFieldNumber_) const;
	// キー値でユニークかどうか
	bool isUnique() const;

	// キーフィールドの型を得る
	const ModVector<Data::Type::Value>& getKeyType() const;
	const ModVector<int>& getKeyPosition() const; 

	// バリューフィールドの型を得る
	const ModVector<Data::Type::Value>& getValueType() const;
	const ModVector<int>& getValuePosition() const;

	// フィールドの最大長を得る
	const ModVector<ModSize>& getFieldSize() const;

	// フィールド数を得る
	ModSize getFieldCount() const { return getFieldSize().getSize(); }
	// 本当のキーフィールド数を得る
	ModSize getRealKeyFieldCount() const { return m_uiRealKeyField; }

	// 最大タプルサイズを得る
	ModSize getTupleSize() const;

	// バージョンをチェックする
	bool checkVersion(int iVersion_) const;
	// バージョンをチェックする
	static bool checkVersion(const LogicalFile::FileID& cLogicalFileID_);

	// 正規化する
	void normalize(Common::DataArrayData& cOutput_,
				   const Common::DataArrayData& cInput_,
				   int startPotision = 0) const;

	// データ取得用のDataArrayDataを作成する
	void makeData(Common::DataArrayData& cTuple_) const;

	// min()の仮想フィールド番号を得る
	ModSize getMinFieldNumber() const;
	// max()の仮想フィールド番号を得る
	ModSize getMaxFieldNumber() const;

	// 最後の要素がROWIDかどうか
	bool isLastRowID() const;

	// エントリヘッダーを利用するか否か
	bool isUseHeader() const;

private:
	// Field情報をロードする
	void loadFieldInformation() const;

	// FieldTypeを得る
	Data::Type::Value getFieldType(int iPosition_,
								   bool isVersion3_,
								   bool isVersion4_,
								   bool& isFixed_) const;

	// FieldSizeを得る
	ModSize getFieldSize(int iPosition_, Data::Type::Value eType_) const;

	// Set the condition of null.
	void setNull();

	// ヒントを解釈し、格納されている文字列を得る
	bool readHint(ModUnicodeString& cstrHint_,
				  FileCommon::HintArray& cHintArray_,
				  const ModUnicodeString& cstrKey_,
				  ModUnicodeString& cstrValue_);

	// normalizedヒントを設定する
	void setNormalized(ModUnicodeString& cstrHint_,
					   FileCommon::HintArray& cHintArray_);

	// 以下はFileIDの中にあるが、スピードを考え同じ値をメンバーとして持つもの

	// Fileのロック名
	mutable Lock::FileName m_cLockName;
	// パス名
	mutable Os::Path m_cPath;
	// キーフィールドの型
	mutable ModVector<Data::Type::Value> m_vecKeyType;
	mutable ModVector<int> m_vecKeyPosition;

	// バリューフィールドの型
	mutable ModVector<Data::Type::Value> m_vecValueType;
	mutable ModVector<int> m_vecValuePosition;

	// フィールドの最大長
	mutable ModVector<ModSize> m_vecFieldSize;
	// フィールド長の合計
	mutable ModSize m_uiTupleSize;
	// 本当のキーフィールド数(インテグリティーチェックに使用する)
	mutable ModSize m_uiRealKeyField;
	// すべてのフィールドがFixedかどうか
	mutable ModSize m_bFixed;
	// 異表記正規化方法
	mutable Utility::CharTrait::NormalizingMethod::Value m_eNormalizingMethod;
	// 異表記正規化するかどうか
	mutable int m_iNormalized;
	// 最後の要素がROWIDかどうか
	mutable bool m_bLastRowID;
};

_SYDNEY_BTREE2_END
_SYDNEY_END

#endif //__SYDNEY_BTREE2_FILEID_H

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
