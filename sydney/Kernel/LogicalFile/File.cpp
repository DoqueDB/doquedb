// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp -- 論理ファイルの基底クラス
// 
// Copyright (c) 1999, 2001, 2003, 2005, 2007, 2009, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "LogicalFile";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "LogicalFile/File.h"
#include "LogicalFile/OpenOption.h"
#include "LogicalFile/TreeNodeInterface.h"
#include "Common/AutoCaller.h"
#include "Common/IntegerArrayData.h"
#include "Exception/NotSupported.h"
#include "Exception/BadArgument.h"

_SYDNEY_USING
_SYDNEY_LOGICALFILE_USING

//
//	FUNCTION
//	LogicalFile::File::File -- コンストラクタ
//
//	NOTES
//	コンストラクタ
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
File::File()
{
}

//
//	FUNCTION public
//	LogicalFile::File::~File -- デストラクタ
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
File::~File()
{
}

//	FUNCTION public
//	LogicalFile::File::initializeInstance --
//		ファイルのインスタンスを初期化する
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
//	なし
//

//virtual
void
File::initializeInstance()
{
}

//	FUNCTION public
//	LogicalFile::File::terminateInstance --
//		ファイルのインスタンスを後処理する
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
//	なし
//

//virtual
void
File::terminateInstance()
{
}

// FUNCTION public
//	LogicalFile::File::getSize -- 論理ファイルサイズを得る
//
// NOTES
//	引数にTrans::Transaction&をとるほうのgetSizeをオーバーライドしない場合は
//	こちらを実装しなければならない
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUInt64
//
// EXCEPTIONS

//virtual
ModUInt64
File::
getSize() const
{
	// defaultは例外
	// -- サブクラスでTransつきかTransなしかのどちらかをオーバーライドする
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	LogicalFile::File::getSize -- 論理ファイルサイズを得る
//
// NOTES
//	openを伴わないでも実行できる場合はこのメソッドをオーバーライドする
//
// ARGUMENTS
//	const Trans::Transaction& cTrans_
//	
// RETURN
//	ModUInt64
//
// EXCEPTIONS

//virtual
ModUInt64
File::
getSize(const Trans::Transaction& cTrans_)
{
	// default実装では内部でopenして実行
	OpenOption cOpenOption;

	// ReadモードでEstimate=trueにしてオープンする
	cOpenOption.setInteger(OpenOption::KeyNumber::OpenMode, OpenOption::OpenMode::Read);
	cOpenOption.setBoolean(OpenOption::KeyNumber::Estimate, true);

	ModInt64 iResult = 0;

	open(cTrans_, cOpenOption);
	{
		// スコープを抜けたら自動的にcloseする
		Common::AutoCaller0<File> autoCloser(this, &File::close);
		iResult = getSize();
	}

	return iResult;
}

//
//	FUNCTION public
//	LogicalFile::File::getProjectionParameter
//		-- プロジェクションオープンパラメータを得る
//
//	NOTES
//	pNode_ で指定されてたプロジェクション情報を解析し、
//	cOpenOption_ に設定する
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface*	pNode_
//		プロジェクション
//	LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	bool
//		実行可能な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
File::getProjectionParameter(const LogicalFile::TreeNodeInterface* pNode_,
							 LogicalFile::OpenOption& cOpenOption_) const
{
	// 旧インターフェースを持つ、仮想列がないファイルドライバーのために、
	// インターフェースを変換する

	// pNode_ は、複数の場合は TreeNodeInterface::List で、
	// 単独の場合は Field 等になる

	Common::IntegerArrayData cProjection;
	
	if (pNode_->getType() == TreeNodeInterface::List)
	{
		// List -- Operand --> Field
		
		int n = static_cast<int>(pNode_->getOperandSize());
		for (int i = 0; i < n; ++i)
		{
			const TreeNodeInterface* pOperand = pNode_->getOperandAt(i);
		
			if (pOperand->getType() != TreeNodeInterface::Field)
				_TRMEISTER_THROW0(Exception::BadArgument);

			int field = ModUnicodeCharTrait::toInt(pOperand->getValue());
			cProjection.setElement(i, field);
		}
	}
	else if (pNode_->getType() == TreeNodeInterface::Field)
	{
		// Field
		
		int field = ModUnicodeCharTrait::toInt(pNode_->getValue());
		cProjection.setElement(0, field);
	}

	// 旧インターフェースを呼び出す
	return getProjectionParameter(cProjection, cOpenOption_);
}

//
//	FUNCTION public
//	LogicalFile::File::getProjectionParameter
//		-- プロジェクションオープンパラメータを得る
//
//	NOTES
//	旧インターフェース
//
//	ARGUMENTS
//	const Common::IntegerArrayData& cProjection_
//		プロジェクションするフィールドの配列
//	LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	bool
//		実行可能な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
File::getProjectionParameter(const Common::IntegerArrayData& cProjection_,
							 LogicalFile::OpenOption& cOpenOption_) const
{
	_TRMEISTER_THROW0(Exception::BadArgument);
}

//
//	FUNCTION public
//	LogicalFile::File::getSortParameter -- ソート順パラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface *pNode_
//		ソート順指定
//	LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	bool
//		実行可能な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
File::getSortParameter(const LogicalFile::TreeNodeInterface* pNode_,
					   LogicalFile::OpenOption& cOpenOption_) const
{
	// 旧インターフェースを持つ、仮想列がないファイルドライバーのために、
	// インターフェースを変換する

	// pNode_ は TreeNodeInterface::OrderBy である。
	//
	// OrderBy -- Operand --> SortKey -- Operand --> Field
	//		 |				    |------- Option ---> SortDirection
	//		 |											0: ASC
	//		 |											1: DESC
	//		 |------- Option ---> GroupBy

	if (pNode_->getType() != TreeNodeInterface::OrderBy)
		_TRMEISTER_THROW0(Exception::BadArgument);

	Common::IntegerArrayData cKey;
	Common::IntegerArrayData cOrder;

	int n = static_cast<int>(pNode_->getOperandSize());
	for (int i = 0; i < n; ++i)
	{
		const TreeNodeInterface* pSortKey = pNode_->getOperandAt(i);

		if (pSortKey->getType() != TreeNodeInterface::SortKey)
			_TRMEISTER_THROW0(Exception::BadArgument);
		if (pSortKey->getOperandSize() != 1)
			_TRMEISTER_THROW0(Exception::BadArgument);
		if (pSortKey->getOptionSize() != 1)
			_TRMEISTER_THROW0(Exception::BadArgument);

		const TreeNodeInterface* pOperand = pSortKey->getOperandAt(0);
		if (pOperand->getType() != TreeNodeInterface::Field)
			_TRMEISTER_THROW0(Exception::BadArgument);

		// フィールド番号
		int field = ModUnicodeCharTrait::toInt(pOperand->getValue());
		cKey.setElement(i, field);
		
		const TreeNodeInterface* pOption = pSortKey->getOptionAt(0);

		// 順序 0:ASC 1:DESC
		int order = ModUnicodeCharTrait::toInt(pOption->getValue());
		cOrder.setElement(i, order);
	}
	
	return getSortParameter(cKey, cOrder, cOpenOption_);
}

//
//	FUNCTION public
//	LogicalFile::File::getSortParameter -- ソート順パラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::IntegerArrayData& cKey_
//		キーのフィールド番号
//	const Common::IntegerArrayData& cOrder_
//		順序指定 (0:ASC 1:DESC)
//	LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	bool
//		実行可能な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//

bool
File::getSortParameter(const Common::IntegerArrayData& cKey_,
					   const Common::IntegerArrayData& cOrder_,
					   LogicalFile::OpenOption& cOpenOption_) const
{
	_TRMEISTER_THROW0(Exception::BadArgument);
}

//
//	FUNCTION public
//	LogicalFile::File::getLimitParameter -- 取得数と取得位置を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::IntegerArrayData& cSpec_
//		1つまたは2つの要素を持つ配列。0番目はLIMIT、1番目はOFFSETを表す
//
//	RETURN
//	bool
//		実行可能な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
File::getLimitParameter(const Common::IntegerArrayData& cSpec_,
						OpenOption& cOpenOption_) const
{
	return false;
}

//
//	FUNCTION public
//	LogicalFile::File::getProperty -- プロパティを得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData* pKey_
//		プロパティの項目を表すキー
//	Common::DataArrayData* pValue_
//		キーに対応するプロパティのバリュー
//
//	RETURN
//
//	EXCEPTIONS
//
void
File::getProperty(Common::DataArrayData* pKey_,
				  Common::DataArrayData* pValue_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	LogicalFile::File::getLocator -- ロケーターを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pKey_
//		ロケーターを得るエントリを特定するための引数
//
//	RETURN
//	LogicalFile::Locator*
//		ロケーター
//
//	EXCEPTIONS
//
Locator*
File::getLocator(const Common::DataArrayData* pKey_)
{
	return 0;
}

//
//	FUNCTION public
//	LogicalFile::File::undoExpunge -- データの削除を取り消す
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pKey_
//		削除を取り消すデータのキー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::undoExpunge(const Common::DataArrayData* pKey_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	LogicalFile::File::undoUpdate -- データの更新を取り消す
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pKey_
//		更新を取り消すデータのキー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::undoUpdate(const Common::DataArrayData* pKey_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	LogicalFile::File:compact -- ファイルから不要なデータを削除する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	bool& bIncomplete_
//		true
//			今回の処理で処理し残しがある
//		false
//			今回の処理で完全に処理してきている
//
//	bool& bModified_
//		true
//			今回の処理で更新されている
//		false
//			今回の処理で更新されていない
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::compact(const Trans::Transaction& cTransaction_,
			  bool& bIncomplete_, bool& bModified_)
{
}

// FUNCTION public
//	LogicalFile::File::getNoLatchOperation -- ラッチが不要なオペレーションを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	File::Operation::Value
//
// EXCEPTIONS

//virtual
File::Operation::Value
File::
getNoLatchOperation()
{
	// デフォルトはすべてラッチが必要
	return Operation::None;
}

// FUNCTION public
//	LogicalFile::File::getCapability -- Capabilities of file driver
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	File::Capability::Value
//
// EXCEPTIONS

//virtual
File::Capability::Value
File::
getCapability()
{
	// Default: no capabilities
	return Capability::None;
}

//
//	Copyright (c) 1999, 2001, 2003, 2005, 2007, 2009, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
