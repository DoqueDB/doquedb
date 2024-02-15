// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataSource.java -- データソースをあらわすクラス
//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2016, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.client;

import java.security.Key;
import java.security.KeyFactory;
import java.security.spec.X509EncodedKeySpec;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;

import jp.co.ricoh.doquedb.common.BinaryData;
import jp.co.ricoh.doquedb.common.ClassID;
import jp.co.ricoh.doquedb.common.IntegerData;
import jp.co.ricoh.doquedb.common.Request;
import jp.co.ricoh.doquedb.common.Serializable;
import jp.co.ricoh.doquedb.common.StringData;
import jp.co.ricoh.doquedb.port.AuthorizeMode;
/**
 * データソースをあらわすクラス
 *
 */
public final class DataSource extends Object
	implements Runnable
{
	/** コネクションの配列 */
	private java.util.Vector _connectionArray;
	/** 最近返したコネクションの要素番号 */
	private int _connectionElement;
	/** ポートのハッシュマップ */
	private java.util.HashMap _portMap;
	/** クライアントが廃棄したポート */
	private java.util.Vector _expungePort;

	/** 接続先のホスト名 */
	private String _hostName;
	/** ポート番号 */
	private int _portNumber;

	/** 全データベースをあらわす ID */
	public final static int	DatabaseAll = 0xffffffff;

	/** １つのコネクションが管理する最大セッション数 */
	private final static int CONNECTION_THRESHOLD = 20;
	/** ポートプールチェック間隔の最小単位(ms) */
	private final static int TIME_UNIT = 500;
	/** ポートプールの最大ポート数 */
	private final static int MAXIMUM_CONNECTION_POOL_COUNT = 100;
	/** ポートプール数をチェックする間隔(ms) */
	private final static int CHECK_CONNECTION_POOL_PERIOD = 60000;

	/** スレッド用のインスタンス */
	private Thread _thread;

	/** セッションのハッシュマップ */
	private java.util.HashMap _sessionMap;

	/** クローズしたかどうか */
	private boolean isClosed;

	/** マスターID */
	private int _masterID;

	/** プロトコルバージョン */
	private int _protocolVersion = 0;	// プロトコルバージョン

	/** 認証方式 */
	private int _authorization;

	/**
	 * 新しくデータソースオブジェクトを作成する
	 *
	 * @param hostName_		接続先ホスト名
	 * @param portNumber_	ポート番号
	 */
	public DataSource(String hostName_, int portNumber_)
	{
		super(Object.DATA_SOURCE);

		//メンバー変数の初期化
		_connectionArray = new java.util.Vector();
		_connectionElement = 0;
		_portMap = new java.util.HashMap();
		_expungePort = new java.util.Vector();
		_hostName = hostName_;
		_portNumber = portNumber_;
		_thread = null;
		_sessionMap = new java.util.HashMap();
		isClosed = true;
		_masterID = 0;
		_authorization = 0;
	}

	/**
	 * オープンする。
	 *
	 * @param protocolVersion_	プロトコルバージョン(暗号化対応)
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	public void open(int protocolVersion_)
		throws	java.io.IOException, ClassNotFoundException,
				java.sql.SQLException,
				java.security.NoSuchAlgorithmException,
				java.security.spec.InvalidKeySpecException,
				java.security.InvalidKeyException,
				java.security.NoSuchProviderException,
				javax.crypto.NoSuchPaddingException,
				javax.crypto.BadPaddingException,
				javax.crypto.IllegalBlockSizeException
	{
		synchronized(_connectionArray)
		{
			_protocolVersion = protocolVersion_;
			// 認証方式が指定されていない場合はパスワード認証を指定する
			if ((_protocolVersion & AuthorizeMode.MaskMode)
				== AuthorizeMode.NONE) {
				_protocolVersion |= AuthorizeMode.PASSWORD;
			}

			// 新たにポートを得る
			Port port =
				getNewPort(jp.co.ricoh.doquedb.port.ConnectionSlaveID.ANY);
			port.open();

			//リクエストを送る
			port.writeObject(new Request(Request.BEGIN_CONNECTION));
			//クライアントのホスト名を送る
			String localHost
				= java.net.InetAddress.getLocalHost().getHostName();
			port.writeObject(new StringData(localHost));
			port.flush();

			//ステータスを得る
			port.readStatus();

			//クライアントコネクションを作成する
			Connection clientConnection = new Connection(this, port);

			//配列に加える
			_connectionArray.add(clientConnection);

			// マスターIDを保存する
			_masterID = port.getMasterID();

			// 認証方式を保存する
			_authorization = port.getAuthorization();

			//スレッドを起動
			if (_thread == null) {
				_thread = new Thread(this);
				_thread.setDaemon(true);
			}
			_thread.start();

			isClosed = false;
		}
	}

	/**
	 * クローズする。
	 *
	 * @throws	java.io.IOException
	 *			通信関係のエラー
	 */
	public synchronized void close()
		throws java.io.IOException
	{
		synchronized(_connectionArray)
		{
			if (isClosed == true)
				return;

			//スレッドを停止する
			_thread.interrupt();
			try
			{
				_thread.join();
				_thread = null;
			}
			catch (InterruptedException e) {} // 無視する

			synchronized(_sessionMap)
			{
				//すべてのセッションをクローズする
				java.util.Iterator i = _sessionMap.values().iterator();
				while (i.hasNext())
				{
					Session s = (Session)i.next();
					s.closeInternal();
				}
				_sessionMap.clear();
			}
			{
				//すべてのコネクションを切断する
				for (int i = 0; i < _connectionArray.size(); ++i)
				{
					Connection c = (Connection)_connectionArray.get(i);
					c.close();
				}
				_connectionArray.clear();
			}
			synchronized(_portMap)
			{
				//すべてのポートを切断する
				java.util.Iterator i = _portMap.values().iterator();
				while (i.hasNext())
				{
					Port p = (Port)i.next();
					p.close();
				}
				_portMap.clear();
				_expungePort.clear();
			}

			isClosed = true;
		}
	}

	/**
	 * サーバを停止する。
	 *
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	public void shutdown()
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		//新たにポートを得る
		Port port
			= getNewPort(jp.co.ricoh.doquedb.port.ConnectionSlaveID.ANY);
		port.open();

		//リクエストを送る
		port.writeObject(new Request(Request.SHUTDOWN));
		port.flush();

		//ステータスを得る
		port.readStatus();

		port.close();
	}

	/**
	 * サーバを停止する。
	 * ユーザ認証のあるサーバに対してはこちらを使わなくてはならない
	 *
	 * @param	userName_		ユーザ名
	 * @param	password_		パスワード
	 * @throws	java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws	java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	public void shutdown(String userName_,
						 String password_)
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		//新たにポートを得る
		Port port
			= getNewPort(jp.co.ricoh.doquedb.port.ConnectionSlaveID.ANY);
		port.open();

		boolean old = false;

		try {
			//リクエストを送る
			port.writeObject(new Request(Request.SHUTDOWN2));
			port.writeObject(new StringData(userName_));
			port.writeObject(new StringData(password_));
			port.flush();

			//ステータスを得る
			port.readStatus();
		} catch (jp.co.ricoh.doquedb.exception.UnknownRequest e) {
			old = true;
		} finally {
			port.close();
		}

		if (old == true)
			// 古いサーバと接続しているので、古い shutdown を送る
			shutdown();
	}

	/**
	 * 新しくセッションを作成する。
	 *
	 * @param databaseName_		データベース名
	 * @return	新しく作成したセッション
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	public Session createSession(String databaseName_)
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException,
			   java.security.NoSuchAlgorithmException,
			   java.security.spec.InvalidKeySpecException,
			   java.security.InvalidKeyException,
			   java.security.NoSuchProviderException,
			   javax.crypto.NoSuchPaddingException,
			   javax.crypto.BadPaddingException,
			   javax.crypto.IllegalBlockSizeException
	{
		Port port = null;

		synchronized(_connectionArray)
		{
			//クライアントコネクションを得る
			Connection clientConnection = getClientConnection();

			try
			{
				if (clientConnection == null)
				{
					// 一旦 close されているので、再度 open する
					open(_protocolVersion);
					clientConnection = getClientConnection();
				}
				//Workerを起動する
				port = clientConnection.beginWorker();
			}
			catch (java.io.IOException e)
			{
				if (isSessionExist() == true)
				{
					// まだ利用中のセッションがあるので再接続しない
					throw e;
				}

				// サーバが再起動したかもしれないので、DataSourceを初期化する
				try
				{
					close();
					open(_protocolVersion);
					clientConnection = getClientConnection();
					port = clientConnection.beginWorker();
				}
				catch (Exception ee) {}
				if (port == null)
					// 再初期化してもだめ
					throw e;
			}
		}

		IntegerData sessionID = null;

		try
		{
			//[<-] BeginSession
			port.writeObject(new Request(Request.BEGIN_SESSION));
			//[<-] データベース名
			port.writeObject(new StringData(databaseName_));
			port.flush();

			//[->] セッション番号
			sessionID = port.readIntegerData();
			//[->] ステータス
			port.readStatus();
		}
		catch (java.io.IOException ioe)
		{
			port.close();
			throw ioe;
		}
		catch (ClassNotFoundException cnfe)
		{
			port.close();
			throw cnfe;
		}
		catch (java.sql.SQLException sqle)
		{
			if (port.isReuse() == true)
				pushPort(port);
			else
				port.close();
			throw sqle;
		}

		//コネクションをプールする
		pushPort(port);

		//セッションを新しく作成する
		Session session = new Session(this, databaseName_,
									  sessionID.getValue());
		putSession(session);

		//必要なら新しいクライアントコネクションを得る
		newClientConnection();

		return session;
	}

	/**
	 * 新しくセッションを作成する。
	 *
	 * @param databaseName_		データベース名
	 * @param userName_			ユーザー名
	 * @param password_			パスワード
	 * @return	新しく作成したセッション
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	public Session createSession(String databaseName_,
								 String userName_,
								 String password_)
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException,
			   java.security.NoSuchAlgorithmException,
			   java.security.spec.InvalidKeySpecException,
			   java.security.InvalidKeyException,
			   java.security.NoSuchProviderException,
			   javax.crypto.NoSuchPaddingException,
			   javax.crypto.BadPaddingException,
			   javax.crypto.IllegalBlockSizeException
	{
		if (_masterID < jp.co.ricoh.doquedb.jdbc.Driver.PROTOCOL_VERSION5) {
			// ユーザー管理が使用できない
			return createSession(databaseName_);
		}

		Port port = null;

		synchronized(_connectionArray)
		{
			//クライアントコネクションを得る
			Connection clientConnection = getClientConnection();

			try
			{
				if (clientConnection == null)
				{
					// 一旦 close されているので、再度 open する
					open(_protocolVersion);
					clientConnection = getClientConnection();
				}
				//Workerを起動する
				port = clientConnection.beginWorker();
			}
			catch (java.io.IOException e)
			{
				if (isSessionExist() == true)
					// まだ利用中のセッションがあるので再接続しない
					throw e;

				// サーバが再起動したかもしれないので、DataSourceを初期化する
				try
				{
					close();
					open(_protocolVersion);
					clientConnection = getClientConnection();
					port = clientConnection.beginWorker();
				}
				catch (Exception ee) {}
				if (port == null)
					// 再初期化してもだめ
					throw e;
			}
		}

		IntegerData sessionID = null;

		try
		{
			//[<-] BeginSession
			port.writeObject(new Request(Request.BEGIN_SESSION2));
			//[<-] データベース名
			port.writeObject(new StringData(databaseName_));
			//[<-] ユーザー名
			port.writeObject(new StringData(userName_));
			//[<-] パスワード
			port.writeObject(new StringData(password_));
			port.flush();

			//[->] セッション番号
			sessionID = port.readIntegerData();
			//[->] ステータス
			port.readStatus();
		}
		catch (java.io.IOException ioe)
		{
			port.close();
			throw ioe;
		}
		catch (ClassNotFoundException cnfe)
		{
			port.close();
			throw cnfe;
		}
		catch (java.sql.SQLException sqle)
		{
			if (port.isReuse() == true)
				pushPort(port);
			else
				port.close();
			throw sqle;
		}

		//コネクションをプールする
		pushPort(port);

		//セッションを新しく作成する
		Session session = new Session(this, databaseName_, userName_,
									  sessionID.getValue());
		putSession(session);

		//必要なら新しいクライアントコネクションを得る
		newClientConnection();

		return session;
	}

	/**
	 * 新しくプリペアステートメントを作成する。
	 *
	 * @param databaseName_		データベース名
	 * @param statement_	    SQL文
	 * @return	新しく作成したプリペアステートメント
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	public PrepareStatement createPrepareStatement(String databaseName_,
												   String statement_)
		throws java.io.IOException, ClassNotFoundException,
				java.sql.SQLException
	{
		//クライアントコネクションを得る
		Connection clientConnection = getClientConnection();

		//Workerを起動する
		Port port = clientConnection.beginWorker();

		IntegerData prepareID = null;

		try
		{
			//[<-] PrepareStatement
			port.writeObject(new Request(Request.PREPARE_STATEMENT));
			//[<-] データベース名
			port.writeObject(new StringData(databaseName_));
			//[<-] SQL文
			port.writeObject(new StringData(statement_));
			port.flush();

			//[->] PrepareID
			prepareID = port.readIntegerData();
			//[->] ステータス
			port.readStatus();
		}
		catch (java.io.IOException ioe)
		{
			port.close();
			throw ioe;
		}
		catch (ClassNotFoundException cnfe)
		{
			port.close();
			throw cnfe;
		}
		catch (java.sql.SQLException sqle)
		{
			if (port.isReuse() == true)
				pushPort(port);
			else
				port.close();
			throw sqle;
		}

		//コネクションをプールする
		pushPort(port);

		//新しくプリペアステートメントを作成する
		PrepareStatement prepareStatement
			= new PrepareStatement(this, databaseName_, prepareID.getValue());

		return prepareStatement;
	}

	/**
	 * サーバの利用可能性を得る。
	 *
	 * @return	サーバが利用可能な場合は true 、利用不可能な場合は false
	 */
	public boolean isServerAvailable()
		throws java.io.IOException, java.lang.ClassNotFoundException,
			   java.sql.SQLException
	{
		// クライアントコネクションを得る
		Connection	clientConnection = getClientConnection();

		// 利用可能性を問い合わせる
		return clientConnection.isServerAvailable();
	}

	/**
	 * すべてのデータベースの利用可能性を得る。
	 *
	 * @return	すべてのデータベースが利用可能な場合は true 、
	 *			いずれかのデータベースが利用不可能な場合は false
	 */
	public boolean isDatabaseAvailable()
		throws java.io.IOException,
			   java.lang.ClassNotFoundException, java.sql.SQLException
	{
		return isDatabaseAvailable(DatabaseAll);
	}

	/**
	 * 任意のデータベースの利用可能性を得る。
	 *
	 * @param	id_	データベース ID
	 * @return	データベースが利用可能な場合は true 、利用不可能な場合は false
	 */
	public boolean isDatabaseAvailable(int	id_)
		throws java.io.IOException,
			   java.lang.ClassNotFoundException, java.sql.SQLException
	{
		// クライアントコネクションを得る
		Connection	clientConnection = getClientConnection();

		// 利用可能性を問い合わせる
		return clientConnection.isDatabaseAvailable(id_);
	}

	/**
	 * finalize
	 */
	protected void finalize() throws java.lang.Throwable
	{
		close();
	}

	/**
	 * コネクション管理クラスを得る。ラウンドロビン方式。
	 *
	 * @return	コネクション管理クラス
	 */
	protected Connection getClientConnection()
	{
		synchronized(_connectionArray)
		{
			if (_connectionArray.size() == 0)
				//すでにクローズされている
				return null;

			if (_connectionElement >= _connectionArray.size())
				_connectionElement = 0;

			return (Connection)_connectionArray.get(_connectionElement++);
		}
	}

	/**
	 * ポートプールからポートを取り出す。
	 *
	 * @return	得られたポート
	 */
	protected Port popPort()
	{
		Port port = null;
		synchronized(_portMap)
		{
			java.util.Iterator i = _portMap.values().iterator();
			if (i.hasNext())
			{
				port = (Port)i.next();
				_portMap.remove(new Integer(port.getSlaveID()));
			}
		}
		return port;
	}

	/**
	 * ポートプールにポートを挿入する。
	 *
	 * @param port_	ポート
	 */
	protected void pushPort(Port port_)
	{
		port_.reset();	// 再利用するためにリセットする

		synchronized(_portMap)
		{
			_portMap.put(new Integer(port_.getSlaveID()), port_);
		}
	}

	/**
	 * 廃棄したポートを登録する。
	 *
	 * @param port_	ポート
	 */
	protected void expungePort(Port port_)
		throws java.io.IOException
	{
		synchronized(_portMap)
		{
			_expungePort.add(new Integer(port_.getSlaveID()));
			port_.close();
		}
	}

	/**
	 * 新しいポートのインスタンスを得る。
	 *
	 * @param slaveID_	スレーブID
	 * @return	新しく作成されたポート
	 */
	protected Port getNewPort(int slaveID_)
	{
		return new Port(_hostName, _portNumber, _protocolVersion, slaveID_);
	}

	/**
	 * マスターIDを得る
	 *
	 */
	public int getMasterID()
	{
		return _masterID;
	}

	/**
	 * 認証方式を得る
	 *
	 */
	public int getAuthorization()
	{
		return _authorization;
	}

	/**
	 * セッション数がCONNECTION_THRESHOLDを越えた場合に、
	 * 新しいクライアントコネクションを作成する。
	 *
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	private void newClientConnection()
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		synchronized(_connectionArray)
		{
			int size = 0;
			synchronized(_sessionMap)
			{
				size = _sessionMap.size();
			}

			if (size > CONNECTION_THRESHOLD * _connectionArray.size())
			{
				//セッション数が越えた
				//	-> 新しいクライアントコネクションを作成する

				//今接続しているクライアントコネクションを得る
				Connection clientConnection = getClientConnection();

				//新しいクライアントコネクションを得る
				Connection newClientConnection
					= clientConnection.beginConnection();

				//配列に加える
				_connectionArray.add(newClientConnection);
			}
		}
	}

	/**
	 * ポートプールを管理するスレッド内で実行されるメソッド。
	 */
	public void run()
	{
		while (true)
		{
			int count = 0;
			int totalCount = CHECK_CONNECTION_POOL_PERIOD/TIME_UNIT;

			while (true)
			{
				try
				{
					Thread.sleep(TIME_UNIT);
				}
				catch (InterruptedException e)
				{
					return;
				}
				if (Thread.interrupted() == true)
					return;
				count++;
				if (count >= totalCount)
					break;
			}

			java.util.Vector slaveID = new java.util.Vector();

			synchronized(_portMap)
			{
				int size = _portMap.size() + _expungePort.size();
				if (size > MAXIMUM_CONNECTION_POOL_COUNT)
				{
					//超えているので超えた分を削除する
					int s = _portMap.size() - MAXIMUM_CONNECTION_POOL_COUNT
						- _expungePort.size();
					if (s > 0)
					{
						java.util.Iterator ite = _portMap.values().iterator();
						for (int i = 0; i < s; ++i)
						{
							Port p = (Port)ite.next();
							slaveID.add(new Integer(p.getSlaveID()));
							try
							{
								p.close();
							}
							catch (Exception e)
							{
								e.printStackTrace();
							}
						}
						for (int i = 0; i < s; ++i)
						{
							//マップから削除する
							_portMap.remove((Integer)slaveID.get(i));
						}
					}
					// 廃棄分を追加する
					slaveID.addAll(_expungePort);
					_expungePort.clear();
				}
			}

			if (slaveID.size() != 0)
			{
				// 削除したものがあるのでサーバに知らせる

				//クライアントコネクションを得る
				Connection clientConnection = getClientConnection();
				if (clientConnection != null)
				{
					try
					{
						clientConnection.disconnectPort(slaveID);
					}
					catch (Exception e) {}
				}
			}
		}
	}

	/**
	 * セッションが存在しているかどうか
	 *
	 * @return			存在している場合はtrue、それ以外の場合はfalse
	 */
	public boolean isSessionExist()
	{
		synchronized(_sessionMap)
		{
			if (_sessionMap.size() == 0)
				return false;
			return true;
		}
	}

	/**
	 * セッションを登録する
	 */
	private void putSession(Session session)
	{
		synchronized(_sessionMap)
		{
			_sessionMap.put(new Integer(session.getID()), session);
		}
	}

	/**
	 * セッションを削除する
	 */
	protected void removeSession(int id)
	{
		synchronized(_sessionMap)
		{
			_sessionMap.remove(new Integer(id));
		}
	}
}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
