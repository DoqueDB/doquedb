// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VariableField.cpp -- 可変長用オブジェクト反復子クラス
// 
// Copyright (c) 2001, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
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
#include "Record/VariableField.h"
#include "Record/LinkedObject.h"
#include "Record/MetaData.h"
#include "Record/Tools.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"
#include "Common/CompressedBinaryData.h"
#include "Common/CompressedStringData.h"
#include "Common/DecimalData.h"

#include "Exception/BadArgument.h"
#include "Exception/Unexpected.h"
#include "FileCommon/DataManager.h"

#include "ModAutoPointer.h"
#include "ModDefaultManager.h"

_SYDNEY_USING
_SYDNEY_RECORD_USING

namespace {
	class _AutoBuffer : public Common::Object
	{
	public:
		explicit _AutoBuffer(Os::Memory::Size iSize_)
		{
			m_pBuffer = (iSize_ > 0) ?
				syd_reinterpret_cast<char*>(ModDefaultManager::allocate(iSize_))
				: 0;
			m_iSize = iSize_;
			m_bAllocated = (iSize_ > 0);
		}
		~_AutoBuffer()
		{
			if (m_bAllocated) {
				ModDefaultManager::free(m_pBuffer, m_iSize);
				m_pBuffer = 0;
				m_iSize = 0;
				m_bAllocated = false;
			}
		}

		char* get() {return m_pBuffer;}
	private:
		char*				m_pBuffer;
		Os::Memory::Size	m_iSize;
		bool				m_bAllocated;
	};

	// createDataの下請け
	const char*
	_createNonArrayData(const char* pPointer_,
						const Tools::DataType& type,
						ModSize iUncompressedSize_,
						ModSize iFieldSize_,
						Common::Data& cData_)
	{
		const char* result = 0;
		if (iUncompressedSize_ != iFieldSize_)
		{
			switch (type._name)
			{
			case Common::DataType::Binary:
				{
					Common::CompressedBinaryData cTmp(iUncompressedSize_);
					result = FileCommon::DataManager::readCommonData(
						cTmp, pPointer_, iFieldSize_);
					; _SYDNEY_ASSERT(cData_.getType()
									 == Common::DataType::Binary);
					Common::BinaryData& c
						= _SYDNEY_DYNAMIC_CAST(Common::BinaryData&, cData_);
					c.setValue(cTmp.getValue(), cTmp.getSize());
				}
				break;
			case Common::DataType::String:
				{
					Common::CompressedStringData cTmp(iUncompressedSize_);
					result = FileCommon::DataManager::readCommonData(
						cTmp, pPointer_, iFieldSize_);
					; _SYDNEY_ASSERT(cData_.getType()
									 == Common::DataType::String);
					Common::StringData& c
						= _SYDNEY_DYNAMIC_CAST(Common::StringData&, cData_);
					c.setValue(cTmp.getValue());
				}
				break;
			}
		}
		else if (type._name == Common::DataType::String)
		{
			result = FileCommon::DataManager::readStringData(
				cData_, type._encodingForm, pPointer_, iFieldSize_);
		}
		else
		{
			// バッファからデータを読み込む
			result = FileCommon::DataManager::readCommonData(
				cData_, pPointer_, iFieldSize_);
		}

		return result;
	}

	// dumpDataの下請け
	char*
	_dumpNonArrayData(char* pPointer_,
					  const Common::Data& cData_,
					  const Tools::DataType& cType_)
	{
		char* p;
		if (cType_._name == Common::DataType::String)
		{
			p = FileCommon::DataManager::writeStringData(
				cData_, cType_._encodingForm, pPointer_);
		}
		else
		{
			p = FileCommon::DataManager::writeCommonData(cData_, pPointer_);
		}
		return p;
	}
}

//
//	FUNCTION public
//	Record::VariableField::VariableField -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const MetaData&				cMetaData_
//		メタデータ(フィールドのデータ種類などが得られる)
//	const char*					pPointer_
//		フィールドデータが開始する位置を指すポインター
//
//	RETURN
//	なし
//
//	EXCEPTIONS

VariableField::
VariableField(const MetaData& cMetaData_,
			  const VariableIterator::ObjectHeader& cHeader_,
			  LinkedObject& cLinkedObject_)
	: m_cMetaData(cMetaData_),
	  m_cHeader(cHeader_),
	  m_cLinkedObject(cLinkedObject_),
	  m_iPosition(0)
{
}

//
//	FUNCTION public
//	Record::VariableField::~VariableField -- デストラクタ
//
//	NOTES
//	デストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	
//
VariableField::
~VariableField()
{
}

//
// アクセッサ
//

//	FUNCTION public
//	Record::VariableField::getFieldID -- フィールド番号を返す
//
//	NOTES
//	反復子に対応しているフィールドのフィールド番号を返す
//
//	ARGUMENTS
//		なし
//	RETURN
//		現在のフィールドID
//
//	EXCEPTIONS

VariableField::FieldID
VariableField::
getFieldID() const
{
	return m_cHeader.m_vecData[m_iPosition].first;
}

//	FUNCTION public
//	Record::VariableField::readField -- 反復子が指しているフィールドを読む
//
//	NOTES
//	反復子が指しているフィールドを読む
//	NULLかどうかのチェックはしないので呼び出し側で行う必要がある
//
//	ARGUMENTS
//	Common::Data& cData_
//		値を設定するクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
VariableField::readField(Common::Data& cData_) const
{
	FieldID iFieldID = getFieldID();
	const VariableIterator::ObjectSize& cSize
		= m_cHeader.m_vecData[m_iPosition].second;
	const Tools::DataType& cType = m_cMetaData.getDataType(iFieldID);
	const Tools::DataType& cElementType = m_cMetaData.getElementType(iFieldID);

	if (m_cLinkedObject.isSplit(cSize.m_iFieldSize) == true)
	{
		// 複数のエリアに分かれているので、バッファを確保して読む

		// データを入れるバッファを作る
		_AutoBuffer buffer(cSize.m_iFieldSize);

		// データを読み込む
		m_cLinkedObject.read(buffer.get(), cSize.m_iFieldSize);

		// データを作る
		createData(cSize, cType, cElementType, cData_, buffer.get());
	}
	else
	{
		// 1つのエリアに収まっているので、そのまま読む

		// データを作る
		createData(cSize, cType, cElementType, cData_,
				   m_cLinkedObject.read(cSize.m_iFieldSize));
	}
	
	// 位置がひとつ進む
	m_iPosition++;

}

//	FUNCTION public
//	Record::VariableField::readField -- 反復子が指しているフィールドを読む
//
//	NOTES
//	反復子が指しているフィールドを読む
//	NULLかどうかのチェックはしないので呼び出し側で行う必要がある
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		読み込んだフィールドを new したデータクラスのインスタンスにセット
//		して返す
//
//	EXCEPTIONS

Common::DataArrayData::Pointer
VariableField::
readField() const
{
	Common::DataArrayData::Pointer	pResult;

	FieldID iFieldID = getFieldID();
	const Tools::DataType& cType = m_cMetaData.getDataType(iFieldID);

	if (cType._name != Common::DataType::Array) {
		// 配列でない場合
		pResult = FileCommon::DataManager::createCommonData(cType);
	} else {
		// 配列の場合
		pResult = new Common::DataArrayData;
	}
	
	readField(*pResult);

	return pResult;
}

//
//	FUNCTION public
//	Record::VariableField::updateField -- フィールドの値を更新
//
//	NOTES
//	フィールドの値を更新。
//	現在のところこのインタフェースで更新できるのは可変長データだけとする。
//
//	ARGUMENTS
//	const Common::DataArrayData::Pointer& pData_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VariableField::updateField(const Common::DataArrayData::Pointer& pData_)
{
	FieldID iFieldID = getFieldID();
	const VariableIterator::ObjectSize& cSize
		= m_cHeader.m_vecData[m_iPosition].second;
	const Tools::DataType& cType = m_cMetaData.getDataType(iFieldID);
	const Tools::DataType& cElementType = m_cMetaData.getElementType(iFieldID);

	// まずはそのまま書き込んで見る
	if (m_cLinkedObject.write(cType, cElementType, pData_, cSize.m_iFieldSize)
		== false)
	{
		// 一度に書き込めなかったので、分割して書く

		// データを入れるバッファを作る
		_AutoBuffer buffer(cSize.m_iFieldSize);

		// データをダンプする
		dumpData(cType, cElementType, pData_, buffer.get());

		// データを書き込む
		m_cLinkedObject.write(buffer.get(), cSize.m_iFieldSize);
	}
	// 位置がひとつ進む
	m_iPosition++;
}

//
//	FUNCTION public
//	Record::VariableField::seek -- 反復子を任意の位置に移動する
//
//	NOTES
//	反復子を任意の位置に移動する
//
//	ARGUMENTS
//	const int iFieldID_
//		反復子の移動先(フィールド番号)
//
//	RETURN
//	移動に成功した場合は true を返す。
//
//	EXCEPTIONS
//
bool
VariableField::seek(VariableField::FieldID	iFieldID_)
{
	if (iFieldID_ == getFieldID()) {
		// 同じ位置なら何もしない
		return true;
	}
	if (iFieldID_ < 1 || iFieldID_ >= m_cMetaData.getFieldNumber()) {
		// 範囲外にはseekできない
		return false;
	}
	if (!m_cMetaData.isVariable(iFieldID_)) {
		// 固定長フィールドにはseekできない
		return false;
	}
	if (iFieldID_ < getFieldID()) {
		// 前方向に戻ることはできない
		return false;
	}

	// 指定されたフィールドIDの位置を得る
	ModSize iPosition = m_iPosition + 1;
	ModSize n = m_cHeader.m_vecData.getSize();
	for (; iPosition < n; ++iPosition) {
		if (m_cHeader.m_vecData[iPosition].first == iFieldID_) {
			break;
		}
	}
	if (iPosition == n) {
		// 見つからなかった
		return false;
	}

	// リンクオブジェクトのポインターを進ませる
	for (ModSize i = m_iPosition; i < iPosition; ++i) {
		m_cLinkedObject.skip(m_cHeader.m_vecData[i].second.m_iFieldSize);
	}
	m_iPosition = iPosition;
	return true;
}

#ifdef OBSOLETE
// Nullでないフィールドの何番目かを得る
ModSize
VariableField::
getPosition() const
{
	return m_iPosition;
}
ModSize
VariableField::
getPosition(FieldID iFieldID_) const
{
	ModSize n = m_cHeader.m_vecData.getSize();
	for (ModSize i = 0; i < n; ++i) {
		if (m_cHeader.m_vecData[i].first == iFieldID_)
			return i;
	}
	return -1;
}
#endif //OBSOLETE

//	FUNCTION private
//	Record::VariableField::createData -- バッファのデータをCommon::Dataにする
//
//	NOTES
//
//	ARGUMENTS
//		const VariableIterator::ObjectSize& cSize_
//			データのサイズを表す型
//		Common::DataType::Type eType_
//			データ型
//		Common::DataType::Type eElementType_
//			データが配列のとき要素の型
//		Common::Data& cData_
//			結果を格納する
//		const _AutoBuffer& cBuffer_
//			ファイルから読み込んだデータが入っているバッファ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
VariableField::
createData(const VariableIterator::ObjectSize& cSize_,
		   const Tools::DataType& cType_,
		   const Tools::DataType& cElementType_,
		   Common::Data& cData_,
		   const char* pBuffer_) const
{
	if (cType_._name != Common::DataType::Array) {

		// 配列でない場合

		(void) _createNonArrayData(
			pBuffer_, cType_,
			cSize_.m_iUncompressedSize,	cSize_.m_iFieldSize, cData_);
	} else {
		// 配列の場合
		// 配列は常にDataArrayDataである
		; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Array);
		Common::DataArrayData& cResult
			= _SYDNEY_DYNAMIC_CAST(Common::DataArrayData&, cData_);
		cResult.clear();	// 初期化する
		cResult.setNull(false);

		if (cSize_.m_iElementNumber == 0) return;

		bool bVariable = cElementType_.isVariable();

		// データ格納部分にも構造があるので解析する必要がある
		const char* pPointer = pBuffer_;
		// 先頭にnull bitmapがある
		ModSize n = cSize_.m_iElementNumber;
		Tools::BitMap cNull;
		pPointer += Tools::readBitmap(pPointer, 0, n, cNull);

		cResult.reserve(n);
		// 要素を作る
		for (ModSize i = 0; i < n; ++i) {

			Common::Data::Pointer pElement
				= FileCommon::DataManager::createCommonData(cElementType_);
			
			if (cNull.test(i)) {
				// NullDataである
				pElement->setNull();
				cResult.pushBack(pElement);
				if (!bVariable) {
					// 固定長の場合はバッファを読み進める必要がある
					pPointer += (cElementType_._name != Common::DataType::Decimal) ?
						FileCommon::DataManager::getFixedCommonDataArchiveSize(cElementType_._name):
						Common::DecimalData::getDumpSizeBy(cElementType_._length, cElementType_._scale);
				}
				continue;
			}

			if (!bVariable) {
				// 固定長の場合
				pPointer = Tools::readFixedField(pPointer,
												 cElementType_,
												 *pElement);
			} else {
				// 可変長の場合は先頭に圧縮前後のサイズが入っている
				Tools::FieldLength iUncompressedSize;
				Tools::FieldLength iFieldSize;

				Os::Memory::copy(&iUncompressedSize, pPointer, Tools::m_FieldLengthArchiveSize);
				pPointer += Tools::m_FieldLengthArchiveSize;

				Os::Memory::copy(&iFieldSize, pPointer, Tools::m_FieldLengthArchiveSize);
				pPointer += Tools::m_FieldLengthArchiveSize;

				pPointer = _createNonArrayData(
					pPointer, cElementType_,
					iUncompressedSize, iFieldSize, *pElement);
			}
			cResult.pushBack(pElement);
		}
	}
}

//	FUNCTION public
//	Record::VariableField::dumpData -- Common::Dataをバッファに書き出す
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataType::Type eType_
//			データ型
//		Common::DataType::Type eElementType_
//			データが配列のとき要素の型
//		Common::DataArrayData::Pointer& pData_
//			結果を格納する
//		const _AutoBuffer& cBuffer_
//			ファイルから読み込んだデータが入っているバッファ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

//static
void
VariableField::
dumpData(const Tools::DataType& cType_,
		 const Tools::DataType& cElementType_,
		 const Common::DataArrayData::Pointer& pData_,
		 char* pBuffer_)
{
	if (cType_._name != Common::DataType::Array) {
		// 配列でない場合
		// 圧縮が必要な場合はヘッダーを作る時点で圧縮されているはず
		(void) _dumpNonArrayData(pBuffer_, *pData_, cType_);

	} else {
		// 配列の場合
		// 配列は常にDataArrayDataである
		const Common::DataArrayData* pDataArray =
			_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData_.get());
		; _SYDNEY_ASSERT(pDataArray);

		bool bVariable = cElementType_.isVariable();

		char* pPointer = pBuffer_;
		// null bitmapを作る
		ModSize n = pDataArray->getCount();
		Tools::BitMap cNull;
		cNull.create(0, n);
		ModSize i = 0;
		for (; i < n; ++i) {
			if (pDataArray->getElement(i)->isNull()) {
				cNull.set(i);
			}
		}

		pPointer += Tools::writeBitmap(pPointer, 0, n, cNull);

		// 要素ごとに書き込む
		for (i = 0; i < n; ++i) {
			if (cNull.test(i)) {
				// NullDataは書き込まない
				if (!bVariable) {
					// 固定長の場合はバッファを読み進める必要がある
					pPointer += (cElementType_._name != Common::DataType::Decimal) ?
						FileCommon::DataManager::getFixedCommonDataArchiveSize(cElementType_._name):
						Common::DecimalData::getDumpSizeBy(cElementType_._length, cElementType_._scale);
				}
				continue;
			}

			// 要素をバッファに書き込む
			const Common::Data& cElement = *pDataArray->getElement(i);
			if (!bVariable) {
				// 固定長の場合
				if (!FileCommon::DataManager::isVariable(cElement)) {
					// 登録データも固定長の場合
					pPointer = Tools::writeFixedField(pPointer, cElementType_, cElement);
				} else {
					// 登録データが可変長の場合
					_SYDNEY_THROW0(Exception::BadArgument);
				}
			} else {
				// 可変長の場合
				if (cElementType_._name == cElement.getType()) {
					//要素型が一致する場合
					// 可変長の場合は先頭に圧縮前後のサイズを書き込む
					Tools::FieldLength iUncompressedSize;
					Tools::FieldLength iFieldSize;
					Tools::getVariableSize(cElement,
										   cElementType_,
										   iUncompressedSize,
										   iFieldSize);

					Os::Memory::copy(pPointer, &iUncompressedSize, Tools::m_FieldLengthArchiveSize);
//					*syd_reinterpret_cast<Tools::FieldLength*>(pPointer) = iUncompressedSize;
					pPointer += Tools::m_FieldLengthArchiveSize;
					Os::Memory::copy(pPointer, &iFieldSize, Tools::m_FieldLengthArchiveSize);
//					*syd_reinterpret_cast<Tools::FieldLength*>(pPointer) = iFieldSize;
					pPointer += Tools::m_FieldLengthArchiveSize;

					pPointer = _dumpNonArrayData(pPointer, cElement, cElementType_);
				} else {
					//要素型が不一致の場合
					if (cElementType_._name == Common::DataType::String) {
						// Stringの場合にのみ cast 可能。
						// 可変長の場合は先頭に圧縮前後のサイズを書き込む
						const Common::StringData cElementStr(cElement.toString());

						Tools::FieldLength iUncompressedSize;
						Tools::FieldLength iFieldSize;
						Tools::getVariableSize(cElementStr,
											   cElementType_,
											   iUncompressedSize,
											   iFieldSize);

						Os::Memory::copy(pPointer, &iUncompressedSize, Tools::m_FieldLengthArchiveSize);
//						*syd_reinterpret_cast<Tools::FieldLength*>(pPointer) = iUncompressedSize;
						pPointer += Tools::m_FieldLengthArchiveSize;
						Os::Memory::copy(pPointer, &iFieldSize, Tools::m_FieldLengthArchiveSize);
//						*syd_reinterpret_cast<Tools::FieldLength*>(pPointer) = iFieldSize;
						pPointer += Tools::m_FieldLengthArchiveSize;

						pPointer = _dumpNonArrayData(pPointer, cElementStr, cElementType_);
					} else {
						_SYDNEY_THROW0(Exception::BadArgument);
					}
				}
			}
		}
	}
}

//
//	Copyright (c) 2001, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
