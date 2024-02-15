// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataManager.h -- 論理ファイル用データマネージャクラスのヘッダーファイル
// 
// Copyright (c) 2000, 2001, 2004, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FILECOMMON_DATAMANAGER_H
#define __SYDNEY_FILECOMMON_DATAMANAGER_H

#include "Common/Object.h"
#include "Common/Assert.h"
#include "Common/DataType.h"
#include "Common/IntegerData.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/IntegerData.h"
#include "Common/Integer64Data.h"
#include "Common/UnsignedInteger64Data.h"
#include "Common/FloatData.h"
#include "Common/DoubleData.h"
#include "Common/DateData.h"
#include "Common/DateTimeData.h"
#include "Common/DataArrayData.h"
#include "Common/StringArrayData.h"
#include "Common/StringData.h"
#include "Common/BinaryData.h"
#include "Common/NullData.h"

#include "FileCommon/Module.h"
#include "FileCommon/ObjectID.h"
#include "FileCommon/VectorKey.h"
#include "FileCommon/HintArray.h"

#include "PhysicalFile/Types.h"

_SYDNEY_BEGIN

namespace PhysicalFile
{
class Page;
}

namespace Trans
{
class Transaction;
}

namespace LogicalFile
{
class TreeNodeInterface;
}

_SYDNEY_FILECOMMON_BEGIN

//	CLASS
//	FileCommon::DataManager -- 論理ファイル用データマネージャクラス
//
//	NOTES
//	論理ファイル用データマネージャクラス
//
class SYD_FILECOMMON_FUNCTION DataManager : public Common::Object
{
public:
	struct DataType
	{
		// デフォルトコンストラクタ
		DataType()
			: _name(Common::DataType::Undefined),
			  _length(0),
			  _scale(0),
			  _encodingForm(Common::StringData::EncodingForm::UCS2)
		{}

		// 可変長か調べる
		bool
		isVariable() const;

		// データ型がなにであるかを表す名前
		Common::DataType::Type					_name;
		// 長さ
		int										_length;
		// 桁数
		int										_scale;
		// 文字列データの場合、その符号化方式
		Common::StringData::EncodingForm::Value	_encodingForm;
	};

//
// 各データ型のアーカイブサイズを求める
//

	// ModInt32 型変数のアーカイブサイズを返す [byte]
#ifdef OBSOLETE
	static ModSize getModInt32ArchiveSize();
#endif

	// ModUInt32 型変数のアーカイブサイズを返す [byte]
	static ModSize getModUInt32ArchiveSize();

#ifdef OBSOLETE
	// ModInt64 型変数のアーカイブサイズを返す [byte]
	static ModSize getModInt64ArchiveSize();

	// ModUInt64 型変数のアーカイブサイズを返す [byte]
	static ModSize getModUInt64ArchiveSize();
#endif

	// VectorKey のアーカイブサイズを返す [byte]
	static ModSize getVectorKeyArchiveSize();

#ifdef OBSOLETE
	// コモンデータのアーカイブサイズを返す [byte]
	static ModSize getCommonDataArchiveSize(const Common::Data&	cCommonData_);
#endif

	// 固定長コモンデータのアーカイブサイズを返す [byte]
	static ModSize getFixedCommonDataArchiveSize(
		const Common::Data&	cFixedCommonData_);

	// 固定長コモンデータのアーカイブサイズを返す [byte]
	static ModSize getFixedCommonDataArchiveSize(
		Common::DataType::Type	eDataType_);

#ifdef OBSOLETE
	// 可変長コモンデータのアーカイブサイズを返す [byte]
	static ModSize getVariableCommonDataArchiveSize(
		const Common::Data&	cVariableCommonData_);
#endif

//
// 可変長データ
//

	// 可変長コモンデータかどうかをチェックする
	static bool isVariable(Common::DataType::Type eDataType_);

	// 可変長コモンデータかどうかをチェックする
	static bool isVariable(const Common::Data&	cCommonData_);

//
// オブジェクトタイプ
//
#ifdef OBSOLETE
	// 代表オブジェクトタイプを返す
	static ModUInt32 getDirectObjectType();

	// ノーマルオブジェクトタイプを返す
	static ModUInt32 getNormalObjectType();

	// インデックスオブジェクトタイプを返す
	static ModUInt32 getIndexObjectType();

	// 分割オブジェクトタイプを返す
	static ModUInt32 getDivideObjectType();

	// 配列オブジェクトタイプを返す
	static ModUInt32 getArrayObjectType();

	// 分割配列オブジェクトタイプを返す
	static ModUInt32 getDivideArrayObjectType();

	// オブジェクト種のアーカイブサイズを返す [byte]
	static ModSize getObjectTypeArchiveSize();
#endif // end of #ifdef OBSOLETE

//
// フィールドオブジェクトタイプ
//

	// ヌルフィールドオブジェクトタイプを返す
	static ModUInt32 getNullFieldType();

#ifdef OBSOLETE
	// ノーマルフィールドオブジェクトタイプを返す
	static ModUInt32 getNormalFieldType();

	// 分割フィールドオブジェクトタイプを返す
	static ModUInt32 getDivideFieldType();

	// オブジェクト ID フィールドタイプを返す
	static ModUInt32 getObjectIDFieldType();

	// 配列フィールドタイプを返す
	static ModUInt32 getArrayFieldType();

	// フィールド種のアーカイブサイズを返す [byte]
	static ModSize getFieldTypeArchiveSize();

	// 可変長フィールドのフィールド長のアーカイブサイズを返す [byte]
	static ModSize getFieldLengthArchiveSize();
#endif // end of #ifdef OBSOLETE


//
// CommonData 関連
//

	// データとサイズ [byte] を返す
	static void getCommonDataBuffer(
		const Common::Data&			cCommonData_,
		const void*&				pBuffer_,
		ModSize&					ulSize_);
	
#ifdef OBSOLETE
	// データ(バイト列) を Common::Data に変換する
	static void setCommonData(
		const void*					pBuffer_,
		ModSize						ulSize_,	
		Common::Data*				pCommonData_);
#endif

	// Common::Data を生成する
	static Common::Data*
	createCommonData(const DataType& type);
	static Common::Data*
	createCommonData(const DataType& type,
					 Common::Data::Function::Value func, const void* arg);
	static Common::Data*
	createCommonData(Common::DataType::Type type, const ModUnicodeString& src);
#ifdef OBSOLETE
	static Common::Data*
	createCommonData(const DataType& type, const ModUnicodeString& src);
#endif

	// ヒント文字列解析関数の下請関数
	static void getQuotedString(
		ModUnicodeString&		cstrHintString_,
		int&					i_,
		ModUnicodeString*		pQuote_);

	// intで取り出す
	static int toInt(const LogicalFile::TreeNodeInterface* pData_);
	// doubleで取り出す
	static double toDouble(const LogicalFile::TreeNodeInterface* pData_);

#ifdef OBSOLETE
	// ヒント文字列を解析する
	static void parseHintString(
		ModUnicodeString&			cstrHintString_,
		HintArray&					cHintArray_);
//		Common::StringArrayData*	pStringArray_);
#endif

//
// File I/O 関連
//

	//
	//	ENUM
	//	FileCommon::DataManager::AccessMode --
	//		データへのアクセスモードを表す列挙型
	//
	//	NOTES
	//	物理エリア内のデータへのアクセスモードを表す列挙型。
	//
	enum AccessMode
	{
		AccessRead = 0,
		AccessWrite
	};

#ifdef OBSOLETE
	// コモンデータ用物理エリアアクセサ関数
	static ModSize accessToCommonData(
		const Trans::Transaction&	cTransaction_,
		PhysicalFile::Page*			pPhysicalPage_,
		const PhysicalFile::AreaID	uiPhysicalAreaID_,
		const ModOffset				lAreaOffset_,
		Common::Data&				cCommonData_,
		const AccessMode			iAccessMode_);

	// accessToCommonDataの下請け
	static ModSize accessToData(
		const Trans::Transaction&	cTransaction_,
		PhysicalFile::Page*			pPhysicalPage_,
		const PhysicalFile::AreaID	uiPhysicalAreaID_,
		const ModOffset				lAreaOffset_,
		void*						pBuffer_,
		const ModSize				ulSize_,
		const AccessMode			iAccessMode_);
#endif // end of #ifdef OBSOLETE

	// CommonDataをページバッファに書き込む
	static char* writeCommonData(
		const Common::Data&			cCommonData_,
		char*						pBuffer_);
	
	// CommonDataをページバッファから読み込む
	static const char* readCommonData(
		Common::Data&				cCommonData_,
		const char*					pBuffer_,
		ModSize						ulSize_);

	// StringDataをページバッファに書き込む
	static char* writeStringData(
		const Common::Data&						cStringData_,
		Common::StringData::EncodingForm::Value eEncoding_,
		char*									pBuffer_);
	
	// StringDataをページバッファから読み込む
	static const char* readStringData(
		Common::Data&							cStringData_,
		Common::StringData::EncodingForm::Value eEncoding_,
		const char*								pBuffer_,
		ModSize									uiSize_);

	// 固定長データのCommonDataをページバッファに書き込む
	// 2001-01-29にgetFixedCommonDataIntoBufferから改名
	static char* writeFixedCommonData(
		const Common::Data&			cCommonData_,
		char*						pBuffer_);
	
	// 固定長データのCommonDataをページバッファから読み込む
	// 2001-01-29にsetFixedCommonDataFromBufferから改名
	static const char* readFixedCommonData(
		Common::Data&				cCommonData_,
		const char*					pBuffer_);

	//- ここから、物理ページ管理機能付き物理ファイル専用アクセサ関数

	// コモンデータ用物理ページアクセサ関数(2001-04-17作成)
	static ModSize accessToCommonData(
		const PhysicalFile::Page*	pPage_,
		ModOffset					ulOffset_,
		Common::Data&				rCommonData_,
		const AccessMode			iAccessMode_);

private:
	//
	// データメンバ
	//

	//
	// オブジェクト種
	//

	// 代表オブジェクトタイプ
	static const ModUInt32	m_ulDirectObjectType;

	// ノーマルオブジェクトタイプ
	static const ModUInt32	m_ulNormalObjectType;

	// インデックスオブジェクトタイプ
	static const ModUInt32	m_ulIndexObjectType;

	// 分割オブジェクトタイプ
	static const ModUInt32	m_ulDivideObjectType;

	// 配列オブジェクトタイプ
	static const ModUInt32	m_ulArrayObjectType;

	// 分割配列オブジェクトタイプ
	static const ModUInt32	m_ulDivideArrayObjectType;

	// オブジェクト種のアーカイブサイズ [byte]
	static const ModSize	m_ulObjectTypeArchiveSize;

	//
	// フィールド種
	//

	// ノーマルフィールドオブジェクトタイプ
	static const ModUInt32	m_ulNormalFieldType;

	// 分割フィールドオブジェクトタイプ
	static const ModUInt32	m_ulDivideFieldType;

	// ※ 分割フィールドオブジェクト ＝ リンクオブジェクトである

	// オブジェクト ID フィールドオブジェクトタイプ
	static const ModUInt32	m_ulObjectIDFieldType;

	// 配列フィールドオブジェクトタイプ
	static const ModUInt32	m_ulArrayFieldType;

	// フィールド種のアーカイブサイズ [byte]
	static const ModSize	m_ulFieldTypeArchiveSize;

	//
	// その他
	//

	// 可変長フィールドのフィールド長のアーカイブサイズ [byte]
	static const ModSize	m_ulFieldLengthArchiveSize;

}; // end of class DataManager


//
//	FUNCTION public static
//	FileCommon::DataManager::getModInt32ArchiveSize -- ModInt32 型変数のアーカイブサイズを返す
//	FileCommon::DataManager::getModUInt32ArchiveSize -- ModUInt32 型変数のアーカイブサイズを返す
//	FileCommon::DataManager::getModInt64ArchiveSize -- ModInt64 型変数のアーカイブサイズを返す
//	FileCommon::DataManager::getModUInt64ArchiveSize -- ModUInt64 型変数のアーカイブサイズを返す
//	FileCommon::DataManager::getObjectIDArchiveSize -- ObjectIDのアーカイブサイズを返す
//
//	NOTES
//	各データ型変数のアーカイブサイズを返す。
//	static const メンバ変数を返す関数の定義は .cpp にある。
//
//	ARGUMTENS
//	なし
//
//	RETURN
//	ModSize
//		各データ型変数のアーカイブサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
#ifdef OBSOLETE

inline
ModSize
DataManager::getModInt32ArchiveSize()
{
	return sizeof(ModInt32);
}

#endif // end of #ifdef OBSOLETE

inline
ModSize
DataManager::getModUInt32ArchiveSize()
{
	return sizeof(ModUInt32);
}

#ifdef OBSOLETE

inline
ModSize
DataManager::getModInt64ArchiveSize()
{
	return sizeof(ModInt64);
}

inline
ModSize
DataManager::getModUInt64ArchiveSize()
{
	return sizeof(ModUInt64);
}

#endif // end of #ifdef OBSOLETE

inline
ModSize
DataManager::getVectorKeyArchiveSize(){
	return FileCommon::VectorKey::ArchiveSize;
}

//
//	FUNCTION public
//	FileCommon::DataManager::getFixedCommonDataArchiveSize -- 固定長コモンデータのアーカイブサイズを返す
//
//	NOTES
//	固定長コモンデータのアーカイブサイズを返す。
//
//	ARGUMENTS
//	const Common::Data&	cFixedCommonData_
//		固定長コモンデータオブジェクトへの参照
//
//	RETURN
//	ModSize
//		固定長コモンデータのアーカイブサイズ [byte]
//
//	EXCEPTIONS
//	Common::BadArgumentException
//		不正な引数
//
// static
inline ModSize
DataManager::getFixedCommonDataArchiveSize(const Common::Data&	cFixedCommonData_)
{
	; _SYDNEY_ASSERT(cFixedCommonData_.isFixedSize());
	return cFixedCommonData_.getDumpSize();
}

//
//	FUNCTION public
//	FileCommon::DataManager::getFixedCommonDataArchiveSize -- 固定長コモンデータのアーカイブサイズを返す
//
//	NOTES
//	固定長コモンデータのアーカイブサイズを返す。
//
//	ARGUMENTS
//	Common::DataType::Type eDataType_
//		データの型
//
//	RETURN
//	ModSize
//		固定長コモンデータのアーカイブサイズ [byte]
//
//	EXCEPTIONS
//	Common::BadArgumentException
//		不正な引数
//
// static
inline ModSize
DataManager::getFixedCommonDataArchiveSize(Common::DataType::Type eDataType_)
{
	return Common::Data::getDumpSize(eDataType_);
}

//	FUNCTION public
//	FileCommon::DataManager::isVariable -- 可変長かを調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			可変長である
//		false
//			可変長でない
//
//	EXCEPTIONS
//		なし

inline
bool
DataManager::DataType::isVariable() const
{
	return DataManager::isVariable(_name);
}

_SYDNEY_FILECOMMON_END
_SYDNEY_END

#endif // __SYDNEY_FILECOMMON_DATAMANAGER_H

//
//	Copyright (c) 2000, 2001, 2004, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
