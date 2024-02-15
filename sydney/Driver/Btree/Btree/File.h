// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.h -- Ｂ＋木ファイルクラスのヘッダーファイル
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE_FILE_H
#define __SYDNEY_BTREE_FILE_H

#include "Btree/Module.h"

#include "Common/StringData.h"
#include "Common/StringArrayData.h"

#include "LogicalFile/File.h"

#include "Btree/SearchHint.h"
#include "Btree/FetchHint.h"
#include "Btree/NullBitmap.h"
#include "Btree/PageVector.h"
#include "Btree/FileParameter.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN

namespace Trans
{
class Transaction;
}

namespace PhysicalFile
{
class File;
}

namespace LogicalFile
{
class FileID;
class OpenOption;
}

_SYDNEY_BTREE_BEGIN

class FileInformation;
class Estimator;
class OpenParameter;
class UseInfo;
class ValueFile;
class HintTools;
class KeyInformation;
class NodePageHeader;

typedef LogicalFile::File	LogicalFileFile;

//
//	CLASS
//	Btree::File -- Ｂ＋木ファイルクラス
//
//	NOTES
//	Ｂ＋木ファイルクラス。
//
class SYD_BTREE_FUNCTION File : public LogicalFile::File
{
	friend class FileParameter;
	friend class Estimator;
	friend class ValueFile;

public:

	// コンストラクタ
	File(const LogicalFile::FileID&	FileID_);

	// デストラクタ
	~File();

	// システムを初期化する
	static void initialize();

	// システムの後処理をする
	static void terminate();

	// 論理ファイルIDを返す
	const LogicalFile::FileID& getFileID() const;

	//
	// Schema Information
	//

	// ファイルサイズを返す [byte]
	ModUInt64 getSize() const;
#ifdef SYD_C_MS6_0
	ModUInt64 getSize(const Trans::Transaction& trans_);
				// VC6だと継承関係の判定にバグがあり、
				// using宣言ができないため、実装の上書きはないが定義する
#else
	using LogicalFile::File::getSize;
#endif
	// 挿入されているオブジェクト数を返す
	ModInt64 getCount() const;

	//
	// Query Optimization
	//

	// 検索時のオーバヘッドコストを返す
	double getOverhead() const;

	// オブジェクトへアクセスする際のプロセスコストを返す
	double getProcessCost() const;

	// 検索オープンパラメータを設定する
	bool
		getSearchParameter(
			const LogicalFile::TreeNodeInterface*	pCondition_,
			LogicalFile::OpenOption&				cOpenOption_) const;

	// プロジェクションオープンパラメータを設定する
	bool
		getProjectionParameter(
			const Common::IntegerArrayData&	cProjection_,
			LogicalFile::OpenOption&		cOpenOption_) const;

	// 更新オープンパラメータを設定する
	bool
		getUpdateParameter(
			const Common::IntegerArrayData&	cUpdateFields_,
			LogicalFile::OpenOption&		cOpenOption_) const;

	// ソート順パラメータを設定する
	bool
		getSortParameter(
			const Common::IntegerArrayData&	cKeys_,
			const Common::IntegerArrayData&	cOrders_,
			LogicalFile::OpenOption&		cOpenOption_) const;

	//
	//	Data Manipulation
	//

	// ファイルを生成する
	const LogicalFile::FileID&
		create(const Trans::Transaction& cTransaction_);

	// ファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_);

	// 実体である OS ファイルが存在するか調べる
	bool
	isAccessible(bool force = false) const;
	// マウントされているか調べる
	bool
	isMounted(const Trans::Transaction& trans) const;

	// ファイルを空の状態にする
	void clear(const Trans::Transaction&	Transaction_);

	// ファイルをオープンする
	void open(const Trans::Transaction&			cTransaction_,
			  const LogicalFile::OpenOption&	cOpenOption_);

	// ファイルをオープンする
	void open(const Trans::Transaction&	Transaction_,
			  const OpenParameter&		OpenParameter_);

	// ファイルがオープンされているかどうかを知らせる
	bool isOpen() const;

	// オープンパラメータを設定し直す
	void resetOpenParameter(const OpenParameter&	OpenParameter_);

	// ファイルをクローズする
	void close();

	// 物理ページキャッシュの開始を指示する
	void startPageCache();

	// 物理ページキャッシュの終了を指示する
	void endPageCache();

	// Fetch検索条件を設定する
	void fetch(const Common::DataArrayData* pOption_);

	// オブジェクトを返す
	//Common::Data* get();
	bool get(Common::DataArrayData* pTuple_);

	// オブジェクトを挿入する
	void insert(Common::DataArrayData* pObject_);

	//
	//	STRUCT
	//	Btree::File::UpdateSearchTarget --
	//		オブジェクト更新時の検索対象
	//
	//	NOTES
	//	オブジェクト更新時の検索対象。
	//
	struct UpdateSearchTarget
	{
		//
		//	ENUM
		//	Btree::File::UpdateSearchTarget::Value --
		//		オブジェクト更新時の検索対象
		//
		//	NOTES
		//	オブジェクト更新時の検索対象。
		//
		enum Value
		{
			// “オブジェクト全体が検索条件と一致するオブジェクト”
			// を削除／更新
			Object = 0,
			// “キー値が検索条件と一致するオブジェクト”
			// を削除／更新
			Key
		};
	};

	// オブジェクトを更新する
	void update(const Common::DataArrayData*	SearchCondition_,
				Common::DataArrayData*			Object_);

	// ※ 転置ファイルドライバは、
	// 　 第3引数にBtree::File::UpdateSearchTarget::Keyを指定して、
	// 　 こちらを呼び出してください。
	void update(const Common::DataArrayData*	SearchCondition_,
				Common::DataArrayData*			Object_,
				const UpdateSearchTarget::Value	SearchTarget_);

	// オブジェクトを削除する
	void expunge(const Common::DataArrayData*	SearchCondition_);

	// ※ 転置ファイルドライバは、
	// 　 第2引数にBtree::File::UpdateSearchTarget::Keyを指定して、
	// 　 こちらを呼び出してください。
	void expunge(const Common::DataArrayData*		SearchCondition_,
				 const UpdateSearchTarget::Value	SearchTarget_);

	// 巻き戻しの位置を記録する
	void mark();

	// 記録した位置に戻る
	void rewind();

	// カーソルをリセットする
	void reset();

	// 比較
	bool equals(const Common::Object* pOther_) const;

	//
	// Utiliry
	//

	// ファイルを移動する
	void move(
		const Trans::Transaction&		cTransaction_,
		const Common::StringArrayData&	cArea_);

	// ラッチが不要なオペレーションを返す
	virtual Operation::Value getNoLatchOperation();

	// ファイルの文字列表現を返す
	ModUnicodeString	toString() const;

	//
	// 運用管理のためのメソッド
	//

	// ファイルをマウントする
	const LogicalFile::FileID&
		mount(const Trans::Transaction&	Transaction_);

	// ファイルをアンマウントする
	const LogicalFile::FileID&
		unmount(const Trans::Transaction&	Transaction_);

	// ファイルをフラッシュする
	void flush(const Trans::Transaction&	Transaction_);

	// バックアップ開始を通知する
	void startBackup(const Trans::Transaction&	Transaction_,
					 const bool					Restorable_);

	// バックアップ終了を通知する
	void endBackup(const Trans::Transaction&	Transaction_);

	// 障害回復する
	void recover(const Trans::Transaction&	Transaction_,
				 const Trans::TimeStamp&	Point_);

	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction&	Transaction_,
				 const Trans::TimeStamp&	Point_);

	// 同期をとる
	void
	sync(const Trans::Transaction& trans, bool& incomplete, bool& modified);

	//
	// 整合性検査のためのメソッド
	//

	// 整合性検査を行う
	void verify(const Trans::Transaction&		Transaction_,
				const unsigned int				Treatment_,
				Admin::Verification::Progress&	Progress_);

	//
	// 以下はデバッグ用の関数
	//

	// オブジェクトまでの道順を設定する
	// （ルートノードから、リーフまでの各ページの物理ページ識別子を設定）
#ifdef DEBUG
	void setRoute(ModUInt64					ValueObjectID_,
				  Common::IntegerArrayData&	Route_) const;
#endif

private:
	friend class HintTools;
	friend class KeyInformation;
	friend class NodePageHeader;

	//
	//	TYPEDEF
	//	Btree::File::ObjectType --
	//		オブジェクトタイプを示すデータ型
	//
	//	NOTES
	//	オブジェクトタイプを示すデータ型。
	//
	typedef	unsigned char	ObjectType;

	//
	//	TYPEDEF
	//	Btree::File::InsideVarFieldLen --
	//		外置きではない可変長フィールドのフィールド長を示すデータ型
	//
	//	NOTES
	//	外置きではない可変長フィールドのフィールド長を示すデータ型。
	//
	typedef unsigned char	InsideVarFieldLen;

	//
	// File.cpp
	//

	// 親クラスとクラス名が同じ場合の operator= 問題を回避する
	File&	operator= (const File&	cObject_);

	// ファイルをオープンする
	void open(const Trans::Transaction&			Transaction_,
			  const LogicalFile::OpenOption*	OpenOption_,
			  const OpenParameter*				OpenParameter_);

	// ノード／リーフページを生成する
	PhysicalFile::PageID createNodePage(const bool bIsLeaf_) const;

	// オブジェクトをビットセットにして返す
	bool getBitSet(FileInformation&	FileInfo_,
				   ValueFile*		ValueFile_,
				   Common::BitSet&	cResult_);

#ifdef OBSOLETE // 将来に対する予約
	// 代表バリューオブジェクトかどうかを知らせる
	bool
		isDirectObject(
			PhysicalFile::Page*			ValuePage_,
			const PhysicalFile::AreaID	ValueObjectAreaID_) const;

	// 代表バリューオブジェクトかどうかを知らせる
	bool isDirectObject(const ObjectType	ObjectType_) const;
#endif

	// オブジェクトIDを生成して返す
	static ModUInt64 makeObjectID(PhysicalFile::PageID	PageID_,
								  PhysicalFile::AreaID	AreaID_);

	// 次のオブジェクトを返す
	bool getNextObject(ValueFile*	ValueFile_, Common::DataArrayData* pTuple_);

	// 次のオブジェクトのIDを返す
	ModUInt64 getNextObjectID(PageVector&	AttachNodePages_,
							  ValueFile*	ValueFile_,
							  PageVector&	AttachValuePages_) const;

	// 次のオブジェクトのIDを返す(Scan)
	ModUInt64 getNextObjectIDScan(PageVector&	AttachNodePages_) const;

	// 次のオブジェクトのIDを返す(Search)
	ModUInt64 getNextObjectIDSearch(PageVector&	AttachNodePages_) const;

	// 次のオブジェクトのIDを返す
	// （先頭キーフィールドに対する単一条件でSearch）
	ModUInt64
		getNextObjectIDBySingleCondition(
			PageVector&	AttachNodePages_) const;

	// 次のオブジェクトのIDを返す
	// （先頭キーフィールドに対するEquals条件でSearch）
	ModUInt64
		getNextObjectIDBySingleEquals(PageVector&	AttachNodePages_) const;

	// 次のオブジェクトのIDを返す
	// （先頭キーフィールドに対するLessThan(Equals)条件でSearch）
	ModUInt64
		getNextObjectIDBySingleLessThan(PageVector&	AttachNodePages_,
										const bool	ContainEquals_) const;

	// 次のオブジェクトのIDを返す
	// （先頭キーフィールドに対するGreaterThan(Equals)条件でSearch）
	ModUInt64
		getNextObjectIDBySingleGreaterThan(
			PageVector&	AttachNodePages_,
			const bool	ContainEquals_) const;

	// 次のオブジェクトのIDを返す
	// （先頭キーフィールドに対するEqualsToNull条件でSearch）
	ModUInt64
		getNextObjectIDBySingleEqualsToNull(
			PageVector&	AttachNodePages_) const;

	// 次のオブジェクトのIDを返す
	// （先頭キーフィールドに対する範囲指定でSearch）
	ModUInt64
		getNextObjectIDBySpanCondition(PageVector&	AttachNodePages_) const;

	// 次のオブジェクトのIDを返す
	// （複数キーフィールドに対する複合条件でSearch）
	ModUInt64
		getNextObjectIDByMultiCondition(PageVector&	AttachNodePages_) const;

	// 前のオブジェクトを返す
	bool getPrevObject(ValueFile*	ValueFile_, Common::DataArrayData* pTuple_);

	// 前のオブジェクトのIDを返す
	ModUInt64 getPrevObjectID(PageVector&	AttachNodePages_,
							  ValueFile*	ValueFile_,
							  PageVector&	AttachValuePages_) const;

	// 前のオブジェクトのIDを返す(Scan)
	ModUInt64 getPrevObjectIDScan(PageVector&	AttachNodePages_) const;

	// 前のオブジェクトのIDを返す(Search)
	ModUInt64
		getPrevObjectIDSearch(PageVector&	AttachNodePages_) const;

	// 前のオブジェクトのIDを返す
	// （先頭キーフィールドに対する単一条件でSearch）
	ModUInt64
		getPrevObjectIDBySingleCondition(
			PageVector&	AttachNodePages_) const;

	// 前のオブジェクトのIDを返す
	// （先頭キーフィールドに対するEquals条件でSearch）
	ModUInt64
		getPrevObjectIDBySingleEquals(PageVector&	AttachNodePages_) const;

	// 前のオブジェクトのIDを返す
	// （先頭キーフィールドに対するLessThan(Equals)条件でSearch）
	ModUInt64
		getPrevObjectIDBySingleLessThan(
			PageVector&	AttachNodePages_,
			const bool	ContainEquals_) const;

	// 前のオブジェクトのIDを返す
	// （先頭キーフィールドに対するGreaterThan(Equals)条件でSearch）
	ModUInt64
		getPrevObjectIDBySingleGreaterThan(
			PageVector&	AttachNodePages_,
			const bool	ContainEquals_) const;

	// 前のオブジェクトのIDを返す
	// （先頭キーフィールドに対するEqualsToNull条件でSearch）
	ModUInt64
		getPrevObjectIDBySingleEqualsToNull(
			PageVector&	AttachNodePages_) const;

	// 前のオブジェクトのIDを返す
	// （先頭キーフィールドに対する範囲指定でSearch）
	ModUInt64
		getPrevObjectIDBySpanCondition(PageVector&	AttachNodePages_) const;

	// 前のオブジェクトのIDを返す
	// （複数キーフィールドに対する複合条件でSearch）
	ModUInt64
		getPrevObjectIDByMultiCondition(PageVector&	AttachNodePages_) const;

	// フィールドの値が記録されている位置へのポインタを返す
	void* getFieldPointer(
		const PhysicalFile::Page*	ObjectPage_,
		const PhysicalFile::AreaID	ObjectAreaID_,
		int							FieldIndex_,
		const bool					IsKeyObject_) const;

	void* getFieldPointer(
		const PhysicalFile::Page&	ObjectPage_,
		const PhysicalFile::AreaID	ObjectAreaID_,
		int							FieldIndex_,
		const bool					IsKeyObject_) const;

	// 先頭キーフィールドの値が記録されている位置へのポインタを返す
	void*
		getTopKeyPointer(PhysicalFile::Page*		ObjectPage_,
						 const PhysicalFile::AreaID	ObjectAreaID_) const;

	// ファイルに記録されている各フィールドの値を読み込む
	bool getObject(const ModUInt64			ValueObjectID_,
				   ValueFile*				ValueFile_,
				   Common::DataArrayData*	ResultObject_,
				   const bool				DoProjection_,
				   PageVector&				AttachNodePages_,
				   PageVector&				AttachValuePages_) const;

	// オブジェクトの先頭フィールドにオブジェクトIDを設定する
	static void setObjectID(const ModUInt64			ObjectID_,
							Common::DataArrayData*	Object_);

	// ファイルに記録されている各キーフィールドの値を読み込む
	void readKey(const ModUInt64		ValueObjectID_,
				 ValueFile*				ValueFile_,
				 Common::DataArrayData*	ResultObject_,
				 int&					iElement_,
				 const bool				DoProjection_,
				 PageVector&			AttachNodePages_,
				 PageVector&			AttachValuePages_) const;

	// ファイルに記録されている各キーフィールドの値を読み込む
	// （代表キーオブジェクトがノーマルキーオブジェクト）
	void readKey(PhysicalFile::Page*	DirectKeyObjectPage_,
				 const void*			DirectKeyObjectAreaTop_,
				 Common::DataArrayData*	ResultObject_,
				 int&					iElement_,
				 const bool				DoProjection_,
				 PageVector&			AttachNodePages_) const;

	// キー情報に記録されているキーフィールドの値を読み込む
	void readSimpleKey(const PhysicalFile::PageID	KeyInfoPageID_,
					   const ModUInt32				KeyInfoIndex_,
					   Common::DataArrayData*		ResultObject_,
					   int&							iElement_,
					   const bool					DoProjection_,
					   PageVector&					AttachNodePages_) const;

#ifdef OBSOLETE
	// 固定長フィールドの値を読み込む
	// Common::Data::copyの仕様が変わった都合で引数を修正
	const char* readFixedField(const int		FildIndex_,
							   const char*		FieldTop_,
							   //Common::Data*&	Field_) const;
							   Common::Data::Pointer&	Field_) const;
#endif //OBSOLETE

	// 固定長フィールドの値を読み込む
	static const char*
		readFixedField(const Common::DataType::Type	FieldDataType_,
					   const char*					FieldTop_,
					   //Common::Data*&				Field_);
					   Common::Data::Pointer&		Field_);
			
	// オブジェクトが記録されている物理ページをアタッチする
	static bool
		attachObjectPage(
			const Trans::Transaction*			Transaction_,
			const ModUInt64						ObjectID_,
			PhysicalFile::Page*					SrcPage_,
			PhysicalFile::Page*&				ObjectPage_,
			const Buffer::Page::FixMode::Value	FixMode_,
			const bool							CatchMemoryExhaust_,
			PageVector&							AttachPages_);

	// 既に生成されている物理ページ記述子を返す
	static PhysicalFile::Page*
		getAttachedPage(PageVector&					AttachPages_,
						const PhysicalFile::PageID	PageID_);

	// 物理ページ記述子を生成する
	static PhysicalFile::Page*
		attachPage(const Trans::Transaction*			Transaction_,
				   PhysicalFile::File*					PhysicalFile_,
				   PhysicalFile::PageID					PageID_,
				   const Buffer::Page::FixMode::Value	FixMode_,
				   const bool							CatchMemoryExhaust_,
				   PageVector&							AttachPages_);

	// 物理ページ記述子を破棄する
	void
		detachPage(
			PageVector&									AttachPages_,
			PhysicalFile::Page*&						Page_,
			const PhysicalFile::Page::UnfixMode::Value	UnfixMode_,
			const bool									SavePage_) const;

	// 物理ページ記述子を破棄する
	static void
		detachPage(
			PhysicalFile::File*							PhysicalFile_,
			PageVector&									AttachPages_,
			PhysicalFile::Page*&						Page_,
			const PhysicalFile::Page::UnfixMode::Value	UnfixMode_,
			const bool									SavePage_);

	// ページベクターにキャッシュされている
	// すべての物理ページ記述子を破棄する
	static void
		detachPageAll(
			PhysicalFile::File*							PhysicalFile_,
			PageVector&									AttachPages_,
			const PhysicalFile::Page::UnfixMode::Value	UnfixMode_,
			const bool									SavePage_);

	// detachPageAll を Node/Value 同時に行う
	static void detachPageAll(
		PhysicalFile::File*							NodePhysicalFile_,
		PageVector&									NodeAttachPages_,
		PhysicalFile::File*							ValuePhysicalFile_,
		PageVector&									ValueAttachPages_,
		const PhysicalFile::Page::UnfixMode::Value	UnfixMode_,
		const bool									SavePage_)
	{
		File::detachPageAll( NodePhysicalFile_, NodeAttachPages_, UnfixMode_,SavePage_);
		File::detachPageAll(ValuePhysicalFile_,ValueAttachPages_, UnfixMode_,SavePage_);
	}

	//本当にデタッチ（アンフィックス）してしまう
	void discardPage(PhysicalFile::Page* pPage_) const;
	void discardPages(PhysicalFile::Page* leafPage_, PhysicalFile::Page* parentNodePage_,PhysicalFile::Page* nextLeafPage_=0) const;

	// MemoryExhaust の発生をしらべ、必要ならデタッチ
	void checkMemoryExhaust(PhysicalFile::Page* pPage_) const;
	void checkMemoryExhaust(PhysicalFile::Page* leafPage_, PhysicalFile::Page* parentNodePage_,PhysicalFile::Page* nextLeafPage_=0) const;

	// ページベクターにキャッシュされている
	// すべての物理ページ記述子を破棄し、
	// ページの内容をアタッチ前の状態に戻す
	static void recoverPageAll(const Trans::Transaction&	Transaction_,
							   PhysicalFile::File*			PhysicalFile_,
							   PageVector&					AttachPages_);
	// recoverPageAll を Node/Value 同時に行う
	inline void recoverPageAll( PageVector&					NodeAttachPages_
	                           ,PageVector&					ValueAttachPages_);

	// ページ識別子ベクターにキャッシュされている
	// すべての物理ページ識別子が示す物理ページを解放する
	static void freePageAll(const Trans::Transaction&	Transaction_,
							PhysicalFile::File*			PhysicalFile_,
							PageIDVector&				PageIDs_);
	// freePageAll を Node/Value 同時に行う
	inline void freePageAll( PageIDVector&					NodePageIDs_
	                        ,PageIDVector&					ValuePageIDs_);

	// ページ識別子ベクターにキャッシュされている
	// すべての物理ページ識別が示す物理ページを再利用する
	static void reusePageAll(const Trans::Transaction&	Transaction_,
							 PhysicalFile::File*		PhysicalFile_,
							 PageIDVector&				PageIDs_);
	// reusePageAll を Node/Value 同時に行う
	inline void reusePageAll( PageIDVector&					NodePageIDs_
	                        ,PageIDVector&					ValuePageIDs_);

	// 次のキー情報が記録されているリーフページを返す
	PhysicalFile::Page*
		getNextKeyInformationPage(NodePageHeader&	LeafPageHeader_,
								  PageVector&		AttachNodePages_) const;

	// リーフページ内の次のキー情報へ移動する
	bool assignNextKeyInformation(PhysicalFile::Page*&	LeafPage_,
								  PageVector&			AttachNodePages_,
								  NodePageHeader&		LeafPageHeader_,
								  KeyInformation&		KeyInfo_) const;

	// 前のキー情報が記録されているリーフページを返す
	PhysicalFile::Page*
		getPrevKeyInformationPage(NodePageHeader&	LeafPageHeader_,
								  PageVector&		AttachNodePages_) const;

	// リーフページ内の前のキー情報へ移動する
	bool assignPrevKeyInformation(PhysicalFile::Page*&	LeafPage_,
								  PageVector&			AttachNodePages_,
								  NodePageHeader&		LeafPageHeader_,
								  KeyInformation&		KeyInfo_) const;

	// ヌル値のフィールドが含まれているか調べる
	bool hasNullField(const Common::DataArrayData*	Object_) const;

	// 物理エリア先頭へのポインタを返す
	static void* getAreaTop(PhysicalFile::Page*			Page_,
							const PhysicalFile::AreaID	AreaID_);
	static void* getAreaTop(PhysicalFile::Page&			Page_,
							const PhysicalFile::AreaID	AreaID_);

	// 物理エリア先頭へのポインタを返す
	static const void* getConstAreaTop(const PhysicalFile::Page*	Page_,
									   const PhysicalFile::AreaID	AreaID_);
	static const void* getConstAreaTop(const PhysicalFile::Page&	Page_,
									   const PhysicalFile::AreaID	AreaID_);

	//
	// File_Variable.cpp
	//

	// ファイルに記録されている可変長フィールドの値を読み込む
	// Common::Data::copyの仕様が変わった都合で引数を修正
	static const char*
		readVariableField(
			const Trans::Transaction*			Transaction_,
			const Common::DataType::Type		FieldType_,
			const Os::Memory::Size				FieldMaxLen_,
			const bool							IsOutside_,
			PhysicalFile::Page*					DirectObjectPage_,
			const char*							FieldTop_,
			Common::Data::Pointer&				Field_,
			const Buffer::Page::FixMode::Value	FixMode_,
			const bool							CatchMemoryExhaust_,
			PageVector&							AttachPages_);

	// ファイルに記録されている外置きではない可変長フィールドの値を読み込む
	static void
		readInsideVariableField(
			const InsideVarFieldLen*		FieldLenReadPos_,
			const Common::DataType::Type	FieldType_,
			Common::Data::Pointer&			Field_);

	// ファイルに記録されている外置き可変長フィールドの値を読み込む
	static void
		readOutsideVariableField(
			const Trans::Transaction*			Transaction_,
			PhysicalFile::Page*					DirectObjectPage_,
			const ModUInt64						FieldObjectID_,
			const Common::DataType::Type		FieldType_,
			Common::Data::Pointer&				Field_,
			const Buffer::Page::FixMode::Value	FixMode_,
			const bool							CatchMemoryExhaust_,
			PageVector&							AttachPages_);

	// ファイルに記録されている
	// 圧縮された外置き可変長フィールドの値を読み込む
	static void
		readCompressedOutsideVariableField(
			const Trans::Transaction*			Transaction_,
			PhysicalFile::Page*					DirectObjectPage_,
			const ModUInt64						FieldObjectID_,
			char*&								FieldBuffer_,
			Os::Memory::Size&					UncompressedSize_,
			Os::Memory::Size&					CompressedSize_,
			const int							BufferIndex_,
			const Buffer::Page::FixMode::Value	FixMode_,
			const bool							CatchMemoryExhaust_,
			PageVector&							AttachPages_,
			const bool							IsRecur_ = false);

	// 圧縮されたフィールド値が記録されている
	// 外置き可変長フィールドオブジェクトかどうかを知らせる
	static bool
		isCompressedFieldObject(
			const Trans::Transaction*			Transaction_,
			PhysicalFile::Page*					DirectObjectPage_,
			const ModUInt64						FieldObjectID_,
			const Buffer::Page::FixMode::Value	FixMode_,
			const bool							CatchMemoryExhaust_,
			PageVector&							AttachPages_);

	// ファイルに記録されている外置きString型フィールドの値を読み込む
	static void
		readOutsideStringField(
			const Trans::Transaction*			Transaction_,
			PhysicalFile::Page*					DirectObjectPage_,
			const ModUInt64						FieldObjectID_,
			ModUnicodeString&					StringField_,
			const Buffer::Page::FixMode::Value	FixMode_,
			const bool							CatchMemoryExhaust_,
			PageVector&							AttachPages_);

	// ファイルに記録されている外置きBinary型フィールドの
	// フィールド長を返す
	static Os::Memory::Size
		getOutsideBinaryFieldLength(
			const Trans::Transaction*			Transaction_,
			PhysicalFile::Page*					DirectObjectPage_,
			const ModUInt64						FieldObjectID_,
			const Buffer::Page::FixMode::Value	FixMode_,
			const bool							CatchMemoryExhaust_,
			PageVector&							AttachPages_);

	// ファイルに記録されている外置きBinary型フィールドの値を読み込む
	static void
		readOutsideBinaryField(
			const Trans::Transaction*			Transaction_,
			PhysicalFile::Page*					DirectObjectPage_,
			const ModUInt64						FieldObjectID_,
			char*								BinaryBuffer_,
			const int							BufferIndex_,
			const Buffer::Page::FixMode::Value	FixMode_,
			const bool							CatchMemoryExhaust_,
			PageVector&							AttachPages_);

	// 可変長キーフィールドオブジェクトをコピーする
	ModUInt64
		copyVariableKeyField(
			PhysicalFile::Page*	DstTopNodePage_,
			const ModUInt64		SrcVariableFieldObjectID_,
			PageVector&			AttachNodePages_,
			PageIDVector&		AllocateNodePageIDs_,
			const bool			IsLeafPage_,
			const bool			IsMove_) const;

	// 外置き可変長キーフィールドの値を書き込む
	ModUInt64
		writeOutsideVariableKey(
			PhysicalFile::Page*	TopNodePage_,
			const Common::Data*	KeyField_,
			PageVector&			AttachNodePages_,
			PageIDVector&		AllocateNodePageIDs_,
			const bool			IsLeafPage_) const;

	// 圧縮されている外置き可変長キーフィールドの値を書き込む
	ModUInt64
		writeCompressedOutsideVariableKey(
			PhysicalFile::Page*		TopNodePage_,
			const char*				FieldBuffer_,
			const Os::Memory::Size	UncompressedSize_,
			const Os::Memory::Size	CompressedSize_,
			const int				BufferIndex_,
			PageVector&				AttachNodePages_,
			PageIDVector&			AllocateNodePageIDs_,
			const bool				IsLeafPage_,
			const bool				IsRecur_ = false) const;

	// 外置き可変長キーフィールドの値を書き込む
	ModUInt64
		writeOutsideVariableKey(
			PhysicalFile::Page*		TopNodePage_,
			const char*				FieldBuffer_,
			const Os::Memory::Size	FieldLength_,
			const	int				BufferIndex_,
			PageVector&				AttachNodePages_,
			PageIDVector&			AllocateNodePageIDs_,
			const bool				IsLeafPage_) const;

	// 外置きではない可変長フィールドの値を書き込む
	static char*
		writeInsideVariableField(char*					FieldTop_,
								 const Common::Data*	Field_,
								 const Os::Memory::Size	MaxLen_);
				

	// 外置き可変長キーフィールドオブジェクトが記録されている
	// 物理エリアを解放する
	void freeOutsideVariableKeyArea(const ModUInt64	FieldObjectID_,
									PageVector&		AttachNodePages_) const;

	// 可変長フィールドオブジェクトが記録されている物理エリアを解放する
	static void
		freeVariableFieldObjectArea(
			const Trans::Transaction*			Transaction_,
			const ModUInt64						VriableFieldObjectID_,
			PhysicalFile::Page*					DirectObjectPage_,
			const Buffer::Page::FixMode::Value	FixMode_,
			const bool							CatchMemoryExhaust_,
			PageVector&							AttachPages_,
			PageIDVector&						FreePageIDs_);

	//
	// File_Fetch.cpp
	//

	// Fetch検索条件でオブジェクトを検索する
	ModUInt64 fetchByKey(const ModUInt32			TreeDepth_,
						 const PhysicalFile::PageID	RootNodePageID_,
						 PageVector&				AttachNodePages_,
						 ValueFile*					ValueFile_,
						 PageVector&				AttachValuePages_);

	// Fetch検索条件と一致するキーオブジェクトが記録されている
	// 可能性があるリーフページを検索する
	PhysicalFile::Page*
		searchLeafPageForFetch(
			const ModUInt32					TreeDepth_,
			const PhysicalFile::PageID		RootNodePageID_,
			PageVector&						AttachNodePages_,
			const Common::DataArrayData*	Condition_) const;

	// Fetch検索条件と一致するキーオブジェクトが記録されている
	// 可能性がある子ノードページを検索する
	PhysicalFile::PageID
		searchChildNodePageForFetch(
			const PhysicalFile::PageID		ParentNodePageID_,
			PageVector&						AttachNodePages_,
			const Common::DataArrayData*	Condition_) const;

	// Fetch検索条件に最も近い値を持つキーオブジェクトへ辿る
	// キー情報のインデックスを返す
	int
		getKeyInformationIndexForFetch(
			PhysicalFile::Page*				KeyInfoPage_,
			const ModUInt32					UseKeyInfoNum_,
			PageVector&						AttachNodePages_,
			const Common::DataArrayData*	Condition_,
			const bool						IsLeafPage_,
			bool&							Match_) const;

	// Fetch検索条件と一致するキーオブジェクトが
	// ノードページ内に存在する可能性があるかどうかを知らせる
	PhysicalFile::Page*
		containTargetKeyObjectForFetch(
			const PhysicalFile::PageID		NodePageID_,
			PageVector&						AttachNodePages_,
			const Common::DataArrayData*	Condition_,
			const bool						IsLeafPage_) const;

	// Fetch検索条件とオブジェクトを比較する
	int
		compareToFetchCondition(
			const Common::DataArrayData*	Condition_,
			const int						ConditionStartIndex_,
			const ModUInt64					ValueObjectID_,
			PageVector&						AttachNodePages_,
			ValueFile*						ValueFile_,
			PageVector&						AttachValuePages_,
			const PhysicalFile::PageID		LeafPageID_ =
								PhysicalFile::ConstValue::UndefinedPageID,
			const ModUInt32					KeyInfoIndex_ =
													ModUInt32Max) const;

	// Fetch検索条件とキーオブジェクトを比較する
	int
		compareToFetchCondition(
			PhysicalFile::Page*				KeyInfoPage_,
			PageVector&						AttachNodePages_,
			ModUInt64						KeyObjectID_,
			const Common::DataArrayData*	Condition_,
			const int						ConditionStartIndex_ = 0) const;

	// Fetch検索条件とキーオブジェクト／バリューオブジェクトを比較する
	int
		compareToFetchCondition(
			PhysicalFile::Page*				ObjectPage_,
			PageVector&						AttachPages_,
			const PhysicalFile::AreaID		ObjectAreaID_,
			const Common::DataArrayData*	Condition_,
			const FetchHint::CompareType::Value
											FetchCompareType_,
			const bool						IsKeyObject_,
			const int						ConditionStartIndex_ = 0) const;

	// Fetch検索条件と記録されているString型のフィールド値を比較する
	int
		compareToStringFetchCondition(
			const int						FieldIndex_,
			const int						HintArrayIndex_,
			PhysicalFile::Page*				ObjectPage_,
			PageVector&						AttachPages_,
			PhysicalFile::AreaID			ObjectAreaID_,
			const Common::DataArrayData*	Condition_,
			const int						ConditionStartIndex_,
			const bool						IsKeyObject_) const;

	// 記録されている外置きではないString型のフィールド値と
	// Fetch検索条件を比較する
	int
		compareToStringFetchCondition(
			const InsideVarFieldLen*		FieldLengthPos_,
			const Common::DataArrayData*	Condition_,
			const int						ConditionStartIndex_,
			const int						HintArrayIndex_) const;

	// 外置きフィールドオブジェクトに記録されているString型の
	// フィールド値とFetch検索条件を比較する
	int
		compareToStringFetchCondition(
			PhysicalFile::Page*				ObjectIDPage_,
			PageVector&						AttachPages_,
			const ModUInt64					ObjectID_,
			const Common::DataArrayData*	Condition_,
			const int						ConditionStartIndex_,
			const int						HintArrayIndex_) const;

	// Fetch検索条件と記録されているBinary型のフィールド値を比較する
	int
		compareToBinaryFetchCondition(
			const int						FieldIndex_,
			const int						HintArrayIndex_,
			PhysicalFile::Page*				ObjectPage_,
			PageVector&						AttachValuePages_,
			PhysicalFile::AreaID			ObjectAreaID_,
			const Common::DataArrayData*	Condition_,
			const int						ConditionStartIndex_) const;

	// 記録されている外置きではないBinary型のフィールド値と
	// Fetch検索条件を比較する
	int
		compareToBinaryFetchCondition(
			const InsideVarFieldLen*		FieldLengthPos_,
			const Common::DataArrayData*	Condition_,
			const int						ConditionStartIndex_,
			const int						HintArrayIndex_) const;


	// 外置きフィールドオブジェクトに記録されているBinary型の
	// フィールド値とFetch検索条件を比較する
	int
		compareToBinaryFetchCondition(
			PhysicalFile::Page*				ObjectIDPage_,
			PageVector&						AttachValuePages_,
			const ModUInt64					ObjectID_,
			const Common::DataArrayData*	Condition_,
			const int						ConditionStartIndex_,
			const int						HintArrayIndex_) const;

	// Fetch検索条件と記録されている配列フィールドの値を比較する
	int
		compareToArrayFetchCondition(
			const int						FieldIndex_,
			const int						HintArrayIndex_,
			PhysicalFile::Page*				DirectObjectPage_,
			PageVector&						AttachValuePages_,
			const ModUInt64					ObjectID_,
			const Common::DataArrayData*	Condition_,
			const int						ConditionStartIndex_) const;

	// Fetch検索条件でオブジェクトを検索する
	ModUInt64 fetchByKeyRev(const ModUInt32				TreeDepth_,
							const PhysicalFile::PageID	RootNodePageID_,
							PageVector&					AttachNodePages_,
							ValueFile*					ValueFile_,
							PageVector&					AttachValuePages_);

#ifdef OBSOLETE // 将来に対する予約
	// 引数からオブジェクトIDを取り出し、返す
	static ModUInt64 convertToObjectID(const Common::Data* CommonData_);
#endif

	// Fetch検索条件と一致する次のオブジェクトのIDを返す
	ModUInt64 getNextObjectIDFetchByKey(PageVector&	AttachNodePages_,
										ValueFile*	ValueFile_,
										PageVector&	AttachValuePages_) const;

	// Fetch検索条件と一致する次のオブジェクトのIDを返す
	ModUInt64
		getNextObjectIDFetchByKey(
			PhysicalFile::Page*				BeforeDataObjectPage_,
			PageVector&						AttachNodePages_,
			ValueFile*						ValueFile_,
			PageVector&						AttachValuePages_,
			const ModUInt64					BeforeObjectID_,
			const Common::DataArrayData*	Condition_) const;

	// Fetch検索条件と一致する次のオブジェクトのIDを返す
	ModUInt64
		getNextObjectIDFetchByKey(
			PhysicalFile::Page*&			LeafPage_,
			KeyInformation&					KeyInfo_,
			PageVector&						AttachNodePages_,
			ValueFile*						ValueFile_,
			PageVector&						AttachValuePages_,
			const Common::DataArrayData*	Condition_) const;

	// Fetch検索条件と一致する前のオブジェクトのIDを返す
	ModUInt64 getPrevObjectIDFetchByKey(PageVector&	AttachNodePages_,
										ValueFile*	ValueFile_,
										PageVector&	AttachValuePages_) const;

	// Fetch検索条件と一致する前のオブジェクトのIDを返す
	ModUInt64
		getPrevObjectIDFetchByKey(
			PhysicalFile::Page*				BeforeDataObjectPage_,
			PageVector&						AttachNodePages_,
			ValueFile*						ValueFile_,
			PageVector&						AttachValuePages_,
			const ModUInt64					BeforeObjectID_,
			const Common::DataArrayData*	Condition_) const;

	// Fetch検索条件と一致する前のオブジェクトのIDを返す
	ModUInt64
		getPrevObjectIDFetchByKey(
			PhysicalFile::Page*&			LeafPage_,
			KeyInformation&					KeyInfo_,
			PageVector&						AttachNodePages_,
			ValueFile*						ValueFile_,
			PageVector&						AttachValuePages_,
			const Common::DataArrayData*	Condition_) const;

	// オブジェクトIDでオブジェクトを検索する
#ifdef OBSOLETE // 将来に対する予約
	ModUInt64 fetchByOID(ValueFile*		ValueFile_,
						 PageVector&	AttachValuePages_);
#endif

	//
	// File_Search.cpp
	//

	// オブジェクトを検索する
	bool search(FileInformation&	FileInfo_,
				ValueFile*		ValueFile_,
				Common::DataArrayData* pTuple_);

	// Search + Fetch でオブジェクトを検索する
	bool searchAndFetch(PageVector&				AttachNodePages_,
						ValueFile*				ValueFile_,
						PageVector&				AttachValuePages_,
						Common::DataArrayData*	ResultObject_,
						const bool				FirstGet_);

	// オブジェクトを検索する
	ModUInt64 search(FileInformation&	FileInfo_,
					 PageVector&		AttachNodePages_,
					 ValueFile*			ValueFile_,
					 PageVector&		AttachValuePages_);

	// キー値順での先頭オブジェクトのIDを返す
	ModUInt64
		getTopObjectID(FileInformation&	FileInfo_,
					   PageVector&		AttachNodePages_,
					   const bool		GetKeyObjectID_ = false) const;

	// キー値順での最終オブジェクトのIDを返す
	ModUInt64
		getLastObjectID(FileInformation&	FileInfo_,
						PageVector&			AttachNodePages_,
						const bool			GetKeyObjectID_ = false) const;

	// キー値によりオブジェクトを検索する
	ModUInt64 searchByKey(FileInformation&	FileInfo_,
						  PageVector&		AttachNodePages_) const;

	// 先頭キーフィールドへの単一条件でオブジェクトを検索する
	ModUInt64
		searchBySingleCondition(FileInformation&	FileInfo_,
								PageVector&			AttachNodePages_) const;

	// 先頭キーフィールドが検索条件と一致するオブジェクトを検索する
	ModUInt64
		searchBySingleEquals(
			const ModUInt32				TreeDepth_,
			const PhysicalFile::PageID	RootNodePageID_,
			PageVector&					AttachNodePages_) const;

	// 先頭キーフィールドが検索条件以下（または未満）の
	// オブジェクトを検索する
	ModUInt64
		searchBySingleLessThan(
			FileInformation&						FileInfo_,
			PageVector&								AttachNodePages_,
			const bool								ContainEquals_,
			const SearchHint::CompareTarget::Value	Target_ =
											SearchHint::CompareTarget::Start,
			PhysicalFile::PageID*					KeyInfoLeafPageID_ = 0,
			ModUInt32*								KeyInfoIndex_ = 0,
			ModUInt64*								KeyObjectID_ = 0) const;

	// 先頭キーフィールドが検索条件以上（または超）の
	// オブジェクトを検索する
	ModUInt64
		searchBySingleGreaterThan(
			FileInformation&		FileInfo_,
			PageVector&				AttachNodePages_,
			const bool				ContainEquals_,
			PhysicalFile::PageID*	KeyInfoLeafPageID_ = 0,
			ModUInt32*				KeyInfoIndex_ = 0,
			ModUInt64*				KeyObjectID_ = 0) const;

	// 先頭キーフィールドがヌル値のオブジェクトを検索する
	ModUInt64
		searchBySingleEqualsToNull(FileInformation&	FileInfo_,
								   PageVector&		AttachNodePages_) const;

	// 先頭キーフィールドの値が検索条件で指定された範囲内の
	// オブジェクトを検索する
	ModUInt64
		searchBySpanCondition(FileInformation&	FileInfo_,
							  PageVector&		AttachNodePages_) const;

	// 複数キーフィールドの値が検索条件で指定された複合条件と一致する
	// オブジェクトを検索する
	ModUInt64
		searchByMultiCondition(
			const ModUInt32				TreeDepth_,
			const PhysicalFile::PageID	RootNodePageID_,
			PageVector&					AttachNodePages_) const;

	// 最終検索対象キーフィールドの値と検索条件を比較する
	bool
		compareToLastCondition(const ModUInt64		KeyObjectID_,
							   PhysicalFile::Page*	KeyInfoPage_,
							   PageVector&			AttachNodePages_) const;

	// 最終検索対象キーフィールドの値と検索条件が一致するかどうかを調べる
	bool compareToLastEquals(const ModUInt64		KeyObjectID_,
							 PhysicalFile::Page*	KeyInfoPage_,
							 PageVector&			AttachNodePages_) const;

	// 最終検索対象キーフィールドの値が検索条件以下（または未満）かどうかを
	// 調べる
	bool compareToLastLessThan(const ModUInt64		KeyObjectID_,
							   PhysicalFile::Page*	KeyInfoPage_,
							   PageVector&			AttachNodePages_,
							   const bool			ContainEquals_) const;

	// 最終検索対象キーフィールドの値が検索条件以上（または超）かどうかを
	// 調べる
	bool
		compareToLastGreaterThan(
			const ModUInt64		KeyObjectID_,
			PhysicalFile::Page*	KeyInfoPage_,
			PageVector&			AttachNodePages_,
			const bool			ContainEquals_) const;

	// 最終検索対象キーフィールドの値がヌル値かどうかを調べる
	bool
		compareToLastEqualsToNull(
			const ModUInt64		KeyObjectID_,
			PhysicalFile::Page*	KeyInfoPage_,
			PageVector&			AttachNodePages_) const;

	// 最終検索対象キーフィールドの値と検索条件を比較する
	int
		compareToLastSearchCondition(
			const ModUInt64							KeyObjectID_,
			PhysicalFile::Page*						KeyInfoPage_,
			PageVector&								AttachNodePages_,
			const SearchHint::CompareTarget::Value	Target_ =
									SearchHint::CompareTarget::Start) const;

	// 先頭キーフィールドへの検索条件と一致するキーオブジェクトが
	// 記録されている可能性のあるリーフページを検索する
	PhysicalFile::Page*
		searchLeafPageForSingleCondition(
			const ModUInt32							TreeDepth_,
			const PhysicalFile::PageID				RootNodePageID_,
			PageVector&								AttachNodePages_,
			const SearchHint::CompareTarget::Value	Target_ =
									SearchHint::CompareTarget::Start) const;

	// 検索条件と一致するキーオブジェクトがノードページ内に
	// 存在する可能性があるかどうかを知らせる
	PhysicalFile::Page*
		containTargetKeyObjectForSingleCondition(
			const PhysicalFile::PageID				NodePageID_,
			PageVector&								AttachNodePages_,
			const bool								IsLeafPage_,
			const SearchHint::CompareTarget::Value	Target_ =
									SearchHint::CompareTarget::Start) const;

	// 検索条件と一致する先頭キーフィールドの値が記録されている可能性のある
	// 子ノードページを検索する
	PhysicalFile::PageID
		searchChildNodePageForSingleCondition(
			const PhysicalFile::PageID				ParentNodePageID_,
			PageVector&								AttachNodePages_,
			const SearchHint::CompareTarget::Value	Target_) const;

	// 検索条件に最も近い先頭キーフィールドの値が書かれている
	// キーオブジェクトへ辿ることができるキー情報のインデックスを返す
	int
		getKeyInformationIndexForSingleCondition(
			PhysicalFile::Page*						KeyInfoPage_,
			const ModUInt32							UseKeyInfoNum_,
			PageVector&								AttachNodePages_,
			const bool								IsLeafPage_,
			bool&									Match_,
			const SearchHint::CompareTarget::Value	Target_ =
									SearchHint::CompareTarget::Start) const;

	// 検索条件と先頭キーフィールドの値を比較する
	int
		compareToTopSearchCondition(
			PhysicalFile::Page*						KeyInfoPage_,
			PageVector&								AttachNodePages_,
			ModUInt64								KeyObjectID_,
			const SearchHint::CompareTarget::Value	Target_ =
									SearchHint::CompareTarget::Start) const;

	// 検索条件と先頭キーフィールドの値を比較する
	int
		compareToTopSearchCondition(
			PhysicalFile::Page*						KeyObjectPage_,
			PageVector&								AttachNodePages_,
			const PhysicalFile::AreaID				KeyObjectAreaID_,
			const SearchHint::CompareTarget::Value	Target_ =
									SearchHint::CompareTarget::Start) const;

	// 検索条件と先頭文字列キーフィールドの値を比較する
	int
		compareToTopStringSearchCondition(
			PhysicalFile::Page*						KeyObjectPage_,
			PageVector&								AttachNodePages_,
			const PhysicalFile::AreaID				KeyObjectAreaID_,
			const SearchHint::CompareTarget::Value	Target_ =
									SearchHint::CompareTarget::Start) const;

	// 先頭文字列キーフィールドの値が記録されている位置へのポインタを返す
	ModUnicodeChar*
		getTopStringFieldPointer(
			PhysicalFile::Page*			KeyObjectPage_,
			PhysicalFile::Page*&		OutsideObjectPage_,
			PageVector&					AttachNodePages_,
			const PhysicalFile::AreaID	KeyObjectAreaID_,
			ModSize&					NumChar_,
			bool&						IsDivide_,
			bool&						IsCompressed_) const;

	// 複合条件と一致するキーフィールドの値が記録されている可能性のある
	// リーフページを検索する
	PhysicalFile::Page*
		searchLeafPageForMultiCondition(
			const ModUInt32				TreeDepth_,
			const PhysicalFile::PageID	RootNodePageID_,
			PageVector&					AttachNodePages_) const;

	// 複合条件と一致するキーフィールドの値が記録されている可能性のある
	// 子ノードページを検索する
	PhysicalFile::PageID
		searchChildNodePageForMultiCondition(
			const PhysicalFile::PageID	ParentNodePageID_,
			PageVector&					AttachNodePages_) const;

	// 複合条件に最も近いキーフィールドの値が書かれている
	// キーオブジェクトへ辿ることができるキー情報のインデックスを返す
	int
		getKeyInformationIndexForMultiCondition(
			PhysicalFile::Page*	KeyInfoPage_,
			const ModUInt32		UseKeyInfoNum_,
			PageVector&			AttachNodePages_,
			const bool			IsLeafPage_,
			bool&				Match_) const;

	// 複合条件と一致するキーオブジェクトがノードページ内に
	// 存在する可能性があるかどうかを知らせる
	PhysicalFile::Page*
		containTargetKeyObjectForMultiCondition(
			const PhysicalFile::PageID	NodePageID_,
			PageVector&					AttachNodePages_,
			const bool					IsLeafPage_) const;

	// 複合条件とキーフィールドの値を比較する
	int
		compareToMultiSearchCondition(
			PhysicalFile::Page*						KeyInfoPage_,
			PageVector&								AttachNodePages_,
			const ModUInt64							KeyObjectID_,
			const SearchHint::CompareTarget::Value	Target_ =
									SearchHint::CompareTarget::Start) const;

	// 複合条件とキーフィールドの値を比較する
	int
		compareToMultiSearchCondition(
			PhysicalFile::Page*						KeyObjectPage_,
			PageVector&								AttachNodePages_,
			const PhysicalFile::AreaID				KeyObjectAreaID_,
			const SearchHint::CompareType::Value	CompareType_,
			const SearchHint::CompareTarget::Value	Target_ =
									SearchHint::CompareTarget::Start) const;

	// 検索条件と外置きではない文字列フィールドの値を比較する
	int
		compareToStringSearchCondition(
			const InsideVarFieldLen*				FieldLengthReadPos_,
			const int								ArrayIndex_,
			const SearchHint::CompareTarget::Value	Target_) const;

	// 検索条件と外置き文字列フィールドの値を比較する
	int
		compareToStringSearchCondition(
			PhysicalFile::Page*						FieldObjectIDPage_,
			PageVector&								AttachNodePages_,
			const ModUInt64							FieldObjectID_,
			const int								ArrayIndex_,
			const SearchHint::CompareTarget::Value	Target_ =
									SearchHint::CompareTarget::Start) const;

	// キー値によりオブジェクトを検索する
	ModUInt64 searchByKeyRev(FileInformation&	FileInfo_,
							 PageVector&		AttachNodePages_,
							 ValueFile*			ValueFile_,
							 PageVector&		AttachValuePages_) const;

	// 先頭キーフィールドへの単一条件でオブジェクトを検索する
	ModUInt64
		searchBySingleConditionRev(FileInformation&	FileInfo_,
								   PageVector&		AttachNodePages_,
								   ValueFile*		ValueFile_,
								   PageVector&		AttachValuePages_) const;

	// 先頭キーフィールドの値が検索条件と一致するオブジェクトを検索する
	ModUInt64
		searchBySingleEqualsRev(
			const ModUInt32				TreeDepth_,
			const PhysicalFile::PageID	RootNodePageID_,
			PageVector&					AttachNodePages_) const;

	// 先頭キーフィールドの値が検索条件以下（または未満）の
	// オブジェクトを検索する
	ModUInt64
		searchBySingleLessThanRev(
			FileInformation&						FileInfo_,
			PageVector&								AttachNodePages_,
			ValueFile*								ValueFile_,
			PageVector&								AttachValuePages_,
			const bool								ContainEquals_,
			const SearchHint::CompareTarget::Value	Target_ =
										SearchHint::CompareTarget::Start,
			PhysicalFile::PageID*					KeyInfoLeafPageID_ = 0,
			ModUInt32*								KeyInfoIndex_ = 0,
			ModUInt64*								KeyObjectID_ = 0) const;

	// 先頭キーフィールドの値が検索条件以上（または超）の
	// オブジェクトを検索する
	ModUInt64
		searchBySingleGreaterThanRev(
			FileInformation&		FileInfo_,
			PageVector&				AttachNodePages_,
			ValueFile*				ValueFile_,
			PageVector&				AttachValuePages_,
			const bool				ContainEquals_,
			PhysicalFile::PageID*	KeyInfoLeafPageID_ = 0,
			ModUInt32*				KeyInfoIndex_ = 0,
			ModUInt64*				KeyObjectID_ = 0) const;

	// 先頭キーフィールドの値がヌル値のオブジェクトを検索する
	ModUInt64
		searchBySingleEqualsToNullRev(
			FileInformation&	FileInfo_,
			PageVector&			AttachNodePages_) const;

	// 先頭キーフィールドの値が検索条件で指定された範囲内の
	// オブジェクトを検索する
	ModUInt64
		searchBySpanConditionRev(FileInformation&	FileInfo_,
								 PageVector&		AttachNodePages_,
								 ValueFile*			ValueFile_,
								 PageVector&		AttachValuePages_) const;

	// 複数キーフィールドの値が検索条件で指定された複合条件と一致する
	// オブジェクトを検索する
	ModUInt64
		searchByMultiConditionRev(
			const ModUInt32				TreeDepth_,
			const PhysicalFile::PageID	RootNodePageID_,
			PageVector&					AttachNodePages_) const;

	//
	// File_Insert.cpp
	//

	// オブジェクト挿入前のチェックを行う
	void insertCheck(Common::DataArrayData*	Object_) const;

	// ツリーファイルにキーフィールドを挿入する
	void insertKey(FileInformation&			FileInfo_,
				   ValueFile*				ValueFile_,
				   Common::DataArrayData*	Object_,
				   PageVector&				AttachNodePages_,
				   PageIDVector&			AllocateNodePageIDs_,
				   PageVector&				AttachValuePages_,
				   const bool				DoUniqueCheck_ = false) const;

	// オブジェクト挿入前にユニークチェックを行う
	void uniqueCheck(FileInformation&			FileInfo_,
					 ValueFile*					ValueFile_,
					 Common::DataArrayData*		Object_,
					 const PhysicalFile::PageID	LeafPageID_,
					 PageVector&				AttachNodePages_,
					 PageVector&				AttachValuePages_) const;

	// ノードページにキーフィールドを挿入する
	void
		insertKeyObject(
			FileInformation&		FileInfo_,
			ValueFile*				ValueFile_,
			const ModUInt32			NodeDepth_,
			PhysicalFile::Page*		TopNodePage_,
			Common::DataArrayData*	Object_,
			PageVector&				AttachNodePages_,
			PageIDVector&			AllocateNodePageIDs_,
			PageVector&				AttachValuePages_,
			PhysicalFile::PageID&	KeyInfoNodePageID_,
			ModUInt32&				KeyInfoIndex_,
			const bool				IsLeafPage_,
			PhysicalFile::Page*		PrevKeyInfoChildNodePage_ = 0) const;

	// 親ノードページのキーオブジェクトを更新する
	void resetParentNodeKey(PhysicalFile::Page*		ChildNodePage_,
							Common::DataArrayData*	Object_,
							const ModUInt32			ParentNodeDepth_,
							PageVector&				AttachNodePages_,
							PageIDVector&			AllocateNodePageIDs_,
							const bool				IsLeafPage_) const;

	// キーオブジェクトを更新する
	void rewriteKeyObject(const ModUInt64			KeyObjectID_,
						  Common::DataArrayData*	Object_,
						  PageVector&				AttachNodePages_,
						  PageIDVector&				AllocateNodePageIDs_,
						  const bool				IsLeafPage_) const;

	// キーオブジェクトを更新する
	void rewriteKeyObject(void*						KeyObjectAreaTop_,
						  Common::DataArrayData*	Object_,
						  PhysicalFile::Page*		KeyObjectPage_,
						  PageVector&				AttachNodePages_,
						  PageIDVector&				AllocateNodePageIDs_,
						  const bool				IsLeafPage_) const;

	// 親ノードページを更新する
	void
		resetParentNodePage(FileInformation&	FileInfo_,
							ValueFile*			ValueFile_,
							const ModUInt32		ChildNodeDepth_,
							PhysicalFile::Page*	ChildNodePage1_,
							PhysicalFile::Page*	ChildNodePage2_,
							PageVector&			AttachNodePages_,
							PageIDVector&		AllocateNodePageIDs_,
							PageVector&			AttachValuePages_,
						    const bool			ChildNodeIsLeafPage_) const;

	// 親ノードページのキーオブジェクトを更新する
	void
		rewriteParentNodeKey(
			PhysicalFile::Page*	ChildNodePage_,
			const bool			ChildNodeIsLeafPage_,
			const ModUInt32		ParentNodeTreeDepth_,
			PageVector&			AttachNodePages_,
			PageIDVector&		AllocateNodePageIDs_) const;

	// キーオブジェクトを更新する
	void rewriteKeyObject(const ModUInt64	SrcKeyObjectID_,
						  const ModUInt64	DstKeyObjectID_,
						  PageVector&		AttachNodePages_,
						  PageIDVector&		AllocateNodePageIDs_,
						  const bool		DstNodeIsLeafPage_) const;

	// キーオブジェクトを更新する
	void rewriteKeyObject(const void*			SrcKeyObjectAreaTop_,
						  void*					DstKeyObjectAreaTop_,
						  PhysicalFile::Page*	DstKeyObjectPage_,
						  PageVector&			AttachNodePages_,
						  PageIDVector&			AllocateNodePageIDs_,
						  const bool			DstNodeIsLeafPage_) const;

	// 文字列キーフィールドの値を更新する
	void rewriteStringKey(const int				KeyFieldIndex_,
						  const NullBitmap&		SrcNullBitmap_,
						  NullBitmap&			DstNullBitmap_,
						  const char*&			SrcKeyTop_,
						  char*&				DstKeyTop_,
						  PhysicalFile::Page*	DstKeyObjectPage_,
						  PageVector&			AttachNodePages_,
						  PageIDVector&			AllocateNodePageIDs_,
						  const bool			DstNodeIsLeafPage_) const;

	// ノードページにキーフィールドを挿入する
	void
		insertKeyObject(
			FileInformation&		FileInfo_,
			ValueFile*				ValueFile_,
			const ModUInt32			NodeDepth_,
			PhysicalFile::Page*		TopNodePage_,
			const ModUInt64			ObjectID_,
			PageVector&				AttachNodePages_,
			PageIDVector&			AllocateNodePageIDs_,
			PageVector&				AttachValuePages_,
			PhysicalFile::PageID&	KeyInfoNodePageID_,
			ModUInt32&				KeyInfoIndex_,
			const bool				IsLeafPage_,
			PhysicalFile::Page*		PrevKeyInfoChildNodePage_ = 0) const;

	// ノードページにキーオブジェクトを書き込む
	ModUInt64 writeKeyObject(PhysicalFile::Page*	TopNodePage_,
							 const ModUInt64		KeyObjectID_,
							 PageVector&			AttachNodePages_,
							 PageIDVector&			AllocateNodePageIDs_,
							 const bool				IsLeafPage_) const;

	// ノードページにキーオブジェクトを書き込む
	void writeKeyObject(const void*			SrcKeyObjectAreaTop_,
						void*				DstKeyObjectAreaTop_,
						PhysicalFile::Page*	DstKeyObjectPage_,
						PageVector&			AttachNodePages_,
						PageIDVector&		AllocateNodePageIDs_,
						const bool			DstNodeIsLeafPage_) const;

	// 固定長フィールドの値を書き込む
	void writeFixedField(const int		FieldIndex_,
						 const char*&	SrcFieldTop_,
						 char*&			DstFieldTop_) const;

	// 文字列型のキーフィールドの値を書き込む
	void writeStringKey(const int			FieldIndex_,
						const char*&		SrcKeyTop_,
						char*&				DstKeyTop_,
						PhysicalFile::Page*	DstKeyObjectPage_,
						PageVector&			AttachNodePages_,
						PageIDVector&		AllocateNodePageIDs_,
						const bool			DstNodeIsLeafPage_) const;

	// 親ノードページのキーオブジェクトを更新する
	void resetParentNodeKey(PhysicalFile::Page*	ChildNodePage_,
							const ModUInt64		ObjectID_,
							const ModUInt32		ParentNodeDepth_,
							PageVector&			AttachNodePages_,
							PageIDVector&		AllocateNodePageIDs_,
							const bool			IsLeafPage_) const;

	// 挿入するオブジェクトのキーフィールドの値に最も近い値が
	// 記録されているキーオブジェクトへ辿るキー情報のインデックスを返す
	int
		getKeyInformationIndexForInsert(
			PhysicalFile::Page*	KeyInfoPage_,
			const ModUInt32		UseKeyInfoNum_,
			PageVector&			AttachNodePages_,
			const ModUInt64		ObjectID_,
			const bool			IsLeafPage_,
			const bool			SearchChildNodePage_,
			PhysicalFile::Page*	PrevKeyInfoChildNodePage_ = 0) const;

	// 親ノードページとキー情報を検索する
	void
		searchKeyInformation(
			PhysicalFile::Page*		ChildNodePage_,
			PhysicalFile::Page*&	ParentNodePage_,
			ModUInt32&				KeyInfoIndex_,
			PageVector&				AttachNodePages_,
			const bool				ChildNodeIsLeafPage_) const;

	// 2つのキーオブジェクトを比較する
	int compareKeyObject(const ModUInt64	SrcKeyObjectID_,
						 const ModUInt64	DstKeyObjectID_,
						 PageVector&		AttachNodePages_) const;

	// 2つの固定長フィールドの値を比較する
	int
		compareFixedField(
			const int					FieldIndex_,
			PhysicalFile::Page*			SrcKeyObjectPage_,
			const PhysicalFile::AreaID	SrcKeyObjectAreaID_,
			PhysicalFile::Page*			DstKeyObjectPage_,
			const PhysicalFile::AreaID	DstKeyObjectAreaID_) const;

	// 2つの32ビット符号付き整数値を比較する
	static int
	compareIntegerField(const void*	SrcValue_, const void* DstValue_);

	// 2つの32ビット符号なし整数値を比較する
	static int
	compareUnsignedIntegerField(
		const void*	SrcValue_, const void* DstValue_);

	// 2つの64ビット符号付き整数値を比較する
	static int
	compareInteger64Field(const void* SrcValue_, const void* DstValue_);

	// 2つの64ビット符号なし整数値を比較する
	static int
	compareUnsignedInteger64Field(
		const void* SrcValue_, const void* DstValue_);

	// 2つの32ビット実数値を比較する
	static int
	compareFloatField(const void* SrcValue_, const void* DstValue_);

	// 2つの64ビット実数値を比較する
	static int
	compareDoubleField(const void* SrcValue_, const void* DstValue_);

	// 2つの日付を比較する
	static int
	compareDateField(const void* SrcValue_, const void* DstValue_);

	// 2つの時間を比較する
	static int
	compareTimeField(const void* SrcValue_, const void* DstValue_);

	// 2つのオブジェクトIDを比較する
	static int
	compareObjectIDField(const void* SrcValue_, const void* DstValue_);

	// 2つの文字列フィールドを比較する
	int
		compareStringField(
			const int					FieldIndex_,
			PhysicalFile::Page*			SrcKeyObjectPage_,
			const PhysicalFile::AreaID	SrcKeyObjectAreaID_,
			PhysicalFile::Page*			DstKeyObjectPage_,
			const PhysicalFile::AreaID	DstKeyObjectAreaID_,
			PageVector&					AttachNodePages_) const;

	// 2つの外置き文字列フィールドを比較する
	int compareOutsideStringField(const int				FieldIndex_,
								  const ModUInt64		SrcFieldObjectID_,
								  PhysicalFile::Page*	SrcDirectObjectPage_,
								  const ModUInt64		DstFieldObjectID_,
								  PhysicalFile::Page*	DstDirectObjectPage_,
								  PageVector&			AttachPages_) const;

	// 2つの文字列フィールドを比較する
	int compareStringField(const void*				SrcField_,
						   const Os::Memory::Size	SrcFieldLen_,
						   const void*				DstField_,
						   const Os::Memory::Size	DstFieldLen_) const;

	// ノード内の最終キー情報に記録されているオブジェクトIDを返す
	ModUInt64
		getLastObjectIDInNode(PhysicalFile::Page*	NodePage_,
							  const bool			IsLeafPage_,
							  const bool			GetValueObjectID_) const;

	// ノードページにキーオブジェクトを書き込む
	ModUInt64 
		writeKeyObject(PhysicalFile::Page*		TopNodePage_,
					   Common::DataArrayData*	Object_,
					   PageVector&				AttachNodePages_,
					   PageIDVector&			AllocateNodePageIDs_,
					   const bool				IsLeafPage_) const;

	// キー値としてヌル値を書き込む
	char* writeNullKey(NullBitmap::Value*	NullBitmapTop_,
					   char*				KeyTop_,
					   const int			FieldIndex_) const;

	// 外置きフィールドオブジェクトのIDを書き込む
	static char* writeObjectID(char*			WritePos_,
							   const ModUInt64	ObjectID_);

	// 外置きフィールドオブジェクトのIDを読み込む
	static const char* readObjectID(const char*	ReadPos_,
									ModUInt64&	ObjectID_);


	// 固定長フィールドの値を書き込む
	static char* writeFixedField(char*					FieldTop_,
								 const Common::Data*	Field_);

	// 挿入のためにキーテーブル内のキー情報をシフトする
	void shiftKeyInfoForInsert(PhysicalFile::Page*	KeyInfoPage_,
							   const ModUInt32		UseKeyInfoNum_,
							   const ModUInt32		KeyInfoIndex_,
							   const bool			IsLeafPage_,
							   ValueFile*			ValueFile_,
							   PageVector&			AttachValuePages_) const;

	// ノードの分割条件の検査
	bool isSplitCondition( const PhysicalFile::Page& KeyInfoPage_
	                      ,const NodePageHeader& pageheader_
	                      ,KeyInformation& keyInfo_
	                      ,const Common::DataArrayData* Object_ = 0
	                      ) const;

	// ノードを分割する
	bool splitNodePage(FileInformation&		FileInfo_,
					   PhysicalFile::Page*	NodePage_,
					   PageVector&			AttachNodePages_,
					   PageIDVector&		AllocateNodePageIDs_,
					   PhysicalFile::Page*&	NewNodePage_,
					   const bool			IsLeafPage_,
					   ValueFile*			ValueFile_,
					   PageVector&			AttachValuePages_,
					   const Common::DataArrayData* Object_ = 0
					   ) const;

	// キーテーブルをベクターにつむ
	void setKeyTableToVector(PhysicalFile::Page*	KeyInfoPage_,
							 const ModUInt32		KeyNumInNode_,
							 const ModUInt32		StartKeyInfoIndex_,
							 ModVector<ModUInt64>&	KeyObjectIDs_,
							 PageIDVector&			ChildNodePageIDs_,
							 ModVector<ModUInt64>&	DataObjectIDs_,
							 const bool				IsLeafPage_) const;

	// ノードページを生成する
	PhysicalFile::Page*
		createNodePage(PageVector&		AttachNodePages_,
					   PageIDVector&	AllocateNodePageIDs_,
					   const bool		IsLeafPage_) const;

	// リーフページ同士の双方向リンクをはりかえる
	void resetLeafPageLink(PhysicalFile::Page*	KeyInfoPage1_,
						   PhysicalFile::Page*	KeyInfoPage2_,
						   PageVector&			AttachNodePages_) const;

	// 親ノードページの物理ページ識別子をコピーする
	void copyParentNodePageID(PhysicalFile::Page*	KeyInfoPage1_,
							  PhysicalFile::Page*	KeyInfoPage2_,
							  PageVector&			AttachNodePages_,
							  const bool			IsLeafPage_) const;

	// キーオブジェクトを移動する
	ModUInt64 moveKey(PhysicalFile::Page*	DstTopNodePage_,
					  const ModUInt64		SrcDirectKeyObjectID_,
					  PageVector&			AttachNodePages_,
					  PageIDVector&			AllocateNodePageIDs_,
					  const bool			IsLeafPage_) const;

	// キーオブジェクトを記録する物理ページを検索する
	PhysicalFile::Page*
		searchInsertNodePage(
			PhysicalFile::Page*		TopNodePage_,
			PageVector&				AttachNodePages_,
			PageIDVector&			AllocateNodePageIDs_,
			const Os::Memory::Size	ObjectSize_,
			const bool				IsLeafPage_) const;

	// ノードページヘッダの親ノードページの物理ページ識別子を更新する
	void
		resetNodePageHeaderParentNodePageID(
			const PhysicalFile::PageID	ParentNodePageID_,
			const PhysicalFile::PageID	TopChildNodePageID_,
			PageVector&					AttachNodePages_,
			const bool					ChildNodeIsLeafPage_) const;

	// ノードページ内の物理エリアを再配置する
	void compactionNodePage(PhysicalFile::Page*	TopNodePage_,
							PageVector&			AttachNodePages_,
							const bool			IsLeafPage_) const;

	// キー値を記録するためのリーフページを検索する
	PhysicalFile::Page*
		searchLeafPageForInsert(
			const ModUInt32				TreeDepth_,
			const PhysicalFile::PageID	RootNodePageID_,
			Common::DataArrayData*		Object_,
			PageVector&					AttachNodePages_) const;

	// キー値を記録するための子ノードページを検索する
	PhysicalFile::PageID
		searchChildNodePageForInsert(
			const PhysicalFile::PageID	ParentNodePageID,
			Common::DataArrayData*		Object_,
			PageVector&					AttachNodePages_) const;

	// 挿入するオブジェクトのキーフィールドの値に最も近い値が
	// 記録されているキーオブジェクトへ辿るキー情報のインデックスを返す
	int
		getKeyInformationIndexForInsert(
			PhysicalFile::Page*		KeyInfoPage_,
			const ModUInt32			UseKeyInfoNum_,
			PageVector&				AttachNodePages_,
			Common::DataArrayData*	Object_,
			const bool				IsLeafPage_,
			const bool				SearchChildNodePage_,
			PhysicalFile::Page*		PrevKeyInfoChildNodePage_ = 0) const;

	// リーフページのキー情報にオブジェクトIDを書き込む
	void setObjectID(PageVector&	AttachNodePages_) const;

	// オブジェクトの先頭フィールドにオブジェクトIDを設定する
	void setObjectID(Common::DataArrayData*	Object_) const;

	//
	// File_Expunge.cpp
	//

	// オブジェクトを削除するための環境が整っているかをチェックする
	void expungeCheck(const Common::DataArrayData*	SearchCondition_) const;

	// リーフページのキーオブジェクトを削除する
	void deleteLeafPageKey(FileInformation&	FileInfo_,
						   PageVector&		AttachNodePages_,
						   PageIDVector&	AllocateNodePageIDs_,
						   PageIDVector&	FreeNodePageIDs_,
						   ValueFile*		ValueFile_,
						   PageVector&		AttachValuePages_) const;

	// キーオブジェクトを削除する
	void deleteKey(FileInformation&		FileInfo_,
				   PhysicalFile::Page*&	NodePage_,
				   const ModUInt32		KeyInfoIndex_,
				   const ModUInt32		NodeDepth_,
				   PageVector&			AttachNodePages_,
				   PageIDVector&		AllocateNodePageIDs_,
				   PageIDVector&		FreeNodePageIDs_,
				   ValueFile*			ValueFile_,
				   PageVector&			AttachValuePages_,
				   const bool			IsLeafPage_) const;

	// マージ可能なノードページかを調べる
	bool isPossibleMerge(PhysicalFile::Page*	NodePage_,
						 const bool				IsLeafPage_) const;

	// 親ノードページが同じかどうかを調べる
	bool
		isSameParentNodePage(
			NodePageHeader&	SrcChildNodePageHeader_,
			NodePageHeader&	DstChildNodePageHeader_) const;

	// マージ先のノードページを検索する
	PhysicalFile::Page*
		searchMergeNodePage(PhysicalFile::Page*	SrcNodePage_,
							NodePageHeader&		SrcNodePageHeader_,
							const bool			IsLeafPage_,
							PageVector&			AttachNodePages_,
							bool&				IsPrev_) const;

	// キーオブジェクトを削除する
	void deleteKeyObject(FileInformation&		FileInfo_,
						 PhysicalFile::Page*&	NodePage_,
						 const ModUInt32		KeyInfoIndex_,
						 const ModUInt32		NodeDepth_,
						 PageVector&			AttachNodePages_,
						 PageIDVector&			AllocateNodePageIDs_,
						 PageIDVector&			FreeNodePageIDs_,
						 ValueFile*				ValueFile_,
						 PageVector&			AttachValuePages_,
						 const bool				IsLeafPage_) const;

	// ノードページをマージする
	void mergeNodePage(FileInformation&		FileInfo_,
					   const ModUInt32		NodeDepth_,
					   PhysicalFile::Page*&	SrcNodePage_,
					   NodePageHeader&		SrcNodePageHeader_,
					   const bool			IsLeafPage_,
					   PageVector&			AttachNodePages_,
					   PageIDVector&		AllocateNodePageIDs_,
					   PageIDVector&		FreeNodePageIDs_,
					   ValueFile*			ValueFile_,
					   PageVector&			AttachValuePages_) const;

	// 前のノードページとマージする
	void mergeWithPrevNodePage(FileInformation&		FileInfo_,
							   const ModUInt32		NodeDepth_,
							   PhysicalFile::Page*	SrcNodePage_,
							   PhysicalFile::Page*	DstNodePage_,
							   const bool			IsLeafPage_,
							   PageVector&			AttachNodePages_,
							   PageIDVector&		AllocateNodePageIDs_,
							   PageIDVector&		FreeNodePageIDs_,
							   ValueFile*			ValueFile_,
							   PageVector&			AttachValuePages_) const;

	// 後ろのノードページとマージする
	void mergeWithNextNodePage(FileInformation&		FileInfo_,
							   const ModUInt32		NodeDepth_,
							   PhysicalFile::Page*	SrcNodePage_,
							   PhysicalFile::Page*	DstNodePage_,
							   const bool			IsLeafPage_,
							   PageVector&			AttachNodePages_,
							   PageIDVector&		AllocateNodePageIDs_,
							   PageIDVector&		FreeNodePageIDs_,
							   ValueFile*			ValueFile_,
							   PageVector&			AttachValuePages_) const;

	// 削除のためにキーテーブル内のキー情報をシフトする
	void shiftKeyInfoForExpunge(PhysicalFile::Page*	KeyInfoPage_,
								const ModUInt32		UseKeyInfoNum_,
								const ModUInt32		KeyInfoIndex_,
								const bool			IsLeafPage_,
								ValueFile*			ValueFile_,
								PageVector&			AttachValuePages_) const;

	// ノードページを解放する
	void freeNodePage(PhysicalFile::Page*&	TopNodePage_,
					  PageVector&			AttachNodePages_,
					  PageIDVector&			FreeNodePageIDs_,
					  const bool			IsLeafPage_) const;

	// キーオブジェクトが記録されている物理エリアを解放する
	void
		freeKeyObjectArea(
			PhysicalFile::Page*			DirectKeyObjectPage_,
			const PhysicalFile::AreaID	DirectKeyObjectAreaID_,
			PageVector&					AttachNodePages_,
			PageIDVector&				FreeNodePageIDs_) const;

	//
	// File_Update.cpp
	//

	// オブジェクトを更新するための環境が整っているかをチェックする
	void updateCheck(const Common::DataArrayData*	SearchCondition_,
					 Common::DataArrayData*			Object_) const;

	// キーオブジェクトの更新を伴う更新処理かをチェックする
	bool isKeyUpdate() const;

	// リーフページのキーの位置が変更される更新処理かをチェックする
	bool isNecessaryMoveKey(Common::DataArrayData*	AfterObject_,
							PageVector&				AttachNodePages_) const;

	// バリューオブジェクトの更新を伴う更新処理かをチェックする
	bool isValueUpdate() const;

	// 更新後のオブジェクトを生成する
	Common::DataArrayData*
		makeUpdateObject(Common::DataArrayData*	AfterObject_,
						 PageVector&			AttachNodePages_,
						 ValueFile*				ValueFile_,
						 PageVector&			AttachValuePages_) const;

	// 選択フィールドかどうかをチェックする
	static bool isSelected(const OpenParameter*	OpenParam_,
						   const int			FieldIndex_);

	// キーオブジェクトを更新する
	void updateKey(Common::DataArrayData*	Object_,
				   PageVector&				AttachNodePages_,
				   PageIDVector&			AllocateNodePageIDs_,
				   PageIDVector&			FreeNodePageIDs_) const;

	// キーオブジェクトを更新する
	void updateKeyObject(Common::DataArrayData*	Object_,
						 PageVector&			AttachNodePages_,
						 PageIDVector&			AllocateNodePageIDs_,
						 PageIDVector&			FreeNodePageIDs_) const;

	//
	// File_Like.cpp
	//

	// like演算子による文字列検索を行う
	ModUInt64 likeSearch(const ModUInt32			TreeDepth_,
						 const PhysicalFile::PageID	RootNodePageID_,
						 PageVector&				AttachNodePages_) const;

	// like演算子による文字列検索を行う
	ModUInt64
		likeSearchRev(const ModUInt32				TreeDepth_,
					  const PhysicalFile::PageID	RootNodePageID_,
					  PageVector&					AttachNodePages_) const;

	// like演算子による検索条件と一致するキーオブジェクトが
	// 記録されている可能性があるリーフページを検索する
	PhysicalFile::Page*
		searchLeafPageForLike(
			const ModUInt32				TreeDepth_,
			const PhysicalFile::PageID	RootNodePageID_,
			PageVector&					AttachNodePages_) const;

	// like演算子による検索条件と一致するキーオブジェクトが
	// ノードページ内に存在する可能性があるかどうかを知らせる
	PhysicalFile::Page*
		containTargetKeyObjectForLike(
			const PhysicalFile::PageID	NodePageID_,
			PageVector&					AttachNodePages_,
			const bool					IsLeafPage_) const;

	// like演算子による検索条件と先頭キーフィールドの値を比較する
	int
		compareToLikeSearchCondition(
			PhysicalFile::Page*			KeyInfoPage_,
			PageVector&					AttachNodePages_,
			const ModUInt64				KeyObjectID_) const;

	// like演算子による検索条件と一致するキーオブジェクトが
	// 記録されている可能性がある子ノードページを検索する
	PhysicalFile::PageID
		searchChildNodePageForLike(
			const PhysicalFile::PageID	ParentNodePageID_,
			PageVector&					AttachNodePages_) const;

	// like演算子による検索条件に最も近い値を持つキーオブジェクトへ辿る
	// キー情報のインデックスを返す
	int
		getKeyInformationIndexForLike(
			PhysicalFile::Page*			KeyInfoPage_,
			const ModUInt32				UseKeyInfoNum_,
			PageVector&					AttachNodePages_,
			const bool					IsLeafPage_,
			bool&						Match_) const;

	// like演算子による検索条件と一致する次のオブジェクトのIDを返す
	ModUInt64 getNextObjectIDByLike(PageVector&	AttachNodePages_) const;

	// like演算子による検索条件と一致する前のオブジェクトのIDを返す
	ModUInt64 getPrevObjectIDByLike(PageVector&	AttachNodePages_) const;

	//
	// File_Verify.cpp
	//

	// ファイル内で使用しているすべての物理ページと物理エリアを登録する
	void setUseInfo(const Trans::Transaction&		Transaction_,
					PhysicalFile::File*				TreePhysicalFile_,
					FileInformation&				FileInfo_,
					UseInfo&						TreeFileUseInfo_,
					ValueFile*						ValueFile_,
					UseInfo&						ValueFileUseInfo_,
					Admin::Verification::Progress&	Progress_) const;

	// ツリーファイル内で使用しているすべてのノードページと
	// ノードページ内で使用しているすべての物理エリアを登録する
	void
		setNodePageUseInfo(
			const Trans::Transaction&		Transaction_,
			PhysicalFile::File*				TreePhysicalFile_,
			const ModUInt32					DepthToLeaf_,
			const PhysicalFile::PageID		NodePageID_,
			UseInfo&						TreeFileUseInfo_,
			ValueFile*						ValueFile_,
			UseInfo&						ValueFileUseInfo_,
			Admin::Verification::Progress&	Progress_) const;

	// キーオブジェクトのために使用している
	// ノードページと物理エリアを登録する
	void setKeyUseInfo(const Trans::Transaction&		Transaction_,
					   PhysicalFile::File*				TreePhysicalFile_,
					   const ModUInt64					KeyObjectID_,
					   UseInfo&							TreeFileUseInfo_,
					   Admin::Verification::Progress&	Progress_) const;

	// 外置き可変長フィールドオブジェクトのために使用している
	// 物理ページと物理エリアを登録する
	static void
		setOutsideVariableFieldUseInfo(
			const Trans::Transaction&		Transaction_,
			PhysicalFile::File*				PhysicalFile_,
			PhysicalFile::Page*				DirectObjectPage_,
			const ModUInt64					FieldObjectID_,
			UseInfo&						UseInfo_,
			Admin::Verification::Progress&	Progress_);

	// 物理ファイルマネージャに、
	// Ｂ＋木ファイル内で使用しているすべての
	// 物理ページと物理エリアを通知する
	void
		notifyUsePageAndArea(
			const Trans::Transaction&		Transaction_,
			PhysicalFile::File*				PhysicalFile_,
			UseInfo&						UseInfo_,
			Admin::Verification::Progress&	Progress_) const;

	// Ｂ＋ファイル内の整合性検査を行う
	void checkBtreeFile(FileInformation&				FileInfo_,
						ValueFile*						ValueFile_,
						Admin::Verification::Progress&	Progress_);

	// ファイルバージョンの確認
	void checkFileVersion(FileInformation&					FileInfo_,
						  Admin::Verification::Progress&	Progress_);

	// リーフページのキー総数検査
	void checkLeafKeyNum(FileInformation&				FileInfo_,
						 const ModUInt64				ObjectNum_,
						 Admin::Verification::Progress&	Progress_);

	// 先頭オブジェクトの確認
	void checkTopObject(FileInformation&				FileInfo_,
						const ModUInt64					ObjectNum_,
						Admin::Verification::Progress&	Progress_);

	// 最終オブジェクトの確認
	void checkLastObject(FileInformation&				FileInfo_,
						 ValueFile*						ValueFile_,
						 const ModUInt64				ObjectNum_,
						 Admin::Verification::Progress&	Progress_);

	// ルートノードページの一貫性検査
	void checkRootNode(FileInformation&					FileInfo_,
					   const ModUInt64					ObjectNum_,
					   Admin::Verification::Progress&	Progress_);

	// 子ノードページの代表キー検査
	void checkDelegateKey(FileInformation&					FileInfo_,
						  const ModUInt64					ObjectNum_,
						  Admin::Verification::Progress&	Progress_);

	// 子ノードページの代表キー検査
	void checkDelegateKey(const ModUInt32					NodeDepth_,
						  const PhysicalFile::PageID		NodePageID_,
						  PageVector&						AttachNodePages_,
						  Admin::Verification::Progress&	Progress_);

	// リーフページのリンク検査
	void checkLeafLink(FileInformation&					FileInfo_,
					   Admin::Verification::Progress&	Progress_);

	// オブジェクトのユニーク性検査
	void checkObjectUnique(FileInformation&					FileInfo_,
						   ValueFile*						ValueFile_,
						   const ModUInt64					ObjectNum_,
						   Admin::Verification::Progress&	Progress_);

	// キーオブジェクトの値を比較する
	bool
		compareKeyObjectForVerify(
			const PhysicalFile::PageID	SrcLeafPageID_,
			const ModUInt32				SrcKeyInfoIndex_,
			const PhysicalFile::PageID	DstLeafPageID_,
			const ModUInt32				DstKeyInfoIndex_,
			PageVector&					AttachNodePages_) const;

	// オブジェクトの値を比較する
	bool compareObjectForVerify(PhysicalFile::File*	PhysicalFile_,
								const ModUInt64		SrcObjectID_,
								const ModUInt64		DstObjectID_,
								const bool			IsKeyObject_,
								PageVector&			AttachPages_) const;

	// オブジェクトの値を比較する
	bool
		compareObjectForVerify(
			PhysicalFile::Page*			SrcObjectPage_,
			const PhysicalFile::AreaID	SrcObjectAreaID_,
			PhysicalFile::Page*			DstObjectPage_,
			const PhysicalFile::AreaID	DstObjectAreaID_,
			const bool					IsKeyObject_,
			PageVector&					AttachPages_) const;

	// 固定長フィールドの値を比較する
	bool
		compareFixedFieldForVerify(
			const int					FieldIndex_,
			PhysicalFile::Page*			SrcObjectPage_,
			const PhysicalFile::AreaID	SrcObjectAreaID_,
			PhysicalFile::Page*			DstObjectPage_,
			const PhysicalFile::AreaID	DstObjectAreaID_,
			const bool					IsKeyObject_) const;

	// 配列フィールドの値を比較する
	bool
		compareArrayFieldForVerify(
			const int					FieldIndex_,
			PhysicalFile::Page*			SrcObjectPage_,
			const PhysicalFile::AreaID	SrcObjectAreaID_,
			PhysicalFile::Page*			DstObjectPage_,
			const PhysicalFile::AreaID	DstObjectAreaID_,
			PageVector&					AttachValuePages_) const;

	// ヌル値の要素が含まれているか調べる
	static bool hasNullElement(const Common::DataArrayData&	ArrayField_);

	// 配列フィールドの値を比較する
	bool
		compareArrayFieldForVerify(
			const Common::DataType::Type	ElementDataType_,
			PhysicalFile::Page*				SrcDirectObjectPage_,
			const ModUInt64					SrcFieldObjectID_,
			PhysicalFile::Page*				DstDirectObjectPage_,
			const ModUInt64					DstFieldObjectID_,
			PageVector&						AttachValuePages_) const;

	
	// 可変長フィールドの値を比較する
	bool
		compareVariableFieldForVerify(
			const int					FieldIndex_,
			PhysicalFile::Page*			SrcObjectPage_,
			const PhysicalFile::AreaID	SrcObjectAreaID_,
			PhysicalFile::Page*			DstObjectPage_,
			const PhysicalFile::AreaID	DstObjectAreaID_,
			const bool					IsKeyObject_,
			PageVector&					AttachPages_) const;

	// 可変長フィールドの値を比較する
	bool
		compareVariableFieldForVerify(
			const Common::DataType::Type	FieldType_,
			const void*						SrcField_,
			const Os::Memory::Size			SrcFieldLen_,
			const void*						DstField_,
			const Os::Memory::Size			DstFieldLen_) const;

	// 文字列フィールドの値を読み込む
	void readStringField(const void*			Field_,
						 const Os::Memory::Size	FieldLen_,
						 Common::StringData&	StringField_) const;

	// 外置き可変長フィールドの値を比較する
	bool
		compareOutsideVariableFieldForVerify(
			const int			FieldIndex_,
			const ModUInt64		SrcFieldObjectID_,
			PhysicalFile::Page*	SrcDirectObjectPage_,
			const ModUInt64		DstFieldObjectID_,
			PhysicalFile::Page*	DstDirectObjectPage_,
			PageVector&			AttachPages_) const;

	//
	// File_SimpleKey_Insert.cpp
	//

	// ツリーファイルにキーフィールドを挿入する
	void
		insertSimpleKey(
			FileInformation&		FileInfo_,
			ValueFile*				ValueFile_,
			Common::DataArrayData*	Object_,
			PageVector&				AttachNodePages_,
			PageIDVector&			AllocateNodePageIDs_,
			PageVector&				AttachValuePages_,
			const bool				DoUniqueCheck_ = false) const;

	// ノードページにキーフィールドを挿入する
	void
		insertSimpleKey(
			FileInformation&		FileInfo_,
			ValueFile*				ValueFile_,
			const ModUInt32			NodeDepth_,
			PhysicalFile::Page*		TopNodePage_,
			Common::DataArrayData*	Object_,
			PageVector&				AttachNodePages_,
			PageIDVector&			AllocateNodePageIDs_,
			PageVector&				AttachValuePages_,
			PhysicalFile::PageID&	KeyInfoNodePageID_,
			ModUInt32&				KeyInfoIndex_,
			const bool				IsLeafPage_,
			PhysicalFile::Page*		PrevKeyInfoChildNodePage_ = 0) const;

	// 親ノードページのキー値を更新する
	void
		resetParentNodeSimpleKey(
			PhysicalFile::Page*	ChildNodePage_,
			const ModUInt32		ChildNodeKeyInfoIndex_,
			const ModUInt32		ParentNodeDepth_,
			PageVector&			AttachNodePages_,
			const bool			ChildNodeIsLeafPage_) const;

	// 親ノードページを更新する
	void
		resetParentSimpleNodePage(
			FileInformation&	FileInfo_,
			ValueFile*			ValueFile_,
			const ModUInt32		ChildNodeDepth_,
			PhysicalFile::Page*	ChildNodePage1_,
			PhysicalFile::Page*	ChildNodePage2_,
			PageVector&			AttachNodePages_,
			PageIDVector&		AllocateNodePageIDs_,
			PageVector&			AttachValuePages_,
			const bool			ChildNodeIsLeafPage_) const;

	// 親ノードページのキー値を更新する
	void
		rewriteParentNodeSimpleKey(
			PhysicalFile::Page*	ChildNodePage_,
			const bool			ChildNodeIsLeafPage_,
			const ModUInt32		ParentNodeDepth_,
			PageVector&			AttachNodePages_) const;

	// ノードページにキーフィールドを挿入する
	void
		insertSimpleKey(
			FileInformation&			FileInfo_,
			ValueFile*					ValueFile_,
			const ModUInt32				NodeDepth_,
			PhysicalFile::Page*			TopNodePage_,
			const NullBitmap::Value*	SrcNullBitmapTop_,
			PageVector&					AttachNodePages_,
			PageIDVector&				AllocateNodePageIDs_,
			PageVector&					AttachValuePages_,
			PhysicalFile::PageID&		KeyInfoNodePageID_,
			ModUInt32&					KeyInfoIndex_,
			const bool					IsLeafPage_,
			PhysicalFile::Page*			PrevKeyInfoChildNodePage_ = 0) const;

	// 親ノードページのキー値を更新する
	void
		resetParentNodeSimpleKey(
			PhysicalFile::Page*			ChildNodePage_,
			const NullBitmap::Value*	SrcNullBitmapTop_,
			const ModUInt32				NodeDepth_,
			PageVector&					AttachNodePages_,
			const bool					IsLeafPage_) const;

	// ノードページにキー値を書き込む
	void writeSimpleKey(PhysicalFile::Page*			NodePage_,
						const ModUInt32				KeyInfoIndex_,
						const NullBitmap::Value*	SrcNullBitmapTop_,
						const bool					IsLeafPage_) const;

	// 異なる領域に記録されているキー値に最も近い値が
	// 記録されているキー情報のインデックスを返す
	int
		getKeyInformationIndexForSimpleKeyInsert(
			PhysicalFile::Page*			KeyInfoPage_,
			const ModUInt32				UseKeyInfoNum_,
			PageVector&					AttachNodePages_,
			const NullBitmap::Value*	SrcNullBitmapTop_,
			const bool					IsLeafPage_,
			const bool					SearchChildNodePage_,
			PhysicalFile::Page*			PrevKeyInfoChildNodePage_
																= 0) const;

	// キー値を比較する
	int compareSimpleKey(const NullBitmap::Value*	SrcNullBitmapTop_,
						 const NullBitmap::Value*	DstNullBitmapTop_) const;


	// ノードページにキー値を書き込む
	void writeSimpleKey(PhysicalFile::Page*				NodePage_,
						const ModUInt32					KeyInfoIndex_,
						const Common::DataArrayData*	Object_,
						const bool						IsLeafPage_) const;

	// 挿入のためにキーテーブル内のキー情報をシフトする
	void
		shiftKeyInfoForSimpleKeyInsert(
			PhysicalFile::Page*	KeyInfoPage_,
			const ModUInt32		UseKeyInfoNum_,
			const ModUInt32		KeyInfoIndex_,
			const bool			IsLeafPage_,
			ValueFile*			ValueFile_,
			PageVector&			AttachValuePages_) const;

	// ノードを分割する
	bool splitSimpleKeyTable(FileInformation&		FileInfo_,
							 PhysicalFile::Page*	KeyInfoPage_,
							 PageVector&			AttachNodePages_,
							 PageIDVector&			AllocateNodePageIDs_,
							 PhysicalFile::Page*&	NewKeyInfoPage_,
							 const bool				IsLeafPage_,
							 ValueFile*				ValueFile_,
							 PageVector&			AttachValuePages_) const;

	// オブジェクト挿入前にユニークチェックを行う
	void
		uniqueCheckSimpleKey(
			FileInformation&			FileInfo_,
			ValueFile*					ValueFile_,
			Common::DataArrayData*		Object_,
			const PhysicalFile::PageID	LeafPageID_,
			PageVector&					AttachNodePages_,
			PageVector&					AttachValuePages_) const;

	// キー値を記録するためのリーフページを検索する
	PhysicalFile::Page*
		searchLeafPageForSimpleKeyInsert(
			const ModUInt32				TreeDepth_,
			const PhysicalFile::PageID	RootNodePageID_,
			PageVector&					AttachNodePages_) const;

	// キー値を記録するための子ノードページを検索する
	PhysicalFile::PageID
		searchChildNodePageForSimpleKeyInsert(
			const PhysicalFile::PageID	ParentNodePageID_,
			PageVector&					AttachNodePages_) const;

	// 挿入するオブジェクトのキーフィールドの値に最も近い
	// 値が記録されているキー情報のインデックスを返す
	int
		getKeyInformationIndexForSimpleKeyInsert(
			PhysicalFile::Page*	KeyInfoPage_,
			const ModUInt32		UseKeyInfoNum_,
			PageVector&			AttachNodePages_,
			const bool			IsLeafPage_,
			const bool			SearchChildNodePage_,
			PhysicalFile::Page*	PrevKeyInfoChildNodePage_ = 0) const;

	// 親ノードページとキー情報を検索する
	void
		searchKeyInformationSimpleKey(
			PhysicalFile::Page*		ChildNodePage_,
			PhysicalFile::Page*&	ParentNodePage_,
			ModUInt32&				KeyInfoIndex_,
			PageVector&				AttachNodePages_,
			const bool				ChildNodeIsLeafPage_) const;

	//
	// File_SimpleKey_Search.cpp
	//

	// 先頭キーフィールドの値が検索条件と一致するオブジェクトを検索する
	ModUInt64
		searchBySingleEqualsSimpleKey(
			const ModUInt32				TreeDepth_,
			const PhysicalFile::PageID	RootNodePageID_,
			PageVector&					AttachNodePages_) const;

	// 先頭キーフィールドの値が検索条件以下（または未満）の
	// オブジェクトを検索する
	ModUInt64
		searchBySingleLessThanSimpleKey(
			FileInformation&						FileInfo_,
			PageVector&								AttachNodePages_,
			const bool								ContainEquals_,
			const SearchHint::CompareTarget::Value	Target_ =
										SearchHint::CompareTarget::Start,
			PhysicalFile::PageID*					KeyInfoLeafPageID_ = 0,
			ModUInt32*								KeyInfoIndex_ = 0) const;

	// 先頭キーフィールドの値が検索条件以上（または超）の
	// オブジェクトを検索する
	ModUInt64
		searchBySingleGreaterThanSimpleKey(
			FileInformation&		FileInfo_,
			PageVector&				AttachNodePages_,
			const bool				ContainEquals_,
			PhysicalFile::PageID*	KeyInfoLeafPageID_ = 0,
			ModUInt32*				KeyInfoIndex_ = 0) const;

	// 先頭キーフィールドの値がヌル値のオブジェクトを検索する
	ModUInt64
		searchBySingleEqualsToNullSimpleKey(
			FileInformation&	FileInfo_,
			PageVector&			AttachNodePages_) const;

	// 先頭キーフィールドの値が検索条件で指定された範囲内の
	// オブジェクトを検索する
	ModUInt64
		searchBySpanConditionSimpleKey(
			FileInformation&	FileInfo_,
			PageVector&			AttachNodePages_) const;

	// 複数キーフィールドの値が検索条件で指定された複合条件と一致する
	// オブジェクトを検索する
	ModUInt64
		searchByMultiConditionSimpleKey(
			const ModUInt32				TreeDepth_,
			const PhysicalFile::PageID	RootNodePageID_,
			PageVector&					AttachNodePages_) const;

	// 先頭キーフィールドの値が検索条件と一致するオブジェクトを検索する
	ModUInt64
		searchBySingleEqualsRevSimpleKey(
			const ModUInt32				TreeDepth_,
			const PhysicalFile::PageID	RootNodePageID_,
			PageVector&					AttachNodePages_) const;

	// 先頭キーフィールドの値が検索条件以下（または未満）の
	// オブジェクトを検索する
	ModUInt64
		searchBySingleLessThanRevSimpleKey(
			FileInformation&						FileInfo_,
			PageVector&								AttachNodePages_,
			ValueFile*								ValueFile_,
			PageVector&								AttachValuePages_,
			const bool								ContainEquals_,
			const SearchHint::CompareTarget::Value	Target_ =
										SearchHint::CompareTarget::Start,
			PhysicalFile::PageID*					KeyInfoLeafPageID_ = 0,
			ModUInt32*								KeyInfoIndex_ = 0) const;

	// 先頭キーフィールドの値が検索条件以上（または超）の
	// オブジェクトを検索する
	ModUInt64
		searchBySingleGreaterThanRevSimpleKey(
			FileInformation&		FileInfo_,
			PageVector&				AttachNodePages_,
			ValueFile*				ValueFile_,
			PageVector&				AttachValuePages_,
			const bool				ContainEquals_,
			PhysicalFile::PageID*	KeyInfoLeafPageID_ = 0,
			ModUInt32*				KeyInfoIndex_ = 0) const;

	// 先頭キーフィールドの値がヌル値のオブジェクトを検索する
	ModUInt64
		searchBySingleEqualsToNullRevSimpleKey(
			FileInformation&	FileInfo_,
			PageVector&			AttachNodePages_) const;

	// 先頭キーフィールドの値が検索条件で指定された範囲内の
	// オブジェクトを検索する
	ModUInt64
		searchBySpanConditionRevSimpleKey(
			FileInformation&	FileInfo_,
			PageVector&			AttachNodePages_,
			ValueFile*			ValueFile_,
			PageVector&			AttachValuePages_) const;

	// 複数キーフィールドの値が検索条件で指定された複合条件と一致する
	// オブジェクトを検索する
	ModUInt64
		searchByMultiConditionRevSimpleKey(
			const ModUInt32				TreeDepth_,
			const PhysicalFile::PageID	RootNodePageID_,
			PageVector&					AttachNodePages_) const;

	// 先頭キーフィールドの値が検索条件と一致する次のオブジェクトのIDを返す
	ModUInt64
		getNextObjectIDBySingleEqualsSimpleKey(
			PageVector&	AttachNodePages_) const;

	// 先頭キーフィールドの値が検索条件以下（または未満）の
	// 次のオブジェクトのIDを返す
	ModUInt64
		getNextObjectIDBySingleLessThanSimpleKey(
			PageVector&	AttachNodePages_,
			const bool	ContainEquals_) const;

	// 先頭キーフィールドの値が検索条件以上（または超）の
	// 次のオブジェクトのIDを返す
	ModUInt64
		getNextObjectIDBySingleGreaterThanSimpleKey(
			PageVector&	AttachNodePages_,
			const bool	ContainEquals_) const;

	// 先頭キーフィールドの値がヌル値の次のオブジェクトを検索する
	ModUInt64
		getNextObjectIDBySingleEqualsToNullSimpleKey(
			PageVector&	AttachNodePages_) const;

	// 先頭キーフィールドの値が検索条件で指定された範囲内の
	// 次のオブジェクトを検索する
	ModUInt64
		getNextObjectIDBySpanConditionSimpleKey(
			PageVector&	AttachNodePages_) const;

	// 複数キーフィールドの値が検索条件で指定された複合条件と一致する
	// 次のオブジェクトのIDを返す
	ModUInt64
		getNextObjectIDByMultiConditionSimpleKey(
			PageVector&	AttachNodePages_) const;

	// 先頭キーフィールドの値が検索条件と一致する前のオブジェクトのIDを返す
	ModUInt64
		getPrevObjectIDBySingleEqualsSimpleKey(
			PageVector&	AttachNodePages_) const;

	// 先頭キーフィールドの値が検索条件以下（または未満）の
	// 前のオブジェクトのIDを返す
	ModUInt64
		getPrevObjectIDBySingleLessThanSimpleKey(
			PageVector&	AttachNodePages_,
			const bool	ContainEquals_) const;

	// 先頭キーフィールドの値が検索条件以上（または超）の
	// 前のオブジェクトのIDを返す
	ModUInt64
		getPrevObjectIDBySingleGreaterThanSimpleKey(
			PageVector&	AttachNodePages_,
			const bool	ContainEquals_) const;

	// 先頭キーフィールドの値がヌル値の前のオブジェクトを検索する
	ModUInt64
		getPrevObjectIDBySingleEqualsToNullSimpleKey(
			PageVector&	AttachNodePages_) const;

	// 先頭キーフィールドの値が検索条件で指定された範囲内の
	// 前のオブジェクトを検索する
	ModUInt64
		getPrevObjectIDBySpanConditionSimpleKey(
			PageVector&	AttachNodePages_) const;

	// 複数キーフィールドの値が検索条件で指定された複合条件と一致する
	// 前のオブジェクトのIDを返す
	ModUInt64
		getPrevObjectIDByMultiConditionSimpleKey(
			PageVector&	AttachNodePages_) const;

	// 最終検索対象キーフィールドの値と検索条件を比較する
	bool
		compareToLastCondition(
			const NullBitmap::Value*	NullBitmapTop_) const;

	// 最終検索対象キーフィールドの値と検索条件が一致するかどうかを調べる
	bool
		compareToLastEquals(
			const NullBitmap::Value*	NullBitmapTop_) const;

	// 最終検索対象キーフィールドの値が検索条件以下（または未満）かどうかを
	// 調べる
	bool
		compareToLastLessThan(
			const NullBitmap::Value*	NullBitmapTop_,
			const bool					ContainEquals_) const;

	// 最終検索対象キーフィールドの値が検索条件以上（または超）かどうかを
	// 調べる
	bool
		compareToLastGreaterThan(
			const NullBitmap::Value*	NullBitmapTop_,
			const bool					ContainEquals_) const;

	// 最終検索対象キーフィールドの値がヌル値かどうかを調べる
	bool
		compareToLastEqualsToNull(
			const NullBitmap::Value*	NullBitmapTop_) const;

	// 最終検索対象キーフィールドの値と検索条件を比較する
	int
		compareToLastSearchCondition(
			const NullBitmap::Value*				NullBitmapTop_,
			const SearchHint::CompareTarget::Value	Target_ =
								SearchHint::CompareTarget::Start) const;

	// 複合条件と一致するキーフィールドの値が記録されている可能性のある
	// リーフページを検索する
	PhysicalFile::Page*
		searchSimpleLeafPageForMultiCondition(
			const ModUInt32				TreeDepth_,
			const PhysicalFile::PageID	RootNodePageID_,
			PageVector&					AttachNodePages_) const;

	// 複合条件と一致するキーフィールドの値が記録されている可能性のある
	// 子ノードページを検索する
	PhysicalFile::PageID
		searchSimpleChildNodePageForMultiCondition(
			const PhysicalFile::PageID	ParentNodePageID_,
			PageVector&					AttachNodePages_) const;

	// 複合条件に最も近いキーフィールドの値が書かれているキー情報の
	// インデックスを返す
	int
		getSimpleKeyInformationIndexForMultiCondition(
			PhysicalFile::Page*	KeyInfoPage_,
			const ModUInt32		UseKeyInfoNum_,
			const bool			IsLeafPage_,
			bool&				Match_) const;


	// 複合条件と一致するキーフィールドの値が書かれているキー情報が
	// ノードページ内に存在する可能性があるかどうかを知らせる
	PhysicalFile::Page*
		containTargetSimpleKeyForMultiCondition(
			const PhysicalFile::PageID	NodePageID_,
			PageVector&					AttachNodePages_,
			const bool					IsLeafPage_) const;

	// 複合条件とキーフィールドの値を比較する
	int
		compareToMultiSearchCondition(
			const NullBitmap::Value*				NullBitmapTop_,
			const SearchHint::CompareTarget::Value	Target_ =
								SearchHint::CompareTarget::Start) const;

	// 複合条件とキーフィールドの値を比較する
	int
		compareToMultiSearchCondition(
			const NullBitmap::Value*				NullBitmapTop_,
			const SearchHint::CompareType::Value	CompareType_,
			const SearchHint::CompareTarget::Value	Target_ =
								SearchHint::CompareTarget::Start) const;

	// キー情報内に記録されているキーフィールドの値を指すポインタを返す
	const void* assignKey(const void*	TopKey_,
						  const int		KeyIndex_) const;

	// 検索条件と一致する先頭キーフィールドの値が記録されている可能性のある
	// リーフページを検索する
	PhysicalFile::Page*
		searchSimpleLeafPageForSingleCondition(
			const ModUInt32							TreeDepth_,
			const PhysicalFile::PageID				RootNodePageID_,
			PageVector&								AttachNodePages_,
			const SearchHint::CompareTarget::Value	Target_ =
								SearchHint::CompareTarget::Start) const;

	// 検索条件と一致する先頭キーフィールドの値が記録されている可能性のある
	// 子ノードページを検索する
	PhysicalFile::PageID
		searchSimpleChildNodePageForSingleCondition(
			const PhysicalFile::PageID				ParentNodePageID_,
			PageVector&								AttachNodePages_,
			const SearchHint::CompareTarget::Value	Target_ =
								SearchHint::CompareTarget::Start) const;

	// 検索条件に最も近い先頭キーフィールドの値が書かれているキー情報の
	// インデックスを返す
	int
		getSimpleKeyInformationIndexForSingleCondition(
			PhysicalFile::Page*						KeyInfoPage_,
			const ModUInt32							UseKeyInfoNum_,
			const bool								IsLeafPage_,
			bool&									Match_,
			const SearchHint::CompareTarget::Value	Target_ =
								SearchHint::CompareTarget::Start) const;

	// 検索条件と一致する先頭キーフィールドの値が書かれているキー情報が
	// ノードページ内に存在する可能性があるかどうかを知らせる
	PhysicalFile::Page*
		containTargetSimpleKeyForSingleCondition(
			const PhysicalFile::PageID				NodePageID_,
			PageVector&								AttachNodePages_,
			const bool								IsLeafPage_,
			const SearchHint::CompareTarget::Value	Target_ =
								SearchHint::CompareTarget::Start) const;

	// 検索条件と先頭キーフィールドの値を比較する
	int
		compareToTopSearchCondition(
			const NullBitmap::Value*				NullBitmapTop_,
			const SearchHint::CompareTarget::Value	Target_ =
								SearchHint::CompareTarget::Start) const;

	// Search + Fetchでオブジェクトを検索する
	bool
		searchAndFetchSimpleKey(PageVector&				AttachNodePages_,
								ValueFile*				ValueFile_,
								PageVector&				AttachValuePages_,
								Common::DataArrayData*	ResultObject_,
								const bool				FirstGet_);

	//
	// File_SimpleKey_Fetch.cpp
	//

	// Fetch検索条件でオブジェクトを検索する
	ModUInt64
		fetchBySimpleKey(const ModUInt32			TreeDepth_,
						 const PhysicalFile::PageID	RootNodePageID_,
						 PageVector&				AttachNodePages_,
						 ValueFile*					ValueFile_,
						 PageVector&				AttachValuePages_);

	// Fetch検索条件でオブジェクトを検索する
	ModUInt64
		fetchBySimpleKeyRev(const ModUInt32				TreeDepth_,
							const PhysicalFile::PageID	RootNodePageID_,
							PageVector&					AttachNodePages_,
							ValueFile*					ValueFile_,
							PageVector&					AttachValuePages_);

	// Fetch検索条件と一致するキーフィールドの値が記録されている
	// 可能性があるリーフページを検索する
	PhysicalFile::Page*
		searchSimpleLeafPageForFetch(
			const ModUInt32				TreeDepth_,
			const PhysicalFile::PageID	RootNodePageID_,
			PageVector&					AttachNodePages_) const;

	// Fetch検索条件と一致するキーフィールドの値が記録されている
	// 可能性がある子ノードページを検索する
	PhysicalFile::PageID
		searchChildSimpleNodePageForFetch(
			const PhysicalFile::PageID	ParentNodePageID_,
			PageVector&					AttachNodePages_) const;

	// Fetch検索条件に最も近いキーフィールドの値を記録している
	// キー情報のインデックスを返す
	int
		getSimpleKeyInformationIndexForFetch(
			PhysicalFile::Page*	KeyInfoPage_,
			const ModUInt32		UseKeyInfoNum_,
			const bool			IsLeafPage_,
			bool&				Match_) const;

	// Fetch検索条件と一致するキーフィールドの値が記録されている
	// キー情報が、ノードページ内に存在する可能性があるかどうかを知らせる
	PhysicalFile::Page*
		containTargetSimpleKeyForFetch(
			const PhysicalFile::PageID	NodePageID_,
			PageVector&					AttachNodePages_,
			const bool					IsLeafPage_) const;

	// Fetch検索条件と記録されているキーフィールドの値を比較する
	int
		compareToFetchCondition(
			const NullBitmap::Value*	NullBitmapTop_) const;

	// Fetch検索条件と記録されているキーフィールドの値を比較する
	int
		compareToFetchCondition(
			const NullBitmap::Value*			NullBitmapTop_,
			const FetchHint::CompareType::Value	FetchCompareType_) const;

	// Fetch検索条件と一致する次のオブジェクトのIDを返す
	ModUInt64
		getNextObjectIDFetchBySimpleKey(
			PhysicalFile::Page*				BeforeValuePage_,
			PageVector&						AttachNodePages_,
			ValueFile*						ValueFile_,
			PageVector&						AttachValuePages_,
			const ModUInt64					BeforeValueObjectID_,
			const Common::DataArrayData*	Condition_) const;

	// Fetch検索条件と一致する次のオブジェクトのIDを返す
	ModUInt64
		getNextObjectIDFetchBySimpleKey(
			PhysicalFile::Page*&			LeafPage_,
			KeyInformation&					KeyInfo_,
			PageVector&						AttachNodePages_,
			ValueFile*						ValueFile_,
			PageVector&						AttachValuePages_,
			const Common::DataArrayData*	Condition_) const;

	// Fetch検索条件と一致する前のオブジェクトのIDを返す
	ModUInt64
		getPrevObjectIDFetchBySimpleKey(
			PhysicalFile::Page*				BeforeValuePage_,
			PageVector&						AttachNodePages_,
			ValueFile*						ValueFile_,
			PageVector&						AttachValuePages_,
			const ModUInt64					BeforeValueObjectID_,
			const Common::DataArrayData*	Condition_) const;

	// Fetch検索条件と一致する前のオブジェクトのIDを返す
	ModUInt64
		getPrevObjectIDFetchBySimpleKey(
			PhysicalFile::Page*&			LeafPage_,
			KeyInformation&					KeyInfo_,
			PageVector&						AttachNodePages_,
			ValueFile*						ValueFile_,
			PageVector&						AttachValuePages_,
			const Common::DataArrayData*	Condition_) const;

	//
	// File_SimpleKey_Expunge.cpp
	//

	// キー情報を削除する
	void deleteSimpleKey(FileInformation&		FileInfo_,
						 PhysicalFile::Page*&	NodePage_,
						 const ModUInt32		KeyInfoIndex_,
						 const ModUInt32		NodeDepth_,
						 PageVector&			AttachNodePages_,
						 PageIDVector&			FreeNodePageIDs_,
						 ValueFile*				ValueFile_,
						 PageVector&			AttachValuePages_,
						 const bool				IsLeafPage_) const;

	// ノードページをマージする
	void
		mergeSimpleNodePage(
			FileInformation&		FileInfo_,
			const ModUInt32			NodeDepth_,
			PhysicalFile::Page*&	SrcNodePage_,
			NodePageHeader&			SrcNodePageHeader_,
			const bool				IsLeafPage_,
			PageVector&				AttachNodePages_,
			PageIDVector&				FreeNodePageIDs_,
			ValueFile*				ValueFile_,
			PageVector&				AttachValuePages_) const;

	// 前のノードページとマージする
	void
		mergeWithPrevSimpleNodePage(
			FileInformation&	FileInfo_,
			const ModUInt32		NodeDepth_,
			PhysicalFile::Page*	SrcNodePage_,
			PhysicalFile::Page*	DstNodePage_,
			const bool			IsLeafPage_,
			PageVector&			AttachNodePages_,
			PageIDVector&		FreeNodePageIDs_,
			ValueFile*			ValueFile_,
			PageVector&			AttachValuePages_) const;

	// 後ろのノードページとマージする
	void
		mergeWithNextSimpleNodePage(
			FileInformation&	FileInfo_,
			const ModUInt32		NodeDepth_,
			PhysicalFile::Page*	SrcNodePage_,
			PhysicalFile::Page*	DstNodePage_,
			const bool			IsLeafPage_,
			PageVector&			AttachNodePages_,
			PageIDVector&		FreeNodePageIDs_,
			ValueFile*			ValueFile_,
			PageVector&			AttachValuePages_) const;

	// 削除のためにキーテーブル内のキー情報をシフトする
	void
		shiftKeyInfoForSimpleKeyExpunge(
			PhysicalFile::Page*	NodePage_,
			const ModUInt32		UseKeyInfoNum_,
			const ModUInt32		KeyInfoIndex_,
			const bool			IsLeafPage_,
			ValueFile*			ValueFile_,
			PageVector&			AttachValuePages_) const;

	//
	// File_SimpleKey_Update.cpp
	//

	// リーフページのキーの位置が変更される更新処理かをチェックする
	bool
		isNecessaryMoveSimpleKey(
			const Common::DataArrayData*	AfterObject_,
			PageVector&						AttachNodePages_) const;

	// キー情報に記録されているキーフィールドの値を更新する
	void
		updateSimpleKey(
			const Common::DataArrayData*	Object_,
			PageVector&						AttachNodePages_) const;

	//
	// File_SimpleKey_Verify.cpp
	//

	// ツリーファイル内で使用しているすべてのノードページと
	// ノードページ内で使用しているすべての物理エリアを登録する
	void
		setSimpleNodePageUseInfo(
			const Trans::Transaction&		Transaction_,
			PhysicalFile::File*				TreePhysicalFile_,
			const ModUInt32					NodeDepth_,
			const PhysicalFile::PageID		NodePageID_,
			UseInfo&						TreeFileUseInfo_,
			ValueFile*						ValueFile_,
			UseInfo&						ValueFileUseInfo_,
			Admin::Verification::Progress&	Progress_) const;

	// 子ノードページの代表キー検査
	void
		checkDelegateSimpleKey(
			const ModUInt32					NodeDepth_,
			const PhysicalFile::PageID		NodePageID_,
			PageVector&						AttachNodePages_,
			Admin::Verification::Progress&	Progress_);

	// リーフページのリンク検査
	void checkSimpleLeafLink(FileInformation&				FileInfo_,
							 Admin::Verification::Progress&	Progress_);

	// キーフィールドの値を比較する
	bool
		compareSimpleKeyForVerify(
			const PhysicalFile::PageID	SrcLeafPageID_,
			const ModUInt32				SrcKeyInfoIndex_,
			const PhysicalFile::PageID	DstLeafPageID_,
			const ModUInt32				DstKeyInfoIndex_,
			PageVector&					AttachNodePages_) const;

	//
	// データメンバ
	//

	// トランザクション記述子
	const Trans::Transaction*		m_pTransaction;

	// 物理ファイル記述子
	PhysicalFile::File*				m_pPhysicalFile;

	// バリューファイル記述子
	ValueFile*						m_ValueFile;

	// オープンパラメータへのポインタ
	OpenParameter*					m_pOpenParameter;

	// オープンされてからクローズするまでの間、
	// ファイルの内容が更新されたかどうか
	//     true  : ファイルの内容が更新された
	//     false : ファイルの内容は更新されてない
	bool							m_Update;

	// オープン後にオブジェクトの検索が行なわれたかどうか
	//     true  : オブジェクト検索済
	//     false : オブジェクト未検索
	bool							m_Searched;

	// オブジェクトID
	ModUInt64						m_ullObjectID;

	// 巻き戻しのために記録されているオブジェクトID
	ModUInt64						m_MarkedObjectID;

	// Btree::File::fetchで指定されたFetch検索条件を保持しておく
	// Common::Data::copyの仕様が変わったので修正
	Common::Data::Pointer			m_FetchOptionData;

	// 物理ページをデタッチしても
	// 物理ファイルマネージャで物理ページ記述子を
	// キャッシュしておくかどうか
	//     true  : キャッシュしておく
	//     false : キャッシュしない
	bool							m_SavePage;

	// 物理ページをアタッチする際に指定するフィックスモード
	// B+木ファイルをオープンする際のオープンモードにより
	// 決められる。
	Buffer::Page::FixMode::Value	m_FixMode;

	// リーフページの物理ページ識別子
	mutable PhysicalFile::PageID	m_LeafPageID;

	// キー情報のインデックス
	mutable ModUInt32				m_KeyInfoIndex;

	// 例外MemoryExhaustをキャッチしたかどうか
	//     true  : キャッチした
	//     false : キャッチしていない
	bool							m_CatchMemoryExhaust;

	// 検索ヒント
	SearchHint						m_SearchHint;

	// Fetchヒント
	FetchHint						m_FetchHint;

	// ファイルパラメータ(実体で保持)
	FileParameter					m_cFileParameter;

	// ノードページ内の最大空き領域サイズ [byte]
	Os::Memory::Size				m_NodePageFreeSizeMax;

	//
	// static データメンバ
	//

	//
	// 物理ページ／エリア識別子
	//

	//
	// 可変長フィールド用情報
	//

	// 無制限可変長フィールドを示す値
	static const Os::Memory::Size	UnlimitedFieldLen; // = 0

	//
	// オブジェクトタイプ
	//

	// ノーマルオブジェクトタイプ
	static const ObjectType			NormalObjectType;           // = 0x01

	// 分割オブジェクトタイプ
	static const ObjectType			DivideObjectType;           // = 0x02

	// 配列オブジェクトタイプ
	static const ObjectType			ArrayObjectType;            // = 0x08

	// 分割配列オブジェクトタイプ
	static const ObjectType			DivideArrayObjectType;      // = 0x10

	// 圧縮オブジェクトタイプ
	static const ObjectType			CompressedObjectType;       // = 0x20

	// 分割圧縮オブジェクトタイプ
	static const ObjectType			DivideCompressedObjectType; // = 0x40

	// 代表オブジェクトタイプ
	static const ObjectType			DirectObjectType;           // = 0x80

	//
	// 記録サイズ
	//

	// ModUInt32型変数の記録サイズ [byte]
	static const Os::Memory::Size	ModUInt32ArchiveSize;

	// オブジェクトタイプの記録サイズ [byte]
	static const Os::Memory::Size	ObjectTypeArchiveSize;

	// 物理ページ識別子の記録サイズ [byte]
	static const Os::Memory::Size	PageIDArchiveSize;

	// オブジェクトIDの記録サイズ [byte]
	static const Os::Memory::Size	ObjectIDArchiveSize;

	// 外置きではない可変長フィールドのフィールド長の記録サイズ [byte]
	static const Os::Memory::Size	InsideVarFieldLenArchiveSize;

public:
	// ディレクトリを削除する
	static void rmdir(const Os::Path& path, const FileParameter* param=0);
	// エラー処理中でディレクトリを削除する
	static void rmdirOnError(const Os::Path& path, const FileParameter* param=0);

private:
	//////////////////////
	// ファイル作成遅延 //
	//////////////////////

	void substantiate();
};

//	FUNCTION private
//	Btree::File::getAreaTop -- 物理エリア先頭へのポインタを返す
//
//	NOTES
//	物理エリア先頭へのポインタを返す。
//
//	ARGUMENTS
//	PhysicalFile::Page&			Page_
//		物理ページ記述子
//	const PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	void*
//		物理エリア先頭へのポインタ
//
//	EXCEPTIONS
//	なし

// static
inline
void*
File::getAreaTop(PhysicalFile::Page& Page_,
				 const PhysicalFile::AreaID AreaID_)
{
	return Page_.operator char*() + Page_.getAreaOffset(AreaID_);
}

//	FUNCTION private
//	Btree::File::getConstAreaTop -- 物理エリア先頭へのポインタを返す
//
//	NOTES
//	物理エリア先頭へのポインタを返す。
//
//	ARGUMENTS
//	const PhysicalFile::Page&	Page_
//		物理ページ記述子
//	const PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	const void*
//		物理エリア先頭へのポインタ
//
//	EXCEPTIONS

// static
inline
const void*
File::getConstAreaTop(const PhysicalFile::Page& Page_,
					  const PhysicalFile::AreaID AreaID_)
{
	return Page_.operator const char*() + Page_.getAreaOffset(AreaID_);
}

//
//	FUNCTION private
//	Btree::File::discardPages --
//
//	NOTES
//		本当にデタッチ（アンフィックス）してしまう
//
//	ARGUMENTS
//	PhysicalFile::Page*
//		解放する物理ページの記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	[YET!]
//
inline void
File::discardPage( PhysicalFile::Page* pPage_) const
{
	// 本当にデタッチ（アンフィックス）
	// してしまい、
	// 物理ファイルマネージャが
	// キャッシュしないようにする。
	if (pPage_) {
		this->m_pPhysicalFile->detachPage(pPage_,PhysicalFile::Page::UnfixMode::NotDirty,false);
	}
}

inline void
File::discardPages( PhysicalFile::Page* leafPage_, 
                    PhysicalFile::Page* parentNodePage_,
                    PhysicalFile::Page* nextLeafPage_) const
{
	discardPage(leafPage_);
	discardPage(parentNodePage_);
	discardPage(nextLeafPage_);
}


//
//	FUNCTION private
//	Btree::File::checkMemoryExhaust --
//
//	NOTES
//		MemoryExhaust の際にデタッチ（アンフィックス）
//
//	ARGUMENTS
//	PhysicalFile::Page*
//		MemoryExhaust の際に解放する物理ページの記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	[YET!]
//
inline void
File::checkMemoryExhaust( PhysicalFile::Page* pPage_) const
{
	if (this->m_CatchMemoryExhaust)
	{
		discardPage(pPage_);
	}
}

inline void
File::checkMemoryExhaust( PhysicalFile::Page* leafPage_, 
                          PhysicalFile::Page* parentNodePage_,
                          PhysicalFile::Page* nextLeafPage_) const
{
	if (this->m_CatchMemoryExhaust)
	{
		discardPages(leafPage_,parentNodePage_,nextLeafPage_);
	}
}

_SYDNEY_BTREE_END
_SYDNEY_END

#endif // __SYDNEY_BTREE_FILE_H

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
