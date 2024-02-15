// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Tools.cpp -- 
// 
// Copyright (c) 2000, 2001, 2003, 2004, 2005, 2006, 2007, 2017, 2023 Ricoh Company, Ltd.
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
const char srcFile[] = __FILE__;
const char moduleName[] = "Record";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Record/Tools.h"
#include "Record/PhysicalPosition.h"

#include "Exception/BadArgument.h"

#include "Common/Assert.h"
#include "Common/CompressedData.h"
#include "Common/Data.h"
#include "Common/Message.h"
#include "Common/NullData.h"
#include "Common/ObjectIDData.h"
#include "Common/DateData.h"
#include "Common/DateTimeData.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"
#include "FileCommon/DataManager.h"
#include "PhysicalFile/File.h"
#include "PhysicalFile/Page.h"
#include "Record/Module.h"
#include "Record/Parameter.h"

_SYDNEY_USING
_SYDNEY_RECORD_USING

namespace {
	namespace _ConstValue {

		// 定数ではなくstatic関数から値を得るものはここでセットする
		const Os::Memory::Size _DateArchiveSize = Common::DateData::getArchiveSize();
		const Os::Memory::Size _DateTimeArchiveSize = Common::DateTimeData::getArchiveSize();
		const Os::Memory::Size _ObjectIDArchiveSize = Common::ObjectIDData::getArchiveSize();
		const Tools::ObjectID _ObjectIDMaxValue = Common::ObjectIDData::getMaxValue();
		const PhysicalFile::AreaID _AreaIDMaxValue = Common::ObjectIDData::getMaxLatterValue();

	} // namespace _ConstValue

	// MinimumObjectPerPageのための設定
	ParameterIntegerInRange _cMinimumObjectPerPage(
		"Record_MinimumObjectPerPage",
		4,	// default
		4,	// min
		_ConstValue::_AreaIDMaxValue, // max
		false);
}

//
// オブジェクトタイプ
//

// static
const Os::Memory::Size
Tools::m_ObjectTypeArchiveSize = sizeof(ObjectType);

// static
const Tools::ObjectType
Tools::m_NormalObjectType = 0x00;

// static
const Tools::ObjectType
Tools::m_VariableObjectType = 0x01;

// static
const Tools::ObjectType
Tools::m_IndexObjectType = 0x02;

// static
const Tools::ObjectType
Tools::m_DivideObjectType = 0x04;

// static
const Tools::ObjectType
Tools::m_LinkedObjectType = 0x08;

// static
const Tools::ObjectType
Tools::m_ArrayObjectType = 0x10;

// static
const Tools::ObjectType
Tools::m_DivideArrayObjectType = 0x20;

//
// オブジェクトタイプの0x40はリザーブ。
//

// static
const Tools::ObjectType
Tools::m_DirectObjectType = 0x80;

// static
const Tools::ObjectType
Tools::m_UndefinedObjectType = 0xFF;

//
// 
//

// static
const Os::Memory::Size
Tools::m_FieldNumArchiveSize = sizeof(Tools::FieldNum);

// static
const Os::Memory::Size
Tools::m_ElementNumArchiveSize = sizeof(Tools::ElementNum);

// static
const Os::Memory::Size
Tools::m_ObjectIDNumArchiveSize = sizeof(Tools::ObjectIDNum);

// static
const Os::Memory::Size
Tools::m_ObjectIDArchiveSize = _ConstValue::_ObjectIDArchiveSize;

// static
const Tools::ObjectID
Tools::m_UndefinedObjectID = _ConstValue::_ObjectIDMaxValue;

// static
const Os::Memory::Size
Tools::m_ObjectNumArchiveSize = sizeof(Tools::ObjectNum);

// static
const Os::Memory::Size
Tools::m_FieldLengthArchiveSize = sizeof(Tools::FieldLength);

// static
const Os::Memory::Size
Tools::m_NullMarkArchiveSize = sizeof(Tools::NullMark);

// static
const Tools::NullMark
Tools::m_ElementIsNull = 0x01;

// static
const Tools::NullMark
Tools::m_ElementIsNotNull = 0x00;

// static
const Os::Memory::Size
Tools::m_DateDataArchiveSize = _ConstValue::_DateArchiveSize;

// static
const Os::Memory::Size
Tools::m_TimeDataArchiveSize = _ConstValue::_DateTimeArchiveSize;

//
//	FUNCTION public
//	Record::Tools::readFixedField -- 固定長フィールドに対応するデータを読む
//
//	NOTES
//
//	ARGUMENTS
//	const char* pAreaPointer_
//		読み込むアドレス
//	const Tools::DataType&		cDataType_
//		フィールドに対応するデータのデータ種 (メタデータなどから取得したもの)
// 	Common::Data& cCommonData_
//		返り値を入れる
//
//	RETURN
//	読み込んだデータの次を指すポインター
//
//	EXCEPTIONS

// static
const char*
Tools::readFixedField(const char* pPointer_,
					  const DataType& cDataType_,
					  Common::Data& cCommonData_)
{
	; _SYDNEY_ASSERT(pPointer_);
	; _SYDNEY_ASSERT(cCommonData_.getType() == cDataType_._name);

	const char*	pPointer = pPointer_;

	if (Common::Data::isFixedSize(cDataType_._name))
	{
		// 型が固定長ならそのまま読めばよい

		pPointer = FileCommon::DataManager::readFixedCommonData(
			cCommonData_, pPointer);

	}
	else
	{
		// 型が可変長ならサイズの後にデータ

		FieldLength iSize;
		Os::Memory::copy(&iSize, pPointer, Tools::m_FieldLengthArchiveSize);
		pPointer += Tools::m_FieldLengthArchiveSize;

		// 文字列のときは専用のメソッドを使う
		if (cDataType_._name == Common::DataType::String)
			(void) FileCommon::DataManager::readStringData(
				cCommonData_, cDataType_._encodingForm, pPointer, iSize);
		else
			(void) FileCommon::DataManager::readCommonData(
				cCommonData_, pPointer, iSize);
		pPointer += cDataType_._length;
	}
	
	return pPointer;
}

//
//	FUNCTION public
//	Record::Tools::writeFixedField -- フィールドに対応するデータを書く
//
//	NOTES
//	フィールド値を書き込む(データは固定長でなければいけない)。
//	フィールドに書かれている情報は「固定長データ」である。
//
//	   ┌─── 固定長データのフィールド ───┐
//	   │┌─────────────────┐│
//	   ││固定長データ                      ││
//	   │└─────────────────┘│
//	   └───────────────────┘
//
//	ARGUMENTS
//	char* pAreaPointer_
//		データを書き出す位置を指すポインター
//	Common::Data* 						pCommonData_
//		フィールドに書き込む値
//
//	RETURN
//	書き込んだデータの次を指すポインター
//
//	EXCEPTIONS
//

// static
char*
Tools::writeFixedField(char* pAreaPointer_,
					   const DataType& cDataType_,
					   const Common::Data& cCommonData_)
{
	; _SYDNEY_ASSERT(pAreaPointer_);

	char*	pPointer = pAreaPointer_;

	if (cCommonData_.isNull())
	{
		// ポインターだけ進める
		pPointer += cDataType_._length;
	}
	else if (cDataType_.isVariable()) {

		// 型が可変長ならサイズの後にデータ

		; _SYDNEY_ASSERT(!(cCommonData_.getFunction() & Common::Data::Function::Compressed));

		FieldLength iSize;
		if (cDataType_._name == Common::DataType::String)
			iSize = _SYDNEY_DYNAMIC_CAST(const Common::StringData&,
										 cCommonData_).getDumpSize(
											 cDataType_._encodingForm);
		else
			iSize = cCommonData_.getDumpSize();
		if (iSize > cDataType_._length) {
			SydErrorMessage << "Can't write over-length data. max=" << cDataType_._length
							<< " length=" << iSize << ModEndl;
			_SYDNEY_THROW0(Exception::BadArgument);
		}
		Os::Memory::copy(pPointer, &iSize, Tools::m_FieldLengthArchiveSize);
		pPointer += Tools::m_FieldLengthArchiveSize;

		if (cDataType_._name == Common::DataType::String)
			(void)FileCommon::DataManager::writeStringData(
				cCommonData_, cDataType_._encodingForm, pPointer);
		else
			(void)FileCommon::DataManager::writeCommonData(
				cCommonData_, pPointer);
		pPointer += cDataType_._length;
	} else

		// 固定長データを書き込む

		pPointer = FileCommon::DataManager::writeFixedCommonData(
			cCommonData_, pPointer);

	return pPointer;
}

//	FUNCTION public
//	Record::Tools::getVariableSize -- 可変長フィールドの圧縮前後のサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& pData_
//		調べるデータ
//	Record::Tools::FieldLength& iUncompressedSize_
//	Record::Tools::FieldLength& iFieldSize_
//		[OUT] 圧縮前後のサイズ
//
//	RETURN
// 		なし
//
//	EXCEPTIONS

//static
void
Tools::
getVariableSize(const Common::Data& cData_,
				const DataType& cDataType_,
				FieldLength& iUncompressedSize_,
				FieldLength& iFieldSize_)
{
	Common::Data::Function::Value eFunction = Common::Data::Function::Compressed;
	if (!(cData_.getFunction() & eFunction)) {
		
		// 圧縮されていない

		if (cData_.getType() == Common::DataType::String)
		{
			const Common::StringData& cStringData
				= _SYDNEY_DYNAMIC_CAST(const Common::StringData&, cData_);
			
			iFieldSize_ = iUncompressedSize_
				= cStringData.getDumpSize(cDataType_._encodingForm);
		}
		else
		{
			iFieldSize_ = iUncompressedSize_ = cData_.getDumpSize();
		}

		return;
	}
	; _SYDNEY_ASSERT(cData_.getFunction() & eFunction);

	// 圧縮前のサイズを得る
	// ここではstatic_castが使えないので常にdynamic_castを使う
	const Common::CompressedData* pCompressedData =
		dynamic_cast<const Common::CompressedData*>(&cData_);
	; _SYDNEY_ASSERT(pCompressedData);

	iUncompressedSize_ = pCompressedData->getValueSize();
	iFieldSize_ = pCompressedData->getCompressedSize();
}

//
//	FUNCTION public
//	Record::Tools::getAreaTop -- 指定されたエリアの先頭を指すポインターを得る
//
//	NOTES
//
//	ARGUMENTS
//		PhysicalFile::Page* Page_
//			エリアが乗っているページの記述子
//		const PhysicalFile::AreaID AreaID_
//			エリアID
//
//	RETURN
//	エリアの先頭アドレス
//
//	EXCEPTIONS

// static
char*
Tools::getAreaTop(PhysicalFile::Page*			Page_,
				  const PhysicalFile::AreaID	AreaID_)
{
	; _SYDNEY_ASSERT(Page_ != 0);
	; _SYDNEY_ASSERT(AreaID_ != PhysicalFile::ConstValue::UndefinedAreaID);

	return (*Page_).operator char*() + Page_->getAreaOffset(AreaID_);
}

//
//	FUNCTION public
//	Record::Tools::getConstAreaTop -- 指定されたエリアの先頭を指すポインターを得る
//
//	NOTES
//
//	ARGUMENTS
//		PhysicalFile::Page* Page_
//			エリアが乗っているページの記述子
//		const PhysicalFile::AreaID AreaID_
//			エリアID
//
//	RETURN
//	エリアの先頭アドレス
//
//	EXCEPTIONS

// static
const char*
Tools::getConstAreaTop(const PhysicalFile::Page*	Page_,
					   const PhysicalFile::AreaID	AreaID_)
{
	; _SYDNEY_ASSERT(Page_ != 0);
	; _SYDNEY_ASSERT(AreaID_ != PhysicalFile::ConstValue::UndefinedAreaID);

	return (const char*)(*Page_) + Page_->getAreaOffset(AreaID_);
}

/////////////////////////////////////
// Tools::BitMap
/////////////////////////////////////

//
//	FUNCTION public
//	Record::Tools::BitMap::BitMap -- ビットマップを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Tools::BitMap::
BitMap()
	: m_pPointer(0), m_pOriginal(0), m_iBase(0), m_iEnd(0)
{
}

//
//	FUNCTION public
//	Record::Tools::BitMap::~BitMap -- ビットマップを表すクラスのデストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Tools::BitMap::
~BitMap()
{
	if (m_pPointer) delete [] m_pPointer, m_pPointer = 0;
}

//
//	FUNCTION public
//	Record::Tools::BitMap::read -- ビットマップを読み込む
//
//	NOTES
//
//	ARGUMENTS
//		const char* pPointer_
//			ビットマップの開始アドレス
//		ModSize iBase_
//			ビットマップの先頭が対応するオブジェクトの順序数
//		ModSize iEnd_
//			ビットマップの最後の次が対応するオブジェクトの順序数
//
//	RETURN
//		読み込んだバイト数
//
//	EXCEPTIONS

ModSize
Tools::BitMap::
read(const char* pPointer_, ModSize iBase_, ModSize iEnd_)
{
	; _SYDNEY_ASSERT(iEnd_ > iBase_);

#if 0
	ModSize iMemorySize = getBitmapSize(iEnd_ - iBase_);
	if (m_pPointer) {
		if (getBitmapSize(m_iEnd - m_iBase) == iMemorySize) {
			// reuse
			Os::Memory::copy(m_pPointer, pPointer_, iMemorySize);
		} else {
			// free the pointer (allocate only when it is needed)
			delete [] m_pPointer, m_pPointer = 0;
		}
	}
	m_pOriginal = pPointer_;
#else
	if (m_pPointer) delete [] m_pPointer, m_pPointer = 0;
	m_pOriginal = pPointer_;
	ModSize iMemorySize = getBitmapSize(iEnd_ - iBase_);
#endif
	m_iBase = iBase_;
	m_iEnd = iEnd_;
	return iMemorySize;
}

//
//	FUNCTION public
//	Record::Tools::BitMap::create -- ビットマップを作る
//
//	NOTES
//
//	ARGUMENTS
//		ModSize iBase_
//			ビットマップの先頭が対応するオブジェクトの順序数
//		ModSize iEnd_
//			ビットマップの最後の次が対応するオブジェクトの順序数
//
//	RETURN
//		ビットマップのバイト数
//
//	EXCEPTIONS

ModSize
Tools::BitMap::
create(ModSize iBase_, ModSize iEnd_)
{
	; _SYDNEY_ASSERT(iEnd_ > iBase_);

#if 0
	ModSize iMemorySize = getBitmapSize(iEnd_ - iBase_);
	if (m_pPointer) {
		if (getBitmapSize(m_iEnd - m_iBase) == iMemorySize) {
			// reuse
		} else {
			delete [] m_pPointer, m_pPointer = 0;
		}
	}
	if (!m_pPointer)
		m_pPointer = new char[iMemorySize];
#else
	if (m_pPointer) delete [] m_pPointer, m_pPointer = 0;
	ModSize iMemorySize = getBitmapSize(iEnd_ - iBase_);
	m_pPointer = new char[iMemorySize];
#endif

	Os::Memory::reset(m_pPointer, iMemorySize);
	m_iBase = iBase_;
	m_iEnd = iEnd_;
	return iMemorySize;
}

//
//	FUNCTION public
//	Record::Tools::BitMap::write -- ビットマップを書き込む
//
//	NOTES
//
//	ARGUMENTS
//		char* pPointer_
//			ビットマップの開始アドレス
//		ModSize iBase_
//			ビットマップの先頭が対応するオブジェクトの順序数
//		ModSize iEnd_
//			ビットマップの最後の次が対応するオブジェクトの順序数
//
//	RETURN
//		書き込んだバイト数
//
//	EXCEPTIONS

ModSize
Tools::BitMap::
write(char* pPointer_, ModSize iBase_, ModSize iEnd_) const
{
	if (m_iEnd != iEnd_ || m_iBase != iBase_) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	ModSize iMemorySize = getBitmapSize(m_iEnd - m_iBase);
	if (m_pPointer)
		Os::Memory::copy(pPointer_, m_pPointer, iMemorySize);
	else
		Os::Memory::copy(pPointer_, m_pOriginal, iMemorySize);
	return iMemorySize;
}

//
//	FUNCTION public
//	Record::Tools::BitMap::test -- ビットが立っているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		ModSize iIndex_
//			調べるビットの位置
//
//	RETURN
//		true .. ビットが立っている
//		false.. ビットが立っていない
//
//	EXCEPTIONS

bool
Tools::BitMap::
test(ModSize iIndex_) const
{
	; _SYDNEY_ASSERT(iIndex_ >= m_iBase);

	const char* bitmap = (m_pPointer ? m_pPointer : m_pOriginal);
	bitmap += ((iIndex_ - m_iBase) / 8);
	return *bitmap & (0x80 >> ((iIndex_ - m_iBase) % 8));
}

//
//	FUNCTION public
//	Record::Tools::BitMap::set -- ビットを立てる
//
//	NOTES
//
//	ARGUMENTS
//		ModSize iIndex_
//			立てるビットの位置
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Tools::BitMap::
set(ModSize iIndex_)
{
	; _SYDNEY_ASSERT(iIndex_ >= m_iBase);

	if (m_pPointer == 0)
	{
		ModSize iMemorySize = getBitmapSize(m_iEnd - m_iBase);
		m_pPointer = new char[iMemorySize];
		Os::Memory::copy(m_pPointer, m_pOriginal, iMemorySize);
	}
	char* bitmap = m_pPointer + ((iIndex_ - m_iBase) / 8);
	*bitmap |= (0x80 >> ((iIndex_ - m_iBase) % 8));
}

//
//	FUNCTION public
//	Record::Tools::BitMap::reset -- ビットを落とす
//
//	NOTES
//
//	ARGUMENTS
//		ModSize iIndex_
//			落とすビットの位置
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Tools::BitMap::
reset(ModSize iIndex_)
{
	; _SYDNEY_ASSERT(iIndex_ >= m_iBase);

	if (m_pPointer == 0)
	{
		ModSize iMemorySize = getBitmapSize(m_iEnd - m_iBase);
		m_pPointer = new char[iMemorySize];
		Os::Memory::copy(m_pPointer, m_pOriginal, iMemorySize);
	}
	char* bitmap = m_pPointer + ((iIndex_ - m_iBase) / 8);
	*bitmap &= ~(0x80 >> ((iIndex_ - m_iBase) % 8));
}

//
//	FUNCTION public
//	Record::Tools::BitMap::initialize -- ビットマップを初期化する
//
//	NOTES
//
//	ARGUMENTS
//		ModSize iBase_
//			ビットマップの先頭が対応するオブジェクトの順序数
//		ModSize iEnd_
//			ビットマップの最後の次が対応するオブジェクトの順序数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Tools::BitMap::
initialize(ModSize iBase_, ModSize iEnd_)
{
	if (m_pPointer && m_iBase == iBase_ && m_iEnd == iEnd_) {
//		ModSize iMemorySize = getBitmapSize(m_iEnd - m_iBase);
//		Os::Memory::reset(m_pPointer, iMemorySize);
	} else {
		create(iBase_, iEnd_);
	}
}

//
//	FUNCTION public
//	Record::Tools::BitMap::save -- m_pOriginalの内容をm_pPointerにコピーする
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
Tools::BitMap::save()
{
	if (m_pPointer == 0)
	{
		ModSize iMemorySize = getBitmapSize(m_iEnd - m_iBase);
		m_pPointer = new char[iMemorySize];
		Os::Memory::copy(m_pPointer, m_pOriginal, iMemorySize);
		m_pOriginal = 0;
	}
}

//
//	FUNCTION public
//	Record::Tools::BitMap::operator= -- ビットマップの代入演算子
//
//	NOTES
//
//	ARGUMENTS
//		const Tools::BitMap& cOther_
//			代入元
//
//	RETURN
//		自身への参照
//
//	EXCEPTIONS

Tools::BitMap&
Tools::BitMap::
operator= (const BitMap& cOther_)
{
#if 0
	m_pOriginal = 0;
	if (cOther_.m_pPointer)
	{
		ModSize iMemorySize = getBitmapSize(cOther_.m_iEnd - cOther_.m_iBase);
		if (m_pPointer) {
			if (getBitmapSize(m_iEnd - m_iBase) == iMemorySize) {
				// reuse
				;
			} else {
				// free current buffer
				delete [] m_pPointer, m_pPointer = 0;
			}
		}
		if (!m_pPointer)
			m_pPointer = new char[iMemorySize];

		Os::Memory::copy(m_pPointer, cOther_.m_pPointer, iMemorySize);
	}
#else
	if (m_pPointer) delete [] m_pPointer, m_pPointer = 0;
	m_pOriginal = 0;
	if (cOther_.m_pPointer)
	{
		ModSize iMemorySize = getBitmapSize(cOther_.m_iEnd - cOther_.m_iBase);
		m_pPointer = new char[iMemorySize];
		Os::Memory::copy(m_pPointer, cOther_.m_pPointer, iMemorySize);
	}
#endif
	m_pOriginal = cOther_.m_pOriginal;
	m_iBase = cOther_.m_iBase;
	m_iEnd = cOther_.m_iEnd;
	return *this;
}

//
//	FUNCTION public
//	Record::Tools::getBitmapSize -- 指定された個数のOn/Offをビットマップで表すのに必要なバイト数を得る
//
//	NOTES
//
//	ARGUMENTS
//		ModSize n_
//			bitmap対象のオブジェクト数
//
//	RETURN
//	bitmapに必要なバイト数
//
//	EXCEPTIONS

// static
ModSize
Tools::
getBitmapSize(ModSize n_)
{
	// N個のものを表すのに必要なバイト数は(N+7)/8である
	return (n_ + 7) / 8;
}

//
//	FUNCTION public
//	Record::Tools::writeBitmap -- ビットマップを書き出す
//
//	NOTES
//
//	ARGUMENTS
//		char*	pAreaPointer_
//			書き出すアドレス
//		ModSize iBeginIndex_
//		ModSize iEndIndex_
//			ベクター中のどこからどこまで調べるか
//		const Tools::BitMap&	cBitMap_
//			trueなら1が立つ
//
//	RETURN
//	bitmapに使用したバイト数
//
//	EXCEPTIONS

// static
ModSize
Tools::
writeBitmap(char*		pAreaPointer_,
			ModSize		iBeginIndex_,
			ModSize		iEndIndex_,
			const BitMap&	cBitMap_)
{
	; _SYDNEY_ASSERT(pAreaPointer_);
	return cBitMap_.write(pAreaPointer_, iBeginIndex_, iEndIndex_);
}

//
//	FUNCTION public
//	Record::Tools::readBitmap -- ビットマップを読み出す
//
//	NOTES
//
//	ARGUMENTS
//		const char*	pAreaPointer_
//			読み出すアドレス
//		ModSize iBeginIndex_
//		ModSize iEndIndex_
//			ベクターのどこからどこまで読み込むか
//		Tools::BitMap&	cBitMap_
//			ビットが立っていればtrueが格納される
//
//	RETURN
//	bitmapに使用したバイト数
//
//	EXCEPTIONS

// static
ModSize
Tools::
readBitmap(const char*		pAreaPointer_,
		   ModSize			iBeginIndex_,
		   ModSize			iEndIndex_,
		   BitMap&			cBitMap_)
{
	; _SYDNEY_ASSERT(pAreaPointer_);
	return cBitMap_.read(pAreaPointer_, iBeginIndex_, iEndIndex_);
}

#ifndef SYD_COVERAGE
//
//	FUNCTION public
//	Record::Tools::isVariableObjectType -- VariableObjectType のビットを検査
//
//	NOTES
// オブジェクト種に VariableObjectType のビットが立っている場合は true を返す。
//
//	ARGUMENTS
//	const ModUInt32 ulObjectType_
//		オブジェクト種
//
//	RETURN
//	VariableObjectType のビットが立っている場合は true
//
//	EXCEPTIONS
//	なし
//

// static
bool
Tools::isVariableObjectType(const Tools::ObjectType	ObjectType_)
{
	return (ObjectType_ & Tools::m_VariableObjectType) != 0;
}
#endif //SYD_COVERAGE

#ifndef SYD_COVERAGE
//
//	FUNCTION public
//	Record::Tools::isDirectObjectType -- DirectObjectType のビットを検査
//
//	NOTES
// オブジェクト種に DirectObjectType のビットが立っている場合は true を返す。
//
//	ARGUMENTS
//	const ModUInt32 ulObjectType_
//		オブジェクト種
//
//	RETURN
//	DirectObjectType のビットが立っている場合は true
//
//	EXCEPTIONS
//	なし
//

// static
bool
Tools::isDirectObjectType(const Tools::ObjectType	ObjectType_)
{
	return (ObjectType_ & Tools::m_DirectObjectType) != 0;
}
#endif //SYD_COVERAGE

#ifndef SYD_COVERAGE
//
//	FUNCTION public
//	Record::Tools::isIndexObjectType -- IndexObjectType のビットを検査
//
//	NOTES
// オブジェクト種に IndexObjectType のビットが立っている場合は true を返す。
//
//	ARGUMENTS
//	const ModUInt32 ulObjectType_
//		オブジェクト種
//
//	RETURN
//	IndexObjectType のビットが立っている場合は true
//
//	EXCEPTIONS
//	なし
//

// static
bool
Tools::isIndexObjectType(const Tools::ObjectType	ObjectType_)
{
	return (ObjectType_ & Tools::m_IndexObjectType) != 0;
}
#endif //SYD_COVERAGE

//
//	FUNCTION public
//	Record::Tools::isLinkedObjectType -- LinkedObjectType のビットを検査
//
//	NOTES
// オブジェクト種に LinkedObjectType のビットが立っている場合は true を返す。
//
//	ARGUMENTS
//	const ModUInt32 ulObjectType_
//		オブジェクト種
//
//	RETURN
//	LinkedObjectType のビットが立っている場合は true
//
//	EXCEPTIONS
//	なし
//

// static
bool
Tools::isLinkedObjectType(const Tools::ObjectType	ObjectType_)
{
	return (ObjectType_ & Tools::m_LinkedObjectType) != 0;
}

#ifndef SYD_COVERAGE
//
//	FUNCTION public
//	Record::Tools::isDivideObjectType -- DivideObjectType のビットを検査
//
//	NOTES
// オブジェクト種に DivideObjectType のビットが立っている場合は true を返す。
//
//	ARGUMENTS
//	const ModUInt32 ulObjectType_
//		オブジェクト種
//
//	RETURN
//	DivideObjectType のビットが立っている場合は true
//
//	EXCEPTIONS
//	なし
//

// static
bool
Tools::isDivideObjectType(const ObjectType	ObjectType_)
{
	return (ObjectType_ & Tools::m_DivideObjectType) != 0;
}
#endif //SYD_COVERAGE

//	FUNCTION public
//	Record::Tools::readObjectID -- オブジェクトIDを読み込む
//
//	NOTES

//static
ModSize
Tools::
readObjectID(const char* pAreaPointer_,
			 Tools::ObjectID& cResult_)
{
	cResult_ = PhysicalPosition::readObjectID(pAreaPointer_);
	return sizeof(Common::ObjectIDData::FormerType)
		+ sizeof(Common::ObjectIDData::LatterType);
}

//	FUNCTION public
//	Record::Tools::writeObjectID -- オブジェクトIDを書き込む
//
//	NOTES

//static
ModSize
Tools::
writeObjectID(char* pAreaPointer_,
			 Tools::ObjectID cID_)
{
	PhysicalPosition::writeObjectID(pAreaPointer_, cID_);
	return sizeof(Common::ObjectIDData::FormerType)
		+ sizeof(Common::ObjectIDData::LatterType);
}

//	FUNCTION public
//	Record::Tools::Configuration::getMinimumObjectPerPage -- ページあたりのオブジェクト数最小値の設定を得る
//
//	NOTES

// static
int
Tools::Configuration::
getMinimumObjectPerPage()
{
	return _cMinimumObjectPerPage.get();
}

//
//	Copyright (c) 2000, 2001, 2003, 2004, 2005, 2006, 2007, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
