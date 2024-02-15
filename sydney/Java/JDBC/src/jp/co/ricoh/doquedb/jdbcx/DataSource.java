// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataSource.java -- JDBCX の DataSourceクラス
//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2011, 2015, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.jdbcx;

import java.sql.Connection;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.Map;
import java.util.Stack;
import java.util.logging.Logger;

import javax.naming.NamingException;
import javax.naming.Reference;
import javax.naming.StringRefAddr;
import javax.sql.ConnectionEvent;
import javax.sql.ConnectionEventListener;
import javax.sql.PooledConnection;

import jp.co.ricoh.doquedb.exception.LockTimeout;
import jp.co.ricoh.doquedb.exception.NotSupported;
import jp.co.ricoh.doquedb.exception.Unexpected;

/**
 * この DataSource オブジェクトが表す物理データソースへの接続に対するファクトリです。
 * DriverManager 機能の代わりに、DataSource オブジェクトが接続を得るための適切な手段となります。
 * DataSource インタフェースを実装するオブジェクトは一般に、JavaTM Naming and Directory (JNDI) API を基にしたネームサービスで登録されています。
 *
 * ここに記述されていない javax.sql.DataSource メソッドは jp.co.ricoh.doquedb.jdbcx.SydDataSource に記述されています。
 *
 */
public class DataSource extends SydDataSource implements javax.sql.DataSource
{
	private static Map dataSources = new HashMap();

	public static DataSource getDataSource(String name)
	{
		return (jp.co.ricoh.doquedb.jdbcx.DataSource)dataSources.get(name);
	}

	// プロパティ
	String _dataSourceName;
	private int initialConnections = 0;
	private int maxConnections = 0;

	private boolean initialized = false;
	private Stack available = new Stack();
	private Stack used = new Stack();
	private Object lock = new Object();
	private ConnectionPoolDataSource source;

	/**
	 * DataSource の対象となるサーバ名を設定します。
	 *
	 * @param	serverName_
	 * 			サーバ名。
	 * @throws	java.lang.IllegalStateException
	 *			初期化後にサーバ名は変更出来ません。
	 */
	public void setServerName(String serverName_)
	{
		if(initialized){
			throw new IllegalStateException("Cannot set Data Source properties after DataSource has been used");
		}
		super.setServerName(serverName_);
	}

	/**
	 * DataSource の対象となるポート番号を設定します。
	 *
	 * @param	portNumber_
	 * 			ポート番号。
	 * @throws	java.lang.IllegalStateException
	 *			初期化後にポート番号は変更出来ません。
	 */
	public void setPortNumber(int portNumber_)
	{
		if(initialized){
			throw new IllegalStateException("Cannot set Data Source properties after DataSource has been used");
		}
		super.setPortNumber(portNumber_);
	}

	/**
	 * DataSource の対象となるデータベース名を設定します。
	 *
	 * @param	databaseName_
	 * 			データベース名。
	 * @throws	java.lang.IllegalStateException
	 *			初期化後にデータベース名は変更出来ません。
	 */
	public void setDatabaseName(String databaseName_)
	{
		if(initialized){
			throw new IllegalStateException("Cannot set Data Source properties after DataSource has been used");
		}
		super.setDatabaseName(databaseName_);
	}

	/**
	 * DataSource の対象となるユーザ名を設定します。
	 *
	 * @param	user_
	 * 			ユーザ名。
	 * @throws	java.lang.IllegalStateException
	 *			初期化後にユーザ名は変更出来ません。
	 */
	public void setUser(String user_)
	{
		if(initialized)	{
			throw new IllegalStateException("Cannot set Data Source properties after DataSource has been used");
		}
		super.setUser(user_);
	}

	/**
	 * DataSource の対象となるユーザパスワードを設定します。
	 *
	 * @param	password_
	 * 			ユーザパスワード。
	 * @throws	java.lang.IllegalStateException
	 *			初期化後にユーザパスワードは変更出来ません。
	 */
	public void setPassword(String password_)
	{
		if(initialized)	{
			throw new IllegalStateException("Cannot set Data Source properties after DataSource has been used");
		}
		super.setPassword(password_);
	}

	/**
	 * DataSource の対象となるプロトコルを設定します。
	 *
	 * @param	networkProtocol_
	 * 			プロトコル。
	 * @throws	java.lang.IllegalStateException
	 *			初期化後にプロトコルは変更出来ません。
	 */
	public void setNetworkProtocol(String networkProtocol_)
	{
		if (initialized) {
			throw new IllegalStateException("Cannot set Data Source properties after DataSource has been used");
		}
		super.setNetworkProtocol(networkProtocol_);
	}

	/**
	 * DataSource の対象となるJDBCのURLを設定します。
	 *
	 * @param	url_
	 *			JDBC の URL
	 */
	public void setUrl(String url_)
	{
		if (initialized) {
			throw new IllegalStateException("Cannot set Data Source properties after DataSource has been used");
		}
		super.setUrl(url_);
	}

	/**
	 * この DataSource が初期化されるとき作成される接続数を得ます。
	 *
	 * @return	初期化される時に作成される接続数。
	 */
	public int getInitialConnections()
	{
		return initialConnections;
	}

	/**
	 * この DataSource が初期化されるとき作成される接続数を設定します。
	 *
	 * @param	initialConnections_
	 * 			初期化される時に作成される接続数。
	 * @throws	java.lang.IllegalStateException
	 *			初期化後には設定出来ません。
	 */
	public void setInitialConnections(int initialConnections_)
	{
		if(initialized){
			throw new IllegalStateException("Cannot set Data Source properties after DataSource has been used");
		}
		this.initialConnections = initialConnections_;
	}

	/**
	 * プールが許容する最大の接続数を得ます。
	 *
	 * @return	許されたプールされた接続の最大数。
	 */
	public int getMaxConnections()
	{
		return maxConnections;
	}

	/**
	 * プールが許容する最大の接続数を設定します。
	 *
	 * @param	maxConnections_
	 *			最大が許すプールされた接続数。
	 * @throws java.lang.IllegalStateException
	 *			初期化後には設定出来ません。
	 */
	public void setMaxConnections(int maxConnections_)
	{
		if(initialized){
			throw new IllegalStateException("Cannot set Data Source properties after DataSource has been used");
		}
		this.maxConnections = maxConnections_;
	}

	/**
	 * この DataSource の名前を得ます。
	 *
	 * @return	データソース名。
	 */
	public String getDataSourceName()
	{
		return _dataSourceName;
	}

	/**
	 * この DataSource に名前を設定します。
	 *
	 * @param	dataSourceName_
	 * 			データソース名。
	 * @throws java.lang.IllegalStateException
	 *			初期化後には設定出来ません。
	 * @throws java.lang.IllegalArgumentException
	 *			既に存在します。
	 */
	public void setDataSourceName(String dataSourceName_)
	{
		if(initialized){
			throw new IllegalStateException("Cannot set Data Source properties after DataSource has been used");
		}
		if(this._dataSourceName != null && dataSourceName_ != null && dataSourceName_.equals(this._dataSourceName)){
			return ;
		}
		synchronized(dataSources){
			if(getDataSource(dataSourceName_) != null){
				throw new IllegalArgumentException("DataSource with name '" + dataSourceName_ + "' already exists!");
			}
			if(this._dataSourceName != null){
				dataSources.remove(this._dataSourceName);
			}
			this._dataSourceName = dataSourceName_;
			dataSources.put(dataSourceName_, this);
		}
	}

	/**
	 * この DataSource を初期化します。
	 *
	 * @throws	java.sql.SQLException
	 *			十分な物理接続を作成することができないとき起こります。
	 */
	public void initialize() throws SQLException
	{
		synchronized(lock){
			source = createConnectionPool();
			source.setDatabaseName(getDatabaseName());
			source.setPassword(getPassword());
			source.setPortNumber(getPortNumber());
			source.setServerName(getServerName());
			source.setUser(getUser());
			source.setNetworkProtocol(getNetworkProtocol());
			source.setUrl(getUrl());
			source.setLogWriter(this.getLogWriter());
			while(available.size() < initialConnections){
				available.push(source.getPooledConnection());
			}
			initialized = true;
		}
	}

	boolean isInitialized() {
		return initialized;
	}

	/**
	 * この DataSource に使用するためにConnectionPoolを作成します。
	 */
	ConnectionPoolDataSource createConnectionPool() {
		return new ConnectionPoolDataSource();
	}

	/**
	 * ユーザとパスワードがデフォルト接続プールに評価するのと同じでない場合、非プールされた接続を得ます。
	 *
	 * @return	java.sql.Connection
	 * @throws java.sql.SQLException
	 *			どんなプールされた接続も手があいていなくて、新しい物理接続を作成することができないとき起こります。
	 */
	public Connection getConnection(String user_, String password_) throws SQLException
	{
		if(user_ == null || (user_.equals(getUser()) && ((password_ == null && getPassword() == null) || (password_ != null && password_.equals(getPassword())))))
		{
			return getConnection();
		}
		// 初期化されていない
		if(!initialized){
			initialize();
		}
		return super.getConnection(user_, password_);
	}

	/**
	 * 接続プールから接続を取得します。
	 *
	 * @return	プールされた接続
	 * @throws	SQLException
	 *			どんなプールされた接続も手があいていなくて、新しい物理接続を作成することができないとき起こります。
	 */
	public Connection getConnection() throws SQLException
	{
		if(!initialized){
			initialize();
		}
		return getPooledConnection();
	}

	/**
	 * 使用中であるか否かに関係なく、この DataSource およびすべてのプールされた接続を終えます。
	 */
	public void close()
	{
		synchronized(lock ){
			while(available.size() > 0){
				PooledConnection pci = (PooledConnection)available.pop();
				try{
					pci.close();
				}catch(SQLException e){
				}
			}
			available = null;
			while(used.size() > 0){
				PooledConnection pci = (PooledConnection)used.pop();
				pci.removeConnectionEventListener(connectionEventListener);
				try{
					pci.close();
				}catch(SQLException e){
				}
			}
			used = null;
		}
		removeStoredDataSource();
	}

	void removeStoredDataSource() {
		synchronized(dataSources){
			dataSources.remove( _dataSourceName );
		}
	}

	/**
	* プールから接続を得ます。
	* 最大限界で存在しているなら利用可能なものを得るか、または新しいものを作成します。
	* すべての中古のものと新しいものが最大を超えると失敗します。
	*
	* @return	java.sql.Connection
	*/
	private Connection getPooledConnection() throws SQLException
	{
		javax.sql.PooledConnection pcn = null;
		synchronized(lock){
			if(available == null){
				throw new Unexpected();
			}
			int nTimeOut = this.getLoginTimeout();
			int nTimeCount = 0;
			while(true){
				if(available.size() > 0){
					pcn = (javax.sql.PooledConnection)available.pop();
					used.push(pcn);
					break;
				}
				if(maxConnections == 0 || used.size() < maxConnections){
					source.setLogWriter(this.getLogWriter());
					pcn = source.getPooledConnection();
					used.push(pcn);
					break;
				}else{
					try{
						// Wake up every second at a minimum
						if(nTimeOut>0){
							nTimeCount++;
							if((nTimeCount)<=nTimeOut){
								lock.wait(1000L);
							}else{
								throw new LockTimeout();
							}
						}else{
							lock.wait(1000L);
						}
					}catch(InterruptedException e){
					}
				}
			}
		}
		pcn.addConnectionEventListener(connectionEventListener);
		return pcn.getConnection();
	}

	/**
	 * プールされた接続が終えられるか、または致命的な誤りがプールされた接続のときに発生するとき通知されます。
	 * これは接続が未使用としてマークされる唯一の方法です。
	 */
	private ConnectionEventListener connectionEventListener = new ConnectionEventListener()
			{
				public void connectionClosed(ConnectionEvent event_)
				{
					javax.sql.PooledConnection pconn
						= (javax.sql.PooledConnection)event_.getSource();
					pconn.removeConnectionEventListener(this);
					synchronized(lock){
						if(available == null){
							// 既にcloseされています
							return;
						}
						boolean removed = used.remove(pconn);
						if(removed){
							if (maxConnections == 0) {
								// プールしないので、クローズする
								try {
									pconn.close();
								} catch (Exception e) {
									// 例外は無視
								}
							} else {
								available.push(pconn);
							}
							// 空きあり
							lock.notify();
						}else{
							// 接続エラー
						}
					}
				}

				/**
				 * これは致命的な誤りによって呼ばれるだけです。
				 * 物理接続がその後、役に立たなくプールから外されるべきであるところで。
				 */
				public void connectionErrorOccurred(ConnectionEvent event_)
				{
					((javax.sql.PooledConnection) event_.getSource()).removeConnectionEventListener(this);
					synchronized(lock){
						if(available == null){
							// 既にcloseされています
							return;
						}
						used.remove(event_.getSource());
						try {
							((javax.sql.PooledConnection)event_.getSource()).close();
						} catch (Exception e) {
							// 例外は無視
						}
						lock.notify();
					}
				}
			};

	/**
	 * このDataSourceのためにカスタム特性を「スーパー-クラス」で定義された特性に追加します。
	 *
	 * @return	javax.naming.Reference
	 */
	public Reference getReference() throws NamingException
	{
		Reference rf = super.getReference();
		rf.add(new StringRefAddr("dataSourceName", _dataSourceName));
		if(initialConnections > 0){
			rf.add(new StringRefAddr("initialConnections", Integer.toString(initialConnections)));
		}
		if(maxConnections > 0){
			rf.add(new StringRefAddr("maxConnections", Integer.toString(maxConnections)));
		}
		return rf;
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public <T> T unwrap(Class<T> iface) throws SQLException {
		// サポート外

		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 常に<code>false</code>を返します
	 */
	@Override
	public boolean isWrapperFor(Class<?> iface) throws SQLException {
		// unwrap を実装している場合は true を返すが、
		// それ以外の場合は false を返す

		return false;
	}


	/**
	 * ドライバで使用されるすべてのロガーの親ロガーを返す。
	 * <B>[サポート外！]</B>
	 *
	 * @return	Loggerオブジェクト。
	 * @throws NotSupported
	 *
	 */
	@Override
	public Logger getParentLogger() throws NotSupported {
		throw new NotSupported();
	}
}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2011, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
