// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModOsDriverLinux.h -- 仮想 OS ドライバーに関するクラス定義
// 
// Copyright (c) 1997, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModOsDriverLinux_H__
#define __ModOsDriverLinux_H__

#ifndef MOD_NO_THREAD
#include <signal.h>						// pthread_kill
#include <pthread.h>					// pthread_*_t
#endif
#include <netdb.h>						// struct addrinfo
#include <poll.h>						// struct pollfd

#include "ModCommonDLL.h"
#include "ModOs.h"
#include "ModException.h"

class ModCharString;
class ModUnicodeString;

//
// モジュールは汎用OSに属する。
// したがって、エラーはModOsXXXである。もちろん、すべてのクラスを
//	ModDefaultObjectのサブクラスとして作成すべきである。ModOsDriver自体は
//	インスタンス化されないので無関係。入れ子クラスは基本クラスである
//	ModOs::XXXの方でModDefaultObjectのサブクラスとして定義済みである。
//

//
// CLASS
// ModOsDriver -- 仮想OSドライバーのクラス定義(プラットフォーム依存) 
//
// NOTES
//	これはLinux 用の定義である。
//	ファイル、ソケット、スレッド、ミューテックス、条件変数、メモリ操作、
//	算術関係の必要な関数を提供する。
// 	共通I/FはModOsで定義され、各内部クラスはModOsにおける相当クラスを継承して
//	実装されている。
//	このレベルでは、OSレベルの違いを吸収することに注力し、機能は付加しない。
//
class ModCommonDLL ModOsDriver
{
public:
	//	CLASS
	//	ModOsDriver::File -- 仮想 OS のファイルクラス
	//
	//	NOTES
	//		仮想 OS のファイルを提供する
	//		ファイルディスクリプターを内部に保持し、
	//		それに対して処理を行う

	class ModCommonDLL File
		: public	ModOs::File
	{
	public:
		File();									// コンストラクター
		~File();								// デストラクター

		void				create(const ModUnicodeString& path,
								 int mode =
								 ModOs::ownerReadMode | ModOs::ownerWriteMode,
								 ModSize block = ModOsBlockSizeDefault);
		void				create(const char* path,
								 int mode =
								 ModOs::ownerReadMode | ModOs::ownerWriteMode,
								 ModSize block = ModOsBlockSizeDefault);
												// ファイルを生成する
		void				open(const ModUnicodeString& path, int flag,
								 int mode =
								 ModOs::ownerReadMode | ModOs::ownerWriteMode,
								 ModSize block = ModOsBlockSizeDefault);
		void				open(const char* path, int flag,
								 int mode =
								 ModOs::ownerReadMode | ModOs::ownerWriteMode,
								 ModSize block = ModOsBlockSizeDefault);
												// ファイルをオープンする
		void				close();			// ファイルをクローズする

		ModBoolean			isOpened() const;	// オープンされているかどうか

		static ModBoolean	isDirectory(const ModUnicodeString& path);
		static ModBoolean	isDirectory(const char* path);
												// ディレクトリーかどうか
		ModBoolean			isTapeDevice() const;
		static ModBoolean	isTapeDevice(const ModUnicodeString& path);
		static ModBoolean	isTapeDevice(const char* path);
												// テープデバイスかどうか

		ModSize				read(void* buf, ModSize size);
												// ファイルを読み出す
		ModSize				write(const void* buf, ModSize size);
												// ファイルへ書き込む
		ModFileOffset		seek(ModFileOffset offset,
								 ModOs::SeekWhence whence);
												// ファイルポインターを移動する

		ModFileSize			getFileSize();
		static ModFileSize	getFileSize(const ModUnicodeString& path);
		static ModFileSize	getFileSize(const char* path);
												// ファイルサイズを求める
		static ModFileSize	getFileSizeLimit();	// ファイルサイズの上限を得る

		static char			getPathSeparator();	// パス名の区切り文字を得る
		const ModCharString& getFullPathName() const;
		const ModUnicodeString& getFullPathNameW() const;
		static void			getFullPathName(const char* src,
											ModCharString& dst);
		static void			getFullPathName(const ModUnicodeString& src,
											ModUnicodeString& dst);
		static void			getFullPathName(const char* src,
											ModUnicodeString& dst);
		static void			getFullPathName(const ModUnicodeString& cBase_,
											const ModUnicodeString& cPath_,
											ModUnicodeString& cResult_);
												// ファイルの絶対パス名を求める
		static ModBoolean	isFullPathName(const char* pszPath_);
		static ModBoolean	isFullPathName(const ModUnicodeString& cPath_);
												// 文字列が絶対パス名を表すか得る
		static ModBoolean	getParentPathName(const ModUnicodeString& src,
											  ModUnicodeString& dst);
		static ModBoolean	getParentPathName(const char* src,
											  ModUnicodeString& dst);
												// ファイルの親を求める
		static const char*	getDefaultTapeDevice();
												// デフォルトのテープデバイスの
												// 絶対パス名を得る

		void				truncate(ModFileSize length = 0);
		static void			truncate(const ModUnicodeString& path, ModFileSize length = 0);
		static void			truncate(const char* path, ModFileSize length = 0);
												// ファイルサイズを変更する
		static void			unlink(const ModUnicodeString& path);
		static void			unlink(const char* path);
												// ファイルを削除する
		void				chmod(int mode);
		static void			chmod(const ModUnicodeString& path, int mode);
		static void			chmod(const char* path, int mode);
												// アクセス権を変更する
		static ModBoolean	access(const ModUnicodeString& path, int amode);
		static ModBoolean	access(const char* path, int amode);
												// 操作する権利の有無を調べる
		static ModBoolean	isNotFound(const ModUnicodeString& path);
		static ModBoolean	isNotFound(const char* path);
												// ファイルが存在しないか調べる
		static void			rename(const ModUnicodeString& oldpath,
								   const ModUnicodeString& newpath);
		static void			rename(const char* oldpath, const char* newpath);
												// ファイルの名前を変更する

		// 以下、将来廃止します
		static void			getcwd(char* buf, ModSize size);
												// カレントワーキング
												// ディレクトリーを求める
		// 以上、将来廃止します

		static void			mkdir(const ModUnicodeString& path,
								  int mode = ModOs::ownerModeMask,
								  ModBoolean recursive = ModFalse);
		static void			mkdir(const char* path,
								  int mode = ModOs::ownerModeMask,
								  ModBoolean recursive = ModFalse);
												// ディレクトリーを生成する
		static void			rmdir(const ModUnicodeString& path);
		static void			rmdir(const char* path);
												// ディレクトリーを削除する

		static void			rmAll(const ModUnicodeString& path,
								  ModBoolean forceFlag = ModFalse);
		static void			rmAll(const char* path,
								  ModBoolean forceFlag = ModFalse);

		static void			copy(const ModUnicodeString& srcPath,
								 const ModUnicodeString& dstPath,
								 int flag = ModOs::truncateFlag);
		static void
		copy(const ModUnicodeString& srcPath,
			 const ModUnicodeString& dstPath, int mode, int flag);
		static void			copy(const char* srcPath, const char* dstPath,
								 int flag = ModOs::truncateFlag);
												// ファイルをコピーする

		static void			copyAll(const ModUnicodeString& sourcePath,
									const ModUnicodeString& targetPath);
		static void
		copyAll(const ModUnicodeString& srcPath,
				const ModUnicodeString& dstPath, int mode);
		static void			copyAll(const char* sourcePath,
									const char* targetPath);

		static void			setNumberOfFileUnlimited();
												// 生成可能なファイル
												// ディスクリプターの数を
												// 最上限に設定する

		static void			getDirectoryEntry(const ModUnicodeString& path,
											  ModUnicodeString*** entrys,
											  int* n_entrys);
		static void			getDirectoryEntry(const char* path,
											  ModCharString*** entrys,
											  int* n_entrys);
											  	// ディレクトリ内の
												// ファイル名を求める

		static ModErrorNumber getErrorNumber(unsigned int error);
		static ModErrorNumber getErrorNumber(unsigned int error,
											 const char* func,
											 const char* file, int line);
												// ファイルに関する
												// C の errno から
												// MOD のエラー番号を得る
	private:
		static void rmAllFilesInDirectory(const char* path);
		static void copyAllFilesInDirectory(const char* sourcePath, const char* targetPath);

		ModCharString*		_path;				// ファイルの絶対パス名
		int					_descriptor;		// ファイルディスクリプター
		ModBoolean			_isTape;			// テープデバイスかどうか
	};

	//
	// CLASS
	// ModOsDriver::Socket -- 仮想OSソケットクラス
	// NOTES
	//	ソケット関係の仮想OSメソッドを集めている。
	//	ソケット作成用のプロトコル、タイプ、ドメインは
	//	現在のところ(PF_INET, SOCK_STREAM, 0)固定で、指定できない。
	//	ネットワークバイトオーダー変換以外は、内部に保持したソケット
	//	に対して処理を行なう。
	//	ネットワークバイトオーダー変換はstaticなメソッドであり、
	//	インスタンス化することなく直接利用する。
	//

	class ModCommonDLL Socket
		: public	ModOs::Socket
	{
	public:
		// コンストラクタ、デストラクタ
		Socket();
		~Socket();
		// ソケット作成の準備をする(まだソケットは作られない)
		void open();
		// ソケットを閉じる
		void close();
		// ソケットが有効かどうか
		ModBoolean			isOpened() const;
		// ソケットにバインド
		void bind(int port, int mark, int option, const char* hostname = 0);
		// peer nameの取得
		ModCharString getpeername();

		// コネクションを受けとり、新たなソケットを返す
		ModOs::Socket* accept(int& mark);
		// コネクションをはり、初期化する。
		void connect(const char* hostname, int port, int option);

		// ループしながら細かく読むメソッドはModSocketクラスで提供する。
		// 以下はOSのまま。
		ModSize read(void* buffer, ModSize size);
		ModSize write(const void* buffer, ModSize size);

		// 入出力の同期
		int select(ModSize second, ModSize millisecond);

		// IPv6か否か
		ModBoolean isIPv6() const;

		static void			getHostname(char* hostname, ModSize size);
												// ホスト名を得る

		// ネットワークオーダーとマシン固有のバイトオーダーの間の変換

	    static unsigned short hostToNetwork(unsigned short data);
		static ModUInt32	hostToNetwork(ModUInt32 data);
		static ModUInt64	hostToNetwork(ModUInt64 data);
		static float		hostToNetwork(float data);
		static double		hostToNetwork(double data);

		static unsigned short networkToHost(unsigned short data);
		static ModUInt32	networkToHost(ModUInt32 data);
		static ModUInt64	networkToHost(ModUInt64 data);
		static float		networkToHost(float data);
		static double		networkToHost(double data);

		// WinSockとI/Fをあわせるために用意したダミー関数。
		static void			initialize();
		static void			terminate();

		static ModErrorNumber getErrorNumber(unsigned int error);
		static ModErrorNumber getErrorNumber(unsigned int error,
											 const char* func,
											 const char* file, int line);
												// ソケットに関する
												// C の errno から
												// MOD のエラー番号を得る
	private:
		// accept用の構造体
		struct AcceptSocket
		{
			AcceptSocket() : socket(-1), mark(-1) {}
			
			int			socket;
			int			mark;	// bind時に呼出し側で指定した数字が格納され、
								// accept時に取得できる
		};
		
		// ソケット記述子の数を得る
		int getSocketSize() { return acceptSocketNum; }
		
		int					socket;				// ソケット記述子(read/write用)
		AcceptSocket*		acceptSocket;		// ソケット記述子(accept用)
		int					acceptSocketNum;	// 上記配列の数

		struct pollfd*		fds;		// accept用


		ModBoolean			ipv6;		// IPv6か否か
	};

	//	CLASS
	//	ModOsDriver::ThreadSpecificKey --
	//		仮想 OS のスレッド固有ストレージクラス
	//
	//	NOTES
	//		仮想 OS のスレッド固有ストレージを提供する
	//		POSIX のスレッド固有データ用キーを内部に保持し、
	//		それに対して処理を行う

	class ModCommonDLL ThreadSpecificKey
		: public	ModOs::ThreadSpecificKey
	{
	public:
		ThreadSpecificKey();					// コンストラクター
		~ThreadSpecificKey();					// デストラクター

		void				setValue(void *value);
												// 値を設定する
		void*				getValue();			// 設定された値を得る
	private:
#ifndef MOD_NO_THREAD
		pthread_key_t		_key;				// POSIX スレッド
												// 固有データ用キー
#else
		void*				_value;				// 設定された値
#endif
	};

	//	CLASS
	//	ModOsDriver::Thread -- 仮想 OS のスレッドクラス
	//
	//	NOTES
	//		仮想 OS のスレッドを提供する
	//		POSIX のスレッド ID とスレッド属性を内部に保持し、
	//		それに対して処理を行う

	class ModCommonDLL Thread
		: public	ModOs::Thread
	{
	public:
		Thread();								// コンストラクター
		~Thread();								// デストラクター

		void				create(Routine routine, void* argument);
												// スレッドを生成する
		void*				join();				// スレッドの終了を待つ
		void				kill();				// スレッドを強制終了する

		ModBoolean			isAlive() const;	// スレッドの存在を調べる

		ModThreadId			getThreadId() const;
												// スレッド ID を得る

		static void			sleep(ModSize milliSecond);
												// 呼び出しスレッドを
												// 任意時間停止する
		static void			exit(unsigned int status);
												// 呼び出しスレッドを終了する

		static ModThreadId	self();				// 呼び出しスレッドの
												// スレッド ID を得る

		static void			initialize();		// 利用のための初期化を行う
		static void			terminate();		// 利用した後処理を行う

		static ModErrorNumber getErrorNumber(unsigned int error);
		static ModErrorNumber getErrorNumber(unsigned int error,
											 const char* func,
											 const char* file, int line);
												// スレッドに関する
												// C の errno から
												// MOD のエラー番号を得る
#ifndef MOD_NO_THREAD
	private:
		static void*		cover(void* wrapper);
												// スレッドとして起動される関数
		static void			handlerPerProcess(int sig);
												// MOD のシグナルハンドラー
		static void			handlerPerThread(int sig);
												// シグナルハンドラー下位関数

		static int			_signalNumber;		// kill() で使用するシグナル

		ModBoolean			_joinable;			// スレッドの終了待ちが可能か
		pthread_t			_id;				// POSIX スレッド ID
		pthread_attr_t		_attribute;			// POSIX スレッド属性

		Wrapper*			_wrapper;			// スレッドで実行する関数
												// および引数を格納するラッパー

		unsigned int		_exitCode;			// スレッドの終了コード

		static ThreadSpecificKey* _selfKey;		// スレッドごとに自分自身を表す
												// 仮想 OS のスレッドの記憶用の
												// スレッド固有ストレージ
#endif
	};

	//	CLASS
	//	ModOsDriver::CriticalSection -- 仮想 OS のクリティカルセクションクラス
	//
	//	NOTES
	//		仮想 OS のクリティカルセクションを提供する
	//		POSIX が提供する同プロセススレッド間用ミューテックスを
	//		内部に保持し、それに対して処理を行う

	class ModCommonDLL CriticalSection
		: public	ModOs::CriticalSection
	{
	public:
		CriticalSection();						// コンストラクター
		~CriticalSection();						// デストラクター

		void				lock();				// ロックする
		ModBoolean			trylock();			// ロックを試みる
		void				unlock();			// ロックをはずす
#ifndef MOD_NO_THREAD
		pthread_mutex_t&	getInternalMutex() const;
												// 管理する
												// POSIX ミューテックスを得る
	private:
		pthread_mutex_t		_mutex;				// POSIX ミューテックス
#endif
	};

	//	CLASS
	//	ModOsDriver::Mutex -- 仮想 OS のミューテックスクラス
	//
	//	NOTES
	//		仮想 OS のミューテックスを提供する
	//		POSIX が提供する異プロセススレッド間用ミューテックスを
	//		内部に保持し、それに対して処理を行う
	//
	//		このクラスを共有メモリー上に生成し、
	//		それぞれのプロセスで参照することにより、
	//		異プロセススレッド間の同時実行制御用
	//		ミューテックスとして使用できる

	class ModCommonDLL Mutex
		: public	ModOs::Mutex
	{
	public:
		Mutex();								// コンストラクター
		~Mutex();								// デストラクター

		void				lock();				// ロックする
		ModBoolean			trylock();			// ロックを試みる
		void				unlock();			// ロックをはずす
#ifndef	MOD_NO_THREAD
		pthread_mutex_t&	getInternalMutex() const;
												// 管理する
												// POSIX ミューテックスを得る
	private:
		pthread_mutex_t		_mutex;				// POSIX ミューテックス
#endif
	};

	//	CLASS
	//	ModOsDriver::ConditionVariable -- 仮想 OS の条件変数
	//
	//	NOTES
	//		仮想 OS の条件変数を提供する
	//		POSIX が提供する同プロセススレッド間用条件変数を内部に保持し、
	//		それに対して処理を行う

	class ModCommonDLL ConditionVariable
		: public	ModOs::ConditionVariable
	{
	public:
		ConditionVariable(ModBoolean doWakeUpAll = ModFalse,
						  ModBoolean doManualReset = ModFalse);
												// コンストラクター
		~ConditionVariable();					// デストラクター

		void				signal();			// シグナル状態にする

		ModBoolean			wait();
		ModBoolean			wait(ModUInt32 milliSecond);
												// シグナル状態になるまで待つ

		void				reset();			// 非シグナル状態にする
	private:
#ifndef MOD_NO_THREAD
		pthread_cond_t		_condition;			// POSIX 条件変数
#endif
		CriticalSection		_mutex;				// 条件変数を保護するための
												// POSIX ミューテックス
		unsigned int		_count;				// 処理されていない
												// シグナル化の回数
		unsigned int		_waiter;			// シグナル化を待っている
												// スレッド数
		int					_behavior;			// 条件変数のふるまいを表す値
	};

	//	CLASS
	//	ModOsDriver::Math -- 仮想 OS の算術演算クラス
	//
	//	NOTES
	//		算術演算関連の仮想 OS インターフェース

	class ModCommonDLL Math
		: public	ModOs::Math
	{
	public:
		static double		log(double x);		// 対数を計算する
		static double		sqrt(double x);		// 平方根を計算する
		static double		pow(double x, double y);
												// 乗数を計算する
	};
	// 以下、将来廃止します
	// 算術
	static double			log(double x);
	static double			sqrt(double x);
	static double			pow(double x, double y);
	// 以上、将来廃止します

	//	CLASS
	//	ModOsDriver::Memory -- 仮想 OS のメモリー操作クラス
	//
	//	NOTES
	//		メモリー操作関連の仮想 OS インターフェース

	class ModCommonDLL Memory
		: public	ModOs::Memory
	{
	public:
		static void*		copy(void* dst, const void* src, ModSize size);
												// メモリー内容のコピー
		static void*		move(void* dst, const void* src, ModSize size);
												// メモリー内容の移動
		static void*		reset(void* dst, ModSize size);
												// メモリー内容を 0 埋めする
		static void*		set(void* dst, unsigned char c, ModSize size);
												// メモリー内容の
												// 特定値による初期化
		static int			compare(const void* l,
									const void* r, ModSize size);
												// メモリー内容の比較

		static void*		alloc(ModSize size, ModBoolean noError = ModFalse);
												// メモリーの確保
		static void			free(void* p);		// メモリーの破棄
	};
	// 以下、将来廃止します
	// メモリ操作
	static void*			memcpy(void* dst, const void* src, ModSize size);
	static void*			memmove(void* dst, const void* src, ModSize size);
	static void*			memset(void* dst, int c, ModSize size);
	static int				memcmp(const void* l, const void* r, ModSize size);
	static void*			malloc(ModSize size,
								   ModBoolean setError = ModTrue);
	static void				free(void* p);
	// 以上、将来廃止します

	//	CLASS
	//	ModOsDriver::Process -- 仮想 OS のプロセスクラス
	//
	//	NOTES
	//		プロセスの仮想 OS インターフェース

	class ModCommonDLL Process
		: public	ModOs::Process
	{
	public:
		static void			exit(int status);	// プロセスを終了する
		static void			abort();			// プロセスを強制終了する

		static void			getcwd(ModUnicodeString& buf);
		static void			getcwd(char* buf, ModSize size);
												// カレントワーキング
												// ディレクトリーを求める
		static void			chdir(const ModUnicodeString& path);
		static void			chdir(const char* path);
												// プロセスのカレントワーキング
												// ディレクトリーを変更する

		static unsigned int umask(unsigned int mask);
												// UMASK を変更する

		static void			getenv(const ModUnicodeString& name,
								   ModUnicodeString& buf);
		static void			getenv(const char* name, char* buf, ModSize size);
												// ある環境変数の値を求める
		static void			setenv(const ModUnicodeString& name,
								   const ModUnicodeString& value);
		static void			setenv(const char* name, const char* value);
												// ある環境変数の値を変更する

		static void			getUserName(ModUnicodeString& buf);
		static void			getUserName(char* buf, ModSize size);
												// 呼び出しプロセスの
												// 実ユーザー名を得る

		static ModProcessId	self();				// 呼び出しプロセスの
												// プロセス ID を得る
	};
	// 以下、将来廃止します
	static ModProcessId		getProcessId();
	static void				getUsername(char* buf, ModSize size);
	// 以上、将来廃止します

	// new()のエラーで例外を送出するためのハンドラとその設定関数
	static void	newErrorHandler();
	static void* newSetHandler(ModSize size);
};

// ****** ファイル関連 ******

//	FUNCTION public
//	ModOsDriver::File::create -- ファイルを作成し、オープンする
//
//	NOTES
//		指定されたパス名のファイルを指定されたアクセス権で生成し、オープンする
//		オープンされたファイルは書き込み可能、読み出し不可である
//		指定されたパス名のファイルが既に存在し、
//		そのファイルに書き込み許可があるときは、
//		そのファイルのサイズを 0 して、そのファイルをオープンする
//		このときは、ファイルの所有者やアクセス権は変更されない
//
//	ARGUMENTS
//		char*				path
//			生成するファイルのパス名
//		int				mode
//			指定されたとき
//				生成するファイルのアクセス権を表す値
//				ModOs::PermissionMode の論理和を指定する
//			指定されないとき
//				ModOs::ownerReadMode | ModOs::ownerWriteMode が
//				指定されたものとみなす
//		unsigned int		blockSize
//			指定されたとき
//				指定されたパス名のファイルがテープデバイスのとき、
//				読み書きを行うブロックのサイズ(B 単位)
//			指定されないとき
//				ModOsBlockSizeDefault が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			パス名として 0 が指定されている
//			(ModOsDriver::File::open より)
//		ModOsErrorFileAlreadyOpened
//			生成しようとしているファイルはすでにオープンされている
//			(ModOsDriver::File::open より)
//		ModOsErrorPermissionDenied
//			指定されたパス名にアクセスできない
//			または指定されたパス名の親ディレクトリーは書き込み不可である
//			または指定されたパス名のファイルが存在し、書き込み不可である
//			(ModOsDriver::File::open より)
//		ModOsErrorInterrupt
//			シグナルにより中断された
//			(ModOsDriver::File::open より)
//		ModOsErrorIsDirectory
//			指定されたパス名が既に存在するディレクトリーである
//			(ModOsDriver::File::open より)
//		ModOsErrorOpenTooManyFiles
//			すでにオープンしているファイルが多すぎる
//			(ModOsDriver::File::open より)
//		ModOsErrorTooLongFilename
//			指定されたパス名が長すぎる
//			(ModOsDriver::File::open より)
//		ModOsErrorNotSpace
//			ファイルシステムのリソースが足りない
//			(ModOsDriver::File::open より)
//		ModOsErrorFileNotFound
//			指定されたパス名の親ディレクトリーが存在しない、
//			またはディレクトリーでない
//			(ModOsDriver::File::open より)
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた
//			(ModOsDriver::File::open より)

inline
void
ModOsDriver::File::create(const char* path, int mode, unsigned int blockSize)
{
	this->open(path,
			   ModOs::writeOnlyFlag | ModOs::createFlag | ModOs::truncateFlag,
			   mode, blockSize);
}

//	FUNCTION public
//	ModOsDriver::File::isOpened -- ファイルがオープンされているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			オープンされている
//		ModFalse
//			オープンされていない
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModOsDriver::File::isOpened() const
{
	return (_descriptor < 0) ? ModFalse : ModTrue;
}

//	FUNCTION public
//	ModOsDriver::File::isTapeDevice -- ファイルがテープデバイスか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			テープデバイスである
//		ModFalse
//			テープデバイスでない
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModOsDriver::File::isTapeDevice() const
{
	return _isTape;
}

//	FUNCTION public
//	ModOsDriver::File::getPathSeparator -- パス名の区切り文字を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたパス名の区切り文字
//
//	EXCEPTIONS
//		なし

// static
inline
char
ModOsDriver::File::getPathSeparator()
{
	return '/';
}

//	FUNCTION public
//	ModOsDriver::File::getFullPathName --
//		オープン中のファイルの絶対パス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイルの絶対パス名
//
//	EXCEPTIONS
//		なし

inline
const ModCharString&
ModOsDriver::File::getFullPathName() const
{
	return *_path;
}

//	FUNCTION public
//	ModOs::File::getDefaultTapeDevice --
//		デフォルトテープデバイスの絶対パス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたデフォルトテープデバイスの絶対パス名
//
//	EXCEPTIONS
//		なし

// static
inline
const char*
ModOsDriver::File::getDefaultTapeDevice()
{
	static const char	path[] = "/dev/rmt/0";
	return path;
}

//	FUNCTION public
//	ModOsDriver::File::getErrorNumber --
//		ファイルに関する C の errno から MOD のエラー番号に変換する
// 
//	NOTES
//
//	ARGUMENTS
//		unsigned int		error
//			MOD のエラー番号を得たいファイルに関する C の errno
//		char*		func
//			指定されたとき
//				得られた MOD のエラー番号が ModOsErrorOtherReason のとき、
//				与えられた関数名を表す文字列を含むエラーメッセージを出力する
//			指定されないとき
//				エラーメッセージを出力しない
//		char*		file
//			指定されたとき
//				与えられた関数を呼び出したファイル名
//			指定されないとき
//				エラーメッセージを出力しない
//		int			line
//			指定されたとき
//				与えられた関数を呼び出した行数
//			指定されないとき
//				エラーメッセージを出力しない
//
//	RETURN
//		得られた MOD のエラー番号
//
//	EXCEPTIONS
//		なし

// static
inline
ModErrorNumber
ModOsDriver::File::getErrorNumber(unsigned int error)
{
	return ModOs::getErrorNumber(error);
}

// static
inline
ModErrorNumber
ModOsDriver::File::getErrorNumber(unsigned int error, const char* func,
								  const char* file, int line)
{
	return ModOs::getErrorNumber(error, func, file, line);
}

// ****** ソケット関連 ******

//	FUNCTION public
//	ModOsDriver::Socket::initialize -- ソケット環境の初期化
// 
//	NOTES
//		Linux ではソケット環境の初期化は必要ないので、なにもしない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
inline
void
ModOsDriver::Socket::initialize()
{ }

//	FUNCTION public
//	ModOsDriver::Socket::terminate -- ソケット環境の後処理
// 
//	NOTES
//		Linux ではソケット環境の後処理は必要ないので、なにもしない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
inline
void
ModOsDriver::Socket::terminate()
{ }

//	FUNCTION public
//	ModOsDriver::Socket::isOpened -- ソケットがオープンされているか調べる
// 
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			オープンされている
//		ModFalse
//			オープンされていない
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModOsDriver::Socket::isOpened() const
{
	return (this->socket >= 0 || this->acceptSocket != 0) ? ModTrue : ModFalse;
}

//	FUNCTION public
//	ModOsDriver::Socket::isIPv6 -- IPv6か否か
// 
//	NOTES
//
//	ARGUMENTS
//	   	なし
//
//	RETURN
//		ModBoolean
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModOsDriver::Socket::isIPv6() const
{
	return this->ipv6;
}

#include <sys/types.h>
#include <netinet/in.h>

//	FUNCTION public
//	ModOsDriver::Socket::hostToNetwork --
//		マシン固有のバイトオーダーからネットワークオーダーに変換する
// 
//	NOTES
//
//	ARGUMENTS
//		unsigned short		data
//			変換する unsigned short
//		ModUInt32			data
//			変換する ModUInt32
//		ModUInt64			data
//			変換する ModUInt64
//		float				data
//			変換する float
//		double				data
//			変換する double
//
//	RETURN
//		変換されたデータ
//
//	EXCEPTIONS
//		なし

// static
inline
unsigned short
ModOsDriver::Socket::hostToNetwork(unsigned short data)
{
	return htons(data);
}

// static
inline
ModUInt32
ModOsDriver::Socket::hostToNetwork(ModUInt32 data)
{
	return htonl(data);
}

// static
inline
ModUInt64
ModOsDriver::Socket::hostToNetwork(ModUInt64 data)
{
#if MOD_CONF_BYTEORDER == 0	// HL
	//【注意】	0x0123456789abcdefLL をファイルに書き出した結果、
	//			上位から下位へ書き出されていたので、そのままにする

    return data;

#else						// LH
	//【注意】	0x0123456789abcdefLL を
	//
	//			ModUInt64	v = 0x0123456789abcdef;
	//			write(&v, sizeof(ModUInt64));
	//
	//			のようにファイルに書き出した結果、
	//			Linux では、そのまま書き出され、
	//			Windows NT では、0xefcdab8967452301LL と書き出された
	//
	//			Linux にあわせるように
	//			下位 4 バイトをネットワークオーダーに
	//			変換したものを上位 4 バイトにし、
	//			上位 4 バイトをネットワークオーダーに
	//			変換したものを下位 4 バイトにする

	return
		(((ModUInt64) ModOsDriver::Socket::hostToNetwork(
								(ModUInt32) (data & 0xffffffff))) << 32) |
		ModOsDriver::Socket::hostToNetwork((ModUInt32) (data >> 32));
#endif
}

// static
inline
float
ModOsDriver::Socket::hostToNetwork(float data)
{
#if MOD_CONF_BYTEORDER == 0	// HL
	// SPARC のバイトオーダを標準とする
    return data;
#else						// LH
	float			ret;
	// reinterpret_cast は遅いのでC言語スタイルのキャストを利用
	unsigned char*	dst = (unsigned char*)&ret;
	unsigned char*	src = (unsigned char*)&data;

    dst[0] = src[3];
    dst[1] = src[2];
    dst[2] = src[1];
    dst[3] = src[0];

    return ret;
#endif
}

// static
inline
double
ModOsDriver::Socket::hostToNetwork(double data)
{
#if MOD_CONF_BYTEORDER == 0	// HL
	// SPARC のバイトオーダを標準とする
	return data;
#else						// LH
	double			ret;
	// reinterpret_cast は遅いのでC言語スタイルのキャストを利用
	unsigned char*	dst = (unsigned char*)&ret;
	unsigned char*	src = (unsigned char*)&data;

    dst[0] = src[7];
    dst[1] = src[6];
    dst[2] = src[5];
    dst[3] = src[4];
    dst[4] = src[3];
    dst[5] = src[2];
    dst[6] = src[1];
    dst[7] = src[0];

	return ret;
#endif
}

//	FUNCTION public
//	ModOsDriver::Socket::networkToHost --
//		ネットワークオーダーからマシン固有のバイトオーダーに変換する
// 
//	NOTES
//
//	ARGUMENTS
//		unsigned short		data
//			変換する unsigned short
//		ModUInt32			data
//			変換する ModUInt32
//		ModUInt64			data
//			変換する ModUInt64
//		float				data
//			変換する float
//		double				data
//			変換する double
//
//	RETURN
//		変換されたデータ
//
//	EXCEPTIONS
//		なし

// static
inline
unsigned short
ModOsDriver::Socket::networkToHost(unsigned short data)
{
	return ntohs(data);
}

// static
inline
ModUInt32
ModOsDriver::Socket::networkToHost(ModUInt32 data)
{
	return ntohl(data);
}

// static
inline
ModUInt64
ModOsDriver::Socket::networkToHost(ModUInt64 data)
{
#if MOD_CONF_BYTEORDER == 0	// HL
	// SPARC のバイトオーダを標準とする
    return data;
#else						// LH
	// 下位 4 バイトをマシン固有のバイトオーダーに
	// 変換したものを上位 4 バイトにし、
	// 上位 4 バイトをマシン固有のバイトオーダーに
	// 変換したものを下位 4 バイトにする
	return
		(((ModUInt64) ModOsDriver::Socket::networkToHost(
								(ModUInt32) (data & 0xffffffff))) << 32) |
		ModOsDriver::Socket::networkToHost((ModUInt32) (data >> 32));
#endif
}

// static
inline
float
ModOsDriver::Socket::networkToHost(float data)
{
#if MOD_CONF_BYTEORDER == 0	// HL
	// SPARC のバイトオーダを標準とする
    return data;
#else						// LH
	return ModOsDriver::Socket::hostToNetwork(data);
#endif
}

// static
inline
double
ModOsDriver::Socket::networkToHost(double data)
{
#if MOD_CONF_BYTEORDER == 0	// HL
	// SPARC のバイトオーダを標準とする
    return data;
#else						// LH
	return ModOsDriver::Socket::hostToNetwork(data);
#endif
}


// ****** スレッド固有ストレージ関連 ******

//	FUNCTION public
//	ModOsDriver::ThreadSpecificKey::~ThreadSpecificKey --
//		仮想 OS のスレッド固有ストレージを表すクラスのデストラクター
//
//	NOTES
//		POSIX が提供するスレッド固有データ用キーを後処理する
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
ModOsDriver::ThreadSpecificKey::~ThreadSpecificKey()
{
#ifndef MOD_NO_THREAD

	//【注意】	引数に与えるキーはおかしくないはずなので、
	//			EINVAL のエラーにならないはず

	(void) ::pthread_key_delete(_key);
#endif
}

//	FUNCTION public
//	ModOsDriver::ThreadSpecificKey::getValue --
//		仮想 OS のスレッド固有ストレージに設定されている値を取り出す
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		設定されている値を返す
//		値が設定されていなければ、0 を返す
//
//	EXCEPTIONS
//		なし

inline
void*
ModOsDriver::ThreadSpecificKey::getValue()
{
#ifndef MOD_NO_THREAD
	return ::pthread_getspecific(_key);
#else
	return _value;
#endif
}

// ****** スレッド関連 ******

//	FUNCTION public
//	ModOsDriver::Thread::self -- 呼び出したスレッドのスレッド ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		呼び出したスレッドのスレッド ID
//
//	EXCEPTIONS
//		なし

// static
inline
ModThreadId
ModOsDriver::Thread::self()
{
#ifndef MOD_NO_THREAD
	return (ModThreadId) ::pthread_self();
#else
	static ModProcessId	pid = ModUndefinedProcessId;

	if (pid == ModUndefinedProcessId)
		pid = ModOsDriver::Process::self();
	return (ModThreadId)pid;
#endif
}

//	FUNCTION public
//	ModOsDriver::Thread::getThreadId -- スレッド ID を得る
//
//	NOTES
//		スレッドが生成されていないときに得られる
//		スレッド ID は不定である
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
ModThreadId
ModOsDriver::Thread::getThreadId() const
{
#ifndef MOD_NO_THREAD
	return (ModThreadId) _id;
#else
	return 0;		// 返す値にまったく意味はない
#endif
}

//	FUNCTION public
//	ModOsDriver::Thread::isAlive -- スレッドが生成され、存在しているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			スレッドが存在している
//		ModFalse
//			スレッドは存在しない
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModOsDriver::Thread::isAlive() const
{
#ifndef MOD_NO_THREAD

	// スレッドに対してシグナルを送るふりをすれば、
	// スレッドの存在がわかる

	return (_joinable && ::pthread_kill(_id, 0) == 0) ? ModTrue : ModFalse;
#else
	return ModTrue;
#endif
}

// ****** クリティカルセクション関連 ******

#ifndef	MOD_NO_THREAD
#include <errno.h>
#endif

//	FUNCTION public
//	ModOsDriver::CriticalSection::CriticalSection --
//		仮想 OS のクリティカルセクションクラスのコンストラクター
//
//	NOTES
// 		POSIX が提供する同プロセススレッド間ミューテックスを表すクラスを
//		コンストラクトする
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
ModOsDriver::CriticalSection::CriticalSection()
{
#ifndef MOD_NO_THREAD

	//【注意】	引数に与えるミューテックスはおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_init(&_mutex, 0);
#endif
}

//	FUNCTION public
//	ModOsDriver::CriticalSection::~CriticalSection --
//		仮想 OS のクリティカルセクションクラスのデストラクター
//
//	NOTES
//		POSIX が提供する同プロセススレッド間ミューテックスを表すクラスを
//		デストラクトする
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
ModOsDriver::CriticalSection::~CriticalSection()
{
#ifndef MOD_NO_THREAD

	//【注意】	引数に与えるミューテックスはおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_destroy(&_mutex);
#endif
}

//	FUNCTION public
//	ModOsDriver::CriticalSection::lock --
//		仮想 OS のクリティカルセクションのロック
//
//	NOTES
//		POSIX が提供する同プロセススレッド間ミューテックスをロックする
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
void
ModOsDriver::CriticalSection::lock()
{
#ifndef MOD_NO_THREAD

	//【注意】	呼び出し回数が多いので、無駄なコードをいれてはいけない

	//【注意】	引数に与えるミューテックスはおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	//【注意】	ロック待ち中にロックを持っていたスレッドが強制終了されたとき、
	//			予期しないエラーが起きる
	//			こちらがエラー処理をしてもうまく動作しないので、なにもしない

	(void) ::pthread_mutex_lock(&_mutex);
#endif
}

//	FUNCTION public
//	ModOsDriver::CriticalSection::trylock --
//		仮想 OS のクリティカルセクションのロックを試みる
//
//	NOTES
//	 	POSIX が提供する同プロセススレッド間ミューテックスのロックを試みる
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			ロックできた
//		ModFalse
//			他のスレッドがロックしているのでロックできなかった
//
//	EXCEPTIONS
//		なし	

inline
ModBoolean
ModOsDriver::CriticalSection::trylock()
{
#ifndef MOD_NO_THREAD

	//【注意】	引数に与えるミューテックスはおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	return (::pthread_mutex_trylock(&_mutex) != EBUSY) ? ModTrue : ModFalse;
#else
	return ModTrue;
#endif
}

//	FUNCTION public
//	ModOsDriver::CriticalSection::unlock --
//		仮想 OS のクリティカルセクションのロックをはずす
//
//	NOTES
//		POSIX が提供する同プロセススレッド間ミューテックスのロックをはずす
//		ロックを持っていないスレッドが、ロックをはずしたときの動作は不定である
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
void
ModOsDriver::CriticalSection::unlock()
{
#ifndef MOD_NO_THREAD

	//【注意】	呼び出し回数が多いので、無駄なコードをいれてはいけない

	//【注意】	引数に与えるミューテックスはおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_unlock(&_mutex);
#endif
}

#ifndef MOD_NO_THREAD
//	FUNCTION public
//	ModOsDriver::CriticalSection::getInternalMutex --
//		仮想 OS のクリティカルセクションを実現している
//		POSIX ミューテックスを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		POSIX ミューテックスへの参照
//
//	EXCEPTIONS
//		なし

inline
pthread_mutex_t&
ModOsDriver::CriticalSection::getInternalMutex() const
{
	return (pthread_mutex_t&) _mutex;
}
#endif

// ****** ミューテックス関連 ******

//	FUNCTION public
//	ModOsDriver::Mutex::~Mutex --
//		仮想 OS のミューテックスクラスのデストラクター
//
//	NOTES
// 		POSIX が提供する異プロセススレッド間ミューテックスを表すクラスを
//		デストラクトする
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
ModOsDriver::Mutex::~Mutex()
{
#ifndef MOD_NO_THREAD

	//【注意】	引数に与えるミューテックスはおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_destroy(&_mutex);
#endif
}

//	FUNCTION public
//	ModOsDriver::Mutex::lock -- 仮想 OS のミューテックスのロック
//
//	NOTES
//		POSIX が提供する異プロセススレッド間ミューテックスをロックする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

#ifndef MOD_NO_THREAD
inline
void
ModOsDriver::Mutex::lock()
{
	//【注意】	呼び出し回数が多いので、無駄なコードをいれてはいけない

	//【注意】	引数に与えるミューテックスはおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	//【注意】	ロック待ち中にロックを持っていたスレッドが強制終了されたとき、
	//			予期しないエラーが起きる
	//			こちらがエラー処理をしてもうまく動作しないので、なにもしない

	(void) ::pthread_mutex_lock(&_mutex);
}
#else

//	【注意】	ModOsException.h をインクルードしたくないので、
//				ソースファイルで定義する

#endif

//	FUNCTION public
//	ModOsDriver::Mutex::trylock -- 仮想 OS のミューテックスのロックを試みる
//
//	NOTES
//	 	POSIX が提供する異プロセススレッド間ミューテックスのロックを試みる
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			ロックできた
//		ModFalse
//			他のスレッドがロックしているのでロックできなかった
//
//	EXCEPTIONS
//		なし	

inline
ModBoolean
ModOsDriver::Mutex::trylock()
{
#ifndef MOD_NO_THREAD

	//【注意】	引数に与えるミューテックスはおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	return (::pthread_mutex_trylock(&_mutex) != EBUSY) ? ModTrue : ModFalse;
#else
	return ModFalse;
#endif
}

//	FUNCTION public
//	ModOsDriver::Mutex::unlock -- 仮想 OS のミューテックスのロックをはずす
//
//	NOTES
//		POSIX が提供する異プロセススレッド間ミューテックスのロックをはずす
//		ロックを持っていないスレッドが、ロックをはずしたときの動作は不定である
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
void
ModOsDriver::Mutex::unlock()
{
#ifndef MOD_NO_THREAD

	//【注意】	呼び出し回数が多いので、無駄なコードをいれてはいけない

	//【注意】	引数に与えるミューテックスはおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_unlock(&_mutex);
#endif
}

#ifndef MOD_NO_THREAD
//	FUNCTION public
//	ModOsDriver::Mutex::getInternalMutex --
//		仮想 OS のミューテックスを実現している
//		POSIX ミューテックスを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		POSIX ミューテックスへの参照
//
//	EXCEPTIONS
//		なし

inline
pthread_mutex_t&
ModOsDriver::Mutex::getInternalMutex() const
{
	return (pthread_mutex_t&) _mutex;
}
#endif

// ****** 条件変数関連 ******

//	FUNCTION public
//	ModOsDriver::ConditionVariable::~ConditionVariable --
//		仮想 OS の条件変数クラスのデストラクター
//
//	NOTES
//		POSIX が提供する同プロセススレッド間条件変数を後処理する
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
ModOsDriver::ConditionVariable::~ConditionVariable()
{
#ifndef MOD_NO_THREAD

	//【注意】	引数に与える条件変数はおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_cond_destroy(&_condition);
#endif
}

//	FUNCTION public
//	ModOsDriver::ConditionVariable::reset -- シグナル状態を解除する
//
//	NOTES
//		シグナル状態になったとき、だれも待っていないと、
//		シグナル状態がそのまま続くことになる
//		この関数はシグナル状態を強制的に解除する
//		ただし、このとき待っているものがあってもそのままになる
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
// EXCEPTIONS
//		なし

inline
void
ModOsDriver::ConditionVariable::reset()
{
	// 内部ミューテックスをロックする

	_mutex.lock();

	// シグナル状態を解除する
	//
	//【注意】待っているものはそのままになる

	_count = 0;

	// 内部ミューテックスのロックをはずす

	_mutex.unlock();
}

// ****** 算術演算関連 ******

#include <math.h>

//	FUNCTION public
//	ModOsDriver::Math::log -- 対数を計算する
//
//	NOTES
//
//	ARGUMENTS
//		double				x
//			対数を計算する数
//
//	RETURN
//		与えられた数の対数
//
//	EXCEPTIONS
//		なし

// static
inline
double
ModOsDriver::Math::log(double x)
{
	return ::log(x);
}

// 以下、将来廃止します
// static
inline
double
ModOsDriver::log(double x)
{
	return ModOsDriver::Math::log(x);
}
// 以上、将来廃止します

//	FUNCTION public
//	ModOsDriver::sqrt -- 平方根を計算する
//
//	NOTES
//
//	ARGUMENTS
//		double				x
//			平方根を計算する数
//
//	RETURN
//		与えられた数の平方根
//
//	EXCEPTIONS
//		なし

// static
inline
double
ModOsDriver::Math::sqrt(double x)
{
	return ::sqrt(x);
}

// 以下、将来廃止します
// static
inline
double
ModOsDriver::sqrt(double x)
{
	return ModOsDriver::Math::sqrt(x);
}
// 以上、将来廃止します

//	FUNCTION public
//	ModOsDriver::Math::pow -- 乗数を計算する
//
//	NOTES
//		x の y 乗を計算する
//
//	ARGUMENTS
//		double				x
//			底
//		double				y
//			指数
//
//	RETURN
//		x の y 乗
//
//	EXCEPTIONS
//		なし

// static
inline
double
ModOsDriver::Math::pow(double x, double y)
{
	return ::pow(x, y);
}

// 以下、将来廃止します
// static
inline
double
ModOsDriver::pow(double x, double y)
{
	return ModOsDriver::Math::pow(x, y);
}
// 以上、将来廃止します

// ****** メモリー操作関連 ******

#include <string.h>

//	FUNCTION public
//	ModOsDriver::Memory::copy -- メモリー内容をコピーする
//
//	NOTES
//		あるサイズのメモリー内容を、ある位置へコピーする
//		コピー先の位置がコピーするメモリー内容に含まれていたときは
//		正しく動作しない
//
//	ARGUMENTS
//		void*				dst
//			コピー先の先頭アドレス
//		const void*			src
//			内容をコピーするメモリーの先頭アドレス
//		ModSize				size
//			コピーするメモリー内容のサイズ(B 単位)
//
//	RETURN
//		dst
//
//	EXCEPTIONS
//		なし

// static
inline
void*
ModOsDriver::Memory::copy(void* dst, const void* src, ModSize size)
{
	return ::memcpy(dst, src, size);
}

// 以下、将来廃止します
// static
inline
void*
ModOsDriver::memcpy(void* dst, const void* src, ModSize size)
{
	return ModOsDriver::Memory::copy(dst, src, size);
}
// 以上、将来廃止します

//	FUNCTION public
//	ModOsDriver::Memory::move -- メモリー内容を移動する
//
//	NOTES
//		あるサイズのメモリー内容を、ある位置へ移動する
//		移動先の位置が移動するメモリー内容に含まれていても正しく動作する
//
//	ARGUMENTS
//		void*				dst
//			移動先の先頭アドレス
//		const void*			src
//			内容を移動するメモリーの先頭アドレス
//		ModSize				size
//			移動するメモリー内容のサイズ(B 単位)
//
//	RETURN
//		dst
//
//	EXCEPTIONS
//		なし

// static
inline
void*
ModOsDriver::Memory::move(void* dst, const void* src, ModSize size)
{
	return ::memmove(dst, src, size);
}

// 以下、将来廃止します
// static
inline
void*
ModOsDriver::memmove(void* dst, const void* src, ModSize size)
{
	return ModOsDriver::Memory::move(dst, src, size);
}
// 以上、将来廃止します

//	FUNCTION public
//	ModOsDriver::Memory::reset -- メモリーを 0 埋めする
//
//	NOTES
//		指定された領域を 1 バイトづつ 0 で埋めていく
//
//	ARGUMENTS
//		void*				dst
//			0 埋めするメモリーの先頭アドレス
//		ModSize				size
//			0 埋めするメモリーのサイズ(B 単位)
//
//	RETURN
//		dst
//
//	EXCEPTIONS
//		なし

// static
inline
void*
ModOsDriver::Memory::reset(void* dst, ModSize size)
{
	return ::memset(dst, 0, size);
}

//	FUNCTION public
//	ModOsDriver::Memory::set -- メモリーを特定の値で初期化する
//
//	NOTES
//		指定された領域を 1 バイトづつ与えられた文字で埋めていく
//
//	ARGUMENTS
//		void*				dst
//			初期化するメモリーの先頭アドレス
//		unsigned char		c
//			メモリー中の 1 バイトごとを初期化する値
//		ModSize				size
//			初期化するメモリーのサイズ(B 単位)
//
//	RETURN
//		dst
//
//	EXCEPTIONS
//		なし

// static
inline
void*
ModOsDriver::Memory::set(void* dst, unsigned char c, ModSize size)
{
	return ::memset(dst, c, size);
}

// 以下、将来廃止します
// static
inline
void*
ModOsDriver::memset(void* dst, int c, ModSize size)
{
	return ModOsDriver::Memory::set(dst, c, size);
}
// 以上、将来廃止します

//	FUNCTION public
//	ModOsDriver::Memory::compare -- メモリー内容を比較する
//
//	NOTES
//
//	ARGUMENTS
//		const void*			l
//			r に与えられたメモリーと比較するメモリーの先頭アドレス
//		const void*			r
//			l に与えられたメモリーと比較するメモリーの先頭アドレス
//		ModSize				size
//			与えられたメモリーの先頭から比較するサイズ(B 単位)
//
//	RETURN
//		0
//			l と r のメモリー内容はまったく等しい
//		-1
//			l より r のほうが辞書順で大きい
//		1
//			l より r のほうが辞書順で小さい
//
//	EXCEPTIONS
//		なし

// static
inline
int
ModOsDriver::Memory::compare(const void* l, const void* r, ModSize size)
{
	return ::memcmp(l, r, size);
}

// 以下、将来廃止します
// static
inline
int
ModOsDriver::memcmp(const void* l, const void* r, ModSize size)
{
	return ModOsDriver::Memory::compare(l, r, size);
}
// 以上、将来廃止します

#include <stdlib.h>

// 以下、将来廃止します
// static
inline
void*
ModOsDriver::malloc(ModSize size, ModBoolean setError)
{
	return ModOsDriver::Memory::alloc(size, setError ? ModFalse : ModTrue);
}
// 以上、将来廃止します

//	FUNCTION public
//	ModOsDriver::Memory::free -- メモリーを破棄する
//
//	NOTES
//		ModOsDriver::Memory::alloc で確保したメモリーを破棄する
//
//	ARGUMENTS
//		void*				p
//			破棄するメモリーの先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
inline
void
ModOsDriver::Memory::free(void* p)
{
	::free(p);
}

// 以下、将来廃止します
// static
inline
void
ModOsDriver::free(void* p)
{
	ModOsDriver::Memory::free(p);
}
// 以上、将来廃止します

// ****** プロセス関連 ******

//	FUNCTION public
//	ModOsDriver::Process::exit -- プロセスを通常終了する
//
//	NOTES
//
//	ARGUMENTS
//		int					status
//			プロセスの終了ステータス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
inline
void
ModOsDriver::Process::exit(int status)
{
	::exit(status);
}

//	FUNCTION public
//	ModOsDriver::Process::abort -- プロセスを異常終了する
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

// static
inline
void
ModOsDriver::Process::abort()
{
	::abort();
}

#include <sys/stat.h>

//	FUNCTION public
//	ModOsDriver::Process::umask -- プロセスのファイルアクセス権マスクを設定する
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int			mask
//			新しいファイルアクセス権マスク
//
//	RETURN
//		設定前のファイルアクセス権マスク
//
//	EXCEPTIONS
//		なし

// static
inline
unsigned int
ModOsDriver::Process::umask(unsigned int mask)
{
	return ::umask((mode_t) mask);
}

#include <unistd.h>

//	FUNCTION public
//	ModOsDriver::Process::self -- 呼び出しプロセスのプロセス ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
// 		得られたプロセス ID
//
//	EXCEPTIONS
//		なし

// static
inline
ModProcessId
ModOsDriver::Process::self()
{
	return (ModProcessId) ::getpid();
}

// 以下、将来廃止します
// static
inline
ModProcessId
ModOsDriver::getProcessId()
{
	return ModOsDriver::Process::self();
}
// 以上、将来廃止します

#endif	// __ModOsDriverLinux_H__

//
// Copyright (c) 1997, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
