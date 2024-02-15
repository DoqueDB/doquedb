// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ConnectionTest.java -- jp.co.ricoh.doquedb.jdbc.Connection クラスのテスト
// 
// Copyright (c) 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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
import junit.framework.*;
import java.sql.*;
import jp.co.ricoh.doquedb.exception.*;

class InsertRun implements Runnable {

	private String	URL;
	private String _user = "root"; 
	private String _password = "doqadmin"; 

	public InsertRun(String	URL_) {
		URL = URL_;
	}

	public void run() {

		try {

			long	maxExecTime = Long.MIN_VALUE;
			long	minExecTime = Long.MAX_VALUE;

			Connection	c = DriverManager.getConnection(URL, _user, _password);
			c.setAutoCommit(false);
			Statement	s = c.createStatement();
			for (int i = 0; i < 10000; i++) {

				long	atStart = (new java.util.GregorianCalendar()).getTimeInMillis();
				s.executeUpdate("insert into t (id, name, maincategory, subcategory) values (1, 'hogehoge', 300, 401)");
				long	atEnd = (new java.util.GregorianCalendar()).getTimeInMillis();
				long	execTime = atEnd - atStart;
				if (maxExecTime < execTime) maxExecTime = execTime;
				if (minExecTime > execTime) minExecTime = execTime;
			}
			c.commit();
			s.close();
			c.close();

			System.out.println("INSERT MIN : " + minExecTime + "[msec]");
			System.out.println("INSERT MAX : " + maxExecTime + "[msec]");

		} catch (java.sql.SQLException	sqle) {
			System.err.println("EXCEPTION OCCURED : " + sqle.getMessage());
			sqle.printStackTrace();
		}
	}
}

class SelectRun implements Runnable {

	private String	URL;
	private String _user = "root"; 
	private String _password = "doqadmin"; 

	public SelectRun(String	URL_) {
		URL = URL_;
	}

	public void run() {

		try {

			long	maxExecTime = Long.MIN_VALUE;
			long	minExecTime = Long.MAX_VALUE;

			Connection	c = DriverManager.getConnection(URL, _user, _password);
			c.setTransactionIsolation(jp.co.ricoh.doquedb.jdbc.Connection.TRANSACTION_USING_SNAPSHOT);
			boolean	notYetInserted10000 = true;
			while (notYetInserted10000) {

				Statement	s = c.createStatement();
				long	atStart = (new java.util.GregorianCalendar()).getTimeInMillis();
				ResultSet	rs = s.executeQuery("select count(*) from t");
				if (rs.next()) {
					int	num = rs.getInt(1);
					switch (num) {
					case 0:
						break;
					case 10000:
						notYetInserted10000 = false;
						break;
					default:
						System.err.println("Sydney BUG No.284 !!!!!");
						break;
					}
				}
				long	atEnd = (new java.util.GregorianCalendar()).getTimeInMillis();
				long	execTime = atEnd - atStart;
				if (maxExecTime < execTime) maxExecTime = execTime;
				if (minExecTime > execTime) minExecTime = execTime;
				rs.close();
				s.close();
				Thread.sleep(10);
			}
			c.close();

			System.out.println("SELECT MIN : " + minExecTime + "[msec]");
			System.out.println("SELECT MAX : " + maxExecTime + "[msec]");

		} catch (Exception	e) {
			System.err.println("EXCEPTION OCCURED : " + e.getMessage());
			e.printStackTrace();
		}
	}
}

public class ConnectionTest extends TestBase
{
	// 警告メッセージ WM_*****

	private final static String	WM_RESULT_SET_TYPE =
		"type of ResultSet currently supported is only java.sql.ResultSet.TYPE_FORWARD_ONLY.";
	private final static String	WM_CONCURRENCY =
		"concurrency of ResultSet currently supported is only java.sql.ResultSet.CONCUR_READ_ONLY.";
	private final static String	WM_HOLDABILITY =
		"holdability of ResultSet currently supported is only java.sql.ResultSet.CLOSE_CURSORS_AT_COMMIT.";

	public ConnectionTest(String	nickname)
	{
		super(nickname);
	}

	// Connection.createStatement() のテスト
	public void test_createStatement1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		// 警告は何もないはず
		assertNull(c.getWarnings());

		s.close();
		c.close();

		// 閉じた後で呼び出すと例外 SessionNotAvailable が発生するはず
		String	SessionNotAvailableSQLState = (new SessionNotAvailable()).getSQLState();
		String	SQLState = "";
		try {
			c.createStatement();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);
	}

	// Connection.createStatement(int, int) のテスト
	public void test_createStatement2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// TYPE_FORWARD_ONLY と CONCUR_READ_ONLY
		// TYPE_FORWARD_ONLY と CONCUR_READ_ONLY の組み合わせでは警告は何もないはず
		Statement	s = null;
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_FORWARD_ONLY,
												ResultSet.CONCUR_READ_ONLY));
		assertNull(c.getWarnings());
		s.close();

		java.util.Vector	wEC = new java.util.Vector();	// error code
		java.util.Vector	wSS = new java.util.Vector();	// SQLState
		java.util.Vector	wMS = new java.util.Vector();	// message

		// TYPE_FORWARD_ONLY と CONCUR_UPDATABLE
		// TYPE_FORWARD_ONLY と CONCUR_UPDATABLE の組み合わせでは
		// CONCUR_UPDATABLE をサポートしていないので警告が出るはず
		// （警告その１）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_FORWARD_ONLY,
												ResultSet.CONCUR_UPDATABLE));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		s.close();

		// TYPE_SCROLL_INSENSITIVE と CONCUR_READ_ONLY
		// TYPE_SCROLL_INSENSITIVE と CONCUR_READ_ONLY の組み合わせでは
		// TYPE_SCROLL_INSENSITIVE をサポートしていないので警告が出るはず
		// （警告その２）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_SCROLL_INSENSITIVE,
												ResultSet.CONCUR_READ_ONLY));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		s.close();

		// TYPE_SCROLL_INSENSITIVE と CONCUR_UPDATABLE
		// TYPE_SCROLL_INSENSITIVE と CONCUR_UPDATABLE の組み合わせでは
		// どちらもサポートしていないので警告が出るはず
		// （警告その３）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		// （警告その４）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_SCROLL_INSENSITIVE,
												ResultSet.CONCUR_UPDATABLE));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		s.close();

		// TYPE_SCROLL_SENSITIVE と CONCUR_READ_ONLY
		// TYPE_SCROLL_SENSITIVE と CONCUR_READ_ONLY の組み合わせでは
		// TYPE_SCROLL_SENSITIVE をサポートしていないので警告が出るはず
		// （警告その５）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_SCROLL_SENSITIVE,
												ResultSet.CONCUR_READ_ONLY));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		s.close();

		// TYPE_SCROLL_SENSITIVE と CONCUR_UPDATABLE
		// TYPE_SCROLL_SENSITIVE と CONCUR_UPDATABLE の組み合わせでは
		// どちらもサポートしていないので警告が出るはず
		// （警告その６）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		// （警告その７）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_SCROLL_SENSITIVE,
												ResultSet.CONCUR_UPDATABLE));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		s.close();

		// 警告をクリア
		c.clearWarnings();
		assertNull(c.getWarnings());

		// -1 と CONCUR_READ_ONLY
		// -1 と CONCUR_READ_ONLY の組み合わせでは
		// -1 で例外 BadArgument が throw されるはず
		assertCreateStatementBadArgument(c, -1, ResultSet.CONCUR_READ_ONLY);

		// -1 と CONCUR_UPDATABLE
		// 同上
		assertCreateStatementBadArgument(c, -1, ResultSet.CONCUR_UPDATABLE);

		// TYPE_FORWARD_ONLY と -1
		// 同上
		assertCreateStatementBadArgument(c, ResultSet.TYPE_FORWARD_ONLY, -1);

		// TYPE_SCROLL_INSENSITIVE と -1
		// 同上
		assertCreateStatementBadArgument(c, ResultSet.TYPE_SCROLL_INSENSITIVE, -1);

		// TYPE_SCROLL_SENSITIVE と -1
		// 同上
		assertCreateStatementBadArgument(c, ResultSet.TYPE_SCROLL_SENSITIVE, -1);

		c.close();
	}

	// Connection.createStatement(int, int, int) のテスト
	public void test_createStatement3() throws Exception
	{

		Connection	c = null;
		assertNotNull(c = getConnection());

		// TYPE_FORWARD_ONLY と CONCUR_READ_ONLY と CLOSE_CURSORS_AT_COMMIT
		// TYPE_FORWARD_ONLY と CONCUR_READ_ONLY と CLOSE_CURSORS_AT_COMMIT の組み合わせでは警告は何もないはず
		Statement	s = null;
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_FORWARD_ONLY,
												ResultSet.CONCUR_READ_ONLY,
												ResultSet.CLOSE_CURSORS_AT_COMMIT));
		assertNull(c.getWarnings());
		s.close();

		java.util.Vector	wEC = new java.util.Vector();	// error code
		java.util.Vector	wSS = new java.util.Vector();	// SQLState
		java.util.Vector	wMS = new java.util.Vector();	// message

		// TYPE_FORWARD_ONLY と CONCUR_READ_ONLY と HOLD_CURSORS_OVER_COMMIT
		// TYPE_FORWARD_ONLY と CONCUR_READ_ONLY と HOLD_CURSORS_OVER_COMMIT の組み合わせでは
		// HOLD_CURSORS_OVER_COMMIT をサポートしていないので警告が出るはず
		// （警告その１）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_HOLDABILITY);
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_FORWARD_ONLY,
												ResultSet.CONCUR_READ_ONLY,
												ResultSet.HOLD_CURSORS_OVER_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		s.close();

		// TYPE_FORWARD_ONLY と CONCUR_UPDATABLE と CLOSE_CURSORS_AT_COMMIT
		// TYPE_FORWARD_ONLY と CONCUR_UPDATABLE と CLOSE_CURSORS_AT_COMMIT の組み合わせでは
		// CONCUR_UPDATABLE をサポートしていないので警告が出るはず
		// （警告その２）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_FORWARD_ONLY,
												ResultSet.CONCUR_UPDATABLE,
												ResultSet.CLOSE_CURSORS_AT_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		s.close();

		// TYPE_FORWARD_ONLY と CONCUR_UPDATABLE と HOLD_CURSORS_OVER_COMMIT
		// TYPE_FORWARD_ONLY と CONCUR_UPDATABLE と HOLD_CURSORS_OVER_COMMIT の組み合わせでは
		// CONCUR_UPDATABLE と HOLD_CURSORS_OVER_COMMIT をサポートしていないので警告が出るはず
		// （警告その３）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		// （警告その４）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_HOLDABILITY);
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_FORWARD_ONLY,
												ResultSet.CONCUR_UPDATABLE,
												ResultSet.HOLD_CURSORS_OVER_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		s.close();

		// TYPE_SCROLL_INSENSITIVE と CONCUR_READ_ONLY と CLOSE_CURSORS_AT_COMMIT
		// TYPE_SCROLL_INSENSITIVE と CONCUR_READ_ONLY と CLOSE_CURSORS_AT_COMMIT の組み合わせでは
		// TYPE_SCROLL_INSENSITIVE をサポートしていないので警告が出るはず
		// （警告その５）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_SCROLL_INSENSITIVE,
												ResultSet.CONCUR_READ_ONLY,
												ResultSet.CLOSE_CURSORS_AT_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		s.close();

		// TYPE_SCROLL_INSENSITIVE と CONCUR_READ_ONLY と HOLD_CURSORS_OVER_COMMIT
		// TYPE_SCROLL_INSENSITIVE と CONCUR_READ_ONLY と HOLD_CURSORS_OVER_COMMIT の組み合わせでは
		// TYPE_SCROLL_INSENSITIVE と HOLD_CURSORS_OVER_COMMIT をサポートしていないので
		// 警告が出るはず
		// （警告その６）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		// （警告その７）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_HOLDABILITY);
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_SCROLL_INSENSITIVE,
												ResultSet.CONCUR_READ_ONLY,
												ResultSet.HOLD_CURSORS_OVER_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		s.close();

		// TYPE_SCROLL_INSENSITIVE と CONCUR_UPDATABLE と CLOSE_CURSORS_AT_COMMIT
		// TYPE_SCROLL_INSENSITIVE と CONCUR_UPDATABLE と CLOSE_CURSORS_AT_COMMIT の組み合わせでは
		// TYPE_SCROLL_INSENSITIVE と CONCUR_UPDATABLE をサポートしていないので
		// 警告が出るはず
		// （警告その８）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		// （警告その９）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_SCROLL_INSENSITIVE,
												ResultSet.CONCUR_UPDATABLE,
												ResultSet.CLOSE_CURSORS_AT_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		s.close();

		// TYPE_SCROLL_INSENSITIVE と CONCUR_UPDATABLE と HOLD_CURSORS_OVER_COMMIT
		// TYPE_SCROLL_INSENSITIVE と CONCUR_UPDATABLE と HOLD_CURSORS_OVER_COMMIT の組み合わせでは
		// いずれもサポートしていないので警告が出るはず
		// （警告その１０）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		// （警告その１１）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		// （警告その１２）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_HOLDABILITY);
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_SCROLL_INSENSITIVE,
												ResultSet.CONCUR_UPDATABLE,
												ResultSet.HOLD_CURSORS_OVER_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		s.close();

		// TYPE_SCROLL_SENSITIVE と CONCUR_READ_ONLY と CLOSE_CURSORS_AT_COMMIT
		// TYPE_SCROLL_SENSITIVE と CONCUR_READ_ONLY と CLOSE_CURSORS_AT_COMMIT の組み合わせでは
		// TYPE_SCROLL_SENSITIVE をサポートしていないので警告が出るはず
		// （警告その１３）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_SCROLL_SENSITIVE,
												ResultSet.CONCUR_READ_ONLY,
												ResultSet.CLOSE_CURSORS_AT_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		s.close();

		// TYPE_SCROLL_SENSITIVE と CONCUR_READ_ONLY と HOLD_CURSORS_OVER_COMMIT
		// TYPE_SCROLL_SENSITIVE と CONCUR_READ_ONLY と HOLD_CURSORS_OVER_COMMIT の組み合わせでは
		// TYPE_SCROLL_SENSITIVE と HOLD_CURSORS_OVER_COMMIT をサポートしていないので
		// 警告がでるはず
		// （警告その１４）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		// （警告その１５）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_HOLDABILITY);
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_SCROLL_SENSITIVE,
												ResultSet.CONCUR_READ_ONLY,
												ResultSet.HOLD_CURSORS_OVER_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		s.close();

		// TYPE_SCROLL_SENSITIVE と CONCUR_UPDATABLE と CLOSE_CURSORS_AT_COMMIT
		// TYPE_SCROLL_SENSITIVE と CONCUR_UPDATABLE と CLOSE_CURSORS_AT_COMMIT の組み合わせでは
		// TYPE_SCROLL_SENSITIVE と CONCUR_UPDATABLE をサポートしていないので
		// 警告が出るはず
		// （警告その１６）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		// （警告その１７）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_SCROLL_SENSITIVE,
												ResultSet.CONCUR_UPDATABLE,
												ResultSet.CLOSE_CURSORS_AT_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		s.close();

		// TYPE_SCROLL_SENSITIVE と CONCUR_UPDATABLE と HOLD_CURSORS_OVER_COMMIT
		// TYPE_SCROLL_SENSITIVE と CONCUR_UPDATABLE と HOLD_CURSORS_OVER_COMMIT の組み合わせでは
		// いずれもサポートしていないので警告が出るはず
		// （警告その１８）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		// （警告その１９）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		// （警告その２０）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_HOLDABILITY);
		assertNotNull(s = c.createStatement(	ResultSet.TYPE_SCROLL_SENSITIVE,
												ResultSet.CONCUR_UPDATABLE,
												ResultSet.HOLD_CURSORS_OVER_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		s.close();

		// 警告をクリア
		c.clearWarnings();
		assertNull(c.getWarnings());

		// -1 と CONCUR_READ_ONLY と CLOSE_CURSORS_AT_COMMIT
		// -1 と CONCUR_READ_ONLY と CLOSE_CURSORS_AT_COMMIT の組み合わせでは
		// -1 で例外 BadArgument が throw されるはず
		assertCreateStatementBadArgument(	c,
											-1,
											ResultSet.CONCUR_READ_ONLY,
											ResultSet.CLOSE_CURSORS_AT_COMMIT);

		// -1 と CONCUR_READ_ONLY と HOLD_CURSORS_OVER_COMMIT
		// 同上
		assertCreateStatementBadArgument(	c,
											-1,
											ResultSet.CONCUR_READ_ONLY,
											ResultSet.HOLD_CURSORS_OVER_COMMIT);

		// -1 と CONCUR_UPDATABLE と CLOSE_CURSORS_AT_COMMIT
		// 同上
		assertCreateStatementBadArgument(	c,
											-1,
											ResultSet.CONCUR_UPDATABLE,
											ResultSet.CLOSE_CURSORS_AT_COMMIT);

		// -1 と CONCUR_UPDATABLE と HOLD_CURSORS_OVER_COMMIT
		// 同上
		assertCreateStatementBadArgument(	c,
											-1,
											ResultSet.CONCUR_UPDATABLE,
											ResultSet.HOLD_CURSORS_OVER_COMMIT);

		// TYPE_FORWARD_ONLY と -1 と CLOSE_CURSORS_AT_COMMIT
		// 同上
		assertCreateStatementBadArgument(	c,
											ResultSet.TYPE_FORWARD_ONLY,
											-1,
											ResultSet.CLOSE_CURSORS_AT_COMMIT);

		// TYPE_FORWARD_ONLY と -1 と HOLD_CURSORS_OVER_COMMIT
		// 同上
		assertCreateStatementBadArgument(	c,
											ResultSet.TYPE_FORWARD_ONLY,
											-1,
											ResultSet.HOLD_CURSORS_OVER_COMMIT);

		// TYPE_SCROLL_INSENSITIVE と -1 と CLOSE_CURSORS_AT_COMMIT
		// 同上
		assertCreateStatementBadArgument(	c,
											ResultSet.TYPE_SCROLL_INSENSITIVE,
											-1,
											ResultSet.CLOSE_CURSORS_AT_COMMIT);

		// TYPE_SCROLL_INSENSITIVE と -1 と HOLD_CURSORS_OVER_COMMIT
		// 同上
		assertCreateStatementBadArgument(	c,
											ResultSet.TYPE_SCROLL_INSENSITIVE,
											-1,
											ResultSet.HOLD_CURSORS_OVER_COMMIT);

		// TYPE_SCROLL_SENSITIVE と -1 と CLOSE_CURSORS_AT_COMMIT
		// 同上
		assertCreateStatementBadArgument(	c,
											ResultSet.TYPE_SCROLL_SENSITIVE,
											-1,
											ResultSet.CLOSE_CURSORS_AT_COMMIT);

		// TYPE_SCROLL_SENSITIVE と -1 と HOLD_CURSORS_OVER_COMMIT
		// 同上
		assertCreateStatementBadArgument(	c,
											ResultSet.TYPE_SCROLL_SENSITIVE,
											-1,
											ResultSet.HOLD_CURSORS_OVER_COMMIT);

		// TYPE_FORWARD_ONLY と CONCUR_READ_ONLY と -1
		// 同上
		assertCreateStatementBadArgument(	c,
											ResultSet.TYPE_FORWARD_ONLY,
											ResultSet.CONCUR_READ_ONLY,
											-1);

		// TYPE_FORWARD_ONLY と CONCUR_UPDATABLE と -1
		// 同上
		assertCreateStatementBadArgument(	c,
											ResultSet.TYPE_FORWARD_ONLY,
											ResultSet.CONCUR_UPDATABLE,
											-1);

		// TYPE_SCROLL_INSENSITIVE と CONCUR_READ_ONLY と -1
		// 同上
		assertCreateStatementBadArgument(	c,
											ResultSet.TYPE_SCROLL_INSENSITIVE,
											ResultSet.CONCUR_READ_ONLY,
											-1);

		// TYPE_SCROLL_INSENSITIVE と CONCUR_UPDATABLE と -1
		// 同上
		assertCreateStatementBadArgument(	c,
											ResultSet.TYPE_SCROLL_INSENSITIVE,
											ResultSet.CONCUR_UPDATABLE,
											-1);

		// TYPE_SCROLL_SENSITIVE と CONCUR_READ_ONLY と -1
		// 同上
		assertCreateStatementBadArgument(	c,
											ResultSet.TYPE_SCROLL_SENSITIVE,
											ResultSet.CONCUR_READ_ONLY,
											-1);

		// TYPE_SCROLL_SENSITIVE と CONCUR_UPDATABLE と -1
		// 同上
		assertCreateStatementBadArgument(	c,
											ResultSet.TYPE_SCROLL_SENSITIVE,
											ResultSet.CONCUR_UPDATABLE,
											-1);

		c.close();
	}

	// Connection.prepareStatement(String) のテスト
	public void test_prepareStatement1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		String	query = "select id, name from t where maincategory = ? and subcategory = ?";
		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement(query));

		// 警告は何もないはず
		assertNull(c.getWarnings());

		ps.close();

		// SQL を指定しなければ例外 BadArgument が throw されるはず
		assertPrepareStatementBadArgument(c, null);

		// SQL に空文字列を指定した場合にも例外 BadArgument が throw されるはず
		assertPrepareStatementBadArgument(c, "");

		// syntax error

		// めちゃめちゃな SQL 文
		assertPrepareStatementSQLSyntaxError(c, "hogehoge");

		// データベース中に存在しない表の指定
		assertPrepareStatementTableNotFound(c, "select name from x where id = ?");

		// [仕様変更]
		// 表中に存在しない列の指定
		//assertPrepareStatementSQLSyntaxError(c, "select x from t where id = ?");
		// SQLSyntaxError から ColumnNotFound に仕様変更。
		// 2005/05/17 
		String	columnNotFoundSQLState = (new ColumnNotFound("foo")).getSQLState();
		String	SQLState = "";
		try {
			c.prepareStatement("select x from t where id = ?");
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(columnNotFoundSQLState, SQLState);

		// スペルミス
		assertPrepareStatementSQLSyntaxError(c, "selct name from t where id = ?");
		assertPrepareStatementSQLSyntaxError(c, "select name form t where id = ?");
		assertPrepareStatementSQLSyntaxError(c, "select name from t whee id = ?");

		// 後始末
		dropTestTable(c);

		c.close();

		// 閉じた後で呼び出すと例外 SessionNotAvailable が発生するはず
		String	SessionNotAvailableSQLState = (new SessionNotAvailable()).getSQLState();
		SQLState = "";
		try {
			c.prepareStatement("select name from t where id = ?");
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);
	}

	// Connection.prepareStatement(String, int, int) のテスト
	public void test_prepareStatement2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		// SQL を指定しなければ例外 BadArgument が throw されるはず
		assertPrepareStatementBadArgument(	c,
											null,
											ResultSet.TYPE_FORWARD_ONLY,
											ResultSet.CONCUR_READ_ONLY);

		// SQL に空文字列を指定した場合にも例外 BadArgument が throw されるはず
		assertPrepareStatementBadArgument(	c,
											"",
											ResultSet.TYPE_FORWARD_ONLY,
											ResultSet.CONCUR_READ_ONLY);

		String	query = "select id, name from t where maincategory = ? and subcategory = ?";

		// TYPE_FORWARD_ONLY と CONCUR_READ_ONLY
		// TYPE_FORWARD_ONLY と CONCUR_READ_ONLY の組み合わせでは警告は何もないはず
		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_FORWARD_ONLY,
												ResultSet.CONCUR_READ_ONLY));
		assertNull(c.getWarnings());
		ps.close();

		java.util.Vector	wEC = new java.util.Vector();	// error code
		java.util.Vector	wSS = new java.util.Vector();	// SQLState
		java.util.Vector	wMS = new java.util.Vector();	// message

		// TYPE_FORWARD_ONLY と CONCUR_UPDATABLE
		// TYPE_FORWARD_ONLY と CONCUR_UPDATABLE の組み合わせでは
		// CONCUR_UPDATABLE をサポートしていないので警告が出るはず
		// （警告その１）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_FORWARD_ONLY,
												ResultSet.CONCUR_UPDATABLE));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		ps.close();

		// TYPE_SCROLL_INSENSITIVE と CONCUR_READ_ONLY
		// TYPE_SCROLL_INSENSITIVE と CONCUR_READ_ONLY の組み合わせでは
		// TYPE_SCROLL_INSENSITIVE をサポートしていないので警告が出るはず
		// （警告その２）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_SCROLL_INSENSITIVE,
												ResultSet.CONCUR_READ_ONLY));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		ps.close();

		// TYPE_SCROLL_INSENSITIVE と CONCUR_UPDATABLE
		// TYPE_SCROLL_INSENSITIVE と CONCUR_UPDATABLE の組み合わせでは
		// どちらもサポートしていないので警告が出るはず
		// （警告その３）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		// （警告その４）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_SCROLL_INSENSITIVE,
												ResultSet.CONCUR_UPDATABLE));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		ps.close();

		// TYPE_SCROLL_SENSITIVE と CONCUR_READ_ONLY
		// TYPE_SCROLL_SENSITIVE と CONCUR_READ_ONLY の組み合わせでは
		// TYPE_SCROLL_SENSITIVE をサポートしていないので警告が出るはず
		// （警告その５）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_SCROLL_SENSITIVE,
												ResultSet.CONCUR_READ_ONLY));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		ps.close();

		// TYPE_SCROLL_SENSITIVE と CONCUR_UPDATABLE
		// TYPE_SCROLL_SENSITIVE と CONCUR_UPDATABLE の組み合わせでは
		// どちらもサポートしていないので警告が出るはず
		// （警告その６）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		// （警告その７）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_SCROLL_SENSITIVE,
												ResultSet.CONCUR_UPDATABLE));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		ps.close();

		// 警告をクリア
		c.clearWarnings();
		assertNull(c.getWarnings());

		// -1 と CONCUR_READ_ONLY
		// -1 と CONCUR_READ_ONLY の組み合わせでは
		// -1 で例外 BadArgument が throw されるはず
		assertPrepareStatementBadArgument(	c,
											query,
											-1,
											ResultSet.CONCUR_READ_ONLY);

		// -1 と CONCUR_UPDATABLE
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											-1,
											ResultSet.CONCUR_UPDATABLE);

		// -1 と -1
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											-1,
											-1);

		// TYPE_FORWARD_ONLY と -1
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_FORWARD_ONLY,
											-1);

		// TYPE_SCROLL_INSENSITIVE と -1
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_SCROLL_INSENSITIVE,
											-1);

		// TYPE_SCROLL_SENSITIVE と -1
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_SCROLL_SENSITIVE,
											-1);

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// Connection.prepareStatement(String, int, int, int) のテスト
	public void test_prepareStatement3() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		// SQL を指定しなければ例外 BadArgument が throw されるはず
		assertPrepareStatementBadArgument(	c,
											null,
											ResultSet.TYPE_FORWARD_ONLY,
											ResultSet.CONCUR_READ_ONLY,
											ResultSet.CLOSE_CURSORS_AT_COMMIT);

		// SQL に空文字列を指定した場合にも例外 BadArgument が throw されるはず
		assertPrepareStatementBadArgument(	c,
											"",
											ResultSet.TYPE_FORWARD_ONLY,
											ResultSet.CONCUR_READ_ONLY,
											ResultSet.CLOSE_CURSORS_AT_COMMIT);

		String	query = "select id, name from t where maincategory = ? and subcategory = ?";

		// TYPE_FORWARD_ONLY と CONCUR_READ_ONLY と CLOSE_CURSORS_AT_COMMIT
		// TYPE_FORWARD_ONLY と CONCUR_READ_ONLY と CLOSE_CURSORS_AT_COMMIT の組み合わせでは
		// 警告は何もないはず
		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_FORWARD_ONLY,
												ResultSet.CONCUR_READ_ONLY,
												ResultSet.CLOSE_CURSORS_AT_COMMIT));
		assertNull(c.getWarnings());
		ps.close();

		java.util.Vector	wEC = new java.util.Vector();	// error code
		java.util.Vector	wSS = new java.util.Vector();	// SQLState
		java.util.Vector	wMS = new java.util.Vector();	// message

		// TYPE_FORWARD_ONLY と CONCUR_READ_ONLY と HOLD_CURSORS_OVER_COMMIT
		// TYPE_FORWARD_ONLY と CONCUR_READ_ONLY と HOLD_CURSORS_OVER_COMMIT の組み合わせでは
		// HOLD_CURSORS_OVER_COMMIT をサポートしていないので警告が出るはず
		// （警告その１）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_HOLDABILITY);
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_FORWARD_ONLY,
												ResultSet.CONCUR_READ_ONLY,
												ResultSet.HOLD_CURSORS_OVER_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		ps.close();

		// TYPE_FORWARD_ONLY と CONCUR_UPDATABLE と CLOSE_CURSORS_AT_COMMIT
		// TYPE_FORWARD_ONLY と CONCUR_UPDATABLE と CLOSE_CURSORS_AT_COMMIT の組み合わせでは
		// CONCUR_UPDATABLE をサポートしていないので警告が出るはず
		// （警告その２）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_FORWARD_ONLY,
												ResultSet.CONCUR_UPDATABLE,
												ResultSet.CLOSE_CURSORS_AT_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		ps.close();

		// TYPE_FORWARD_ONLY と CONCUR_UPDATABLE と HOLD_CURSORS_OVER_COMMIT
		// TYPE_FORWARD_ONLY と CONCUR_UPDATABLE と HOLD_CURSORS_OVER_COMMIT の組み合わせでは
		// CONCUR_UPDATABLE と HOLD_CURSORS_OVER_COMMIT をサポートしていないので
		// 警告が出るはず
		// （警告その３）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		// （警告その４）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_HOLDABILITY);
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_FORWARD_ONLY,
												ResultSet.CONCUR_UPDATABLE,
												ResultSet.HOLD_CURSORS_OVER_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		ps.close();

		// TYPE_SCROLL_INSENSITIVE と CONCUR_READ_ONLY と CLOSE_CURSORS_AT_COMMIT
		// TYPE_SCROLL_INSENSITIVE と CONCUR_READ_ONLY と CLOSE_CURSORS_AT_COMMIT の組み合わせでは
		// TYPE_SCROLL_INSENSITIVE をサポートしていないので警告が出るはず
		// （警告その５）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_SCROLL_INSENSITIVE,
												ResultSet.CONCUR_READ_ONLY,
												ResultSet.CLOSE_CURSORS_AT_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		ps.close();

		// TYPE_SCROLL_INSENSITIVE と CONCUR_READ_ONLY と HOLD_CURSORS_OVER_COMMIT
		// TYPE_SCROLL_INSENSITIVE と CONCUR_READ_ONLY と HOLD_CURSORS_OVER_COMMIT の組み合わせでは
		// TYPE_SCROLL_INSENSITIVE と HOLD_CURSORS_OVER_COMMIT をサポートしていないので
		// 警告が出るはず
		// （警告その６）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		// （警告その７）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_HOLDABILITY);
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_SCROLL_INSENSITIVE,
												ResultSet.CONCUR_READ_ONLY,
												ResultSet.HOLD_CURSORS_OVER_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		ps.close();

		// TYPE_SCROLL_INSENSITIVE と CONCUR_UPDATABLE と CLOSE_CURSORS_AT_COMMIT
		// TYPE_SCROLL_INSENSITIVE と CONCUR_UPDATABLE と CLOSE_CURSORS_AT_COMMIT の組み合わせでは
		// TYPE_SCROLL_INSENSITIVE と CONCUR_UPDATABLE をサポートしていないので
		// 警告が出るはず
		// （警告その８）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		// （警告その９）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_SCROLL_INSENSITIVE,
												ResultSet.CONCUR_UPDATABLE,
												ResultSet.CLOSE_CURSORS_AT_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		ps.close();

		// TYPE_SCROLL_INSENSITIVE と CONCUR_UPDATABLE と HOLD_CURSORS_OVER_COMMIT
		// TYPE_SCROLL_INSENSITIVE と CONCUR_UPDATABLE と HOLD_CURSORS_OVER_COMMIT の組み合わせでは
		// いずれもサポートしていないので警告が出るはず
		// （警告その１０）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		// （警告その１１）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		// （警告その１２）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_HOLDABILITY);
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_SCROLL_INSENSITIVE,
												ResultSet.CONCUR_UPDATABLE,
												ResultSet.HOLD_CURSORS_OVER_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		ps.close();

		// TYPE_SCROLL_SENSITIVE と CONCUR_READ_ONLY と CLOSE_CURSORS_AT_COMMIT
		// TYPE_SCROLL_SENSITIVE と CONCUR_READ_ONLY と CLOSE_CURSORS_AT_COMMIT の組み合わせでは
		// TYPE_SCROLL_SENSITIVE をサポートしていないので警告が出るはず
		// （警告その１３）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_SCROLL_SENSITIVE,
												ResultSet.CONCUR_READ_ONLY,
												ResultSet.CLOSE_CURSORS_AT_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		ps.close();

		// TYPE_SCROLL_SENSITIVE と CONCUR_READ_ONLY と HOLD_CURSORS_OVER_COMMIT
		// TYPE_SCROLL_SENSITIVE と CONCUR_READ_ONLY と HOLD_CURSORS_OVER_COMMIT の組み合わせでは
		// TYPE_SCROLL_SENSITIVE と HOLD_CURSORS_OVER_COMMIT をサポートしていないので
		// 警告が出るはず
		// （警告その１４）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		// （警告その１５）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_HOLDABILITY);
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_SCROLL_SENSITIVE,
												ResultSet.CONCUR_READ_ONLY,
												ResultSet.HOLD_CURSORS_OVER_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		ps.close();

		// TYPE_SCROLL_SENSITIVE と CONCUR_UPDATABLE と CLOSE_CURSORS_AT_COMMIT
		// TYPE_SCROLL_SENSITIVE と CONCUR_UPDATABLE と CLOSE_CURSORS_AT_COMMIT の組み合わせでは
		// TYPE_SCROLL_SENSITIVE と CONCUR_UPDATABLE をサポートしていないので
		// 警告が出るはず
		// （警告その１６）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		// （警告その１７）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_SCROLL_SENSITIVE,
												ResultSet.CONCUR_UPDATABLE,
												ResultSet.CLOSE_CURSORS_AT_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		ps.close();

		// TYPE_SCROLL_SENSITIVE と CONCUR_UPDATABLE と HOLD_CURSORS_OVER_COMMIT
		// TYPE_SCROLL_SENSITIVE と CONCUR_UPDATABLE と HOLD_CURSORS_OVER_COMMIT の組み合わせでは
		// いずれもサポートしていないので警告が出るはず
		// （警告その１８）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_RESULT_SET_TYPE);
		// （警告その１９）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CONCURRENCY);
		// （警告その２０）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_HOLDABILITY);
		assertNotNull(ps = c.prepareStatement(	query,
												ResultSet.TYPE_SCROLL_SENSITIVE,
												ResultSet.CONCUR_UPDATABLE,
												ResultSet.HOLD_CURSORS_OVER_COMMIT));
		// 警告のチェック
		assertSQLWarning(c.getWarnings(), wEC, wSS, wMS);
		ps.close();

		// 警告をクリア
		c.clearWarnings();
		assertNull(c.getWarnings());

		// -1 と CONCUR_READ_ONLY と CLOSE_CURSORS_AT_COMMIT
		// -1 と CONCUR_READ_ONLY と CLOSE_CURSORS_AT_COMMIT の組み合わせでは
		// -1 で例外 BadArgument が throw されるはず
		assertPrepareStatementBadArgument(	c,
										 	query,
											-1,
											ResultSet.CONCUR_READ_ONLY,
											ResultSet.CLOSE_CURSORS_AT_COMMIT);

		// -1 と CONCUR_READ_ONLY と HOLD_CURSORS_OVER_COMMIT
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											-1,
											ResultSet.CONCUR_READ_ONLY,
											ResultSet.HOLD_CURSORS_OVER_COMMIT);

		// -1 と CONCUR_READ_ONLY と -1
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											-1,
											ResultSet.CONCUR_READ_ONLY,
											-1);

		// -1 と CONCUR_UPDATABLE と CLOSE_CURSORS_AT_COMMIT
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											-1,
											ResultSet.CONCUR_UPDATABLE,
											ResultSet.CLOSE_CURSORS_AT_COMMIT);

		// -1 と CONCUR_UPDATABLE と HOLD_CURSORS_OVER_COMMIT
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											-1,
											ResultSet.CONCUR_UPDATABLE,
											ResultSet.HOLD_CURSORS_OVER_COMMIT);

		// -1 と CONCUR_UPDATABLE と -1
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											-1,
											ResultSet.CONCUR_UPDATABLE,
											-1);

		// -1 と -1 と CLOSE_CURSORS_AT_COMMIT
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											-1,
											-1,
											ResultSet.CLOSE_CURSORS_AT_COMMIT);

		// -1 と -1 と HOLD_CURSORS_OVER_COMMIT
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											-1,
											-1,
											ResultSet.HOLD_CURSORS_OVER_COMMIT);

		// -1 と -1 と -1
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											-1,
											-1,
											-1);

		// TYPE_FORWARD_ONLY と -1 と CLOSE_CURSORS_AT_COMMIT
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_FORWARD_ONLY,
											-1,
											ResultSet.CLOSE_CURSORS_AT_COMMIT);

		// TYPE_FORWARD_ONLY と -1 と HOLD_CURSORS_OVER_COMMIT
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_FORWARD_ONLY,
											-1,
											ResultSet.HOLD_CURSORS_OVER_COMMIT);

		// TYPE_FORWARD_ONLY と -1 と -1
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_FORWARD_ONLY,
											-1,
											-1);

		// TYPE_SCROLL_INSENSITIVE と -1 と CLOSE_CURSORS_AT_COMMIT
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_SCROLL_INSENSITIVE,
											-1,
											ResultSet.CLOSE_CURSORS_AT_COMMIT);

		// TYPE_SCROLL_INSENSITIVE と -1 と HOLD_CURSORS_OVER_COMMIT
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_SCROLL_INSENSITIVE,
											-1,
											ResultSet.HOLD_CURSORS_OVER_COMMIT);

		// TYPE_SCROLL_INSENSITIVE と -1 と -1
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_SCROLL_INSENSITIVE,
											-1,
											-1);

		// TYPE_SCROLL_SENSITIVE と -1 と CLOSE_CURSORS_AT_COMMIT
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_SCROLL_SENSITIVE,
											-1,
											ResultSet.CLOSE_CURSORS_AT_COMMIT);

		// TYPE_SCROLL_SENSITIVE と -1 と HOLD_CURSORS_OVER_COMMIT
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_SCROLL_SENSITIVE,
											-1,
											ResultSet.HOLD_CURSORS_OVER_COMMIT);

		// TYPE_SCROLL_SENSITIVE と -1 と -1
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_SCROLL_SENSITIVE,
											-1,
											-1);

		// TYPE_FORWARD_ONLY と CONCUR_READ_ONLY と -1
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_FORWARD_ONLY,
											ResultSet.CONCUR_READ_ONLY,
											-1);

		// TYPE_FORWARD_ONLY と CONCUR_UPDATABLE と -1
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_FORWARD_ONLY,
											ResultSet.CONCUR_UPDATABLE,
											-1);

		// TYPE_SCROLL_INSENSITIVE と CONCUR_READ_ONLY と -1
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_SCROLL_INSENSITIVE,
											ResultSet.CONCUR_READ_ONLY
											-1);

		// TYPE_SCROLL_INSENSITIVE と CONCUR_UPDATABLE と -1
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_SCROLL_INSENSITIVE,
											ResultSet.CONCUR_UPDATABLE,
											-1);

		// TYPE_SCROLL_SENSITIVE と CONCUR_READ_ONLY と -1
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_SCROLL_SENSITIVE,
											ResultSet.CONCUR_READ_ONLY,
											-1);

		// TYPE_SCROLL_SENSITIVE と CONCUR_UPDATABLE と -1
		// 同上
		assertPrepareStatementBadArgument(	c,
											query,
											ResultSet.TYPE_SCROLL_SENSITIVE,
											ResultSet.CONCUR_UPDATABLE,
											-1);

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// Connection.prepareStatement(String, int) のテスト
	public void test_prepareStatement4() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		// 自動生成キーはサポートしていないので例外 NotSupported が throw されるはず
		int	autoGeneratedKey = 1;
		String	query = "select id, name from t where maincategory = ? and subcategory = ?";
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.prepareStatement(query, autoGeneratedKey);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		// SQL を指定しない場合でもサポートしていないのだから例外 NotSupported が throw されるはず
		SQLState = "";
		try {
			c.prepareStatement(null, autoGeneratedKey);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		// SQL に空文字列を指定した場合でもサポートしていないのだから例外 NotSupported が throw されるはず
		try {
			c.prepareStatement("", autoGeneratedKey);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// Connection.prepareStatement(String, int[]) のテスト
	public void test_prepareStatement5() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		// 自動生成キーはサポートしていないので例外 NotSupported が throw されるはず
		int[]	columnIndexes = { 1, 2, 3, 4 };
		String	query = "select id, name from t where maincategory = ? and subcategory = ?";
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.prepareStatement(query, columnIndexes);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		// SQL を指定しない場合でもサポートしていないのだから例外 NotSupported が throw されるはず
		SQLState = "";
		try {
			c.prepareStatement(null, columnIndexes);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		// SQL に空文字列を指定した場合でもサポートしていないのだから例外 NotSupported が throw されるはず
		try {
			c.prepareStatement("", columnIndexes);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// Connection.prepareStatement(String, String[]) のテスト
	public void test_prepareStatement6() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		// 自動生成キーはサポートしていないので例外 NotSupported が throw されるはず
		String[]	columnNames = { "id", "name", "maincategory", "subcategory" };
		String	query = "select id, name from t where maincategory = ? and subcategory = ?";
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.prepareStatement(query, columnNames);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		// SQL を指定しない場合でもサポートしていないのだから例外 NotSupported が throw されるはず
		SQLState = "";
		try {
			c.prepareStatement(null, columnNames);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		// SQL に空文字列を指定した場合でもサポートしていないのだから例外 NotSupported が throw されるはず
		try {
			c.prepareStatement("", columnNames);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// Connection.setAutoCommit() および Connection.getAutoCommit() のテスト
	public void test_CommitMode1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 接続直後は自動コミットモードのはず
		assertTrue(c.getAutoCommit());

		// 下準備
		createTestTable(c);

		// Statement での挿入

		Statement	s = c.createStatement();
		int	expected = 1;
		int count = s.executeUpdate("insert into t (id) values (1)");	// 1 件目
		assertEquals(count, expected);
		count = s.executeUpdate("insert into t (id) values (2)");	// 2 件目
		assertEquals(count, expected);

		s.close();

		s = c.createStatement();
		ResultSet	rs = s.executeQuery("select count(*) from t");
		// 自動コミットモードでは、暗黙のトランザクションで executeUpdate() ごとに
		// commit されているはずなので、2 件挿入されているはず
		while (rs.next()) assertEquals(2, rs.getInt(1));
		rs.close();
		s.close();

		// PreparedStatement での挿入

		// v15.0 から executeUpdate() が更新件数を返すようになった
		PreparedStatement	ps = c.prepareStatement("insert into t (id) values (?)");
		for (int id = 3; id < 6; id++) {	// 3 〜 5 件目
			ps.setInt(1, id);
			assertEquals(expected, ps.executeUpdate());
		}
		ps.close();

		s = c.createStatement();
		rs = s.executeQuery("select count(*) from t");
		// 自動コミットモードでは、暗黙のトランザクションで executeUpdate() ごとに
		// commit されているはずなので、5 件挿入されているはず
		while (rs.next()) assertEquals(5, rs.getInt(1));
		rs.close();
		s.close();

		// 手動コミットモードにしてみる
		c.setAutoCommit(false);

		// 手動コミットモードになったはず
		assertFalse(c.getAutoCommit());

		// Statement での挿入

		// commit

		s = c.createStatement();
		assertEquals(expected, s.executeUpdate("insert into t (id) values (6)"));	// 6 件目
		c.commit();
		s.close();

		s = c.createStatement();
		rs = s.executeQuery("select count(*) from t");
		// commit したら当然 6 件挿入されているはず
		while (rs.next()) assertEquals(6, rs.getInt(1));
		rs.close();
		c.commit();
		s.close();

		// rollback

		s = c.createStatement();
		assertEquals(expected, s.executeUpdate("insert into t (id) values (7)"));	// 7 件目
		assertEquals(expected, s.executeUpdate("insert into t (id) values (8)"));	// 8 件目
		c.rollback();
		s.close();

		s = c.createStatement();
		rs = s.executeQuery("select count(*) from t");
		// rollback したら当然 7 件目と 8 件目は取り消されているので 6 件挿入されているはず
		while (rs.next()) assertEquals(6, rs.getInt(1));
		rs.close();
		s.close();

		// PreparedStatement での挿入

		// commit

		ps = c.prepareStatement("insert into t (id) values (?)");
		for (int id = 7; id < 10; id++) {	// 7 〜 9 件目
			ps.setInt(1, id);
			assertEquals(expected, ps.executeUpdate());
		}
		c.commit();
		ps.close();

		s = c.createStatement();
		rs = s.executeQuery("select count(*) from t");
		// commit したら当然 9 件挿入されているはず
		while (rs.next()) assertEquals(9, rs.getInt(1));
		rs.close();
		s.close();

		// rollback

		ps = c.prepareStatement("insert into t (id) values (?)");
		for (int id = 10; id < 12; id++) {	// 10 〜 11 件目
			ps.setInt(1, id);
			assertEquals(expected, ps.executeUpdate());
		}
		c.rollback();
		ps.close();

		s = c.createStatement();
		rs = s.executeQuery("select count(*) from t");
		// rollback したら当然 10 件目と 11 件目は取り消されているので 9 件挿入されているはず
		while (rs.next()) assertEquals(9, rs.getInt(1));
		rs.close();
		s.close();

		// 後始末
		s = c.createStatement();
		expected = 9;
		assertEquals(expected, s.executeUpdate("delete from t"));
		c.commit();
		c.setAutoCommit(true);
		s.close();
		dropTestTable(c);

		c.close();

		// 閉じた後で呼び出すと例外 SessionNotAvailable が発生するはず
		String	SessionNotAvailableSQLState = (new SessionNotAvailable()).getSQLState();
		String	SQLState = "";
		try {
			c.setAutoCommit(true);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);
		SQLState = "";
		try {
			c.getAutoCommit();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);
	}
	
	// Connection.setAutoCommit() のテスト
	public void test_CommitMode2() throws Exception
	{
		// トランザクション途中での setAutoCommit() 呼び出しにより commit されることを確認する

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		// 手動コミットモードにする
		c.setAutoCommit(false);

		Statement	s = c.createStatement();
		// v15.0 から executeUpdate() が更新件数を返すようになった
		int	expected = 1;
		
		assertEquals(expected, s.executeUpdate("insert into t (id) values (1)"));	// 1 件目
		assertEquals(expected, s.executeUpdate("insert into t (id) values (2)"));	// 2 件目

		// commit せずに自動コミットモードにする
		c.setAutoCommit(true);

		s.close();

		s = c.createStatement();
		ResultSet	rs = s.executeQuery("select count(*) from t");
		// commit せずに手動コミットモードから自動コミットモードに変えたので
		// 挿入は commit されていて 2 件挿入されているはず
		while (rs.next()) assertEquals(2, rs.getInt(1));
		rs.close();
		s.close();

		// 後始末
		s = c.createStatement();
		expected = 2;
		assertEquals(expected, s.executeUpdate("delete from t"));
		s.close();
		dropTestTable(c);

		c.close();
	}

	// Connection.isClosed() のテスト
	public void test_isClosed() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 接続しているのだから、閉じていないはず
		assertFalse(c.isClosed());

		c.close();

		// 閉じたはず
		assertTrue(c.isClosed());
	}

	// Connection.nativeSQL() のテスト
	public void test_nativeSQL() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// v15.0 では、渡した SQL 文がそのまま返されるはず
		String	query = "insert into t (id) values (0)";
		assertEquals(query, c.nativeSQL(query));

		// たとえ、でたらめな SQL 文でも…
		query = "hogehoge";
		assertEquals(query, c.nativeSQL(query));

		c.close();

		// 閉じた後で呼び出すと例外 SessionNotAvailable が発生するはず
		String	SessionNotAvailableSQLState = (new SessionNotAvailable()).getSQLState();
		String	SQLState = "";
		try {
			c.nativeSQL("insert into t (id) values (0)");
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);
	}

	// Connection.commit() のテスト
	public void test_commit() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// トランザクションを開始していないのに commit すると例外が発生するはず

		String	NotBeginTransactionSQLState = (new NotBeginTransaction()).getSQLState();
		String	SQLState = "";
		try {
			c.commit();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotBeginTransactionSQLState, SQLState);

		c.setAutoCommit(false);

		c.close();

		// 閉じた後で呼び出すと例外 SessionNotAvailable が発生するはず
		String	SessionNotAvailableSQLState = (new SessionNotAvailable()).getSQLState();
		SQLState = "";
		try {
			c.commit();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);
	}

	// Connection.rollback() のテスト
	public void test_rollback() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// トランザクションを開始していないのに rollback すると例外が発生するはず

		String	NotBeginTransactionSQLState = (new NotBeginTransaction()).getSQLState();
		String	SQLState = "";
		try {
			c.rollback();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotBeginTransactionSQLState, SQLState);

		c.setAutoCommit(false);

		c.close();

		// 閉じた後で呼び出すと例外 SessionNotAvailable が発生するはず
		String	SessionNotAvailableSQLState = (new SessionNotAvailable()).getSQLState();
		SQLState = "";
		try {
			c.rollback();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);
	}

	// Connection.getMetaData() のテスト
	public void test_getMetaData() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		assertNotNull(c.getMetaData());

		c.close();

		// 閉じた後で呼び出すと例外 SessionNotAvailable が発生するはず
		String	SessionNotAvailableSQLState = (new SessionNotAvailable()).getSQLState();
		String	SQLState = "";
		try {
			c.getMetaData();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);
	}

	// Connection.setTransactionIsolation() および Connection.getTransactionIsolation() のテスト
	public void test_setAndGetTransactionIsolation() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		// 接続直後は read committed のはず
		assertEquals(Connection.TRANSACTION_READ_COMMITTED, c.getTransactionIsolation());

		// read uncommitted に変えてみる
		c.setTransactionIsolation(Connection.TRANSACTION_READ_UNCOMMITTED);

		// read uncommitted になったはず
		assertEquals(Connection.TRANSACTION_READ_UNCOMMITTED, c.getTransactionIsolation());

		// none は指定できないはず
		String	BadArgumentSQLState = (new BadArgument()).getSQLState();
		String	SQLState = "";
		try {
			c.setTransactionIsolation(Connection.TRANSACTION_NONE);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(BadArgumentSQLState, SQLState);

		// トランザクション中に指定しようとすると例外 AlreadyBeginTransaction が発生するはず

		SQLState = "";

		c.setAutoCommit(false);

		Statement	s = c.createStatement();
		int expected = 1;
		assertEquals(expected, s.executeUpdate("insert into t (id) values (1)"));

		String	AlreadyBeginTransactionSQLState = (new AlreadyBeginTransaction()).getSQLState();
		try {
			c.setTransactionIsolation(Connection.TRANSACTION_READ_COMMITTED);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(AlreadyBeginTransactionSQLState, SQLState);

		c.commit();

		s.close();

		// 後始末
		c.setAutoCommit(true);
		s = c.createStatement();
		s.close();
		dropTestTable(c);

		c.close();

		// 閉じた後で呼び出すと例外 SessionNotAvailable が発生するはず
		String	SessionNotAvailableSQLState = (new SessionNotAvailable()).getSQLState();
		SQLState = "";
		try {
			c.setTransactionIsolation(Connection.TRANSACTION_READ_COMMITTED);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);
		SQLState = "";
		try {
			c.getTransactionIsolation();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);
	}	
	
	// Connection.setReadOnly() と Connection.isReadOnly() のテスト
	public void test_setReadOnly() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		//
		// まずは setReadOnly() できちんと設定されることを isReadOnly() により確認
		//

		// 初期状態では書き込み可となっているはず
		assertFalse(c.isReadOnly());

		// 書き込み不可を設定
		c.setReadOnly(true);
		assertTrue(c.isReadOnly());

		// 書き込み可を設定
		c.setReadOnly(false);
		assertFalse(c.isReadOnly());

		//
		// 書き込み不可に設定した後に更新系の処理ができないことを確認
		//

		// 書き込み不可を設定
		c.setReadOnly(true);

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		// 更新系の処理実行で例外 ReadOnlyTransaction が発生するはず
		String	query = "insert into t (id, name, maincategory, subcategory) values (1, 'foo', 1001, 3)";
		String	ReadOnlyTransactionSQLState = (new ReadOnlyTransaction()).getSQLState();
		String	SQLState = "";
		try {
			s.executeUpdate(query);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(ReadOnlyTransactionSQLState, SQLState);

		// 参照系の処理はできるはず
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select count(*) from t"));
		assertTrue(rs.next());
		assertEquals(0, rs.getInt(1)); // count(*) のチェック（何にも入っていないはず）
		assertFalse(rs.next());

		//
		// 今度は書き込み可に設定し更新系の処理ができるようになったことを確認
		//

		// 書き込み可を設定
		c.setReadOnly(false);

		// v15.0 から executeUpdate() が更新件数を返すようになった
		int	expected = 1;
		assertEquals(expected, s.executeUpdate(query));
		assertNotNull(rs = s.executeQuery("select * from t"));
		assertTrue(rs.next());

		// 列名での getter メソッドも v15.0 からサポート
		assertEquals(1, rs.getInt("id"));			// 以下、挿入した中身のチェック
		assertEquals("foo", rs.getString("name"));
		assertEquals(1001, rs.getInt("maincategory"));
		assertEquals(3, rs.getInt("subcategory"));

		assertFalse(rs.next());

		//
		// トランザクション中に設定しようとすると例外 AlreadyBeginTransaction が発生するはず
		//

		// 手動コミットモードを設定し更新系の処理を実行
		// （これでトランザクションが開始される）
		c.setAutoCommit(false);
		assertEquals(expected, s.executeUpdate(query));

		String	AlreadyBeginTransactionSQLState = (new AlreadyBeginTransaction()).getSQLState();
		SQLState = "";
		try {
			c.setReadOnly(true);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(AlreadyBeginTransactionSQLState, SQLState);

		c.commit();
		s.close();

		//
		// トランザクション遮断レベルに TRANSACTION_USING_SNAPSHOT を設定後
		// 書き込み可を設定しようとすると例外 BadTransaction が発生するはず
		//

		c.setTransactionIsolation(jp.co.ricoh.doquedb.jdbc.Connection.TRANSACTION_USING_SNAPSHOT);
		String	badTransactionSQLState = (new BadTransaction()).getSQLState();
		SQLState = "";
		try {
			c.setReadOnly(false);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(badTransactionSQLState, SQLState);

		// 後始末
		c.setTransactionIsolation(java.sql.Connection.TRANSACTION_READ_COMMITTED);
		c.setAutoCommit(true);
		c.setReadOnly(false);
		s = c.createStatement();
		expected = 2;
		assertEquals(expected, s.executeUpdate("delete from t"));
		s.close();
		dropTestTable(c);

		c.close();

		// 閉じた後で呼び出すと例外 SessionNotAvailable が発生するはず
		String	SessionNotAvailableSQLState = (new SessionNotAvailable()).getSQLState();
		SQLState = "";
		try {
			c.setReadOnly(false);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);
		SQLState = "";
		try {
			c.isReadOnly();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);
	}

	// Connection.setHoldability() と Connection.getHoldability() のテスト
	public void test_setAndGetHoldability() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 
		// 現状では ResultSet.CLOSE_CURSORS_AT_COMMIT のみをサポートしているので
		// それ以外では例外 NotSupported が発生するはず
		//

		// CLOSE_CURSORS_AT_COMMIT
		c.setHoldability(ResultSet.CLOSE_CURSORS_AT_COMMIT);

		// 常に CLOSE_CURSORS_AT_COMMIT が得られるはず
		assertEquals(ResultSet.CLOSE_CURSORS_AT_COMMIT, c.getHoldability());

		// HOLD_CURSORS_OVER_COMMIT
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.setHoldability(ResultSet.HOLD_CURSORS_OVER_COMMIT);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		// 常に CLOSE_CURSORS_AT_COMMIT が得られるはず
		assertEquals(ResultSet.CLOSE_CURSORS_AT_COMMIT, c.getHoldability());

		// -1
		SQLState = "";
		try {
			c.setHoldability(-1);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		// 常に CLOSE_CURSORS_AT_COMMIT が得られるはず
		assertEquals(ResultSet.CLOSE_CURSORS_AT_COMMIT, c.getHoldability());

		c.close();

		// 閉じた後で呼び出すと例外 SessionNotAvailable が発生するはず
		String	SessionNotAvailableSQLState = (new SessionNotAvailable()).getSQLState();
		SQLState = "";
		try {
			c.setHoldability(ResultSet.CLOSE_CURSORS_AT_COMMIT);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);
		SQLState = "";
		try {
			c.getHoldability();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);
	}

	// Connection.getWarnings() と Connection.clearWarnings() のテスト
	public void test_getAndClearWarnings() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// getWarnings() と clearWarnings() は、
		// createStatement() と prepareStatement() のテストで
		// 散々やっているので、ここでは省く。

		c.close();

		//
		// 閉じた後で呼び出すと例外 SessionNotAvailable が発生するはず
		//

		// Connection.getWarnings();
		String	SessionNotAvailableSQLState = (new SessionNotAvailable()).getSQLState();
		String	SQLState = "";
		try {
			c.getWarnings();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);

		// Connection.clearWarnings();
		SQLState = "";
		try {
			c.clearWarnings();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);
	}

	// Connection.setCatalog() と Connection.getCatalog() のテスト
	public void test_setAndGetCatalog() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 
		// カタログをサポートしていないので setCatalog() で何を指定しても未処理のはず
		// また getCatalog() は常に null を返すはず
		//

		// null
		c.setCatalog(null);
		assertNull(c.getCatalog());

		// 空文字列
		c.setCatalog("");
		assertNull(c.getCatalog());

		// 適当なカタログ名
		c.setCatalog("foo");
		assertNull(c.getCatalog());

		c.close();

		//
		// 閉じた後で呼び出すと例外 SessionNotAvailable が発生するはず
		//

		// Connection.setCatalog()
		String	SessionNotAvailableSQLState = (new SessionNotAvailable()).getSQLState();
		String	SQLState = "";
		try {
			c.setCatalog("workingCatalog");
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);

		// Connection.getCatalog()
		SQLState = "";
		try {
			c.getCatalog();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);
	}

	// Connection.setSavepoint() と Connection.releaseSavepoint() のテスト
	public void test_setAndReleaseSavepoint() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		//
		// セーブポイントをサポートしていないので例外 NotSupported が発生するはず
		//

		// Connection.setSavepoint()
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.setSavepoint();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		// Connection.setSavepoint(String)
		SQLState = "";
		try {
			c.setSavepoint("hogehoge");
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		// Connection.releaseSavepoint(java.sql.Savepoint)
		SQLState = "";
		try {
			c.releaseSavepoint(null);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// Connection.setTypeMap() と Connection.getTypeMap() のテスト
	public void test_setAndGetTypeMap() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		java.util.HashMap	hashMap = new java.util.HashMap();

		//
		// カスタムマッピングをサポートしていないので例外 NotSupported が発生するはず
		//

		// Connection.setTypeMap(java.util.Map)
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.setTypeMap(hashMap);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		// Connection.getTypeMap()
		SQLState = "";
		try {
			c.getTypeMap();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// Connection.prepareCall() のテスト
	public void test_prepareCall() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		String	query = "insert into t (id, name, maincategory, subcategory) values (30, 'foo', 1004, 2)";

		//
		// ストアドプロシージャをサポートしていないので例外 NotSupported が発生するはず
		//

		// Connection.prepareCall(String)
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			c.prepareCall(query);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		// Connection.prepareCall(String, int, int)
		SQLState = "";
		try {
			c.prepareCall(	query,
							ResultSet.TYPE_FORWARD_ONLY,
							ResultSet.CONCUR_READ_ONLY);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		// Connection.prepareCall(String, int, int, int)
		SQLState = "";
		try {
			c.prepareCall(	query,
							ResultSet.TYPE_FORWARD_ONLY,
							ResultSet.CONCUR_READ_ONLY,
							ResultSet.CLOSE_CURSORS_AT_COMMIT);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		c.close();
	}

	// transaction read only, using snapshot のテスト（Connection.setTransactionIsolation()の実行時間の計測）
	public void test_measureSetTransactionIsolation() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		long	atStart = (new java.util.GregorianCalendar()).getTimeInMillis();
		c.setTransactionIsolation(java.sql.Connection.TRANSACTION_READ_UNCOMMITTED);
		long	atEnd = (new java.util.GregorianCalendar()).getTimeInMillis();
		System.out.println("TRANSACTION_READ_UNCOMMITTED = " + (atEnd - atStart) + "[msec]");

		atStart = (new java.util.GregorianCalendar()).getTimeInMillis();
		c.setTransactionIsolation(java.sql.Connection.TRANSACTION_READ_COMMITTED);
		atEnd = (new java.util.GregorianCalendar()).getTimeInMillis();
		System.out.println("TRANSACTION_READ_COMMITTED = " + (atEnd - atStart) + "[msec]");

		atStart = (new java.util.GregorianCalendar()).getTimeInMillis();
		c.setTransactionIsolation(java.sql.Connection.TRANSACTION_REPEATABLE_READ);
		atEnd = (new java.util.GregorianCalendar()).getTimeInMillis();
		System.out.println("TRANSACTION_REPEATABLE_READ = " + (atEnd - atStart) + "[msec]");

		atStart = (new java.util.GregorianCalendar()).getTimeInMillis();
		c.setTransactionIsolation(java.sql.Connection.TRANSACTION_SERIALIZABLE);
		atEnd = (new java.util.GregorianCalendar()).getTimeInMillis();
		System.out.println("TRANSACTION_SERIALIZABLE = " + (atEnd - atStart) + "[msec]");

		atStart = (new java.util.GregorianCalendar()).getTimeInMillis();
		c.setTransactionIsolation(jp.co.ricoh.doquedb.jdbc.Connection.TRANSACTION_USING_SNAPSHOT);
		atEnd = (new java.util.GregorianCalendar()).getTimeInMillis();
		System.out.println("TRANSACTION_USING_SNAPSHOT = " + (atEnd - atStart) + "[msec]");
	}

	// transaction read only, using snapshot のテスト
	// 『USING SNAPSHOTで別スレッドのトランザクション中の更新結果が見えてしまう』の再現
	public void test_usingSnapshot() throws Exception
	{
		// 下準備
		Connection	c = null;
		assertNotNull(c = getConnection());
		createTestTable(c);
		c.close();

		Runnable	insertRun = new InsertRun(super.URL);
		Thread		insertThread = new Thread(insertRun);
		Runnable	selectRun = new SelectRun(super.URL);
		Thread		selectThread = new Thread(selectRun);

		insertThread.start();
		Thread.sleep(100);
		selectThread.start();

		insertThread.join();
		selectThread.join();

		// 後始末
		assertNotNull(c = getConnection());
		dropTestTable(c);
		c.close();
	}

	// 『Connectionのnew、open、closeを繰り返すとConnection ran outになることがある』の再現
	public void test_0253() throws Exception
	{
		int	i = 0;
		try {
			for (i = 0; i < 10000; i++) {
				Connection	c = getConnection();
				c.close();
			}
		} catch (SQLException	sqle) {
			System.err.println("counter = " + i);
			sqle.printStackTrace();
			throw sqle;
		}
	}

	public void test_createUser() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		try {
			((jp.co.ricoh.doquedb.jdbc.Connection)c).createUser("aaa", "");
			((jp.co.ricoh.doquedb.jdbc.Connection)c).createUser("bbb", "bbb");

			Statement s = c.createStatement();
			s.executeUpdate("grant reference_operations to aaa");
			s.close();

			Connection	c0 = DriverManager.getConnection(URL, "aaa", "");

			Statement s0 = c0.createStatement();
			ResultSet rs = s0.executeQuery("select name, type from system_user");
			assertTrue(rs.next());
			assertEquals(rs.getString(1), _user);
			assertEquals(rs.getInt(2), 0);

			assertTrue(rs.next());
			assertEquals(rs.getString(1), "aaa");
			assertEquals(rs.getInt(2), 1);

			assertTrue(rs.next());
			assertEquals(rs.getString(1), "bbb");
			assertEquals(rs.getInt(2), 1);

			assertFalse(rs.next());

			rs.close();
			s.close();
			c0.close();

		} catch (SQLException	sqle) {
			System.err.println("can't create user");
			sqle.printStackTrace();
			throw sqle;
		} finally {
			c.close();
		}
	}

	public void test_dropUser() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		try {
			((jp.co.ricoh.doquedb.jdbc.Connection)c).dropUser("aaa");
			((jp.co.ricoh.doquedb.jdbc.Connection)c).dropUser("bbb");

			Statement s = c.createStatement();
			ResultSet rs = s.executeQuery("select name, type from system_user");
			assertTrue(rs.next());
			assertEquals(rs.getString(1), _user);
			assertEquals(rs.getInt(2), 0);

			assertFalse(rs.next());

			rs.close();
			s.close();

		} catch (SQLException	sqle) {
			System.err.println("can't drop user");
			sqle.printStackTrace();
			throw sqle;
		} finally {
			c.close();
		}
	}

	// データベースを準備する
	public void setUp()	throws Exception
	{
		super.setUp();
	}

	// データベースを削除する
	public void tearDown() throws Exception
	{
		super.tearDown();
	}

	private void createTestTable(Connection	c) throws Exception
	{
		Statement	s = null;
		assertNotNull(s = c.createStatement());
		assertEquals(0, s.executeUpdate("create table t (id int, name nvarchar(100), maincategory int, subcategory int)"));
		s.close();
	}

	private void dropTestTable(Connection	c) throws Exception
	{
		Statement	s = c.createStatement();
		assertEquals(0, s.executeUpdate("drop table t"));
		s.close();
	}

	private void assertCreateStatementBadArgument(	Connection	c,
													int			resultSetType,
													int			resultSetConcurrency)
	{
		String	BadArgumentSQLState = (new BadArgument()).getSQLState();
		String	SQLState = "";
		try {
			c.createStatement(resultSetType, resultSetConcurrency);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(BadArgumentSQLState, SQLState);
	}

	private void assertCreateStatementBadArgument(	Connection	c,
													int			resultSetType,
													int			resultSetConcurrency,
													int			resultSetHoldability)
	{
		String	BadArgumentSQLState = (new BadArgument()).getSQLState();
		String	SQLState = "";
		try {
			c.createStatement(resultSetType, resultSetConcurrency, resultSetHoldability);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(BadArgumentSQLState, SQLState);
	}

	private void assertPrepareStatementSQLSyntaxError(	Connection	c,
														String		query)
	{
		String	SQLSyntaxErrorSQLState = (new SQLSyntaxError("")).getSQLState();
		String	SQLState = "";
		try {
			c.prepareStatement(query);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SQLSyntaxErrorSQLState, SQLState);
	}

	private void assertPrepareStatementTableNotFound(	Connection	c,
														String		query)
	{
		String	tableNotFoundSQLState = (new TableNotFound("foo", "foo")).getSQLState();
		String	SQLState = "";
		try {
			c.prepareStatement(query);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(tableNotFoundSQLState, SQLState);
	}

	private void assertPrepareStatementBadArgument(	Connection	c,
													String		query)
	{
		String	BadArgumentSQLState = (new BadArgument()).getSQLState();
		String	SQLState = "";
		try {
			c.prepareStatement(query);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(BadArgumentSQLState, SQLState);
	}

	private void assertPrepareStatementBadArgument(	Connection	c,
													String		query,
													int			resultSetType,
													int			resultSetConcurrency)
	{
		String	BadArgumentSQLState = (new BadArgument()).getSQLState();
		String	SQLState = "";
		try {
			c.prepareStatement(query, resultSetType, resultSetConcurrency);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(BadArgumentSQLState, SQLState);
	}

	private void assertPrepareStatementBadArgument(	Connection	c,
													String		query,
													int			resultSetType,
													int			resultSetConcurrency,
													int			resultSetHoldability)
	{
		String	BadArgumentSQLState = (new BadArgument()).getSQLState();
		String	SQLState = "";
		try {
			c.prepareStatement(query, resultSetType, resultSetConcurrency, resultSetHoldability);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(BadArgumentSQLState, SQLState);
	}
}

//
// Copyright (c) 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
