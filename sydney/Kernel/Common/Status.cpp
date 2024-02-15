// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Status.cpp -- ステータスをあらわすクラス
// 
// Copyright (c) 1999, 2001, 2006, 2023 Ricoh Company, Ltd.
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

#include "Common/ClassID.h"
#include "Common/Status.h"
#include "Common/UnicodeString.h"

#include "ModUnicodeString.h"

_TRMEISTER_USING

//
//	FUNCTION public
//	Common::Status::Status -- コンストラクタ(1)
//
//	NOTES
//	コンストラクタ。データ m_eStatus は Undefined に初期化される。
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
Common::Status::Status()
: m_eStatus(Undefined)
{
}

//
//	FUNCTION public
//	Common::Status::Status -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ。データ m_eStatus を引数 eStatus_ で初期化する。
//
//	ARGUMENTS
//	Common::Status::Type eStatus_
//		ステータス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Common::Status::Status(Common::Status::Type eStatus_)
: m_eStatus(eStatus_)
{
}

//
//	FUNCTION public
//	Common::Status::~Status -- デストラクタ
//
//	NOTES
//	デストラクタ。
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
Common::Status::~Status()
{
}

//
//	FUNCTION public
//	Common::Status::getStatus -- ステータスを得る
//
//	NOTES
//	ステータスを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Status::Type
//		ステータス
//
//	EXCEPTIONS
//	なし
//
Common::Status::Type
Common::Status::getStatus() const
{
	return m_eStatus;
}

//
//	FUNCTION public
//	Common::Status::isError -- エラーかどうか判定する
//
//	NOTES
//	エラーかどうかをtrue、falseで返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		エラーの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
Common::Status::isError() const
{
	bool bResult = false;
	if (m_eStatus == Error)
		bResult = true;
	return bResult;
}

//
//	FUNCTION public
//	Common::Status::serialize -- シリアル化のためのメソッド
//
//	NOTES
//	シリアル化を行う
//
//	ARGUMENTS
//	ModSrchive& cArchiver_
//		アーカイバ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Common::Status::serialize(ModArchive& cArchiver_)
{
	if (cArchiver_.isStore())
	{
		//書出し
		int iStatus = m_eStatus;
		cArchiver_ << iStatus;
	}
	else
	{
		//読出し
		int iStatus;
		cArchiver_ >> iStatus;
		m_eStatus = static_cast<Type>(iStatus);
	}
}

//
//	FUNCTION public
//	Common::Status::getClassID -- クラスIDを得る
//
//	NOTES
//	クラスIDを得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		クラスID
//
//	EXCEPTIONS
//	なし
//
int
Common::Status::getClassID() const
{
	return ClassID::StatusClass;
}

//	FUNCTION public
//	Common::Status::toString -- 文字列でデータを得る
//
//	NOTES
//	文字列でデータを得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		データをあらわす文字列
//
//	EXCEPTIONS

ModUnicodeString
Common::Status::toString() const
{
	switch (m_eStatus) {
	case Success:
		return _TRMEISTER_U_STRING("Success");
	case Error:
		return _TRMEISTER_U_STRING("Error");
	case Canceled:
		return _TRMEISTER_U_STRING("Canceled");
	case HasMoreData:
		return _TRMEISTER_U_STRING("HasMoreData");
	}
	return _TRMEISTER_U_STRING("Undefined");
}

//
//	Copyright (c) 1999, 2001, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
