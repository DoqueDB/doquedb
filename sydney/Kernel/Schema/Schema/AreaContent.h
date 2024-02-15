// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaContent.h -- エリアに格納されるスキーマオブジェクトを表すクラス
// 
// Copyright (c) 2000, 2001, 2002, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SCHEMA_AREACONTENT_H
#define __SYDNEY_SCHEMA_AREACONTENT_H

#include "Common/Common.h"
#include "Schema/Object.h"
#include "Schema/AreaCategory.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

class Area;
namespace SystemTable
{
	class AreaContent;
}

//	CLASS
//	Schema::AreaContent --
//		エリアとそれに格納されるスキーマオブジェクトの関係を表すクラス
//
//	NOTES

class AreaContent
	: public Schema::Object
{
public:
	friend class SystemTable::AreaContent;

	//	TYPEDEF public
	//	Schema::AreaContent::Pointer -- AreaContentを保持するObjectPointer
	//
	//	NOTES

	typedef AreaContentPointer Pointer;

	// コンストラクター
	AreaContent();
	// デストラクター
	~AreaContent();

	static AreaContent*		getNewInstance(const Common::DataArrayData& cData_);
												// DataArrayDataを元にインスタンスを生成する

	void					clear();			// メンバーをすべて初期化する

	static Pointer			create(Area& cArea_, const Object& cObject_,
								   AreaCategory::Value eAreaCategory_,
								   Trans::Transaction& cTrans_);
												// オブジェクトを生成する
	static void				drop(Area& cArea_, const Object& cObject_,
								 AreaCategory::Value eAreaCategory_,
								 Trans::Transaction& cTrans_,
								 bool bRecovery_ = false);
	void					drop(Trans::Transaction& cTrans_, bool bRecovery_ = false);
												// オブジェクトを抹消する
	static void				undoDrop(Area& cArea_, const Object& cObject_,
									 AreaCategory::Value eAreaCategory_,
									 Trans::Transaction& cTrans_);
												// 破棄マークをクリアする

	void					moveFile(Trans::Transaction& cTrans_,
									 const ModVector<ModUnicodeString>& vecPrevPath_,
									 const ModVector<ModUnicodeString>& vecPostPath_,
									 bool bUndo_ = false, bool bRecovery_ = false,
									 bool bMount_ = false);
												// エリアの定義変更による
												// ファイルの移動
	static AreaContent*		moveArea(Trans::Transaction& cTrans_,
									 Area* pPrevArea_, Area* pPostArea_,
									 const Object* pObject_, AreaCategory::Value eCategory_,
									 bool bUndo_ = false, bool bRecovery_ = false, bool bMount_ = false);
												// エリアの割り当て変更による
												// 格納関係の変更
	static void				undoMoveArea(Trans::Transaction& cTrans_,
										 Area* pPrevArea_, Area* pPostArea_,
										 const Object* pObject_, AreaCategory::Value eCategory_,
										 AreaContent* pContent_);
												// エリアの割り当て変更による
												// 格納関係の変更の取り消し
	static void				doBeforePersist(const Pointer& pContent_,
											Status::Value eStatus_,
											bool bNeedToErase_,
											Trans::Transaction& cTrans_);
												// 永続化前に行う処理

	static void				doAfterPersist(const Pointer& pContent_,
										   Status::Value eStatus_,
										   bool bNeedToErase_,
										   Trans::Transaction& cTrans_);
												// 永続化後に行う処理
	// システム表からの読み込み前に行う処理
	static void				doAfterLoad(const Pointer& pContent_,
										Area& cArea_,
										Trans::Transaction& cTrans_);

//	Object::
//	Timestamp				getTimestamp();		// タイムスタンプを得る
//	ID::Value				getID() const;		// オブジェクト ID を得る
//	ID::Value				getParentID() const;
//												// 親オブジェクトの
//												// オブジェクト ID を得る
//	const Name&				getName() const;	// オブジェクトの名前を得る
//	Category::Value			getCategory() const;
//												// オブジェクトの種別を得る

	void					reset(Database& cDatabase_);
	void					reset();
												// 下位オブジェクトを抹消する

	SYD_SCHEMA_FUNCTION
	ID::Value				getAreaID() const;	// 対象とするエリアの
												// オブジェクトIDを得る
	SYD_SCHEMA_FUNCTION
	Area*					getArea(Trans::Transaction& cTrans_) const;
												// 対象とするエリアを得る
	void					setAreaID(ID::Value iID_);
												// 対象とするエリアの
												// オブジェクトIDを設定する
	// 対象とするオブジェクトのID
	SYD_SCHEMA_FUNCTION
	ID::Value				getObjectID() const;

	void					setObjectID(ID::Value id);

	// 対象とするオブジェクトの種別
	SYD_SCHEMA_FUNCTION
	Category::Value			getObjectCategory() const;
	void					setObjectCategory(Category::Value category_);

	// 格納関係の種別
	SYD_SCHEMA_FUNCTION
	AreaCategory::Value		getAreaCategory() const;
	void					setAreaCategory(AreaCategory::Value category_);

	SYD_SCHEMA_FUNCTION
	virtual void			serialize(ModArchive& archiver);
												// このクラスをシリアル化する
	SYD_SCHEMA_FUNCTION
	virtual int				getClassID() const;	// このクラスのクラス ID を得る

	// 論理ログ出力、REDOのためのメソッド
	// pack、unpackの下請けとしても使用される

	virtual int				getMetaFieldNumber() const;
	virtual Meta::MemberType::Value
							getMetaMemberType(int iMemberID_) const;

	virtual Common::Data::Pointer packMetaField(int iMemberID_) const;
	virtual bool			unpackMetaField(const Common::Data* pData_, int iMemberID_);

protected:
//	Object::
//	void					addTimestamp();		// タイムスタンプを進める
//	ID::Value				setID(ID::Value id);
//												// オブジェクト ID を設定する
//	ID::Value				setParentID(ID::Value parent);
//												// 親オブジェクトの
//												// オブジェクト ID を設定する
//	Category::Value			setCategory(Category::Value category);
//												// オブジェクトの種別を設定する

private:
	// コンストラクター
	AreaContent(Area& cArea_, const Object& cObject_, AreaCategory::Value eAreaCategory_);
	void					destruct();			// デストラクター下位関数

	// 以下のメンバーは、コンストラクト時にセットされる
//	Object::

	// 以下のメンバーは、「エリア格納関係」表を検索して得られる

//	Object::
//	ID::Value				_id;				// オブジェクト ID
//	ID::Value				_parent;			// 親オブジェクトの
//												// オブジェクト ID
//	Name					_name;				// オブジェクトの名前
//	Category::Value			_category;			// オブジェクトの種別

// エリアIDは_parentを用いる
//	ID::Value				m_iAreaID;			// エリアのID
	ID::Value				m_iObjectID;		// エリアに格納される
												// オブジェクトのID
	Category::Value			m_eObjectCategory;	// エリアに格納される
												// オブジェクトの種別
	AreaCategory::Value		m_eAreaCategory;	// オブジェクトから見た
												// エリアの種別
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif //__SYDNEY_SCHEMA_AREACONTENT_H

//
//	Copyright (c) 2000, 2001, 2002, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
