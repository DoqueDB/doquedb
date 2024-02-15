// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OutputArchive.cpp -- オブジェクトをポートに書き込む
// 
// Copyright (c) 1999, 2001, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/Externalizable.h"
#include "Common/OutputArchive.h"

_TRMEISTER_USING

//
//	FUNCTION public
//	Common::OutputArchive::OutputArchive -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	ModSerialID& cIO_
//		シリアル化入出力先
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Common::OutputArchive::OutputArchive(ModSerialIO& cIO_)
: ModArchive(cIO_, ModeStoreArchive)
{
}

//
//	FUNCTION public
//	Common::OutputArchive::OutputArchive -- デストラクタ
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
Common::OutputArchive::~OutputArchive()
{
}

//
//	FUNCTION public
//	Common::OutputArchive::writeObject -- オブジェクトを書き出す
//
//	NOTES
//	オブジェクトを書き出す
//
//	ARGUMENTS
//	const Common::Externalizable* pObject_
//		ポートに書出すオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Common::OutputArchive::writeObject(const Externalizable* pObject_)
{
	if (pObject_)
	{
		//クラスIDを書く
		writeArchive(pObject_->getClassID());
		//オブジェクトを書く
		(const_cast<Externalizable*>(pObject_))->serialize(*this);
	}
	else
	{
		//nullなのでExternalizable::Noneを書く
		int n = Externalizable::None;
		writeArchive(n);
	}
}

//
//	Copyright (c) 1999, 2001, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
