// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenParameter.cpp -- オープンパラメータの実装ファイル
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Vector";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "ModString.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Exception/BadArgument.h"
#include "FileCommon/OpenMode.h"
#include "FileCommon/OpenOption.h"
#include "FileCommon/VectorKey.h"
#include "LogicalFile/TreeNodeInterface.h"
#include "LogicalFile/OpenOption.h"
#include "Vector/OpenOption.h"
#include "Vector/OpenParameter.h"

_SYDNEY_USING

using namespace Vector;

//
//	FUNCTION public
//	Vector::OpenParameter::OpenParameter -- コンストラクタ
//
//	NOTES
//	オープンオプションのコンストラクタ。
//
//	ARGUMENTS
//	
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
OpenParameter::OpenParameter(
	const LogicalFile::OpenOption& rOpenOption_,
	ModUInt32 ulOuterFieldNumber_):
		m_iOpenMode(FileCommon::OpenMode::Unknown),
		m_iReadSubMode(FileCommon::ReadSubMode::Unknown),
		m_cProjection(),
		m_iSelectedFieldCount(0),
//		m_bEstimate(false),		// 既値
		m_bSortOrder(false),	// 既値
		m_bGetByBitSet(false),	// 既値
		m_ulSearchValue(FileCommon::VectorKey::Undefined),
		m_bGetCount(false),
		m_bGottenCount(false)
{

	// オープンモードをデータメンバに保存
	int iOpenMode = rOpenOption_.getInteger( _SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key) );

	if (iOpenMode == FileCommon::OpenOption::OpenMode::Initialize) {
		m_iOpenMode = FileCommon::OpenMode::Initialize;
	} else if (iOpenMode == FileCommon::OpenOption::OpenMode::Read) {
		m_iOpenMode = FileCommon::OpenMode::Read;

		// さらにReadSubModeをデータメンバに保存
		int iReadSubMode = -1;
		bool bFound = rOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::ReadSubMode::Key), iReadSubMode);
		if ( iReadSubMode == FileCommon::OpenOption::ReadSubMode::Scan ) {
			m_iReadSubMode = FileCommon::ReadSubMode::ScanRead;

		} else if (iReadSubMode == FileCommon::OpenOption::ReadSubMode::Fetch ) {
			m_iReadSubMode = FileCommon::ReadSubMode::FetchRead;

			//ReadSubModeが省略されたときはScanと見なす
		} else if (!bFound) {
			m_iReadSubMode = FileCommon::ReadSubMode::ScanRead;
		} else { // 不正な引数
#ifdef DEBUG
			SydDebugMessage << ":" << iReadSubMode << ":" << ModEndl;
			SydDebugMessage << "Wrong ReadSubmode." << ModEndl;
#endif
			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}

	} else if (iOpenMode == FileCommon::OpenOption::OpenMode::Search) {
		m_iOpenMode = FileCommon::OpenMode::Search;

	} else if (iOpenMode == FileCommon::OpenOption::OpenMode::Update) {
		m_iOpenMode = FileCommon::OpenMode::Update;

	} else if (iOpenMode == FileCommon::OpenOption::OpenMode::Batch) {
		m_iOpenMode = FileCommon::OpenMode::Batch;

	} else { // 不正な引数
#ifdef DEBUG
		SydDebugMessage << "wrong openmode." << ModEndl;
#endif
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

#if 0
	{// 見積り指定をデータメンバに保存
	const bool bFind = rOpenOption_.getBoolean(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::Estimate::Key), m_bEstimate);
	if (!bFind) {
		// ユーザが値を設置していない時はデフォルト値を設定
		//- (現状では常に偽となる)
		m_bEstimate = false;
	}
	}
#endif

	{// BitSet指定
    const bool bFind = rOpenOption_.getBoolean(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::GetByBitSet::Key), m_bGetByBitSet);
	if (!bFind) {
		// ユーザが値を設置していない時はデフォルト値を設定
		m_bGetByBitSet = false;
	}
	}

	// フィールド選択指定

	if ( rOpenOption_.getBoolean(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::FieldSelect::Key)) ) {
		// フィールド選択指定が true になっているときは
		// 選択されたフィールドの数と位置も指定されなければならない

		//- m_iSelectedFieldCountをセット
		if ( !(rOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::TargetFieldNumber::Key), m_iSelectedFieldCount)) )
		{
#ifdef DEBUG
			// フィールド選択指定されているのに、
			// いくつ指定したのかを表す値が設定されていない
			SydDebugMessage << "wrong projection count." << ModEndl;
#endif
			throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		}

		int iProjectionFieldID;
		// TargetFieldIndexの値は昇順にソートされていると前提
		for ( int i=0; i<m_iSelectedFieldCount; ++i ) {
			const bool bFind = rOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(FileCommon::OpenOption::TargetFieldIndex::Key, i), iProjectionFieldID);
			if ( bFind
			  && !(m_iOpenMode == FileCommon::OpenMode::Update && iProjectionFieldID == 0)
			  && !(m_iSelectedFieldCount > 1 && iProjectionFieldID >= static_cast<int>(ulOuterFieldNumber_))
			  && !(m_iSelectedFieldCount == 1 && iProjectionFieldID > static_cast<int>(ulOuterFieldNumber_))
			   )
			{
				if (m_iSelectedFieldCount == 1 && iProjectionFieldID == static_cast<int>(ulOuterFieldNumber_))
					// 特殊列が指定されている
					m_bGetCount = true;
				else
//					SydMessage << "[[" << iProjectionFieldID << ModEndl;
					m_cProjection.set(iProjectionFieldID);
			} else {
				//- あるはずのオプションが存在しないか、
				//- あってはならないオプションが存在する
#ifdef DEBUG
				SydDebugMessage << "wrong projection argument." << ModEndl;
#endif
				throw Exception::BadArgument(moduleName, srcFile, __LINE__);
			}
		}			
	} else { // Projectionを設定しない場合

		//まず[1..ulOuterFieldNumber_-1]までをtrueで埋める
		m_iSelectedFieldCount = ulOuterFieldNumber_-1;
	for (int i=1; i<=m_iSelectedFieldCount; ++i) {
			m_cProjection.set(i);
		}
		//続いて、OpenModeがUpdateか否かに従って[0]の値を違える
		if (m_iOpenMode != FileCommon::OpenMode::Update){
			m_cProjection.set(0);
			++m_iSelectedFieldCount;
		}
	}

	// ソートオーダー
	// デフォルト値はfalse(昇順)
	rOpenOption_.getBoolean(_SYDNEY_VECTOR_OPEN_PARAMETER_KEY(Vector::OpenOption::SortOrder::Key), m_bSortOrder);

	if (m_iOpenMode == FileCommon::OpenMode::Search) {
	// Search オプション
#if 0 
		// SearchFieldNumber(必ず1なので無視)
		m_iSearchFieldNumber = rOpenOption_.getInteger(_SYDNEY_VECTOR_OPEN_PARAMETER_KEY(Vector::OpenOption::SearchFieldNumber::Key));
		// SearchFieldIndex(必ず0)
		m_iSearchFieldIndex = rOpenOption_.getInteger(_SYDNEY_VECTOR_OPEN_PARAMETER_KEY(Vector::OpenOption::SearchFieldIndex::Key));
		// SearchOpe(これも"EQ"と決まっている)
		//- ↑が、そのうち他の演算子が復活する可能性あり
		ModUnicodeString cstrSearchOpe = "";
		rOpenOption_.getString(_SYDNEY_VECTOR_OPEN_PARAMETER_KEY(Vector::OpenOption::SearchOpe::Key), cstrSearchOpe);
		if (cstrSearchOpe == Vector::OpenOption::SearchOpe::Equals) {
			m_iSearchOpe = LogicalFile::TreeNodeInterface::Equals;
		}
#endif
		// SearchValue (VectorFileで取る価値があるのはこれだけ)
		m_ulSearchValue = static_cast<ModUInt32>(
			rOpenOption_.getLongLong(
				_SYDNEY_VECTOR_OPEN_PARAMETER_KEY(
					Vector::OpenOption::SearchValue::Key)));
	}
}

//	FUNCTION
//	Vector::OpenParameter::~OpenParameter -- デストラクタ
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
OpenParameter::~OpenParameter()
{
}

// アクセサ

//
//	FUNCTION
//	Vector::OpenParameter::getOpenMode -- オープンモードを返す
//
//	NOTE
//		オープンモードを返す。
//
//	ARGUMENTS
//		なし
//			
//	RETURN
//		const FileCommon::OpenMode::Mode
//
//	EXCEPTIONS
//		なし
//			
const FileCommon::OpenMode::Mode
OpenParameter::getOpenMode() const
{
	return m_iOpenMode;
}

//
//	FUNCTION
//	Vector::OpenParameter::getReadSubMode -- 副オープンモードを返す
//
//	NOTE
//		副オープンモードを返す
//
//	ARGUMENTS
//		なし
//			
//	RETURN
//		const FileCommon::ReadSubMode::Mode
//
//	EXCEPTIONS
//		なし
//			
const FileCommon::ReadSubMode::Mode
OpenParameter::getReadSubMode() const
{
	return m_iReadSubMode;
}

#if 0
//
//	FUNCTION
//	Vector::OpenParameter::getEstimateMode -- 
//    評価モードでファイルをオープンしているか否かを返す
//
//	NOTES
//  評価モードでファイルをオープンしているか否かを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//	  true:  評価モードである
//	  false: 評価モードではない
//
//	EXCEPTIONS
//	なし
//
bool
OpenParameter::getEstimateMode() const
{
	return m_bEstimate;
}
#endif

//
//	FUNCTION
//	Vector::OpenParameter::getsByBitSet -- getの値をBitSetで返すか否かを返す
//
//	NOTE
//		getの値をBitSetで返すか否かを返す
//
//	ARGUMENTS
//		なし
//			
//	RETURN
//		bool
//
//	EXCEPTIONS
//		なし
//			
bool
OpenParameter::getsByBitSet() const
{
	return m_bGetByBitSet;
}

//	FUNCTION
//	Vector::OpenParameter::getSortOrder -- 
//    scanモードのときにどちらの方向にファイルを走査していくかを返す
//
//	NOTES
//	scanモードのときにどちらの方向にファイルを走査していくかを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//	  true:  逆方向
//	  false: 順方向
//
//	EXCEPTIONS
//	なし
//
bool
OpenParameter::getSortOrder() const
{
	return m_bSortOrder;
}

//
//	FUNCTION
//	Vector::OpenParameter::getSearchValue --
//    searchモードのときに対象となるベクタキーを返す
//
//	NOTES
//	searchモードのときに対象となるベクタキーを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//	  対象となるオブジェクトのベクタキー
//
//	EXCEPTIONS
//	なし
//
ModUInt32
OpenParameter::getSearchValue() const
{
	return m_ulSearchValue;
}

//
//	FUNCTION
//	Vector::OpenParameter::getSelectedFieldCount --
//	  プロジェクションの対象になっているフィールドの個数を返す
//
//	NOTES
//	プロジェクションの対象になっているフィールドの個数を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//	  フィールドの個数(外向けの意味で)。
//	 
//	EXCEPTIONS
//	なし
//
int
OpenParameter::getSelectedFieldCount() const
{
	return m_iSelectedFieldCount;
}

//
//	FUNCTION
//	Vector::OpenParameter::isEmptyInnerProjection -- 
//      内部的な意味でのprojectionが空かどうかを確かめる
//
//	NOTE
//		内部的な意味でのprojectionが空かどうかを確かめる
//
//	ARGUMENTS
//		なし
//			
//	RETURN
//		bool
//
//	EXCEPTIONS
//		なし
//			
bool
OpenParameter::isEmptyInnerProjection() const
{
	return (m_iSelectedFieldCount == 0
		|| (m_iSelectedFieldCount == 1 && m_cProjection.test(0)));
}

//
//	FUNCTION
//	Vector::OpenParameter::getOuterMaskAt -- 
//	  或るフィールドがProjectionの対象になっているかどうかを調べる。
//
//	NOTES
//	外向きの意味でのフィールドIDを用いて、
//	或るフィールドがProjectionの対象になっているかどうかを調べる。
//
//	ARGUMENTS
//	ModUInt32 ulOuterFieldID_
//   外向きの意味でのフィールドID。
//
//	RETURN
//	bool
//	  true:  引数のフィールドはプロジェクションの対象である。
//	  false: 引数のフィールドはプロジェクションの対象ではない。
//	 
//	EXCEPTIONS
//	なし
//
bool
OpenParameter::getOuterMaskAt(ModUInt32 ulOuterFieldID_) const
{
	return m_cProjection.test(ulOuterFieldID_);
}

//
//	FUNCTION
//	Vector::OpenParameter::getInnerMaskAt -- 
//	  或るフィールドがProjectionの対象になっているかどうかを調べる。
//
//	NOTES
//	内向きの意味でのフィールドIDを用いて、
//	或るフィールドがProjectionの対象になっているかどうかを調べる。
//
//	ARGUMENTS
//	ModUInt32 ulInnerFieldID_
//	  内向きの意味でのフィールドID。
//
//	RETURN
//	bool
//	  true:  引数のフィールドはプロジェクションの対象である。
//	  false: 引数のフィールドはプロジェクションの対象ではない。
//	 
//	EXCEPTIONS
//	なし
//
bool
OpenParameter::getInnerMaskAt(ModUInt32 ulInnerFieldID_) const
{
	// OuterFieldID == InnerFieldID + 1
	return m_cProjection.test(ulInnerFieldID_+1); 
}	

// FUNCTION public
//	Vector::OpenParameter::isGetCount -- COUNTの特殊列の取得が指示されているか
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
OpenParameter::
isGetCount() const
{
	return m_bGetCount;
}

// FUNCTION public
//	Vector::OpenParameter::isGottenCount -- COUNTの特殊列を取得したか
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
OpenParameter::
isGottenCount() const
{
	return m_bGottenCount;
}

// FUNCTION public
//	Vector::OpenParameter::setGottenCount -- COUNTの特殊列を取得したことにする
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS

void
OpenParameter::
setGottenCount()
{
	m_bGottenCount = true;
}

// FUNCTION public
//	Vector::OpenParameter::clearGottenCount -- COUNTの特殊列を取得していないことにする
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS

void
OpenParameter::
clearGottenCount()
{
	m_bGottenCount = false;
}

// バッチモードか
bool
OpenParameter::
isBatchMode() const
{
	return (m_iOpenMode == FileCommon::OpenMode::Batch);
}

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
