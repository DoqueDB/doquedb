// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InputArchive.cpp -- 入力用アーカイブ
// 
// Copyright (c) 1999, 2001, 2004, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Common";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/Externalizable.h"
#include "Common/InputArchive.h"

#include "Exception/ClassNotFound.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

//
//	FUNCTION public
//	Common::InputArchive::InputArchive -- コンストラクタ
//
//	NOTES
//	コンストラクタ。基底クラスのModArchiveの初期化を行う。
//
//	ARGUMENTS
//	ModSerialIO& cIO_
//		シリアル化入出力先
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
InputArchive::InputArchive(ModSerialIO& cIO_)
: ModArchive(cIO_, ModeLoadArchive)
{
}

//
//	FUNCTION public
//	Common::InputArchive::~InputArchive -- デストラクタ
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
//	なし
//
InputArchive::~InputArchive()
{
}

//
//	FUNCTION public
//	Common::InputArchive::readObject -- オブジェクトを読み込む
//
//	NOTES
//	オブジェクトを読み込む。
//	まずクラスIDを読み込みそのクラスのインスタンスを
//	Common::Externalizable::getClassInstanceで確保し、
//	そのメソッドserializeを実行し、データをポートから読み込む。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Externalizable*
//		Externalizableの派生クラスへのポインタ
//
//	EXCEPTIONS
//	Exception::ClassNotFound
//		Common::Externalizableでは扱えないクラスだった
//	その他
//		下位の例外はそのまま再送
//
Externalizable*
InputArchive::readObject()
{
	//クラスIDを得る
	int iClassID;
	readArchive(iClassID);
	Externalizable* pObject = 0;
	if (iClassID != Externalizable::None)
	{
		//オブジェクトのインスタンスを得る
		pObject = Externalizable::getClassInstance(iClassID);
		if (pObject == 0)
		{
			//Common::Externalizableでは扱えないクラスである。
			throw Exception::ClassNotFound(moduleName,
										 srcFile,
										 __LINE__,
										 iClassID);
		}
		//オブジェクトの中身を読み込む
		pObject->serialize(*this);
	}
	return pObject;
}

//
//	FUNCTION public
//	Common::InputArchive::readObject -- オブジェクトを読み込む
//
//	NOTES
//	オブジェクトを読み込む。
//	まずクラスIDを読み込み、引数のクラスIDと同じなら引数にデータを読み込み、
//	異なっていたら、そのクラスのインスタンスを
//	Common::Externalizable::getClassInstanceで確保し、
//	そのメソッドserializeを実行し、データをポートから読み込む。
//
//	ARGUMENTS
//	Common::Externalizable* data_
//		Externalizableの派生クラスへのポインタ
//
//	RETURN
//	Common::Externalizable*
//		Externalizableの派生クラスへのポインタ
//
//	EXCEPTIONS
//	Exception::ClassNotFound
//		Common::Externalizableでは扱えないクラスだった
//	その他
//		下位の例外はそのまま再送
//
Externalizable*
InputArchive::readObject(Externalizable* data_)
{
	//クラスIDを得る
	int iClassID;
	readArchive(iClassID);
	Externalizable* pObject = 0;
	if (data_ != 0 && data_->getClassID() == iClassID)
	{
		pObject = data_;
	}
	else if (iClassID != Externalizable::None)
	{
		//オブジェクトのインスタンスを得る
		pObject = Externalizable::getClassInstance(iClassID);
		if (pObject == 0)
		{
			//Common::Externalizableでは扱えないクラスである。
			throw Exception::ClassNotFound(moduleName,
										 srcFile,
										 __LINE__,
										 iClassID);
		}
	}
	if (pObject)
	{
		//オブジェクトの中身を読み込む
		pObject->serialize(*this);
	}
	return pObject;
}

//
//	Copyright (c) 1999, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
