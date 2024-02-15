// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// StatementTest.java -- jp.co.ricoh.doquedb.jdbc.Statement クラスのテスト
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

public class StatementTest extends TestBase
{
	// 警告メッセージ WM_*****

	private final static String	WM_DIRECTION =
		"direction of ResultSet currently supported is only java.sql.ResultSet.FETCH_FORWARD.";

	private final static String	WM_FETCH_SIZE =
		"hint about the number of rows which needs to be taken out from a database is not supported other than zero.";

	private final static String	WM_CURSOR_NOT_SUPPORT =
		"cursor is not supporting.";

	public StatementTest(String	nickname)
	{
		super(nickname);
	}

	// Statement.executeQuery() のテスト
	public void test_executeQuery() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);
		Statement	s = null;
		assertNotNull(s = c.createStatement());
		s.executeUpdate("insert into t (id, name, maincategory, subcategory) values (1, 'foo',  3, 101)");
		s.executeUpdate("insert into t (id, name, maincategory, subcategory) values (2, 'hoge', 3, 102)");
		s.executeUpdate("insert into t (id, name, maincategory) values (3, 'abc', 2)");
		s.close();

		assertNotNull(s = c.createStatement());

		int	numberOfTuples = 3;
		int[]		ids				= { 1,		2,		3		};
		String[]	names			= { "foo",	"hoge",	"abc"	};
		int[]		maincategories	= { 3,		3,		2		};
		int[]		subcategories	= { 101,	102,	-1		};

		//
		// 正常系
		//

		String	query = null;
		ResultSet	rs = null;

		boolean	autoCommit = true;

		// 自動コミットモードと手動コミットモード
		for (int i = 0; i < 2; i++, autoCommit = false) {

			c.setAutoCommit(autoCommit);

			assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);

			if (autoCommit == false) c.commit();
		}

		// コミットモードの後始末
		c.setAutoCommit(true);

		//
		// 異常系
		//

		// 引数に null を指定すると例外 BadArgument が throw されるはず
		query = null;
		String	BadArgumentSQLState = (new BadArgument()).getSQLState();
		String	SQLState = "";
		try {
			s.executeQuery(query);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(BadArgumentSQLState, SQLState);

		// 引数に空文字列を指定しても例外 BadArgument が throw されるはず
		query = "";
		SQLState = "";
		try {
			s.executeQuery(query);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(BadArgumentSQLState, SQLState);

		// [仕様変更]
		// 以前は存在しない表を指定すると例外 SQLSyntaxError が
		// 発生していたが、現在は例外 TableNotFound となっている。
		// （ v14.0 に合わせるための仕様変更。）
		// 2005/04/21

		// 存在しない表を指定すると例外 TableNotFound が throw されるはず
		query = "select * from x";
		// Statement.executeQuery() を実行しただけではなんの例外も発生しないはず
		assertNotNull(rs = s.executeQuery(query));
		String	TableNotFoundSQLState = (new TableNotFound("foo", "foo")).getSQLState();
		SQLState = "";
		try {
			// ResultSet.next() を実行すると例外 TableNotFound が throw されるはず
			rs.next();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		rs.close();
		assertEquals(TableNotFoundSQLState, SQLState);

		// 以下、syntax error

		// [仕様変更]
		// 存在しない列の指定は insert, update 文では ColumnNotFound 、
		// select 文では SQLSyntaxError 。
		// 2005/04/21

		String[]	queries = {
			"select foo from t",	// 存在しない列からの取得
			"sel * by t"			// めちゃめちゃな SQL 文
		};

		// [仕様変更]
		// SQLSyntaxError から ColumnNotFound に仕様変更。
		// 2005/05/17 

		String	SQLSyntaxErrorSQLState = (new SQLSyntaxError("")).getSQLState();
		String	columnNotFoundSQLState = (new ColumnNotFound("foo")).getSQLState();
		for (int i = 0; i < queries.length; i++) {

			rs = null;
			// Statement.executeQuery() を実行しただけではなんの例外も発生しないはず
			assertNotNull(rs = s.executeQuery(queries[i]));
			// ResultSet.next() を実行すると例外 SQLSyntaxError が throw されるはず
			SQLState = "";
			try {
				rs.next();
			} catch (SQLException	sqle) {
				SQLState = sqle.getSQLState();
			}
			if (i == 0)	assertEquals(columnNotFoundSQLState, SQLState);
			else		assertEquals(SQLSyntaxErrorSQLState, SQLState);
			rs.close();
		}

		s.close();

		//
		// ResultSet を閉じていなくても Statement を閉じると
		// 内部で ResultSet も閉じられる。
		// 閉じた ResultSet の next() は常に false を返すはず。
		//

		assertNotNull(s = c.createStatement());
		query = "select * from t";
		assertNotNull(rs = s.executeQuery(query));
		assertTrue(rs.next());
		s.close();				// ResultSet を閉じずに Statement を閉じちゃう
		assertFalse(rs.next());	// そうすると、ResultSet.next() は false を返すはず

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// Statement.executeUpdate() のテスト
	public void test_executeUpdate1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		Statement	s = null;
		int	numberOfTuples = 0;

		//
		// 自動コミットモード
		//

		{
			assertNotNull(s = c.createStatement());

			int	expected = 1;
			assertEquals(expected, s.executeUpdate("insert into t (id, name, maincategory, subcategory) values (1, 'userA', 4, 52)"));	numberOfTuples++;
			assertEquals(expected, s.executeUpdate("insert into t (id, name, maincategory, subcategory) values (2, 'userB', 2, 19)"));	numberOfTuples++;
			assertEquals(expected, s.executeUpdate("insert into t (id, name, maincategory) values (3, 'userC', 3)"));					numberOfTuples++;

			int[]		ids				= { 1,			2,			3		};
			String[]	names			= { "userA",	"userB",	"userC"	};
			int[]		maincategories	= { 4,			2,			3		};
			int[]		subcategories	= { 52,			19,			-1		};

			s.close();

			assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);
		}

		//
		// 手動コミットモード
		//
		c.setAutoCommit(false);

		//
		// commit
		//

		assertNotNull(s = c.createStatement());
		assertEquals(1, s.executeUpdate("insert into t (id, name, maincategory, subcategory) values (4, 'userD', 1, 102)"));	numberOfTuples++;
		assertEquals(1, s.executeUpdate("insert into t (id, name, maincategory) values (5, 'userE', 3)"));						numberOfTuples++;
		assertEquals(2, s.executeUpdate("update t set name = 'userX' where maincategory = 4 or maincategory = 2"));

		int[]		ids				= { 1,			2,			3,			4,			5		};
		String[]	names			= { "userX",	"userX",	"userC",	"userD",	"userE"	};
		int[]		maincategories	= { 4,			2,			3,			1,			3		};
		int[]		subcategories	= { 52,			19,			-1,			102,		-1		};

		c.commit();
		s.close();
		c.setAutoCommit(true);

		assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);

		c.setAutoCommit(false);

		//
		// rollback
		//

		assertNotNull(s = c.createStatement());
		assertEquals(1, s.executeUpdate("insert into t (id, name, maincategory, subcategory) values (6, 'userF', 3, 101)"));
		assertEquals(1, s.executeUpdate("delete from t where name = 'userD'"));

		c.rollback();
		s.close();
		c.setAutoCommit(true);

		assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);

		//
		// 異常系
		//

		// 引数に null を指定すると例外 BadArgument が throw されるはず
		String	query = null;
		String	BadArgumentSQLState = (new BadArgument()).getSQLState();
		String	SQLState = "";
		try {
			s.executeUpdate(query);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(BadArgumentSQLState, SQLState);

		// 引数に空文字列を指定しても例外 BadArgument が throw されるはず
		query = "";
		SQLState = "";
		try {
			s.executeUpdate(query);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(BadArgumentSQLState, SQLState);

		// [仕様変更]
		// 以前は存在しない表を指定すると例外 SQLSyntaxError が
		// 発生していたが、現在は例外 TableNotFound となっている。
		// （ v14.0 に合わせるための仕様変更。）
		// 2005/04/21

		//
		// 以下、TableNotFound
		//

		String[]	queries1 = {
			"insert into x (id) values (0)",							// 存在しない表への挿入
			"update x set name = 'foo' where id = 3",					// 存在しない表の更新
			"delete from x where id = 1"								// 存在しない表からの削除
		};
		String	TableNotFoundSQLState = (new TableNotFound("foo", "foo")).getSQLState();
		for (int i = 0; i < queries1.length; i++) {

			try {
				s.executeUpdate(queries1[i]);
			} catch (SQLException	sqle) {
				SQLState = sqle.getSQLState();
			}
			assertEquals(TableNotFoundSQLState, SQLState);
		}

		//
		// 以下、ColumnNotFound
		//

		// [仕様変更]
		// 存在しない列の指定は insert, update 文では ColumnNotFound 、
		// その他の文では SQLSyntaxError 。
		// 2005/04/21

		String[]	queries2 = {
			"insert into t (id, name, subid) values (1, 'foo', 30)",	// 存在しない列を含む挿入
			"update t set memo = 'hogehoge' where name = 'foo'"			// 存在しない列の更新
		};
		String	columnNotFoundSQLState = (new ColumnNotFound("foo")).getSQLState();
		for (int i = 0; i < queries2.length; i++) {

			try {
				s.executeUpdate(queries2[i]);
			} catch (SQLException	sqle) {
				SQLState = sqle.getSQLState();
			}
			assertEquals(columnNotFoundSQLState, SQLState);
		}

		//
		// 以下、syntax error
		//

		// [仕様変更]
		// SQLSyntaxError から ColumnNotFound に仕様変更。
		// 2005/05/17 

		String[]	queries3 = {
//			"insert into x (id) values (0)",							// 存在しない表への挿入
//			"update x set name = 'foo' where id = 3",					// 存在しない表の更新
//			"delete from x where id = 1",								// 存在しない表からの削除
//			"insert into t (id, name, subid) values (1, 'foo', 30)",	// 存在しない列を含む挿入
//			"update t set memo = 'hogehoge' where name = 'foo'",		// 存在しない列の更新
			"delete from t where title = 'hoge'",						// 存在しない列の値を指定しての削除
			"updat t st id = 1"											// めちゃめちゃな SQL 文
		};
		String	SQLSyntaxErrorSQLState = (new SQLSyntaxError("")).getSQLState();
		for (int i = 0; i < queries3.length; i++) {

			try {
				s.executeUpdate(queries3[i]);
			} catch (SQLException	sqle) {
				SQLState = sqle.getSQLState();
			}
			if (i == 0) assertEquals(columnNotFoundSQLState, SQLState);
			else		assertEquals(SQLSyntaxErrorSQLState, SQLState);
		}

		s.close();

		//
		// 閉じた Statement に対して executeUpdate() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		query = "insert into t (id) values (1001)";
		s.executeUpdate(query);
		numberOfTuples++;
		assertNotNull(s = c.createStatement());
		query = "select * from t where id = 1001";
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery(query));
		assertTrue(rs.next());
		assertEquals(1001, rs.getInt("id"));
		assertFalse(rs.wasNull());
		assertFalse(rs.next());
		rs.close();
		s.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// Statement.close() のテスト
	public void test_close() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);
		Statement	s = null;
		assertNotNull(s = c.createStatement());
		int	numberOfTuples = 3;
		s.executeUpdate("insert into t (id, name, maincategory, subcategory) values (1, 'foo',  3, 101)");
		s.executeUpdate("insert into t (id, name, maincategory, subcategory) values (2, 'hoge', 3, 102)");
		s.executeUpdate("insert into t (id, name, maincategory) values (3, 'abc', 2)");

		//
		// ResultSet のない状態で Statement を close
		//

		s.close();

		//
		// ResultSet のある状態で Statement を close
		//

		assertNotNull(s = c.createStatement());
		String	query = "select * from t";
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery(query));
		assertTrue(rs.next());
		s.close();

		assertFalse(rs.next());

		//
		// close した ResultSet のある状態で Statement を close
		//

		assertNotNull(s = c.createStatement());
		assertNotNull(rs = s.executeQuery(query));
		for (int i = 0; i < numberOfTuples; i++) assertTrue(rs.next());
		assertFalse(rs.next());
		rs.close();
		s.close();

		// 既に閉じた Statement に対して再度 close() を呼び出しても問題ないはず
		s.close();

		// 何回呼び出したって平気
		for (int i = 0; i < 10; i++) s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// Statement.setMaxFieldSize() と Statement.getMaxFieldSize() のテスト
	public void test_setAndGetMaxFieldSize() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		// デフォルト状態でも“無制限”を示す 0 が得られるはず
		assertZero(s.getMaxFieldSize());

		// 現状では setMaxFieldSize() をサポートしておらず何もしないはず
		s.setMaxFieldSize(0);

		// 現状では getMaxFieldSize() をサポートしていないので、
		// 常に“無制限”を示す 0 が得られるはず
		assertZero(s.getMaxFieldSize());

		// 同上

		s.setMaxFieldSize(256);
		assertZero(s.getMaxFieldSize());

		s.setMaxFieldSize(100000);
		assertZero(s.getMaxFieldSize());

		// 負数では例外 BadArgument が throw されるはず
		String	BadArgumentSQLState = (new BadArgument()).getSQLState();
		String	SQLState = "";
		try {
			s.setMaxFieldSize(-1);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(BadArgumentSQLState, SQLState);

		s.close();

		//
		// 閉じた Statement に対して setMaxFieldSize() と getMaxFieldSize() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		s.setMaxFieldSize(100);
		assertZero(s.getMaxFieldSize());

		c.close();
	}

	// Statement.setMaxRows() と Statement.getMaxRows() のテスト
	public void test_setAndGetMaxRows() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		// デフォルト状態でも“無制限”を示す 0 が得られるはず
		assertZero(s.getMaxRows());

		// 現状では setMaxRows() をサポートしておらず何もしないはず
		s.setMaxFieldSize(0);

		// 現状では getMaxRows() をサポートしていないので、
		// 常に“無制限”を示す 0 が得られるはず
		assertZero(s.getMaxRows());

		// 同上

		s.setMaxRows(256);
		assertZero(s.getMaxRows());

		s.setMaxRows(100000);
		assertZero(s.getMaxRows());

		// 負数では例外 BadArgument が throw されるはず
		String	BadArgumentSQLState = (new BadArgument()).getSQLState();
		String	SQLState = "";
		try {
			s.setMaxRows(-1);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(BadArgumentSQLState, SQLState);

		s.close();

		//
		// 閉じた Statement に対して setMaxRows() と getMaxRows() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		s.setMaxRows(100);
		assertZero(s.getMaxRows());

		c.close();
	}

	// Statement.setFetchDirection() と Statement.getFetchDirection() のテスト
	public void test_setAndGetFetchDirection() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		// デフォルト状態でも FETCH_FORWARD が得られるはず
		assertEquals(ResultSet.FETCH_FORWARD, s.getFetchDirection());

		// FETCH_FORWARD では警告は何もないはず
		s.setFetchDirection(ResultSet.FETCH_FORWARD);
		assertNull(s.getWarnings());

		// 常に FETCH_FORWARD が得られるはず
		assertEquals(ResultSet.FETCH_FORWARD, s.getFetchDirection());

		java.util.Vector	wEC = new java.util.Vector();	// error code
		java.util.Vector	wSS = new java.util.Vector();	// SQLState
		java.util.Vector	wMS = new java.util.Vector();	// message

		// FETCH_REVERSE はサポートしていないので警告が出るはず
		// （警告その１）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_DIRECTION);
		s.setFetchDirection(ResultSet.FETCH_REVERSE);
		// 警告のチェック
		assertSQLWarning(s.getWarnings(), wEC, wSS, wMS);

		// 常に FETCH_FORWARD が得られるはず
		assertEquals(ResultSet.FETCH_FORWARD, s.getFetchDirection());

		// FETCH_UNKNOWN はサポートしていないので警告が出るはず
		// （警告その２）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_DIRECTION);
		s.setFetchDirection(ResultSet.FETCH_UNKNOWN);
		// 警告のチェック
		assertSQLWarning(s.getWarnings(), wEC, wSS, wMS);

		// 常に FETCH_FORWARD が得られるはず
		assertEquals(ResultSet.FETCH_FORWARD, s.getFetchDirection());

		// 警告をクリア
		s.clearWarnings();
		assertNull(s.getWarnings());

		// -1 では例外 BadArgument が throw されるはず
		String	BadArgumentSQLState = (new BadArgument()).getSQLState();
		String	SQLState = "";
		try {
			s.setFetchDirection(-1);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(BadArgumentSQLState, SQLState);

		// 常に FETCH_FORWARD が得られるはず
		assertEquals(ResultSet.FETCH_FORWARD, s.getFetchDirection());

		s.close();

		//
		// 閉じた Statement に対して setFetchDirection() と getFetchDirection() を
		// 呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		s.setFetchDirection(ResultSet.FETCH_FORWARD);
		assertEquals(ResultSet.FETCH_FORWARD, s.getFetchDirection());

		c.close();
	}

	// Statement.setFetchSize() と Statement.getFetchSize() のテスト
	public void test_setAndGetFetchSize() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		// デフォルト状態でも 0 が得られるはず
		assertZero(s.getFetchSize());

		// 0 では警告は何もないはず
		s.setFetchSize(0);
		assertNull(s.getWarnings());

		// 常に 0 が得られるはず
		assertZero(s.getFetchSize());

		java.util.Vector	wEC = new java.util.Vector();	// error code
		java.util.Vector	wSS = new java.util.Vector();	// SQLState
		java.util.Vector	wMS = new java.util.Vector();	// message

		// 0 以外はサポートしていないので警告が出るはず − その１
		// （警告その１）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_FETCH_SIZE);
		s.setFetchSize(-1);
		// 警告のチェック
		assertSQLWarning(s.getWarnings(), wEC, wSS, wMS);

		// 常に 0 が得られるはず
		assertZero(s.getFetchSize());

		// 0 以外はサポートしていないので警告が出るはず − その２
		// （警告その２）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_FETCH_SIZE);
		s.setFetchSize(1);
		// 警告のチェック
		assertSQLWarning(s.getWarnings(), wEC, wSS, wMS);

		// 常に 0 が得られるはず
		assertZero(s.getFetchSize());

		// 警告をクリア
		s.clearWarnings();
		assertNull(s.getWarnings());

		// 常に 0 が得られるはず
		assertZero(s.getFetchSize());

		s.close();

		//
		// 閉じた Statement に対して setFetchSize() と getFetchSize() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		s.setFetchSize(0);
		assertZero(s.getFetchSize());

		c.close();
	}

	// Statement.getResultSetConcurrency() のテスト
	public void test_getResultSetConcurrency() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		// 常に ResultSet.CONCUR_READ_ONLY が得られるはず
		assertEquals(ResultSet.CONCUR_READ_ONLY, s.getResultSetConcurrency());

		s.close();

		//
		// 閉じた Statement に対して getResultSetConcurrency() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		assertEquals(ResultSet.CONCUR_READ_ONLY, s.getResultSetConcurrency());

		c.close();
	}

	// Statement.getResultSetType() のテスト
	public void test_getResultSetType() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		// 常に ResultSet.TYPE_FORWARD_ONLY が得られるはず
		assertEquals(ResultSet.TYPE_FORWARD_ONLY, s.getResultSetType());

		s.close();

		//
		// 閉じた Statement に対して getResultSetType() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		assertEquals(ResultSet.TYPE_FORWARD_ONLY, s.getResultSetType());

		c.close();
	}

	// Statement.addBatch() と Statement.clearBatch() と Statement.executeBatch() のテスト
	public void test_batch() throws Exception
	{

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);


		//
		// SQL 文を指定しなければ（空文字列も）、何もしないはず
		//

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		s.addBatch(null);
		s.addBatch("");
		assertZero(s.executeBatch().length);
		s.close();

		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select count(*) from t"));
		assertTrue(rs.next());
		assertZero(rs.getInt(1));	assertFalse(rs.wasNull());
		assertFalse(rs.next());
		rs.close();
		s.close();

		//
		// 正常系−その１（ insert での確認 − 自動コミットモード）
		//

		{
			assertNotNull(s = c.createStatement());

			s.addBatch("insert into t (id, name, maincategory, subcategory) values (1, 'title01', 2, 1024)");
			s.addBatch("insert into t (id, name, maincategory, subcategory) values (2, 'title02', 1, 1001)");
			s.addBatch("insert into t (id, name, maincategory) values (3, 'title03', 4)");
			s.addBatch("insert into t (id, name, maincategory, subcategory) values (4, 'title04', 2, 1020)");
			s.addBatch("insert into t (id, name, maincategory, subcategory) values (5, 'title05', 1, 1010)");
			s.addBatch("insert into t (id, name, maincategory) values (6, 'title06', 1)");
			s.addBatch("insert into t (id, name, maincategory, subcategory) values (7, 'title07', 3, 402)");
			s.addBatch("insert into t (id, name, maincategory, subcategory) values (8, 'title08', 4, 31)");

			int	numberOfTuples = 8;
			int	currentUpdateCount = 8;
			int[]		ids				= { 1,			2,			3,			4,			5,			6,			7,			8			};
			String[]	names			= { "title01",	"title02",	"title03",	"title04",	"title05",	"title06",	"title07",	"title08"	};
			int[]		maincategories	= { 2,			1,			4,			2,			1,			1,			3,			4			};
			int[]		subcategories	= { 1024,		1001,		-1,			1020,		1010,		-1,			402,		31			};

			int[]	updateCounts = null;
			assertNotNull(updateCounts = s.executeBatch());
			assertEquals(currentUpdateCount, updateCounts.length);
			int	expected = 1;
			for (int i = 0; i < currentUpdateCount; i++) assertEquals(expected, updateCounts[i]);
			s.close();

			assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);
		}

		//
		// 正常系−その２（ insert での確認 − 手動コミットモード）
		//

		{
			c.setAutoCommit(false);

			//
			// commit
			//

			assertNotNull(s = c.createStatement());

			s.addBatch("insert into t (id, name, maincategory) values (9, 'title09', 5)");
			s.addBatch("insert into t (id, name, maincategory, subcategory) values (10, 'title10', 3, 1025)");
			s.addBatch("insert into t (id, name, maincategory, subcategory) values (11, 'title11', 4, 2)");

			int	numberOfTuples = 11;
			int	currentUpdateCount = 3;
			int[]		ids				= { 1,			2,			3,			4,			5,			6,			7,			8,			9,			10,			11			};
			String[]	names			= { "title01",	"title02",	"title03",	"title04",	"title05",	"title06",	"title07",	"title08",	"title09",	"title10",	"title11"	};
			int[]		maincategories	= { 2,			1,			4,			2,			1,			1,			3,			4,			5,			3,			4			};
			int[]		subcategories	= { 1024,		1001,		-1,			1020,		1010,		-1,			402,		31,			-1,			1025,		2			};

			int[]	updateCounts = null;
			assertNotNull(updateCounts = s.executeBatch());
			assertEquals(currentUpdateCount, updateCounts.length);
			for (int i = 0; i < currentUpdateCount; i++) assertEquals(1, updateCounts[i]);
			c.commit();
			s.close();
			c.setAutoCommit(true);

			assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);

			c.setAutoCommit(false);

			//
			// rollback
			//

			assertNotNull(s = c.createStatement());
			currentUpdateCount = 2;
			s.addBatch("insert into t (id, name, maincategory, subcategory) values (12, 'title12', 3, 1001)");
			s.addBatch("insert into t (id, name, maincategory, subcategory) values (13, 'title13', 3, 1002)");
			assertNotNull(updateCounts = s.executeBatch());
			assertEquals(currentUpdateCount, updateCounts.length);
			for (int i = 0; i < currentUpdateCount; i++) assertEquals(1, updateCounts[i]);
			c.rollback();
			s.close();
			c.setAutoCommit(true);

			assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);
		}

		//
		// 正常系−その３（ update での確認 − 自動コミットモード）
		//

		{
			assertNotNull(s = c.createStatement());

			s.addBatch("update t set maincategory = 1 where id = 3");
			s.addBatch("update t set subcategory = 34 where maincategory = 3");

			int	numberOfTuples = 11;
			int[]	currentUpdateCounts = { 1, 2 };
			int[]		ids				= { 1,			2,			3,			4,			5,			6,			7,			8,			9,			10,			11			};
			String[]	names			= { "title01",	"title02",	"title03",	"title04",	"title05",	"title06",	"title07",	"title08",	"title09",	"title10",	"title11"	};
			int[]		maincategories	= { 2,			1,			1,			2,			1,			1,			3,			4,			5,			3,			4			};
			int[]		subcategories	= { 1024,		1001,		-1,			1020,		1010,		-1,			34,			31,			-1,			34,			2			};

			int[]	updateCounts = null;
			assertNotNull(updateCounts = s.executeBatch());
			assertEquals(currentUpdateCounts.length, updateCounts.length);
			for (int i = 0; i < currentUpdateCounts.length; i++) assertEquals(currentUpdateCounts[i], updateCounts[i]);
			s.close();

			assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);
		}

		//
		// 正常系−その４（ update での確認 − 手動コミットモード）
		//

		{
			c.setAutoCommit(false);

			//
			// commit
			//

			assertNotNull(s = c.createStatement());

			s.addBatch("update t set maincategory = 1 where id = 4");
			s.addBatch("update t set subcategory = 0 where subcategory is null");

			int	numberOfTuples = 11;
			int[]	currentUpdateCounts = { 1, 3 };
			int[]		ids				= { 1,			2,			3,			4,			5,			6,			7,			8,			9,			10,			11			};
			String[]	names			= { "title01",	"title02",	"title03",	"title04",	"title05",	"title06",	"title07",	"title08",	"title09",	"title10",	"title11"	};
			int[]		maincategories	= { 2,			1,			1,			1,			1,			1,			3,			4,			5,			3,			4			};
			int[]		subcategories	= { 1024,		1001,		0,			1020,		1010,		0,			34,			31,			0,			34,			2			};

			int[]	updateCounts = null;
			assertNotNull(updateCounts = s.executeBatch());
			assertEquals(currentUpdateCounts.length, updateCounts.length);
			for (int i = 0; i < currentUpdateCounts.length; i++) assertEquals(currentUpdateCounts[i], updateCounts[i]);
			c.commit();
			s.close();
			c.setAutoCommit(true);

			assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);

			c.setAutoCommit(false);

			//
			// rollback
			//

			assertNotNull(s = c.createStatement());

			s.addBatch("update t set name = 'foo' where maincategory = 5");
			s.addBatch("update t set id = 9 where name = 'title04'");
			s.addBatch("update t set maincategory = 6 where maincategory = 4");

			int[]	currentUpdateCounts2 = { 1, 1, 2 };

			int[]	updateCounts2 = null;
			assertNotNull(updateCounts2 = s.executeBatch());
			assertEquals(currentUpdateCounts2.length, updateCounts2.length);
			for (int i = 0; i < currentUpdateCounts2.length; i++) assertEquals(currentUpdateCounts2[i], updateCounts2[i]);
			c.rollback();
			s.close();
			c.setAutoCommit(true);

			assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);
		}

		//
		// 正常系−その５（ delete での確認 − 自動コミットモード）
		//

		{
			assertNotNull(s = c.createStatement());

			s.addBatch("delete from t where maincategory = 1");
			s.addBatch("delete from t where subcategory < 10");

			int	numberOfTuples = 4;
			int[]	currentUpdateCounts = { 5, 2 };
			int[]		ids				= { 1,			7,			8,			10			};
			String[]	names			= { "title01",	"title07",	"title08",	"title10"	};
			int[]		maincategories	= { 2,			3,			4,			3			};
			int[]		subcategories	= { 1024,		34,			31,			34			};

			int[]	updateCounts = null;
			assertNotNull(updateCounts = s.executeBatch());
			assertEquals(currentUpdateCounts.length, updateCounts.length);
			for (int i = 0; i < currentUpdateCounts.length; i++) assertEquals(currentUpdateCounts[i], updateCounts[i]);
			s.close();

			assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);
		}

		//
		// 正常系−その６（ delete での確認 − 手動コミットモード）
		//

		{
			c.setAutoCommit(false);

			//
			// commit
			//

			assertNotNull(s = c.createStatement());

			s.addBatch("delete from t where subcategory = 34");
			s.addBatch("delete from t where name = 'title01'");

			int	numberOfTuples = 1;
			int[]	currentUpdateCounts = { 2, 1 };
			int[]		ids				= { 8			};
			String[]	names			= { "title08"	};
			int[]		maincategories	= { 4			};
			int[]		subcategories	= { 31			};

			int[]	updateCounts = null;
			assertNotNull(updateCounts = s.executeBatch());
			assertEquals(currentUpdateCounts.length, updateCounts.length);
			for (int i = 0; i < currentUpdateCounts.length; i++) assertEquals(currentUpdateCounts[i], updateCounts[i]);
			c.commit();
			s.close();
			c.setAutoCommit(true);

			assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);

			c.setAutoCommit(false);

			//
			// rollback
			//

			assertNotNull(s = c.createStatement());

			s.addBatch("delete from t where id = 8");

			int[]	currentUpdateCounts2 = { 1 };

			int[]	updateCounts2 = null;
			assertNotNull(updateCounts2 = s.executeBatch());
			assertEquals(currentUpdateCounts.length, updateCounts.length);
			for (int i = 0; i < currentUpdateCounts2.length; i++) assertEquals(currentUpdateCounts2[i], updateCounts2[i]);
			c.rollback();
			s.close();
			c.setAutoCommit(true);

			assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);
		}

		//
		// 該当しない検索条件で更新 − 自動コミットモード
		//

		{
			assertNotNull(s = c.createStatement());

			s.addBatch("update t set name = 'foo' where id = 1");
			s.addBatch("delete from t where maincategory = 3");

			int	numberOfTuples = 1;
			int[]	currentUpdateCounts = { 0, 0 };
			int[]		ids				= { 8			};
			String[]	names			= { "title08"	};
			int[]		maincategories	= { 4			};
			int[]		subcategories	= { 31			};

			int[]	updateCounts = null;
			assertNotNull(updateCounts = s.executeBatch());
			assertEquals(currentUpdateCounts.length, updateCounts.length);
			for (int i = 0; i < currentUpdateCounts.length; i++) assertEquals(currentUpdateCounts[i], updateCounts[i]);
			s.close();

			assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);
		}

		//
		// 該当しない検索条件で更新 − 手動コミットモード
		//

		{
			c.setAutoCommit(false);

			//
			// commit
			//

			assertNotNull(s = c.createStatement());

			s.addBatch("update t set maincategory = 0 where maincategory = 1");
			s.addBatch("delete from t where name = 'foo'");
			s.addBatch("update t set subcategory = 101 where id = 9");

			int	numberOfTuples = 1;
			int[]	currentUpdateCounts = { 0, 0, 0 };
			int[]		ids				= { 8			};
			String[]	names			= { "title08"	};
			int[]		maincategories	= { 4			};
			int[]		subcategories	= { 31			};

			int[]	updateCounts = null;
			assertNotNull(updateCounts = s.executeBatch());
			assertEquals(currentUpdateCounts.length, updateCounts.length);
			for (int i = 0; i < currentUpdateCounts.length; i++) assertEquals(currentUpdateCounts[i], updateCounts[i]);
			c.commit();
			s.close();
			c.setAutoCommit(true);

			assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);

			c.setAutoCommit(false);

			//
			// rollback
			//

			assertNotNull(s = c.createStatement());

			s.addBatch("update t set id = 3 where maincategory = 3");
			s.addBatch("update t set name = 'foo' where name = 'title00'");
			s.addBatch("update t set maincategory = 100 where maincategory = 1 and subcategory = 30");

			int[]	updateCounts2 = null;
			assertNotNull(updateCounts2 = s.executeBatch());
			assertEquals(currentUpdateCounts.length, updateCounts2.length);
			for (int i = 0; i < currentUpdateCounts.length; i++) assertEquals(currentUpdateCounts[i], updateCounts2[i]);
			c.rollback();
			s.close();
			c.setAutoCommit(true);

			assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);
		}

		//
		// clearBatch のチェック
		//

		{
			assertNotNull(s = c.createStatement());

			s.addBatch("insert into t (id, name, maincategory, subcategory) values (1001, 'titleA', 3, 103)");
			s.addBatch("insert into t (id, name, maincategory, subcategory) values (1002, 'titleB', 2, 51)");
			s.addBatch("insert into t (id, name, maincategory, subcategory) values (1003, 'titleC', 2, 55)");

			int	numberOfTuples = 1;
			int[]		ids				= { 8			};
			String[]	names			= { "title08"	};
			int[]		maincategories	= { 4			};
			int[]		subcategories	= { 31			};

			s.clearBatch();
			int[]	updateCounts = null;
			assertNotNull(updateCounts = s.executeBatch());
			assertZero(updateCounts.length);
			s.close();

			assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);
		}

		//
		// 既に閉じた Statement に対して addBatch() と executeBatch() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		{
			assertNotNull(s = c.createStatement());

			s.close();

			s.addBatch("insert into t (id, name, maincategory, subcategory) values (2001, 'hogehoge', 5, 1)");
			s.addBatch("insert into t (id, name, maincategory, subcategory) values (2002, 'hoge', 3, 1001)");
			s.addBatch("update t set name = 'x' where id > 2000");

			int	numberOfTuples = 3;
			int[]	currentUpdateCounts = { 1, 1, 2 };
			int[]		ids				= { 8,			2001,		2002	};
			String[]	names			= { "title08",	"x",		"x"		};
			int[]		maincategories	= { 4,			5,			3		};
			int[]		subcategories	= { 31,			1,			1001	};

			int[]	updateCounts = null;
			assertNotNull(updateCounts = s.executeBatch());
			assertEquals(currentUpdateCounts.length, updateCounts.length);
			for (int i = 0; i < currentUpdateCounts.length; i++) assertEquals(currentUpdateCounts[i], updateCounts[i]);

			assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);
		}

		//
		// 既に閉じた Statement に対して clearBatch() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		{
			assertNotNull(s = c.createStatement());

			s.close();

			s.addBatch("delete from t");
			s.clearBatch();

			int	numberOfTuples = 3;
			int[]		ids				= { 8,			2001,		2002	};
			String[]	names			= { "title08",	"x",		"x"		};
			int[]		maincategories	= { 4,			5,			3		};
			int[]		subcategories	= { 31,			1,			1001	};

			int[]	updateCounts = null;
			assertNotNull(updateCounts = s.executeBatch());
			assertZero(updateCounts.length);

			assertTuples(c, numberOfTuples, ids, names, maincategories, subcategories);
		}

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// Statement.getConnection() のテスト
	public void test_getConnection() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		assertEquals(c, s.getConnection());

		s.close();

		//
		// 閉じた Statement に対して getConnection() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		assertEquals(c, s.getConnection());

		c.close();
	}

	// Statement.getResultSetHoldability() のテスト
	public void test_getResultSetHoldability() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		// 常に ResultSet.CLOSE_CUSORS_AT_COMMIT が得られるはず
		assertEquals(ResultSet.CLOSE_CURSORS_AT_COMMIT, s.getResultSetHoldability());

		s.close();

		//
		// 閉じた Statement に対して getResultSetHoldability() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		assertEquals(ResultSet.CLOSE_CURSORS_AT_COMMIT, s.getResultSetHoldability());

		c.close();
	}

	// Statement.setCursorName() のテスト
	public void test_setCursorName() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		SQLWarning	w = null;

		java.util.Vector	wEC = new java.util.Vector();	// error code
		java.util.Vector	wSS = new java.util.Vector();	// SQLState
		java.util.Vector	wMS = new java.util.Vector();	// message

		// 現状ではカーソルをサポートしていないので警告が出るはず
		// （警告その１）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CURSOR_NOT_SUPPORT);
		s.setCursorName("hogehoge");
		// 警告のチェック
		assertSQLWarning(s.getWarnings(), wEC, wSS, wMS);

		// カーソル名に null を指定しても、未サポートの警告が出るはず
		// （警告その２）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CURSOR_NOT_SUPPORT);
		s.setCursorName(null);
		// 警告のチェック
		assertSQLWarning(s.getWarnings(), wEC, wSS, wMS);

		// カーソル名に空文字列を指定しても、未サポートの警告が出るはず
		// （警告その３）
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CURSOR_NOT_SUPPORT);
		s.setCursorName("");
		// 警告のチェック
		assertSQLWarning(s.getWarnings(), wEC, wSS, wMS);

		s.clearWarnings();

		s.close();

		//
		// 閉じた Statement に対して setCursorName() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		s.setCursorName("foo");

		c.close();
	}

	// Statement.setEscapeProcessing() のテスト
	public void test_setEscapeProcessing() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		// 現状では setEscapeProcessing() をサポートしていないはず
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			s.setEscapeProcessing(false);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);
		SQLState = "";
		try {
			s.setEscapeProcessing(true);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		s.close();
		c.close();
	}

	// Statement.setQueryTimeout() と Statement.getQueryTimeout() のテスト
	public void test_setAndGetQueryTimeout() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		// デフォルト状態でも“無制限”を示す 0 が得られるはず
		assertZero(s.getQueryTimeout());

		// 現状では setQueryTimeout() をサポートしておらず何もしないはず
		s.setQueryTimeout(10);

		// 現状では getQueryTimeout() をサポートしていないので、
		// 常に“無制限”を示す 0 が得られるはず
		assertZero(s.getQueryTimeout());

		// 同上

		s.setQueryTimeout(10000);
		assertZero(s.getQueryTimeout());

		s.setQueryTimeout(100000);
		assertZero(s.getQueryTimeout());

		// 負数では例外 BadArgument が throw されるはず
		String	BadArgumentSQLState = (new BadArgument()).getSQLState();
		String	SQLState = "";
		try {
			s.setQueryTimeout(-1);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(BadArgumentSQLState, SQLState);

		s.close();

		//
		// 閉じた Statement に対して setQueryTimeout() と getQueryTimeout()
		// を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		s.setQueryTimeout(60);
		assertZero(s.getQueryTimeout());

		c.close();
	}

	// Statement.cancel() のテスト
	public void test_cancel() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);
		Statement	s = null;
		assertNotNull(s = c.createStatement());
		int	numberOfTuples = 1000;
		for (int i = 0; i < numberOfTuples; i++)
			s.executeUpdate("insert into t (id, name, maincategory, subcategory) values (1, 'userX', 1,  1)");
		s.close();

		assertNotNull(s = c.createStatement());
		String	query = "select * from t";
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery(query));

		//
		// ※ cancel() したからといって、すぐに ResultSet.next() が
		// 　 false を返すわけではない。
		// 　 若干のブランクがある。
		//

		// YET! キャンセルされたら警告が積まれる仕様となったので、ちゃんと getWarnings() してチェックする！

		int	t = 0;
		assertTrue(rs.next());
		t++;
		s.cancel();
		while (rs.next()) t++;
		assertFalse(rs.next());
		assertTrue(t < numberOfTuples); // でも全件取得する前にキャンセルされるはず

		// ここで、下のテストのために ResultSet はわざと閉じない

		s.close();

		//
		// 閉じた Statement に対して cancel() を呼び出しても問題ない…
		// とりあえずこれは仕様。
		//

		s.cancel();

		// 後処理

		c.close();
	}

	// Statement.getWarnings() と Statement.clearWarnings() のテスト
	public void test_getAndClearWarnings() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		// getWarnings() と clearWarnings() は、
		// setFetchDirection() などのテストで
		// 散々やっているので、ここでは省く。

		s.close();

		//
		// 閉じた Statement に対して getWarnings() を呼び出すと
		// 例外 SessionNotAvailable が発生するはず
		//

		String	SessionNotAvailableSQLState = (new SessionNotAvailable()).getSQLState();
		String	SQLState = "";
		try {
			s.getWarnings();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);

		c.close();
	}

	// Statement.execute(String) と Statement.execute(String, int) と
	// Statement.execute(String, int[]) と Statement.execute(String, String[]) のテスト
	public void test_execute() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		createTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		//
		// 現状では（引数にかかわらず）execute() をサポートしていないはず
		//

		String		query = 
			"insert into t(id) values (1), (2), (3); update t set id = id * 2; delete t where id = 2; select * from t";
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		boolean result = false;
		try {
			result = s.execute(query);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}

		// execute has been supported
		assertEquals("", SQLState);
		assertTrue(result);
		ResultSet r = null;
		assertNotNull(r = s.getResultSet()); // result of insert
		while (r.next());
		assertEquals(3, s.getUpdateCount());
		assertEquals(-1, s.getUpdateCount());
		assertTrue(s.getMoreResults());
		assertNotNull(r = s.getResultSet()); // result of update
		while (r.next());
		assertEquals(3, s.getUpdateCount());
		assertEquals(-1, s.getUpdateCount());
		assertTrue(s.getMoreResults());
		assertNotNull(r = s.getResultSet()); // result of delete
		while (r.next());
		assertEquals(1, s.getUpdateCount());
		assertEquals(-1, s.getUpdateCount());
		assertTrue(s.getMoreResults());
		assertNotNull(r = s.getResultSet()); // result of select
		while (r.next());
		assertEquals(2, s.getUpdateCount());
		assertEquals(-1, s.getUpdateCount());
		assertFalse(s.getMoreResults());
		assertEquals(-1, s.getUpdateCount());

		int			autoGeneratedKeys = Statement.RETURN_GENERATED_KEYS;
		SQLState = "";
		try {
			s.execute(query, autoGeneratedKeys);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		int[]		columnIndexes = { 1, 2 };
		SQLState = "";
		try {
			s.execute(query, columnIndexes);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		String[]	columnNames = { "c1", "c2" };
		SQLState = "";
		try {
			s.execute(query, columnNames);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		s.close();
		dropTestTable(c);

		c.close();
	}

	// Statement.getResultSet() のテスト
	public void test_getResultSet() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		//
		// 現状では execute() をサポートしていないので
		// getResultSet() もサポートしていないはず
		//

		ResultSet r = null;
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			r = s.getResultSet();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		// getResultSet has been supported
		assertEquals("", SQLState);
		assertNull(r);

		s.close();
		c.close();
	}

	// Statement.getUpdateCount() のテスト
	public void test_getUpdateCount() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		//
		// 現状では getUpdateCount() をサポートしていないはず
		//

		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			s.getUpdateCount();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals("", SQLState);

		s.close();
		c.close();
	}

	// Statement.getMoreResults() と Statement.getMoreResults(int) のテスト
	public void test_getMoreResults() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		//
		// 現状では getMoreResults() をサポートしていないはず
		//

		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			s.getMoreResults();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals("", SQLState);

		SQLState = "";
		try {
			s.getMoreResults(Statement.CLOSE_CURRENT_RESULT);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		s.close();
		c.close();
	}

	// Statement.getGeneratedKeys() のテスト
	public void test_getGeneratedKeys() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		//
		// 現状では getGeneratedKeys() をサポートしていないはず
		//

		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			s.getGeneratedKeys();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		s.close();
		c.close();
	}

	// Statement.executeUpdate(String, int) と Statement.executeUpdate(String, int[]) と
	// Statement.executeUpdate(String, String[]) のテスト
	public void test_executeUpdate2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		//
		// 現状では SQL 文を唯一の引数として取る executeUpdate() 以外の
		// executeUpdate() をサポートしていないはず
		//

		String	query = "select * from t";
		int		autoGeneratedKeys = Statement.RETURN_GENERATED_KEYS;
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try {
			s.executeUpdate(query, autoGeneratedKeys);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		int[]	columnIndexes = { 1, 2 };
		SQLState = "";
		try {
			s.executeUpdate(query, columnIndexes);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		String[]	columnNames = { "c1", "c2" };
		SQLState = "";
		try {
			s.executeUpdate(query, columnNames);
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);

		s.close();
		c.close();
	}

	// 『JDBCのPreparedStatementを使用するとSocketがリークする』の再現
	public void test_0326() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);
		Statement	s = null;
		assertNotNull(s = c.createStatement());
		s.executeUpdate("insert into t (id, name, maincategory, subcategory) values (1, 'foo',  3, 101)");
		s.executeUpdate("insert into t (id, name, maincategory, subcategory) values (2, 'hoge', 3, 102)");
		s.executeUpdate("insert into t (id, name, maincategory) values (3, 'abc', 2)");
		s.close();
		c.close();

		int	i = 0;
		try {
			for (i = 0; i < 10000; i++) {
				assertNotNull(c = getConnection());
				assertNotNull(s = c.createStatement());
				ResultSet	rs = null;
				assertNotNull(rs = s.executeQuery("select * from t"));
				while (rs.next());
				rs.close();
				s.close();
				c.close();
			}
		} catch (SQLException	sqle) {
			System.err.println("counter = " + i);
			sqle.printStackTrace();
			throw sqle;
		}

		assertNotNull(c = getConnection());

		// 後処理
		dropTestTable(c);

		c.close();
	}

	// Sydney 障害管理−障害処理票No.0380『JDBC ドライバが BIGINT 型に対応していない』に関連して、
	// int 列への java.lang.Integer.MIN_VALUE / java.lang.Integer.MAX_VALUE 挿入テスト
	// および bigint 列への java.lang.Long.MIN_VALUE / java.lang.LONG.MAX_VALUE 挿入テスト
	public void test_0380() throws Exception
	{

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createContainsBigIntColumnTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		ResultSet	rs = null;

		//
		// 正常系
		//

		try {

			// int 列へ java.lang.Integer.MIN_VALUE をリテラルで
			s.executeUpdate("insert into t (intColumn) values (" + Integer.MIN_VALUE + ")");

			// int 列へ java.lang.Integer.MIN_VALUE を文字列で
			s.executeUpdate("insert into t (intColumn) values ('" + Integer.MIN_VALUE + "')");

			// int 列へ java.lang.Integer.MAX_VALUE をリテラルで
			s.executeUpdate("insert into t (intColumn) values (" + Integer.MAX_VALUE + ")");

			// int 列へ java.lang.Integer.MAX_VALUE を文字列で
			s.executeUpdate("insert into t (intColumn) values ('" + Integer.MAX_VALUE + "')");

			// bigint 列へ java.lang.Long.MIN_VALUE をリテラルで
			s.executeUpdate("insert into t (bigintColumn) values (" + Long.MIN_VALUE + ")");

			// bigint 列へ java.lang.Long.MIN_VALUE を文字列で
			s.executeUpdate("insert into t (bigintColumn) values ('" + Long.MIN_VALUE + "')");

			// bigint 列へ java.lang.Long.MAX_VALUE をリテラルで
			s.executeUpdate("insert into t (bigintColumn) values (" + Long.MAX_VALUE + ")");

			// bigint 列へ java.lang.Long.MAX_VALUE を文字列で
			s.executeUpdate("insert into t (bigintColumn) values ('" + Long.MAX_VALUE + "')");

			//
			// t.intColumn には java.lang.Integer.MIN_VALUE がふたつ、java.lang.Integer.MAX_VALUE がふたつ、挿入されたはず
			//

			assertNotNull(rs = s.executeQuery("select intColumn from t where intColumn is not null order by intColumn"));
			assertTrue(rs.next());	assertEquals(Integer.MIN_VALUE, rs.getInt(1));
			assertTrue(rs.next());	assertEquals(Integer.MIN_VALUE, rs.getInt(1));
			assertTrue(rs.next());	assertEquals(Integer.MAX_VALUE, rs.getInt(1));
			assertTrue(rs.next());	assertEquals(Integer.MAX_VALUE, rs.getInt(1));
			assertFalse(rs.next());

			//
			// t.bigintColumn には
			//

			assertNotNull(rs = s.executeQuery("select bigintColumn from t where bigintColumn is not null order by bigintColumn"));
			assertTrue(rs.next());	assertEquals(Long.MIN_VALUE, rs.getLong(1));
			assertTrue(rs.next());	assertEquals(Long.MIN_VALUE, rs.getLong(1));
			assertTrue(rs.next());	assertEquals(Long.MAX_VALUE, rs.getLong(1));
			assertTrue(rs.next());	assertEquals(Long.MAX_VALUE, rs.getLong(1));
			assertFalse(rs.next());

		} catch (SQLException	sqle) {
			sqle.printStackTrace();
		}

		if (rs != null) rs.close();

		//
		// 異常系 ( out of range )
		//

		// int 列へ java.lang.Integer.MIN_VALUE - 1 をリテラルで
		java.math.BigInteger	bigInteger = null;
		boolean	caught = false;
		try {
			bigInteger = (new java.math.BigInteger(Integer.toString(Integer.MIN_VALUE))).add(java.math.BigInteger.ONE.negate());
			s.executeUpdate("insert into t (intColumn) values (" + bigInteger.toString() + ")");
		} catch (SQLException	sqle) {
			caught = true;
			assertNumericValueOutOfRange(sqle);
		}
		assertTrue(caught);

		// int 列へ java.lang.Integer.MIN_VALUE - 1 を文字列で
		caught = false;
		try {
			s.executeUpdate("insert into t (intColumn) values ('" + bigInteger.toString() + "')");
		} catch (SQLException	sqle) {
			caught = true;
			assertNumericValueOutOfRange(sqle);
		}
		assertTrue(caught);

		// int 列へ java.lang.Integer.MAX_VALUE + 1 をリテラルで
		bigInteger = (new java.math.BigInteger(Integer.toString(Integer.MAX_VALUE))).add(java.math.BigInteger.ONE);
		caught = false;
		try {
			s.executeUpdate("insert into t (intColumn) values (" + bigInteger.toString() + ")");
		} catch (SQLException	sqle) {
			caught = true;
			assertNumericValueOutOfRange(sqle);
		}
		assertTrue(caught);

		// int 列へ java.lang.Integer.MAX_VALUE + 1 を文字列で
		caught = false;
		try {
			s.executeUpdate("insert into t (intColumn) values ('" + bigInteger.toString() + "')");
		} catch (SQLException	sqle) {
			caught = true;
			assertNumericValueOutOfRange(sqle);
		}
		assertTrue(caught);

		// bigint 列へ java.lang.Long.MIN_VALUE - 1 をリテラルで
		bigInteger = (new java.math.BigInteger(Long.toString(Long.MIN_VALUE))).add(java.math.BigInteger.ONE.negate());
		caught = false;
		try {
			s.executeUpdate("insert into t (bigintColumn) values (" + bigInteger.toString() + ")");
		} catch (SQLException	sqle) {
			caught = true;
			assertNumericValueOutOfRange(sqle);
		}
		assertTrue(caught);

		// bigint 列へ java.lang.Long.MIN_VALUE - 1 を文字列で
		caught = false;
		try {
			s.executeUpdate("insert into t (bigintColumn) values ('" + bigInteger.toString() + "')");
		} catch (SQLException	sqle) {
			caught = true;
			assertNumericValueOutOfRange(sqle);
		}
		assertTrue(caught);

		// bigint 列へ java.lang.Long.MAX_VALUE + 1 をリテラルで
		bigInteger = (new java.math.BigInteger(Long.toString(Long.MAX_VALUE))).add(java.math.BigInteger.ONE);
		caught = false;
		try {
			s.executeUpdate("insert into t (bigintColumn) values (" + bigInteger.toString() + ")");
		} catch (SQLException	sqle) {
			caught = true;
			assertNumericValueOutOfRange(sqle);
		}
		assertTrue(caught);

		// bigint 列へ java.lang.Long.MAX_VALUE + 1 を文字列で
		caught = false;
		try {
			s.executeUpdate("insert into t (bigintColumn) values ('" + bigInteger.toString() + "')");
		} catch (SQLException	sqle) {
			caught = true;
			assertNumericValueOutOfRange(sqle);
		}
		assertTrue(caught);

		s.close();

		// 後処理
		dropTestTable(c);

		c.close();
	}

	public void test_closeOnCompletion() throws Exception {
		Connection	c = null;
		assertNotNull(c = getConnection());

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		assertFalse(s.isCloseOnCompletion());
		
		String	NotSupportedSQLState = (new NotSupported()).getSQLState();
		String	SQLState = "";
		try{
			s.closeOnCompletion();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(NotSupportedSQLState, SQLState);
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
		s.executeUpdate("create table t (id int, name nvarchar(100), maincategory int, subcategory int)");
		s.close();
	}

	private void createContainsBigIntColumnTestTable(Connection	c) throws Exception
	{
		Statement	s = null;
		assertNotNull(s = c.createStatement());
		s.executeUpdate("create table t (intColumn int, bigintColumn bigint)");
		s.close();
	}

	private void dropTestTable(Connection	c) throws Exception
	{
		Statement	s = c.createStatement();
		s.executeUpdate("drop table t");
		s.close();
	}

	private void assertTuples(	Connection	c,
								int			numberOfTuples,
								int[]		ids,
								String[]	names,
								int[]		maincategories,
								int[]		subcategories) throws Exception
	{
		Statement	s = null;
		assertNotNull(s = c.createStatement());

		String	query = "select count(*) from t";
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery(query));
		assertTrue(rs.next());
		assertEquals(numberOfTuples, rs.getInt(1));	assertFalse(rs.wasNull());
		assertFalse(rs.next());
		rs.close();
		s.close();

		assertNotNull(s = c.createStatement());
		String	alias = "number_of_tuples";
		query = "select count(*) as " + alias + " from t";
		assertNotNull(rs = s.executeQuery(query));
		assertTrue(rs.next());
		assertEquals(numberOfTuples, rs.getInt(alias));	assertFalse(rs.wasNull());
		assertFalse(rs.next());
		rs.close();
		s.close();

		assertNotNull(s = c.createStatement());
		query = "select * from t order by id";
		assertNotNull(rs = s.executeQuery(query));
		for (int i = 0; i < numberOfTuples; i++) {

			assertTrue(rs.next());
			assertEquals(ids[i], rs.getInt(1));				assertFalse(rs.wasNull());
			assertEquals(names[i], rs.getString(2));		assertFalse(rs.wasNull());
			if (maincategories[i] < 0) {
				rs.getInt(3);	assertTrue(rs.wasNull());
			} else {
				assertEquals(maincategories[i], rs.getInt(3));	assertFalse(rs.wasNull());
			}
			if (subcategories[i] < 0) {
				rs.getInt(4);	assertTrue(rs.wasNull());
			} else {
				assertEquals(subcategories[i], rs.getInt(4));	assertFalse(rs.wasNull());
			}
		}
		assertFalse(rs.next());
		rs.close();
		s.close();

		assertNotNull(s = c.createStatement());
		query = "select name, maincategory, subcategory, id from t order by id";
		assertNotNull(rs = s.executeQuery(query));
		for (int i = 0; i < numberOfTuples; i++) {

			assertTrue(rs.next());
			assertEquals(names[i], rs.getString("name"));	assertFalse(rs.wasNull());
			if (maincategories[i] < 0) {
				rs.getInt("maincategory");	assertTrue(rs.wasNull());
			} else {
				assertEquals(maincategories[i], rs.getInt("maincategory"));	assertFalse(rs.wasNull());
			}
			if (subcategories[i] < 0) {
				rs.getInt("subcategory");	assertTrue(rs.wasNull());
			} else {
				assertEquals(subcategories[i], rs.getInt("subcategory"));	assertFalse(rs.wasNull());
			}
			assertEquals(ids[i], rs.getInt("id"));	assertFalse(rs.wasNull());
		}
		assertFalse(rs.next());
		rs.close();
		s.close();
	}

	private void assertNumericValueOutOfRange(SQLException	e) throws Exception
	{
		assertEquals((new NumericValueOutOfRange()).getSQLState(), e.getSQLState());
	}
}

//
// Copyright (c) 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
