// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Session.java -- セッションクラス
//
// Copyright (c) 2002, 2003, 2004, 2005, 2007, 2008, 2023 Ricoh Company, Ltd.
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

import jp.co.ricoh.doquedb.common.ClassID;
import jp.co.ricoh.doquedb.common.DataArrayData;
import jp.co.ricoh.doquedb.common.IntegerData;
import jp.co.ricoh.doquedb.common.Request;
import jp.co.ricoh.doquedb.common.Serializable;
import jp.co.ricoh.doquedb.common.StringData;
import jp.co.ricoh.doquedb.exception.SessionNotAvailable;

/**
 * セッションクラス
 *
 */
public final class Session extends Object
{
	/** データソース */
	private DataSource _dataSource;
	/** データベース名 */
	private String _databaseName;
	/** ユーザー名 */
	private String _userName;
	/** セッションID */
	private int _sessionID;

	/**
	 * 新しくセッションオブジェクトを作成する。
	 *
	 * @param dataSource_	データソース
	 * @param databaseName_	データベース名
	 * @param sessionID_	セッションID
	 */
	protected Session(DataSource dataSource_,
					  String databaseName_,
					  int sessionID_)
	{
		this(dataSource_, databaseName_, null, sessionID_);
	}

	/**
	 * 新しくセッションオブジェクトを作成する。
	 *
	 * @param dataSource_	データソース
	 * @param databaseName_	データベース名
	 * @param usuerName_	ユーザー名
	 * @param sessionID_	セッションID
	 */
	protected Session(DataSource dataSource_,
					  String databaseName_,
					  String userName_,
					  int sessionID_)
	{
		super(Object.SESSION);
		_dataSource = dataSource_;
		_databaseName = databaseName_;
		_userName = userName_;
		_sessionID = sessionID_;
	}

	/**
	 * クローズする。
	 *
	 * @throws	java.io.IOException
	 *			通信関係のエラー
	 */
	public void close()
		throws java.io.IOException
	{
		int id = closeInternal();

		if (id != 0)
		{
			// セッションを削除する
			getDataSource().removeSession(id);
		}
	}

	/**
	 * SQL文を実行する。
	 *
	 * @param statement_	SQL文
	 * @param parameter_	パラメータ
	 * @return 	結果集合
	 * @throws	java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws	java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	public ResultSet executeStatement(String statement_,
									  DataArrayData parameter_)
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		if (isValid() == false)
			throw new SessionNotAvailable();

		//クライアントコネクションを得る
		Connection clientConnection = _dataSource.getClientConnection();

		//ワーカを起動する
		Port port = clientConnection.beginWorker();

		//[<-] リクエスト
		port.writeObject(new Request(Request.EXECUTE_STATEMENT));
		//[<-] セッションID
		port.writeObject(new IntegerData(_sessionID));
		//[<-] SQL文
		port.writeObject(new StringData(statement_));
		//[<-] パラメータ
		port.writeObject(parameter_);
		port.flush();

		//結果集合を返す
		return new ResultSet(_dataSource, port);
	}

	/**
	 * プリペアしたSQL文を実行する。
	 *
	 * @param prepare_	プリペアステートメント
	 * @param parameter_	パラメータ
	 * @return 	結果集合
	 * @throws	java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws	java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	public ResultSet executePrepareStatement(PrepareStatement prepare_,
											 DataArrayData parameter_)
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		if (isValid() == false)
			throw new SessionNotAvailable();

		//クライアントコネクションを得る
		Connection clientConnection = _dataSource.getClientConnection();

		//ワーカを起動する
		Port port = clientConnection.beginWorker();

		//[<-] リクエスト
		port.writeObject(new Request(Request.EXECUTE_PREPARE_STATEMENT));
		//[<-] セッションID
		port.writeObject(new IntegerData(_sessionID));
		//[<-] プリペアステートメントID
		port.writeObject(new IntegerData(prepare_.getPrepareID()));
		//[<-] パラメータ
		port.writeObject(parameter_);
		port.flush();

		//結果集合を返す
		return new ResultSet(_dataSource, port);
	}

	/**
	 * 新しくプリペアステートメントを作成する。
	 *
	 * @param statement_	SQL文
	 * @return	新しく作成したプリペアステートメント
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	public PrepareStatement createPrepareStatement(String statement_)
		throws java.io.IOException, ClassNotFoundException,
				java.sql.SQLException
	{
		if (_dataSource.getMasterID() < jp.co.ricoh.doquedb.jdbc.Driver.PROTOCOL_VERSION3)
		{
			// PREPARE_STATEMENT2がない
			return _dataSource.createPrepareStatement(_databaseName,
													  statement_);
		}

		// クライアントコネクションを得る
		Connection clientConnection = _dataSource.getClientConnection();


		//Workerを起動する
		Port port = clientConnection.beginWorker();

		IntegerData prepareID = null;

		try
		{
			//[<-] リクエスト
			port.writeObject(new Request(Request.PREPARE_STATEMENT2));
			//[<-] セッションID
			port.writeObject(new IntegerData(_sessionID));
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
				_dataSource.pushPort(port);
			else
				port.close();
			throw sqle;
		}

		//コネクションをプールする
		_dataSource.pushPort(port);

		return new PrepareStatement(this, prepareID.getValue());
	}

	/**
	 * データソースオブジェクトを得る。
	 *
	 * @return		データソースオブジェクト
	 */
	public DataSource getDataSource()
	{
		return _dataSource;
	}

	/**
	 * データベース名を得る。
	 *
	 * @return		データベース名
	 */
	public String getDatabaseName()
	{
		return _databaseName;
	}

	/**
	 * ユーザー名を得る。
	 *
	 * @return		ユーザー名
	 */
	public String getUserName()
	{
		return _userName;
	}

	/**
	 * セッションIDを得る。
	 *
	 * @return		セッションID
	 */
	public int getID()
	{
		return _sessionID;
	}

	/**
	 * プリペアステートメントを削除する
	 *
	 * @param id_		プリペアステートメントID
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	protected void erasePrepareStatement(int id_)
		throws java.io.IOException, ClassNotFoundException,
				java.sql.SQLException
	{
		// クライアントコネクションを得る
		Connection clientConnection = _dataSource.getClientConnection();
		// Workerを起動する
		Port port = clientConnection.beginWorker();

		try
		{
			//[<-] リクエスト
			port.writeObject(new Request(Request.ERASE_PREPARE_STATEMENT2));
			//[<-] セッションID
			port.writeObject(new IntegerData(_sessionID));
			//[<-] PrepareID
			port.writeObject(new IntegerData(id_));
			port.flush();

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
				_dataSource.pushPort(port);
			else
				port.close();
			throw sqle;
		}

		//コネクションをプールする
		_dataSource.pushPort(port);
	}

	/**
	 * DBユーザーを新規登録する
	 *
	 * @param	userName_
	 *			新規登録するユーザーの名称。
	 * @param	password_
	 *			新規登録するユーザーの初期パスワード。
	 * @param	userID_
	 *			新規登録するユーザーのユーザーID。
	 *			<code>jp.co.ricoh.doquedb.port.UserID.AUTO</code>が指定されていると、
	 *			サーバー内のユーザーIDの最大値に1加えた値が使用されます。
	 * @return	なし
	 * @throws	java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws	java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void createUser(String userName_, String password_, int userID_)
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		// クライアントコネクションを得る
		Connection clientConnection = _dataSource.getClientConnection();
		// Workerを起動する
		Port port = clientConnection.beginWorker();

		try
		{
			//[<-] リクエスト
			port.writeObject(new Request(Request.CREATE_USER));
			//[<-] セッションID
			port.writeObject(new IntegerData(_sessionID));
			//[<-] User name
			port.writeObject(new StringData(userName_));
			//[<-] Password
			port.writeObject(new StringData(password_));
			//[<-] User ID
			port.writeObject(new IntegerData(userID_));
			port.flush();

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
				_dataSource.pushPort(port);
			else
				port.close();
			throw sqle;
		}

		//コネクションをプールする
		_dataSource.pushPort(port);
	}


	/**
	 * DBユーザーを削除する
	 *
	 * @param	userName_
	 *			削除するユーザーの名称。
	 * @param	dropBehavior_
	 *			すべてのデータベースに対する削除するユーザーのPrivilege情報の扱い。
	 * @return	なし
	 * @throws	java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws	java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void dropUser(String userName_, int dropBehavior_)
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		// クライアントコネクションを得る
		Connection clientConnection = _dataSource.getClientConnection();
		// Workerを起動する
		Port port = clientConnection.beginWorker();

		try
		{
			//[<-] リクエスト
			port.writeObject(new Request(Request.DROP_USER));
			//[<-] セッションID
			port.writeObject(new IntegerData(_sessionID));
			//[<-] User name
			port.writeObject(new StringData(userName_));
			//[<-] Drop behavior
			port.writeObject(new IntegerData(dropBehavior_));
			port.flush();

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
				_dataSource.pushPort(port);
			else
				port.close();
			throw sqle;
		}

		//コネクションをプールする
		_dataSource.pushPort(port);
	}


	/**
	 * 自分自身のパスワードを変更する
	 *
	 * @param	password_
	 *			変更後のパスワード。
	 * @return	なし
	 * @throws	java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws	java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void changeOwnPassword(String password_)
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		// クライアントコネクションを得る
		Connection clientConnection = _dataSource.getClientConnection();
		// Workerを起動する
		Port port = clientConnection.beginWorker();

		try
		{
			//[<-] リクエスト
			port.writeObject(new Request(Request.CHANGE_OWN_PASSWORD));
			//[<-] セッションID
			port.writeObject(new IntegerData(_sessionID));
			//[<-] Password
			port.writeObject(new StringData(password_));
			port.flush();

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
				_dataSource.pushPort(port);
			else
				port.close();
			throw sqle;
		}

		//コネクションをプールする
		_dataSource.pushPort(port);
	}

	/**
	 * DBユーザーのパスワードを変更する
	 *
	 * @param	userName_
	 *			パスワードを変更するユーザーの名称。
	 * @param	password_
	 *			変更後のパスワード。
	 * @return	なし
	 * @throws	java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws	java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void changePassword(String userName_, String password_)
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		// クライアントコネクションを得る
		Connection clientConnection = _dataSource.getClientConnection();
		// Workerを起動する
		Port port = clientConnection.beginWorker();

		try
		{
			//[<-] リクエスト
			port.writeObject(new Request(Request.CHANGE_PASSWORD));
			//[<-] セッションID
			port.writeObject(new IntegerData(_sessionID));
			//[<-] User name
			port.writeObject(new StringData(userName_));
			//[<-] Password
			port.writeObject(new StringData(password_));
			port.flush();

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
				_dataSource.pushPort(port);
			else
				port.close();
			throw sqle;
		}

		//コネクションをプールする
		_dataSource.pushPort(port);
	}

	/**
	 * サーバーのバージョンを問い合わせる
	 *
	 * @param	なし
	 * @return	String サーバーのProductVersion
	 * @throws	java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws	java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String queryProductVersion()
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		// クライアントコネクションを得る
		Connection clientConnection = _dataSource.getClientConnection();
		// Workerを起動する
		Port port = clientConnection.beginWorker();

		String result;
		try
		{
			//[<-] リクエスト
			port.writeObject(new Request(Request.QUERY_PRODUCT_VERSION));
			port.flush();

			//[->] プロダクトバージョン
			Serializable object = port.readObject();
			if ((object instanceof StringData) == false) {
				throw new jp.co.ricoh.doquedb.exception.Unexpected();
			}
			result = ((StringData)object).getValue();
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
				_dataSource.pushPort(port);
			else
				port.close();
			throw sqle;
		}

		//コネクションをプールする
		_dataSource.pushPort(port);

		return result;
	}

	/**
	 * クローズする。
	 *
	 * @throws	java.io.IOException
	 *			通信関係のエラー
	 */
	protected synchronized int closeInternal()
		throws java.io.IOException
	{
		int id = getID();

		if (isValid() == true)
		{
			try
			{
				Connection clientConnection
					= _dataSource.getClientConnection();
				if (clientConnection != null)
				{
					//ワーカを起動し、セッションを終了する

					//ワーカを起動する
					Port port = clientConnection.beginWorker();

					try
					{
						//[<-] リクエスト
						port.writeObject(new Request(Request.END_SESSION));
						//[<-] セッションID
						port.writeObject(new IntegerData(_sessionID));
						port.flush();

						//[->] ステータス
						port.readStatus();
					}
					catch (java.sql.SQLException sqle)
					{
						if (port.isReuse() == true)
							_dataSource.pushPort(port);
						else
							port.close();
						throw sqle;
					}

					//ポートを返す
					_dataSource.pushPort(port);
				}
			}
			catch (Exception e) {}	//例外は無視する
		}

		invalid();

		return id;
	}

	/**
	 * セッションを利用不可にする。
	 */
	protected void invalid()
	{
		_sessionID = 0;	// 0は使用していない
	}

	/**
	 * セッションが利用可能かどうか
	 */
	protected boolean isValid()
	{
		return (_sessionID == 0) ? false : true;
	}

}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2007, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
