// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Data.cpp -- ベクターで扱うデータ
// 
// Copyright (c) 2005, 2006, 2009, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Vector2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Vector2/Data.h"
#include "Vector2/FileID.h"

//#include "SyDynamicCast.h"
#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/ObjectIDData.h"
#include "Exception/Object.h"
#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "FileCommon/FileOption.h"
#include "LogicalFile/FileID.h"
#include "Os/Memory.h"

_SYDNEY_BEGIN
_SYDNEY_VECTOR2_BEGIN


//
//	FUNCTION public static
//	Vector2::Data::UnsignedInteger::dump -- ダンプする
//
//	NOTES
//	データをメモリに書き込む
//
//	ARGUMENTS
//	const Common::Data& cData_
//		書き込むデータ
//	char*& buf_
//		書き込まれるメモリ
//		書き込んだ分だけ位置も進める
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::UnsignedInteger::dump(const Common::Data& cData_, char*& buf_)
{
	if (cData_.getType() != Common::DataType::UnsignedInteger)
		_SYDNEY_THROW0(Exception::BadArgument);

	if (cData_.isNull() == true)
		// Vector2のUnsignedIntegerは0xffffffffがnullを表す
		Os::Memory::set(buf_, 0xff, getSize());
	else
		cData_.dumpValue(buf_);

	buf_ += getSize();
}

//
//	FUNCTION public static
//	Vector2::Data::UnsignedInteger::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cData_
//		メモリから取り出したデータの格納先
//	const char*& buf_
//		データが書き込まれているメモリ
//		取り出した分だけ位置も進める
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::UnsignedInteger::getData(Common::Data& cData_, const char*& buf_)
{
	if (cData_.getType() != Common::DataType::UnsignedInteger)
		_SYDNEY_THROW0(Exception::BadArgument);

	if (isNull(buf_) == true)
		cData_.setNull();
	else
		cData_.setDumpedValue(buf_);

	buf_ += getSize();
}

//
//	FUNCTION static
//	Vector2::Data::UnsignedInteger::isNull -- nullかどうか
//
//	NOTES
//	全て0xffならnull
//
//	ARGUMENTS
//	const char* buf_
//		dumpしたメモリー
//
//	RETURN
//	bool
//		nullの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Data::UnsignedInteger::isNull(const char* buf_)
{
	bool isnull = true;
	const unsigned char* p = syd_reinterpret_cast<const unsigned char*>(buf_);
	for (ModSize i = 0; i < getSize(); ++i, ++p)
	{
		if (*p != 0xff)
		{
			isnull = false;
			break;
		}
	}
	
	return isnull;
}

//
//	FUNCTION static
//	Vector2::Data::UnsignedInteger::reset -- 初期化する
//
//	NOTES
//	全て0xffにする
//
//	ARGUMENTS
//	char* buf_
//		初期化するメモリー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::UnsignedInteger::reset(char*& buf_)
{
	Os::Memory::set(buf_, 0xff, getSize());
	buf_ += getSize();
}

//
//	FUNCTION public static
//	Vector2::Data::ObjectID::dump -- ダンプする
//
//	NOTES
//	データをメモリに書き込む
//
//	ARGUMENTS
//	const Common::Data& cData_
//		書き込むデータ
//	char*& buf_
//		書き込まれるメモリ
//		書き込んだ分だけ位置も進める
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::ObjectID::dump(const Common::Data& cData_, char*& buf_)
{
	if (cData_.getType() != Common::DataType::ObjectID)
		_SYDNEY_THROW0(Exception::BadArgument);
	if (cData_.isNull() == true)
	{
		Os::Memory::set(buf_, 0xff, getSize());
	}
	else
	{
		//const Common::ObjectIDData& c
		//	= _SYDNEY_DYNAMIC_CAST(const Common::ObjectIDData&, cData_);
		//c.dumpValue(buf_);

		//    オーバーライドにかかわらず、オブジェクトのクラスに適した
		//	  dumpValueが呼ばれるので、キャストは不要だと思う。
		cData_.dumpValue(buf_);
	}
	buf_ += getSize();
}

//
//	FUNCTION public static
//	Vector2::Data::ObjectID::getData -- 取り出す
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cData_
//		メモリから取り出したデータの格納先
//	const char*& buf_
//		データが書き込まれているメモリ
//		取り出した分だけ位置も進める
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::ObjectID::getData(Common::Data& cData_, const char*& buf_)
{
	if (cData_.getType() != Common::DataType::ObjectID)
		_SYDNEY_THROW0(Exception::BadArgument);
	// dump同様、ここもキャストは不要
	if (isNull(buf_) == true)
		cData_.setNull();
	else
		cData_.setDumpedValue(buf_);
	buf_ += getSize();
}

//
//	FUNCTION static
//	Vector2::Data::ObjectID::isNull -- nullかどうか
//
//	NOTES
//	全て0xffならnull
//
//	ARGUMENTS
//	const char* buf_
//		dumpしたメモリー
//
//	RETURN
//	bool
//		nullの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Data::ObjectID::isNull(const char* buf_)
{
	bool isnull = true;
	const unsigned char* p = syd_reinterpret_cast<const unsigned char*>(buf_);
	for (ModSize i = 0; i < getSize(); ++i, ++p)
	{
		if (*p != 0xff)
		{
			isnull = false;
			break;
		}
	}
	
	return isnull;
}

//
//	FUNCTION static
//	Vector2::Data::ObjectID::reset -- 初期化する
//
//	NOTES
//	全て0xffにする
//
//	ARGUMENTS
//	char* buf_
//		初期化するメモリー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::ObjectID::reset(char*& buf_)
{
	Os::Memory::set(buf_, 0xff, getSize());
	buf_ += getSize();
}

//
//	FUNCTION public
//	Vector2::Data::Data -- コンストラクタ
//
//	NOTES
//	[?] setType()でm_Sizeを初期化している。コンストラクタでも初期化する必要はない？
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Data::Data()
	: m_uiFieldCount(0), m_Size(0)
{
}

//
//	FUNCTION public
//	Vector2::Data::~Data -- デストラクタ
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
Data::~Data()
{
}

//
//	FUNCTION public
//	Vector2::Data::setType -- 型配列を設定する
//
//	NOTES
//	型配列とフィールドの合計長を設定する。
//	FileIDで設定された配列（キーは除かれている）が渡される。
//
//	ARGUMENTS
//	const FileID& cFileID_
//		ファイルID（フィールド情報を持っている）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::setType(const FileID& cFileID_)
{
	m_uiFieldCount = cFileID_.getFieldCount();
	Os::Memory::copy(m_pType, cFileID_.getFieldType(),
					 sizeof(Type::Value) * m_uiFieldCount);
	m_Size = cFileID_.getFieldSize();
}

//
//	FUNCTION public
//	Vector2::Data::dump -- ダンプする
//
//	NOTES
//	データをメモリに書き込む
//
//	ARGUMENTS
//	char* p
//		書き込まれるメモリ
//	const Common::DataArrayData& cData_
//		書き込むデータの配列
//
//	RETURN
//	ModSize
//		サイズ
//
//	EXCEPTIONS
//
ModSize
Data::dump(char* p, const Common::DataArrayData& cData_) const
{
	; _SYDNEY_ASSERT(m_uiFieldCount > 0);
	// m_vecTypeはキーが除かれている
	; _SYDNEY_ASSERT((int)m_uiFieldCount + 1 == cData_.getCount());

	char* s = p;
	// 先頭はキーなので除く
	int n = 1;

	for (ModUInt32 i = 0; i < m_uiFieldCount; ++i, ++n)
	{
		dump(p, *cData_.getElement(n).get(), m_pType[i]);
	}
	return static_cast<ModSize>(p - s);
}

//
//	FUNCTION public
//	Vector2::Data::update -- 更新する
//
//	NOTES
//	与えられたデータのみをメモリに書き込む
//
//	ARGUMENTS
//	char* p
//		書き込まれるメモリ
//	const Common::DataArrayData& cData_
//		書き込むデータの配列
//	const ModVector<int>& vecUpdateField_
//		更新フィールド
//
//	RETURN
//	ModSize
//		サイズ
//
//	EXCEPTIONS
//
ModSize
Data::update(char* p, const Common::DataArrayData& cData_,
			 const int* pUpdateField_,
			 const ModSize uiFieldCount_) const
{
	; _SYDNEY_ASSERT(m_uiFieldCount > 0);

	char* s = p;

	// キーのフィールド数
	int keynum = 1;

	// pUpdateField_で指定されたフィールドを順々に処理する
	int k = 0;
	while (pUpdateField_[k] < keynum)
	{
		// 更新するフィールドがキーフィールドの場合

		++k;
		
		if (k == static_cast<int>(uiFieldCount_))
			// pUpdateField_[uiFieldCount_]は設定されていない
			// つまり、更新するフィールドがキーフィールドだけだった
			return 0;
	}

	// 更新するフィールドはpUpdateField_に昇順に入っている
	// キーフィールドの次から順々に処理する
	// つまり、n = keynum から n = keynum + m_uiFieldCount - 1 まで
	// m_uiFieldCountはキーが除いてある。FileIDのm_uiFieldCountと同じ
	int n = keynum;
	for (ModUInt32 i = 0; i < m_uiFieldCount; ++i, ++n)
	{
		if (k < static_cast<int>(uiFieldCount_) && pUpdateField_[k] == n)
		{
			// 同じフィールド番号

			dump(p, *cData_.getElement(k++).get(), m_pType[i]);
		}
		else
		{
			// 読み飛ばす
			p += getSize(m_pType[i]);
		}
	}
	return static_cast<ModSize>(p - s);
}

//
//	FUNCTION public
//	Vector2::Data::reset -- すべてのフィールドを初期化する
//
//	NOTES
//
//	ARGUMENTS
//	char* p
//		初期化されるメモリー
//
//	RETURN
//	ModSize
//		サイズ
//
//	EXCEPTIONS
//
ModSize
Data::reset(char* p) const
{
	; _SYDNEY_ASSERT(m_uiFieldCount > 0);

	char* s = p;

	for (ModUInt32 i = 0; i < m_uiFieldCount; ++i)
	{
		reset(p, m_pType[i]);
	}

	return static_cast<ModSize>(p - s);
}

//
//	FUNCTION public
//	Vector2::Data::isNull -- すべての要素がnullかどうか
//
//	NOTES
//
//	ARGUMENTS
//	const char* p
//		チェックするメモリー
//
//	RETURN
//	bool
//		すべての要素がnullの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Data::isNull(const char* p) const
{
	; _SYDNEY_ASSERT(m_uiFieldCount > 0);

	bool result = true;

	for (ModUInt32 i = 0; i < m_uiFieldCount; ++i)
	{
		// nullかどうかチェックする
		if (isNull(p, m_pType[i]) == false)
		{
			result = false;
			break;
		}
		
		// 次の要素のメモリーに移動する
		p += getSize(m_pType[i]);
	}

	return result;
}

//
//	FUNCTION public
//	Vector2::Data::getData -- データを得る
//
//	NOTES
//	dump()時にkeyを除いているので、keyは取得できない。
//
//	ARGUMENTS
//	const char* p
//		タプル(field列またはvalue列)が書き込まれているメモリ
//	Common::DataArrayData& cTuple_
//		読み込んだデータを格納する配列
//	const ModVector<int>& vecGetField_
//		取得するフィールド番号
//
//	RETURN
//	bool
//		nullではないデータが含まれていた場合はtrue、それ以外はfalse
//		[?] 指定フィールドにはnullしか含まれていなくても、
//			指定フィールド以外にnull以外のデータが含まれていればtrue？
//
//	EXCEPTIONS
//
bool
Data::getData(const char* p,
			  Common::DataArrayData& cTuple_,
			  const int* pGetField_,
			  const ModSize uiFieldCount_) const
{
	; _SYDNEY_ASSERT(m_uiFieldCount > 0);

	bool result = false;

	// [NOTE] 新オプティマイザーからはVector2も索引と同等のものとして使用する。
	//  キーでfetchしてキーだけを取得するという動作も許す。
	bool bGetOnlyKey = false;
	int k = 0;
	while (pGetField_[k] == 0)
	{
		if (++k == static_cast<int>(uiFieldCount_))
		{
			// 指定フィールドがキーフィールドだけだった
			; _TRMEISTER_ASSERT(uiFieldCount_ == 1);
			bGetOnlyKey = true;
			break;
		}
	}

	// 各フィールドを順々に指定フィールドかどうか調べる。
	// nはキーを含めたフィールド番号。
	// kは指定フィールド配列（キーを含めたフィールド番号）の添え字。
	// iはフィールド型配列（キーを除いたフィールド番号）の添え字。
	int n = 1;
	for (ModUInt32 i = 0; i < m_uiFieldCount; ++n, ++i)
	{
		if (bGetOnlyKey == false && pGetField_[k] == n)
		{
			// 指定フィールドの場合

			Common::Data::Pointer pData = cTuple_.getElement(k);
			getData(p, m_pType[i], *pData);
			if (pData->isNull() == false)
				result = true;

			if (++k == static_cast<int>(uiFieldCount_))
				break;
		}
		else
		{
			// キーだけを取得する、または、指定フィールド以外の場合
			
			// nullかどうかチェックする
			if (isNull(p, m_pType[i]) == false)
				result = true;

			// 読み飛ばす
			p += getSize(m_pType[i]);
		}
	}
	
	return result;
}

//
//	FUNCTION public static
//	Vector2::Data::getFieldType-- フィールドタイプを得る
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID
//  int iPosition_
//		フィールドの位置
//	ModSize& uiFieldSize_
//		フィールドのサイズ
//
//	RETURN
//	Type::Value result
//		フィールドの型
//
//	EXCEPTIONS
//		サポートしていない型
//
Data::Type::Value
Data::getFieldType(const LogicalFile::FileID&	cFileID_,
				   int							iPosition_,
				   ModSize&						uiFieldSize_)
{
	Data::Type::Value result;

	Common::DataType::Type eType
		= static_cast<Common::DataType::Type>(
			cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileCommon::FileOption::FieldType::Key, iPosition_)));

	switch (eType)
	{
	case Common::DataType::ObjectID:
		result = Data::Type::ObjectID;
		break;
	case Common::DataType::UnsignedInteger:
		result = Data::Type::UnsignedInteger;
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	uiFieldSize_ = getSize(result);
	return result;
}

//
//	FUNCTION private
//	Vector2::Data::dump -- 1つのダンプする
//
//	NOTES
//
//	ARGUMENTS
//	char*& p
//		書き込まれるメモリ
//	const Common::Data& cData_
//		書き込むデータ
//	Vector2::Data::Type::Value eType_
//		データ型
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::dump(char*& p, const Common::Data& cData_, Type::Value eType_) const
{
	switch (eType_)
	{
	case Type::ObjectID:
		Data::ObjectID::dump(cData_, p);
		break;
	case Type::UnsignedInteger:
		Data::UnsignedInteger::dump(cData_, p);
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

//
//	FUNCTION private
//	Vector2::Data::getData -- 1つデータを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32*& p
//		読み込まれるフィールドのメモリ(フィールド列全体のメモリではない)
//	Vector2::Data::Type::Value eType_
//		データ型
//	Common::Data& cData_
//		読み込んだデータの格納先
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::getData(const char*& p,
			  Type::Value eType_,
			  Common::Data& cData_) const
{
	switch (eType_)
	{
	case Type::ObjectID:
		Data::ObjectID::getData(cData_, p);
		break;
	case Type::UnsignedInteger:
		Data::UnsignedInteger::getData(cData_, p);
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

//
//	FUNCTION private
//	Vector2::Data::reset -- １つのデータを初期化する
//
//	NOTES
//
//	ARGUMENTS
//	char*& p
//		初期化されるメモリ
//	Vector2::Data::Type::Value eType_
//		データ型
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Data::reset(char*& p,
			Type::Value eType_) const
{
	switch (eType_)
	{
	case Type::ObjectID:
		Data::ObjectID::reset(p);
		break;
	case Type::UnsignedInteger:
		Data::UnsignedInteger::reset(p);
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

//
//	FUNCTION private static
//	Vector2::Data::getSize -- 1つのサイズを得る。
//
//	NOTES
//
//	ARGUMENTS
//	Vector2::Data::Type::Value eType_
//		サイズを知りたいデータ型
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ModSize
Data::getSize(Type::Value eType_)
{
	ModSize size = 0;
	
	switch (eType_)
	{
	case Type::ObjectID:
		size = Data::ObjectID::getSize();
		break;
	case Type::UnsignedInteger:
		size = Data::UnsignedInteger::getSize();
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	return size;
}	

//
//	FUNCTION private static
//	Vector2::Data::isNull -- 1つのデータがnullかどうかを得る
//
//	NOTES
//
//	ARGUMENTS
//	const char* p
//		読み込むメモリー
//	Vector2::Data::Type::Value eType_
//		サイズを知りたいデータ型
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
bool
Data::isNull(const char* p, Type::Value eType_)
{
	bool result = false;
	
	switch (eType_)
	{
	case Type::ObjectID:
		result = Data::ObjectID::isNull(p);
		break;
	case Type::UnsignedInteger:
		result = Data::UnsignedInteger::isNull(p);
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	return result;
}	

_SYDNEY_VECTOR2_END
_SYDNEY_END

//
//	Copyright (c) 2005, 2006, 2009, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
