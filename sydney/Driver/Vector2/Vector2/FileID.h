// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.h -- LogicalFile::FileIDのラッパークラス
// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR2_FILEID_H
#define __SYDNEY_VECTOR2_FILEID_H

#include "Vector2/Module.h"
#include "Vector2/Data.h"

#include "Lock/Name.h"
#include "LogicalFile/FileID.h"
#include "Os/Path.h"
#include "Os/Memory.h"
#include "ModTypes.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_VECTOR2_BEGIN

//
//	TYPEDEF
//	Vector2::LogicalFileID --
//
//	NOTES
//	Vector2::FileIDが直接LogicalFile::FileIDを継承できないので、
//	このtypedefを間に挟む。VC6のバグ。
//
typedef LogicalFile::FileID LogicalFileID;

//
//	CLASS
//	Vector2::FileID --
//
//	NOTES
//	【FileIDとは】
//	[?] ファイルの情報や状態を保持？
//	LogicalFile::FileIDを継承したVector2のFileID。
//
//	【オブジェクトの寿命】
//	"create table"時にcreateされる。
//	フィールド型以外は、FILE_PARAMETER_KEYに設定される。
//	[?] LogicalInterface::create()はFileIDを返しているので、そこで保持されている？
//	[?] さらにLogicalInterfaceのコントラクタの引数がFileIDなので、二回目以降は、ここで作成されたFileIDを使っている？
//
//	LogicalInterfaceのコンストラクタでFileIDは渡される。
//	FileIDのcreateが呼ばれるのはcreate tableの時だけだけど、
//	FileIDのコンストラクタ自体は毎回呼ばれる。
//
//	【制約】
//	ページサイズ
//	最大フィールド数
//		キーは含める
//		仮想フィールドは含めない
//
//	【Dataとは】
//	フィールドの型の情報を保持する。
//	[?] Dataも型の配列とサイズ情報を持っているので、FileIDはDataへの参照を持って、
//		自分ではDataを管理しないほうがいい？
//
//	【ページデータサイズとは】
//	ページサイズからVersionなどの下位層が使う分を除いたサイズのこと。
//	create で設定されるページサイズをVectorが全て使えるわけではない。
//	Versionのような下位層もその一部を使っているため。
//
class FileID : public LogicalFileID
{
public:
	// バージョン
	enum
	{
		Version1 = 0,
		Version2,
		Version3,

		// バージョン数
		ValueNum,
		// 現在のバージョン
		CurrentVersion = ValueNum - 1
	};

	// コンストラクタ
	FileID(const LogicalFile::FileID& cLogicalFileID_);
	// デストラクタ
	virtual ~FileID();

	// ファイルIDの内容を作成する
	void create();

	// ページサイズ
	Os::Memory::Size getPageSize() const;
	// ページデータサイズ
	Os::Memory::Size getPageDataSize() const;

	// LockNameを得る
	const Lock::FileName& getLockName() const;

	// 読み取り専用か
	bool isReadOnly() const;
	// 一時か
	bool isTemporary() const;
	// マウントされているか
	bool isMounted() const;
	void setMounted(bool bFlag_);

	// パス名を得る
	const Os::Path& getPath() const;
	void setPath(const Os::Path& cPath_);

	// フィールドの型を得る（キーを除く）
	const Data::Type::Value* getFieldType() const;
	// フィールド数を得る（キーを除く）
	const ModSize getFieldCount() const;
	//@@const ModSize getFieldCount() const { return m_uiFieldCount; }
	// 合計フィールドサイズを得る（キーを除く）
	const ModSize getFieldSize() const;
	//@@const ModSize getFieldSize() const { return m_uiFieldSize; }

	// バージョンをチェックする
	static bool checkVersion(const LogicalFile::FileID& cLogicalFileID_);

	// count()の仮想フィールド番号を得る
	ModSize getCountFieldNumber() const;

private:
	// Field情報をロードする
	void loadFieldInformation() const;

	// FieldTypeとサイズを得る
	Data::Type::Value getFieldType(int iPosition_,
								   ModSize& uiFieldSize_ ) const;

	// 以下はFileIDの中にあるが、スピードを考え同じ値をメンバーとして持つもの

	// Fileのロック名
	mutable Lock::FileName m_cLockName;
	// パス名
	mutable Os::Path m_cPath;

	// フィールドの型（キーを除く）
	mutable Data::Type::Value m_pFieldType[Data::MaxFieldCount];
	//@@mutable ModVector<Data::Type::Value> m_vecFieldType;
	// フィールドの個数（キーを除く）
	mutable ModSize m_uiFieldCount;
	// フィールド長の合計(byte)（キーを除く）
	mutable ModSize m_uiFieldSize;
};

_SYDNEY_VECTOR2_END
_SYDNEY_END

#endif //__SYDNEY_VECTOR2_FILEID_H

//
//	Copyright (c) 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
