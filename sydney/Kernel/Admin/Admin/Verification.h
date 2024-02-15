// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Verification.h -- 整合性検査関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2004, 2005, 2006, 2007, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_ADMIN_VERIFICATION_H
#define	__SYDNEY_ADMIN_VERIFICATION_H

#define ADMIN_RETURN_INTERMEDIATE_RESULT

#include "Admin/Module.h"

#include "Common/Object.h"
#include "Common/DataArrayData.h"
#include "Exception/Object.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN

namespace Communication
{
	class Connection;
}
namespace Server
{
	class Session;
}
namespace Statement
{
	class VerifyStatement;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_ADMIN_BEGIN

//	NAMESPACE
//	Admin::Verification -- 整合性検査に関する名前空間
//
//	NOTES

namespace Verification
{
	//	CLASS
	//	Admin::Verification::Status -- 整合性検査の結果を表すクラス
	//
	//	NOTES

	struct Status
	{
		//	ENUM
		//	Admin::Verification::Status::Value --
		//		整合性検査結果を表す値の列挙型
		//
		//	NOTES

		enum Value
		{
			// 一貫性のある
			Consistent =	0,
			// 矛盾があったが、訂正できた
			Corrected,
			// 訂正可能な矛盾である
			Correctable,
			// 訂正不能な矛盾である
			Inconsistent,
			// 中止の指示を受けた
			Interrupted,
			// エラーが発生した
			Aborted,
			// 値の数
			ValueNum
		};
	};

	//	CLASS
	//	Admin::Verification::Treatment --
	//		整合性検査で矛盾を見つけたときの処置を表すクラス
	//
	//	NOTES

	struct Treatment
	{
		//	TYPEDEF
		//	Admin::Verification::Treatment::Value --
		//		整合性検査で矛盾を見つけたときの処置を表す値の型
		//
		//	NOTES

		typedef	unsigned int	Value;
		enum
		{
			// なにもしない
			None =		0x0,
			// 可能であれば、訂正する
			Correct =	0x1,
			// 可能であれば、検査を続ける		
			Continue =	0x2,
			// 構成する論理ファイルも検査する
			Cascade =	0x4,
			// 途中経過を結果として返す
			Verbose =	0x8,
			// レコードと索引をタプル単位で検査する
			Data =		0x10,
			// マスク
			Mask =		0x1f
		};
	};

	//	CLASS
	//	Admin::Verification::Progress -- 整合性検査の経過を表すクラス
	//
	//	NOTES

	class Progress
		: public	Common::Object
	{
	public:
		//	TYPEDEF
		//	Admin::Verification::Progress::Message --
		//		詳細な説明用のメッセージを表すクラス
		//
		//	NOTES

		typedef	Exception::Object	Message;

		// コンストラクター
		Progress();
		explicit Progress(Communication::Connection& cConnection_);
		explicit Progress(const Progress& cOther_);
		// デストラクター
		~Progress();

		// += 演算子
		SYD_ADMIN_FUNCTION
		Progress&
		operator +=(const Progress& v);

		// これまでの整合性検査の結果を得る
		Status::Value
		getStatus() const;
		// 検査中のスキーマオブジェクトの名称を得る
		const ModUnicodeString&
		getSchemaObjectName() const;
		// 詳細な説明を得る
		const Common::DataArrayData&
		getDescription() const;
		// 結果を返すコネクションのオブジェクトを得る
		Communication::Connection&
		getConnection();

		// 経過は良好か
		bool
		isGood() const;

		// 検査中のスキーマオブジェクトの名称を設定する
		void
		setSchemaObjectName(const ModUnicodeString& v);
		// 詳細な説明を追加する
		SYD_ADMIN_FUNCTION
		void
		pushDescription(const char* module,
						const char* file, unsigned int line,
						const ModUnicodeString& path, Status::Value status);
		SYD_ADMIN_FUNCTION
		void
		pushDescription(const char* module,
						const char* file, unsigned int line,
						const ModUnicodeString& path, Status::Value status,
						const Message& message);

		// 返り値のColumnMetaDataをクライアントに返す
		void
		putMetaData();

	private:
		// 詳細な説明を追加する
		void
		pushDescription(const char* module,
						const char* file, unsigned int line,
						const ModUnicodeString& path, Status::Value status,
						const ModUnicodeString& message);

		// これまでの整合性検査の結果
		Status::Value		_status;
		// 検査中のスキーマオブジェクトの名称
		ModUnicodeString	_objectName;
		// 詳細な説明
		Common::DataArrayData _description;

		// クライアントに結果を返すコネクション
		Communication::Connection* m_pConnection;
	};

	// あるオブジェクトの整合性検査を行う
	SYD_ADMIN_FUNCTION
	void
	verify(Trans::Transaction& trans,
		   Server::Session* pSession_,
		   const Statement::VerifyStatement& stmt,
		   const ModUnicodeString& dbName, Progress& result);
	// データベースの整合性検査を行う
	SYD_ADMIN_FUNCTION
	void
	verifyDatabase(Trans::Transaction& trans,
				   Server::Session* pSession_,
				   const Statement::VerifyStatement& stmt,
				   const ModUnicodeString& dbName, Progress& result);
	// 表の整合性検査を行う
	SYD_ADMIN_FUNCTION
	void
	verifyTable(Trans::Transaction& trans,
				Server::Session* pSession_,
				const Statement::VerifyStatement& stmt,
				const ModUnicodeString& dbName, Progress& result);
	// 索引の整合性検査を行う
	SYD_ADMIN_FUNCTION
	void
	verifyIndex(Trans::Transaction& trans,
				Server::Session* pSession_,
				const Statement::VerifyStatement& stmt,
				const ModUnicodeString& dbName, Progress& result);
}

//	FUNCTION public
//	Admin::Verification::Progress::Progress --
//		整合性検査の経過を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Communication::Connection& cConnection_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
Verification::Progress::Progress()
	: _status(Status::Consistent), m_pConnection(0)
{}

inline
Verification::Progress::Progress(Communication::Connection& cConnection_)
	: _status(Status::Consistent), m_pConnection(&cConnection_)
{}

inline
Verification::Progress::Progress(const Progress& cOther_)
	: _status(cOther_._status), m_pConnection(cOther_.m_pConnection),
	  _objectName(cOther_._objectName)
{
	_description = cOther_._description; // ポインターのコピー
}

//	FUNCTION public
//	Admin::Verification::Progress::~Progress --
//		整合性検査の結果を表すクラスのデストラクター
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
Verification::Progress::~Progress()
{}

//	FUNCTION public
//	Admin::Verification::Progress::getStatus --
//		これまでの整合性検査の結果を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた整合性検査の結果
//
//	EXCEPTIONS
//		なし

inline
Verification::Status::Value
Verification::Progress::getStatus() const
{
	return _status;
}

//	FUNCTION public
//	Admin::Verification::Progress::getSchemaObjectName --
//		検査中のスキーマオブジェクトの名称を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたスキーマオブジェクトの名称
//
//	EXCEPTIONS
//		なし

inline
const ModUnicodeString&
Verification::Progress::getSchemaObjectName() const
{
	return _objectName;
}

//	FUNCTION public
//	Admin::Verification::Progress::getDescription -- 詳細な説明を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた詳細な説明を表す文字列
//
//	EXCEPTIONS
//		なし

inline
const Common::DataArrayData&
Verification::Progress::getDescription() const
{
	return _description;
}

//	FUNCTION public
//	Admin::Verification::Progress::getConnection -- 結果を返すコネクションのオブジェクトを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		コネクションオブジェクト
//
//	EXCEPTIONS
//		なし

inline
Communication::Connection&
Verification::Progress::getConnection()
{
	return *m_pConnection;
}

//	FUNCTION public
//	Admin::Verification::Progress::setSchemaObjectName --
//		検査中のスキーマオブジェクトの名称を設定する
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	v
//			設定する検査中のスキーマオブジェクトの名称
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
Verification::Progress::setSchemaObjectName(const ModUnicodeString& v)
{
	_objectName = v;
}

//	FUNCTION public
//	Admin::Verification::Progress::isGood -- 検査の経過は良好か
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			良好である
//		false
//			不良である
//
//	EXCEPTIONS
//		なし

inline
bool
Verification::Progress::isGood() const
{
	return getStatus() < Status::Correctable;
}

_SYDNEY_ADMIN_END
_SYDNEY_END

//	MACRO
//	_SYDNEY_VERIFY -- 整合性検査の説明を追加する
//
//	NOTES

#define	_SYDNEY_VERIFY(progress, path, status, message)						\
	(progress).pushDescription(moduleName, srcFile, __LINE__,				\
							   path, status, message)

//	MACRO
//	_SYDNEY_VERIFY_INFO --
//		整合性検査の途中経過に関する説明を追加する
//
//	NOTES

#define	_SYDNEY_VERIFY_INFO(progress, path, message, treatment)				\
	if (!(treatment & _SYDNEY::Admin::Verification::Treatment::Verbose))	\
		;																	\
	else																	\
		_SYDNEY_VERIFY(progress, path,										\
					   _SYDNEY::Admin::Verification::Status::Consistent,	\
					   message)

//	MACRO
//	_SYDNEY_VERIFY_CORRECTED --
//		整合性検査の矛盾の訂正に関する説明を追加する
//
//	NOTES

#define	_SYDNEY_VERIFY_CORRECTED(progress, path, message)					\
	_SYDNEY_VERIFY(progress, path,											\
				   _SYDNEY::Admin::Verification::Status::Corrected,			\
				   message)

//	MACRO
//	_SYDNEY_VERIFY_CORRECTABLE --
//		整合性検査の訂正可能な矛盾の発見に関する説明を追加する
//
//	NOTES

#define	_SYDNEY_VERIFY_CORRECTABLE(progress, path, message)					\
	_SYDNEY_VERIFY(progress, path,											\
				   _SYDNEY::Admin::Verification::Status::Correctable,		\
				   message)

//	MACRO
//	_SYDNEY_VERIFY_CORRECTED --
//		整合性検査の訂正不能な矛盾の発見に関する説明を追加する
//
//	NOTES

#define	_SYDNEY_VERIFY_INCONSISTENT(progress, path, message)				\
	_SYDNEY_VERIFY(progress, path,											\
				   _SYDNEY::Admin::Verification::Status::Inconsistent,		\
				   message)

//	MACRO
//	_SYDNEY_VERIFY_INTERRUPTED --
//		整合性検査の中止の指示を検知したことに関する説明を追加する
//
//	NOTES

#define	_SYDNEY_VERIFY_INTERRUPTED(progress, path, message)					\
	_SYDNEY_VERIFY(progress, path,											\
				   _SYDNEY::Admin::Verification::Status::Interrupted,		\
				   message)

//	MACRO
//	_SYDNEY_VERIFY_ABORTED --
//		整合性検査のエラーの発生したことに関する説明を追加する
//
//	NOTES

#define	_SYDNEY_VERIFY_ABORTED(progress, path, message)						\
	_SYDNEY_VERIFY(progress, path,											\
				   _SYDNEY::Admin::Verification::Status::Aborted,			\
				   message)

#endif	// __SYDNEY_ADMIN_VERIFICATION_H

//
// Copyright (c) 2001, 2004, 2005, 2006, 2007, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
