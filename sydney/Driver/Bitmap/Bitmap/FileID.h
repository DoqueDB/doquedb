// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.h -- LogicalFile::FileIDのラッパークラス
// 
// Copyright (c) 2005, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_FILEID_H
#define __SYDNEY_BITMAP_FILEID_H

#include "Bitmap/Module.h"
#include "Bitmap/Data.h"

#include "Common/DataArrayData.h"
#include "FileCommon/HintArray.h"
#include "LogicalFile/FileID.h"
#include "Lock/Name.h"
#include "Buffer/Page.h"
#include "Trans/Transaction.h"

#include "ModCharString.h"

_SYDNEY_BEGIN

namespace FileCommon
{
	class HintArray;
}

_SYDNEY_BITMAP_BEGIN

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
//	Bitmap::FileID -- ビットマップファイルドライバーのFileID
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
			// 圧縮するかどうか
			Compressed = LogicalFile::FileID::DriverNumber::Bitmap
		};
	};

	// 最大行サイズ(ModUInt32単位)
	enum { MAX_SIZE = 250 };

	// バージョン
	enum
	{
		Version1 = 0,
		Version2,

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

	// パス名を得る
	const Os::Path& getPath() const;
	void setPath(const Os::Path& cPath_);

	// すべてのフィールドがFixedかどうか
	bool isFixed() const;

	// キーフィールドの型を得る
	Data::Type::Value getKeyType() const;

	// 最大タプルサイズを得る(バイト)
	ModSize getTupleSize() const;
	// キーの最大サイズを得る(バイト)
	ModSize getKeySize() const;

	// バージョンをチェックする
	bool checkVersion(int iVersion_) const;
	
	// 配列か？
	bool isArray() const;

	// 圧縮か？
	bool isCompressed() const;

	// キーのデータ型を作成する
	Common::Data::Pointer createKeyData() const;
	// バリューのデータ型を作成する
	Common::Data::Pointer createValueData() const;

private:
	// ヒントを読む
	bool readHint(ModUnicodeString& cstrHint_,
				  FileCommon::HintArray& cHintArray_,
				  const ModUnicodeString& cstrKey_,
				  ModUnicodeString& cstrValue_);
	
	// Field情報をロードする
	void loadFieldInformation() const;

	// FieldTypeを得る
	Data::Type::Value getFieldType(int iPosition_,
								   bool isVersion2_,
								   bool& isFixed_,
								   bool& isArray_) const;
	// FieldSizeを得る
	ModSize getFieldSize(int iPosition_,
						 Data::Type::Value eType_,
						 bool isArray_, int& iPrecision_, int& iScale_) const;

	// 圧縮かどうかを設定する
	void setCompressed(ModUnicodeString& cstrHint_,
					   FileCommon::HintArray& cHintArray_);

	// 以下はFileIDの中にあるが、スピードを考え同じ値をメンバーとして持つもの

	// Fileのロック名
	mutable Lock::FileName m_cLockName;
	// パス名
	mutable Os::Path m_cPath;
	// キーフィールドの型
	mutable Data::Type::Value m_eKeyType;
	// フィールド長の合計
	mutable ModSize m_uiTupleSize;
	// すべてのフィールドがFixedかどうか
	mutable bool m_bFixed;
	// キーが配列かどうか
	mutable bool m_bArray;
	// 圧縮するかどうか
	mutable int m_iCompressed;
	// Decimalのprecisionとscale
	mutable int m_iPrecision;
	mutable int m_iScale;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_FILEID_H

//
//	Copyright (c) 2005, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
