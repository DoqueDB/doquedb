// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// XID.h -- トランザクションブランチ識別子関連のクラス定義、関数宣言
// 
// Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_TRANS_XID_H
#define __SYDNEY_TRANS_XID_H

#include "Trans/Module.h"
#include "Trans/Externalizable.h"

#include "Common/BinaryData.h"

_SYDNEY_BEGIN
_SYDNEY_TRANS_BEGIN

class Literal;

//	CLASS
//	Trans::XID -- トランザクションブランチ識別子を表す型
//
//	NOTES

class XID
	: public	Common::Object,				//【注意】	最初に継承すること
	  public	Externalizable
{
public:
	struct GlobalTransactionID
	{
		enum
		{
			// グローバルトランザクション識別子の最小サイズ(B単位)
			SizeMin =			1,
			// グローバルトランザクション識別子の最大サイズ(B単位)
			SizeMax =			64
		};
	};

	struct BranchQualifier
	{
		enum
		{
			// トランザクションブランチ限定子の最小サイズ(B単位)
			SizeMin =			0,
			// トランザクションブランチ限定子の最大サイズ(B単位)
			SizeMax =			64
		};
	};

	struct FormatID
	{
		//	ENUM
		//	Trans::XID::FormatID::Value --
		//		トランザクションブランチ識別子における
		//		グローバルトランザクション識別子と
		//		トランザクションブランチ限定子の生成の仕方を表す値の列挙型
		//
		//	NOTES

		typedef int				Value;
		enum
		{
			// 識別子が空値であることを表す
			Null  =				-1,
			// OSR CCR名前付けルール
			OSR_CCR,
			// デフォルト値
			Default
		};
	};

	// デフォルトコンストラクタ
	XID();
	// コンストラクタ
	explicit XID(const Common::BinaryData& gtrID);
	XID(const Common::BinaryData& gtrID,
		const Common::BinaryData& bqual,
		FormatID::Value formatID = FormatID::Default);
	// デストラクタ
	~XID();

	// == 演算子
	bool
	operator ==(const XID& r) const;

	// このクラスをシリアル化する
	SYD_TRANS_FUNCTION
	virtual void
	serialize(ModArchive& archiver);
	// このクラスのクラスIDを得る
	virtual int
	getClassID() const;

	// 不正な識別子か
	bool
	isIllegal() const;
	// 空値か
	bool
	isNull() const;

	// グローバルトランザクション識別子の取得
	const Common::BinaryData&
	getGlobalTransactionID() const;
	// トランザクションブランチ限定子の取得
	const Common::BinaryData&
	getBranchQualifier() const;
	// フォーマット識別子を得る
	FormatID::Value
	getFormatID() const;

private:
	// グローバルトランザクション識別子
	Common::BinaryData	_gtrID;
	// トランザクションブランチ限定子
	Common::BinaryData	_bqual;
	// フォーマット識別子
	FormatID::Value		_formatID;
};

//	FUNCTION public
//	Trans::XID::XID -- デフォルトコンストラクタ
//
//	NOTES
//		空値なトランザクションブランチ識別子を生成する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
XID::XID()
	: _formatID(FormatID::Null)
{}

//	FUNCTION public
//	Trans::XID::XID -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Common::BinaryData&	gtrID
//			グローバルトランザクション識別子を表すバイナリデータ
//		Common::BinaryData&	bqual
//			指定されたとき
//				トランザクションブランチ限定子を表すバイナリデータ
//			指定されないとき
//				長さ0のバイナリデータが指定されたものとみなす
//		Trans::XID::FormatID::Value	formatID
//			指定されたとき
//				フォーマット識別子を表す整数値
//			指定されないとき
//				Trans::XID::FormatID::Defaultが指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::XA_InvalidIdentifier
//			

inline
XID::XID(const Common::BinaryData& gtrID)
	: _gtrID(gtrID),
	  _formatID(FormatID::Default)
{}

inline
XID::XID(const Common::BinaryData& gtrID,
		 const Common::BinaryData& bqual, int formatID)
	: _gtrID(gtrID),
	  _bqual(bqual),
	  _formatID(formatID)
{}

//	FUNCTION public
//	Trans::XID::~XID -- デストラクタ
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

inline
XID::~XID()
{}

//	FUNCTION public
//	Trans::XID::operator == -- == 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Trans::XID&	r
//			自分自身と比較するトランザクションブランチ識別子
//
//	RETURN
//		true
//			自分自身と等しい
//		false
//			等しくない
//
//	EXCEPTIONS
//		なし

inline
bool
XID::operator ==(const XID& r) const
{
	return (isNull() && r.isNull()) ||
		!(isNull() || r.isNull() ||
		  !getGlobalTransactionID().equals(&r.getGlobalTransactionID()) ||
		  !getBranchQualifier().equals(&r.getBranchQualifier()) ||
		  getFormatID() != r.getFormatID());
}

//	FUNCTION public
//	Trans::XID::getClassID -- このクラスのクラス ID を得る
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
//		なし

inline
int
XID::getClassID() const
{
	return Externalizable::Category::XID +
		Common::Externalizable::TransClasses;
}

//	FUNCTION public
//	Trans::XID::isIllegal --
//		自分自身が不正なグローバルトランザクション識別子であるか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			不正である
//		false
//			不正でない
//
//	EXCEPTIONS
//		なし

inline
bool
XID::isIllegal() const
{
	return getFormatID() < FormatID::Null ||
		(getFormatID() > FormatID::Null &&
		 (getGlobalTransactionID().isNull() ||
		  getGlobalTransactionID().getSize() < GlobalTransactionID::SizeMin ||
		  getGlobalTransactionID().getSize() > GlobalTransactionID::SizeMax ||
		  getBranchQualifier().isNull() ||
		  getBranchQualifier().getSize() < BranchQualifier::SizeMin ||
		  getBranchQualifier().getSize() > BranchQualifier::SizeMax));
}

//	FUNCTION public
//	Trans::XID::isNull --
//		自分自身が空値であるか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			空値である
//		false
//			空値でない
//
//	EXCEPTIONS
//		なし

inline
bool
XID::isNull() const
{
	return getFormatID() == FormatID::Null;
}

//	FUNCTION public
//	Trans::XID::getGlobalTransactionID --
//		グローバルトランザクション識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		グローバルトランザクション識別子を表すバイナリ値
//
//	EXCEPTIONS
//		なし

inline
const Common::BinaryData&
XID::getGlobalTransactionID() const
{
	return _gtrID;
}

//	FUNCTION public
//	Trans::XID::getBranchQualifier --
//		トランザクションブランチ限定子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		トランザクションブランチ限定子を表すバイナリ値
//
//	EXCEPTIONS
//		なし

inline
const Common::BinaryData&
XID::getBranchQualifier() const
{
	return _bqual;
}

//	FUNCTION public
//	Trans::XID::getFormatID -- フォーマット識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		フォーマット識別子を表す整数値
//
//	EXCEPTIONS
//		なし

inline
int
XID::getFormatID() const
{
	return _formatID;
}

_SYDNEY_TRANS_END
_SYDNEY_END

#endif //__SYDNEY_TRANS_XID_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
