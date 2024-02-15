// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MetaData.h -- メタデータクラスのヘッダーファイル
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD_METADATA_H
#define __SYDNEY_RECORD_METADATA_H

#include "Record/Tools.h"
#include "Record/Module.h"

#include "Common/Common.h"
#include "Common/DataType.h"
#include "Common/Object.h"
#include "FileCommon/DataManager.h"
#include "FileCommon/IDNumber.h"
#include "LogicalFile/FileID.h"
#include "Os/Path.h"
#include "PhysicalFile/Types.h"
#include "PhysicalFile/File.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_RECORD_BEGIN

class TargetFields;

//
//	CLASS
//	Record::MetaData -- メタデータのクラス
//
//	NOTES
//	メタデータに相当するクラス。
//	レコードファイルのメタデータを効率良くアクセスできるようにするクラス。
//	(現在、メタデータは 構造体＋HashMap であり、マップからキー数値で
//	必要な値を探している)
//
//	ユーザーが指定していない情報を取得しようとするとデフォルト値が返る。
//
class MetaData : public Common::Object
{
public:
	// コンストラクタ
	MetaData(const LogicalFile::FileID&	cFileID_);

	// デストラクタ
	~MetaData();

	// FileIDを作成する
	void create();

	//
	// アクセッサ
	//

	// レコードファイル格納先ディレクトリパスを取得
	const Os::Path& getDirectoryPath() const;

	// レコードファイル格納先ディレクトリパスを設定
	void setDirectoryPath(const LogicalFile::FileID&	cFileID_) const;
	void setDirectoryPath(const ModUnicodeString&	cstrPath_);

	// 固定長部分のオブジェクトサイズを得る
	Os::Memory::Size getObjectSize() const;

	// 固定長部分の1ページあたりのオブジェクト数を得る
	int getObjectPerPage() const;

	// 物理ページサイズを取得
	Os::Memory::Size getDirectPageSize() const;
	Os::Memory::Size getVariablePageSize() const;

#ifdef OBSOLETE
	// 1ページあたりの領域使用率を取得
	int getUseRatePerPage() const;
#endif

	// フィールド数を取得
	Tools::FieldNum getFieldNumber() const;
	Tools::FieldNum getVariableFieldNumber() const;

	// 任意のフィールドのフィールドデータ型を取得
	const Tools::DataType& getDataType(const int	FieldID_) const;

	// 任意のフィールドの配列要素のデータ型を取得
	const Tools::DataType& getElementType(const int	FieldID_) const;

#ifdef OBSOLETE
	// 任意のフィールドのフィールド最大長を取得
	int getFieldLengthMax(const int	FieldID_) const;
#endif

	// 任意のフィールドの小数点以下の桁数
	int getFieldFraction(const int	FieldID_) const;

	// 可変長か
	bool isVariable(int FieldID) const;
	// 配列か
	bool isArray(int FieldID) const;
	// 可変長要素の配列か
	bool isVariableArray(int FieldID) const;
	// 圧縮して格納するかどうか
	bool getCompress(const int FieldID_) const;

	// ひとつのフィールド(フィールド種＋フィールド値)の領域サイズを返す
	Os::Memory::Size getFieldSize(const int	FieldID_) const;

	// 可変長フィールドのサイズを表すのに必要なサイズ
	Os::Memory::Size getHeaderSize(const int FieldID_) const;

	// 同値性の検査
	// (メタデータだけ比較する。従って論理ファイルIDは比較しない)
	bool equals(const MetaData& MetaData_) const;

	bool isTemporary() const;
	bool isReadOnly() const;
	bool hasVariable() const;
	bool hasCompress() const;

	const TargetFields* getDirectFields() const;
	const TargetFields* getVariableFields() const;

	const LogicalFile::FileID& getFileID() const { return m_FileID; }

	Lock::FileName getLockName() const { return m_IDNumber.getLockName(); }

	// バージョン番号をチェックする
	int getVersion() const;

private:

	friend class File;

	// 初期化
	void initialize() const;
	// フィールド情報の配列を初期化
	void initializeFieldInfoArray(const LogicalFile::FileID& cFileID_) const;

	// ページサイズを設定する
	void setDirectPageSize() const;
	void calculateDirectPageSize() const;
	void setVariablePageSize() const;
	// オブジェクトサイズを設定する
	void setObjectSize() const;

	void setTemporary(const LogicalFile::FileID&	cFileID_) const;
	void setReadOnly(const LogicalFile::FileID&	cFileID_)const ;
	void divideFields() const;

	// フィールド情報
	// (検索時にはかなり頻繁にアクセスされるのでデータメンバに直接アクセス
	//  できるように変更した。これで関数呼び出しのオーバーヘッドが消滅した)
	class FieldInfo : public Common::Object
	{
	public:
		FieldInfo() {;}
		~FieldInfo() {;}

		// フィールド種類
		// (デフォルト operator= を利用したいので const にしない)
//		Common::DataType::Type	m_DataType;
		Tools::DataType			m_DataType;
		// 配列フィールドの要素のデータ型
//		Common::DataType::Type	m_ElementType;
		Tools::DataType			m_ElementType;
		// number of elements
		Tools::ElementNum		m_ElementNum;

		// フィールドの書き出しに必要なサイズ(B 単位)
		Os::Memory::Size		m_FieldSize;
		// フィールドのヘッダーに必要なサイズ(B 単位)
		Os::Memory::Size		m_HeaderSize;
//		// フィールドの最大長(B 単位)
//		Tools::FieldLength		m_FieldLengthMax;
//		// 配列要素の最大長
//		Tools::FieldLength		m_ElementLengthMax;

		// 可変長か
		bool					m_Variable;
		// 圧縮して格納するか
		bool					m_Compress;
	};

	// フィールド情報を格納するベクタ。ベクタの要素数はフィールド数と同じ。
	// ベクタの添字０の要素がオブジェクトIDのフィールドに相当する。
	mutable FieldInfo*						m_FieldInfoArray;

	mutable Tools::FieldNum					m_FieldNum;
	mutable Tools::FieldNum					m_VariableFieldNum;
	mutable bool							m_HasCompress;

	// レコードオブジェクトのアーカイブサイズ
	//	OID (Field[0]) はファイルには記録しないので含まれない
	//	可変長データの実値は別のオブジェクトとして記録されるので含まれない
	//		ポインタ(pid+aid) の領域のみ
	mutable Os::Memory::Size				m_RecordArchiveSize;

	// ファイルを格納するディレクトリーのトップパス名
	mutable Os::Path						m_cstrPathName;

	// 1ページあたりの領域使用率
	mutable int								m_UseRatePerPage;

	// 指定された(またはデフォルトの)ページサイズ
	mutable Os::Memory::Size				m_DirectPageSize;
	mutable Os::Memory::Size				m_VariablePageSize;

	// 固定長部分のオブジェクトサイズ
	mutable Os::Memory::Size				m_ObjectSize;

	// 固定長部分の1ページあたりのオブジェクト数
	mutable int								m_ObjectPerPage;

	// 一時データベースか
	mutable bool							m_IsTemporary;

	// 読み込み専用データベースか
	mutable bool							m_ReadOnly;

	//
	mutable TargetFields*					m_pDirectFields;
	mutable TargetFields*					m_pVariableFields;

	// ファイルIDオブジェクト
	// ※ Record::File::createがレコードファイル利用者へ返すもの。
	mutable LogicalFile::FileID				m_FileID;

	// ファイルIDから一意な整数値を得る。（注意：生成引数には、FileIDのローカルのコピーへの参照を与える）
	const FileCommon::IDNumber				m_IDNumber;
};

// 固定長部分のオブジェクトサイズを得る
inline
Os::Memory::Size
MetaData::getObjectSize() const
{
	return this->m_ObjectSize;
}

// 固定長部分の1ページあたりのオブジェクト数を得る
inline
int
MetaData::getObjectPerPage() const
{
	return this->m_ObjectPerPage;
}

// 物理ページサイズを取得
inline
Os::Memory::Size
MetaData::getDirectPageSize() const
{
	return this->m_DirectPageSize;
}
inline
Os::Memory::Size
MetaData::getVariablePageSize() const
{
	return this->m_VariablePageSize;
}

#ifdef OBSOLETE
// 1ページあたりの領域使用率を取得
inline
int
MetaData::getUseRatePerPage() const
{
	return this->m_UseRatePerPage;
}
#endif

// フィールド数(0番フィールドに存在するオブジェクトIDを含む)を取得
inline
Tools::FieldNum
MetaData::getFieldNumber() const
{
	return this->m_FieldNum;
}
// 可変長フィールドの個数を取得
inline
Tools::FieldNum
MetaData::getVariableFieldNumber() const
{
	return this->m_VariableFieldNum;
}

// 任意のフィールドのフィールドデータ型を取得
inline
const Tools::DataType&
MetaData::getDataType(const int	FieldID_) const
{
	return this->m_FieldInfoArray[FieldID_].m_DataType;
}

// 任意のフィールドの配列要素のデータ型を取得
inline
const Tools::DataType&
MetaData::getElementType(const int	FieldID_) const
{
	return m_FieldInfoArray[FieldID_].m_ElementType;
}

#ifdef OBSOLETE
// 任意のフィールドのフィールド最大長を取得
inline
int
MetaData::getFieldLengthMax(const int	FieldID_) const
{
	return this->m_FieldInfoArray[FieldID_].m_DataType._length;
}
#endif

// 任意のフィールドの小数点以下の桁数
inline
int
MetaData::getFieldFraction(const int	FieldID_) const
{
	return this->m_FieldInfoArray[FieldID_].m_DataType._scale;
}

// 圧縮して格納するかどうか
inline
bool
MetaData::getCompress(const int	FieldID_) const
{
	return this->m_FieldInfoArray[FieldID_].m_Compress;
}

//	FUNCTION public
//	Record::MetaData::getDirectoryPath -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

inline
const Os::Path&
MetaData::getDirectoryPath() const
{
	return m_cstrPathName;
}

//	FUNCTION public
//	Sydney::Record::MetaData::isVariable -- あるフィールドが可変長か
//
//	NOTES
//
//	ARGUMENTS
//		int			FieldID
//			調べるフィールドが先頭からなん番目のものか
//
//	RETURN
//		true
//			可変長である
//		false
//			固定長である
//
//	EXCEPTIONS
//		なし

inline
bool
MetaData::isVariable(int FieldID) const
{
	return m_FieldInfoArray[FieldID].m_Variable;
}

//	FUNCTION public
//	Sydney::Record::MetaData::isArray -- あるフィールドが配列か
//
//	NOTES
//
//	ARGUMENTS
//		int			FieldID
//			調べるフィールドが先頭からなん番目のものか
//
//	RETURN
//		true
//			可変長である
//		false
//			固定長である
//
//	EXCEPTIONS
//		なし

inline
bool
MetaData::isArray(int iFieldID_) const
{
	return m_FieldInfoArray[iFieldID_].m_DataType._name == Common::DataType::Array;
}

//	FUNCTION public
//	Sydney::Record::MetaData::isVariableArray -- あるフィールドが可変長要素の配列か
//
//	NOTES
//
//	ARGUMENTS
//		int			FieldID
//			調べるフィールドが先頭からなん番目のものか
//
//	RETURN
//		true
//			可変長要素の配列である
//		false
//			固定長要素の配列であるまたは配列でない
//
//	EXCEPTIONS
//		なし

inline
bool
MetaData::isVariableArray(int iFieldID_) const
{
	return isArray(iFieldID_)
		&& FileCommon::DataManager::isVariable(getElementType(iFieldID_)._name);
}
inline
bool
MetaData::isTemporary() const
{
	return m_IsTemporary;
}
inline
bool
MetaData::isReadOnly() const
{
	return m_ReadOnly;
}
inline
bool
MetaData::hasVariable() const
{
	return m_VariableFieldNum > 0;
}
inline
bool
MetaData::hasCompress() const
{
	return m_HasCompress;
}
inline
const TargetFields*
MetaData::
getDirectFields() const
{
	return m_pDirectFields;
}
inline
const TargetFields*
MetaData::
getVariableFields() const
{
	return m_pVariableFields;
}

_SYDNEY_RECORD_END
_SYDNEY_END

#endif // __SYDNEY_RECORD_METADATA_H

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
