// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModOs.h -- 仮想 OS に関するクラス定義
// 
// Copyright (c) 1997, 2009, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModOs_H__
#define __ModOs_H__

//
// モジュールは汎用OSに属する。
//	したがって、エラーはModOsXXXである。すべてのクラスを
//	ModOsObjectのサブクラスとして作成する。ModOsDriver自体は
//	インスタンス化されないので無関係。
//	*** これを実装したところ、LinkedList, MemoryPoolなどに
//	ModOsDriver::Mutexを入れるところで、XXX -> OsDriver-> MemoryHandle
//	-> XXXとなり、相互参照が発生してうまくいかない。
//	OsDriverは最下層として考え、メモリハンドルの管理下からはずしてみる。

#include "ModCommonDLL.h"
#include "ModTypes.h"
#include "ModException.h"

#include "ModKanjiCode.h"		// マルチバイト文字のエンコーディング
								// 方式を表現するため

class ModCharString;
class ModUnicodeString;

//	CONST
//	ModOsBlockSizeDefault --
//		仮想 OS のテープの入出力サイズのデフォルトの大きさ(B 単位)
//
//	NOTES

const ModSize			ModOsBlockSizeDefault = 1024;

// CLASS
// ModOs -- 仮想OSクラス
//
// NOTES
//	仮想OSのI/Fを基底クラスとして規定する。
//	各OSでは、本クラスの内部クラスから派生させてModOsDriverを実装する。

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModOs
{
	friend class ModCommonInitialize;
public:
	//	ENUM
	//	ModOs::OpenFlag -- どのようにファイルをオープンするかを表す列挙型
	//
	//	NOTES
	//		どのようにファイルをオープンするか、
	//		またはオープンされたファイルがどのような性質を持つかを
	//		指定するために ModOs::File::open の第 2 引数に論理積で与える
	//
	//		fcntl.h と同じ値を持つ

	enum OpenFlag
	{
#ifdef OS_RHLINUX6_0

		readOnlyFlag =			0x0000,			// 読み出し可能、書き込み不可
		writeOnlyFlag =			0x0001,			// 書き込み可能、読み出し不可
		readWriteFlag =			0x0002,			// 読み書き可能
		appendFlag =			0x0400,			// 必ずファイルの末尾に
												// 書き込まれる
		writeThroughFlag =		0x1000,			// 書き込みが成功すると、
												// 装置に書き込まれたことが
												// 保証される
												// O_SYNC

		createFlag =			0x0040,			// ファイルがなければ作る
		truncateFlag =			0x0200,			// ファイルがあれば、
												// サイズを 0 にする
		exclusiveFlag =			0x0080,			// すでにファイルがあると
												// 作れない

		OpenFlagMask =			0x16C3			// マスク


#else	// OS_RHLINUX6_0
		readOnlyFlag =			0x0000,			// 読み出し可能、書き込み不可
		writeOnlyFlag =			0x0001,			// 書き込み可能、読み出し不可
		readWriteFlag =			0x0002,			// 読み書き可能
		appendFlag =			0x0008,			// 必ずファイルの末尾に
												// 書き込まれる
		writeThroughFlag =		0x0010,			// 書き込みが成功すると、
												// 装置に書き込まれたことが
												// 保証される
		createFlag =			0x0100,			// ファイルがなければ作る
		truncateFlag =			0x0200,			// ファイルがあれば、
												// サイズを 0 にする
		exclusiveFlag =			0x0400,			// すでにファイルがあると
												// 作れない

		OpenFlagMask =			0x071b			// マスク
#endif	// OS_RHLINUX6_0


	};

	//	ENUM
	//	ModOs::PermissionMode -- ファイルのアクセス権を表す列挙型
	//
	//	NOTES
	//		ファイルがどのようなアクセス権を持つかを指定するために
	//		ModOs::File::open の第 3 引数に論理積で与える

	enum PermissionMode
	{
		ownerReadMode =			0000400,		// 所有者が読み込み可
		ownerWriteMode =		0000200,		// 所有者が書き出し可
		ownerExecuteMode =		0000100,		// 所有者が実行可

		groupReadMode =			0000040,		// グループが読み込み可
		groupWriteMode =		0000020,		// グループが書き出し可
		groupExecuteMode =		0000010,		// グループが実行可

		otherReadMode =			0000004,		// その他が読み込み可
		otherWriteMode =		0000002,		// その他が書き出し可
		otherExecuteMode =		0000001,		// その他が実行可

		readModeMask =			0000444,		// 読み込みマスク
		writeModeMask =			0000222,		// 書き出しマスク
		executeModeMask =		0000111,		// 実行可マスク

		ownerModeMask =			0000700,		// 所有者マスク
		groupModeMask =			0000070,		// グループマスク
		otherModeMask =			0000007,		// その他マスク

		PermissionModeMask =	0000777			// マスク
	};

	//	ENUM
	//	ModOs::SeekWhence -- ファイルポインターの移動開始位置を表す列挙型
	//
	//	NOTES
	//		ファイルポインターの移動先を指定するときに、
	//		与えられた数値がどこからのオフセットかを指定するために
	//		ModOs::File::seek の第 2 引数に与える

	enum SeekWhence
	{
		seekSet =				0,				// ファイルの先頭から
		seekBegin =				0,				// 同上
		seekCurrent =			1,				// 現在位置から
		seekEnd =				2				// ファイルの末尾から
	};

	//	ENUM
	//	ModOs::AccessMode -- ファイルに対して可能な操作権を表す列挙型
	//
	//	NOTES
	//		ファイルにどのような権利を持つかを調べるときに、
	//		ModOs::File::access の第 2 引数に与える

	enum AccessMode
	{
		accessFile =			0,				// 存在確認のみ行う
		accessExecute =			1,				// 実行する権利
		accessWrite =			2,				// 書き込みする権利
		accessRead =			4				// 読み出しする権利
	};
	
	//	CLASS
	//	ModOs::File -- 仮想 OS のファイルクラス
	//
	//	NOTES
	//		ファイルの仮想 OS インターフェース

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

	class File
	{
	public:
		virtual void		create(const ModUnicodeString& path,
								   int mode, ModSize block) = 0;
		virtual void		create(const char* path,
								   int mode, ModSize block) = 0;
												// ファイルを生成する
		virtual void		open(const ModUnicodeString& path, int flag,
								 int mode, ModSize block) = 0;
		virtual void		open(const char* path, int flag,
								 int mode, ModSize block) = 0;
												// ファイルをオープンする
		virtual void		close() = 0;		// ファイルをクローズする

		virtual ModBoolean	isOpened() const = 0;
												// オープンされているかどうか
		virtual ModBoolean	isTapeDevice() const = 0;
												// テープデバイスかどうか

		virtual	ModSize		read(void* buf, ModSize size) = 0;
												// ファイルを読み出す
		virtual ModSize		write(const void* buf, ModSize size) = 0;
												// ファイルへ書き込む
		virtual ModFileOffset
							seek(ModFileOffset offset, SeekWhence whence) = 0;
												// ファイルポインターを移動する

		virtual ModFileSize	getFileSize() = 0;	// ファイルサイズを求める

		virtual const ModCharString& getFullPathName() const = 0;
		virtual const ModUnicodeString& getFullPathNameW() const = 0;
												// ファイルの絶対パス名を求める

		virtual	void		truncate(ModFileSize length) = 0;
												// ファイルサイズを変更する
		virtual	void		chmod(int mode) = 0;
												// アクセス権を変更する

//		【注意】 以下の static メソッドは ModOsDriver ごとに定義される
//
//		static ModBoolean	isDirectory(const ModUnicodeString& path);
//		static ModBoolean	isDirectory(const char* path);
//												// ディレクトリーかどうか
//		static ModBoolean	isTapeDevice(const ModUnicodeString& path);
//		static ModBoolean	isTapeDevice(const char* path);
//												// テープデバイスかどうか
//		static ModFileSize	getFileSize(const ModUnicodeString& path);
//		static ModFileSize	getFileSize(const char* path);
//												// ファイルサイズを求める
//		static ModFileSize	getFileSizeLimit();
//												// ファイルサイズの上限を得る
//		static char			getPathSeparator();
//												// パス名の区切り文字を得る
//		static void			getFullPathName(const char* src,
// 											ModCharString& dst);
//		static void			getFullPathName(const ModUnicodeString& src,
// 											ModUnicodeString& dst);
//		static void			getFullPathName(const char* src,
// 											ModUnicodeString& dst);
//												// ファイルの絶対パス名を求める
//		static ModBoolean	getParentPathName(const ModUnicodeString& src,
//											  ModUnicodeString& dst);
//		static ModBoolean	getParentPathName(const char* src,
//											  ModUnicodeString& dst);
//												// ファイルの親を求める
//		static const char*	getDefaultTapeDevice();
//												// デフォルトのテープデバイスの
//												// 絶対パス名を得る
//		static void			truncate(const ModUnicodeString& path,
//									 ModFileSize length = 0);
//		static void			truncate(const char* path, ModFileSize length = 0);
//												// ファイルサイズを変更する
//		static void			unlink(const ModUnicodeString& path);
//		static void			unlink(const char* path);
//												// ファイルを削除する
//		static void			chmod(const ModUnicodeString& path, int mode);
//		static void			chmod(const char* path, int mode);
//												// アクセス権を変更する
//		static ModBoolean	access(const ModUnicodeString& path, int amode);
//		static ModBoolean	access(const char* path, int amode);
//												// 操作する権利の有無を調べる
//		static ModBoolean	isNotFound(const ModUnicodeString& path);
//		static ModBoolean	isNotFound(const char* path);
//												// ファイルが存在しないか調べる
//		static void			rename(const ModUnicodeString& oldpath,
//								   const ModUnicodeString& newpath);
//		static void			rename(const char* oldpath, const char* newpath);
//												// ファイルの名前を変更する
//		static void			mkdir(const ModUnicodeString& path,
//								int mode =
//								ModOs::ownerReadMode | ModOs::ownerWriteMode |
//								ModOs::ownerExecuteMode,
//								ModBoolean recursive = ModFalse);
//		static void			mkdir(const char* path,
//								int mode =
//								ModOs::ownerReadMode | ModOs::ownerWriteMode |
//								ModOs::ownerExecuteMode,
//								ModBoolean recursive = ModFalse);
//												// ディレクトリーを生成する
//		static void			rmdir(const ModUnicodeString& path);
//		static void			rmdir(const char* path);
//												// ディレクトリーを削除する
//		static void			rmAll(const ModUnicodeString& path,
//								  ModBoolean forceFlag) = 0;
//		static void			rmAll(const char* path, ModBoolean forceFlag) = 0;
//												// ディレクトリー以下を削除する
//		static void			copy(const ModUnicodeString& srcPath,
//								 const ModUnicodeString& dstPath,
//								 int flag = ModOs::truncateFlag);
//		static void			copy(const char* srcPath, const char* dstPath,
//								 int flag = ModOs::truncateFlag);
//												// ファイルをコピーする
//		static void			copyAll(const char* sourcePath,
//									const char* targetPath) = 0;
//											// ディレクトリー以下をコピーする
//		static void			setNumberOfFileUnlimited();
//												// 生成可能なファイル
//												// ディスクリプターの数を
//												// 最上限に設定する
//		static void			getDirectoryEntry(const ModUnicodeString& path,
//											  ModUnicodeString*** entrys,
//											  int* n_entrys);
//		static void			getDirectoryEntry(const char* path,
//											  ModCharString*** entrys,
//											  int* n_entrys);
//											  // ディレクトリ内の
//											  // ファイル名を求める
//

#ifndef MOD_SELF_MEMORY_MANAGEMENT_OFF
		ModCommonDLL
		void*				operator new(size_t size);
#endif
	};

	//
	// ENUM
	// SocketOption -- ソケットオプション設定の種類
	// NOTES
	// ソケットのオプション設定の種類。必要そうなもののみ。
	// 
	enum SocketOption {
		none			= 0,
		
		debug			= 0x0001,	// SO_DEBUG
		reuseAddress	= 0x0002,	// SO_REUSEADDR
		keepAlive		= 0x0004,	// SO_KEEPALIVE
		broadcast		= 0x0008,	// SO_BROADCASE
		openType		= 0x0010,	// SO_OPENTYPE	(Windowsのみ, 未使用)
		readTimeout     = 0x0020,   // SO_RECVTIMEO

		only_v4			= 0x1000,	// IPV4のみ
		only_v6			= 0x2000	// IPV6のみ
	};

	//
	// CLASS
	// ModOs::Socket -- 仮想OSソケットクラス
	// NOTES
	//	ソケットを扱う仮想OSインターフェース。(TCPのみ)
	//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

	class Socket
	{
	public:
		static const ModSize hostnameLengthMax;

		virtual void open() = 0;
		virtual void close() = 0;
		virtual ModBoolean isOpened() const = 0;
		virtual void bind(int port, int mark, int option,
						  const char* interfaceName = 0) = 0;
		virtual ModCharString getpeername() = 0;
		virtual Socket* accept(int& mark) = 0;
		virtual void connect(const char* hostname, int port, int option) = 0;

		virtual ModSize read(void* buffer, ModSize size) = 0;
		virtual ModSize write(const void* buffer, ModSize size) = 0;

		virtual int select(ModSize second, ModSize millisecond) = 0;

		virtual ModBoolean isIPv6() const = 0;

#ifndef MOD_SELF_MEMORY_MANAGEMENT_OFF
		// メモリ獲得が失敗したとき例外を送出するため
		ModCommonDLL
		void* operator new(size_t size);
#endif

//		【注意】 以下の static メソッドは ModOsDriver ごとに定義される
//
//		static void			getHostname(char* hostname, ModSize size);
//												// ホスト名を得る
//
//		// ネットワークオーダーとマシン固有のバイトオーダーの間の変換
//
//	    static unsigned short hostToNetwork(unsigned short data);
//		static ModUInt32	hostToNetwork(ModUInt32 data);
//		static ModUInt64	hostToNetwork(ModUInt64 data);
//		static float		hostToNetwork(float data);
//		static double		hostToNetwork(double data);
//
//		static unsigned short networkToHost(unsigned short data);
//		static ModUInt32	networkToHost(ModUInt32 data);
//		static ModUInt64	networkToHost(ModUInt64 data);
//		static float		networkToHost(float data);
//		static double		networkToHost(double data);
	};

	//	CLASS
	//	ModOs::ThreadSpecificKey --
	//		仮想 OS のスレッド固有ストレージクラス
	//
	//	NOTES
	//		スレッド固有ストレージの仮想 OS インタフェース

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

	class ThreadSpecificKey
	{
	public:
		virtual void		setValue(void* value) = 0;
												// 値を設定する
		virtual void*		getValue() = 0;		// 設定された値を得る

#ifndef MOD_SELF_MEMORY_MANAGEMENT_OFF
		ModCommonDLL
		void*				operator new(size_t size);
#endif
	};

	//	CLASS
	//	ModOs::Thread -- 仮想 OS のスレッドクラス
	//
	//	NOTES
	//		スレッドの仮想 OS インターフェース

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

	class Thread
	{
	public:
		//	TYPEDEF
		//	ModOs::Thread::Routine -- 仮想 OS のスレッドで実行する関数の型
		//
		//	NOTES

		typedef	void*		(*Routine)(void*);

		//	CLASS
		//	ModOs::Thread::Wrapper -- 仮想OS のスレッドラッパークラス
		//
		//	NOTES
		//		生成するスレッドで実行する関数とその引数を格納する

//【注意】	ライブラリ外に公開しないクラスなので dllexport しない

		class Wrapper
		{
		public:
			Wrapper(Routine routine, void* argument, Thread* thread);
												// コンストラクター

			void			setExitStatus(void* status);
												// 実行した関数の返り値を
												// 設定する

			Routine			getRoutine() const;	// 実行する関数を得る
			void*			getArgument() const;
												// 実行する関数の引数を得る
			void*			getExitStatus() const;
												// 実行した関数の返り値を得る
			Thread*			getThread() const;	// 生成するスレッドを表す
												// 仮想 OS のスレッド
												// クラスを得る

#ifndef MOD_SELF_MEMORY_MANAGEMENT_OFF
			void*			operator new(size_t size);
#endif
		private:
			Routine			_routine;			// 実行する関数
			void*			_argument;			// 実行する関数の引数
			void*			_status;			// 実行した関数の返り値
			Thread*			_thread;			// 生成するスレッドを表す
												// 仮想 OS のスレッドクラス
		};

		//	ENUM
		//	ModOs::Thread::ExitType -- 仮想 OS のスレッドの終了状態を表す列挙型
		//
		//	NOTES

		enum ExitType {
			unknown,							// 一度も実行していないか
												// 実行中なので、不明
			normally,							// 正常終了(返り値)
			exited,								// 終了(終了コード)
			except,								// 例外が発生した
			killed								// 強制終了した(終了コード)
		};

		Thread();								// コンストラクター
		virtual ~Thread();						// デストラクター

		virtual void		create(Routine routine, void* argument) = 0;
												// スレッドを生成する
		virtual void*		join() = 0;			// スレッドの終了を待つ
		virtual void		kill() = 0;			// スレッドの強制終了する

		virtual ModBoolean	isAlive() const = 0;
												// スレッドの存在を調べる
		virtual ModThreadId	getThreadId() const = 0;
												// スレッド ID を得る
		ExitType			getExitType() const;
												// スレッドの終了状態を得る

//		【注意】 以下の static メソッドは ModOsDriver ごとに定義される
//
//		static void			sleep(ModSize millisecond);
//												// 呼び出しスレッドを
//												// 任意時間停止する
//		static void			exit(unsigned int status);
//												// 呼び出しスレッドを終了する
//
//		static ModThreadId	self();				// 呼び出しスレッドの
//												// スレッド ID を得る

#ifndef MOD_SELF_MEMORY_MANAGEMENT_OFF
		ModCommonDLL
		void*				operator new(size_t size);
#endif

	protected:
		ExitType			_exitType;			// スレッドの終了状態
	};

	//	CLASS
	//	ModOs::CriticalSection -- 仮想 OS のクリティカルセクションクラス
	//
	//	NOTES
	//		クリティカルセクションの仮想 OS インターフェース
	//		関数名は ModOs::Mutex にあわせ、
	//		動作は WIN32API のクリティカルセクション準拠の仕様とする

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

	class CriticalSection
	{
	public:
		virtual void		lock() = 0;			// 所有権を得る
		virtual ModBoolean	trylock() = 0;		// 所有権を得ようと試みる
		virtual void		unlock() = 0;		// 所有権を放棄する

		//【注意】	ModMemoryPool の初期化で必要となるため、
		//			operator new は定義しない
	};

	//	CLASS
	//	ModOs::Mutex -- 仮想 OS のミューテックスクラス
	//
	//	NOTES
	//		ミューテックスの仮想 OS インターフェース

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

	class Mutex
	{
	public:
		virtual void		lock() = 0;			// ロックする
		virtual ModBoolean	trylock() = 0;		// ロックしようと試みる
		virtual void		unlock() = 0;		// アンロックする

		//【注意】	ModMemoryPool の初期化で必要となるため、
		//			operator new は定義しない
	};

	//	CLASS
	//	ModOs::ConditionVariable -- 仮想 OS の条件変数クラス
	//
	//	NOTES
	//		条件変数の仮想 OS インタフェース
	//		動作は WIN32API のイベント準拠の仕様とする

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

	class ConditionVariable
	{
	public:
		//	ENUM
		//	ModOs::ConditionVariable::Behavior -- 条件変数のふるまいを表す
		//
		//	NOTES

		enum Behavior {
			behaviorDefault =		0x0,		// デフォルトの動作
			behaviorWakeUpAll =		0x1,		// 待ちスレッドをすべて起こす
			behaviorManualReset =	0x2,		// 明示的に非シグナル化する
												// 必要がある
			behaviorMask =			0x3			// マスク
		};

		virtual void		signal() = 0;		// シグナル状態にする

		virtual ModBoolean	wait() = 0;
		virtual ModBoolean	wait(ModUInt32 milliSecond) = 0;
												// シグナル状態になるまで待つ

		virtual void		reset() = 0;		// 非シグナル状態にする

#ifndef MOD_SELF_MEMORY_MANAGEMENT_OFF
		ModCommonDLL
		void*				operator new(size_t size);
#endif
	};

	// 文字、文字列関係
	// atoi, atof, toLower, toUpper, isAscii, isSpaceは
	// strcmp, strncmp, strlen, strcpy はModTraitで集中して扱う。
	// 詳しくはModTrait.hを参照のこと
	// 

	//	CLASS
	//	ModOs::Math -- 仮想 OS の算術演算クラス
	//
	//	NOTES
	//		算術演算関連の仮想 OS インターフェース

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

	class Math
	{
	public:
//		【注意】 以下の static メソッドは ModOsDriver ごとに定義される
//
//		static double		log(double x);		// 対数を計算する
//		static double		sqrt(double x);		// 平方根を計算する
//		static double		pow(double x, double y);
//												// 乗数を計算する
	};

	//	CLASS
	//	ModOs::Memory -- 仮想 OS のメモリー操作クラス
	//
	//	NOTES
	//		メモリー操作関連の仮想 OS インターフェース

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

	class Memory
	{
	public:
//		【注意】 以下の static メソッドは ModOsDriver ごとに定義される
//
//		static void*		copy(void* dst, const void* src, ModSize size);
//												// メモリー内容のコピー
//		static void*		move(void* dst, const void* src, ModSize size);
//												// メモリー内容の移動
//		static void*		reset(void* dst, ModSize size);
//												// メモリー内容を 0 埋めする
//		static void*		set(void* dst, int c, ModSize size);
//												// メモリー内容の
//												// 特定値による初期化
//		static int			compare(const void* l,
//									const void* r, ModSize size);
//												// メモリー内容の比較
//		static void*		alloc(ModSize size, ModBoolean noError = ModFalse);
//												// メモリーの確保
//		static void			free(void* p);		// メモリーの破棄
	};

	//	CLASS
	//	ModOs::Process -- 仮想 OS のプロセスクラス
	//
	//	NOTES
	//		プロセスの仮想 OS インターフェース

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

	class Process
	{
	public:
//		【注意】 以下の static メソッドは ModOsDriver ごとに定義される
//
//		static void			exit(int status);	// プロセスを終了する
//		static void			abort();			// プロセスを強制終了する
//
//		static void			getcwd(ModUnicodeString& buf, ModSize size);
//		static void			getcwd(char* buf, ModSize size);
//												// カレントワーキング
//												// ディレクトリーを求める
//		static void			chdir(const ModUnicodeString& path);
//		static void			chdir(const char* path);
//												// プロセスのカレントワーキング
//												// ディレクトリーを変更する
//
//		static unsigned int umask(unsigned int mask);
//												// UMASK を変更する
//
//		static void			getenv(const ModUnicodeString& name,
//								   ModUnicodeString& buf);
//		static void			getenv(const char* name, char* buf, ModSize size);
//												// ある環境変数の値を求める
//		static void			setenv(const ModUnicodeString& name,
//								   ModUnicodeString& value);
//		static void			setenv(const char* name, char* value);
//												// ある環境変数の値を変更する
//
//		static void			getUserName(ModUnicodeString& buf);
//		static void			getUserName(char* buf, ModSize size);
//												// 呼び出しプロセスの
//												// 実ユーザー名を得る
//
//		static ModProcessId	self();				// 呼び出しプロセスの
//												// プロセス ID を得る

		static void			setEncodingType(const ModKanjiCode::KanjiCodeType type);
												// マルチバイト文字の
												// エンコーディング方式を変更

		static ModKanjiCode::KanjiCodeType	getEncodingType();
												// マルチバイト文字の
												// エンコーディング方式を得る
	private:
		ModCommonDLL
		static ModKanjiCode::KanjiCodeType	_encodeingType;
												// マルチバイト文字の
												// エンコーディング方式
	};

	ModCommonDLL
	static ModErrorNumber	getErrorNumber(unsigned int error);
	ModCommonDLL
	static ModErrorNumber	getErrorNumber(unsigned int error,
										   const char* func,
										   const char* file, int line);
												// C の errno から
												// MOD のエラー番号を得る
	ModCommonDLL
	static ModErrorNumber	getOsErrorOtherReason(unsigned int error,
												  const char* func,
												  const char* file, int line);
												// C の errno から
												// その他のエラーを表す
												// MOD のエラー番号を得る
	ModCommonDLL
	static void				printOsErrorNumber(unsigned int error,
											   const char* func,
											   const char* file, int line);
												// C の errno を含む
												// エラーメッセージを出力する
	ModCommonDLL
	static void				printOsErrorMessage(unsigned int error,
												const char* file, int line);
												// C の errno の表す
												// エラーメッセージを出力する
protected:
	// 初期化、後処理関数
	ModCommonDLL
	static void initialize();
	ModCommonDLL
	static void terminate();
};

//	FUNCTION public
//	ModOs::Thread::Thread -- 仮想 OS のスレッドクラスのコンストラクター
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
ModOs::Thread::Thread()
	: _exitType(unknown)
{ }

//	FUNCTION public
//	ModOs::Thread::~Thread -- 仮想 OS のスレッドクラスのデストラクター
//
//	NOTES
//		仮想デストラクターにするために定義している
//		他になにもしない
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
ModOs::Thread::~Thread()
{ }

//	FUNCTION public
//	ModOs::Thread::getExitType -- 仮想 OS のスレッドの終了状態を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた仮想 OS のスレッドの終了状態
//
//	EXCEPTIONS
//		なし

inline
ModOs::Thread::ExitType
ModOs::Thread::getExitType() const
{
	return _exitType;
}

//	FUNCTION public
//	ModOs::Thread::Wrapper::Wrapper --
//		仮想 OS のスレッドラッパークラスのコンストラクター
//
//	NOTES
//		生成するスレッドで実際に実行する関数である
//		ModOsDriver::Thread::cover の引数を表す
//		スレッドラッパークラスのコンストラクターである
//		スレッド生成時に指定されたスレッドで実行する関数および引数を記録する
//
//	ARGUMENTS
//		ModOs::Thread::Routine	routine
//			生成するスレッドで実行する関数
//		void*				argument
//			生成するスレッドで実行する関数に与える引数
//		ModOsDriver::Thread*	thread
//			生成するスレッドの情報を記録する仮想 OS のスレッドクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
ModOs::Thread::Wrapper::
Wrapper(Routine routine, void* argument, Thread* thread)
	: _routine(routine),
	  _argument(argument),
	  _status(0),
	  _thread(thread)
{ }

//	FUNCTION public
//	ModOs::Thread::Wrapper::getRoutine -- 実行する関数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた実行する関数へのポインター
//
//	EXCEPTIONS
//		なし

inline
ModOs::Thread::Routine
ModOs::Thread::Wrapper::getRoutine() const
{
	return _routine;
}

//	FUNCTION public
//	ModOs::Thread::Wrapper::getArgument -- 実行する関数の引数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた実行する関数の引数
//
//	EXCEPTIONS
//		なし

inline
void* 
ModOs::Thread::Wrapper::getArgument() const
{
	return _argument;
}

//	FUNCTION public
//	ModOs::Thread::Wrapper::getThread --
//		生成するスレッドを表す仮想 OS のスレッドクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた生成するスレッドを表す仮想 OS の
//		スレッドクラスが格納された領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

inline
ModOs::Thread*
ModOs::Thread::Wrapper::getThread() const
{
	return _thread;
}

//	FUNCTION public
//	ModOs::Thread::Wrapper::getExitStatus -- 実行した関数の返り値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた実行した関数の返り値
//
//	EXCEPTIONS
//		なし

inline
void* 
ModOs::Thread::Wrapper::getExitStatus() const
{
	return _status;
}

//	FUNCTION public
//	ModOs::Thread::Wrapper::setExitStatus -- 実行した関数の返り値を設定する
//
//	NOTES
//		この関数が呼び出されるのは、実行した関数が正常終了したときのみ
//
//	ARGUMENTS
//		void*				status_
//			設定する実行した関数の返り値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModOs::Thread::Wrapper::setExitStatus(void* status)
{
	_status = status; 
}

//	FUNCTION public
//	ModOs::Process::setEncodingType -- マルチバイト文字のエンコーディング方式を変更
//
//	NOTES
//	このプロセスに渡す文字列、または、このプロセスが書き出す文字列の
//	エンコーディング方式を変更する。
//
//	ARGUMENTS

//
//	RETURN
// 		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModOs::Process::setEncodingType(const ModKanjiCode::KanjiCodeType type)
{
	_encodeingType = type;
}

inline
ModKanjiCode::KanjiCodeType
ModOs::Process::getEncodingType()
{
	return _encodeingType;
}

#endif	// __ModOs_H__

//
// Copyright (c) 1997, 2009, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
