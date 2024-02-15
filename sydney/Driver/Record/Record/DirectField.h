// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirectField.h -- 固定長用フィールド反復子クラスのヘッダーファイル
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

#ifndef __SYDNEY_RECORD_DIRECTFIELD_H
#define __SYDNEY_RECORD_DIRECTFIELD_H

#include "Common/Common.h"
#include "Common/Object.h"

#include "Record/Module.h"
#include "Record/Tools.h"

_SYDNEY_BEGIN

namespace Common
{
class Data;
}

_SYDNEY_RECORD_BEGIN

class MetaData;

//
//	CLASS
//	Record::DirectField -- 固定長用フィールド反復子のクラス
//
//	NOTES
//	オブジェクトのフィールドをフィールド番号の順に昇順で走査するためのクラス。
//
//	反復子がコンストラクトされてからデストラクトされるまでの間にフィールド
//	値が更新されたり、オブジェクトそのものが削除されることはないと仮定する。
//
//	呼出側で、削除や更新が起きないことを保証しなければいけない。
//
class DirectField : public Common::Object
{
public:
	// コンストラクタ / デストラクタ
	DirectField(const MetaData&	cMetaData_, const char*		pPointer_);
	DirectField(const MetaData&	cMetaData_,       char*		pPointer_);

	~DirectField();

	//
	// アクセッサ
	//

	typedef Tools::FieldNum	FieldID;

#ifdef OBSOLETE
	// フィールドIDを取得
	FieldID getFieldID() const;
#endif //OBSOLETE

	// フィールド値を読み込む
	void readField(Common::Data& cData_) const;

#ifdef DEBUG
	// メソッドが正常に動作可能な状態にあるときだけ true を返す
	// もし false (使用不能状態) ならば seek を実行して正常に戻せる
	bool isValid() const;
#endif //DEBUG

	//
	// マニピュレータ
	//

	// フィールド値を更新する
	void updateField(Common::Data*	pFieldData_);

#ifdef OBSOLETE
	// 順方向への移動
	//   - 固定長フィールドの最後を指していた場合は移動しない
	bool next();
#endif //OBSOLETE

	// 任意の位置(フィールドID)に移動
	bool seek(FieldID	iFieldID_);

	// 使用不能状態にする(seek を呼び出せば、使用不能状態から復帰できる)
	void	clear();

private:
	// メタデータには各フィールドのデータ型など、様々な情報が詰まっている
	const MetaData&								m_cMetaData;
	
	// フィールドデータが開始する位置を指すポインター
	union {
		char*									m_pPointer;
		const char*								m_pConstPointer;
	};

	// 反復子が指しているフィールドのフィールドID
	FieldID										m_iFieldID;
};

_SYDNEY_RECORD_END
_SYDNEY_END

#endif // __SYDNEY_RECORD_DIRECTFIELD_H

//
//	Copyright (c) 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
