// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalInterface.h -- 
// 
// Copyright (c) 2005, 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR2_LOGICALINTERFACE_H
#define __SYDNEY_VECTOR2_LOGICALINTERFACE_H

#include "Vector2/Module.h"
#include "Vector2/Data.h"
#include "Vector2/FileID.h"

#include "LogicalFile/File.h"
#include "LogicalFile/OpenOption.h"

_SYDNEY_BEGIN

namespace Common
{
	class DataArrayData;
	class IntegerArrayData;
	class Object;
	class StringArrayData;
}

namespace LogicalFile
{
	class FileID;
	class TreeNodeInterface;
}

namespace Trans
{
	class Transaction;
}

_SYDNEY_VECTOR2_BEGIN

class VectorFile;

//
//	CLASS
//	Btree2::LogicalInterface -- Vector2ファイルの論理ファイルクラス
//
//	NOTES
//	他のモジュールからVector2(以降Vector)を使う際のインターフェースである。
//
//	【Vectorとは】
//	Sydneyで使われる配列で、今のところの使われ方は以下のとおり。
//		key		ROWID
//		value	RecordのObjectID
//	データを取得するにはObjectIDが必要だが、
//	索引はROWIDしか返さないので、対応するObjectIDへの変換に使われる。
//
//	※ 索引がObjectIDを使わない理由は、
//	ROWIDは4バイトだが、RecordのObjectIDは6バイトなので、
//	索引サイズを小さくするため、だったと思う。（未確認）
//
//	【設計概要】
//	・格納データ
//	ファイルには、ROWIDを格納しない。
//	格納されたObjectIDのファイル内での位置で表現できるため。
//	ObjectID以外では、Vector2がファイルの内容を管理するデータや、
//	下位層が管理しているデータも格納される。（下位層のデータは関知しない）
//
//	・ファイルとページ
//	ファイルは、複数のページから構成されている。
//	ファイルをハードディスクからメモリに全て読み込むのは現実的ではないので、
//	データを読み書きするのに適した大きさ、ページ、に分割して処理している。
//
//	※ 各ページの詳細は PageManager.h、ページサイズは FileID.cpp を参照
//
//	・高速化とオブジェクト指向
//	上述のようにkeyはファイルに格納しないので、オブジェクト指向としては、
//	ここでkeyを除いたデータを作成して、それをVectorFileに渡すべきだが、
//	高速化を優先してkeyもそのまま渡すことにする。
//
//	【コマンド概要】
//	・LogicalInterface/~LogicalInterface
//	各SQL文の実行時にコンストラクトし、終了時にデストラクトする。（未確認）
//
//	・open
//	VectorFileの中身を実際に参照したり検索したりできる状態にする。
//	PageManagerを設定する。
//	(PageManagerのコンストラクタ自体はVectorFileのコンストラクタで実行済み)
//
//	・attachFile
//	openや_AutoAttachFileなどから呼ばれる。
//	VectorFileをnewする。
//	参照するにせよ検索するにせよ、VectorFileをメモリに読み込まなくては、
//	処理ができない。
//	VectorFileの状態を調べるだけならattachFileで十分。ex: getSize()
//	・detachFile
//	closeや_AutoAttachFileのデコンストラクタから呼ばれる。
//	attachFileでnewしたVectorFileをdeleteする。
//	・_AutoAttachFile
//	自動的にattachFile()とdetachFile()を呼ぶ仕組み。
//
//	・detachAllPages
//	外部（？）または、_AutoDetachPageから呼ばれる。
//	insertやgetCountなどを実行すると、ページがメモリに読み込まれる。
//	処理が終わったときは、ファイルをdeleteするように、
//	ページも下位層にページが不要になったことを伝える。
//	・_AutoDetachPage
//	自動的にdetachAllPagesを呼ぶ仕組み。
//
//	・create
//	"create table"を実行した時に呼ばれる。
//	FileIDをcreateする。
//	Vectorは索引が使われなければ必要ないが、常に作成される。
//	ただし、実際に作成されるのは、最初のinsertが実行される時。
//
//	・insert
//	最初のinsertの時に、VectorFileをcreateする。
//	渡されるROWIDは、常に新しいもので、
//	delete(expunge)によって空いたROWIDは使い回されない。
//
//	・isMounted
//	すでにファイルが作成されているかどうか。
//	つまり過去に少なくとも1件インサートされたかどうかを調べる。
//	1件でもインサートされると実際にファイルが作られる。
//	アンマウントされたかどうかを調べるわけではない。
//	アンマウントされた状態でこの関数は呼ばれない。
//
//	・update
//	[?] どういう状況で使われる？
//
//	・getSearchParameter
//	木構造を解析して、検索に必要なオープンオプションを設定する。
//	木構造は、Type, Value, Operand, Optionからなる。
//	- SCAN
//	オープンオプションは未設定。
//	- FETCH
//	Type==Fetch
//		Option[0]
//			Operand[0] Type==Field, Value==0
//		Option[1] 未使用
//	- Search
//	詳しくはCondition.hを参照
//
//	・getProjectionParameter
//	木構造を解析して、指定フィールドをオープンオプションに設定する。
//
//	・getUpdateParameter
//	現在、Vectorは一部のフィールドだけでなく、そもそもupdateしないので使わない。
//
//	・getSortParameter
//	[?] 現在、Vectorはキーフィールドでソートされているのはわかっているので使わない？
//
//
//	【その他】
//	・LogicalFile::FileID
//	文字通りVectorFileを識別するもの。
//	LogicalFileのコンストラクタで渡される唯一の引数で、
//	VectorFileのコンストラクタにも渡される。
//	Vector2::FileIDはこれを継承している。
//
//	・オープンモード
//	Read,Search
//		検索に使う
//	Update
//		挿入と削除に使う。
//		Updateと名づけられているが、ベクターには更新がない。
//	Initialize
//		使われないらしい。
//	[?] Lobはreadのみ設定。
//	[?] Btree2はreadとそれ以外。
//
//	【今後の拡張】
//	1. 複数のvalueを持つこともできるようにする。
//	(ただし、どのvalueも固定長であること。)
//	つまりRecordのデータのうち、よく使うデータとあまり使わないデータを
//	分割して管理し、どのデータにアクセスするか指定できるようになる。
//	この分割管理によってキャッシュの使用効率があがる。
//
//	2. getSearchParameterで全件取得条件にfalseを返す。
//	Condition.cppで判定できるものは既にfalseを返している。
//	挿入されているkeyの最小値と最大値をPageManagerで保管しておいて、
//	検索条件が全てのkeyを含むことがわかればfalseを返す。
//	○ Recordで直接取得できる。
//	× ヘッダーページをアタッチする必要がある。
//
class SYD_VECTOR2_FUNCTION LogicalInterface : public LogicalFile::File
{
public:

	//
	//	オープンパラメータ
	//
	struct OpenOptionKey
	{
		enum Value
		{
			MaxKeyIndex = LogicalFile::OpenOption::DriverNumber::Vector,
			MinKeyIndex,
			ConditionSize,
			Reverse,
			Count
		};
	};
	
	// コンストラクタ
	LogicalInterface(const LogicalFile::FileID& cFileID_);
	// デストラクタ
	virtual ~LogicalInterface();

	//
	//	Schema Information
	//

	// ファイルIDを返す
	const LogicalFile::FileID& getFileID() const;

	// ベクターファイルサイズを返す
	ModUInt64 getSize(const Trans::Transaction& cTrans_);

	// 挿入されているオブジェクト数を返す
	ModInt64 getCount() const;

	//
	//	Query Optimization
	//

	// オブジェクト検索時のオーバヘッドを返す
	double getOverhead() const;

	// オブジェクトへアクセスする際のプロセスコストを返す
	double getProcessCost() const;

	// 検索オープンパラメータを設定する
	bool getSearchParameter(const LogicalFile::TreeNodeInterface* pCondition_,
							LogicalFile::OpenOption& cOpenOption_) const;

	// プロジェクションオープンパラメータを設定する
	bool getProjectionParameter(const LogicalFile::TreeNodeInterface* pNode_,
								LogicalFile::OpenOption& cOpenOption_) const;

	// 更新オープンパラメータを設定する
	bool getUpdateParameter(const Common::IntegerArrayData&	cUpdateFields_,
							LogicalFile::OpenOption& cOpenOption_) const;

	// ソート順パラメータを設定する
	bool getSortParameter(const Common::IntegerArrayData& cKeys_,
						  const Common::IntegerArrayData& cOrders_,
						  LogicalFile::OpenOption& cOpenOption_) const;

	//
	//	Data Manipulation
	//

	// ベクターファイルを生成する
	const LogicalFile::FileID& create(const Trans::Transaction& cTransaction_);

	// ベクターファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_);

	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool bForce_ = false) const;
	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& cTransaction_) const;

	// ベクターファイルをオープンする
	void open(const Trans::Transaction&	cTransaction_,
			  const LogicalFile::OpenOption& cOpenOption_);

	// ベクターファイルをクローズする
	void close();

	// オープンされているか
	bool isOpened() const;
	
	// 検索条件 (オブジェクトID) を設定する
	void fetch(const Common::DataArrayData* pOption_);

	// 挿入されているオブジェクトを返す
	bool get(Common::DataArrayData* pTuple_);

	// オブジェクトを挿入する
	void insert(Common::DataArrayData* pTuple_);

	// オブジェクトを更新する
	void update(const Common::DataArrayData* pKey_,
				Common::DataArrayData* pTuple_);

	// オブジェクトを削除する
	void expunge(const Common::DataArrayData* pKey_);

	// 巻き戻しの位置を記録する
	void mark();

	// 記録した位置に戻る
	void rewind();

	// カーソルをリセットする
	void reset();

	// 比較
	bool equals(const Common::Object* pOther_) const;

	// 同期を取る
	void sync(const Trans::Transaction& cTransaction_,
			  bool& bIncomplete_, bool& bModified_);

	//
	// Utility
	//

	// ファイルを移動する
	void move(const Trans::Transaction&	cTransaction_,
			  const Common::StringArrayData& cArea_);

	// ラッチ不要なオペレーションを返す
	LogicalFile::File::Operation::Value	getNoLatchOperation();

	// ファイルを識別するための文字列を返す
	ModUnicodeString toString() const;

	//
	// 運用管理のためのメソッド
	//

	// ベクターファイルをマウントする
	const LogicalFile::FileID& mount(const Trans::Transaction& cTransaction_);
	// ベクターファイルをアンマウントする
	const LogicalFile::FileID& unmount(const Trans::Transaction& cTransaction_);

	// ベクターファイルをフラッシュする
	void flush(const Trans::Transaction& cTransaction_);

	// ベクターファイルに対してバックアップ開始を通知する
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_);

	// ベクターファイルに対してバックアップ終了を通知する
	void endBackup(const Trans::Transaction& cTransaction_);

	// ベクターファイルを障害回復する
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	//
	// 整合性検査のためのメソッド
	//

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				const unsigned int	uiTreatment_,
				Admin::Verification::Progress& cProgress_);

	// 以下publicであるが、外部には公開していないメソッド

	// Fileをattachする
	void attachFile();
	// Fileをdetachする
	void detachFile();
	// Fileがattachされているかどうか
	bool isAttached() const;

	// すべてのページをdetachする new!
	void detachAllPages();

	// 利用不可にする new!
	void setNotAvailable();

private:
	// オープンする
	void open(const Trans::Transaction& cTransaction_,
			  LogicalFile::OpenOption::OpenMode::Value eOpenMode_);
	// 実際にファイルを作成する
	void substantiate();

	// 条件の個数を取得する
	int getConditionSize() const;
	// 条件の最小値を取得する
	ModUInt32 getMin() const;
	// 条件の最大値を取得する
	ModUInt32 getMax() const;

	// オープンオプションを設定する
	void setOpenMode(LogicalFile::OpenOption& cOpenOption_,
					 int mode_) const;

	// 昇順でオブジェクトを取得する
	bool getInAscending(Common::DataArrayData* pTuple_,
						ModUInt32& key_);
	// 降順でオブジェクトを取得する
	bool getInDescending(Common::DataArrayData* pTuple_,
						 ModUInt32& key_);
	// ビットセットでオブジェクトを取得する
	bool getByBitSet(Common::DataArrayData* pTuple_);
	
	// ファイルID
	FileID m_cFileID;
	// Vectorファイル
	VectorFile* m_pVectorFile;

	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// オープンモード
	LogicalFile::OpenOption::OpenMode::Value m_eOpenMode;

	// 更新または取得するフィールド（キーも含む）
	int m_pFieldID[Data::MaxFieldCount];
	// 更新または取得するフィールドの個数（キーも含む）
	ModSize m_uiFieldCount;
	
	// キーを取得するかどうか
	bool m_bGetKey;
	
	// 取り出し順序
	bool m_bReverse;
	// ビットセットで得るか
	bool m_bGetByBitSet;
	// countを取得するか
	// getProjectionParameterで設定すると思ってmutableにしていたが、
	// 設定されることはないのでmutableを外した。
	// getXXXParameterで得られた結果は、実際に使われるとは限らないので、
	// OpenParameterに設定しておき、そのOpenParameterで再度openが呼ばれた際に
	// メンバ変数に設定すべき。
	bool m_bGetCount;
	
	// fetchかどうか
	bool m_bFetch;

	// 初めてのgetかどうか
	bool m_bFirstGet;
	// 直前に取得したキー値
	unsigned int m_uiKeyID;

	// マークしたキー値
	unsigned int m_uiMarkID;

	// 検索で使用する最小値と最大値
	unsigned int m_uiMinKeyID;
	unsigned int m_uiMaxKeyID;
	// 検索で使用する最小値と最大値の配列
	ModVector<ModUInt32> m_vecMinKeyID;
	ModVector<ModUInt32> m_vecMaxKeyID;
	// 現在の条件
	ModUInt32 m_uiConditionID;

};

_SYDNEY_VECTOR2_END
_SYDNEY_END

#endif // __SYDNEY_VECTOR2_LOGICALINTERFACE_H

//
//	Copyright (c) 2005, 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
