// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirectField.cpp -- 固定長用オブジェクト反復子クラス
// 
// Copyright (c) 2001, 2004, 2023 Ricoh Company, Ltd.
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
#include "Record/DirectField.h"
#include "Record/MetaData.h"
#include "Record/Tools.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"
#include "Exception/BadArgument.h"
#include "Exception/Unexpected.h"
#include "FileCommon/DataManager.h"

_SYDNEY_USING
_SYDNEY_RECORD_USING

//
//	FUNCTION public
//	Record::DirectField::DirectField -- コンストラクタ
//
//	NOTES
//		読み込み用
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

DirectField::
DirectField(const MetaData& cMetaData_, const char* pPointer_)
	: m_cMetaData(cMetaData_),
	  m_pConstPointer(pPointer_),
	  m_iFieldID(1)
{
	; _SYDNEY_ASSERT(m_pConstPointer);
}

//
//	FUNCTION public
//	Record::DirectField::DirectField -- コンストラクタ
//
//	NOTES
//		書き込み用
//
//	ARGUMENTS
//	const MetaData&				cMetaData_
//		メタデータ(フィールドのデータ種類などが得られる)
//	char*						pPointer_
//		フィールドデータが開始する位置を指すポインター
//
//	RETURN
//	なし
//
//	EXCEPTIONS

DirectField::
DirectField(const MetaData& cMetaData_, char* pPointer_)
	: m_cMetaData(cMetaData_),
	  m_pPointer(pPointer_),
	  m_iFieldID(1)
{
	; _SYDNEY_ASSERT(m_pPointer);
}

//
//	FUNCTION public
//	Record::DirectField::~DirectField -- デストラクタ
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
DirectField::
~DirectField()
{
	clear();
}

//
// アクセッサ
//

#ifdef OBSOLETE
//
//	FUNCTION public
//	Record::DirectField::getFieldID -- フィールド番号を返す
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

DirectField::FieldID
DirectField::
getFieldID() const
{
	return m_iFieldID;
}
#endif //OBSOLETE

//	FUNCTION public
//	Record::DirectField::readField -- 反復子が指しているフィールドを読む
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
DirectField::readField(Common::Data& cData_) const
{
	; _SYDNEY_ASSERT(isValid());

	// 反復子が指しているフィールドを読んで値を返す

	const char*	pPointer = m_pConstPointer;
	; _SYDNEY_ASSERT(pPointer);

	Tools::readFixedField(pPointer,
						  m_cMetaData.getDataType(m_iFieldID),
						  cData_);
}

#ifdef DEBUG
//	FUNCTION public
//	Record::DirectField::isValid -- 反復子の正当性検査
//
//	NOTES
// メソッドが正常に動作可能な状態にあるときだけ true を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	メソッドが正常に動作可能な状態にあるときだけ true
//
//	EXCEPTIONS

bool
DirectField::isValid() const
{
	return (m_pConstPointer
			&& m_iFieldID > 0
			&& m_iFieldID < m_cMetaData.getFieldNumber());
}
#endif //DEBUG

//
//	FUNCTION public
//	Record::DirectField::updateField -- フィールドの値を更新
//
//	NOTES
//	フィールドの値を更新。
//	現在のところこのインタフェースで更新できるのは固定長データだけとする。
//
//	ARGUMENTS
//	Common::Data* pData_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DirectField::updateField(Common::Data*	pFieldData_)
{
	; _SYDNEY_ASSERT(pFieldData_->getType() == m_cMetaData.getDataType(m_iFieldID)._name);
	Tools::writeFixedField(m_pPointer, m_cMetaData.getDataType(m_iFieldID), *pFieldData_);
}

#ifdef OBSOLETE
//	FUNCTION public
//	Record::DirectField::next -- 反復子を順方向に移動
//
//	NOTES
//	反復子を順方向に移動する。
//	移動した結果、データメンバが次のフィールドの先頭を指した状態に変化する。
//
//	フィールド数の上限値を越えて移動することはできない。
//	もし、上限を越えて移動しようとすると false が返る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	移動に成功した場合は true を返す。
//
//	EXCEPTIONS

bool
DirectField::next()
{
	return seek(m_iFieldID + 1);
}
#endif //OBSOLETE

//
//	FUNCTION public
//	Record::DirectField::seek -- 反復子を任意の位置に移動する
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
DirectField::seek(DirectField::FieldID	iFieldID_)
{
	if (iFieldID_ == m_iFieldID) {
		// 同じ位置なら何もしない
		return true;
	}
	if (iFieldID_ < 1 || iFieldID_ >= m_cMetaData.getFieldNumber()) {
		// 範囲外にはseekできない
		return false;
	}
	if (m_cMetaData.isVariable(iFieldID_)) {
		// 可変長フィールドにはseekできない
		return false;
	}

	// 現在のフィールドから目的のフィールドまでの距離(単位:フィールド数)
	int	iSkipCount = iFieldID_ - m_iFieldID;

	// 前方向に戻る
	{
		for (int i = -1; i >= iSkipCount; --i) {
			if (!m_cMetaData.isVariable(m_iFieldID + i)) {
				m_pConstPointer -= m_cMetaData.getFieldSize(m_iFieldID + i);
			}
		}
	}
	// 後方向に戻る
	{
		for (int i = 0; i < iSkipCount; ++i) {
			if (!m_cMetaData.isVariable(m_iFieldID + i)) {
				m_pConstPointer += m_cMetaData.getFieldSize(m_iFieldID + i);
			}
		}
	}
	m_iFieldID = iFieldID_;
	return true;
}

//
//	FUNCTION public
//	Record::DirectField::clear -- 反復子をコンストラクト直後の状態にする
//
//	NOTES
//	反復子をコンストラクト直後の状態にする。もし、この状態で isValid が
//	呼び出されると false が得られる。seek を呼び出せば反復子に対応する
//	オブジェクトが存在するようになるので isValid が true を返すようになる。
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
DirectField::clear()
{
	m_iFieldID = 0;
	m_pConstPointer = 0;
	; _SYDNEY_ASSERT(!m_pPointer);
}

//
//	Copyright (c) 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
