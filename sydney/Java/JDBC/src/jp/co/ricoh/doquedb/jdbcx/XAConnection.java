// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// XAConnection.java -- JDBCX の XAConnectionクラス
//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2015, 2023 Ricoh Company, Ltd.
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

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.sql.CallableStatement;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

import javax.sql.ConnectionEvent;
import javax.sql.ConnectionEventListener;
import javax.sql.StatementEventListener;
import javax.transaction.xa.XAException;
import javax.transaction.xa.XAResource;
import javax.transaction.xa.Xid;

import jp.co.ricoh.doquedb.exception.Unexpected;

/**
 * 接続プール管理のフックを提供するオブジェクトです。
 * PooledConnection オブジェクトは、データソースへの物理接続を表します。
 * アプリケーションがある接続で終了したとき、その接続は閉じられずにリサイクルされます。
 * そのため、生成する必要のある接続数を減らすことができます。
 *
 * アプリケーションプログラマが PooledConnection インタフェースを直接使用することはありません。
 * むしろ、PooledConnection インタフェースは、接続のプーリングを管理する中間層インフラストラクチャが使用します。
 *
 * XAConnectionは上記のインターフェースを継承しています。
 *
 */
public class XAConnection implements javax.sql.XAConnection, javax.transaction.xa.XAResource
{
	private List listeners = new LinkedList();
	private Connection conct;
	private ConnectionHandler last;
	private boolean autoCommit;

	private Xid[] recoverXids;

	/**
	 * 新しいXAConnectionを作成します。
	 *
	 * @param	con_
	 *			コネクションオブジェクト
	 * @param	autoCommit_
	 *			オートコミットかどうか
	 */
	public XAConnection(Connection con_, boolean autoCommit_)
	{
		this.conct = con_;
		this.autoCommit = autoCommit_;
		this.recoverXids = null;
	}

	/**
	 * 指定したイベントリスナーを登録して、この PooledConnection オブジェクトで
	 * イベントが発生したときに通知されるようにします。
	 *
	 * @param	connectionEventListener_
	 *			インタフェースを実装し、接続が閉じたかエラーが発生したときに通知されるようにするコンポーネント。
	 *			通常は接続プール管理プログラム
	 */
	public void addConnectionEventListener(ConnectionEventListener connectionEventListener_)
	{
		listeners.add(connectionEventListener_);
	}

	/**
	 * 指定したイベントリスナーを、この PooledConnection オブジェクトで
	 * イベントが発生したときに通知されるコンポーネントのリストから削除します。
	 *
	 * @param	connectionEventListener_
	 *			インタフェースを実装し、リスナーとして登録されたコンポーネント。通常は接続プール管理プログラム
	 */
	public void removeConnectionEventListener(ConnectionEventListener connectionEventListener_)
	{
		listeners.remove(connectionEventListener_);
	}

	/**
	 * この PooledConnection オブジェクトが表す物理接続を閉じます。
	 *
	 * @throws	SQLException
	 *			データベースアクセスエラーが発生した場合
	 */
	public void close() throws SQLException
	{
		if(last != null){
			last.close();
			if(!conct.getAutoCommit()){
				try{
					conct.rollback();
				}catch(SQLException ex){
					ex.printStackTrace();
				}
			}
		}
		try{
			conct.close();
		}finally{
			conct = null;
		}
	}

	/**
	 * この PooledConnection オブジェクトが表す物理接続のオブジェクトハンドルを作成します。
	 * 返されるオブジェクトは、プールされている物理接続 (この PooledConnection オブジェクト) を
	 * 参照するアプリケーションコードで使用する一時ハンドルになります。
	 *
	 * @return	この PooledConnection オブジェクトのハンドルである <code>java.sql.Connection</code> オブジェクト
	 * @throws	SQLException
	 *			データベースアクセスエラーが発生した場合
	 */
	public Connection getConnection() throws SQLException
	{
		if(conct == null){
			// 誤って登録されたリスナーに通知
			SQLException sqlExcep = new Unexpected();
			fireConnectionFatalError(sqlExcep);
			throw sqlExcep;
		}
		// 新しい接続を開いている間のどんな誤りoccuresであるならも、リスナーは通知されなければなりません。
		// これは機会をelliminateの悪いプールされた接続への接続プールに与えます。
		try{
			if(last != null){
				last.close();
				if(!conct.getAutoCommit()){
					try{
						conct.rollback();
					}catch(SQLException e){
					}
				}
				conct.clearWarnings();
			}
			conct.setAutoCommit(autoCommit);
		}catch(SQLException sqlEx){
			fireConnectionFatalError(sqlEx);
			throw (SQLException)sqlEx.fillInStackTrace();
		}
		ConnectionHandler cHandler = new ConnectionHandler(conct);
		last = cHandler;

//		Connection proxyConn = (Connection)Proxy.newProxyInstance(getClass().getClassLoader(), new Class[]{Connection.class, SydConnection.class}, cHandler);
		Connection proxyConn = (Connection)Proxy.newProxyInstance(getClass().getClassLoader(), new Class[]{Connection.class}, cHandler);
		last.setProxy(proxyConn);
		return proxyConn;
	}

	// 閉じられた
	void fireConnectionClosed()
	{
		ConnectionEvent evnt = null;
		// メソッド呼び出し中に立ち退ける様にコピーする
		ConnectionEventListener[] local = (ConnectionEventListener[]) listeners.toArray(new ConnectionEventListener[listeners.size()]);
		for(int i = 0; i < local.length; i++){
			ConnectionEventListener listener = local[i];
			if(evnt == null){
				evnt = new ConnectionEvent(this);
			}
			listener.connectionClosed(evnt);
		}
	}

	// 接続エラー
	void fireConnectionFatalError(SQLException ex_)
	{
		ConnectionEvent evnt = null;
		// メソッド呼び出し中に立ち退ける様にコピーする
		ConnectionEventListener[] local = (ConnectionEventListener[])listeners.toArray(new ConnectionEventListener[listeners.size()]);
		for(int i = 0; i < local.length; i++){
			ConnectionEventListener listener = local[i];
			if(evnt == null){
				evnt = new ConnectionEvent(this, ex_);
			}
			listener.connectionErrorOccurred(evnt);
		}
	}

	// Classes we consider fatal.
	private static String[] fatalClasses = {
		"08",  // connection error
		"53",  // insufficient resources

		// nb: not just "57" as that includes query cancel which is nonfatal
		"57P01",  // admin shutdown
		"57P02",  // crash shutdown
		"57P03",  // cannot connect now

		"58",  // system error (backend)
		"60",  // system error (driver)
		"99",  // unexpected error
		"F0",  // configuration file error (backend)
		"XX",  // internal error (backend)
	};

	private static boolean isFatalState(String state_) {
		if(state_ == null)			// 情報無し
			return true;
		if(state_.length() < 2)		// クラス情報無し
			return true;

		for(int i = 0; i < fatalClasses.length; ++i)
			if(state_.startsWith(fatalClasses[i]))
				return true;		// 致命的

		return false;
	}

	/**
	 * Fires a connection error event, but only if we
	 * think the exception is fatal.
	 *
	 * @param e the SQLException to consider
	 */
	private void fireConnectionError(SQLException ex_)
	{
		if(!isFatalState(ex_.getSQLState()))
			return;
		fireConnectionFatalError(ex_);
	}

	// Connectionを実装するクラスを宣言することの代わりにダイナミックなプロキシを使用して、
	// Connectionインタフェースを通るすべての呼び出しを扱ってください。
	private class ConnectionHandler implements InvocationHandler
	{
		private Connection conn;
		private Connection proxy; // the Connection the client is currently using, which is a proxy
		private boolean automatic = false;

		public ConnectionHandler(Connection conn_)
		{
			this.conn = conn_;
		}

		public Object invoke(Object proxy_, Method method_, Object[] args_)
		throws Throwable
		{
			// From Object
			if(method_.getDeclaringClass().getName().equals("java.lang.Object")){
				if(method_.getName().equals("toString")){
					return "Pooled connection wrapping physical connection " + conn;
				}
				if(method_.getName().equals("hashCode")){
					return new Integer(conn.hashCode());
				}
				if(method_.getName().equals("equals")){
					if(args_[0] == null){
						return Boolean.FALSE;
					}
					try{
						return Proxy.isProxyClass(args_[0].getClass()) && ((ConnectionHandler) Proxy.getInvocationHandler(args_[0])).conn == conn ? Boolean.TRUE : Boolean.FALSE;
					}catch(ClassCastException ex){
						return Boolean.FALSE;
					}
				}
				try{
					return method_.invoke(conn, args_);
				}catch(InvocationTargetException ex){
					throw ex.getTargetException();
				}
			}
			// All the rest is from the Connection or PGConnection interface
			if(method_.getName().equals("isClosed")){
				return conn == null ? Boolean.TRUE : Boolean.FALSE;
			}
			if(conn == null && !method_.getName().equals("close")){
				throw new Unexpected();
			}
			if(method_.getName().equals("close")){
				// we are already closed and a double close
				// is not an error.
				if(conn == null)
					return null;

				SQLException ex = null;
				if(!conn.getAutoCommit()){
					try{
						conn.rollback();
					}catch(SQLException exp){
						ex = exp;
					}
				}
				conn.clearWarnings();
				conn = null;
				proxy = null;
				last = null;
				fireConnectionClosed();
				if(ex != null){
					throw ex;
				}
				return null;
			}

			// From here on in, we invoke via reflection, catch exceptions,
			// and check if they're fatal before rethrowing.

			try{
				if(method_.getName().equals("createStatement")){
					Statement stm = (Statement)method_.invoke(conn, args_);
//					return Proxy.newProxyInstance(getClass().getClassLoader(), new Class[]{Statement.class, jp.co.ricoh.doquedb.jdbcx.SydStatement.class}, new StatementHandler(this, stm));
					return Proxy.newProxyInstance(getClass().getClassLoader(), new Class[]{Statement.class}, new StatementHandler(this, stm));

				}else if(method_.getName().equals("prepareCall")){
					Statement stm = (Statement)method_.invoke(conn, args_);
//					return Proxy.newProxyInstance(getClass().getClassLoader(), new Class[]{CallableStatement.class, jp.co.ricoh.doquedb.jdbcx.SydStatement.class}, new StatementHandler(this, stm));
					return Proxy.newProxyInstance(getClass().getClassLoader(), new Class[]{CallableStatement.class}, new StatementHandler(this, stm));

				}else if(method_.getName().equals("prepareStatement")){
					Statement stm = (Statement)method_.invoke(conn, args_);
//					return Proxy.newProxyInstance(getClass().getClassLoader(), new Class[]{PreparedStatement.class, jp.co.ricoh.doquedb.jdbcx.SydStatement.class}, new StatementHandler(this, stm));
					return Proxy.newProxyInstance(getClass().getClassLoader(), new Class[]{PreparedStatement.class}, new StatementHandler(this, stm));

				}else{
					return method_.invoke(conn, args_);
				}
			}catch(InvocationTargetException ex){
				Throwable tw = ex.getTargetException();
				if (tw instanceof SQLException)
					fireConnectionError((SQLException)tw); // Tell listeners about exception if it's fatal
				throw tw;
			}
		}

		Connection getProxy(){
			return proxy;
		}

		void setProxy(Connection proxy_){
			this.proxy = proxy_;
		}

		public void close()
		{
			if(conn != null){
				automatic = true;
			}
			conn = null;
			proxy = null;
			// No close event fired here: see JDBC 2.0 Optional Package spec section 6.3
		}

		public boolean isClosed(){
			return conn == null;
		}
	}

	// クラスがStatementを実行して、PreparedStatementと、
	// CallableStatementであると宣言することの代わりに、ダイナミックなプロキシを使用します。Statementインタフェースを通るすべての呼び出しを扱ってください
	//
	// StatementHandlerが、getConnection方法のために適切なConnectionプロキシを返すのに必要です。
	private class StatementHandler implements InvocationHandler{
		private XAConnection.ConnectionHandler conn;
		private Statement stm;

		public StatementHandler(XAConnection.ConnectionHandler conn_, Statement stm_) {
			this.conn = conn_;
			this.stm = stm_;
		}
		public Object invoke(Object proxy_, Method method_, Object[] args_)
			throws Throwable
		{
			// From Object
			if(method_.getDeclaringClass().getName().equals("java.lang.Object")){
				if(method_.getName().equals("toString")){
					return "Pooled statement wrapping physical statement " + stm;
				}
				if(method_.getName().equals("hashCode")){
					return new Integer(stm.hashCode());
				}
				if(method_.getName().equals("equals")){
					if(args_[0] == null){
						return Boolean.FALSE;
					}
					try{
						return Proxy.isProxyClass(args_[0].getClass()) && ((StatementHandler)Proxy.getInvocationHandler(args_[0])).stm == stm ? Boolean.TRUE : Boolean.FALSE;
					}catch(ClassCastException ex){
						return Boolean.FALSE;
					}
				}
				return method_.invoke(stm, args_);
			}
			// All the rest is from the Statement interface
			if(method_.getName().equals("close")){
				// closing an already closed object is a no-op
				if(stm == null || conn.isClosed())return null;

				try{
					stm.close();
				}finally{
					conn = null;
					stm = null;
				}
				return null;
			}
			if(stm == null || conn.isClosed()){
				throw new Unexpected();
			}
			if(method_.getName().equals("getConnection")){
				return conn.getProxy(); // the proxied connection, not a physical connection
			}
			try{
				return method_.invoke(stm, args_);
			}catch(InvocationTargetException ex){
				Throwable tw = ex.getTargetException();
				if(tw instanceof SQLException)
					fireConnectionError((SQLException)tw); // Tell listeners about exception if it's fatal
				throw tw;
			}
		}
	}

	/**
	 * XAResourceを取得する。
	 *
	 * @return	<code>javax.transaction.xa.XAResource</code> オブジェクト
	 * @throws	SQLException
	 *			データベースアクセスエラーが発生した場合
	 */
	public XAResource getXAResource() throws SQLException {
		return this;
	}

	/**
	 * トランザクションブランチ識別子で識別されるトランザクションブランチをコミットする。
	 *
	 * トランザクションブランチのコミット ::= XA COMMIT トランザクションブランチ識別子 [ONE PHASE]
	 *
	 * @param	xid_
	 *			グローバルトランザクション識別子
	 * @param	onePhase_
	 *			ONE PHASE句指定の有無
	 * @throws	XA_ProtocolError
	 *			ONE PHASE句を指定するときに『待機中』もしくは『コミット準備完了』でない場合
	 * @throws	XA_UnknownIdentifier
	 *			トランザクションブランチ識別子で識別されるトランザクションブランチが存在しない場合
	 * @throws	XA_Heur
	 *			トランザクションブランチの状態が『ヒューリスティックな解決済』である場合
	 */
	public void commit(Xid xid_, boolean onePhase_) throws XAException {

		String sql = new String();
		sql = "XA COMMIT ";

		// トランザクションブランチ識別子を追加
		sql += getTransactionBranchIdentifier(xid_);

		// リソースマネージャは 1 段階のコミットプロトコルを使用して、xid のために行われた処理をコミットする必要がある
		if (onePhase_) {
			sql += " ONE PHASE";
		}

		try
		{
			// sqlの実行
			executeStatement(sql);
		}
		catch (SQLException ex_)
		{
			jp.co.ricoh.doquedb.jdbcx.exception.ErrorCode.throwXAException(ex_);
		}

	}

	/**
	 * SUSPEND句またはSUSPEND FOR MIGRATE句を指定しないとき、
	 * トランザクションブランチ識別子で識別されるトランザクションブランチを待機する。
	 * SUSPEND句またはSUSPEND FOR MIGRATE句を指定すると、トランザクションブランチの実行を中断できる。
	 *
	 * トランザクションブランチの待機 ::= XA END トランザクションブランチ識別子 [ SUSPEND [ FOR MIGRATE ] ]
	 *
	 * SUSPEND句およびSUSPEND FOR MIGRATE句は現在、サポートしない。
	 *
	 * @param	xid_
	 *			グローバルトランザクション識別子
	 * @param	flags_
	 *			SUSPEND句の指定等
	 * @throws	XA_ProtocolError
	 *			・SUSPEND句またはSUSPEND FOR MIGRATE句を指定しないときに、
	 *			　トランザクションブランチの状態が『データ操作中』・『中断中』でない場合
	 *			・SUSPEND句またはSUSPEND FOR MIGRATE句を指定したときに、
	 *			　トランザクションブランチの状態が『データ操作中』でない場合
	 *			・このSQL文を実行するセッションでXA STARTにより開始・合流・再開したトランザクションブランチでない場合
	 * @throws	XA_UnknownIdentifier
	 *			トランザクションブランチ識別子で識別されるトランザクションブランチが存在しない場合
	 */
	public void end(Xid xid_, int flags_) throws XAException {

		String sql = new String();
		sql = "XA END ";

		// トランザクションブランチ識別子を追加
		sql += getTransactionBranchIdentifier(xid_);

		// SUSPEND句の指定等
		switch (flags_) {
		case TMSUCCESS:
			// 処理部分は正常に完了
			break;
		case TMSUSPEND:
			// トランザクションブランチは不完全な状態で一時的に中断
			//sql += " SUSPEND";
			//break;
			throwNoSupport();
		case TMFAIL:
			// 処理部分は失敗
			break;
		default:
			throw new XAException(XAException.XAER_INVAL);
		}

		try
		{
			// sqlの実行
			executeStatement(sql);
		}
		catch (SQLException ex_)
		{
			jp.co.ricoh.doquedb.jdbcx.exception.ErrorCode.throwXAException(ex_);
		}

	}

	/**
	 * 状態が『ヒューリスティックな解決済』のトランザクションブランチ識別子で識別される
	 * トランザクションブランチのどうやって解決したかの情報を抹消する。
	 *
	 * ヒューリステックに解決済のトランザクションブランチの抹消 ::= XA FORGET トランザクションブランチ識別子
	 *
	 * @param	xid_
	 *			グローバルトランザクション識別子
	 * @throws	XA_UnknownIdentifier
	 *			トランザクションブランチ識別子で識別されるトランザクションブランチが存在してかつ
	 *			『ヒューリスティックな解決済』でない場合
	 */
	public void forget(Xid xid_) throws XAException {
		String sql = new String();
		sql = "XA FORGET ";

		// トランザクションブランチ識別子を追加
		sql += getTransactionBranchIdentifier(xid_);

		try
		{
			// sqlの実行
			executeStatement(sql);
		}
		catch (SQLException ex_)
		{
			jp.co.ricoh.doquedb.jdbcx.exception.ErrorCode.throwXAException(ex_);
		}
	}

	/**
	 * この XAResource インスタンスに対して設定された現在のトランザクションタイムアウト値を取得します。
	 *
	 * @return	トランザクションタイムアウト値
	 * @throws	XAException
	 */
	public int getTransactionTimeout() throws XAException {
		// TODO 自動生成されたメソッド・スタブ
		return 0;
	}

	/**
	 * ターゲットオブジェクトで表されるリソースマネージャインスタンスが
	 * パラメータ xares で表されるリソースマネージャインスタンスと同じかどうかを判定する。
	 *
	 * @param	xares_
	 *			ターゲットオブジェクトのリソースマネージャインスタンスと比較される
	 *			リソースマネージャインスタンスを持つ XAResource オブジェクト
	 * @return 同じ RM インスタンスである場合は true、そうでない場合は false
	 * @throws	XAException
	 *
	 */
	public boolean isSameRM(XAResource xares_) throws XAException {
		if (xares_ instanceof XAConnection) {
			return true;
		}
		return false;
	}

	/**
	 * トランザクションブランチ識別子で識別されるトランザクションブランチをコミット準備する。
	 *
	 * トランザクションブランチのコミット準備 ::= XA PREPARE トランザクションブランチ識別子
	 *
	 * @param	xid_
	 *			グローバルトランザクション識別子
	 * @return XA_RDONLY または XA_OK
	 * @throws	XA_ProtocolError
	 * 			トランザクションブランチの状態が『待機中』でない場合
	 * @throws	XA_UnknownIdentifier
	 *			トランザクションブランチ識別子で識別されるトランザクションブランチが存在しない場合
	 */
	public int prepare(Xid xid_) throws XAException {

		// sqlの作成
		String sql = new String();
		sql = "XA PREPARE ";

		// トランザクションブランチ識別子を追加
		sql += getTransactionBranchIdentifier(xid_);

		// sqlの実行
		try
		{
			executeStatement(sql);

			if ( this.conct.isReadOnly() )
			{
				return XA_RDONLY;
			}
		}
		catch (SQLException ex_)
		{
			jp.co.ricoh.doquedb.jdbcx.exception.ErrorCode.throwXAException(ex_);
		}

		return XA_OK;
	}

	/**
	 * 状態が『コミット準備完了』・『ヒューリスティックな解決済』のトランザクションブランチのトランザクションブランチ識別子を得る。
	 *
	 * コミット準備済トランザクションブランチの確認 ::= XA RECOVER
	 *
	 * @param	flag_
	 *			TMSTARTRSCAN、TMENDRSCAN、TMNOFLAGSのいずれか
	 * @return XA_RDONLY または XA_OK
	 * @throws	XA_ProtocolError
	 * 			トランザクションブランチの状態が『待機中』でない場合
	 * @throws	XA_UnknownIdentifier
	 *			トランザクションブランチ識別子で識別されるトランザクションブランチが存在しない場合
	 */
	public Xid[] recover(int flag_) throws XAException {


		// TMSTARTRSCAN、TMENDRSCAN、TMNOFLAGS 以外の場合
		if (flag_ != TMSTARTRSCAN && flag_ != TMENDRSCAN && flag_ != TMNOFLAGS) {
			throw new XAException(XAException.XAER_INVAL);
		}

		ResultSet resultSet = null;
		Statement statement = null;

		List<jp.co.ricoh.doquedb.jdbcx.Xid> xidList = new ArrayList<jp.co.ricoh.doquedb.jdbcx.Xid>();

		// コマンドを実行し、調査結果をリストに格納
		try {

			statement = this.conct.createStatement();

			resultSet = statement.executeQuery("XA RECOVER");

			while (resultSet.next()) {

				byte[] globalTransactionIdentifier = resultSet.getBytes(1);
				byte[] branchQualifier = resultSet.getBytes(2);
				int formatId = resultSet.getInt(3);

				xidList.add(new jp.co.ricoh.doquedb.jdbcx.Xid(globalTransactionIdentifier, branchQualifier, formatId));
			}

			if (resultSet != null) {
				resultSet.close();
			}

			if (statement != null) {
				statement.close();
			}

		} catch (SQLException ex_) {
			jp.co.ricoh.doquedb.jdbcx.exception.ErrorCode.throwXAException(ex_);
		}


		return getRecoverXids(flag_, xidList);
	}

	private Xid[] getRecoverXids(int flag_, List<jp.co.ricoh.doquedb.jdbcx.Xid> xidList_) {

		// 回復調査開始
		if (flag_ == TMSTARTRSCAN) {
			recoverXids = SetXid(xidList_);
			return recoverXids;
		}
		// 回復調査中・終了
		else
		{
			// 前回がnull
			if ( recoverXids == null || recoverXids.length <= 0)
			{
				recoverXids = SetXid(xidList_);
				return recoverXids;
			}
			// 前回の結果とサイズが違う
			if ( xidList_.size() != recoverXids.length)
			{
				recoverXids = SetXid(xidList_);
				return recoverXids;
			}
			// 前回と結果が同じか調査
			Xid[] resultXids = null;
			resultXids = SetXid(xidList_);
			boolean diffFlg = true;
			for (int i = 0 ; i < recoverXids.length ; i++ )
			{

				if (!resultXids[i].equals(recoverXids[i]))
				{
					// 前回と違う
					diffFlg = false;
					recoverXids = SetXid(xidList_);
					break;
				}
			}

			// 前回と同じ場合
			if ( diffFlg )
			{
				return new Xid[0];
			}

			return recoverXids;
		}

	}

	private Xid[] SetXid(List<jp.co.ricoh.doquedb.jdbcx.Xid> xidList_) {
		Xid[] setXid = null;
		setXid = new Xid[xidList_.size()];
		setXid = (Xid[]) xidList_.toArray(new Xid[0]);
		return setXid;
	}

	/**
	 * トランザクションブランチ識別子で識別されるトランザクションブランチをロールバックする。
	 *
	 * トランザクションブランチのロールバック ::= XA ROLLBACK トランザクションブランチ識別子
	 *
	 * @param	xid_
	 *			グローバルトランザクション識別子
	 * @throws	XA_ProtocolError
	 * 			トランザクションブランチの状態が『待機中』・『コミット準備完了』・『ロールバックのみ可』でない場合
	 * @throws	XA_UnknownIdentifier
	 *			トランザクションブランチ識別子で識別されるトランザクションブランチが存在しない場合
	 * @throws XA_Heur
	 * 			トランザクションブランチの状態が『ヒューリスティックな解決済』である場合
	 */
	public void rollback(Xid xid_) throws XAException {
		String sql = new String();
		sql = "XA ROLLBACK ";

		// トランザクションブランチ識別子を追加
		sql += getTransactionBranchIdentifier(xid_);

		try
		{
			// sqlの実行
			executeStatement(sql);
		}
		catch (SQLException ex_)
		{
			jp.co.ricoh.doquedb.jdbcx.exception.ErrorCode.throwXAException(ex_);
		}
	}
	/**
	 * この XAResource インスタンスの現在のトランザクションタイムアウト値を設定します。
	 *
	 * @param	timeout_
	 * 			トランザクションタイムアウト値
	 * @return	トランザクションタイムアウト値が正常に設定された場合は true、そうでない場合は false
	 * @throws	XAException
	 */
	public boolean setTransactionTimeout(int timeout_) throws XAException {
		// TODO 自動生成されたメソッド・スタブ
		return false;
	}

	/**
	 * トランザクションブランチ識別子で識別されるトランザクションブランチを開始する。
	 *
	 * トランザクションブランチの開始 ::= XA START トランザクションブランチ識別子
	 * 		[ { <トランザクションモード>
	 * 			[ { <カンマ> <トランザクションモード> } ... ] } | JOIN | RESUME ]
	 *
	 * ただし、JOIN句およびRESUME句は現在、サポートしない。
	 *
	 * @param	xid_
	 *			グローバルトランザクション識別子
	 * @param	flags_
	 *			JOIN句の指定等
	 * @throws	XA_DuplicateIdentifier
	 * 			『データ操作中』・『中断中』・『待機中』のトランザクションブランチに対し、
	 * 			後述するJOIN句やRESUME句を指定せずにこのSQL文を実行した場合
	 * @throws	XA_ProtocolError
	 *			『コミット準備完了』・『ヒューリスティックな解決済』・『ロールバックのみ可』の状態のトランザクションブランチに対し、
	 *			このSQL文を実行した場合
	 * @throws XA_UnknownIdentifier
	 * 			JOIN句やRESUME句を指定したとき、トランザクションブランチ識別子で識別されるトランザクションブランチが存在しない場合
	 */
	public void start(Xid xid_, int flg_) throws XAException {

		String sql = new String();
		sql = "XA START ";

		// トランザクションブランチ識別子を追加
		sql += getTransactionBranchIdentifier(xid_);

		try
		{
			// トランザクションモードの指定
			if ( conct.isReadOnly() )
				sql += " READ ONLY ";
			else
				sql += " READ WRITE ";

			int isolationLevel = conct.getTransactionIsolation();
			switch (isolationLevel) {
			case jp.co.ricoh.doquedb.jdbc.Connection.TRANSACTION_NONE:
				break;
			case jp.co.ricoh.doquedb.jdbc.Connection.TRANSACTION_READ_COMMITTED:
				sql += ",ISOLATION LEVEL READ COMMITTED ";
				break;
			case jp.co.ricoh.doquedb.jdbc.Connection.TRANSACTION_READ_UNCOMMITTED:
				sql += ",ISOLATION LEVEL READ UNCOMMITTED ";
				break;
			case jp.co.ricoh.doquedb.jdbc.Connection.TRANSACTION_REPEATABLE_READ:
				sql += ",ISOLATION LEVEL REPEATABLE READ ";
				break;
			case jp.co.ricoh.doquedb.jdbc.Connection.TRANSACTION_SERIALIZABLE:
				sql += ",ISOLATION LEVEL SERIALIZABLE ";
				break;
			case jp.co.ricoh.doquedb.jdbc.Connection.TRANSACTION_USING_SNAPSHOT:
				sql += ",USING SNAPSHOT ";
				break;
			default:
				break;
			}

			// JOIN句等の指定
			switch (flg_) {
			case TMJOIN:
				//sql += " JOIN";
				//break;
				throwNoSupport();
			case TMRESUME:
				//sql += " RESUME";
				//break;
				throwNoSupport();

			case TMNOFLAGS:
				break;
			default:
				throw new XAException(XAException.XAER_INVAL);
			}

			// sqlの実行
			executeStatement(sql);
		}
		catch(SQLException ex_)
		{
			jp.co.ricoh.doquedb.jdbcx.exception.ErrorCode.throwXAException(ex_);
		}
	}

	/**
	 * トランザクションブランチ識別子の作成。
	 *
	 * トランザクションブランチ識別子 ::=
	 * 		<グローバルトランザクション識別子> [, <トランザクションブランチ限定子> [, <フォーマット識別子> ] ]
	 * <グローバルトランザクション識別子> ::= <バイナリ列定数>
	 * <トランザクションブランチ限定子> ::= <バイナリ列定数>
	 * <フォーマット識別子> ::= <符号付き整数>
	 *
	 * <グローバルトランザクション識別子>と<トランザクションブランチ限定子>は64バイト以下のバイナリ列である。
	 * この組でユニークでなければならない。
	 *
	 * <トランザクションブランチ限定子>は省略できる。
	 * その場合のデフォルト値は長さ0のバイナリ列である。
	 *
	 * <フォーマット識別子>は、グローバルトランザクション識別子とトランザクションブランチ限定子の生成の仕方を表す。
	 * -1以上の整数でなければならない。
	 * 0はOSI CCR名前付けルールで生成していることを表す。
	 * -1は空値であることを表す。
	 * <フォーマット識別子>は省略できる。
	 * その場合のデフォルト値は1である。
	 *
	 * @param	xid_
	 *			グローバルトランザクション識別子
	 * @return	String
	 *			トランザクションブランチ識別子
	 */
	private static String getTransactionBranchIdentifier(Xid xid_) {

		String asString = new String();

		// Xid のグローバルトランザクション識別子部分をバイトの配列として取得
		byte[] globalTransactionID = xid_.getGlobalTransactionId();

		if (globalTransactionID != null) {
			asString = "X'";
			for (int i = 0; i < globalTransactionID.length; i++) {
				String asIntHex = Integer.toHexString(globalTransactionID[i] & 0xff);

				if ((asIntHex.length() % 2) != 0) {
					asString += "0";
				}

				asString += asIntHex;
			}
			asString += "'";
		}


		// Xid のトランザクションの枝の識別子部分をバイトの配列として取得
		byte[] branchQualifier = xid_.getBranchQualifier();

		asString += ",";
		if (branchQualifier != null) {
			asString += "X'";
			for (int i = 0; i < branchQualifier.length; i++) {
				String asIntHex = Integer.toHexString(branchQualifier[i] & 0xff);

				if ((asIntHex.length() % 2) != 0) {
					asString += "0";
				}

				asString += asIntHex;
			}
			asString += "'";
		}


		// Xid の形式識別子部分をセット
		if ( xid_.getFormatId() > -1 )
		{
			asString += ",";
			asString += Integer.toString(xid_.getFormatId());
		}

		return asString;
	}

	/**
	 * コマンドの実行
	 *
	 * @param	sql_
	 *			実行するコマンド
	 * @return ResultSet
	 * 			実行結果
	 * @throws	SQLException
	 *
	 */
	private ResultSet executeStatement(String sql_) throws SQLException {

		Statement statement = null;

		statement = this.conct.createStatement();

		statement.execute(sql_);

		ResultSet resultSet = statement.getResultSet();


		if (statement != null) {
			statement.close();
		}

		return resultSet;

	}

	/**
	 * XAExceptionのスロー
	 *
	 * @throws	XAException
	 *
	 */
	private void throwNoSupport() throws XAException {
		XAException ex = new XAException("not supported");
		ex.errorCode = XAException.XAER_RMERR;
		throw ex;
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void addStatementEventListener(StatementEventListener listener) {
		// TODO 自動生成されたメソッド・スタブ

	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public void removeStatementEventListener(StatementEventListener listener) {
		// TODO 自動生成されたメソッド・スタブ

	}
}



//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
