// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LanguageData.cpp -- 言語指定関連の関数定義
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2007, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Common";
const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Common/Assert.h"
#include "Common/ClassID.h"
#include "Common/LanguageData.h"
#include "Common/NullData.h"
#include "Common/StringData.h"

#include "Exception/BadArgument.h"
#include "Exception/ClassCast.h"
#include "Exception/NullNotAllowed.h"

#include "ModHasher.h"
#include "ModMemory.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace
{
}

// FUNCTION public
//	Common::LanguageData::hashCode -- ハッシュコードを取り出す
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
LanguageData::
hashCode() const
{
	if (isNull()) return 0;

	return ModUnicodeStringHasher()(getString());
}

//	FUNCTION private
//	Common::LanguageData::serialize_NotNull -- シリアライズする
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&		archiver
//			シリアライズに使用するアーカイバ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
LanguageData::serialize_NotNull(ModArchive& archiver)
{
	; _TRMEISTER_ASSERT(!isNull());

	ScalarData::serialize_NotNull(archiver);
	_v.serialize(archiver);
}

//	FUNCTION private
//	Common::LanguageData::copy_NotNull -- 自分自身の複製を生成する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		生成された複製へのポインタ
//
//	EXCEPTIONS

Data::Pointer
LanguageData::copy_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return new LanguageData(*this);
}

//	FUNCTION private
//	Common::LanguageData::cast_NotNull -- キャストする
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataType::Type	type
//			このデータ型にキャストする
//
//	RETURN
//		キャストされたデータへのポインタ
//
//	EXCEPTIONS
//		Exception::ClassCast
//			不可能な型へキャストしようとした

Data::Pointer
LanguageData::cast_NotNull(DataType::Type type, bool bForAssign_ /* = false */) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (type) {
	case DataType::String:
		return new StringData(getString());

	case DataType::Language:
		return copy();

	case DataType::Null:
		return NullData::getInstance();
	}

	_TRMEISTER_THROW0(Exception::ClassCast);
}

//	FUNCTION private
//	Common::LanguageData::getString_NotNull -- 言語指定の文字列表現を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた言語指定の文字列表現
//
//	EXCEPTIONS

ModUnicodeString
LanguageData::getString_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return _v.getName();
}

//	FUNCTION public
//	Common::LanguageData::getValue -- 言語指定集合を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた言語指定集合
//
//	EXCEPTIONS
//		なし

const ModLanguageSet&
LanguageData::getValue() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return _v;
}

// FUNCTION public
//	Common::LanguageData::setValue -- 文字列の形式から言語指定集合を設定する
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LanguageData::
setValue(const ModUnicodeString& cstrValue_)
{
	_v = cstrValue_;
	setNull(false);
}

//	FUNCTION private
//	Common::LanguageData::compareTo_NoCast -- 大小比較を行う
//
//	NOTES
//		比較するデータは自分自身と同じ型である必要がある
//
//	ARGUMENTS
//		Common::Data&	r
//			右辺に与えられたデータ
//
//	RETURN
//		0
//			左辺と右辺は等しい
//		-1
//			左辺のほうが小さい
//		1
//			右辺のほうが小さい
//
//	EXCEPTIONS
//		なし

int
LanguageData::compareTo_NoCast(const Data& r) const
{
#ifdef OBSOLETE // castの結果Nullになることがある場合生かす
	if (isNull())
		return NullData::getInstance()->compareTo(r);
#endif

	; _TRMEISTER_ASSERT(r.getType() == DataType::Language);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const LanguageData& data = _SYDNEY_DYNAMIC_CAST(const LanguageData&, r);

	return (getValue() == data.getValue()) ? 0 :
		(getValue() < data.getValue()) ? -1 : 1;
}

//	FUNCTION private
//	Common::LanguageData::getClassID_NotNull -- クラス ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたクラス ID
//
//	EXCEPTIONS
//		なし

int
LanguageData::getClassID_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return ClassID::LanguageDataClass;
}

// FUNCTION public
//	Common::LanguageData::assign_NoCast -- 代入を行う(キャストなし)
//
// NOTES
//
// ARGUMENTS
//	const Data& r
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
LanguageData::
assign_NoCast(const Data& r)
{
#ifdef OBSOLETE // castの結果Nullになることがある場合生かす
	if (r.isNull()) {
		setNull();
		return false;
	}
#endif

	; _TRMEISTER_ASSERT(r.getType() == getType());
	; _TRMEISTER_ASSERT(!r.isNull());

	const LanguageData& data = _SYDNEY_DYNAMIC_CAST(const LanguageData&, r);
	_v = data.getValue();
	setNull(false);
	return true;
}

//	FUNCTION private
//	Common::LanguageData::isAbleToDump_NotNull -- ダンプ可能か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			ダンプ可能である
//		false
//			ダンプ不可である
//
//	EXCEPTIONS
//		なし

bool
LanguageData::isAbleToDump_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::LanguageData::getDumpSize_NotNull -- ダンプサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたサイズ(B 単位)
//
//	EXCEPTIONS

ModSize
LanguageData::getDumpSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	//【注意】	ModSerializer::sizeOfSerialized は
	//			const 関数でよいはずなのに、const 関数でない

	return const_cast<LanguageData*>(this)->_v.sizeOfSerialized();
}

//	FUNCTION public
//	Common::LanguageData::setDumpedValue --
//		ダンプされた値を自分にリストアする
//
//	NOTES
//
//	ARGUMENTS
//		char*			p
//			ダンプされた値が格納されている領域の先頭アドレス
//		ModSize			size
//			ダンプされた値が格納されている領域のサイズ(B 単位)
//
//	RETURN
//		自分にリストアしたダンプされた値のサイズ(B 単位)
//
//	EXCEPTIONS

ModSize
LanguageData::setDumpedValue(const char* p, ModSize size)
{
	ModMemory memory(const_cast<char*>(p), size);
	ModArchive archiver(memory, ModArchive::ModeLoadArchive);

	archiver >> _v;

	// NULL 値でなくする

	setNull(false);

	return archiver.getSize();
}

ModSize
LanguageData::setDumpedValue(ModSerialIO& cSerialIO_, ModSize size)
{
	ModArchive archiver(cSerialIO_, ModArchive::ModeLoadArchive);
		
	archiver >> _v;

	// NULL 値でなくする

	setNull(false);

	return size;
}

ModSize
LanguageData::dumpValue_NotNull(ModSerialIO& cSerialIO_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModArchive archiver(cSerialIO_, ModArchive::ModeStoreArchive);

	archiver << const_cast<LanguageData*>(this)->_v;

	return getDumpSize();
}

//	FUNCTION private
//	Common::LanguageData::dumpValue_NotNull -- 値をダンプする
//
//	NOTES
//
//	ARGUMENTS
//		char*			p
//			値をダンプする領域の先頭アドレス
//
//	RETURN
//		ダンプされた値のサイズ(B 単位)
//
//	EXCEPTIONS

ModSize
LanguageData::dumpValue_NotNull(char* p) const
{
	; _TRMEISTER_ASSERT(!isNull());

	const ModSize size = getDumpSize();
	ModMemory memory(p, size);
	ModArchive archiver(memory, ModArchive::ModeStoreArchive);

	archiver << const_cast<LanguageData*>(this)->_v;

	return size;
}

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2007, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
