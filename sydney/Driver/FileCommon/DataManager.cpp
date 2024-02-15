// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataManager.cpp -- 論理ファイル用データマネージャクラスの実現ファイル
// 
// Copyright (c) 2000, 2001, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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

namespace
{
const char	srcFile[] = __FILE__;
const char	moduleName[] = "FileCommon";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "FileCommon/DataManager.h"
#include "FileCommon/HintArray.h"

#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/ClassID.h"
#include "Common/CompressedBinaryData.h"
#include "Common/CompressedStringData.h"
#include "Common/DataInstance.h"
#include "Common/DecimalData.h"
#include "Common/ObjectIDData.h"
#include "Common/StringData.h"
#include "Common/LanguageData.h"
#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "PhysicalFile/Page.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "ModAutoPointer.h"
#include "ModUnicodeCharTrait.h"

_SYDNEY_USING
_SYDNEY_FILECOMMON_USING

//
// オブジェクト種
//

//
//	CONST
//	FileCommon::DataManager::m_ulDirectObjectType -- 代表オブジェクトタイプ
//
//	NOTES
//	代表オブジェクトタイプ。
//	単独で設定されるオブジェクト種ではない。
//	m_ulNormalObjectType には常に OR されている。
//	m_ulIndexObjectType と OR することがある。
//
// static
const ModUInt32
DataManager::m_ulDirectObjectType = 0x00010000;

//
//	CONST
//	FileCommon::DataManager::m_ulNormalObjectType -- ノーマルオブジェクトタイプ
//
//	NOTES
//	ノーマルオブジェクトタイプ。
//
// static
const ModUInt32
DataManager::m_ulNormalObjectType = 0x00000001;

//
//	CONST
//	FileCommon::DataManager::m_ulIndexObjectType -- インデックスオブジェクトタイプ
//
//	NOTES
//	インデックスオブジェクトタイプ。
//
// static
const ModUInt32
DataManager::m_ulIndexObjectType = 0x00000002;

//
//	CONST
//	FileCommon::DataManager::m_ulDivideObjectType -- 分割オブジェクトタイプ
//
//	NOTES
//	分割オブジェクトタイプ。
//
// static
const ModUInt32
DataManager::m_ulDivideObjectType = 0x00000004;

//
//	CONST
//	FileCommon::DataManager::m_ulArrayObjectType -- 配列オブジェクトタイプ
//
//	NOTES
//	配列オブジェクトタイプ。
//
// static
const ModUInt32
DataManager::m_ulArrayObjectType = 0x00000010;

//
//	CONST
//	FileCommon::DataManager::m_ulDivideArrayObjectType -- 分割配列オブジェクトタイプ
//
//	NOTES
//	分割配列オブジェクトタイプ。
//
// static
const ModUInt32
DataManager::m_ulDivideArrayObjectType = 0x00000020;

//
//	CONST
//	FileCommon::DataManager::m_ulObjectTypeArchiveSize -- オブジェクト種のアーカイブサイズ
//
//	NOTES
//	オブジェクト種のアーカイブサイズ。 [byte]
//
// static
const ModSize
DataManager::m_ulObjectTypeArchiveSize = sizeof(ModUInt32);

//
//	フィールド種
//

// 0x00000001はリザーブ

//
//	CONST
//	FileCommon::DataManager::m_ulNormalFieldType -- ノーマルフィールドオブジェクトタイプ
//
//	NOTES
//	ノーマルフィールドオブジェクトタイプ。
//
// static
const ModUInt32
DataManager::m_ulNormalFieldType	= 0x00000002;

//
//	CONST
//	FileCommon::DataManager::m_m_ulDivideFieldType -- 分割フィールドタイプ
//
//	NOTES
//
// static
const ModUInt32
DataManager::m_ulDivideFieldType		= 0x00000004;

//
//	CONST
//	FileCommon::DataManager::m_ulObjectIDFieldType -- オブジェクト ID フィールドタイプ
//
//	NOTES
//	オブジェクト ID フィールドタイプ。
//
// static
const ModUInt32
DataManager::m_ulObjectIDFieldType	= 0x00000008;

//
//	CONST
//	FileCommon::DataManager::m_ulArrayFieldType -- 配列フィールドオブジェクトタイプ
//
//	NOTES
//	配列フィールドオブジェクトタイプ。
//
// static
const ModUInt32
DataManager::m_ulArrayFieldType		= 0x00000010;

//
//	CONST
//	FileCommon::DataManager::m_ulFieldTypeArchiveSize -- フィールド種のアーカイブサイズ
//
//	NOTES
//	フィールド種のアーカイブサイズ。 [byte]
//
// static
const ModSize
DataManager::m_ulFieldTypeArchiveSize = sizeof(ModUInt32);

//
// その他
//

//
//	CONST
//	FileCommon::DataManager::m_ulFieldLengthArchiveSize -- 可変長フィールドのフィールド長のアーカイブサイズ
//
//	NOTES
//	可変長フィールドのフィールド長のアーカイブサイズ。 [byte]
//
// static
const ModSize
DataManager::m_ulFieldLengthArchiveSize = sizeof(ModSize);

//
//	FUNCTION public
//	FileCommon::DataManager::getDirectObjectType() -- 代表オブジェクトタイプを返す
//
//	NOTES
//	代表オブジェクトタイプを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		代表オブジェクトタイプ
//
//	EXCEPTIONS
//	なし
//
// static
#ifdef OBSOLETE

ModUInt32
DataManager::getDirectObjectType()
{
	return m_ulDirectObjectType;
}

#endif // end of #ifdef OBSOLETE

//
//	FUNCTION public
//	FileCommon::DataManager::getNormalObjectType() -- ノーマルオブジェクトタイプを返す
//
//	NOTES
//	ノーマルオブジェクトタイプを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		ノーマルオブジェクトタイプ
//
//	EXCEPTIONS
//	なし
//
// static
#ifdef OBSOLETE

ModUInt32
DataManager::getNormalObjectType()
{
	return m_ulNormalObjectType;
}

#endif // end of #ifdef OBSOLETE

//
//	FUNCTION public
//	FileCommon::DataManager::getIndexObjectType() -- インデックスオブジェクトタイプを返す
//
//	NOTES
//	インデックスオブジェクトタイプを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		インデックスオブジェクトタイプ
//
//	EXCEPTIONS
//	なし
//
// static
#ifdef OBSOLETE

ModUInt32
DataManager::getIndexObjectType()
{
	return m_ulIndexObjectType;
}

#endif // end of #ifdef OBSOLETE

//
//	FUNCTION public
//	FileCommon::DataManager::getDivideObjectType() -- 分割オブジェクトタイプを返す
//
//	NOTES
//	分割オブジェクトタイプを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		分割オブジェクトタイプ
//
//	EXCEPTIONS
//	なし
//
// static
#ifdef OBSOLETE

ModUInt32
DataManager::getDivideObjectType()
{
	return m_ulDivideObjectType;
}

#endif // end of #ifdef OBSOLETE

//
//	FUNCTION public
//	FileCommon::DataManager::getArrayObjectType() -- 配列オブジェクトタイプを返す
//
//	NOTES
//	配列オブジェクトタイプを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		配列オブジェクトタイプ
//
//	EXCEPTIONS
//	なし
//
// static
#ifdef OBSOLETE

ModUInt32
DataManager::getArrayObjectType()
{
	return m_ulArrayObjectType;
}

#endif // end of #ifdef OBSOLETE

//
//	FUNCTION public
//	FileCommon::DataManager::getDivideArrayObjectType() -- 分割配列オブジェクトタイプを返す
//
//	NOTES
//	分割配列オブジェクトタイプを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		分割配列オブジェクトタイプ
//
//	EXCEPTIONS
//	なし
//
// static
#ifdef OBSOLETE

ModUInt32
DataManager::getDivideArrayObjectType()
{
	return m_ulDivideArrayObjectType;
}

#endif // end of #ifdef OBSOLETE

//
//	FUNCTION public
//	FileCommon::DataManager::getObjectTypeArchiveSize -- オブジェクト種のアーカイブサイズを返す
//
//	NOTES
//	オブジェクト種のアーカイブサイズを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		オブジェクト種のアーカイブサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
// static
#ifdef OBSOLETE

ModSize
DataManager::getObjectTypeArchiveSize()
{
	return m_ulObjectTypeArchiveSize;
}

#endif // end of #ifdef OBSOLETE
//
//	FUNCTION public
//	FileCommon::DataManager::getNormalFieldType() -- ノーマルフィールドオブジェクトタイプを返す
//
//	NOTES
//	ノーマルフィールドオブジェクトタイプを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		ノーマルフィールドオブジェクトタイプ
//
//	EXCEPTIONS
//	なし
//
// static
#ifdef OBSOLETE

ModUInt32
DataManager::getNormalFieldType()
{
	return m_ulNormalFieldType;
}

#endif // end of #ifdef OBSOLETE

//
//	FUNCTION public
//	FileCommon::DataManager::getDivideFieldType() -- インデックスフィールドオブジェクトタイプを返す
//
//	NOTES
//	インデックスフィールドオブジェクトタイプを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		インデックスフィールドオブジェクトタイプ
//
//	EXCEPTIONS
//	なし
//
// static
#ifdef OBSOLETE

ModUInt32
DataManager::getDivideFieldType()
{
	return m_ulDivideFieldType;
}

#endif // end of #ifdef OBSOLETE

//
//	FUNCTION public
//	FileCommon::DataManager::getObjectIDFieldType() -- オブジェクト ID フィールドオブジェクトタイプを返す
//
//	NOTES
//	オブジェクト ID フィールドオブジェクトタイプを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		オブジェクト ID フィールドオブジェクトタイプ
//
//	EXCEPTIONS
//	なし
//
// static
#ifdef OBSOLETE

ModUInt32
DataManager::getObjectIDFieldType()
{
	return m_ulObjectIDFieldType;
}

#endif // end of #ifdef OBSOLETE

//
//	FUNCTION public
//	FileCommon::DataManager::getArrayFieldType() -- 配列フィールドオブジェクトタイプを返す
//
//	NOTES
//	配列フィールドオブジェクトタイプを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		配列フィールドオブジェクトタイプ
//
//	EXCEPTIONS
//	なし
//
// static
#ifdef OBSOLETE

ModUInt32
DataManager::getArrayFieldType()
{
	return m_ulArrayFieldType;
}

#endif // end of #ifdef OBSOLETE

//
//	FUNCTION public
//	FileCommon::DataManager::getFieldTypeArchiveSize -- フィールド種のアーカイブサイズを返す
//
//	NOTES
//	フィールド種のアーカイブサイズを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		フィールド種のアーカイブサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
// static
#ifdef OBSOLETE

ModSize
DataManager::getFieldTypeArchiveSize()
{
	return m_ulFieldTypeArchiveSize;
}

#endif // end of #ifdef OBSOLETE

//
//	FUNCTION public
//	FileCommon::DataManager::getFieldLengthArchiveSize -- 可変長フィールドのフィールド長のアーカイブサイズを返す
//
//	NOTES
//	可変長フィールドのフィールド長のアーカイブサイズを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		可変長フィールドのフィールド長のアーカイブサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
// static
#ifdef OBSOLETE

ModSize
DataManager::getFieldLengthArchiveSize()
{
	return m_ulFieldLengthArchiveSize;
}

#endif // end of #ifdef OBSOLETE

//
//	FUNCTION public
//	FileCommon::DataManager::getCommonDataArchiveSize -- コモンデータのアーカイブサイズを返す
//
//	NOTES
//	引数 cCommonData_ のもつ値のアーカイブサイズを返す。
//	※ FileCommon::DataManager::accessToCommonData で
//	   実際に物理ファイルへ書き込み／読み込みするサイズ
//
//	ARGUMENTS
//	const Common::Data&	cCommonData_
//		コモンデータオブジェクトへの参照
//
//	RETURN
//	ModSize
//		コモンでーたのアーカイブサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
// static
#ifdef OBSOLETE

ModSize
DataManager::getCommonDataArchiveSize(const Common::Data&	cCommonData_)
{
	// 可変長コモンデータ？
	if (isVariable(cCommonData_) == true)
	{
		return getVariableCommonDataArchiveSize(cCommonData_);
	}

	// 固定長コモンデータ
	return getFixedCommonDataArchiveSize(cCommonData_);
}

#endif // end of #ifdef OBSOLETE

//
//	FUNCTION public
//	FileCommon::DataManager::getVariableCommonDataArchiveSize -- 可変長コモンデータのアーカイブサイズを返す
//
//	NOTES
//	可変長コモンデータのアーカイブサイズを返す。
//	圧縮されているときは圧縮前のサイズを返すので注意すること。
//
//	ARGUMENTS
//	const Common::Data&	cVariableCommonData_
//		可変長コモンデータオブジェクトへの参照
//
//	RETURN
//	ModSize
//		可変長コモンデータのアーカイブサイズ [byte]
//
//	EXCEPTIONS
//	Common::NotSupportedException
//		※ Common::DecimalData は、カーネル側に実装されていないため、
//		   Common::NotSupportedException を投げる
//	Common::BadArgumentException
//		不正な引数
//
// static
#ifdef OBSOLETE

ModSize
DataManager::getVariableCommonDataArchiveSize(const Common::Data&	cVariableCommonData_)
{
	; _SYDNEY_ASSERT(!cVariableCommonData_.isFixedSize());

	ModSize	ulValueSize = 0;

	if (cVariableCommonData_.getFunction() == Common::Data::Function::Compressed) {
		const Common::CompressedData* pCompressedData =
			dynamic_cast<const Common::CompressedData*>(&cVariableCommonData_);
		if (!pCompressedData) {
			_SYDNEY_THROW0(Exception::BadArgument);
		}
		ulValueSize = pCompressedData->getValueSize();

	}
	else if (cVariableCommonData_.isAbleToDump())
	{
		ulValueSize = cVariableCommonData_.getDumpSize();
	}
	else
	{
		// ダンプできない型は扱えない
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// 可変長のデータは、先頭にデータ長を付加して物理ファイルに書き込む。
	return getFieldLengthArchiveSize() + ulValueSize;
}

#endif // end of #ifdef OBSOLETE

//
//	FUNCTION public
//	FileCommon::DataManager::isVariable -- 可変長コモンデータかどうかをチェックする
//
//	NOTES
//	引数 cCommonData_ が可変長コモンデータかどうかをチェックする。
//	現在、可変長コモンデータには、
//		・Common::StringData
//		・Common::DecimalData
//		・Common::BinaryData
//	がある。
//	※ このうち、 Common::DecimalData はカーネル側で実装されていないので
//	   ファイルドライバもサポートしない。
//
//	ARGUMENTS
//	const Common::Data&	cCommonData_
//		チェックするコモンデータオブジェクトへの参照
//
//	RETURN
//	bool
//		可変長コモンデータの場合には true を、
//		固定長コモンデータの場合には false を返す。
//
//	EXCEPTIONS
//	なし
//
// static
bool
DataManager::isVariable(Common::DataType::Type eDataType_)
{
	return !Common::Data::isFixedSize(eDataType_);
}

//
//	FUNCTION public
//	FileCommon::DataManager::isVariable -- 可変長コモンデータかどうかをチェックする
//
//	NOTES
//	引数 cCommonData_ が可変長コモンデータかどうかをチェックする。
//	現在、可変長コモンデータには、
//		・Common::StringData
//		・Common::DecimalData
//		・Common::BinaryData
//	がある。
//	※ このうち、 Common::DecimalData はカーネル側で実装されていないので
//	   ファイルドライバもサポートしない。
//
//	ARGUMENTS
//	const Common::Data&	cCommonData_
//		チェックするコモンデータオブジェクトへの参照
//
//	RETURN
//	bool
//		可変長コモンデータの場合には true を、
//		固定長コモンデータの場合には false を返す。
//
//	EXCEPTIONS
//	なし
//
// static
bool
DataManager::isVariable(const Common::Data&	cCommonData_)
{
	return isVariable(cCommonData_.getType());
}

//
//	FUNCTION public
//	FileCommon::DataManager::getCommonDataBuffer -- コモンデータのバッファとサイズを返す(可変長データのみ対応)
//
//	NOTES
//	コモンデータのバッファとサイズを返す。
//		但し、Common::BinaryData及びCommon::StringDataにのみ対応している。
//	データのコピーは行わないので上位で解放してはいけない。
//	Common::DecimalData はカーネル側で実装されていないので
//	   ファイルドライバもサポートしない。
//	ARGUMENTS
//	const Common::Data&	cCommonData_
//		コモンデータオブジェクトへの参照	[IN]
//	const void*&		pBuffer_
//		コモンデータのバッファ				[OUT]
//	ModSize&			ulSize_
//		コモンデータのサイズ(byte)			[OUT]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Common::NotSupportedException
//		固定長データ及びDecimalDataはサポートしていない。
//
// static
void
DataManager::getCommonDataBuffer(const Common::Data& cCommonData_,
								 const void*&		 pBuffer_,
								 ModSize&	ulSize_)
{
#ifndef SYD_COVERAGE
	//
	// 【注意】
	// このメソッドは旧B木からしか呼ばれないので、カバレージ測定の対象外
	//
	
	switch (cCommonData_.getFunction()) {
	case Common::Data::Function::Compressed:
	{
		const Common::CompressedData*	pCompressedData =
			dynamic_cast<const Common::CompressedData*>(&cCommonData_);
		; _SYDNEY_ASSERT(pCompressedData);
		if (!pCompressedData->isCompressed()) {
			// 圧縮されていないので通常のデータとして取得する
			break;
		}
		pBuffer_ = pCompressedData->getCompressedValue();
		ulSize_ = pCompressedData->getCompressedSize();
		return;
	}
	default:
		break;
	}

	switch (cCommonData_.getType()) {

	case Common::DataType::String:
	{
		const Common::StringData*	pStringData =
			_SYDNEY_DYNAMIC_CAST(const Common::StringData* ,&cCommonData_);
		; _SYDNEY_ASSERT(pStringData != 0);
		pBuffer_ = (const ModUnicodeChar*)pStringData->getValue();
		ulSize_ = pStringData->getValue().getLength() * sizeof(ModUnicodeChar);
		break;
	}

	case Common::DataType::Binary:
	{
		const Common::BinaryData*	pBinaryData =
			_SYDNEY_DYNAMIC_CAST(const Common::BinaryData* ,&cCommonData_);
		; _SYDNEY_ASSERT(pBinaryData != 0);
		pBuffer_ = pBinaryData->getValue();
		ulSize_ = pBinaryData->getSize();
		break;
	}

	case Common::DataType::Decimal:
		// decimal はサポートしていない
		throw Exception::NotSupported(moduleName, srcFile, __LINE__);

	default:
		// 固定長データはサポートしていない
		throw Exception::NotSupported(moduleName, srcFile, __LINE__);
	}
#endif
}

//
//	FUNCTION public
//	FileCommon::DataManager::setCommonData -- データ(バイト列) を Common::Data に変換する(可変長データのみ)。
//
//	NOTES
//	データ(バイト列) をコモンデータに変換する。
//  BinaryData及びStringDataのみに対応している。
//	コモンデータは渡されたデータをコピーする
//	※ このうち、 Common::DecimalData はカーネル側で実装されていないので
//	   ファイルドライバもサポートしない。
//
//	ARGUMENTS
//	const void*					pBuffer_
//		バッファ
//	ModSize						ulSize_
//		サイズ
//	Common::Data*				pCommonData_
//		変換したデータをセットする対象
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Common::NotSupportedException
//		固定長データ及びDecimalDataはサポートしていない。
//
// static
#ifdef OBSOLETE

void
DataManager::setCommonData(
	   const void*				pBuffer_,
	   ModSize					ulSize_,
	   Common::Data*			pCommonData_)
{
	if (pCommonData_->isAbleToDump()) {
		pCommonData_->setDumpedValue(syd_reinterpret_cast<const char*>(pBuffer_), ulSize_);

	} else {
		// dumpできないDataは対応していない
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

#endif // end of #ifdef OBSOLETE

//	FUNCTION public
//	FileCommon::DataManager::createCommonData --
//		指定されたデータ型の Common::Data を生成する
//
//	NOTES
//
//	ARGUMENTS
//		FileCommon::DataManager::DataType&	type
//			生成する Common::Data のデータ型
//
//	RETURN
//		生成された Common::Data を格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Common::Data*
DataManager::createCommonData(const DataType& type)
{
	switch (type._name) {
	case Common::DataType::String:
		return new Common::StringData(type._encodingForm);
	case Common::DataType::Decimal:
		return new Common::DecimalData(type._length, type._scale);
	}

	int classID;

	if (type._name > Common::DataType::Data
		&& type._name < Common::DataType::BitSet) {

		const int table[] =
		{
			// Common::DataType::Integer ->
			Common::ClassID::IntegerDataClass,
			// Common::DataType::UnsignedInteger ->
			Common::ClassID::UnsignedIntegerDataClass,
			// Common::DataType::Integer64 ->
			Common::ClassID::Integer64DataClass,
			// Common::DataType::UnsignedInteger64 ->
			Common::ClassID::UnsignedInteger64DataClass,
			// Common::DataType::String ->
			Common::ClassID::StringDataClass,
			// Common::DataType::Float ->
			Common::ClassID::FloatDataClass,
			// Common::DataType::Double ->
			Common::ClassID::DoubleDataClass,
			// Common::DataType::Decimal ->
			Common::ClassID::DecimalDataClass,
			// Common::DataType::Date ->
			Common::ClassID::DateDataClass,
			// Common::DataType::DateTime ->
			Common::ClassID::DateTimeDataClass,
			// Common::DataType::Binary
			Common::ClassID::BinaryDataClass
		};
		classID = table[type._name - Common::DataType::Data - 1];

	} else if (type._name == Common::DataType::Null)
		classID = Common::ClassID::NullDataClass;
	else if (type._name == Common::DataType::ObjectID)
		classID = Common::ClassID::ObjectIDDataClass;
	else if (type._name == Common::DataType::Language)
		classID = Common::ClassID::LanguageDataClass;
	else
		_SYDNEY_THROW0(Exception::BadArgument);

	return _SYDNEY_DYNAMIC_CAST(Common::Data*, Common::getClassInstance(classID));
}

// static
Common::Data*
DataManager::createCommonData(
	const DataType& type, Common::Data::Function::Value func, const void* arg)
{
#ifndef SYD_COVERAGE
	//
	//	【注意】
	//	この関数は旧B木からしか呼ばれないので、カバレージ測定の対象外
	//
	
	if (func == Common::Data::Function::None)
		return createCommonData(type);

	switch (type._name) {
	case Common::DataType::String:
		switch (func) {
		case Common::Data::Function::Compressed:
			if (arg)
				return new Common::CompressedStringData(
					*syd_reinterpret_cast<const ModSize*>(arg));
		}
		break;

	case Common::DataType::Binary:
		switch (func) {
		case Common::Data::Function::Compressed:
			if (arg)
				return new Common::CompressedBinaryData(
					*syd_reinterpret_cast<const ModSize*>(arg));
		}
		break;
	}

	_SYDNEY_THROW0(Exception::NotSupported);
#endif
	return 0;
}

//
//	FUNCTION public
//	FileCommon::DataManager::createCommonData -- 文字列表現からコモンデータオブジェクトを生成して返す
//
//	NOTES
//	文字列表現からコモンデータオブジェクトを生成して返す。
//	Common::DataType::Decimal, Common::DataType::Binary, Common::DataType::Language は未対応
//
//	ARGUMENTS
//	Common::DataType::Type	eDataType_
//		生成するコモンデータオブジェクトのデータタイプ
//	ModUnicodeString&		cstrValueString_
//		データの文字列表現
//
//	RETURN
//	Common::Data*
//		生成したコモンデータオブジェクトへのポインタ
//
//	EXCEPTIONS
//	Common::BadArgumentException
//		未知のデータ種別
//	Common::NotSupportedException
//		未対応のデータ種別
//
// static
Common::Data*
DataManager::createCommonData(
	Common::DataType::Type type, const ModUnicodeString& src)
{
	//
	//	【注意】
	//	この関数は検索条件をCommon::Dataに変換するときにしか利用されない
	//	OBSOLETEで囲んであるものは、検索条件に指定する可能性のないものである。
	//
	const ModUnicodeChar* pHead = static_cast<const ModUnicodeChar*>(src);
	const ModUnicodeChar* pTail = src.getTail();

	switch (type)
	{
	case Common::DataType::Integer:
	case Common::DataType::UnsignedInteger:
	case Common::DataType::Integer64:
#ifdef OBSOLETE
	case Common::DataType::UnsignedInteger64:
	case Common::DataType::ObjectID:
#endif
	case Common::DataType::Decimal:
		{
			ModAutoPointer<Common::Data> pData = Common::DataInstance::create(type);
			if (!Common::StringData::getInteger(*pData, pHead, pTail)) {
				// if conversion failed, set null
				pData->setNull(true);
			}
			return pData.release();
		}
#ifdef OBSOLETE
	case Common::DataType::Float:
#endif
	case Common::DataType::Double:
		{
			ModAutoPointer<Common::Data> pData = Common::DataInstance::create(type);
			if (!Common::StringData::getFloat(*pData, pHead, pTail)) {
				// if conversion failed, set null
				pData->setNull(true);
			}
			return pData.release();
		}
#ifdef OBSOLETE
	case Common::DataType::Date:
		return new Common::DateData(src);
#endif
	case Common::DataType::DateTime:
		return new Common::DateTimeData(src);
	case Common::DataType::String:
		return new Common::StringData(src);
	case Common::DataType::Language:
		return new Common::LanguageData(src);
	default:
		break;
	}

	// 未対応のデータ種別
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}

#ifdef OBSOLETE
//	FUNCTION public
//	FileCommon::DataManager::createCommonData --
//		指定されたデータ型の Common::Data を生成する
//
//	NOTES
//
//	ARGUMENTS
//		FileCommon::DataManager::DataType&	type
//			生成する Common::Data のデータ型
//		ModUnicodeString&		src
//			生成する Common::Data に設定する値
//
//	RETURN
//		生成された Common::Data を格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Common::Data*
DataManager::createCommonData(
	const DataType& type, const ModUnicodeString& src)
{
	switch (type._name) {
	case Common::DataType::String:
		return new Common::StringData(src, type._encodingForm);
	}

	return createCommonData(type._name, src);
}
#endif

//
//	FUNCTION public static
//	FileCommon::DataManager::toInt -- intで取り出す
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* p_
//		データ
//
//	RETURN
//	int
//
//	EXCEPTION
//
int
DataManager::toInt(const LogicalFile::TreeNodeInterface* p_)
{
	const Common::Data* pData = p_->getData();
	if (pData == 0)
		return ModUnicodeCharTrait::toInt(p_->getValue());
	
	if (pData->isNull() == true || pData->isScalar() == false)
		return 0;

	int value = 0;
	
	switch (pData->getType())
	{
	case Common::DataType::Integer:
		value = _SYDNEY_DYNAMIC_CAST(const Common::IntegerData*, pData)
			->getValue();
		break;
	case Common::DataType::UnsignedInteger:
		value = static_cast<int>(
			_SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData*, pData)
			->getValue());
		break;
	case Common::DataType::Integer64:
		value = static_cast<int>(
			_SYDNEY_DYNAMIC_CAST(const Common::Integer64Data*, pData)
			->getValue());
		break;
#ifdef OBSOLETE
	case Common::DataType::UnsignedInteger64:
		value = static_cast<int>(
			_SYDNEY_DYNAMIC_CAST(const Common::UnsignedInteger64Data*, pData)
			->getValue());
		break;
#endif
	case Common::DataType::String:
		value = ModUnicodeCharTrait::toInt(
			_SYDNEY_DYNAMIC_CAST(const Common::StringData*, pData)
			->getValue());
		break;
#ifdef OBSOLETE
	case Common::DataType::Float:
		value = static_cast<int>(
			_SYDNEY_DYNAMIC_CAST(const Common::FloatData*, pData)
			->getValue());
		break;
#endif
	case Common::DataType::Double:
		value = static_cast<int>(
			_SYDNEY_DYNAMIC_CAST(const Common::DoubleData*, pData)
			->getValue());
		break;
	default:
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	return value;
}

//
//	FUNCTION public static
//	FileCommon::DataManager::toDouble -- doubleで取り出す
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* p_
//		データ
//
//	RETURN
//	double
//
//	EXCEPTION
//
double
DataManager::toDouble(const LogicalFile::TreeNodeInterface* p_)
{
	const Common::Data* pData = p_->getData();
	if (pData == 0)
		return ModUnicodeCharTrait::toDouble(p_->getValue());
	
	if (pData->isNull() == true || pData->isScalar() == false)
		return 0;

	double value = 0.0;
	
	switch (pData->getType())
	{
#ifdef OBSOLETE
	case Common::DataType::Integer:
		value = static_cast<double>(
			_SYDNEY_DYNAMIC_CAST(const Common::IntegerData*, pData)
			->getValue());
		break;
	case Common::DataType::UnsignedInteger:
		value = static_cast<double>(
			_SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData*, pData)
			->getValue());
		break;
	case Common::DataType::Integer64:
		value = static_cast<double>(
			_SYDNEY_DYNAMIC_CAST(const Common::Integer64Data*, pData)
			->getValue());
		break;
	case Common::DataType::UnsignedInteger64:
		value = static_cast<double>(
			static_cast<ModInt64>(
			_SYDNEY_DYNAMIC_CAST(const Common::UnsignedInteger64Data*, pData)
			->getValue()));
		break;
	case Common::DataType::String:
		value = ModUnicodeCharTrait::toDouble(
			_SYDNEY_DYNAMIC_CAST(const Common::StringData*, pData)
			->getValue());
		break;
	case Common::DataType::Float:
		value = static_cast<double>(
			_SYDNEY_DYNAMIC_CAST(const Common::FloatData*, pData)
			->getValue());
		break;
#endif
	case Common::DataType::Double:
		value = _SYDNEY_DYNAMIC_CAST(const Common::DoubleData*, pData)
			->getValue();
		break;
	default:
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	return value;
}

//
//	FUNCTION
//	FileCommon::DataManager::parseHintString -- ヒント文字列を解析する。
//
//	NOTE
//		ヒント文字列を解析する。
//
//	ARGUMENTS
//		ModUnicodeString&	cstrHintString_
//			
//		HintArray&			cHintArray_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//	
#ifdef OBSOLETE
		
void
DataManager::parseHintString(
		ModUnicodeString&	cstrHintString_,
		HintArray&			cHintArray_)
{
	cHintArray_.initialize(cstrHintString_);
}

#endif // end of #ifdef OBSOLETE

//
//----------------------------------------------------------------------------
//
//	File I/O 関数
//
//----------------------------------------------------------------------------
//

//
//	FUNCTION public
//	FileCommon::DataManager::accessToCommonData -- コモンデータ用物理エリアアクセサ関数
//
//	NOTES
//	コモンデータ用物理エリアアクセサ関数。
//
//	ARGUMENTS
//	Trans::Transaction&		cTransaction_
//		トランザクション記述子
//	PhysicalFile::Page*		pPhysicalPage_
//		物理ページ記述子
//	PhysicalFile::AreaID	uiPhysicalAreaID_
//		物理エリア識別子
//	ModOffset				lAreaOffset_
//		物理エリア内のオフセット（物理エリア開始位置が 0 ） [byte]
//	Common::Data&			cCommonData_
//		コモンデータオブジェクトへの参照
//		引数 iAccessMode_ に AccessWrite が指定されている場合には、
//		このオブジェクトがもつ値を物理エリア内に書き込み、
//		AccessRead が指定されている場合には、
//		物理エリア内からこのオブジェクトの値を読み込む。
//	AccessMode				iAccessMode_
//		物理エリア内のデータへのアクセスモード
//
//	RETURN
//	ModSize
//		アクセスしたサイズ [byte]
//
//	EXCEPTIONS
//	YET!
//
// static
#ifdef OBSOLETE

ModSize
DataManager::accessToCommonData(
	const Trans::Transaction&	cTransaction_,
	PhysicalFile::Page*			pPhysicalPage_,
	const PhysicalFile::AreaID	uiPhysicalAreaID_,
	const ModOffset				lAreaOffset_,
	Common::Data&				cCommonData_,
	const AccessMode			iAccessMode_)
{
	ModSize	ulAccessSize = 0;

	if (!cCommonData_.isAbleToDump()) {
		// ダンプできないデータは対応しない
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// データサイズを入れるための変数
	ModSize uiDumpSize = 0;

	if (cCommonData_.isFixedSize() || iAccessMode_ == AccessWrite) {
		// 固定長データはデータサイズが決まっている
		// 可変長のときは書き込みのときに限りデータにサイズが入っている
		uiDumpSize = cCommonData_.getDumpSize();
	}

	if (!cCommonData_.isFixedSize()) {
		// 可変長データは最初にデータサイズを記録する
		ulAccessSize += accessToData(cTransaction_,
									 pPhysicalPage_,
									 uiPhysicalAreaID_,
									 lAreaOffset_,
									 &uiDumpSize,
									 sizeof(ModSize),
									 iAccessMode_);
	}

	// 読み書きに使うバッファを用意する
	char* pBuffer = 0;
	if (uiDumpSize > 0) {
		pBuffer = syd_reinterpret_cast<char*>(ModDefaultManager::allocate(uiDumpSize));
	}

	// 以降でエラーが起きたらfreeしなければいけないのでtry-catchで囲む
	try {
		// 書き込みのときはバッファにデータをダンプする
		if (iAccessMode_ == AccessWrite) {
			ModSize uiSize = cCommonData_.dumpValue(pBuffer);
			; _SYDNEY_ASSERT(uiSize == uiDumpSize);
		}

		ulAccessSize += accessToData(cTransaction_,
									 pPhysicalPage_,
									 uiPhysicalAreaID_,
									 lAreaOffset_ + ulAccessSize,
									 pBuffer,
									 uiDumpSize,
									 iAccessMode_);

		// 読み込みのときはバッファのデータを使って値をセットする
		if (iAccessMode_ == AccessRead) {
			ModSize uiSize = cCommonData_.setDumpedValue(pBuffer, uiDumpSize);
			; _SYDNEY_ASSERT(uiSize == uiDumpSize);
		}

		if (pBuffer) {
			ModDefaultManager::free(pBuffer, uiDumpSize);
		}

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		if (pBuffer) {
			ModDefaultManager::free(pBuffer, uiDumpSize);
		}
		_SYDNEY_RETHROW;
	}

	return ulAccessSize;
}

//
//	FUNCTION public
//	FileCommon::DataManager::accessToData -- 物理エリアアクセサの下請け関数
//
//	NOTES
//	物理エリアアクセサの下請け関数。
//	PhysicalFile::Page のメソッドを使用するので .cpp に定義をおく。
//
//	ARGUMENTS
//	Trans::Transaction&		cTransaction_
//		トランザクション記述子
//	PhysicalFile::Page*		pPhysicalPage_
//		物理ページ記述子
//	PhysicalFile::AreaID	uiPhysicalAreaID_
//		物理エリア識別子
//	ModOffset				lAreaOffset_
//		物理エリア内のオフセット（物理エリア開始位置が 0 ） [byte]
//	void*					pBuffer_
//		アクセス用バッファへのポインタ
//		引数 iAccessMode_ に AccessWrite が指定されている場合には、
//		このバッファのデータを物理エリア内に書き込み、
//		AccessRead が指定されている場合には、
//		物理エリア内からこのバッファにデータを読み込む。
//	ModSize					ulSize_
//		バッファサイズ [byte]
//	AccessMode				iAccessMode_
//		物理エリア内のデータへのアクセスモード
//
//	RETURN
//	ModSize
//		アクセスしたサイズ [byte]
//
//	EXCEPTIONS
//	YET!
//
// static
ModSize
DataManager::accessToData(
	const Trans::Transaction&	cTransaction_,
	PhysicalFile::Page*			pPhysicalPage_,
	const PhysicalFile::AreaID	uiPhysicalAreaID_,
	const ModOffset				lAreaOffset_,
	void*						pBuffer_,
	const ModSize				ulSize_,
	const AccessMode			iAccessMode_)
{
	if (iAccessMode_ == AccessRead) {
		pPhysicalPage_->readArea(cTransaction_, uiPhysicalAreaID_,
								 pBuffer_, lAreaOffset_, ulSize_);
	} else {
		pPhysicalPage_->writeArea(cTransaction_, uiPhysicalAreaID_,
								  pBuffer_, lAreaOffset_, ulSize_);
	}
	return ulSize_;
}

#endif // end of #ifdef OBSOLETE

//
//	FUNCTION public
//	FileCommon::DataManager::writeCommonData -- CommonDataからバッファに値を納める
//
//	NOTES
//	CommonDataからバッファに値を納める
//
//	ARGUMENTS
//	Common::Data&			cCommonData_
//		コモンデータオブジェクトへの参照。
//	char* pBuffer_
//		CommonDataの値を入れるバッファへのポインタ。
//
//	RETURN
//		データの直後をさすポインター
//
//	EXCEPTIONS
//
//static
char*
DataManager::writeCommonData(
	const Common::Data&			cCommonData_,
	char*						pBuffer_)
{
	ModSize iSize = cCommonData_.dumpValue(pBuffer_);
	return pBuffer_ + iSize;
}

//
//	FUNCTION public
//	FileCommon::DataManager::readCommonData -- バッファからCommonDataに値を納める
//
//	NOTES
//	バッファからCommonDataに値を納める。
//
//	ARGUMENTS
//	Common::Data&			cCommonData_
//		コモンデータオブジェクトへの参照。
//	char* pBuffer_
//		CommonDataに入れるべき値が入っているバッファへのポインタ。
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//
//static
const char*
DataManager::readCommonData(
	Common::Data&				cCommonData_,
	const char*					pBuffer_,
	ModSize						ulSize_)
{
	ModSize iSize = (!isVariable(cCommonData_)) ?
		cCommonData_.setDumpedValue(pBuffer_)
		: cCommonData_.setDumpedValue(pBuffer_, ulSize_);
	return pBuffer_ + iSize;
}

//
//	FUNCTION public
//	FileCommon::DataManager::writeStringData
//		-- StringDataの内容をダンプする
//
//	NOTES
//
//	ARGUMENTS
//	Common::StringData& cStringData_
//		ダンプするStringData
//	Common::StringData::EncodingForm::Value eEncoding_
//		エンコーディング
//	char* pBuffer_
//		書き出すバッファへのポインタ。
//
//	RETURN
//		データの直後をさすポインター
//
//	EXCEPTIONS
//
//static
char*
DataManager::writeStringData(
	const Common::Data& cStringData_,
	Common::StringData::EncodingForm::Value eEncoding_,
	char* pBuffer_)
{
	if (cStringData_.getType() != Common::DataType::String)
		_SYDNEY_THROW0(Exception::BadArgument);

	const Common::StringData& c
		= _SYDNEY_DYNAMIC_CAST(const Common::StringData&, cStringData_);
	ModSize iSize = c.dumpValue(pBuffer_, eEncoding_);
	return pBuffer_ + iSize;
}

//
//	FUNCTION public
//	FileCommon::DataManager::readStringData
//		-- ダンプした内容をStringDataに読み込む
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data&			cStringData_
//		書き出すStringData
//	Common::StringData::EncodingForm::Value eEncoding_
//		エンコーディング
//	char* pBuffer_
//		CommonDataに入れるべき値が入っているバッファへのポインタ。
//	ModSize uiSize_
//		バッファの長さ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//
//static
const char*
DataManager::readStringData(
	Common::Data& cStringData_,
	Common::StringData::EncodingForm::Value eEncoding_,
	const char* pBuffer_,
	ModSize uiSize_)
{
	if (cStringData_.getType() != Common::DataType::String)
		_SYDNEY_THROW0(Exception::BadArgument);

	Common::StringData& c
		= _SYDNEY_DYNAMIC_CAST(Common::StringData&, cStringData_);
	ModSize iSize = c.setDumpedValue(pBuffer_, uiSize_, eEncoding_);
	return pBuffer_ + iSize;
}

//
//	FUNCTION public
//	FileCommon::DataManager::writeFixedCommonData -- 固定長データのCommonDataからバッファに値を納める
//
//	NOTES
//	固定長データのCommonDataからバッファに値を納める
//	2001-01-29にgetFixedCommonDataIntoBufferから改名。
//
//	ARGUMENTS
//	Common::Data&			cCommonData_
//		コモンデータオブジェクトへの参照。
//	char* pBuffer_
//		CommonDataの値を入れるバッファへのポインタ。
//
//	RETURN
//		データの直後をさすポインター
//
//	EXCEPTIONS
//
//static
char*
DataManager::writeFixedCommonData(
	const Common::Data&			cCommonData_,
	char*						pBuffer_)
{
	ModSize iSize = cCommonData_.dumpValue(pBuffer_);
	return pBuffer_ + iSize;
}

//
//	FUNCTION public
//	FileCommon::DataManager::readFixedCommonData -- バッファから固定長データのCommonDataに値を納める
//
//	NOTES
//	バッファから固定長データのCommonDataに値を納める。
//	2001-01-29にsetFixedCommonDataFromBufferから改名。
//
//	ARGUMENTS
//	Common::Data&			cCommonData_
//		コモンデータオブジェクトへの参照。
//	char* pBuffer_
//		CommonDataに入れるべき値が入っているバッファへのポインタ。
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//
//static
const char*
DataManager::readFixedCommonData(
	Common::Data&				cCommonData_,
	const char*					pBuffer_)
{
	ModSize iSize = cCommonData_.setDumpedValue(pBuffer_);
	return pBuffer_ + iSize;
}

//- ここから、物理ページ管理機能付き物理ファイル用アクセス関数

//
//	FUNCTION public
//	FileCommon::DataManager::accessToCommonData -- コモンデータ用物理ページアクセス関数
//
//	NOTES
//	コモンデータ用物理ページアクセス関数(2001-04-17追加版)。
//
//	ARGUMENTS
//	PhysicalFile::Page* pPage_
//		物理ページへのポインタ。
//	ModSize				ulOffset_
//		ページ先頭からのオフセット。
//	Common::Data&	cCommonData_
//		コモンデータオブジェクトへの参照。
//		引数 iAccessMode_ に AccessWrite が指定されている場合には、
//		このオブジェクトがもつ値を物理エリア内に書き込み、
//		AccessRead が指定されている場合には、
//		物理エリア内からこのオブジェクトの値を読み込む。
//	AccessMode				iAccessMode_
//		物理エリア内のデータへのアクセスモード。
//
//	RETURN
//	ModSize
//		アクセスしたサイズ [byte]
//
//	EXCEPTIONS
//	YET!
//
//static
ModSize 
DataManager::accessToCommonData(
	const PhysicalFile::Page*	pPage_,
	ModOffset				ulOffset_,
	Common::Data&			rCommonData_,
	const AccessMode		iAccessMode_)
{
	if (!isVariable(rCommonData_)) {
		if (iAccessMode_ == AccessWrite)
		{
			char* pPageHead = (*pPage_).operator char*();
			DataManager::writeFixedCommonData
				(rCommonData_, pPageHead + ulOffset_);
		}
		else
		{
			const char* pPageHead = (*pPage_).operator const char*();
			DataManager::readFixedCommonData
				(rCommonData_, pPageHead + ulOffset_);
		}

	} else {
		throw Exception::NotSupported(moduleName, srcFile, __LINE__);
	}

	return rCommonData_.getDumpSize();
}

//
//	Copyright (c) 2000, 2001, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
