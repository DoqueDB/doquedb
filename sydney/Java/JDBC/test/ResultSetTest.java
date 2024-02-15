// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ResultSetTest.java -- 
// 
// Copyright (c) 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
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
import java.math.BigDecimal;

import jp.co.ricoh.doquedb.exception.*;
import jp.co.ricoh.doquedb.common.*;

public class ResultSetTest extends TestBase
{
	private final static String	WM_CANCELROWUPDATES_NO_PERFORMED =
		"cursor was not supported, no cancelRowUpdates method were performed.";

	private final static String	WM_REFRESHROW_NO_PERFORMED =
			"cursor was not supported, no refreshRow method were performed.";

	private final static String	WM_CURRENTLY_SUPPORTED_ONLY_FETCH_FORWARD =
				"direction of ResultSet currently supported is only java.sql.ResultSet.FETCH_FORWARD.";

	private final static String	WM_FETCH_SIZE_SUPPORTED_ONLY_ZERO =
				"hint about the number of rows which needs to be taken out from a database is not supported other than zero.";

	public ResultSetTest(String nickname)
	{
		super(nickname);
	}

	// ResultSet.cancelRowUpdates() のテスト
	public void test_cancelRowUpdates() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c, 1);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		assertNull(rs.getWarnings());

		java.util.Vector	wEC = new java.util.Vector();	// error code
		java.util.Vector	wSS = new java.util.Vector();	// SQLState
		java.util.Vector	wMS = new java.util.Vector();	// message

		// 現状ではカーソルをサポートしていないので警告が出るはず
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CANCELROWUPDATES_NO_PERFORMED);
		rs.cancelRowUpdates();
		// 警告のチェック
		assertSQLWarning(rs.getWarnings(), wEC, wSS, wMS);

		// next しても警告が出るはず
		assertTrue(rs.next());
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CANCELROWUPDATES_NO_PERFORMED);
		rs.cancelRowUpdates();
		// 警告のチェック
		assertSQLWarning(rs.getWarnings(), wEC, wSS, wMS);

		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getWarnings() と ResultSet.clearWarnings() のテスト
	public void test_getAndClearWarnings() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c, 1);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		// getWarnings() と clearWarnings() は、
		// cancelRowUpdates() などのテストで
		// やっているので、ここでは省く。

		rs.close();

		//
		// 閉じた ResultSet に対して getWarnings() を呼び出すと
		// 例外 SessionNotAvailable が発生するはず
		//

		String	SessionNotAvailableSQLState = (new SessionNotAvailable()).getSQLState();
		String	SQLState = "";
		try {
			rs.getWarnings();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);

		assertNotNull(rs = s.executeQuery("select * from t"));

		s.close();

		//
		// Statement を閉じると自動的に ResultSet も閉じられるので
		// この場合も getWarnings() を呼び出すと例外 SessionNotAvailable が発生するはず
		//

		SQLState = "";
		try {
			rs.getWarnings();
		} catch (SQLException	sqle) {
			SQLState = sqle.getSQLState();
		}
		assertEquals(SessionNotAvailableSQLState, SQLState);

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.close() のテスト
	public void test_close() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c, 3);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		// next せずに close

		rs.close();

		// next ですべてを読みきる前に close

		assertNotNull(rs = s.executeQuery("select * from t"));
		assertTrue(rs.next());
		rs.close();

		// next ですべて読みきった後に close

		assertNotNull(rs = s.executeQuery("select * from t"));
		while (rs.next());
		rs.close();

		// 何回呼び出したって平気
		for (int i = 0; i < 10; i++) rs.close();

		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.findColumn() のテスト − その１
	public void test_findColumn1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		s.executeUpdate("insert into t (f_int_not_null, f_int1, f_char8_not_null, f_int2) values (101, 1, 'foo', 3)");
		s.close();

		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		// next しないと例外 NotSupported が throw されるはず
		boolean	caught = false;
		try {
			rs.findColumn("f_int1");
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		int	columnIndex = 1;

		assertTrue(rs.next());
		assertEquals(columnIndex++, rs.findColumn("f_int_not_null"));
		assertEquals(columnIndex++, rs.findColumn("f_int1"));
		assertEquals(columnIndex++, rs.findColumn("f_int2"));
		assertEquals(columnIndex++, rs.findColumn("f_bigint"));
			assertEquals(columnIndex++, rs.findColumn("f_decimal"));
		assertEquals(columnIndex++, rs.findColumn("f_char8_not_null"));
		assertEquals(columnIndex++, rs.findColumn("f_char8"));
		assertEquals(columnIndex++, rs.findColumn("f_float"));
		assertEquals(columnIndex++, rs.findColumn("f_datetime"));
		assertEquals(columnIndex++, rs.findColumn("f_id"));
		assertEquals(columnIndex++, rs.findColumn("f_image"));
		assertEquals(columnIndex++, rs.findColumn("f_language"));
		assertEquals(columnIndex++, rs.findColumn("f_nchar6"));
		assertEquals(columnIndex++, rs.findColumn("f_nvarchar256"));
		assertEquals(columnIndex++, rs.findColumn("f_varchar128"));
		assertEquals(columnIndex++, rs.findColumn("f_ntext"));
		assertEquals(columnIndex++, rs.findColumn("f_ntext_compressed"));
		assertEquals(columnIndex++, rs.findColumn("f_fulltext"));
		assertEquals(columnIndex++, rs.findColumn("f_binary50"));
		assertEquals(columnIndex++, rs.findColumn("f_blob"));
		assertEquals(columnIndex++, rs.findColumn("f_nclob"));
		assertEquals(columnIndex++, rs.findColumn("af_int"));
		assertEquals(columnIndex++, rs.findColumn("af_bigint"));
		assertEquals(columnIndex++, rs.findColumn("af_decimal"));
		assertEquals(columnIndex++, rs.findColumn("af_char8"));
		assertEquals(columnIndex++, rs.findColumn("af_float"));
		assertEquals(columnIndex++, rs.findColumn("af_datetime"));
		assertEquals(columnIndex++, rs.findColumn("af_id"));
		assertEquals(columnIndex++, rs.findColumn("af_image"));
		assertEquals(columnIndex++, rs.findColumn("af_language"));
		assertEquals(columnIndex++, rs.findColumn("af_nchar6"));
		assertEquals(columnIndex++, rs.findColumn("af_nvarchar256"));
		assertEquals(columnIndex++, rs.findColumn("af_varchar128"));
		assertEquals(columnIndex++, rs.findColumn("af_ntext"));
		assertEquals(columnIndex++, rs.findColumn("af_ntext_compressed"));
		assertEquals(columnIndex++, rs.findColumn("af_fulltext"));
		assertEquals(columnIndex++, rs.findColumn("af_binary50"));

		// 存在しない列名を指定すると例外 EntryNotFound が throw されるはず
		caught = false;
		try {
			rs.findColumn("f_charx");
		} catch (SQLException	sqle) {
			caught = true;
			assertEntryNotFound(sqle);
		}
		assertTrue(caught);

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.findColumn() のテスト − その２
	public void test_findColumn2() throws Exception
	{

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		s.executeUpdate("insert into t (f_int_not_null, f_int1, f_char8_not_null, f_nchar6) values (102, 2, 'hogehoge', 'abcdef')");
		s.close();

		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select f_int1, f_int_not_null, f_varchar128, f_nchar6, f_char8_not_null, af_id from t"));

		// next しないと例外 NotSupported が throw されるはず
		boolean	caught = false;
		try {
			rs.findColumn("f_int1");
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		assertTrue(rs.next());
		assertEquals(1, rs.findColumn("f_int1"));
		assertEquals(2, rs.findColumn("f_int_not_null"));
		assertEquals(3, rs.findColumn("f_varchar128"));
		assertEquals(4, rs.findColumn("f_nchar6"));
		assertEquals(5, rs.findColumn("f_char8_not_null"));
		assertEquals(6, rs.findColumn("af_id"));

		// 存在はするが select 句で指定されていない列名を指定すると例外 EntryNotFound が throw されるはず
		caught = false;
		try {
			rs.findColumn("f_language");
		} catch (SQLException	sqle) {
			caught = true;
			assertEntryNotFound(sqle);
		}
		assertTrue(caught);

		// 存在しない列名を指定すると例外 EntryNotFound が throw されるはず
		caught = false;
		try {
			rs.findColumn("f_binary500");
		} catch (SQLException	sqle) {
			caught = true;
			assertEntryNotFound(sqle);
		}
		assertTrue(caught);

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.findColumn() のテスト − その３
	public void test_findColumn3() throws Exception
	{

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		s.executeUpdate("insert into t (f_int_not_null, f_int1, f_char8_not_null, f_fulltext) values (103, 3, 'abc', 'alias and functions')");
		s.close();

		//
		// 別名
		//

		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select f_int1 as primary_key, f_int_not_null as tid from t"));

		// next しないと例外 NotSupported が throw されるはず
		boolean	caught = false;
		try {
			rs.findColumn("primary_key");
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		assertTrue(rs.next());
		assertEquals(1, rs.findColumn("primary_key"));
		assertEquals(2, rs.findColumn("tid"));

		// 別名を使うと元の名前では例外 EntryNotFound が throw されるはず
		caught = false;
		try {
			rs.findColumn("f_int1");
		} catch (SQLException	sqle) {
			caught = true;
			assertEntryNotFound(sqle);
		}
		assertTrue(caught);
		caught = false;
		try {
			rs.findColumn("f_int_not_null");
		} catch (SQLException	sqle) {
			caught = true;
			assertEntryNotFound(sqle);
		}
		assertTrue(caught);

		rs.close();

		//
		// 関数−その１
		//

		assertNotNull(rs = s.executeQuery("select count(*) from t"));

		// next しないと例外 NotSupported が throw されるはず
		caught = false;
		try {
			rs.findColumn("count(*)");
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		assertTrue(rs.next());
		assertEquals(1, rs.findColumn("count(*)"));

		rs.close();

		//
		// 関数−その２
		//

		assertNotNull(rs = s.executeQuery("select max(f_int_not_null) from t"));

		// next しないと例外 NotSupported が throw されるはず
		caught = false;
		try {
			rs.findColumn("max(f_int_not_null)");
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		assertTrue(rs.next());
		assertEquals(1, rs.findColumn("max(f_int_not_null)"));

		rs.close();

		//
		// 関数の別名
		//

		assertNotNull(rs = s.executeQuery("select count(*) as num_t from t"));

		// next しないと例外 NotSupported が throw されるはず
		caught = false;
		try {
			rs.findColumn("num_t");
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		assertTrue(rs.next());
		assertEquals(1, rs.findColumn("num_t"));

		// 別名を使うと元の名前では例外 EntryNotFound が throw されるはず
		caught = false;
		try {
			rs.findColumn("count(*)");
		} catch (SQLException	sqle) {
			caught = true;
			assertEntryNotFound(sqle);
		}
		assertTrue(caught);

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getArray(int) のテスト
	public void test_getArray1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	e = new ClassCast();

			int	columnIndex = 1;

			// INT (not null) 列を getArray
			assertGetArray(rs, columnIndex++, f_int_not_nulls[i], e);		// f_int_not_null

			// INT (primary key) 列を getArray
			assertGetArray(rs, columnIndex++, f_int1s[i], e);				// f_int1

			// INT 列を getArray
			assertGetArray(rs, columnIndex++, f_int2s[i], e);				// f_int2

			// BIGINT 列を getArray
			assertGetArray(rs, columnIndex++, f_bigints[i], e);				// f_bigint
			assertGetArray(rs, columnIndex++, f_decimals[i], e);			// f_decimal

			// CHAR (not null) 列を getArray
			assertGetArray(rs, columnIndex++, f_char8_not_nulls[i], e);		// f_char8_not_null

			// CHAR 列を getArray
			assertGetArray(rs, columnIndex++, f_char8s[i], e);				// f_char8

			// FLOAT 列を getArray
			assertGetArray(rs, columnIndex++, f_floats[i], e);				// f_float

			// DATETIME 列を getArray
			assertGetArray(rs, columnIndex++, f_datetimes[i], e);			// f_datetime

			// UNIQUEIDENTIFIER 列を getArray
			assertGetArray(rs, columnIndex++, f_ids[i], e);					// f_id

			// IMAGE 列を getArray
			assertGetArray(rs, columnIndex++, f_images[i], e);				// f_image

			// LANGUAGE 列を getArray
			assertGetArray(rs, columnIndex++, f_languages[i], e);			// f_language

			// NCHAR 列を getArray
			assertGetArray(rs, columnIndex++, f_nchar6s[i], e);				// f_nchar6

			// NVARCHAR 列を getArray
			assertGetArray(rs, columnIndex++, f_nvarchar256s[i], e);		// f_nvarchar256

			// VARCHAR 列を getArray
			assertGetArray(rs, columnIndex++, f_varchar128s[i], e);			// f_varchar128

			// NTEXT 列を getArray
			assertGetArray(rs, columnIndex++, f_ntexts[i], e);				// f_ntext

			// NTEXT (compressed) 列を getArray
			assertGetArray(rs, columnIndex++, f_ntext_compresseds[i], e);	// f_ntext_compressed

			// FULLTEXT 列を getArray
			assertGetArray(rs, columnIndex++, f_fulltexts[i], e);			// f_fulltext

			// BINARY 列を getArray
			assertGetArray(rs, columnIndex++, f_binary50s[i], e);			// f_binary50

			// BLOB 列を getArray
			assertGetArray(rs, columnIndex++, f_blobs[i], e);				// f_blob

			// NCLOB 列を getArray
			assertGetArray(rs, columnIndex++, f_nclobs[i], e);				// f_nclob

			// INT 配列を getArray
			assertGetArray(rs, columnIndex++, af_ints[i]);					// af_int

			// BIGINT 配列を getArray
			assertGetArray(rs, columnIndex++, af_bigints[i]);				// af_bigint
			assertGetArray(rs, columnIndex++, af_decimals[i]);				// af_decimal

			// CHAR 配列を getArray
			assertGetArray(rs, columnIndex++, af_char8s[i]);				// af_char8

			// FLOAT 配列を getArray
			assertGetArray(rs, columnIndex++, af_floats[i]);				// af_float

			// DATETIME 配列を getArray
			assertGetArray(rs, columnIndex++, af_datetimes[i]);				// af_datetime

			// UNIQUEIDENTIFIER 配列を getArray
			assertGetArray(rs, columnIndex++, af_ids[i]);					// af_id

			// IMAGE 配列を getArray
			assertGetArray(rs, columnIndex++, af_images[i]);				// af_image

			// LANGUAGE 配列を getArray
			assertGetArray(rs, columnIndex++, af_languages[i]);				// af_language

			// NCHAR 配列を getArray
			assertGetArray(rs, columnIndex++, af_nchar6s[i]);				// af_nchar6

			// NVARCHAR 配列を getArray
			assertGetArray(rs, columnIndex++, af_nvarchar256s[i]);			// af_nvarchar256

			// VARCHAR 配列を getArray
			assertGetArray(rs, columnIndex++, af_varchar128s[i]);			// af_varchar128

			// NTEXT 配列を getArray
			assertGetArray(rs, columnIndex++, af_ntexts[i]);				// af_ntext

			// NTEXT (compressed) 配列を getArray
			assertGetArray(rs, columnIndex++, af_ntext_compresseds[i]);		// af_ntext_compressed

			// FULLTEXT 配列を getArray
			assertGetArray(rs, columnIndex++, af_fulltexts[i]);				// af_fulltext

			// BINARY 配列を getArray
			assertGetArray(rs, columnIndex++, af_binary50s[i]);				// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getArray(String) のテスト
	public void test_getArray2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	e;
			e = new ClassCast();

			// INT (not null) 列を getArray
			assertGetArray(rs, "f_int_not_null", f_int_not_nulls[i], e);

			// INT (primary key) 列を getArray
			assertGetArray(rs, "f_int1", f_int1s[i], e);

			// INT 列を getArray
			assertGetArray(rs, "f_int2", f_int2s[i], e);

			// BIGINT 列を getArray
			assertGetArray(rs, "f_bigint", f_bigints[i], e);
			assertGetArray(rs, "f_decimal", f_decimals[i], e);

			// CHAR (not null) 列を getArray
			assertGetArray(rs, "f_char8_not_null", f_char8_not_nulls[i], e);

			// CHAR 列を getArray
			assertGetArray(rs, "f_char8", f_char8s[i], e);

			// FLOAT 列を getArray
			assertGetArray(rs, "f_float", f_floats[i], e);

			// DATETIME 列を getArray
			assertGetArray(rs, "f_datetime", f_datetimes[i], e);

			// UNIQUEIDENTIFIER 列を getArray
			assertGetArray(rs, "f_id", f_ids[i], e);

			// IMAGE 列を getArray
			assertGetArray(rs, "f_image", f_images[i], e);

			// LANGUAGE 列を getArray
			assertGetArray(rs, "f_language", f_languages[i], e);

			// NCHAR 列を getArray
			assertGetArray(rs, "f_nchar6", f_nchar6s[i], e);

			// NVARCHAR 列を getArray
			assertGetArray(rs, "f_nvarchar256", f_nvarchar256s[i], e);

			// VARCHAR 列を getArray
			assertGetArray(rs, "f_varchar128", f_varchar128s[i], e);

			// NTEXT 列を getArray
			assertGetArray(rs, "f_ntext", f_ntexts[i], e);

			// NTEXT (compressed) 列を getArray
			assertGetArray(rs, "f_ntext_compressed", f_ntext_compresseds[i], e);

			// FULLTEXT 列を getArray
			assertGetArray(rs, "f_fulltext", f_fulltexts[i], e);

			// BINARY 列を getArray
			assertGetArray(rs, "f_binary50", f_binary50s[i], e);

			// BLOB 列を getArray
			assertGetArray(rs, "f_blob", f_blobs[i], e);

			// NCLOB 列を getArray
			assertGetArray(rs, "f_nclob", f_nclobs[i], e);

			// INT 配列を getArray
			assertGetArray(rs, "af_int", af_ints[i]);

			// BIGINT 配列を getArray
			assertGetArray(rs, "af_bigint", af_bigints[i]);
			assertGetArray(rs, "af_decimal", af_decimals[i]);

			// CHAR 配列を getArray
			assertGetArray(rs, "af_char8", af_char8s[i]);

			// FLOAT 配列を getArray
			assertGetArray(rs, "af_float", af_floats[i]);

			// DATETIME 配列を getArray
			assertGetArray(rs, "af_datetime", af_datetimes[i]);

			// UNIQUEIDENTIFIER 配列を getArray
			assertGetArray(rs, "af_id", af_ids[i]);

			// IMAGE 配列を getArray
			assertGetArray(rs, "af_image", af_images[i]);

			// LANGUAGE 配列を getArray
			assertGetArray(rs, "af_language", af_languages[i]);

			// NCHAR 配列を getArray
			assertGetArray(rs, "af_nchar6", af_nchar6s[i]);

			// NVARCHAR 配列を getArray
			assertGetArray(rs, "af_nvarchar256", af_nvarchar256s[i]);

			// VARCHAR 配列を getArray
			assertGetArray(rs, "af_varchar128", af_varchar128s[i]);

			// NTEXT 配列を getArray
			assertGetArray(rs, "af_ntext", af_ntexts[i]);

			// NTEXT (compressed) 配列を getArray
			assertGetArray(rs, "af_ntext_compressed", af_ntext_compresseds[i]);

			// FULLTEXT 配列を getArray
			assertGetArray(rs, "af_fulltext", af_fulltexts[i]);

			// BINARY 配列を getArray
			assertGetArray(rs, "af_binary50", af_binary50s[i]);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getBigDecimal(int) のテスト
	public void test_getBigDecimal1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			int	columnIndex = 1;

			// INT (not null) 列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, f_int_not_nulls[i]);			// f_int_not_null

			// INT (primary key) 列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, f_int1s[i]);					// f_int1

			// INT 列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, f_int2s[i]);					// f_int2

			// BIGINT 列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, f_bigints[i]);				// f_bigint
			assertGetBigDecimal(rs, columnIndex++, f_decimals[i]);				// f_decimal

			// CHAR (not null) 列を getBigDecimal
			e = checkParseBigInteger(f_char8_not_nulls[i]);
			assertGetBigDecimal(rs, columnIndex++, f_char8_not_nulls[i], e);	// f_char8_not_null

			// CHAR 列を getBigDecimal
			e = checkParseBigInteger(f_char8s[i]);
			assertGetBigDecimal(rs, columnIndex++, f_char8s[i], e);				// f_char8

			// FLOAT 列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, f_floats[i]);				// f_float

			// DATETIME 列を getBigDecimal
			e = classCast;
			assertGetBigDecimal(rs, columnIndex++, f_datetimes[i], e);			// f_datetime

			// UNIQUEIDENTIFIER 列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, f_ids[i], e);				// f_id

			// IMAGE 列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, f_images[i], e);				// f_image

			// LANGUAGE 列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, f_languages[i], e);			// f_language

			// NCHAR 列を getBigDecimal
			e = checkParseBigInteger(f_nchar6s[i]);
			assertGetBigDecimal(rs, columnIndex++, f_nchar6s[i], e);			// f_nchar6

			// NVARCHAR 列を getBigDecimal
			e = checkParseBigInteger(f_nvarchar256s[i]);
			assertGetBigDecimal(rs, columnIndex++, f_nvarchar256s[i], e);		// f_nvarchar256

			// VARCHAR 列を getBigDecimal
			e = checkParseBigInteger(f_varchar128s[i]);
			assertGetBigDecimal(rs, columnIndex++, f_varchar128s[i], e);		// f_varchar128

			// NTEXT 列を getBigDecimal
			e = checkParseBigInteger(f_ntexts[i]);
			assertGetBigDecimal(rs, columnIndex++, f_ntexts[i], e);				// f_ntext

			// NTEXT (compressed) 列を getBigDecimal
			e = checkParseBigInteger(f_ntext_compresseds[i]);
			assertGetBigDecimal(rs, columnIndex++, f_ntext_compresseds[i], e);	// f_ntext_compressed

			// FULLTEXT 列を getBigDecimal
			e = checkParseBigInteger(f_fulltexts[i]);
			assertGetBigDecimal(rs, columnIndex++, f_fulltexts[i], e);			// f_fulltext

			// BINARY 列を getBigDecimal
			e = classCast;
			assertGetBigDecimal(rs, columnIndex++, f_binary50s[i], e);			// f_binary50

			// BLOB 列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, f_blobs[i], e);				// f_blob

			// NCLOB 列を getBigDecimal
			e = checkParseBigInteger(f_nclobs[i]);
			assertGetBigDecimal(rs, columnIndex++, f_nclobs[i], e);				// f_nclob

			// INT 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, af_ints[i], e);				// af_int

			// BIGINT 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, af_bigints[i], e);			// af_bigint
			assertGetBigDecimal(rs, columnIndex++, af_decimals[i], e);			// af_decimal

			// CHAR 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, af_char8s[i], e);			// af_char8

			// FLOAT 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, af_floats[i], e);			// af_float

			// DATETIME 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, af_datetimes[i], e);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getArray
			assertGetBigDecimal(rs, columnIndex++, af_ids[i], e);				// af_id

			// IMAGE 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, af_images[i], e);			// af_image

			// LANGUAGE 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, af_languages[i], e);			// af_language

			// NCHAR 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, af_nchar6s[i], e);			// af_nchar6

			// NVARCHAR 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, af_nvarchar256s[i], e);		// af_nvarchar256

			// VARCHAR 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, af_varchar128s[i], e);		// af_varchar128

			// NTEXT 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, af_ntexts[i], e);			// af_ntext

			// NTEXT (compressed) 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, af_ntext_compresseds[i], e);	// af_ntext_compressed

			// FULLTEXT 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, af_fulltexts[i], e);			// af_fulltext

			// BINARY 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, af_binary50s[i], e);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getBigDecimal(int, int) のテスト
	public void test_getBigDecimal2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		int	scale = 2;

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			int	columnIndex = 1;

			// INT (not null) 列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, f_int_not_nulls[i]);			// f_int_not_null

			// INT (primary key) 列をgetBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, f_int1s[i]);					// f_int1

			// INT 列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, f_int2s[i]);					// f_int2

			// BIGINT 列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, f_bigints[i]);				// f_bigint
			assertGetBigDecimal(rs, columnIndex++, scale, f_decimals[i]);				// f_decimal

			// CHAR (not null) 列を getBigDecimal
			e = checkParseBigInteger(f_char8_not_nulls[i]);
			assertGetBigDecimal(rs, columnIndex++, scale, f_char8_not_nulls[i], e);		// f_char8_not_null

			// CHAR 列を getBigDecimal
			e = checkParseBigInteger(f_char8s[i]);
			assertGetBigDecimal(rs, columnIndex++, scale, f_char8s[i], e);				// f_char8

			// FLOAT 列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, f_floats[i]);					// f_float

			// DATETIME 列を getBigDecimal
			e = classCast;
			assertGetBigDecimal(rs, columnIndex++, scale, f_datetimes[i], e);			// f_datetime

			// UNIQUEIDENTIFIER 列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, f_ids[i], e);					// f_id

			// IMAGE 列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, f_images[i], e);				// f_image

			// LANGUAGE 列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, f_languages[i], e);			// f_language

			// NCHAR 列を getBigDecimal
			e = checkParseBigInteger(f_nchar6s[i]);
			assertGetBigDecimal(rs, columnIndex++, scale, f_nchar6s[i], e);				// f_nchar6

			// NVARCHAR 列を getBigDecimal
			e = checkParseBigInteger(f_nvarchar256s[i]);
			assertGetBigDecimal(rs, columnIndex++, scale, f_nvarchar256s[i], e);		// f_nvarchar256

			// VARCHAR 列を getBigDecimal
			e = checkParseBigInteger(f_varchar128s[i]);
			assertGetBigDecimal(rs, columnIndex++, scale, f_varchar128s[i], e);			// f_varchar128

			// NTEXT 列を getBigDecimal
			e = checkParseBigInteger(f_ntexts[i]);
			assertGetBigDecimal(rs, columnIndex++, scale, f_ntexts[i], e);				// f_ntext

			// NTEXT (compressed) 列を getBigDecimal
			e = checkParseBigInteger(f_ntext_compresseds[i]);
			assertGetBigDecimal(rs, columnIndex++, scale, f_ntext_compresseds[i], e);	// f_ntext_compressed

			// FULLTEXT 列を getBigDecimal
			e = checkParseBigInteger(f_fulltexts[i]);
			assertGetBigDecimal(rs, columnIndex++, scale, f_fulltexts[i], e);			// f_fulltext

			// BINARY 列を getBigDecimal
			e = classCast;
			assertGetBigDecimal(rs, columnIndex++, scale, f_binary50s[i], e);			// f_binary50

			// BLOB 列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, f_blobs[i], e);				// f_blob

			// NCLOB 列を getBigDecimal
			e = checkParseBigInteger(f_nclobs[i]);
			assertGetBigDecimal(rs, columnIndex++, scale, f_nclobs[i], e);				// f_nclob

			// INT 配列を getBigDecimal
			e = classCast;
			assertGetBigDecimal(rs, columnIndex++, scale, af_ints[i], e);				// af_int

			// BIGINT 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, af_bigints[i], e);				// af_bigint
			assertGetBigDecimal(rs, columnIndex++, scale, af_decimals[i], e);				// af_decimal

			// CHAR 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, af_char8s[i], e);				// af_char8

			// FLOAT 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, af_floats[i], e);				// af_float

			// DATETIME 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, af_datetimes[i], e);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getArray
			assertGetBigDecimal(rs, columnIndex++, scale, af_ids[i], e);				// af_id

			// IMAGE 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, af_images[i], e);				// af_image

			// LANGUAGE 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, af_languages[i], e);			// af_language

			// NCHAR 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, af_nchar6s[i], e);			// af_nchar6

			// NVARCHAR 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, af_nvarchar256s[i], e);		// af_nvarchar256

			// VARCHAR 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, af_varchar128s[i], e);		// af_varchar128

			// NTEXT 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, af_ntexts[i], e);				// af_ntext

			// NTEXT (compressed) 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, af_ntext_compresseds[i], e);	// af_ntext_compressed

			// FULLTEXT 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, af_fulltexts[i], e);			// af_fulltext

			// BINARY 配列を getBigDecimal
			assertGetBigDecimal(rs, columnIndex++, scale, af_binary50s[i], e);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getBigDecimal(String) のテスト
	public void test_getBigDecimal3() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			// INT (not null) 列を getBigDecimal
			assertGetBigDecimal(rs, "f_int_not_null", f_int_not_nulls[i]);

			// INT (primary key) 列を getBigDecimal
			assertGetBigDecimal(rs, "f_int1", f_int1s[i]);

			// INT 列を getBigDecimal
			assertGetBigDecimal(rs, "f_int2", f_int2s[i]);

			// BIGINT 列を getBigDecimal
			assertGetBigDecimal(rs, "f_bigint", f_bigints[i]);
			assertGetBigDecimal(rs, "f_decimal", f_decimals[i]);

			// CHAR (not null) 列を getBigDecimal
			e = checkParseBigInteger(f_char8_not_nulls[i]);
			assertGetBigDecimal(rs, "f_char8_not_null", f_char8_not_nulls[i], e);

			// CHAR 列を getBigDecimal
			e = checkParseBigInteger(f_char8s[i]);
			assertGetBigDecimal(rs, "f_char8", f_char8s[i], e);

			// FLOAT 列を getBigDecimal
			assertGetBigDecimal(rs, "f_float", f_floats[i]);

			// DATETIME 列を getBigDecimal
			e = classCast;
			assertGetBigDecimal(rs, "f_datetime", f_datetimes[i], e);

			// UNIQUEIDENTIFIER 列を getBigDecimal
			assertGetBigDecimal(rs, "f_id", f_ids[i], e);

			// IMAGE 列を getBigDecimal
			assertGetBigDecimal(rs, "f_image", f_images[i], e);

			// LANGUAGE 列を getBigDecimal
			assertGetBigDecimal(rs, "f_language", f_languages[i], e);

			// NCHAR 列を getBigDecimal
			e = checkParseBigInteger(f_nchar6s[i]);
			assertGetBigDecimal(rs, "f_nchar6", f_nchar6s[i], e);

			// NVARCHAR 列を getBigDecimal
			e = checkParseBigInteger(f_nvarchar256s[i]);
			assertGetBigDecimal(rs, "f_nvarchar256", f_nvarchar256s[i], e);

			// VARCHAR 列を getBigDecimal
			e = checkParseBigInteger(f_varchar128s[i]);
			assertGetBigDecimal(rs, "f_varchar128", f_varchar128s[i], e);

			// NTEXT 列を getBigDecimal
			e = checkParseBigInteger(f_ntexts[i]);
			assertGetBigDecimal(rs, "f_ntext", f_ntexts[i], e);

			// NTEXT (compressed) 列を getBigDecimal
			e = checkParseBigInteger(f_ntext_compresseds[i]);
			assertGetBigDecimal(rs, "f_ntext_compressed", f_ntext_compresseds[i], e);

			// FULLTEXT 列を getBigDecimal
			e = checkParseBigInteger(f_fulltexts[i]);
			assertGetBigDecimal(rs, "f_fulltext", f_fulltexts[i], e);

			// BINARY 列を getBigDecimal
			e = classCast;
			assertGetBigDecimal(rs, "f_binary50", f_binary50s[i], e);

			// BLOB 列を getBigDecimal
			assertGetBigDecimal(rs, "f_blob", f_blobs[i], e);

			// NCLOB 列を getBigDecimal
			e = checkParseBigInteger(f_nclobs[i]);
			assertGetBigDecimal(rs, "f_nclob", f_nclobs[i], e);

			// INT 配列を getBigDecimal
			e = classCast;
			assertGetBigDecimal(rs, "af_int", af_ints[i], e);

			// BIGINT 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_bigint", af_bigints[i], e);
			assertGetBigDecimal(rs, "af_decimal", af_decimals[i], e);

			// CHAR 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_char8", af_char8s[i], e);

			// FLOAT 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_float", af_floats[i], e);

			// DATETIME 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_datetime", af_datetimes[i], e);

			// UNIQUEIDENTIFIER 配列を getArray
			assertGetBigDecimal(rs, "af_id", af_ids[i], e);

			// IMAGE 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_image", af_images[i], e);

			// LANGUAGE 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_language", af_languages[i], e);

			// NCHAR 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_nchar6", af_nchar6s[i], e);

			// NVARCHAR 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_nvarchar256", af_nvarchar256s[i], e);

			// VARCHAR 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_varchar128", af_varchar128s[i], e);

			// NTEXT 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_ntext", af_ntexts[i], e);

			// NTEXT (compressed) 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_ntext_compressed", af_ntext_compresseds[i], e);

			// FULLTEXT 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_fulltext", af_fulltexts[i], e);

			// BINARY 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_binary50", af_binary50s[i], e);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getBigDecimal(String, int) のテスト
	public void test_getBigDecimal4() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		int	scale = 5;

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			// INT (not null) 列を getBigDecimal
			assertGetBigDecimal(rs, "f_int_not_null", scale, f_int_not_nulls[i]);

			// INT (primary key) 列を getBigDecimal
			assertGetBigDecimal(rs, "f_int1", scale, f_int1s[i]);

			// INT 列を getBigDecimal
			assertGetBigDecimal(rs, "f_int2", scale, f_int2s[i]);

			// BIGINT 列を getBigDecimal
			assertGetBigDecimal(rs, "f_bigint", scale, f_bigints[i]);
			assertGetBigDecimal(rs, "f_decimal", scale, f_decimals[i]);

			// CHAR (not null) 列を getBigDecimal
			e = checkParseBigInteger(f_char8_not_nulls[i]);
			assertGetBigDecimal(rs, "f_char8_not_null", scale, f_char8_not_nulls[i], e);

			// CHAR 列を getBigDecimal
			e = checkParseBigInteger(f_char8s[i]);
			assertGetBigDecimal(rs, "f_char8", scale, f_char8s[i], e);

			// FLOAT 列を getBigDecimal
			assertGetBigDecimal(rs, "f_float", scale, f_floats[i]);

			// DATETIME 列を getBigDecimal
			e = classCast;
			assertGetBigDecimal(rs, "f_datetime", scale, f_datetimes[i], e);

			// UNIQUEIDENTIFIER 列を getBigDecimal
			assertGetBigDecimal(rs, "f_id", scale, f_ids[i], e);

			// IMAGE 列を getBigDecimal
			assertGetBigDecimal(rs, "f_image", scale, f_images[i], e);

			// LANGUAGE 列を getBigDecimal
			assertGetBigDecimal(rs, "f_language", scale, f_languages[i], e);

			// NCHAR 列を getBigDecimal
			e = checkParseBigInteger(f_nchar6s[i]);
			assertGetBigDecimal(rs, "f_nchar6", scale, f_nchar6s[i], e);

			// NVARCHAR 列を getBigDecimal
			e = checkParseBigInteger(f_nvarchar256s[i]);
			assertGetBigDecimal(rs, "f_nvarchar256", scale, f_nvarchar256s[i], e);

			// VARCHAR 列を getBigDecimal
			e = checkParseBigInteger(f_varchar128s[i]);
			assertGetBigDecimal(rs, "f_varchar128", scale, f_varchar128s[i], e);

			// NTEXT 列を getBigDecimal
			e = checkParseBigInteger(f_ntexts[i]);
			assertGetBigDecimal(rs, "f_ntext", scale, f_ntexts[i], e);

			// NTEXT (compressed) 列を getBigDecimal
			e = checkParseBigInteger(f_ntext_compresseds[i]);
			assertGetBigDecimal(rs, "f_ntext_compressed", scale, f_ntext_compresseds[i], e);

			// FULLTEXT 列を getBigDecimal
			e = checkParseBigInteger(f_fulltexts[i]);
			assertGetBigDecimal(rs, "f_fulltext", scale, f_fulltexts[i], e);

			// BINARY 列を getBigDecimal
			e = classCast;
			assertGetBigDecimal(rs, "f_binary50", scale, f_binary50s[i], e);

			// BLOB 列を getBigDecimal
			assertGetBigDecimal(rs, "f_blob", scale, f_blobs[i], e);

			// NCLOB 列を getBigDecimal
			e = checkParseBigInteger(f_nclobs[i]);
			assertGetBigDecimal(rs, "f_nclob", scale, f_nclobs[i], e);

			// INT 配列を getBigDecimal
			e = classCast;
			assertGetBigDecimal(rs, "af_int", scale, af_ints[i], e);

			// BIGINT 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_bigint", scale, af_bigints[i], e);
			assertGetBigDecimal(rs, "af_decimal", scale, af_decimals[i], e);

			// CHAR 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_char8", scale, af_char8s[i], e);

			// FLOAT 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_float", scale, af_floats[i], e);

			// DATETIME 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_datetime", scale, af_datetimes[i], e);

			// UNIQUEIDENTIFIER 配列を getArray
			assertGetBigDecimal(rs, "af_id", scale, af_ids[i], e);

			// IMAGE 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_image", scale, af_images[i], e);

			// LANGUAGE 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_language", scale, af_languages[i], e);

			// NCHAR 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_nchar6", scale, af_nchar6s[i], e);

			// NVARCHAR 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_nvarchar256", scale, af_nvarchar256s[i], e);

			// VARCHAR 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_varchar128", scale, af_varchar128s[i], e);

			// NTEXT 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_ntext", scale, af_ntexts[i], e);

			// NTEXT (compressed) 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_ntext_compressed", scale, af_ntext_compresseds[i], e);

			// FULLTEXT 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_fulltext", scale, af_fulltexts[i], e);

			// BINARY 配列を getBigDecimal
			assertGetBigDecimal(rs, "af_binary50", scale, af_binary50s[i], e);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getBinaryStream(int) のテスト
	public void test_getBinaryStream1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	e = new ClassCast();

			int	columnIndex = 1;

			// INT (not null) 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_int_not_nulls[i], e);		// f_int_not_null

			// INT (primary key) 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_int1s[i], e);				// f_int1

			// INT 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_int2s[i], e);				// f_int2

			// BIGINT 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_bigints[i], e);			// f_bigint
			assertGetBinaryStream(rs, columnIndex++, f_decimals[i], e);			// f_decimal

			// CHAR (not null) 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_char8_not_nulls[i], e);		// f_char8_not_null

			// CHAR 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_char8s[i], e);				// f_char8

			// FLOAT 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_floats[i], e);				// f_float

			// DATETIME 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_datetimes[i], e);			// f_datetime

			// UNIQUEIDENTIFIER 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_ids[i], e);					// f_id

			// IMAGE 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_images[i]);					// f_image

			// LANGUAGE 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_languages[i], e);			// f_language

			// NCHAR 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_nchar6s[i], e);				// f_nchar6

			// NVARCHAR 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_nvarchar256s[i], e);			// f_nvarchar256

			// VARCHAR 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_varchar128s[i], e);			// f_varchar128

			// NTEXT 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_ntexts[i], e);				// f_ntext

			// NTEXT (compressed) 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_ntext_compresseds[i], e);	// f_ntext_compressed

			// FULLTEXT 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_fulltexts[i], e);			// f_fulltext

			// BINARY 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_binary50s[i]);				// f_binary50

			// BLOB 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_blobs[i]);					// f_blob

			// NCLOB 列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, f_nclobs[i], e);				// f_nclob

			// INT 配列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, af_ints[i], e);				// af_int

			// BIGINT 配列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, af_bigints[i], e);				// af_bigint
			assertGetBinaryStream(rs, columnIndex++, af_decimals[i], e);			// af_decimal

			// CHAR 配列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, af_char8s[i], e);				// af_char8

			// FLOAT 配列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, af_floats[i], e);				// af_float

			// DATETIME 配列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, af_datetimes[i], e);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, af_ids[i], e);					// af_id

			// IMAGE 配列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, af_images[i], e);				// af_image

			// LANGUAGE 配列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, af_languages[i], e);			// af_language

			// NCHAR 配列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, af_nchar6s[i], e);				// af_nchar6

			// NVARCHAR 配列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, af_nvarchar256s[i], e);		// af_nvarchar256

			// VARCHAR 配列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, af_varchar128s[i], e);			// af_varchar128

			// NTEXT 配列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, af_ntexts[i], e);				// af_ntext

			// NTEXT (compressed) 配列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, af_ntext_compresseds[i], e);	// af_ntext_compressed

			// FULLTEXT 配列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, af_fulltexts[i], e);			// af_fulltext

			// BINARY 配列を getBinaryStream
			assertGetBinaryStream(rs, columnIndex++, af_binary50s[i], e);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getBinaryStream(String) のテスト
	public void test_getBinaryStream2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	e = new ClassCast();

			// INT (not null) 列を getBinaryStream
			assertGetBinaryStream(rs, "f_int_not_null", f_int_not_nulls[i], e);

			// INT (primary key) 列を getBinaryStream
			assertGetBinaryStream(rs, "f_int1", f_int1s[i], e);

			// INT 列を getBinaryStream
			assertGetBinaryStream(rs, "f_int2", f_int2s[i], e);

			// BIGINT 列を getBinaryStream
			assertGetBinaryStream(rs, "f_bigint", f_bigints[i], e);
			assertGetBinaryStream(rs, "f_decimal", f_decimals[i], e);

			// CHAR (not null) 列を getBinaryStream
			assertGetBinaryStream(rs, "f_char8_not_null", f_char8_not_nulls[i], e);

			// CHAR 列を getBinaryStream
			assertGetBinaryStream(rs, "f_char8", f_char8s[i], e);

			// FLOAT 列を getBinaryStream
			assertGetBinaryStream(rs, "f_float", f_floats[i], e);

			// DATETIME 列を getBinaryStream
			assertGetBinaryStream(rs, "f_datetime", f_datetimes[i], e);

			// UNIQUEIDENTIFIER 列を getBinaryStream
			assertGetBinaryStream(rs, "f_id", f_ids[i], e);

			// IMAGE 列を getBinaryStream
			assertGetBinaryStream(rs, "f_image", f_images[i]);

			// LANGUAGE 列を getBinaryStream
			assertGetBinaryStream(rs, "f_language", f_languages[i], e);

			// NCHAR 列を getBinaryStream
			assertGetBinaryStream(rs, "f_nchar6", f_nchar6s[i], e);

			// NVARCHAR 列を getBinaryStream
			assertGetBinaryStream(rs, "f_nvarchar256", f_nvarchar256s[i], e);

			// VARCHAR 列を getBinaryStream
			assertGetBinaryStream(rs, "f_varchar128", f_varchar128s[i], e);

			// NTEXT 列を getBinaryStream
			assertGetBinaryStream(rs, "f_ntext", f_ntexts[i], e);

			// NTEXT (compressed) 列を getBinaryStream
			assertGetBinaryStream(rs, "f_ntext_compressed", f_ntext_compresseds[i], e);

			// FULLTEXT 列を getBinaryStream
			assertGetBinaryStream(rs, "f_fulltext", f_fulltexts[i], e);

			// BINARY 列を getBinaryStream
			assertGetBinaryStream(rs, "f_binary50", f_binary50s[i]);

			// BLOB 列を getBinaryStream
			assertGetBinaryStream(rs, "f_blob", f_blobs[i]);

			// NCLOB 列を getBinaryStream
			assertGetBinaryStream(rs, "f_nclob", f_nclobs[i], e);

			// INT 配列を getBinaryStream
			assertGetBinaryStream(rs, "af_int", af_ints[i], e);

			// BIGINT 配列を getBinaryStream
			assertGetBinaryStream(rs, "af_bigint", af_bigints[i], e);
			assertGetBinaryStream(rs, "af_decimal", af_decimals[i], e);

			// CHAR 配列を getBinaryStream
			assertGetBinaryStream(rs, "af_char8", af_char8s[i], e);

			// FLOAT 配列を getBinaryStream
			assertGetBinaryStream(rs, "af_float", af_floats[i], e);

			// DATETIME 配列を getBinaryStream
			assertGetBinaryStream(rs, "af_datetime", af_datetimes[i], e);

			// UNIQUEIDENTIFIER 配列を getBinaryStream
			assertGetBinaryStream(rs, "af_id", af_ids[i], e);

			// IMAGE 配列を getBinaryStream
			assertGetBinaryStream(rs, "af_image", af_images[i], e);

			// LANGUAGE 配列を getBinaryStream
			assertGetBinaryStream(rs, "af_language", af_languages[i], e);

			// NCHAR 配列を getBinaryStream
			assertGetBinaryStream(rs, "af_nchar6", af_nchar6s[i], e);

			// NVARCHAR 配列を getBinaryStream
			assertGetBinaryStream(rs, "af_nvarchar256", af_nvarchar256s[i], e);

			// VARCHAR 配列を getBinaryStream
			assertGetBinaryStream(rs, "af_varchar128", af_varchar128s[i], e);

			// NTEXT 配列を getBinaryStream
			assertGetBinaryStream(rs, "af_ntext", af_ntexts[i], e);

			// NTEXT (compressed) 配列を getBinaryStream
			assertGetBinaryStream(rs, "af_ntext_compressed", af_ntext_compresseds[i], e);

			// FULLTEXT 配列を getBinaryStream
			assertGetBinaryStream(rs, "af_fulltext", af_fulltexts[i], e);

			// BINARY 配列を getBinaryStream
			assertGetBinaryStream(rs, "af_binary50", af_binary50s[i], e);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getBoolean(int) のテスト
	public void test_getBoolean1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	e = new ClassCast();

			int	columnIndex = 1;

			// INT (not null) 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_int_not_nulls[i]);			// f_int_not_null

			// INT (primary key) 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_int1s[i]);					// f_int1

			// INT 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_int2s[i]);					// f_int2

			// BIGINT 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_bigints[i]);				// f_bigint
			assertGetBoolean(rs, columnIndex++, f_decimals[i]);				// f_decimal

			// CHAR (not null) 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_char8_not_nulls[i]);			// f_char8_not_null

			// CHAR 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_char8s[i]);					// f_char8

			// FLOAT 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_floats[i]);					// f_float

			// DATETIME 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_datetimes[i], e);				// f_datetime

			// UNIQUEIDENTIFIER 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_ids[i]);						// f_id

			// IMAGE 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_images[i], e);				// f_image

			// LANGUAGE 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_languages[i], e);				// f_language

			// NCHAR 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_nchar6s[i]);					// f_nchar6

			// NVARCHAR 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_nvarchar256s[i]);				// f_nvarchar256

			// VARCHAR 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_varchar128s[i]);				// f_varchar128

			// NTEXT 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_ntexts[i]);					// f_ntext

			// NTEXT (compressed) 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_ntext_compresseds[i]);		// f_ntext_compressed

			// FULLTEXT 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_fulltexts[i]);				// f_fulltext

			// BINARY 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_binary50s[i], e);				// f_binary50

			// BLOB 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_blobs[i], e);					// f_blob

			// NCLOB 列を getBoolean
			assertGetBoolean(rs, columnIndex++, f_nclobs[i]);					// f_nclob

			// INT 配列を getBoolean
			assertGetBoolean(rs, columnIndex++, af_ints[i], e);					// af_int

			// BIGINT 配列を getBoolean
			assertGetBoolean(rs, columnIndex++, af_bigints[i], e);			// af_bigint
			assertGetBoolean(rs, columnIndex++, af_decimals[i], e);			// af_decimal

			// CHAR 配列を getBoolean
			assertGetBoolean(rs, columnIndex++, af_char8s[i], e);				// af_char8

			// FLOAT 配列を getBoolean
			assertGetBoolean(rs, columnIndex++, af_floats[i], e);				// af_float

			// DATETIME 配列を getBoolean
			assertGetBoolean(rs, columnIndex++, af_datetimes[i], e);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getBoolean
			assertGetBoolean(rs, columnIndex++, af_ids[i], e);					// af_id

			// IMAGE 配列を getBoolean
			assertGetBoolean(rs, columnIndex++, af_images[i], e);				// af_image

			// LANGUAGE 配列を getBoolean
			assertGetBoolean(rs, columnIndex++, af_languages[i], e);			// af_language

			// NCHAR 配列を getBoolean
			assertGetBoolean(rs, columnIndex++, af_nchar6s[i], e);				// af_nchar6

			// NVARCHAR 配列を getBoolean
			assertGetBoolean(rs, columnIndex++, af_nvarchar256s[i], e);			// af_nvarchar256

			// VARCHAR 配列を getBoolean
			assertGetBoolean(rs, columnIndex++, af_varchar128s[i], e);			// af_varchar128

			// NTEXT 配列を getBoolean
			assertGetBoolean(rs, columnIndex++, af_ntexts[i], e);				// af_ntext

			// NTEXT (compressed) 配列を getBoolean
			assertGetBoolean(rs, columnIndex++, af_ntext_compresseds[i], e);	// af_ntext_compressed

			// FULLTEXT 配列を getBoolean
			assertGetBoolean(rs, columnIndex++, af_fulltexts[i], e);			// af_fulltext

			// BINARY 配列を getBoolean
			assertGetBoolean(rs, columnIndex++, af_binary50s[i], e);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getBoolean(String) のテスト
	public void test_getBoolean2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	e = new ClassCast();

			// INT (not null) 列を getBoolean
			assertGetBoolean(rs, "f_int_not_null", f_int_not_nulls[i]);

			// INT (primary key) 列を getBoolean
			assertGetBoolean(rs, "f_int1", f_int1s[i]);

			// INT 列を getBoolean
			assertGetBoolean(rs, "f_int2", f_int2s[i]);

			// BIGINT 列を getBoolean
			assertGetBoolean(rs, "f_bigint", f_bigints[i]);
			assertGetBoolean(rs, "f_decimal", f_decimals[i]);

			// CHAR (not null) 列を getBoolean
			assertGetBoolean(rs, "f_char8_not_null", f_char8_not_nulls[i]);

			// CHAR 列を getBoolean
			assertGetBoolean(rs, "f_char8", f_char8s[i]);

			// FLOAT 列を getBoolean
			assertGetBoolean(rs, "f_float", f_floats[i]);

			// DATETIME 列を getBoolean
			assertGetBoolean(rs, "f_datetime", f_datetimes[i], e);

			// UNIQUEIDENTIFIER 列を getBoolean
			assertGetBoolean(rs, "f_id", f_ids[i]);

			// IMAGE 列を getBoolean
			assertGetBoolean(rs, "f_image", f_images[i], e);

			// LANGUAGE 列を getBoolean
			assertGetBoolean(rs, "f_language", f_languages[i], e);

			// NCHAR 列を getBoolean
			assertGetBoolean(rs, "f_nchar6", f_nchar6s[i]);

			// NVARCHAR 列を getBoolean
			assertGetBoolean(rs, "f_nvarchar256", f_nvarchar256s[i]);

			// VARCHAR 列を getBoolean
			assertGetBoolean(rs, "f_varchar128", f_varchar128s[i]);

			// NTEXT 列を getBoolean
			assertGetBoolean(rs, "f_ntext", f_ntexts[i]);

			// NTEXT (compressed) 列を getBoolean
			assertGetBoolean(rs, "f_ntext_compressed", f_ntext_compresseds[i]);

			// FULLTEXT 列を getBoolean
			assertGetBoolean(rs, "f_fulltext", f_fulltexts[i]);

			// BINARY 列を getBoolean
			assertGetBoolean(rs, "f_binary50", f_binary50s[i], e);

			// BLOB 列を getBoolean
			assertGetBoolean(rs, "f_blob", f_blobs[i], e);

			// NCLOB 列を getBoolean
			assertGetBoolean(rs, "f_nclob", f_nclobs[i]);

			// INT 配列を getBoolean
			assertGetBoolean(rs, "af_int", af_ints[i], e);

			// BIGINT 配列を getBoolean
			assertGetBoolean(rs, "af_bigint", af_bigints[i], e);
			assertGetBoolean(rs, "af_decimal", af_decimals[i], e);

			// CHAR 配列を getBoolean
			assertGetBoolean(rs, "af_char8", af_char8s[i], e);

			// FLOAT 配列を getBoolean
			assertGetBoolean(rs, "af_float", af_floats[i], e);

			// DATETIME 配列を getBoolean
			assertGetBoolean(rs, "af_datetime", af_datetimes[i], e);

			// UNIQUEIDENTIFIER 配列を getBoolean
			assertGetBoolean(rs, "af_id", af_ids[i], e);

			// IMAGE 配列を getBoolean
			assertGetBoolean(rs, "af_image", af_images[i], e);

			// LANGUAGE 配列を getBoolean
			assertGetBoolean(rs, "af_language", af_languages[i], e);

			// NCHAR 配列を getBoolean
			assertGetBoolean(rs, "af_nchar6", af_nchar6s[i], e);

			// NVARCHAR 配列を getBoolean
			assertGetBoolean(rs, "af_nvarchar256", af_nvarchar256s[i], e);

			// VARCHAR 配列を getBoolean
			assertGetBoolean(rs, "af_varchar128", af_varchar128s[i], e);

			// NTEXT 配列を getBoolean
			assertGetBoolean(rs, "af_ntext", af_ntexts[i], e);

			// NTEXT (compressed) 配列を getBoolean
			assertGetBoolean(rs, "af_ntext_compressed", af_ntext_compresseds[i], e);

			// FULLTEXT 配列を getBoolean
			assertGetBoolean(rs, "af_fulltext", af_fulltexts[i], e);

			// BINARY 配列を getBoolean
			assertGetBoolean(rs, "af_binary50", af_binary50s[i], e);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getByte(int) のテスト
	public void test_getByte1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			int	columnIndex = 1;

			// INT (not null) 列を getByte
			assertGetByte(rs, columnIndex++, f_int_not_nulls[i]);			// f_int_not_null

			// INT (primary key) 列を getByte
			assertGetByte(rs, columnIndex++, f_int1s[i]);					// f_int1

			// INT 列を getByte
			assertGetByte(rs, columnIndex++, f_int2s[i]);					// f_int2

			// BIGINT 列を getByte
			assertGetByte(rs, columnIndex++, f_bigints[i]);				// f_bigint
			assertGetByte(rs, columnIndex++, f_decimals[i]);				// f_decimal

			// CHAR (not null) 列を getByte
			e = checkParseByte(f_char8_not_nulls[i]);
			assertGetByte(rs, columnIndex++, f_char8_not_nulls[i], e);		// f_char8_not_null

			// CHAR 列を getByte
			e = checkParseByte(f_char8s[i]);
			assertGetByte(rs, columnIndex++, f_char8s[i], e);				// f_char8

			// FLOAT 列を getByte
			assertGetByte(rs, columnIndex++, f_floats[i]);					// f_float

			// DATETIME 列を getByte
			e = classCast;
			assertGetByte(rs, columnIndex++, f_datetimes[i], e);			// f_datetime

			// UNIQUEIDENTIFIER 列を getByte
			assertGetByte(rs, columnIndex++, f_ids[i], e);					// f_id

			// IMAGE 列を getByte
			assertGetByte(rs, columnIndex++, f_images[i], e);				// f_image

			// LANGUAGE 列を getByte
			assertGetByte(rs, columnIndex++, f_languages[i], e);			// f_language

			// NCHAR 列を getByte
			e = checkParseByte(f_nchar6s[i]);
			assertGetByte(rs, columnIndex++, f_nchar6s[i], e);				// f_nchar6

			// NVARCHAR 列を getByte
			e = checkParseByte(f_nvarchar256s[i]);
			assertGetByte(rs, columnIndex++, f_nvarchar256s[i], e);			// f_nvarchar256

			// VARCHAR 列を getByte
			e = checkParseByte(f_varchar128s[i]);
			assertGetByte(rs, columnIndex++, f_varchar128s[i], e);			// f_varchar128

			// NTEXT 列を getByte
			e = checkParseByte(f_ntexts[i]);
			assertGetByte(rs, columnIndex++, f_ntexts[i], e);				// f_ntext

			// NTEXT (compressed) 列を getByte
			e = checkParseByte(f_ntext_compresseds[i]);
			assertGetByte(rs, columnIndex++, f_ntext_compresseds[i], e);	// f_ntext_compressed

			// FULLTEXT 列を getByte
			e = checkParseByte(f_fulltexts[i]);
			assertGetByte(rs, columnIndex++, f_fulltexts[i], e);			// f_fulltext

			// BINARY 列を getByte
			e = classCast;
			assertGetByte(rs, columnIndex++, f_binary50s[i], e);			// f_binary50

			// BLOB 列を getByte
			assertGetByte(rs, columnIndex++, f_blobs[i], e);				// f_blob

			// NCLOB 列を getByte
			e = checkParseByte(f_nclobs[i]);
			assertGetByte(rs, columnIndex++, f_nclobs[i], e);				// f_nclob

			// INT 配列を getByte
			e = classCast;
			assertGetByte(rs, columnIndex++, af_ints[i], e);				// af_int

			// BIGINT 配列を getByte
			assertGetByte(rs, columnIndex++, af_bigints[i], e);			// af_bigint
			assertGetByte(rs, columnIndex++, af_decimals[i], e);			// af_decimal

			// CHAR 配列を getByte
			assertGetByte(rs, columnIndex++, af_char8s[i], e);				// af_char8

			// FLOAT 配列を getByte
			assertGetByte(rs, columnIndex++, af_floats[i], e);				// af_float

			// DATETIME 配列を getByte
			assertGetByte(rs, columnIndex++, af_datetimes[i], e);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getByte
			assertGetByte(rs, columnIndex++, af_ids[i], e);					// af_id

			// IMAGE 配列を getByte
			assertGetByte(rs, columnIndex++, af_images[i], e);				// af_image

			// LANGUAGE 配列を getByte
			assertGetByte(rs, columnIndex++, af_languages[i], e);			// af_language

			// NCHAR 配列を getByte
			assertGetByte(rs, columnIndex++, af_nchar6s[i], e);				// af_nchar6

			// NVARCHAR 配列を getByte
			assertGetByte(rs, columnIndex++, af_nvarchar256s[i], e);		// af_nvarchar256

			// VARCHAR 配列を getByte
			assertGetByte(rs, columnIndex++, af_varchar128s[i], e);			// af_varchar128

			// NTEXT 配列を getByte
			assertGetByte(rs, columnIndex++, af_ntexts[i], e);				// af_ntext

			// NTEXT (compressed) 配列を getByte
			assertGetByte(rs, columnIndex++, af_ntext_compresseds[i], e);	// af_ntext_compressed

			// FULLTEXT 配列を getByte
			assertGetByte(rs, columnIndex++, af_fulltexts[i], e);			// af_fulltext

			// BINARY 配列を getByte
			assertGetByte(rs, columnIndex++, af_binary50s[i], e);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getByte(String) のテスト
	public void test_getByte2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			// INT (not null) 列を getByte
			assertGetByte(rs, "f_int_not_null", f_int_not_nulls[i]);

			// INT (primary key) 列を getByte
			assertGetByte(rs, "f_int1", f_int1s[i]);

			// INT 列を getByte
			assertGetByte(rs, "f_int2", f_int2s[i]);

			// BIGINT 列を getByte
			assertGetByte(rs, "f_bigint", f_bigints[i]);
			assertGetByte(rs, "f_decimal", f_decimals[i]);

			// CHAR (not null) 列を getByte
			e = checkParseByte(f_char8_not_nulls[i]);
			assertGetByte(rs, "f_char8_not_null", f_char8_not_nulls[i], e);

			// CHAR 列を getByte
			e = checkParseByte(f_char8s[i]);
			assertGetByte(rs, "f_char8", f_char8s[i], e);

			// FLOAT 列を getByte
			assertGetByte(rs, "f_float", f_floats[i]);

			// DATETIME 列を getByte
			e = classCast;
			assertGetByte(rs, "f_datetime", f_datetimes[i], e);

			// UNIQUEIDENTIFIER 列を getByte
			assertGetByte(rs, "f_id", f_ids[i], e);

			// IMAGE 列を getByte
			assertGetByte(rs, "f_image", f_images[i], e);

			// LANGUAGE 列を getByte
			assertGetByte(rs, "f_language", f_languages[i], e);

			// NCHAR 列を getByte
			e = checkParseByte(f_nchar6s[i]);
			assertGetByte(rs, "f_nchar6", f_nchar6s[i], e);

			// NVARCHAR 列を getByte
			e = checkParseByte(f_nvarchar256s[i]);
			assertGetByte(rs, "f_nvarchar256", f_nvarchar256s[i], e);

			// VARCHAR 列を getByte
			e = checkParseByte(f_varchar128s[i]);
			assertGetByte(rs, "f_varchar128", f_varchar128s[i], e);

			// NTEXT 列を getByte
			e = checkParseByte(f_ntexts[i]);
			assertGetByte(rs, "f_ntext", f_ntexts[i], e);

			// NTEXT (compressed) 列を getByte
			e = checkParseByte(f_ntext_compresseds[i]);
			assertGetByte(rs, "f_ntext_compressed", f_ntext_compresseds[i], e);

			// FULLTEXT 列を getByte
			e = checkParseByte(f_fulltexts[i]);
			assertGetByte(rs, "f_fulltext", f_fulltexts[i], e);

			// BINARY 列を getByte
			e = classCast;
			assertGetByte(rs, "f_binary50", f_binary50s[i], e);

			// BLOB 列を getByte
			assertGetByte(rs, "f_blob", f_blobs[i], e);

			// NCLOB 列を getByte
			e = checkParseByte(f_nclobs[i]);
			assertGetByte(rs, "f_nclob", f_nclobs[i], e);

			// INT 配列を getByte
			e = classCast;
			assertGetByte(rs, "af_int", af_ints[i], e);

			// BIGINT 配列を getByte
			assertGetByte(rs, "af_bigint", af_bigints[i], e);
			assertGetByte(rs, "af_decimal", af_decimals[i], e);

			// CHAR 配列を getByte
			assertGetByte(rs, "af_char8", af_char8s[i], e);

			// FLOAT 配列を getByte
			assertGetByte(rs, "af_float", af_floats[i], e);

			// DATETIME 配列を getByte
			assertGetByte(rs, "af_datetime", af_datetimes[i], e);

			// UNIQUEIDENTIFIER 配列を getByte
			assertGetByte(rs, "af_id", af_ids[i], e);

			// IMAGE 配列を getByte
			assertGetByte(rs, "af_image", af_images[i], e);

			// LANGUAGE 配列を getByte
			assertGetByte(rs, "af_language", af_languages[i], e);

			// NCHAR 配列を getByte
			assertGetByte(rs, "af_nchar6", af_nchar6s[i], e);

			// NVARCHAR 配列を getByte
			assertGetByte(rs, "af_nvarchar256", af_nvarchar256s[i], e);

			// VARCHAR 配列を getByte
			assertGetByte(rs, "af_varchar128", af_varchar128s[i], e);

			// NTEXT 配列を getByte
			assertGetByte(rs, "af_ntext", af_ntexts[i], e);

			// NTEXT (compressed) 配列を getByte
			assertGetByte(rs, "af_ntext_compressed", af_ntext_compresseds[i], e);

			// FULLTEXT 配列を getByte
			assertGetByte(rs, "af_fulltext", af_fulltexts[i], e);

			// BINARY 配列を getByte
			assertGetByte(rs, "af_binary50", af_binary50s[i], e);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getBytes(int) のテスト
	public void test_getBytes1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	e = new ClassCast();

			int	columnIndex = 1;

			// INT (not null) 列を getBytes
			assertGetBytes(rs, columnIndex++, f_int_not_nulls[i], e);		// f_int_not_null

			// INT (primary key) 列を getBytes
			assertGetBytes(rs, columnIndex++,f_int1s[i], e);				// f_int1

			// INT 列を getBytes
			assertGetBytes(rs, columnIndex++, f_int2s[i], e);				// f_int2

			// BIGINT 列を getBytes
			assertGetBytes(rs, columnIndex++, f_bigints[i], e);			// f_bigint
			assertGetBytes(rs, columnIndex++, f_decimals[i], e);			// f_decimal

			// CHAR (not null) 列を getBytes
			assertGetBytes(rs, columnIndex++, f_char8_not_nulls[i], e);		// f_char8_not_null

			// CHAR 列を getBytes
			assertGetBytes(rs, columnIndex++, f_char8s[i], e);				// f_char8

			// FLOAT 列を getBytes
			assertGetBytes(rs, columnIndex++, f_floats[i], e);				// f_float

			// DATETIME 列を getBytes
			assertGetBytes(rs, columnIndex++, f_datetimes[i], e);			// f_datetime

			// UNIQUEIDENTIFIER 列を getBytes
			assertGetBytes(rs, columnIndex++, f_ids[i], e);					// f_id

			// IMAGE 列を getBytes
			assertGetBytes(rs, columnIndex++, f_images[i]);					// f_image

			// LANGUAGE 列を getBytes
			assertGetBytes(rs, columnIndex++, f_languages[i], e);			// f_language

			// NCHAR 列を getBytes
			assertGetBytes(rs, columnIndex++, f_nchar6s[i], e);				// f_nchar6

			// NVARCHAR 列を getBytes
			assertGetBytes(rs, columnIndex++, f_nvarchar256s[i], e);		// f_nvarchar256

			// VARCHAR 列を getBytes
			assertGetBytes(rs, columnIndex++, f_varchar128s[i], e);			// f_varchar128

			// NTEXT 列を getBytes
			assertGetBytes(rs, columnIndex++, f_ntexts[i], e);				// f_ntext

			// NTEXT (compressed) 列を getBytes
			assertGetBytes(rs, columnIndex++, f_ntext_compresseds[i], e);	// f_ntext_compressed

			// FULLTEXT 列を getBytes
			assertGetBytes(rs, columnIndex++, f_fulltexts[i], e);			// f_fulltext

			// BINARY 列を getBytes
			assertGetBytes(rs, columnIndex++, f_binary50s[i]);				// f_binary50

			// BLOB 列を getBytes
			assertGetBytes(rs, columnIndex++, f_blobs[i]);					// f_blob

			// NCLOB 列を getBytes
			assertGetBytes(rs, columnIndex++, f_nclobs[i], e);				// f_nclob

			// INT 配列を getBytes
			assertGetBytes(rs, columnIndex++, af_ints[i], e);				// af_int

			// BIGINT 配列を getBytes
			assertGetBytes(rs, columnIndex++, af_bigints[i], e);		// af_bigint
			assertGetBytes(rs, columnIndex++, af_decimals[i], e);		// af_decimal

			// CHAR 配列を getBytes
			assertGetBytes(rs, columnIndex++, af_char8s[i], e);				// af_char8

			// FLOAT 配列を getBytes
			assertGetBytes(rs, columnIndex++, af_floats[i], e);				// af_float

			// DATETIME 配列を getBytes
			assertGetBytes(rs, columnIndex++, af_datetimes[i], e);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getBytes
			assertGetBytes(rs, columnIndex++, af_ids[i], e);				// af_id

			// IMAGE 配列を getBytes
			assertGetBytes(rs, columnIndex++, af_images[i], e);				// af_image

			// LANGUAGE 配列を getBytes
			assertGetBytes(rs, columnIndex++, af_languages[i], e);			// af_language

			// NCHAR 配列を getBytes
			assertGetBytes(rs, columnIndex++, af_nchar6s[i], e);			// af_nchar6

			// NVARCHAR 配列を getBytes
			assertGetBytes(rs, columnIndex++, af_nvarchar256s[i], e);		// af_nvarchar256

			// VARCHAR 配列を getBytes
			assertGetBytes(rs, columnIndex++, af_varchar128s[i], e);		// af_varchar128

			// NTEXT 配列を getBytes
			assertGetBytes(rs, columnIndex++, af_ntexts[i], e);				// af_ntext

			// NTEXT (compressed) 配列を getBytes
			assertGetBytes(rs, columnIndex++, af_ntext_compresseds[i], e);	// af_ntext_compressed

			// FULLTEXT 配列を getBytes
			assertGetBytes(rs, columnIndex++, af_fulltexts[i], e);			// af_fulltext

			// BINARY 配列を getBytes
			assertGetBytes(rs, columnIndex++, af_binary50s[i], e);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getBytes(String) のテスト
	public void test_getBytes2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	e = new ClassCast();

			// INT (not null) 列を getBytes
			assertGetBytes(rs, "f_int_not_null", f_int_not_nulls[i], e);

			// INT (primary key) 列を getBytes
			assertGetBytes(rs, "f_int1", f_int1s[i], e);

			// INT 列を getBytes
			assertGetBytes(rs, "f_int2", f_int2s[i], e);

			// BIGINT 列を getBytes
			assertGetBytes(rs, "f_bigint", f_bigints[i], e);
			assertGetBytes(rs, "f_decimal", f_decimals[i], e);

			// CHAR (not null) 列を getBytes
			assertGetBytes(rs, "f_char8_not_null", f_char8_not_nulls[i], e);

			// CHAR 列を getBytes
			assertGetBytes(rs, "f_char8", f_char8s[i], e);

			// FLOAT 列を getBytes
			assertGetBytes(rs, "f_float", f_floats[i], e);

			// DATETIME 列を getBytes
			assertGetBytes(rs, "f_datetime", f_datetimes[i], e);

			// UNIQUEIDENTIFIER 列を getBytes
			assertGetBytes(rs, "f_id", f_ids[i], e);

			// IMAGE 列を getBytes
			assertGetBytes(rs, "f_image", f_images[i]);

			// LANGUAGE 列を getBytes
			assertGetBytes(rs, "f_language", f_languages[i], e);

			// NCHAR 列を getBytes
			assertGetBytes(rs, "f_nchar6", f_nchar6s[i], e);

			// NVARCHAR 列を getBytes
			assertGetBytes(rs, "f_nvarchar256", f_nvarchar256s[i], e);

			// VARCHAR 列を getBytes
			assertGetBytes(rs, "f_varchar128", f_varchar128s[i], e);

			// NTEXT 列を getBytes
			assertGetBytes(rs, "f_ntext", f_ntexts[i], e);

			// NTEXT (compressed) 列を getBytes
			assertGetBytes(rs, "f_ntext_compressed", f_ntext_compresseds[i], e);

			// FULLTEXT 列を getBytes
			assertGetBytes(rs, "f_fulltext", f_fulltexts[i], e);

			// BINARY 列を getBytes
			assertGetBytes(rs, "f_binary50", f_binary50s[i]);

			// BLOB 列を getBytes
			assertGetBytes(rs, "f_blob", f_blobs[i]);

			// NCLOB 列を getBytes
			assertGetBytes(rs, "f_nclob", f_nclobs[i], e);

			// INT 配列を getBytes
			assertGetBytes(rs, "af_int", af_ints[i], e);

			// BIGINT 配列を getBytes
			assertGetBytes(rs, "af_bigint", af_bigints[i], e);
			assertGetBytes(rs, "af_decimal", af_decimals[i], e);

			// CHAR 配列を getBytes
			assertGetBytes(rs, "af_char8", af_char8s[i], e);

			// FLOAT 配列を getBytes
			assertGetBytes(rs, "af_float", af_floats[i], e);

			// DATETIME 配列を getBytes
			assertGetBytes(rs, "af_datetime", af_datetimes[i], e);

			// UNIQUEIDENTIFIER 配列を getBytes
			assertGetBytes(rs, "af_id", af_ids[i], e);

			// IMAGE 配列を getBytes
			assertGetBytes(rs, "af_image", af_images[i], e);

			// LANGUAGE 配列を getBytes
			assertGetBytes(rs, "af_language", af_languages[i], e);

			// NCHAR 配列を getBytes
			assertGetBytes(rs, "af_nchar6", af_nchar6s[i], e);

			// NVARCHAR 配列を getBytes
			assertGetBytes(rs, "af_nvarchar256", af_nvarchar256s[i], e);

			// VARCHAR 配列を getBytes
			assertGetBytes(rs, "af_varchar128", af_varchar128s[i], e);

			// NTEXT 配列を getBytes
			assertGetBytes(rs, "af_ntext", af_ntexts[i], e);

			// NTEXT (compressed) 配列を getBytes
			assertGetBytes(rs, "af_ntext_compressed", af_ntext_compresseds[i], e);

			// FULLTEXT 配列を getBytes
			assertGetBytes(rs, "af_fulltext", af_fulltexts[i], e);

			// BINARY 配列を getBytes
			assertGetBytes(rs, "af_binary50", af_binary50s[i], e);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getCharacterStream(int) のテスト
	public void test_getCharacterStream1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	e = new ClassCast();

			int	columnIndex = 1;

			// INT (not null) 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_int_not_nulls[i], e);			// f_int_not_null

			// INT (primary key) 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_int1s[i], e);					// f_int1

			// INT 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_int2s[i], e);					// f_int2

			// BIGINT 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_bigints[i], e);			// f_bigint
			assertGetCharacterStream(rs, columnIndex++, f_decimals[i], e);			// f_decimal

			// CHAR (not null) 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_char8_not_nulls[i]);			// f_char8_not_null

			// CHAR 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_char8s[i]);					// f_char8

			// FLOAT 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_floats[i], e);				// f_float

			// DATETIME 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_datetimes[i], e);				// f_datetime

			// UNIQUEIDENTIFIER 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_ids[i]);						// f_id

			// IMAGE 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_images[i]);					// f_image

			// LANGUAGE 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_languages[i], e);				// f_language

			// NCHAR 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_nchar6s[i]);					// f_nchar6

			// NVARCHAR 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_nvarchar256s[i]);				// f_nvarchar256

			// VARCHAR 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_varchar128s[i]);				// f_varchar128

			// NTEXT 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_ntexts[i]);					// f_ntext

			// NTEXT (compressed) 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_ntext_compresseds[i]);		// f_ntext_compressed

			// FULLTEXT 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_fulltexts[i]);				// f_fulltext

			// BINARY 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_binary50s[i]);				// f_binary50

			// BLOB 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_blobs[i]);					// f_blob

			// NCLOB 列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, f_nclobs[i]);					// f_nclob

			// INT 配列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, af_ints[i], e);					// af_int

			// BIGINT 配列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, af_bigints[i], e);			// af_bigint
			assertGetCharacterStream(rs, columnIndex++, af_decimals[i], e);			// af_decimal

			// CHAR 配列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, af_char8s[i], e);				// af_char8

			// FLOAT 配列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, af_floats[i], e);				// af_float

			// DATETIME 配列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, af_datetimes[i], e);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, af_ids[i], e);					// af_id

			// IMAGE 配列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, af_images[i], e);				// af_image

			// LANGUAGE 配列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, af_languages[i], e);			// af_language

			// NCHAR 配列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, af_nchar6s[i], e);				// af_nchar6

			// NVARCHAR 配列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, af_nvarchar256s[i], e);			// af_nvarchar256

			// VARCHAR 配列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, af_varchar128s[i], e);			// af_varchar128

			// NTEXT 配列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, af_ntexts[i], e);				// af_ntext

			// NTEXT (compressed) 配列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, af_ntext_compresseds[i], e);	// af_ntext_compressed

			// FULLTEXT 配列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, af_fulltexts[i], e);			// af_fulltext

			// BINARY 配列を getCharacterStream
			assertGetCharacterStream(rs, columnIndex++, af_binary50s[i], e);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getCharacterStream(String) のテスト
	public void test_getCharacterStream2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	e = new ClassCast();

			// INT (not null) 列を getCharacterStream
			assertGetCharacterStream(rs, "f_int_not_null", f_int_not_nulls[i], e);

			// INT (primary key) 列を getCharacterStream
			assertGetCharacterStream(rs, "f_int1", f_int1s[i], e);

			// INT 列を getCharacterStream
			assertGetCharacterStream(rs, "f_int2", f_int2s[i], e);

			// BIGINT 列を getCharacterStream
			assertGetCharacterStream(rs, "f_bigint", f_bigints[i], e);
			assertGetCharacterStream(rs, "f_decimal", f_decimals[i], e);

			// CHAR (not null) 列を getCharacterStream
			assertGetCharacterStream(rs, "f_char8_not_null", f_char8_not_nulls[i]);

			// CHAR 列を getCharacterStream
			assertGetCharacterStream(rs, "f_char8", f_char8s[i]);

			// FLOAT 列を getCharacterStream
			assertGetCharacterStream(rs, "f_float", f_floats[i], e);

			// DATETIME 列を getCharacterStream
			assertGetCharacterStream(rs, "f_datetime", f_datetimes[i], e);

			// UNIQUEIDENTIFIER 列を getCharacterStream
			assertGetCharacterStream(rs, "f_id", f_ids[i]);

			// IMAGE 列を getCharacterStream
			assertGetCharacterStream(rs, "f_image", f_images[i]);

			// LANGUAGE 列を getCharacterStream
			assertGetCharacterStream(rs, "f_language", f_languages[i], e);

			// NCHAR 列を getCharacterStream
			assertGetCharacterStream(rs, "f_nchar6", f_nchar6s[i]);

			// NVARCHAR 列を getCharacterStream
			assertGetCharacterStream(rs, "f_nvarchar256", f_nvarchar256s[i]);

			// VARCHAR 列を getCharacterStream
			assertGetCharacterStream(rs, "f_varchar128", f_varchar128s[i]);

			// NTEXT 列を getCharacterStream
			assertGetCharacterStream(rs, "f_ntext", f_ntexts[i]);

			// NTEXT (compressed) 列を getCharacterStream
			assertGetCharacterStream(rs, "f_ntext_compressed", f_ntext_compresseds[i]);

			// FULLTEXT 列を getCharacterStream
			assertGetCharacterStream(rs, "f_fulltext", f_fulltexts[i]);

			// BINARY 列を getCharacterStream
			assertGetCharacterStream(rs, "f_binary50", f_binary50s[i]);

			// BLOB 列を getCharacterStream
			assertGetCharacterStream(rs, "f_blob", f_blobs[i]);

			// NCLOB 列を getCharacterStream
			assertGetCharacterStream(rs, "f_nclob", f_nclobs[i]);

			// INT 配列を getCharacterStream
			assertGetCharacterStream(rs, "af_int", af_ints[i], e);

			// BIGINT 配列を getCharacterStream
			assertGetCharacterStream(rs, "af_bigint", af_bigints[i], e);
			assertGetCharacterStream(rs, "af_decimal", af_decimals[i], e);

			// CHAR 配列を getCharacterStream
			assertGetCharacterStream(rs, "af_char8", af_char8s[i], e);

			// FLOAT 配列を getCharacterStream
			assertGetCharacterStream(rs, "af_float", af_floats[i], e);

			// DATETIME 配列を getCharacterStream
			assertGetCharacterStream(rs, "af_datetime", af_datetimes[i], e);

			// UNIQUEIDENTIFIER 配列を getCharacterStream
			assertGetCharacterStream(rs, "af_id", af_ids[i], e);

			// IMAGE 配列を getCharacterStream
			assertGetCharacterStream(rs, "af_image", af_images[i], e);

			// LANGUAGE 配列を getCharacterStream
			assertGetCharacterStream(rs, "af_language", af_languages[i], e);

			// NCHAR 配列を getCharacterStream
			assertGetCharacterStream(rs, "af_nchar6", af_nchar6s[i], e);

			// NVARCHAR 配列を getCharacterStream
			assertGetCharacterStream(rs, "af_nvarchar256", af_nvarchar256s[i], e);

			// VARCHAR 配列を getCharacterStream
			assertGetCharacterStream(rs, "af_varchar128", af_varchar128s[i], e);

			// NTEXT 配列を getCharacterStream
			assertGetCharacterStream(rs, "af_ntext", af_ntexts[i], e);

			// NTEXT (compressed) 配列を getCharacterStream
			assertGetCharacterStream(rs, "af_ntext_compressed", af_ntext_compresseds[i], e);

			// FULLTEXT 配列を getCharacterStream
			assertGetCharacterStream(rs, "af_fulltext", af_fulltexts[i], e);

			// BINARY 配列を getCharacterStream
			assertGetCharacterStream(rs, "af_binary50", af_binary50s[i], e);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getConcurrency() のテスト
	public void test_getConcurrency() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c, 1);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		// 常に java.lsq.ResultSet.CONCUR_READ_ONLY が返るはず
		assertEquals(ResultSet.CONCUR_READ_ONLY, rs.getConcurrency());

		// next しても同じはず
		assertTrue(rs.next());
		assertEquals(ResultSet.CONCUR_READ_ONLY, rs.getConcurrency());

		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getCursorName() のテスト
	public void test_getCursorName() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c, 1);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		// カーソルはサポートしていないので常に null が返るはず
		assertNull(rs.getCursorName());

		// 警告は発生しないはず
		assertNull(rs.getWarnings());

		// next しても同じはず
		assertTrue(rs.next());
		assertNull(rs.getCursorName());

		// 警告は発生しないはず
		assertNull(rs.getWarnings());

		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getDate(int) のテスト
	public void test_getDate1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			int	columnIndex = 1;

			// INT (not null) 列を getDate
			assertGetDate(rs, columnIndex++, f_int_not_nulls[i], e);		// f_int_not_null

			// INT (primary key) 列を getDate
			assertGetDate(rs, columnIndex++, f_int1s[i], e);				// f_int1

			// INT 列を getDate
			assertGetDate(rs, columnIndex++, f_int2s[i], e);				// f_int2

			// BIGINT 列を getDate
			assertGetDate(rs, columnIndex++, f_bigints[i], e);			// f_bigint
			assertGetDate(rs, columnIndex++, f_decimals[i], e);			// f_decimal

			// CHAR (not null) 列を getDate
			e = checkParseDate(f_char8_not_nulls[i]);
			assertGetDate(rs, columnIndex++, f_char8_not_nulls[i], e);		// f_char8_not_null

			// CHAR 列を getDate
			e = checkParseDate(f_char8s[i]);
			assertGetDate(rs, columnIndex++, f_char8s[i], e);				// f_char8

			// FLOAT 列を getDate
			e = classCast;
			assertGetDate(rs, columnIndex++, f_floats[i], e);				// f_float

			// DATETIME 列を getDate
			assertGetDate(rs, columnIndex++, f_datetimes[i]);				// f_datetime

			// UNIQUEIDENTIFIER 列を getDate
			assertGetDate(rs, columnIndex++, f_ids[i], e);					// f_id

			// IMAGE 列を getDate
			assertGetDate(rs, columnIndex++, f_images[i], e);				// f_image

			// LANGUAGE 列を getDate
			assertGetDate(rs, columnIndex++, f_languages[i], e);			// f_language

			// NCHAR 列を getDate
			assertGetDate(rs, columnIndex++, f_nchar6s[i], e);				// f_nchar6

			// NVARCHAR 列を getDate
			e = checkParseDate(f_nvarchar256s[i]);
			assertGetDate(rs, columnIndex++, f_nvarchar256s[i], e);			// f_nvarchar256

			// VARCHAR 列を getDate
			e = checkParseDate(f_varchar128s[i]);
			assertGetDate(rs, columnIndex++, f_varchar128s[i], e);			// f_varchar128

			// NTEXT 列を getDate
			e = checkParseDate(f_ntexts[i]);
			assertGetDate(rs, columnIndex++, f_ntexts[i], e);				// f_ntext

			// NTEXT (compressed) 列を getDate
			e = checkParseDate(f_ntext_compresseds[i]);
			assertGetDate(rs, columnIndex++, f_ntext_compresseds[i], e);	// f_ntext_compressed

			// FULLTEXT 列を getDate
			e = checkParseDate(f_fulltexts[i]);
			assertGetDate(rs, columnIndex++, f_fulltexts[i], e);			// f_fulltext

			// BINARY 列を getDate
			e = classCast;
			assertGetDate(rs, columnIndex++, f_binary50s[i], e);			// f_binary50

			// BLOB 列を getDate
			assertGetDate(rs, columnIndex++, f_blobs[i], e);				// f_blob

			// NCLOB 列を getDate
			e = checkParseDate(f_nclobs[i]);
			assertGetDate(rs, columnIndex++, f_nclobs[i], e);				// f_nclob

			// INT 配列を getDate
			e = classCast;
			assertGetDate(rs, columnIndex++, af_ints[i], e);				// af_int

			// BIGINT 配列を getDate
			assertGetDate(rs, columnIndex++, af_bigints[i], e);			// af_bigint
			assertGetDate(rs, columnIndex++, af_decimals[i], e);			// af_decimal

			// CHAR 配列を getDate
			assertGetDate(rs, columnIndex++, af_char8s[i], e);				// af_char8

			// FLOAT 配列を getDate
			assertGetDate(rs, columnIndex++, af_floats[i], e);				// af_float

			// DATETIME 配列を getDate
			assertGetDate(rs, columnIndex++, af_datetimes[i], e);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getDate
			assertGetDate(rs, columnIndex++, af_ids[i], e);					// af_id

			// IMAGE 配列を getDate
			assertGetDate(rs, columnIndex++, af_images[i], e);				// af_image

			// LANGUAGE 配列を getDate
			assertGetDate(rs, columnIndex++, af_languages[i], e);			// af_language

			// NCHAR 配列を getDate
			assertGetDate(rs, columnIndex++, af_nchar6s[i], e);				// af_nchar6

			// NVARCHAR 配列を getDate
			assertGetDate(rs, columnIndex++, af_nvarchar256s[i], e);		// af_nvarchar256

			// VARCHAR 配列を getDate
			assertGetDate(rs, columnIndex++, af_varchar128s[i], e);			// af_varchar128

			// NTEXT 配列を getDate
			assertGetDate(rs, columnIndex++, af_ntexts[i], e);				// af_ntext

			// NTEXT (compressed) 配列を getDate
			assertGetDate(rs, columnIndex++, af_ntext_compresseds[i], e);	// af_ntext_compressed

			// FULLTEXT 配列を getDate
			assertGetDate(rs, columnIndex++, af_fulltexts[i], e);			// af_fulltext

			// BINARY 配列を getDate
			assertGetDate(rs, columnIndex++, af_binary50s[i], e);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getDate(String) のテスト
	public void test_getDate2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			// INT (not null) 列を getDate
			assertGetDate(rs, "f_int_not_null", f_int_not_nulls[i], e);

			// INT (primary key) 列を getDate
			assertGetDate(rs, "f_int1", f_int1s[i], e);

			// INT 列を getDate
			assertGetDate(rs, "f_int2", f_int2s[i], e);

			// BIGINT 列を getDate
			assertGetDate(rs, "f_bigint", f_bigints[i], e);
			assertGetDate(rs, "f_decimal", f_decimals[i], e);

			// CHAR (not null) 列を getDate
			e = checkParseDate(f_char8_not_nulls[i]);
			assertGetDate(rs, "f_char8_not_null", f_char8_not_nulls[i], e);

			// CHAR 列を getDate
			e = checkParseDate(f_char8s[i]);
			assertGetDate(rs, "f_char8", f_char8s[i], e);

			// FLOAT 列を getDate
			e = classCast;
			assertGetDate(rs, "f_float", f_floats[i], e);

			// DATETIME 列を getDate
			assertGetDate(rs, "f_datetime", f_datetimes[i]);

			// UNIQUEIDENTIFIER 列を getDate
			assertGetDate(rs, "f_id", f_ids[i], e);

			// IMAGE 列を getDate
			assertGetDate(rs, "f_image", f_images[i], e);

			// LANGUAGE 列を getDate
			assertGetDate(rs, "f_language", f_languages[i], e);

			// NCHAR 列を getDate
			assertGetDate(rs, "f_nchar6", f_nchar6s[i], e);

			// NVARCHAR 列を getDate
			e = checkParseDate(f_nvarchar256s[i]);
			assertGetDate(rs, "f_nvarchar256", f_nvarchar256s[i], e);

			// VARCHAR 列を getDate
			e = checkParseDate(f_varchar128s[i]);
			assertGetDate(rs, "f_varchar128", f_varchar128s[i], e);

			// NTEXT 列を getDate
			e = checkParseDate(f_ntexts[i]);
			assertGetDate(rs, "f_ntext", f_ntexts[i], e);

			// NTEXT (compressed) 列を getDate
			e = checkParseDate(f_ntext_compresseds[i]);
			assertGetDate(rs, "f_ntext_compressed", f_ntext_compresseds[i], e);

			// FULLTEXT 列を getDate
			e = checkParseDate(f_fulltexts[i]);
			assertGetDate(rs, "f_fulltext", f_fulltexts[i], e);

			// BINARY 列を getDate
			e = classCast;
			assertGetDate(rs, "f_binary50", f_binary50s[i], e);

			// BLOB 列を getDate
			assertGetDate(rs, "f_blob", f_blobs[i], e);

			// NCLOB 列を getDate
			e = checkParseDate(f_nclobs[i]);
			assertGetDate(rs, "f_nclob", f_nclobs[i], e);

			// INT 配列を getDate
			e = classCast;
			assertGetDate(rs, "af_int", af_ints[i], e);

			// BIGINT 配列を getDate
			assertGetDate(rs, "af_bigint", af_bigints[i], e);
			assertGetDate(rs, "af_decimal", af_decimals[i], e);

			// CHAR 配列を getDate
			assertGetDate(rs, "af_char8", af_char8s[i], e);

			// FLOAT 配列を getDate
			assertGetDate(rs, "af_float", af_floats[i], e);

			// DATETIME 配列を getDate
			assertGetDate(rs, "af_datetime", af_datetimes[i], e);

			// UNIQUEIDENTIFIER 配列を getDate
			assertGetDate(rs, "af_id", af_ids[i], e);

			// IMAGE 配列を getDate
			assertGetDate(rs, "af_image", af_images[i], e);

			// LANGUAGE 配列を getDate
			assertGetDate(rs, "af_language", af_languages[i], e);

			// NCHAR 配列を getDate
			assertGetDate(rs, "af_nchar6", af_nchar6s[i], e);

			// NVARCHAR 配列を getDate
			assertGetDate(rs, "af_nvarchar256", af_nvarchar256s[i], e);

			// VARCHAR 配列を getDate
			assertGetDate(rs, "af_varchar128", af_varchar128s[i], e);

			// NTEXT 配列を getDate
			assertGetDate(rs, "af_ntext", af_ntexts[i], e);

			// NTEXT (compressed) 配列を getDate
			assertGetDate(rs, "af_ntext_compressed", af_ntext_compresseds[i], e);

			// FULLTEXT 配列を getDate
			assertGetDate(rs, "af_fulltext", af_fulltexts[i], e);

			// BINARY 配列を getDate
			assertGetDate(rs, "af_binary50", af_binary50s[i], e);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getDouble(int) のテスト
	public void test_getDouble1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			int	columnIndex = 1;

			// INT (not null) 列を getDouble
			assertGetDouble(rs, columnIndex++, f_int_not_nulls[i]);			// f_int_not_null

			// INT (primary key) 列を getDouble
			assertGetDouble(rs, columnIndex++, f_int1s[i]);					// f_int1

			// INT 列を getDouble
			assertGetDouble(rs, columnIndex++, f_int2s[i]);					// f_int2

			// BIGINT 列を getDouble
			assertGetDouble(rs, columnIndex++, f_bigints[i]);			// f_bigint
			assertGetDouble(rs, columnIndex++, f_decimals[i]);			// f_decimal

			// CHAR (not null) 列を getDouble
			e = checkParseDouble(f_char8_not_nulls[i]);
			assertGetDouble(rs, columnIndex++, f_char8_not_nulls[i], e);	// f_char8_not_null

			// CHAR 列を getDouble
			e = checkParseDouble(f_char8s[i]);
			assertGetDouble(rs, columnIndex++, f_char8s[i], e);				// f_char8

			// FLOAT 列を getDouble
			assertGetDouble(rs, columnIndex++, f_floats[i]);				// f_float

			// DATETIME 列を getDouble
			e = classCast;
			assertGetDouble(rs, columnIndex++, f_datetimes[i], e);			// f_datetime

			// UNIQUEIDENTIFIER 列を getDouble
			assertGetDouble(rs, columnIndex++, f_ids[i], e);				// f_id

			// IMAGE 列を getDouble
			assertGetDouble(rs, columnIndex++, f_images[i], e);				// f_image

			// LANGUAGE 列を getDouble
			assertGetDouble(rs, columnIndex++, f_languages[i], e);			// f_language

			// NCHAR 列を getDouble
			e = checkParseDouble(f_nchar6s[i]);
			assertGetDouble(rs, columnIndex++, f_nchar6s[i], e);			// f_nchar6

			// NVARCHAR 列を getDouble
			e = checkParseDouble(f_nvarchar256s[i]);
			assertGetDouble(rs, columnIndex++, f_nvarchar256s[i], e);		// f_nvarchar256

			// VARCHAR 列を getDouble
			e = checkParseDouble(f_varchar128s[i]);
			assertGetDouble(rs, columnIndex++, f_varchar128s[i], e);		// f_varchar128

			// NTEXT 列を getDouble
			e = checkParseDouble(f_ntexts[i]);
			assertGetDouble(rs, columnIndex++, f_ntexts[i], e);				// f_ntext

			// NTEXT (compressed) 列を getDouble
			e = checkParseDouble(f_ntext_compresseds[i]);
			assertGetDouble(rs, columnIndex++, f_ntext_compresseds[i], e);	// f_ntext_compressed

			// FULLTEXT 列を getDouble
			e = checkParseDouble(f_fulltexts[i]);
			assertGetDouble(rs, columnIndex++, f_fulltexts[i], e);			// f_fulltext

			// BINARY 列を getDouble
			e = classCast;
			assertGetDouble(rs, columnIndex++, f_binary50s[i], e);			// f_binary50

			// BLOB 列を getDouble
			assertGetDouble(rs, columnIndex++, f_blobs[i], e);				// f_blob

			// NCLOB 列を getDouble
			e = checkParseDouble(f_nclobs[i]);
			assertGetDouble(rs, columnIndex++, f_nclobs[i], e);				// f_nclob

			// INT 配列を getDouble
			e = classCast;
			assertGetDouble(rs, columnIndex++, af_ints[i], e);				// af_int

			// BIGINT 配列を getDouble
			assertGetDouble(rs, columnIndex++, af_bigints[i], e);		// af_bigint
			assertGetDouble(rs, columnIndex++, af_decimals[i], e);		// af_decimal

			// CHAR 配列を getDouble
			assertGetDouble(rs, columnIndex++, af_char8s[i], e);			// af_char8

			// FLOAT 配列を getDouble
			assertGetDouble(rs, columnIndex++, af_floats[i], e);			// af_float

			// DATETIME 配列を getDouble
			assertGetDouble(rs, columnIndex++, af_datetimes[i], e);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getDouble
			assertGetDouble(rs, columnIndex++, af_ids[i], e);				// af_id

			// IMAGE 配列を getDouble
			assertGetDouble(rs, columnIndex++, af_images[i], e);			// af_image

			// LANGUAGE 配列を getDouble
			assertGetDouble(rs, columnIndex++, af_languages[i], e);			// af_language

			// NCHAR 配列を getDouble
			assertGetDouble(rs, columnIndex++, af_nchar6s[i], e);			// af_nchar6

			// NVARCHAR 配列を getDouble
			assertGetDouble(rs, columnIndex++, af_nvarchar256s[i], e);		// af_nvarchar256

			// VARCHAR 配列を getDouble
			assertGetDouble(rs, columnIndex++, af_varchar128s[i], e);		// af_varchar128

			// NTEXT 配列を getDouble
			assertGetDouble(rs, columnIndex++, af_ntexts[i], e);			// af_ntext

			// NTEXT (compressed) 配列を getDouble
			assertGetDouble(rs, columnIndex++, af_ntext_compresseds[i], e);	// af_ntext_compressed

			// FULLTEXT 配列を getDouble
			assertGetDouble(rs, columnIndex++, af_fulltexts[i], e);			// af_fulltext

			// BINARY 配列を getDouble
			assertGetDouble(rs, columnIndex++, af_binary50s[i], e);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getDouble(String) のテスト
	public void test_getDouble2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			// INT (not null) 列を getDouble
			assertGetDouble(rs, "f_int_not_null", f_int_not_nulls[i]);

			// INT (primary key) 列を getDouble
			assertGetDouble(rs, "f_int1", f_int1s[i]);

			// INT 列を getDouble
			assertGetDouble(rs, "f_int2", f_int2s[i]);

			// BIGINT 列を getDouble
			assertGetDouble(rs, "f_bigint", f_bigints[i]);
			assertGetDouble(rs, "f_decimal", f_decimals[i]);

			// CHAR (not null) 列を getDouble
			e = checkParseDouble(f_char8_not_nulls[i]);
			assertGetDouble(rs, "f_char8_not_null", f_char8_not_nulls[i], e);

			// CHAR 列を getDouble
			e = checkParseDouble(f_char8s[i]);
			assertGetDouble(rs, "f_char8", f_char8s[i], e);

			// FLOAT 列を getDouble
			assertGetDouble(rs, "f_float", f_floats[i]);

			// DATETIME 列を getDouble
			e = classCast;
			assertGetDouble(rs, "f_datetime", f_datetimes[i], e);

			// UNIQUEIDENTIFIER 列を getDouble
			assertGetDouble(rs, "f_id", f_ids[i], e);

			// IMAGE 列を getDouble
			assertGetDouble(rs, "f_image", f_images[i], e);

			// LANGUAGE 列を getDouble
			assertGetDouble(rs, "f_language", f_languages[i], e);

			// NCHAR 列を getDouble
			e = checkParseDouble(f_nchar6s[i]);
			assertGetDouble(rs, "f_nchar6", f_nchar6s[i], e);

			// NVARCHAR 列を getDouble
			e = checkParseDouble(f_nvarchar256s[i]);
			assertGetDouble(rs, "f_nvarchar256", f_nvarchar256s[i], e);

			// VARCHAR 列を getDouble
			e = checkParseDouble(f_varchar128s[i]);
			assertGetDouble(rs, "f_varchar128", f_varchar128s[i], e);

			// NTEXT 列を getDouble
			e = checkParseDouble(f_ntexts[i]);
			assertGetDouble(rs, "f_ntext", f_ntexts[i], e);

			// NTEXT (compressed) 列を getDouble
			e = checkParseDouble(f_ntext_compresseds[i]);
			assertGetDouble(rs, "f_ntext_compressed", f_ntext_compresseds[i], e);

			// FULLTEXT 列を getDouble
			e = checkParseDouble(f_fulltexts[i]);
			assertGetDouble(rs, "f_fulltext", f_fulltexts[i], e);

			// BINARY 列を getDouble
			e = classCast;
			assertGetDouble(rs, "f_binary50", f_binary50s[i], e);

			// BLOB 列を getDouble
			assertGetDouble(rs, "f_blob", f_blobs[i], e);

			// NCLOB 列を getDouble
			e = checkParseDouble(f_nclobs[i]);
			assertGetDouble(rs, "f_nclob", f_nclobs[i], e);

			// INT 配列を getDouble
			e = classCast;
			assertGetDouble(rs, "af_int", af_ints[i], e);

			// BIGINT 配列を getDouble
			assertGetDouble(rs, "af_bigint", af_bigints[i], e);
			assertGetDouble(rs, "af_decimal", af_decimals[i], e);

			// CHAR 配列を getDouble
			assertGetDouble(rs, "af_char8", af_char8s[i], e);

			// FLOAT 配列を getDouble
			assertGetDouble(rs, "af_float", af_floats[i], e);

			// DATETIME 配列を getDouble
			assertGetDouble(rs, "af_datetime", af_datetimes[i], e);

			// UNIQUEIDENTIFIER 配列を getDouble
			assertGetDouble(rs, "af_id", af_ids[i], e);

			// IMAGE 配列を getDouble
			assertGetDouble(rs, "af_image", af_images[i], e);

			// LANGUAGE 配列を getDouble
			assertGetDouble(rs, "af_language", af_languages[i], e);

			// NCHAR 配列を getDouble
			assertGetDouble(rs, "af_nchar6", af_nchar6s[i], e);

			// NVARCHAR 配列を getDouble
			assertGetDouble(rs, "af_nvarchar256", af_nvarchar256s[i], e);

			// VARCHAR 配列を getDouble
			assertGetDouble(rs, "af_varchar128", af_varchar128s[i], e);

			// NTEXT 配列を getDouble
			assertGetDouble(rs, "af_ntext", af_ntexts[i], e);

			// NTEXT (compressed) 配列を getDouble
			assertGetDouble(rs, "af_ntext_compressed", af_ntext_compresseds[i], e);

			// FULLTEXT 配列を getDouble
			assertGetDouble(rs, "af_fulltext", af_fulltexts[i], e);

			// BINARY 配列を getDouble
			assertGetDouble(rs, "af_binary50", af_binary50s[i], e);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getFloat(int) のテスト
	public void test_getFloat1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			int	columnIndex = 1;

			// INT (not null) 列を getFloat
			assertGetFloat(rs, columnIndex++, f_int_not_nulls[i]);			// f_int_not_null

			// INT (primary key) 列を getFloat
			assertGetFloat(rs, columnIndex++, f_int1s[i]);					// f_int1

			// INT 列を getFloat
			assertGetFloat(rs, columnIndex++, f_int2s[i]);					// f_int2

			// BIGINT 列を getFloat
			assertGetFloat(rs, columnIndex++, f_bigints[i]);			// f_bigint
			assertGetFloat(rs, columnIndex++, f_decimals[i]);			// f_decimal

			// CHAR (not null) 列を getFloat
			e = checkParseFloat(f_char8_not_nulls[i]);
			assertGetFloat(rs, columnIndex++, f_char8_not_nulls[i], e);		// f_char8_not_null

			// CHAR 列を getFloat
			e = checkParseFloat(f_char8s[i]);
			assertGetFloat(rs, columnIndex++, f_char8s[i], e);				// f_char8

			// FLOAT 列を getFloat
			assertGetFloat(rs, columnIndex++, f_floats[i]);					// f_float

			// DATETIME 列を getFloat
			e = classCast;
			assertGetFloat(rs, columnIndex++, f_datetimes[i], e);			// f_datetime

			// UNIQUEIDENTIFIER 列を getFloat
			assertGetFloat(rs, columnIndex++, f_ids[i], e);					// f_id

			// IMAGE 列を getFloat
			assertGetFloat(rs, columnIndex++, f_images[i], e);				// f_image

			// LANGUAGE 列を getFloat
			assertGetFloat(rs, columnIndex++, f_languages[i], e);			// f_language

			// NCHAR 列を getFloat
			e = checkParseFloat(f_nchar6s[i]);
			assertGetFloat(rs, columnIndex++, f_nchar6s[i], e);				// f_nchar6

			// NVARCHAR 列を getFloat
			e = checkParseFloat(f_nvarchar256s[i]);
			assertGetFloat(rs, columnIndex++, f_nvarchar256s[i], e);		// f_nvarchar256

			// VARCHAR 列を getFloat
			e = checkParseFloat(f_varchar128s[i]);
			assertGetFloat(rs, columnIndex++, f_varchar128s[i], e);			// f_varchar128

			// NTEXT 列を getFloat
			e = checkParseFloat(f_ntexts[i]);
			assertGetFloat(rs, columnIndex++, f_ntexts[i], e);				// f_ntext

			// NTEXT (compressed) 列を getFloat
			e = checkParseFloat(f_ntext_compresseds[i]);
			assertGetFloat(rs, columnIndex++, f_ntext_compresseds[i], e);	// f_ntext_compressed

			// FULLTEXT 列を getFloat
			e = checkParseFloat(f_fulltexts[i]);
			assertGetFloat(rs, columnIndex++, f_fulltexts[i], e);			// f_fulltext

			// BINARY 列を getFloat
			e = classCast;
			assertGetFloat(rs, columnIndex++, f_binary50s[i], e);			// f_binary50

			// BLOB 列を getFloat
			assertGetFloat(rs, columnIndex++, f_blobs[i], e);				// f_blob

			// NCLOB 列を getFloat
			e = checkParseFloat(f_nclobs[i]);
			assertGetFloat(rs, columnIndex++, f_nclobs[i], e);				// f_nclob

			// INT 配列を getFloat
			e = classCast;
			assertGetFloat(rs, columnIndex++, af_ints[i], e);				// af_int

			// BIGINT 配列を getFloat
			assertGetFloat(rs, columnIndex++, af_bigints[i], e);		// af_bigint
			assertGetFloat(rs, columnIndex++, af_decimals[i], e);		// af_decimal

			// CHAR 配列を getFloat
			assertGetFloat(rs, columnIndex++, af_char8s[i], e);				// af_char8

			// FLOAT 配列を getFloat
			assertGetFloat(rs, columnIndex++, af_floats[i], e);				// af_float

			// DATETIME 配列を getFloat
			assertGetFloat(rs, columnIndex++, af_datetimes[i], e);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getFloat
			assertGetFloat(rs, columnIndex++, af_ids[i], e);				// af_id

			// IMAGE 配列を getFloat
			assertGetFloat(rs, columnIndex++, af_images[i], e);				// af_image

			// LANGUAGE 配列を getFloat
			assertGetFloat(rs, columnIndex++, af_languages[i], e);			// af_language

			// NCHAR 配列を getFloat
			assertGetFloat(rs, columnIndex++, af_nchar6s[i], e);			// af_nchar6

			// NVARCHAR 配列を getFloat
			assertGetFloat(rs, columnIndex++, af_nvarchar256s[i], e);		// af_nvarchar256

			// VARCHAR 配列を getFloat
			assertGetFloat(rs, columnIndex++, af_varchar128s[i], e);		// af_varchar128

			// NTEXT 配列を getFloat
			assertGetFloat(rs, columnIndex++, af_ntexts[i], e);				// af_ntext

			// NTEXT (compressed) 配列を getFloat
			assertGetFloat(rs, columnIndex++, af_ntext_compresseds[i], e);	// af_ntext_compressed

			// FULLTEXT 配列を getFloat
			assertGetFloat(rs, columnIndex++, af_fulltexts[i], e);			// af_fulltext

			// BINARY 配列を getFloat
			assertGetFloat(rs, columnIndex++, af_binary50s[i], e);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getFloat(String) のテスト
	public void test_getFloat2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			// INT (not null) 列を getFloat
			assertGetFloat(rs, "f_int_not_null", f_int_not_nulls[i]);

			// INT (primary key) 列を getFloat
			assertGetFloat(rs, "f_int1", f_int1s[i]);

			// INT 列を getFloat
			assertGetFloat(rs, "f_int2", f_int2s[i]);

			// BIGINT 列を getFloat
			assertGetFloat(rs, "f_bigint", f_bigints[i]);
			assertGetFloat(rs, "f_decimal", f_decimals[i]);

			// CHAR (not null) 列を getFloat
			e = checkParseFloat(f_char8_not_nulls[i]);
			assertGetFloat(rs, "f_char8_not_null", f_char8_not_nulls[i], e);

			// CHAR 列を getFloat
			e = checkParseFloat(f_char8s[i]);
			assertGetFloat(rs, "f_char8", f_char8s[i], e);

			// FLOAT 列を getFloat
			assertGetFloat(rs, "f_float", f_floats[i]);

			// DATETIME 列を getFloat
			e = classCast;
			assertGetFloat(rs, "f_datetime", f_datetimes[i], e);

			// UNIQUEIDENTIFIER 列を getFloat
			assertGetFloat(rs, "f_id", f_ids[i], e);

			// IMAGE 列を getFloat
			assertGetFloat(rs, "f_image", f_images[i], e);

			// LANGUAGE 列を getFloat
			assertGetFloat(rs, "f_language", f_languages[i], e);

			// NCHAR 列を getFloat
			e = checkParseFloat(f_nchar6s[i]);
			assertGetFloat(rs, "f_nchar6", f_nchar6s[i], e);

			// NVARCHAR 列を getFloat
			e = checkParseFloat(f_nvarchar256s[i]);
			assertGetFloat(rs, "f_nvarchar256", f_nvarchar256s[i], e);

			// VARCHAR 列を getFloat
			e = checkParseFloat(f_varchar128s[i]);
			assertGetFloat(rs, "f_varchar128", f_varchar128s[i], e);

			// NTEXT 列を getFloat
			e = checkParseFloat(f_ntexts[i]);
			assertGetFloat(rs, "f_ntext", f_ntexts[i], e);

			// NTEXT (compressed) 列を getFloat
			e = checkParseFloat(f_ntext_compresseds[i]);
			assertGetFloat(rs, "f_ntext_compressed", f_ntext_compresseds[i], e);

			// FULLTEXT 列を getFloat
			e = checkParseFloat(f_fulltexts[i]);
			assertGetFloat(rs, "f_fulltext", f_fulltexts[i], e);

			// BINARY 列を getFloat
			e = classCast;
			assertGetFloat(rs, "f_binary50", f_binary50s[i], e);

			// BLOB 列を getFloat
			assertGetFloat(rs, "f_blob", f_blobs[i], e);

			// NCLOB 列を getFloat
			e = checkParseFloat(f_nclobs[i]);
			assertGetFloat(rs, "f_nclob", f_nclobs[i], e);

			// INT 配列を getFloat
			e = classCast;
			assertGetFloat(rs, "af_int", af_ints[i], e);

			// BIGINT 配列を getFloat
			assertGetFloat(rs, "af_bigint", af_bigints[i], e);
			assertGetFloat(rs, "af_decimal", af_decimals[i], e);

			// CHAR 配列を getFloat
			assertGetFloat(rs, "af_char8", af_char8s[i], e);

			// FLOAT 配列を getFloat
			assertGetFloat(rs, "af_float", af_floats[i], e);

			// DATETIME 配列を getFloat
			assertGetFloat(rs, "af_datetime", af_datetimes[i], e);

			// UNIQUEIDENTIFIER 配列を getFloat
			assertGetFloat(rs, "af_id", af_ids[i], e);

			// IMAGE 配列を getFloat
			assertGetFloat(rs, "af_image", af_images[i], e);

			// LANGUAGE 配列を getFloat
			assertGetFloat(rs, "af_language", af_languages[i], e);

			// NCHAR 配列を getFloat
			assertGetFloat(rs, "af_nchar6", af_nchar6s[i], e);

			// NVARCHAR 配列を getFloat
			assertGetFloat(rs, "af_nvarchar256", af_nvarchar256s[i], e);

			// VARCHAR 配列を getFloat
			assertGetFloat(rs, "af_varchar128", af_varchar128s[i], e);

			// NTEXT 配列を getFloat
			assertGetFloat(rs, "af_ntext", af_ntexts[i], e);

			// NTEXT (compressed) 配列を getFloat
			assertGetFloat(rs, "af_ntext_compressed", af_ntext_compresseds[i], e);

			// FULLTEXT 配列を getFloat
			assertGetFloat(rs, "af_fulltext", af_fulltexts[i], e);

			// BINARY 配列を getFloat
			assertGetFloat(rs, "af_binary50", af_binary50s[i], e);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getInt(int) のテスト
	public void test_getInt1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			int	columnIndex = 1;

			// INT (not null) 列を getInt
			assertGetInt(rs, columnIndex++, f_int_not_nulls[i]);			// f_int_not_null

			// INT (primary key) 列を getInt
			assertGetInt(rs, columnIndex++, f_int1s[i]);					// f_int1

			// INT 列を getInt
			assertGetInt(rs, columnIndex++, f_int2s[i]);					// f_int2

			// BIGINT 列を getInt
			assertGetInt(rs, columnIndex++, f_bigints[i]);				// f_bigint
			assertGetInt(rs, columnIndex++, f_decimals[i]);				// f_decimal

			// CHAR (not null) 列を getInt
			e = checkParseInt(f_char8_not_nulls[i]);
			assertGetInt(rs, columnIndex++, f_char8_not_nulls[i], e);		// f_char8_not_null

			// CHAR 列を getInt
			e = checkParseInt(f_char8s[i]);
			assertGetInt(rs, columnIndex++, f_char8s[i], e);				// f_char8

			// FLOAT 列を getInt
			assertGetInt(rs, columnIndex++, f_floats[i]);					// f_float

			// DATETIME 列を getInt
			e = classCast;
			assertGetInt(rs, columnIndex++, f_datetimes[i], e);				// f_datetime

			// UNIQUEIDENTIFIER 列を getInt
			assertGetInt(rs, columnIndex++, f_ids[i], e);					// f_id

			// IMAGE 列を getInt
			assertGetInt(rs, columnIndex++, f_images[i], e);				// f_image

			// LANGUAGE 列を getInt
			assertGetInt(rs, columnIndex++, f_languages[i], e);				// f_language

			// NCHAR 列を getInt
			e = checkParseInt(f_nchar6s[i]);
			assertGetInt(rs, columnIndex++, f_nchar6s[i], e);				// f_nchar6

			// NVARCHAR 列を getInt
			e = checkParseInt(f_nvarchar256s[i]);
			assertGetInt(rs, columnIndex++, f_nvarchar256s[i], e);			// f_nvarchar256

			// VARCHAR 列を getInt
			e = checkParseInt(f_varchar128s[i]);
			assertGetInt(rs, columnIndex++, f_varchar128s[i], e);			// f_varchar128

			// NTEXT 列を getInt
			e = checkParseInt(f_ntexts[i]);
			assertGetInt(rs, columnIndex++, f_ntexts[i], e);				// f_ntext

			// NTEXT (compressed) 列を getInt
			e = checkParseInt(f_ntext_compresseds[i]);
			assertGetInt(rs, columnIndex++, f_ntext_compresseds[i], e);		// f_ntext_compressed

			// FULLTEXT 列を getInt
			e = checkParseInt(f_fulltexts[i]);
			assertGetInt(rs, columnIndex++, f_fulltexts[i], e);				// f_fulltext

			// BINARY 列を getInt
			e = classCast;
			assertGetInt(rs, columnIndex++, f_binary50s[i], e);				// f_binary50

			// BLOB 列を getInt
			assertGetInt(rs, columnIndex++, f_blobs[i], e);					// f_blob

			// NCLOB 列を getInt
			e = checkParseInt(f_nclobs[i]);
			assertGetInt(rs, columnIndex++, f_nclobs[i], e);				// f_nclob

			// INT 配列を getInt
			e = classCast;
			assertGetInt(rs, columnIndex++, af_ints[i], e);					// af_int

			// BIGINT 配列を getInt
			assertGetInt(rs, columnIndex++, af_bigints[i], e);			// af_bigint
			assertGetInt(rs, columnIndex++, af_decimals[i], e);			// af_decimal

			// CHAR 配列を getInt
			assertGetInt(rs, columnIndex++, af_char8s[i], e);				// af_char8

			// FLOAT 配列を getInt
			assertGetInt(rs, columnIndex++, af_floats[i], e);				// af_float

			// DATETIME 配列を getInt
			assertGetInt(rs, columnIndex++, af_datetimes[i], e);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getInt
			assertGetInt(rs, columnIndex++, af_ids[i], e);					// af_id

			// IMAGE 配列を getInt
			assertGetInt(rs, columnIndex++, af_images[i], e);				// af_image

			// LANGUAGE 配列を getInt
			assertGetInt(rs, columnIndex++, af_languages[i], e);			// af_language

			// NCHAR 配列を getInt
			assertGetInt(rs, columnIndex++, af_nchar6s[i], e);				// af_nchar6

			// NVARCHAR 配列を getInt
			assertGetInt(rs, columnIndex++, af_nvarchar256s[i], e);			// af_nvarchar256

			// VARCHAR 配列を getInt
			assertGetInt(rs, columnIndex++, af_varchar128s[i], e);			// af_varchar128

			// NTEXT 配列を getInt
			assertGetInt(rs, columnIndex++, af_ntexts[i], e);				// af_ntext

			// NTEXT (compressed) 配列を getInt
			assertGetInt(rs, columnIndex++, af_ntext_compresseds[i], e);	// af_ntext_compressed

			// FULLTEXT 配列を getInt
			assertGetInt(rs, columnIndex++, af_fulltexts[i], e);			// af_fulltext

			// BINARY 配列を getInt
			assertGetInt(rs, columnIndex++, af_binary50s[i], e);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getInt(String) のテスト
	public void test_getInt2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			// INT (not null) 列を getInt
			assertGetInt(rs, "f_int_not_null", f_int_not_nulls[i]);

			// INT (primary key) 列を getInt
			assertGetInt(rs, "f_int1", f_int1s[i]);

			// INT 列を getInt
			assertGetInt(rs, "f_int2", f_int2s[i]);

			// BIGINT 列を getInt
			assertGetInt(rs, "f_bigint", f_bigints[i]);
			assertGetInt(rs, "f_decimal", f_decimals[i]);

			// CHAR (not null) 列を getInt
			e = checkParseInt(f_char8_not_nulls[i]);
			assertGetInt(rs, "f_char8_not_null", f_char8_not_nulls[i], e);

			// CHAR 列を getInt
			e = checkParseInt(f_char8s[i]);
			assertGetInt(rs, "f_char8", f_char8s[i], e);

			// FLOAT 列を getInt
			assertGetInt(rs, "f_float", f_floats[i]);

			// DATETIME 列を getInt
			e = classCast;
			assertGetInt(rs, "f_datetime", f_datetimes[i], e);

			// UNIQUEIDENTIFIER 列を getInt
			assertGetInt(rs, "f_id", f_ids[i], e);

			// IMAGE 列を getInt
			assertGetInt(rs, "f_image", f_images[i], e);

			// LANGUAGE 列を getInt
			assertGetInt(rs, "f_language", f_languages[i], e);

			// NCHAR 列を getInt
			e = checkParseInt(f_nchar6s[i]);
			assertGetInt(rs, "f_nchar6", f_nchar6s[i], e);

			// NVARCHAR 列を getInt
			e = checkParseInt(f_nvarchar256s[i]);
			assertGetInt(rs, "f_nvarchar256", f_nvarchar256s[i], e);

			// VARCHAR 列を getInt
			e = checkParseInt(f_varchar128s[i]);
			assertGetInt(rs, "f_varchar128", f_varchar128s[i], e);

			// NTEXT 列を getInt
			e = checkParseInt(f_ntexts[i]);
			assertGetInt(rs, "f_ntext", f_ntexts[i], e);

			// NTEXT (compressed) 列を getInt
			e = checkParseInt(f_ntext_compresseds[i]);
			assertGetInt(rs, "f_ntext_compressed", f_ntext_compresseds[i], e);

			// FULLTEXT 列を getInt
			e = checkParseInt(f_fulltexts[i]);
			assertGetInt(rs, "f_fulltext", f_fulltexts[i], e);

			// BINARY 列を getInt
			e = classCast;
			assertGetInt(rs, "f_binary50", f_binary50s[i], e);

			// BLOB 列を getInt
			assertGetInt(rs, "f_blob", f_blobs[i], e);

			// NCLOB 列を getInt
			e = checkParseInt(f_nclobs[i]);
			assertGetInt(rs, "f_nclob", f_nclobs[i], e);

			// INT 配列を getInt
			e = classCast;
			assertGetInt(rs, "af_int", af_ints[i], e);

			// BIGINT 配列を getInt
			assertGetInt(rs, "af_bigint", af_bigints[i], e);
			assertGetInt(rs, "af_decimal", af_decimals[i], e);

			// CHAR 配列を getInt
			assertGetInt(rs, "af_char8", af_char8s[i], e);

			// FLOAT 配列を getInt
			assertGetInt(rs, "af_float", af_floats[i], e);

			// DATETIME 配列を getInt
			assertGetInt(rs, "af_datetime", af_datetimes[i], e);

			// UNIQUEIDENTIFIER 配列を getInt
			assertGetInt(rs, "af_id", af_ids[i], e);

			// IMAGE 配列を getInt
			assertGetInt(rs, "af_image", af_images[i], e);

			// LANGUAGE 配列を getInt
			assertGetInt(rs, "af_language", af_languages[i], e);

			// NCHAR 配列を getInt
			assertGetInt(rs, "af_nchar6", af_nchar6s[i], e);

			// NVARCHAR 配列を getInt
			assertGetInt(rs, "af_nvarchar256", af_nvarchar256s[i], e);

			// VARCHAR 配列を getInt
			assertGetInt(rs, "af_varchar128", af_varchar128s[i], e);

			// NTEXT 配列を getInt
			assertGetInt(rs, "af_ntext", af_ntexts[i], e);

			// NTEXT (compressed) 配列を getInt
			assertGetInt(rs, "af_ntext_compressed", af_ntext_compresseds[i], e);

			// FULLTEXT 配列を getInt
			assertGetInt(rs, "af_fulltext", af_fulltexts[i], e);

			// BINARY 配列を getInt
			assertGetInt(rs, "af_binary50", af_binary50s[i], e);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getLong(int) のテスト
	public void test_getLong1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			int	columnIndex = 1;

			// INT (not null) 列を getLong
			assertGetLong(rs, columnIndex++, f_int_not_nulls[i]);			// f_int_not_null

			// INT (primary key) 列を getLong
			assertGetLong(rs, columnIndex++, f_int1s[i]);					// f_int1

			// INT 列を getLong
			assertGetLong(rs, columnIndex++, f_int2s[i]);					// f_int2

			// BIGINT 列を getLog
			assertGetLong(rs, columnIndex++, f_bigints[i]);				// f_bigint
			assertGetLong(rs, columnIndex++, f_decimals[i]);				// f_decimal

			// CHAR (not null) 列を getLong
			e = checkParseLong(f_char8_not_nulls[i]);
			assertGetLong(rs, columnIndex++, f_char8_not_nulls[i], e);		// f_char8_not_null

			// CHAR 列を getLong
			e = checkParseLong(f_char8s[i]);
			assertGetLong(rs, columnIndex++, f_char8s[i], e);				// f_char8

			// FLOAT 列を getLong
			assertGetLong(rs, columnIndex++, f_floats[i]);					// f_float

			// DATETIME 列を getLong
			e = classCast;
			assertGetLong(rs, columnIndex++, f_datetimes[i], e);			// f_datetime

			// UNIQUEIDENTIFIER 列を getLong
			assertGetLong(rs, columnIndex++, f_ids[i], e);					// f_id

			// IMAGE 列を getLong
			assertGetLong(rs, columnIndex++, f_images[i], e);				// f_image

			// LANGUAGE 列を getLong
			assertGetLong(rs, columnIndex++, f_languages[i], e);			// f_language

			// NCHAR 列を getLong
			e = checkParseLong(f_nchar6s[i]);
			assertGetLong(rs, columnIndex++, f_nchar6s[i], e);				// f_nchar6

			// NVARCHAR 列を getLong
			e = checkParseLong(f_nvarchar256s[i]);
			assertGetLong(rs, columnIndex++, f_nvarchar256s[i], e);			// f_nvarchar256

			// VARCHAR 列を getLong
			e = checkParseLong(f_varchar128s[i]);
			assertGetLong(rs, columnIndex++, f_varchar128s[i], e);			// f_varchar128

			// NTEXT 列を getLong
			e = checkParseLong(f_ntexts[i]);
			assertGetLong(rs, columnIndex++, f_ntexts[i], e);				// f_ntext

			// NTEXT (compressed) 列を getLong
			e = checkParseLong(f_ntext_compresseds[i]);
			assertGetLong(rs, columnIndex++, f_ntext_compresseds[i], e);	// f_ntext_compressed

			// FULLTEXT 列を getLong
			e = checkParseLong(f_fulltexts[i]);
			assertGetLong(rs, columnIndex++, f_fulltexts[i], e);			// f_fulltext

			// BINARY 列を getLong
			e = classCast;
			assertGetLong(rs, columnIndex++, f_binary50s[i], e);			// f_binary50

			// BLOB 列を getLong
			assertGetLong(rs, columnIndex++, f_blobs[i], e);				// f_blob

			// NCLOB 列を getLong
			e = checkParseLong(f_nclobs[i]);
			assertGetLong(rs, columnIndex++, f_nclobs[i], e);				// f_nclob

			// INT 配列を getLong
			e = classCast;
			assertGetLong(rs, columnIndex++, af_ints[i], e);				// af_int

			// BIGINT 配列を getLong
			assertGetLong(rs, columnIndex++, af_bigints[i], e);			// af_bigint
			assertGetLong(rs, columnIndex++, af_decimals[i], e);			// af_decimal

			// CHAR 配列を getLong
			assertGetLong(rs, columnIndex++, af_char8s[i], e);				// af_char8

			// FLOAT 配列を getLong
			assertGetLong(rs, columnIndex++, af_floats[i], e);				// af_float

			// DATETIME 配列を getLong
			assertGetLong(rs, columnIndex++, af_datetimes[i], e);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getLong
			assertGetLong(rs, columnIndex++, af_ids[i], e);					// af_id

			// IMAGE 配列を getLong
			assertGetLong(rs, columnIndex++, af_images[i], e);				// af_image

			// LANGUAGE 配列を getLong
			assertGetLong(rs, columnIndex++, af_languages[i], e);			// af_language

			// NCHAR 配列を getLong
			assertGetLong(rs, columnIndex++, af_nchar6s[i], e);				// af_nchar6

			// NVARCHAR 配列を getLong
			assertGetLong(rs, columnIndex++, af_nvarchar256s[i], e);		// af_nvarchar256

			// VARCHAR 配列を getLong
			assertGetLong(rs, columnIndex++, af_varchar128s[i], e);			// af_varchar128

			// NTEXT 配列を getLong
			assertGetLong(rs, columnIndex++, af_ntexts[i], e);				// af_ntext

			// NTEXT (compressed) 配列を getLong
			assertGetLong(rs, columnIndex++, af_ntext_compresseds[i], e);	// af_ntext_compressed

			// FULLTEXT 配列を getLong
			assertGetLong(rs, columnIndex++, af_fulltexts[i], e);			// af_fulltext

			// BINARY 配列を getLong
			assertGetLong(rs, columnIndex++, af_binary50s[i], e);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getLong(String) のテスト
	public void test_getLong2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			assertTrue(rs.next());

			// INT (not null) 列を getLong
			assertGetLong(rs, "f_int_not_null", f_int_not_nulls[i]);

			// INT (primary key) 列を getLong
			assertGetLong(rs, "f_int1", f_int1s[i]);

			// INT 列を getLong
			assertGetLong(rs, "f_int2", f_int2s[i]);

			// BIGINT 列を getLong
			assertGetLong(rs, "f_bigint", f_bigints[i]);
			assertGetLong(rs, "f_decimal", f_decimals[i]);

			// CHAR (not null) 列を getLong
			e = checkParseLong(f_char8_not_nulls[i]);
			assertGetLong(rs, "f_char8_not_null", f_char8_not_nulls[i], e);

			// CHAR 列を getLong
			e = checkParseLong(f_char8s[i]);
			assertGetLong(rs, "f_char8", f_char8s[i], e);

			// FLOAT 列を getLong
			assertGetLong(rs, "f_float", f_floats[i]);

			// DATETIME 列を getLong
			e = classCast;
			assertGetLong(rs, "f_datetime", f_datetimes[i], e);

			// UNIQUEIDENTIFIER 列を getLong
			assertGetLong(rs, "f_id", f_ids[i], e);

			// IMAGE 列を getLong
			assertGetLong(rs, "f_image", f_images[i], e);

			// LANGUAGE 列を getLong
			assertGetLong(rs, "f_language", f_languages[i], e);

			// NCHAR 列を getLong
			e = checkParseLong(f_nchar6s[i]);
			assertGetLong(rs, "f_nchar6", f_nchar6s[i], e);

			// NVARCHAR 列を getLong
			e = checkParseLong(f_nvarchar256s[i]);
			assertGetLong(rs, "f_nvarchar256", f_nvarchar256s[i], e);

			// VARCHAR 列を getLong
			e = checkParseLong(f_varchar128s[i]);
			assertGetLong(rs, "f_varchar128", f_varchar128s[i], e);

			// NTEXT 列を getLong
			e = checkParseLong(f_ntexts[i]);
			assertGetLong(rs, "f_ntext", f_ntexts[i], e);

			// NTEXT (compressed) 列を getLong
			e = checkParseLong(f_ntext_compresseds[i]);
			assertGetLong(rs, "f_ntext_compressed", f_ntext_compresseds[i], e);

			// FULLTEXT 列を getLong
			e = checkParseLong(f_fulltexts[i]);
			assertGetLong(rs, "f_fulltext", f_fulltexts[i], e);

			// BINARY 列を getLong
			e = classCast;
			assertGetLong(rs, "f_binary50", f_binary50s[i], e);

			// BLOB 列を getLong
			assertGetLong(rs, "f_blob", f_blobs[i], e);

			// NCLOB 列を getLong
			e = checkParseLong(f_nclobs[i]);
			assertGetLong(rs, "f_nclob", f_nclobs[i], e);

			// INT 配列を getLong
			e = classCast;
			assertGetLong(rs, "af_int", af_ints[i], e);

			// BIGINT 配列を getLong
			assertGetLong(rs, "af_bigint", af_bigints[i], e);
			assertGetLong(rs, "af_decimal", af_decimals[i], e);

			// CHAR 配列を getLong
			assertGetLong(rs, "af_char8", af_char8s[i], e);

			// FLOAT 配列を getLong
			assertGetLong(rs, "af_float", af_floats[i], e);

			// DATETIME 配列を getLong
			assertGetLong(rs, "af_datetime", af_datetimes[i], e);

			// UNIQUEIDENTIFIER 配列を getLong
			assertGetLong(rs, "af_id", af_ids[i], e);

			// IMAGE 配列を getLong
			assertGetLong(rs, "af_image", af_images[i], e);

			// LANGUAGE 配列を getLong
			assertGetLong(rs, "af_language", af_languages[i], e);

			// NCHAR 配列を getLong
			assertGetLong(rs, "af_nchar6", af_nchar6s[i], e);

			// NVARCHAR 配列を getLong
			assertGetLong(rs, "af_nvarchar256", af_nvarchar256s[i], e);

			// VARCHAR 配列を getLong
			assertGetLong(rs, "af_varchar128", af_varchar128s[i], e);

			// NTEXT 配列を getLong
			assertGetLong(rs, "af_ntext", af_ntexts[i], e);

			// NTEXT (compressed) 配列を getLong
			assertGetLong(rs, "af_ntext_compressed", af_ntext_compresseds[i], e);

			// FULLTEXT 配列を getLong
			assertGetLong(rs, "af_fulltext", af_fulltexts[i], e);

			// BINARY 配列を getLong
			assertGetLong(rs, "af_binary50", af_binary50s[i], e);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getMetaData() のテストは ResultSetMetaData のテストで散々やるので省く

	// ResultSet.getObject(int) のテスト
	public void test_getObject1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			int	columnIndex = 1;

			// INT (not null) 列を getObject
			assertGetObject(rs, columnIndex++, f_int_not_nulls[i]);			// f_int_not_null

			// INT (primary key) 列を getObject
			assertGetObject(rs, columnIndex++, f_int1s[i]);					// f_int1

			// INT 列を getObject
			assertGetObject(rs, columnIndex++, f_int2s[i]);					// f_int2

			// BIGINT 列を getObject
			assertGetObject(rs, columnIndex++, f_bigints[i]);			// f_bigint
			assertGetObject(rs, columnIndex++, f_decimals[i]);			// f_decimal

			// CHAR (not null) 列を getObject
			assertGetObject(rs, columnIndex++, f_char8_not_nulls[i]);		// f_char8_not_null

			// CHAR 列を getObject
			assertGetObject(rs, columnIndex++, f_char8s[i]);				// f_char8

			// FLOAT 列を getObject
			assertGetObject(rs, columnIndex++, f_floats[i]);				// f_float

			// DATETIME 列を getObject
			assertGetObject(rs, columnIndex++, f_datetimes[i]);				// f_datetime

			// UNIQUEIDENTIFIER 列を getObject
			assertGetObject(rs, columnIndex++, f_ids[i]);					// f_id

			// IMAGE 列を getObject
			assertGetObject(rs, columnIndex++, f_images[i]);				// f_image

			// LANGUAGE 列を getObject
			assertGetObject(rs, columnIndex++, f_languages[i]);				// f_language

			// NCHAR 列を getObject
			assertGetObject(rs, columnIndex++, f_nchar6s[i]);				// f_nchar6

			// NVARCHAR 列を getObject
			assertGetObject(rs, columnIndex++, f_nvarchar256s[i]);			// f_nvarchar256

			// VARCHAR 列を getObject
			assertGetObject(rs, columnIndex++, f_varchar128s[i]);			// f_varchar128

			// NTEXT 列を getObject
			assertGetObject(rs, columnIndex++, f_ntexts[i]);				// f_ntext

			// NTEXT (compressed) 列を getObject
			assertGetObject(rs, columnIndex++, f_ntext_compresseds[i]);		// f_ntext_compressed

			// FULLTEXT 列を getObject
			assertGetObject(rs, columnIndex++, f_fulltexts[i]);				// f_fulltext

			// BINARY 列を getObject
			assertGetObject(rs, columnIndex++, f_binary50s[i]);				// f_binary50

			// BLOB 列を getObject
			assertGetObject(rs, columnIndex++, f_blobs[i]);					// f_blob

			// NCLOB 列を getObject
			assertGetObject(rs, columnIndex++, f_nclobs[i]);				// f_nclob

			// INT 配列を getObject
			assertGetObject(rs, columnIndex++, af_ints[i]);					// af_int

			// BIGINT 配列を getObject
			assertGetObject(rs, columnIndex++, af_bigints[i]);			// af_bigint
			assertGetObject(rs, columnIndex++, af_decimals[i]);			// af_decimal

			// CHAR 配列を getObject
			assertGetObject(rs, columnIndex++, af_char8s[i]);				// af_char8

			// FLOAT 配列を getObject
			assertGetObject(rs, columnIndex++, af_floats[i]);				// af_float

			// DATETIME 配列を getObject
			assertGetObject(rs, columnIndex++, af_datetimes[i]);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getObject
			assertGetObject(rs, columnIndex++, af_ids[i]);					// af_id

			// IMAGE 配列を getObject
			assertGetObject(rs, columnIndex++, af_images[i]);				// af_image

			// LANGUAGE 配列を getObject
			assertGetObject(rs, columnIndex++, af_languages[i]);			// af_language

			// NCHAR 配列を getObject
			assertGetObject(rs, columnIndex++, af_nchar6s[i]);				// af_nchar6

			// NVARCHAR 配列を getObject
			assertGetObject(rs, columnIndex++, af_nvarchar256s[i]);			// af_nvarchar256

			// VARCHAR 配列を getObject
			assertGetObject(rs, columnIndex++, af_varchar128s[i]);			// af_varchar128

			// NTEXT 配列を getObject
			assertGetObject(rs, columnIndex++, af_ntexts[i]);				// af_ntext

			// NTEXT (compressed) 配列を getObject
			assertGetObject(rs, columnIndex++, af_ntext_compresseds[i]);	// af_ntext_compressed

			// FULLTEXT 配列を getObject
			assertGetObject(rs, columnIndex++, af_fulltexts[i]);			// af_fulltext

			// BINARY 配列を getObject
			assertGetObject(rs, columnIndex++, af_binary50s[i]);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getObject(String) のテスト
	public void test_getObject2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			// INT (not null) 列を getObject
			assertGetObject(rs, "f_int_not_null", f_int_not_nulls[i]);

			// INT (primary key) 列を getObject
			assertGetObject(rs, "f_int1", f_int1s[i]);

			// INT 列を getObject
			assertGetObject(rs, "f_int2", f_int2s[i]);

			// BIGINT 列を getObject
			assertGetObject(rs, "f_bigint", f_bigints[i]);
			assertGetObject(rs, "f_decimal", f_decimals[i]);

			// CHAR (not null) 列を getObject
			assertGetObject(rs, "f_char8_not_null", f_char8_not_nulls[i]);

			// CHAR 列を getObject
			assertGetObject(rs, "f_char8", f_char8s[i]);

			// FLOAT 列を getObject
			assertGetObject(rs, "f_float", f_floats[i]);

			// DATETIME 列を getObject
			assertGetObject(rs, "f_datetime", f_datetimes[i]);

			// UNIQUEIDENTIFIER 列を getObject
			assertGetObject(rs, "f_id", f_ids[i]);

			// IMAGE 列を getObject
			assertGetObject(rs, "f_image", f_images[i]);

			// LANGUAGE 列を getObject
			assertGetObject(rs, "f_language", f_languages[i]);

			// NCHAR 列を getObject
			assertGetObject(rs, "f_nchar6", f_nchar6s[i]);

			// NVARCHAR 列を getObject
			assertGetObject(rs, "f_nvarchar256", f_nvarchar256s[i]);

			// VARCHAR 列を getObject
			assertGetObject(rs, "f_varchar128", f_varchar128s[i]);

			// NTEXT 列を getObject
			assertGetObject(rs, "f_ntext", f_ntexts[i]);

			// NTEXT (compressed) 列を getObject
			assertGetObject(rs, "f_ntext_compressed", f_ntext_compresseds[i]);

			// FULLTEXT 列を getObject
			assertGetObject(rs, "f_fulltext", f_fulltexts[i]);

			// BINARY 列を getObject
			assertGetObject(rs, "f_binary50", f_binary50s[i]);

			// BLOB 列を getObject
			assertGetObject(rs, "f_blob", f_blobs[i]);

			// NCLOB 列を getObject
			assertGetObject(rs, "f_nclob", f_nclobs[i]);

			// INT 配列を getObject
			assertGetObject(rs, "af_int", af_ints[i]);

			// BIGINT 配列を getObject
			assertGetObject(rs, "af_bigint", af_bigints[i]);
			assertGetObject(rs, "af_decimal", af_decimals[i]);

			// CHAR 配列を getObject
			assertGetObject(rs, "af_char8", af_char8s[i]);

			// FLOAT 配列を getObject
			assertGetObject(rs, "af_float", af_floats[i]);

			// DATETIME 配列を getObject
			assertGetObject(rs, "af_datetime", af_datetimes[i]);

			// UNIQUEIDENTIFIER 配列を getObject
			assertGetObject(rs, "af_id", af_ids[i]);

			// IMAGE 配列を getObject
			assertGetObject(rs, "af_image", af_images[i]);

			// LANGUAGE 配列を getObject
			assertGetObject(rs, "af_language", af_languages[i]);

			// NCHAR 配列を getObject
			assertGetObject(rs, "af_nchar6", af_nchar6s[i]);

			// NVARCHAR 配列を getObject
			assertGetObject(rs, "af_nvarchar256", af_nvarchar256s[i]);

			// VARCHAR 配列を getObject
			assertGetObject(rs, "af_varchar128", af_varchar128s[i]);

			// NTEXT 配列を getObject
			assertGetObject(rs, "af_ntext", af_ntexts[i]);

			// NTEXT (compressed) 配列を getObject
			assertGetObject(rs, "af_ntext_compressed", af_ntext_compresseds[i]);

			// FULLTEXT 配列を getObject
			assertGetObject(rs, "af_fulltext", af_fulltexts[i]);

			// BINARY 配列を getObject
			assertGetObject(rs, "af_binary50", af_binary50s[i]);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getObject(int, java.util.Map) のテスト
	public void test_getObject3() throws Exception
	{
		// Map は無視されるはず
		java.util.HashMap	hashMap = new java.util.HashMap();

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			int	columnIndex = 1;

			// INT (not null) 列を getObject
			assertGetObject(rs, columnIndex++, f_int_not_nulls[i], hashMap);		// f_int_not_null

			// INT (primary key) 列を getObject
			assertGetObject(rs, columnIndex++, f_int1s[i], hashMap);				// f_int1

			// INT 列を getObject
			assertGetObject(rs, columnIndex++, f_int2s[i], hashMap);				// f_int2

			// BIGINT 列を getObject
			assertGetObject(rs, columnIndex++, f_bigints[i], hashMap);			// f_bigint
			assertGetObject(rs, columnIndex++, f_decimals[i], hashMap);			// f_decimal

			// CHAR (not null) 列を getObject
			assertGetObject(rs, columnIndex++, f_char8_not_nulls[i], hashMap);		// f_char8_not_null

			// CHAR 列を getObject
			assertGetObject(rs, columnIndex++, f_char8s[i], hashMap);				// f_char8

			// FLOAT 列を getObject
			assertGetObject(rs, columnIndex++, f_floats[i], hashMap);				// f_float

			// DATETIME 列を getObject
			assertGetObject(rs, columnIndex++, f_datetimes[i], hashMap);			// f_datetime

			// UNIQUEIDENTIFIER 列を getObject
			assertGetObject(rs, columnIndex++, f_ids[i], hashMap);					// f_id

			// IMAGE 列を getObject
			assertGetObject(rs, columnIndex++, f_images[i], hashMap);				// f_image

			// LANGUAGE 列を getObject
			assertGetObject(rs, columnIndex++, f_languages[i], hashMap);			// f_language

			// NCHAR 列を getObject
			assertGetObject(rs, columnIndex++, f_nchar6s[i], hashMap);				// f_nchar6

			// NVARCHAR 列を getObject
			assertGetObject(rs, columnIndex++, f_nvarchar256s[i], hashMap);			// f_nvarchar256

			// VARCHAR 列を getObject
			assertGetObject(rs, columnIndex++, f_varchar128s[i], hashMap);			// f_varchar128

			// NTEXT 列を getObject
			assertGetObject(rs, columnIndex++, f_ntexts[i], hashMap);				// f_ntext

			// NTEXT (compressed) 列を getObject
			assertGetObject(rs, columnIndex++, f_ntext_compresseds[i], hashMap);	// f_ntext_compressed

			// FULLTEXT 列を getObject
			assertGetObject(rs, columnIndex++, f_fulltexts[i], hashMap);			// f_fulltext

			// BINARY 列を getObject
			assertGetObject(rs, columnIndex++, f_binary50s[i], hashMap);			// f_binary50

			// BLOB 列を getObject
			assertGetObject(rs, columnIndex++, f_blobs[i], hashMap);				// f_blob

			// NCLOB 列を getObject
			assertGetObject(rs, columnIndex++, f_nclobs[i], hashMap);				// f_nclob

			// INT 配列を getObject
			assertGetObject(rs, columnIndex++, af_ints[i], hashMap);				// af_int

			// BIGINT 配列を getObject
			assertGetObject(rs, columnIndex++, af_bigints[i], hashMap);			// af_bigint
			assertGetObject(rs, columnIndex++, af_decimals[i], hashMap);			// af_decimal

			// CHAR 配列を getObject
			assertGetObject(rs, columnIndex++, af_char8s[i], hashMap);				// af_char8

			// FLOAT 配列を getObject
			assertGetObject(rs, columnIndex++, af_floats[i], hashMap);				// af_float

			// DATETIME 配列を getObject
			assertGetObject(rs, columnIndex++, af_datetimes[i], hashMap);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getObject
			assertGetObject(rs, columnIndex++, af_ids[i], hashMap);					// af_id

			// IMAGE 配列を getObject
			assertGetObject(rs, columnIndex++, af_images[i], hashMap);				// af_image

			// LANGUAGE 配列を getObject
			assertGetObject(rs, columnIndex++, af_languages[i], hashMap);			// af_language

			// NCHAR 配列を getObject
			assertGetObject(rs, columnIndex++, af_nchar6s[i], hashMap);				// af_nchar6

			// NVARCHAR 配列を getObject
			assertGetObject(rs, columnIndex++, af_nvarchar256s[i], hashMap);		// af_nvarchar256

			// VARCHAR 配列を getObject
			assertGetObject(rs, columnIndex++, af_varchar128s[i], hashMap);			// af_varchar128

			// NTEXT 配列を getObject
			assertGetObject(rs, columnIndex++, af_ntexts[i], hashMap);				// af_ntext

			// NTEXT (compressed) 配列を getObject
			assertGetObject(rs, columnIndex++, af_ntext_compresseds[i], hashMap);	// af_ntext_compressed

			// FULLTEXT 配列を getObject
			assertGetObject(rs, columnIndex++, af_fulltexts[i], hashMap);			// af_fulltext

			// BINARY 配列を getObject
			assertGetObject(rs, columnIndex++, af_binary50s[i], hashMap);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getObject(String, java.util.Map) のテスト
	public void test_getObject4() throws Exception
	{
		// Map は無視されるはず
		java.util.HashMap	hashMap = new java.util.HashMap();

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			// INT (not null) 列を getObject
			assertGetObject(rs, "f_int_not_null", f_int_not_nulls[i]);

			// INT (primary key) 列を getObject
			assertGetObject(rs, "f_int1", f_int1s[i], hashMap);

			// INT 列を getObject
			assertGetObject(rs, "f_int2", f_int2s[i], hashMap);

			// BIGINT 列を getObject
			assertGetObject(rs, "f_bigint", f_bigints[i], hashMap);
			assertGetObject(rs, "f_decimal", f_decimals[i], hashMap);

			// CHAR (not null) 列を getObject
			assertGetObject(rs, "f_char8_not_null", f_char8_not_nulls[i], hashMap);

			// CHAR 列を getObject
			assertGetObject(rs, "f_char8", f_char8s[i], hashMap);

			// FLOAT 列を getObject
			assertGetObject(rs, "f_float", f_floats[i], hashMap);

			// DATETIME 列を getObject
			assertGetObject(rs, "f_datetime", f_datetimes[i], hashMap);

			// UNIQUEIDENTIFIER 列を getObject
			assertGetObject(rs, "f_id", f_ids[i], hashMap);

			// IMAGE 列を getObject
			assertGetObject(rs, "f_image", f_images[i], hashMap);

			// LANGUAGE 列を getObject
			assertGetObject(rs, "f_language", f_languages[i], hashMap);

			// NCHAR 列を getObject
			assertGetObject(rs, "f_nchar6", f_nchar6s[i], hashMap);

			// NVARCHAR 列を getObject
			assertGetObject(rs, "f_nvarchar256", f_nvarchar256s[i], hashMap);

			// VARCHAR 列を getObject
			assertGetObject(rs, "f_varchar128", f_varchar128s[i], hashMap);

			// NTEXT 列を getObject
			assertGetObject(rs, "f_ntext", f_ntexts[i], hashMap);

			// NTEXT (compressed) 列を getObject
			assertGetObject(rs, "f_ntext_compressed", f_ntext_compresseds[i], hashMap);

			// FULLTEXT 列を getObject
			assertGetObject(rs, "f_fulltext", f_fulltexts[i], hashMap);

			// BINARY 列を getObject
			assertGetObject(rs, "f_binary50", f_binary50s[i], hashMap);

			// BLOB 列を getObject
			assertGetObject(rs, "f_blob", f_blobs[i], hashMap);

			// NCLOB 列を getObject
			assertGetObject(rs, "f_nclob", f_nclobs[i], hashMap);

			// INT 配列を getObject
			assertGetObject(rs, "af_int", af_ints[i], hashMap);

			// BIGINT 配列を getObject
			assertGetObject(rs, "af_bigint", af_bigints[i], hashMap);
			assertGetObject(rs, "af_decimal", af_decimals[i], hashMap);

			// CHAR 配列を getObject
			assertGetObject(rs, "af_char8", af_char8s[i], hashMap);

			// FLOAT 配列を getObject
			assertGetObject(rs, "af_float", af_floats[i], hashMap);

			// DATETIME 配列を getObject
			assertGetObject(rs, "af_datetime", af_datetimes[i], hashMap);

			// UNIQUEIDENTIFIER 配列を getObject
			assertGetObject(rs, "af_id", af_ids[i], hashMap);

			// IMAGE 配列を getObject
			assertGetObject(rs, "af_image", af_images[i], hashMap);

			// LANGUAGE 配列を getObject
			assertGetObject(rs, "af_language", af_languages[i], hashMap);

			// NCHAR 配列を getObject
			assertGetObject(rs, "af_nchar6", af_nchar6s[i], hashMap);

			// NVARCHAR 配列を getObject
			assertGetObject(rs, "af_nvarchar256", af_nvarchar256s[i], hashMap);

			// VARCHAR 配列を getObject
			assertGetObject(rs, "af_varchar128", af_varchar128s[i], hashMap);

			// NTEXT 配列を getObject
			assertGetObject(rs, "af_ntext", af_ntexts[i], hashMap);

			// NTEXT (compressed) 配列を getObject
			assertGetObject(rs, "af_ntext_compressed", af_ntext_compresseds[i], hashMap);

			// FULLTEXT 配列を getObject
			assertGetObject(rs, "af_fulltext", af_fulltexts[i], hashMap);

			// BINARY 配列を getObject
			assertGetObject(rs, "af_binary50", af_binary50s[i], hashMap);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getShort(int) のテスト
	public void test_getShort1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			int	columnIndex = 1;

			// INT (not null) 列を getShort
			assertGetShort(rs, columnIndex++, f_int_not_nulls[i]);			// f_int_not_null

			// INT (primary key) 列を getShort
			assertGetShort(rs, columnIndex++, f_int1s[i]);					// f_int1

			// INT 列を getShort
			assertGetShort(rs, columnIndex++, f_int2s[i]);					// f_int2
			
			// BIGINT 列を getShort
			assertGetShort(rs, columnIndex++, f_bigints[i]);				// f_bigint
			assertGetShort(rs, columnIndex++, f_decimals[i]);				// f_decimal

			// CHAR (not null) 列を getShort
			e = checkParseShort(f_char8_not_nulls[i]);
			assertGetShort(rs, columnIndex++, f_char8_not_nulls[i], e);		// f_char8_not_null

			// CHAR 列を getShort
			e = checkParseShort(f_char8s[i]);
			assertGetShort(rs, columnIndex++, f_char8s[i], e);				// f_char8

			// FLOAT 列を getShort
			assertGetShort(rs, columnIndex++, f_floats[i]);					// f_float

			// DATETIME 列を getShort
			assertGetShort(rs, columnIndex++, f_datetimes[i], e);			// f_datetime

			// UNIQUEIDENTIFIER 列を getShort
			assertGetShort(rs, columnIndex++, f_ids[i], e);					// f_id

			// IMAGE 列を getShort
			assertGetShort(rs, columnIndex++, f_images[i], e);				// f_image

			// LANGUAGE 列を getShort
			assertGetShort(rs, columnIndex++, f_languages[i], e);			// f_language

			// NCHAR 列を getShort
			e = checkParseShort(f_nchar6s[i]);
			assertGetShort(rs, columnIndex++, f_nchar6s[i], e);				// f_nchar6

			// NVARCHAR 列を getShort
			e = checkParseShort(f_nvarchar256s[i]);
			assertGetShort(rs, columnIndex++, f_nvarchar256s[i], e);		// f_nvarchar256

			// VARCHAR 列を getShort
			e = checkParseShort(f_varchar128s[i]);
			assertGetShort(rs, columnIndex++, f_varchar128s[i], e);			// f_varchar128

			// NTEXT 列を getShort
			e = checkParseShort(f_ntexts[i]);
			assertGetShort(rs, columnIndex++, f_ntexts[i], e);				// f_ntext

			// NTEXT (compressed) 列を getShort
			e = checkParseShort(f_ntext_compresseds[i]);
			assertGetShort(rs, columnIndex++, f_ntext_compresseds[i], e);	// f_ntext_compressed

			// FULLTEXT 列を getShort
			e = checkParseShort(f_fulltexts[i]);
			assertGetShort(rs, columnIndex++, f_fulltexts[i], e);			// f_fulltext

			// BINARY 列を getShort
			e = classCast;
			assertGetShort(rs, columnIndex++, f_binary50s[i], e);			// f_binary50

			// BLOB 列を getShort
			assertGetShort(rs, columnIndex++, f_blobs[i], e);				// f_blob

			// NCLOB 列を getShort
			e = checkParseShort(f_nclobs[i]);
			assertGetShort(rs, columnIndex++, f_nclobs[i], e);				// f_nclob

			// INT 配列を getShort
			e = classCast;
			assertGetShort(rs, columnIndex++, af_ints[i], e);				// af_int

			// BIGINT 配列を getShort
			assertGetShort(rs, columnIndex++, af_bigints[i], e);		// af_bigint
			assertGetShort(rs, columnIndex++, af_decimals[i], e);		// af_decimal

			// CHAR 配列を getShort
			assertGetShort(rs, columnIndex++, af_char8s[i], e);				// af_char8

			// FLOAT 配列を getShort
			assertGetShort(rs, columnIndex++, af_floats[i], e);				// af_float

			// DATETIME 配列を getShort
			assertGetShort(rs, columnIndex++, af_datetimes[i], e);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getShort
			assertGetShort(rs, columnIndex++, af_ids[i], e);				// af_id

			// IMAGE 配列を getShort
			assertGetShort(rs, columnIndex++, af_images[i], e);				// af_image

			// LANGUAGE 配列を getShort
			assertGetShort(rs, columnIndex++, af_languages[i], e);			// af_language

			// NCHAR 配列を getShort
			assertGetShort(rs, columnIndex++, af_nchar6s[i], e);			// af_nchar6

			// NVARCHAR 配列を getShort
			assertGetShort(rs, columnIndex++, af_nvarchar256s[i], e);		// af_nvarchar256

			// VARCHAR 配列を getShort
			assertGetShort(rs, columnIndex++, af_varchar128s[i], e);		// af_varchar128

			// NTEXT 配列を getShort
			assertGetShort(rs, columnIndex++, af_ntexts[i], e);				// af_ntext

			// NTEXT (compressed) 配列を getShort
			assertGetShort(rs, columnIndex++, af_ntext_compresseds[i], e);	// af_ntext_compressed

			// FULLTEXT 配列を getShort
			assertGetShort(rs, columnIndex++, af_fulltexts[i], e);			// af_fulltext

			// BINARY 配列を getShort
			assertGetShort(rs, columnIndex++, af_binary50s[i], e);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getShort(String) のテスト
	public void test_getShort2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			// INT (not null) 列を getShort
			assertGetShort(rs, "f_int_not_null", f_int_not_nulls[i]);

			// INT (primary key) 列を getShort
			assertGetShort(rs, "f_int1", f_int1s[i]);

			// INT 列を getShort
			assertGetShort(rs, "f_int2", f_int2s[i]);

			// BIGINT 列を getShort
			assertGetShort(rs, "f_bigint", f_bigints[i]);
			assertGetShort(rs, "f_decimal", f_decimals[i]);

			// CHAR (not null) 列を getShort
			e = checkParseShort(f_char8_not_nulls[i]);
			assertGetShort(rs, "f_char8_not_null", f_char8_not_nulls[i], e);

			// CHAR 列を getShort
			e = checkParseShort(f_char8s[i]);
			assertGetShort(rs, "f_char8", f_char8s[i], e);

			// FLOAT 列を getShort
			assertGetShort(rs, "f_float", f_floats[i]);

			// DATETIME 列を getShort
			e = classCast;
			assertGetShort(rs, "f_datetime", f_datetimes[i], e);

			// UNIQUEIDENTIFIER 列を getShort
			assertGetShort(rs, "f_id", f_ids[i], e);

			// IMAGE 列を getShort
			assertGetShort(rs, "f_image", f_images[i], e);

			// LANGUAGE 列を getShort
			assertGetShort(rs, "f_language", f_languages[i], e);

			// NCHAR 列を getShort
			e = checkParseShort(f_nchar6s[i]);
			assertGetShort(rs, "f_nchar6", f_nchar6s[i], e);

			// NVARCHAR 列を getShort
			e = checkParseShort(f_nvarchar256s[i]);
			assertGetShort(rs, "f_nvarchar256", f_nvarchar256s[i], e);

			// VARCHAR 列を getShort
			e = checkParseShort(f_varchar128s[i]);
			assertGetShort(rs, "f_varchar128", f_varchar128s[i], e);

			// NTEXT 列を getShort
			e = checkParseShort(f_ntexts[i]);
			assertGetShort(rs, "f_ntext", f_ntexts[i], e);

			// NTEXT (compressed) 列を getShort
			e = checkParseShort(f_ntext_compresseds[i]);
			assertGetShort(rs, "f_ntext_compressed", f_ntext_compresseds[i], e);

			// FULLTEXT 列を getShort
			e = checkParseShort(f_fulltexts[i]);
			assertGetShort(rs, "f_fulltext", f_fulltexts[i], e);

			// BINARY 列を getShort
			e = classCast;
			assertGetShort(rs, "f_binary50", f_binary50s[i], e);

			// BLOB 列を getShort
			assertGetShort(rs, "f_blob", f_blobs[i], e);

			// NCLOB 列を getShort
			e = checkParseShort(f_nclobs[i]);
			assertGetShort(rs, "f_nclob", f_nclobs[i], e);

			// INT 配列を getShort
			e = classCast;
			assertGetShort(rs, "af_int", af_ints[i], e);

			// BIGINT 配列を getShort
			assertGetShort(rs, "af_bigint", af_bigints[i], e);
			assertGetShort(rs, "af_decimal", af_decimals[i], e);

			// CHAR 配列を getShort
			assertGetShort(rs, "af_char8", af_char8s[i], e);

			// FLOAT 配列を getShort
			assertGetShort(rs, "af_float", af_floats[i], e);

			// DATETIME 配列を getShort
			assertGetShort(rs, "af_datetime", af_datetimes[i], e);

			// UNIQUEIDENTIFIER 配列を getShort
			assertGetShort(rs, "af_id", af_ids[i], e);

			// IMAGE 配列を getShort
			assertGetShort(rs, "af_image", af_images[i], e);

			// LANGUAGE 配列を getShort
			assertGetShort(rs, "af_language", af_languages[i], e);

			// NCHAR 配列を getShort
			assertGetShort(rs, "af_nchar6", af_nchar6s[i], e);

			// NVARCHAR 配列を getShort
			assertGetShort(rs, "af_nvarchar256", af_nvarchar256s[i], e);

			// VARCHAR 配列を getShort
			assertGetShort(rs, "af_varchar128", af_varchar128s[i], e);

			// NTEXT 配列を getShort
			assertGetShort(rs, "af_ntext", af_ntexts[i], e);

			// NTEXT (compressed) 配列を getShort
			assertGetShort(rs, "af_ntext_compressed", af_ntext_compresseds[i], e);

			// FULLTEXT 配列を getShort
			assertGetShort(rs, "af_fulltext", af_fulltexts[i], e);

			// BINARY 配列を getShort
			assertGetShort(rs, "af_binary50", af_binary50s[i], e);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getStatement() のテスト
	public void test_getStatement() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		int[]		ids =		{	1,			2,			3		};
		String[]	titles =	{	"RICOH",	"Sydney",	"Java"	};

		// 下準備
		createSimpleTable(c, ids, titles);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		assertEquals(s, rs.getStatement());

		rs.close();
		s.close();

		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("select * from t where id = ?"));
		ps.setInt(1, 2);
		rs = ps.executeQuery();

		assertEquals(ps, rs.getStatement());

		rs.close();
		ps.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getString(int) のテスト
	public void test_getString1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	e = new ClassCast();

			int	columnIndex = 1;

			// INT (not null) 列を getString
			assertGetString(rs, columnIndex++, f_int_not_nulls[i]);			// f_int_not_null

			// INT (primary key) 列を getString
			assertGetString(rs, columnIndex++, f_int1s[i]);					// f_int1

			// INT 列を getString
			assertGetString(rs, columnIndex++, f_int2s[i]);					// f_int2

			// BIGINT 列を getString
			assertGetString(rs, columnIndex++, f_bigints[i]);			// f_bigint
			assertGetString(rs, columnIndex++, f_decimals[i]);			// f_decimal

			// CHAR (not null) 列を getString
			assertGetString(rs, columnIndex++, f_char8_not_nulls[i]);		// f_char8_not_null

			// CHAR 列を getString
			assertGetString(rs, columnIndex++, f_char8s[i]);				// f_char8

			// FLOAT 列を getString
			assertGetString(rs, columnIndex++, f_floats[i]);				// f_float

			// DATETIME 列を getString
			assertGetString(rs, columnIndex++, f_datetimes[i]);				// f_datetime

			// UNIQUEIDENTIFIER 列を getString
			assertGetString(rs, columnIndex++, f_ids[i]);					// f_id

			// IMAGE 列を getString
			assertGetString(rs, columnIndex++, f_images[i]);				// f_image

			// LANGUAGE 列を getString
			assertGetString(rs, columnIndex++, f_languages[i]);				// f_language

			// NCHAR 列を getString
			assertGetString(rs, columnIndex++, f_nchar6s[i]);				// f_nchar6

			// NVARCHAR 列を getString
			assertGetString(rs, columnIndex++, f_nvarchar256s[i]);			// f_nvarchar256

			// VARCHAR 列を getString
			assertGetString(rs, columnIndex++, f_varchar128s[i]);			// f_varchar128

			// NTEXT 列を getString
			assertGetString(rs, columnIndex++, f_ntexts[i]);				// f_ntext

			// NTEXT (compressed) 列を getString
			assertGetString(rs, columnIndex++, f_ntext_compresseds[i]);		// f_ntext_compressed

			// FULLTEXT 列を getString
			assertGetString(rs, columnIndex++, f_fulltexts[i]);				// f_fulltext

			// BINARY 列を getString
			assertGetString(rs, columnIndex++, f_binary50s[i]);				// f_binary50

			// BLOB 列を getString
			assertGetString(rs, columnIndex++, f_blobs[i]);					// f_blob

			// NCLOB 列を getString
			assertGetString(rs, columnIndex++, f_nclobs[i]);				// f_nclob

			// INT 配列を getString
			assertGetString(rs, columnIndex++, af_ints[i], e);				// af_int

			// BIGINT 配列を getString
			assertGetString(rs, columnIndex++, af_bigints[i], e);		// af_bigint
			assertGetString(rs, columnIndex++, af_decimals[i], e);		// af_decimal

			// CHAR 配列を getString
			assertGetString(rs, columnIndex++, af_char8s[i], e);			// af_char8

			// FLOAT 配列を getString
			assertGetString(rs, columnIndex++, af_floats[i], e);			// af_float

			// DATETIME 配列を getString
			assertGetString(rs, columnIndex++, af_datetimes[i], e);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getString
			assertGetString(rs, columnIndex++, af_ids[i], e);				// af_id

			// IMAGE 配列を getString
			assertGetString(rs, columnIndex++, af_images[i], e);			// af_image

			// LANGUAGE 配列を getString
			assertGetString(rs, columnIndex++, af_languages[i], e);			// af_language

			// NCHAR 配列を getString
			assertGetString(rs, columnIndex++, af_nchar6s[i], e);			// af_nchar6

			// NVARCHAR 配列を getString
			assertGetString(rs, columnIndex++, af_nvarchar256s[i], e);		// af_nvarchar256

			// VARCHAR 配列を getString
			assertGetString(rs, columnIndex++, af_varchar128s[i], e);		// af_varchar128

			// NTEXT 配列を getString
			assertGetString(rs, columnIndex++, af_ntexts[i], e);			// af_ntext

			// NTEXT (compressed) 配列を getString
			assertGetString(rs, columnIndex++, af_ntext_compresseds[i], e);	// af_ntext_compressed

			// FULLTEXT 配列を getString
			assertGetString(rs, columnIndex++, af_fulltexts[i], e);			// af_fulltext

			// BINARY 配列を getString
			assertGetString(rs, columnIndex++, af_binary50s[i], e);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getString(String) のテスト
	public void test_getString2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	e = new ClassCast();

			// INT (not null) 列を getString
			assertGetString(rs, "f_int_not_null", f_int_not_nulls[i]);

			// INT (primary key) 列を getString
			assertGetString(rs, "f_int1", f_int1s[i]);

			// INT 列を getString
			assertGetString(rs, "f_int2", f_int2s[i]);

			// BIGINT 列を getString
			assertGetString(rs, "f_bigint", f_bigints[i]);
			assertGetString(rs, "f_decimal", f_decimals[i]);

			// CHAR (not null) 列を getString
			assertGetString(rs, "f_char8_not_null", f_char8_not_nulls[i]);

			// CHAR 列を getString
			assertGetString(rs, "f_char8", f_char8s[i]);

			// FLOAT 列を getString
			assertGetString(rs, "f_float", f_floats[i]);

			// DATETIME 列を getString
			assertGetString(rs, "f_datetime", f_datetimes[i]);

			// UNIQUEIDENTIFIER 列を getString
			assertGetString(rs, "f_id", f_ids[i]);

			// IMAGE 列を getString
			assertGetString(rs, "f_image", f_images[i]);

			// LANGUAGE 列を getString
			assertGetString(rs, "f_language", f_languages[i]);

			// NCHAR 列を getString
			assertGetString(rs, "f_nchar6", f_nchar6s[i]);

			// NVARCHAR 列を getString
			assertGetString(rs, "f_nvarchar256", f_nvarchar256s[i]);

			// VARCHAR 列を getString
			assertGetString(rs, "f_varchar128", f_varchar128s[i]);

			// NTEXT 列を getString
			assertGetString(rs, "f_ntext", f_ntexts[i]);

			// NTEXT (compressed) 列を getString
			assertGetString(rs, "f_ntext_compressed", f_ntext_compresseds[i]);

			// FULLTEXT 列を getString
			assertGetString(rs, "f_fulltext", f_fulltexts[i]);

			// BINARY 列を getString
			assertGetString(rs, "f_binary50", f_binary50s[i]);

			// BLOB 列を getString
			assertGetString(rs, "f_blob", f_blobs[i]);

			// NCLOB 列を getString
			assertGetString(rs, "f_nclob", f_nclobs[i]);

			// INT 配列を getString
			assertGetString(rs, "af_int", af_ints[i], e);

			// BIGINT 配列を getString
			assertGetString(rs, "af_bigint", af_bigints[i], e);
			assertGetString(rs, "af_decimal", af_decimals[i], e);

			// CHAR 配列を getString
			assertGetString(rs, "af_char8", af_char8s[i], e);

			// FLOAT 配列を getString
			assertGetString(rs, "af_float", af_floats[i], e);

			// DATETIME 配列を getString
			assertGetString(rs, "af_datetime", af_datetimes[i], e);

			// UNIQUEIDENTIFIER 配列を getString
			assertGetString(rs, "af_id", af_ids[i], e);

			// IMAGE 配列を getString
			assertGetString(rs, "af_image", af_images[i], e);

			// LANGUAGE 配列を getString
			assertGetString(rs, "af_language", af_languages[i], e);

			// NCHAR 配列を getString
			assertGetString(rs, "af_nchar6", af_nchar6s[i], e);

			// NVARCHAR 配列を getString
			assertGetString(rs, "af_nvarchar256", af_nvarchar256s[i], e);

			// VARCHAR 配列を getString
			assertGetString(rs, "af_varchar128", af_varchar128s[i], e);

			// NTEXT 配列を getString
			assertGetString(rs, "af_ntext", af_ntexts[i], e);

			// NTEXT (compressed) 配列を getString
			assertGetString(rs, "af_ntext_compressed", af_ntext_compresseds[i], e);

			// FULLTEXT 配列を getString
			assertGetString(rs, "af_fulltext", af_fulltexts[i], e);

			// BINARY 配列を getString
			assertGetString(rs, "af_binary50", af_binary50s[i], e);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getTime(int) のテスト
	public void test_getTime1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			int	columnIndex = 1;

			// INT (not null) 列を getTime
			assertGetTime(rs, columnIndex++, f_int_not_nulls[i], e);		// f_int_not_null

			// INT (primary key) 列を getTime
			assertGetTime(rs, columnIndex++, f_int1s[i], e);				// f_int1

			// INT 列を getTime
			assertGetTime(rs, columnIndex++, f_int2s[i], e);				// f_int2

			// BIGINT 列を getTime
			assertGetTime(rs, columnIndex++, f_bigints[i], e);			// f_bigint
			assertGetTime(rs, columnIndex++, f_decimals[i], e);			// f_decimal

			// CHAR (not null) 列を getTime
			e = checkParseTime(f_char8_not_nulls[i]);
			assertGetTime(rs, columnIndex++, f_char8_not_nulls[i], e);		// f_char8_not_null

			// CHAR 列を getTime
			e = checkParseTime(f_char8s[i]);
			assertGetTime(rs, columnIndex++, f_char8s[i], e);				// f_char8

			// FLOAT 列を getTime
			e = classCast;
			assertGetTime(rs, columnIndex++, f_floats[i], e);				// f_float

			// DATETIME 列を getTime
			assertGetTime(rs, columnIndex++, f_datetimes[i]);				// f_datetime

			// UNIQUEIDENTIFIER 列を getTime
			assertGetTime(rs, columnIndex++, f_ids[i], e);					// f_id

			// IMAGE 列を getTime
			assertGetTime(rs, columnIndex++, f_images[i], e);				// f_image

			// LANGUAGE 列を getTime
			assertGetTime(rs, columnIndex++, f_languages[i], e);			// f_language

			// NCHAR 列を getTime
			assertGetTime(rs, columnIndex++, f_nchar6s[i], e);				// f_nchar6

			// NVARCHAR 列を getTime
			e = checkParseTime(f_nvarchar256s[i]);
			assertGetTime(rs, columnIndex++, f_nvarchar256s[i], e);			// f_nvarchar256

			// VARCHAR 列を getTime
			e = checkParseTime(f_varchar128s[i]);
			assertGetTime(rs, columnIndex++, f_varchar128s[i], e);			// f_varchar128

			// NTEXT 列を getTime
			e = checkParseTime(f_ntexts[i]);
			assertGetTime(rs, columnIndex++, f_ntexts[i], e);				// f_ntext

			// NTEXT (compressed) 列を getTime
			e = checkParseTime(f_ntext_compresseds[i]);
			assertGetTime(rs, columnIndex++, f_ntext_compresseds[i], e);	// f_ntext_compressed

			// FULLTEXT 列を getTime
			e = checkParseTime(f_fulltexts[i]);
			assertGetTime(rs, columnIndex++, f_fulltexts[i], e);			// f_fulltext

			// BINARY 列を getTime
			e = classCast;
			assertGetTime(rs, columnIndex++, f_binary50s[i], e);			// f_binary50

			// BLOB 列を getTime
			assertGetTime(rs, columnIndex++, f_blobs[i], e);				// f_blob

			// NCLOB 列を getTime
			e = checkParseTime(f_nclobs[i]);
			assertGetTime(rs, columnIndex++, f_nclobs[i], e);				// f_nclob

			// INT 配列を getTime
			e = classCast;
			assertGetTime(rs, columnIndex++, af_ints[i], e);				// af_int

			// BIGINT 配列を getTime
			assertGetTime(rs, columnIndex++, af_bigints[i], e);			// af_bigint
			assertGetTime(rs, columnIndex++, af_decimals[i], e);			// af_decimal

			// CHAR 配列を getTime
			assertGetTime(rs, columnIndex++, af_char8s[i], e);				// af_char8

			// FLOAT 配列を getTime
			assertGetTime(rs, columnIndex++, af_floats[i], e);				// af_float

			// DATETIME 配列を getTime
			assertGetTime(rs, columnIndex++, af_datetimes[i], e);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getTime
			assertGetTime(rs, columnIndex++, af_ids[i], e);					// af_id

			// IMAGE 配列を getTime
			assertGetTime(rs, columnIndex++, af_images[i], e);				// af_image

			// LANGUAGE 配列を getTime
			assertGetTime(rs, columnIndex++, af_languages[i], e);			// af_language

			// NCHAR 配列を getTime
			assertGetTime(rs, columnIndex++, af_nchar6s[i], e);				// af_nchar6

			// NVARCHAR 配列を getTime
			assertGetTime(rs, columnIndex++, af_nvarchar256s[i], e);		// af_nvarchar256

			// VARCHAR 配列を getTime
			assertGetTime(rs, columnIndex++, af_varchar128s[i], e);			// af_varchar128

			// NTEXT 配列を getTime
			assertGetTime(rs, columnIndex++, af_ntexts[i], e);				// af_ntext

			// NTEXT (compressed) 配列を getTime
			assertGetTime(rs, columnIndex++, af_ntext_compresseds[i], e);	// af_ntext_compressed

			// FULLTEXT 配列を getTime
			assertGetTime(rs, columnIndex++, af_fulltexts[i], e);			// af_fulltext

			// BINARY 配列を getTime
			assertGetTime(rs, columnIndex++, af_binary50s[i], e);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getTime(String) のテスト
	public void test_getTime2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			// INT (not null) 列を getTime
			assertGetTime(rs, "f_int_not_null", f_int_not_nulls[i], e);

			// INT (primary key) 列を getTime
			assertGetTime(rs, "f_int1", f_int1s[i], e);

			// INT 列を getTime
			assertGetTime(rs, "f_int2", f_int2s[i], e);

			// BIGINT 列を getTime
			assertGetTime(rs, "f_bigint", f_bigints[i], e);
			assertGetTime(rs, "f_decimal", f_decimals[i], e);

			// CHAR (not null) 列を getTime
			e = checkParseTime(f_char8_not_nulls[i]);
			assertGetTime(rs, "f_char8_not_null", f_char8_not_nulls[i], e);

			// CHAR 列を getTime
			e = checkParseTime(f_char8s[i]);
			assertGetTime(rs, "f_char8", f_char8s[i], e);

			// FLOAT 列を getTime
			e = classCast;
			assertGetTime(rs, "f_float", f_floats[i], e);

			// DATETIME 列を getTime
			assertGetTime(rs, "f_datetime", f_datetimes[i]);

			// UNIQUEIDENTIFIER 列を getTime
			assertGetTime(rs, "f_id", f_ids[i], e);

			// IMAGE 列を getTime
			assertGetTime(rs, "f_image", f_images[i], e);

			// LANGUAGE 列を getTime
			assertGetTime(rs, "f_language", f_languages[i], e);

			// NCHAR 列を getTime
			assertGetTime(rs, "f_nchar6", f_nchar6s[i], e);

			// NVARCHAR 列を getTime
			e = checkParseTime(f_nvarchar256s[i]);
			assertGetTime(rs, "f_nvarchar256", f_nvarchar256s[i], e);

			// VARCHAR 列を getTime
			e = checkParseTime(f_varchar128s[i]);
			assertGetTime(rs, "f_varchar128", f_varchar128s[i], e);

			// NTEXT 列を getTime
			e = checkParseTime(f_ntexts[i]);
			assertGetTime(rs, "f_ntext", f_ntexts[i], e);

			// NTEXT (compressed) 列を getTime
			e = checkParseTime(f_ntext_compresseds[i]);
			assertGetTime(rs, "f_ntext_compressed", f_ntext_compresseds[i], e);

			// FULLTEXT 列を getTime
			e = checkParseTime(f_fulltexts[i]);
			assertGetTime(rs, "f_fulltext", f_fulltexts[i], e);

			// BINARY 列を getTime
			e = classCast;
			assertGetTime(rs, "f_binary50", f_binary50s[i], e);

			// BLOB 列を getTime
			assertGetTime(rs, "f_blob", f_blobs[i], e);

			// NCLOB 列を getTime
			e = checkParseTime(f_nclobs[i]);
			assertGetTime(rs, "f_nclob", f_nclobs[i], e);

			// INT 配列を getTime
			e = classCast;
			assertGetTime(rs, "af_int", af_ints[i], e);

			// BIGINT 配列を getTime
			assertGetTime(rs, "af_bigint", af_bigints[i], e);
			assertGetTime(rs, "af_decimal", af_decimals[i], e);

			// CHAR 配列を getTime
			assertGetTime(rs, "af_char8", af_char8s[i], e);

			// FLOAT 配列を getTime
			assertGetTime(rs, "af_float", af_floats[i], e);

			// DATETIME 配列を getTime
			assertGetTime(rs, "af_datetime", af_datetimes[i], e);

			// UNIQUEIDENTIFIER 配列を getTime
			assertGetTime(rs, "af_id", af_ids[i], e);

			// IMAGE 配列を getTime
			assertGetTime(rs, "af_image", af_images[i], e);

			// LANGUAGE 配列を getTime
			assertGetTime(rs, "af_language", af_languages[i], e);

			// NCHAR 配列を getTime
			assertGetTime(rs, "af_nchar6", af_nchar6s[i], e);

			// NVARCHAR 配列を getTime
			assertGetTime(rs, "af_nvarchar256", af_nvarchar256s[i], e);

			// VARCHAR 配列を getTime
			assertGetTime(rs, "af_varchar128", af_varchar128s[i], e);

			// NTEXT 配列を getTime
			assertGetTime(rs, "af_ntext", af_ntexts[i], e);

			// NTEXT (compressed) 配列を getTime
			assertGetTime(rs, "af_ntext_compressed", af_ntext_compresseds[i], e);

			// FULLTEXT 配列を getTime
			assertGetTime(rs, "af_fulltext", af_fulltexts[i], e);

			// BINARY 配列を getTime
			assertGetTime(rs, "af_binary50", af_binary50s[i], e);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getTimestamp(int) のテスト
	public void test_getTimestamp1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			int	columnIndex = 1;

			// INT (not null) 列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, f_int_not_nulls[i], e);		// f_int_not_null

			// INT (primary key) 列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, f_int1s[i], e);				// f_int1

			// INT 列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, f_int2s[i], e);				// f_int2

			// BIGINT 列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, f_bigints[i], e);			// f_bigint
			assertGetTimestamp(rs, columnIndex++, f_decimals[i], e);			// f_decimal

			// CHAR (not null) 列を getTimestamp
			e = checkParseTimestamp(f_char8_not_nulls[i]);
			assertGetTimestamp(rs, columnIndex++, f_char8_not_nulls[i], e);		// f_char8_not_null

			// CHAR 列を getTimestamp
			e = checkParseTimestamp(f_char8s[i]);
			assertGetTimestamp(rs, columnIndex++, f_char8s[i], e);				// f_char8

			// FLOAT 列を getTimestamp
			e = classCast;
			assertGetTimestamp(rs, columnIndex++, f_floats[i], e);				// f_float

			// DATETIME 列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, f_datetimes[i]);				// f_datetime

			// UNIQUEIDENTIFIER 列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, f_ids[i], e);					// f_id

			// IMAGE 列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, f_images[i], e);				// f_image

			// LANGUAGE 列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, f_languages[i], e);			// f_language

			// NCHAR 列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, f_nchar6s[i], e);				// f_nchar6

			// NVARCHAR 列を getTimestamp
			e = checkParseTimestamp(f_nvarchar256s[i]);
			assertGetTimestamp(rs, columnIndex++, f_nvarchar256s[i], e);		// f_nvarchar256

			// VARCHAR 列を getTimestamp
			e = checkParseTimestamp(f_varchar128s[i]);
			assertGetTimestamp(rs, columnIndex++, f_varchar128s[i], e);			// f_varchar128

			// NTEXT 列を getTimestamp
			e = checkParseTimestamp(f_ntexts[i]);
			assertGetTimestamp(rs, columnIndex++, f_ntexts[i], e);				// f_ntext

			// NTEXT (compressed) 列を getTimestamp
			e = checkParseTimestamp(f_ntext_compresseds[i]);
			assertGetTimestamp(rs, columnIndex++, f_ntext_compresseds[i], e);	// f_ntext_compressed

			// FULLTEXT 列を getTimestamp
			e = checkParseTimestamp(f_fulltexts[i]);
			assertGetTimestamp(rs, columnIndex++, f_fulltexts[i], e);			// f_fulltext

			// BINARY 列を getTimestamp
			e = classCast;
			assertGetTimestamp(rs, columnIndex++, f_binary50s[i], e);			// f_binary50

			// BLOB 列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, f_blobs[i], e);				// f_blob

			// NCLOB 列を getTimestamp
			e = checkParseTimestamp(f_nclobs[i]);
			assertGetTimestamp(rs, columnIndex++, f_nclobs[i], e);				// f_nclob

			// INT 配列を getTimestamp
			e = classCast;
			assertGetTimestamp(rs, columnIndex++, af_ints[i], e);				// af_int

			// BIGINT 配列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, af_bigints[i], e);		// af_bigint
			assertGetTimestamp(rs, columnIndex++, af_decimals[i], e);		// af_decimal

			// CHAR 配列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, af_char8s[i], e);				// af_char8

			// FLOAT 配列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, af_floats[i], e);				// af_float

			// DATETIME 配列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, af_datetimes[i], e);			// af_datetime

			// UNIQUEIDENTIFIER 配列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, af_ids[i], e);				// af_id

			// IMAGE 配列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, af_images[i], e);				// af_image

			// LANGUAGE 配列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, af_languages[i], e);			// af_language

			// NCHAR 配列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, af_nchar6s[i], e);			// af_nchar6

			// NVARCHAR 配列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, af_nvarchar256s[i], e);		// af_nvarchar256

			// VARCHAR 配列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, af_varchar128s[i], e);		// af_varchar128

			// NTEXT 配列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, af_ntexts[i], e);				// af_ntext

			// NTEXT (compressed) 配列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, af_ntext_compresseds[i], e);	// af_ntext_compressed

			// FULLTEXT 配列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, af_fulltexts[i], e);			// af_fulltext

			// BINARY 配列を getTimestamp
			assertGetTimestamp(rs, columnIndex++, af_binary50s[i], e);			// af_binary50
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getTimestamp(String) のテスト
	public void test_getTimestamp2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		for (int i = 0; i < f_int2s.length; i++) {

			assertTrue(rs.next());

			SQLException	classCast = new ClassCast();
			SQLException	e = new ClassCast();

			// INT (not null) 列を getTimestamp
			assertGetTimestamp(rs, "f_int_not_null", f_int_not_nulls[i], e);

			// INT (primary key) 列を getTimestamp
			assertGetTimestamp(rs, "f_int1", f_int1s[i], e);

			// INT 列を getTimestamp
			assertGetTimestamp(rs, "f_int2", f_int2s[i], e);

			// BIGINT 列を getTimestamp
			assertGetTimestamp(rs, "f_bigint", f_bigints[i], e);
			assertGetTimestamp(rs, "f_decimal", f_decimals[i], e);

			// CHAR (not null) 列を getTimestamp
			e = checkParseTimestamp(f_char8_not_nulls[i]);
			assertGetTimestamp(rs, "f_char8_not_null", f_char8_not_nulls[i], e);

			// CHAR 列を getTimestamp
			e = checkParseTimestamp(f_char8s[i]);
			assertGetTimestamp(rs, "f_char8", f_char8s[i], e);

			// FLOAT 列を getTimestamp
			e = classCast;
			assertGetTimestamp(rs, "f_float", f_floats[i], e);

			// DATETIME 列を getTimestamp
			assertGetTimestamp(rs, "f_datetime", f_datetimes[i]);

			// UNIQUEIDENTIFIER 列を getTimestamp
			assertGetTimestamp(rs, "f_id", f_ids[i], e);

			// IMAGE 列を getTimestamp
			assertGetTimestamp(rs, "f_image", f_images[i], e);

			// LANGUAGE 列を getTimestamp
			assertGetTimestamp(rs, "f_language", f_languages[i], e);

			// NCHAR 列を getTimestamp
			assertGetTimestamp(rs, "f_nchar6", f_nchar6s[i], e);

			// NVARCHAR 列を getTimestamp
			e = checkParseTimestamp(f_nvarchar256s[i]);
			assertGetTimestamp(rs, "f_nvarchar256", f_nvarchar256s[i], e);

			// VARCHAR 列を getTimestamp
			e = checkParseTimestamp(f_varchar128s[i]);
			assertGetTimestamp(rs, "f_varchar128", f_varchar128s[i], e);

			// NTEXT 列を getTimestamp
			e = checkParseTimestamp(f_ntexts[i]);
			assertGetTimestamp(rs, "f_ntext", f_ntexts[i], e);

			// NTEXT (compressed) 列を getTimestamp
			e = checkParseTimestamp(f_ntext_compresseds[i]);
			assertGetTimestamp(rs, "f_ntext_compressed", f_ntext_compresseds[i], e);

			// FULLTEXT 列を getTimestamp
			e = checkParseTimestamp(f_fulltexts[i]);
			assertGetTimestamp(rs, "f_fulltext", f_fulltexts[i], e);

			// BINARY 列を getTimestamp
			e = classCast;
			assertGetTimestamp(rs, "f_binary50", f_binary50s[i], e);

			// BLOB 列を getTimestamp
			assertGetTimestamp(rs, "f_blob", f_blobs[i], e);

			// NCLOB 列を getTimestamp
			e = checkParseTimestamp(f_nclobs[i]);
			assertGetTimestamp(rs, "f_nclob", f_nclobs[i], e);

			// INT 配列を getTimestamp
			e = classCast;
			assertGetTimestamp(rs, "af_int", af_ints[i], e);

			// BIGINT 配列を getTimestamp
			assertGetTimestamp(rs, "af_bigint", af_bigints[i], e);
			assertGetTimestamp(rs, "af_decimal", af_decimals[i], e);

			// CHAR 配列を getTimestamp
			assertGetTimestamp(rs, "af_char8", af_char8s[i], e);

			// FLOAT 配列を getTimestamp
			assertGetTimestamp(rs, "af_float", af_floats[i], e);

			// DATETIME 配列を getTimestamp
			assertGetTimestamp(rs, "af_datetime", af_datetimes[i], e);

			// UNIQUEIDENTIFIER 配列を getTimestamp
			assertGetTimestamp(rs, "af_id", af_ids[i], e);

			// IMAGE 配列を getTimestamp
			assertGetTimestamp(rs, "af_image", af_images[i], e);

			// LANGUAGE 配列を getTimestamp
			assertGetTimestamp(rs, "af_language", af_languages[i], e);

			// NCHAR 配列を getTimestamp
			assertGetTimestamp(rs, "af_nchar6", af_nchar6s[i], e);

			// NVARCHAR 配列を getTimestamp
			assertGetTimestamp(rs, "af_nvarchar256", af_nvarchar256s[i], e);

			// VARCHAR 配列を getTimestamp
			assertGetTimestamp(rs, "af_varchar128", af_varchar128s[i], e);

			// NTEXT 配列を getTimestamp
			assertGetTimestamp(rs, "af_ntext", af_ntexts[i], e);

			// NTEXT (compressed) 配列を getTimestamp
			assertGetTimestamp(rs, "af_ntext_compressed", af_ntext_compresseds[i], e);

			// FULLTEXT 配列を getTimestamp
			assertGetTimestamp(rs, "af_fulltext", af_fulltexts[i], e);

			// BINARY 配列を getTimestamp
			assertGetTimestamp(rs, "af_binary50", af_binary50s[i], e);
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.getType() のテスト
	public void test_getType() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c, 1);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		// 常に java.sql.ResultSet.TYPE_FORWARD_ONLY が返るはず
		assertEquals(ResultSet.TYPE_FORWARD_ONLY, rs.getType());

		// next しても同じはず
		assertTrue(rs.next());
		assertEquals(ResultSet.TYPE_FORWARD_ONLY, rs.getType());

		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.isBeforeFirst() と ResultSet.isFirst() と
	// ResultSet.isLast() と ResultSet.isAfterLast() と ResultSet.getRow() のテスト
	public void test_checkPosition() throws Exception
	{
		for (int numTuple = 0; numTuple <= 5; numTuple++) {

			Connection	c = null;
			assertNotNull(c = getConnection());

			// 下準備
			createSimpleTable(c, numTuple);

			Statement	s = null;
			assertNotNull(s = c.createStatement());
			ResultSet	rs = null;
			assertNotNull(rs = s.executeQuery("select id from t"));

			// まだ next していない状態
			assertTrue(rs.isBeforeFirst());
			assertFalse(rs.isFirst());
			if (numTuple == 0)	assertTrue(rs.isLast());
			else				assertFalse(rs.isLast());
			assertFalse(rs.isAfterLast());
			assertEquals(0, rs.getRow());

			// 1st next

			if (numTuple == 0)	assertFalse(rs.next());
			else				assertTrue(rs.next());
			assertFalse(rs.isBeforeFirst());
			if (numTuple == 0)	assertFalse(rs.isFirst());
			else				assertTrue(rs.isFirst());
			if (numTuple == 1)	assertTrue(rs.isLast());
			else				assertFalse(rs.isLast());
			if (numTuple == 0)	assertTrue(rs.isAfterLast());
			else				assertFalse(rs.isAfterLast());
			assertEquals((numTuple == 0) ? 0 : 1, rs.getRow());

			if (numTuple > 0) {

				// 2nd next

				if (numTuple == 1)	assertFalse(rs.next());
				else				assertTrue(rs.next());
				assertFalse(rs.isBeforeFirst());
				assertFalse(rs.isFirst());
				if (numTuple == 2)	assertTrue(rs.isLast());
				else				assertFalse(rs.isLast());
				if (numTuple == 1)	assertTrue(rs.isAfterLast());
				else				assertFalse(rs.isAfterLast());
				assertEquals((numTuple == 1) ? 0 : 2, rs.getRow());

				if (numTuple > 1) {

					// 3rd next

					if (numTuple == 2)	assertFalse(rs.next());
					else				assertTrue(rs.next());
					assertFalse(rs.isBeforeFirst());
					assertFalse(rs.isFirst());
					if (numTuple == 3)	assertTrue(rs.isLast());
					else				assertFalse(rs.isLast());
					if (numTuple == 2)	assertTrue(rs.isAfterLast());
					else				assertFalse(rs.isAfterLast());
					assertEquals((numTuple == 2) ? 0 : 3, rs.getRow());

					if (numTuple > 2) {

						// 4th next

						if (numTuple == 3)	assertFalse(rs.next());
						else				assertTrue(rs.next());
						assertFalse(rs.isBeforeFirst());
						assertFalse(rs.isFirst());
						if (numTuple == 4)	assertTrue(rs.isLast());
						else				assertFalse(rs.isLast());
						if (numTuple == 3)	assertTrue(rs.isAfterLast());
						else				assertFalse(rs.isAfterLast());
						assertEquals((numTuple == 3) ? 0 : 4, rs.getRow());

						if (numTuple > 3) {

							// 5th next

							if (numTuple == 4)	assertFalse(rs.next());
							else				assertTrue(rs.next());
							assertFalse(rs.isBeforeFirst());
							assertFalse(rs.isFirst());
							if (numTuple == 5)	assertTrue(rs.isLast());
							else				assertFalse(rs.isLast());
							if (numTuple == 4)	assertTrue(rs.isAfterLast());
							else				assertFalse(rs.isAfterLast());
							assertEquals((numTuple == 4) ? 0 : 5, rs.getRow());

							if (numTuple > 4) {

								// 6th next

								assertFalse(rs.next());
								assertFalse(rs.isBeforeFirst());
								assertFalse(rs.isFirst());
								assertFalse(rs.isLast());
								assertTrue(rs.isAfterLast());
								assertEquals(0, rs.getRow());
							}
						}
					}
				}
			}

			// 後始末
			dropTestTable(c);

			c.close();
		}
	}

	// ResultSet.next() のテストはいろんなところでやっているので省く

	// ResultSet.refreshRow() のテスト
	public void test_refreshRow() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c, 1);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		assertNull(rs.getWarnings());

		java.util.Vector	wEC = new java.util.Vector();	// error code
		java.util.Vector	wSS = new java.util.Vector();	// SQLState
		java.util.Vector	wMS = new java.util.Vector();	// message

		// 現状ではカーソルをサポートしていないので警告が出るはず
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_REFRESHROW_NO_PERFORMED);
		rs.refreshRow();
		// 警告のチェック
		assertSQLWarning(rs.getWarnings(), wEC, wSS, wMS);

		// next しても警告が出るはず
		assertTrue(rs.next());
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_REFRESHROW_NO_PERFORMED);
		rs.refreshRow();
		// 警告のチェック
		assertSQLWarning(rs.getWarnings(), wEC, wSS, wMS);

		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.rowDeleted() と ResultSet.rowInserted() と ResultSet.rowUpdated() のテスト
	public void test_rowUpdated() throws Exception
	{

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c, 1);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		// 常に false が返るはず
		assertFalse(rs.rowDeleted());
		assertFalse(rs.rowInserted());
		assertFalse(rs.rowUpdated());

		// next しても同じはず
		assertTrue(rs.next());
		assertFalse(rs.rowDeleted());
		assertFalse(rs.rowInserted());
		assertFalse(rs.rowUpdated());

		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.setFetchDirection() と ResultSet.getFetchDirection() のテスト
	public void test_setAndGetFetchDirection() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c, 1);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		java.util.Vector	wEC = new java.util.Vector();	// error code
		java.util.Vector	wSS = new java.util.Vector();	// SQLState
		java.util.Vector	wMS = new java.util.Vector();	// message

		assertNull(rs.getWarnings());

		// 常に java.sql.ResultSet.FETCH_FORWARD が返るはず
		assertEquals(ResultSet.FETCH_FORWARD, rs.getFetchDirection());

		// java.sql.ResultSet.FETCH_FORWARD -> 未処理
		rs.setFetchDirection(ResultSet.FETCH_FORWARD);
		assertNull(rs.getWarnings());
		assertEquals(ResultSet.FETCH_FORWARD, rs.getFetchDirection());

		// java.sql.ResultSet.FETCH_REVERSE -> 警告
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CURRENTLY_SUPPORTED_ONLY_FETCH_FORWARD);
		rs.setFetchDirection(ResultSet.FETCH_REVERSE);
		assertSQLWarning(rs.getWarnings(), wEC, wSS, wMS);
		assertEquals(ResultSet.FETCH_FORWARD, rs.getFetchDirection());

		// java.sql.ResultSet.FETCH_UNKNOWN -> 警告
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CURRENTLY_SUPPORTED_ONLY_FETCH_FORWARD);
		rs.setFetchDirection(ResultSet.FETCH_UNKNOWN);
		assertSQLWarning(rs.getWarnings(), wEC, wSS, wMS);
		assertEquals(ResultSet.FETCH_FORWARD, rs.getFetchDirection());

		// 上記みっつ以外の値 -> BadArgument
		boolean	caught = false;
		try {
			rs.setFetchDirection(9999);
		} catch (SQLException	sqle) {
			caught = true;
			assertBadArgument(sqle);
		}
		assertTrue(caught);
		assertSQLWarning(rs.getWarnings(), wEC, wSS, wMS);
		assertEquals(ResultSet.FETCH_FORWARD, rs.getFetchDirection());

		//
		// next しても同じはず
		//

		assertTrue(rs.next());

		// java.sql.ResultSet.FETCH_FORWARD -> 未処理
		rs.setFetchDirection(ResultSet.FETCH_FORWARD);
		assertSQLWarning(rs.getWarnings(), wEC, wSS, wMS);
		assertEquals(ResultSet.FETCH_FORWARD, rs.getFetchDirection());

		// java.sql.ResultSet.FETCH_REVERSE -> 警告
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CURRENTLY_SUPPORTED_ONLY_FETCH_FORWARD);
		rs.setFetchDirection(ResultSet.FETCH_REVERSE);
		assertSQLWarning(rs.getWarnings(), wEC, wSS, wMS);
		assertEquals(ResultSet.FETCH_FORWARD, rs.getFetchDirection());

		// java.sql.ResultSet.FETCH_UNKNOWN -> 警告
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_CURRENTLY_SUPPORTED_ONLY_FETCH_FORWARD);
		rs.setFetchDirection(ResultSet.FETCH_UNKNOWN);
		assertSQLWarning(rs.getWarnings(), wEC, wSS, wMS);
		assertEquals(ResultSet.FETCH_FORWARD, rs.getFetchDirection());

		// 上記みっつ以外の値 -> BadArgument
		caught = false;
		try {
			rs.setFetchDirection(9999);
		} catch (SQLException	sqle) {
			caught = true;
			assertBadArgument(sqle);
		}
		assertTrue(caught);
		assertSQLWarning(rs.getWarnings(), wEC, wSS, wMS);
		assertEquals(ResultSet.FETCH_FORWARD, rs.getFetchDirection());

		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.setFetchSize() と ResultSet.getFetchSize() のテスト
	public void test_setAndGetFetchSize() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c, 1);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		java.util.Vector	wEC = new java.util.Vector();	// error code
		java.util.Vector	wSS = new java.util.Vector();	// SQLState
		java.util.Vector	wMS = new java.util.Vector();	// message

		assertNull(rs.getWarnings());

		// 常に 0 が返るはず
		assertZero(rs.getFetchSize());

		// 0 -> 未処理
		rs.setFetchSize(0);
		assertNull(rs.getWarnings());
		assertZero(rs.getFetchSize());

		// 0 以外 -> 警告
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_FETCH_SIZE_SUPPORTED_ONLY_ZERO);
		rs.setFetchSize(1);
		assertSQLWarning(rs.getWarnings(), wEC, wSS, wMS);
		assertZero(rs.getFetchSize());

		//
		// next しても同じはず
		//

		assertTrue(rs.next());

		// 0 -> 未処理
		rs.setFetchSize(0);
		assertSQLWarning(rs.getWarnings(), wEC, wSS, wMS);
		assertZero(rs.getFetchSize());

		// 0 以外 -> 警告
		wEC.add(new Integer(0));	wSS.add("01000");	wMS.add(WM_FETCH_SIZE_SUPPORTED_ONLY_ZERO);
		rs.setFetchSize(1);
		assertSQLWarning(rs.getWarnings(), wEC, wSS, wMS);
		assertZero(rs.getFetchSize());

		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// ResultSet.wasNull() のテストはいろんなところでやっているので省く

	// 列名指定の getter メソッドメソッドのテストその１−列名を小文字で指定
	public void test_getByColumnName1() throws Exception
	{

		Connection	c = null;
		assertNotNull(c = getConnection());

		int[]		ids =		{	1,			2,			3		};
		String[]	titles =	{	"RICOH",	"Sydney",	"Java"	};

		// 下準備
		createSimpleTable(c, ids, titles);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		assertTrue(rs.next());
		assertEquals(ids[0], rs.getInt("id"));
		assertEquals(titles[0], rs.getString("title"));

		assertTrue(rs.next());
		assertEquals(ids[1], rs.getInt("id"));
		assertEquals(titles[1], rs.getString("title"));

		assertTrue(rs.next());
		assertEquals(ids[2], rs.getInt("id"));
		assertEquals(titles[2], rs.getString("title"));

		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// 列名指定の getter メソッドのテストその２−列名を大文字で指定
	public void test_getByColumnName2() throws Exception
	{

		Connection	c = null;
		assertNotNull(c = getConnection());

		int[]		ids =		{	1,			2,			3		};
		String[]	titles =	{	"RICOH",	"Sydney",	"Java"	};

		// 下準備
		createSimpleTable(c, ids, titles);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		assertTrue(rs.next());
		assertEquals(ids[0], rs.getInt("ID"));
		assertEquals(titles[0], rs.getString("TITLE"));

		assertTrue(rs.next());
		assertEquals(ids[1], rs.getInt("ID"));
		assertEquals(titles[1], rs.getString("TITLE"));

		assertTrue(rs.next());
		assertEquals(ids[2], rs.getInt("ID"));
		assertEquals(titles[2], rs.getString("TITLE"));

		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// 列名指定の getter メソッドのテストその３−列名の別名で指定
	public void test_getByColumnName3() throws Exception
	{

		Connection	c = null;
		assertNotNull(c = getConnection());

		int[]		ids =		{	1,			2,			3		};
		String[]	titles =	{	"RICOH",	"Sydney",	"Java"	};

		// 下準備
		createSimpleTable(c, ids, titles);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select id as ii, title as tt from t order by id"));

		assertTrue(rs.next());
		assertEquals(ids[0], rs.getInt("ii"));
		assertEquals(titles[0], rs.getString("tt"));

		assertTrue(rs.next());
		assertEquals(ids[1], rs.getInt("II"));
		assertEquals(titles[1], rs.getString("TT"));

		assertTrue(rs.next());
		assertEquals(ids[2], rs.getInt(1));
		assertEquals(titles[2], rs.getString(2));

		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// 列インデックス指定の getter メソッドのテスト
	public void test_getByColumnPosition() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		int[]		ids =		{	1,			2,			3		};
		String[]	titles =	{	"RICOH",	"Sydney",	"Java"	};

		// 下準備
		createSimpleTable(c, ids, titles);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		assertTrue(rs.next());
		assertEquals(ids[0], rs.getInt(1));
		assertEquals(titles[0], rs.getString(2));

		assertTrue(rs.next());
		assertEquals(ids[1], rs.getInt(1));
		assertEquals(titles[1], rs.getString(2));

		assertTrue(rs.next());
		assertEquals(ids[2], rs.getInt(1));
		assertEquals(titles[2], rs.getString(2));

		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// 『同じselect文に同じ列が複数表れるときにエラーになる』の再現
	public void test_0252() throws Exception
	{

		Connection	c = null;
		assertNotNull(c = getConnection());

		int[]		ids =		{	1,			2,			3		};
		String[]	titles =	{	"RICOH",	"Sydney",	"Java"	};

		// 下準備
		createSimpleTable(c, ids, titles);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select id as t_id, id as tid from t"));

		for (int i = 1; rs.next(); i++) {
			assertEquals(i, rs.getInt("t_id"));
			assertEquals(i, rs.getInt("tid"));
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	public void test_0252_v14() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		int[]		ids =		{	1,			2,			3		};
		String[]	titles =	{	"RICOH",	"Sydney",	"Java"	};

		// 下準備
		createSimpleTable(c, ids, titles);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select id as t_id, id as tid from t"));

		for (int i = 1; rs.next(); i++) {
			assertEquals(i, rs.getInt(1));
			assertEquals(i, rs.getInt(2));
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// 未サポートのメソッドテスト
	public void test_NotSupported() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createSimpleTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t"));

		// ResultSet.moveToCurrentRow()
		boolean	caught = false;
		try {
			rs.moveToCurrentRow();
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.moveToInsertRow()
		caught = false;
		try {
			rs.moveToInsertRow();
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.deleteRow()
		caught = false;
		try {
			rs.deleteRow();
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateRow()
		caught = false;
		try {
			rs.updateRow();
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.insertRow()
		caught = false;
		try {
			rs.insertRow();
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateNull(int)
		caught = false;
		try {
			rs.updateNull(1);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateNull(String)
		caught = false;
		try {
			rs.updateNull("id");
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateArray(int, java.sql.Array)
		String[]	ary = { "hoge", "hoge" };
		caught = false;
		try {
			rs.updateArray(1, new TestArray(ary));
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateArray(String, java.sql.Array)
		caught = false;
		try {
			rs.updateArray("id", new TestArray(ary));
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateAsciiStream(int, java.io.InputStream, int)
		caught = false;
		try {
			rs.updateAsciiStream(1, new java.io.FileInputStream("build.xml"), 100);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateAsciiStream(String, java.io.IntputStream, int)
		caught = false;
		try {
			rs.updateAsciiStream("id", new java.io.FileInputStream("build.xml"), 100);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateBigDecimal(int, BigDecimal)
		caught = false;
		try {
			rs.updateBigDecimal(1, new BigDecimal("9483729"));
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateBigDecimal(String, BigDecimal)
		caught = false;
		try {
			rs.updateBigDecimal("id", new BigDecimal("4987234"));
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateBinaryStream(int, java.io.InputStream)
		caught = false;
		try {
			rs.updateBinaryStream(1, new java.io.FileInputStream("build.xml"), 100);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateBinaryStream(String, java.io.InputStream)
		caught = false;
		try {
			rs.updateBinaryStream("id", new java.io.FileInputStream("build.xml"), 100);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateBlob(int, java.sql.Blob)
		caught = false;
		try {
			rs.updateBlob(1, new DummyBlob());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateBlob(String, java.sql.Blob)
		caught = false;
		try {
			rs.updateBlob("id", new DummyBlob());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateBoolean(int, boolean)
		caught = false;
		try {
			rs.updateBoolean(1, true);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateBoolean(String, boolean)
		caught = false;
		try {
			rs.updateBoolean("id", true);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateByte(int, byte)
		caught = false;
		try {
			rs.updateByte(1, (byte)1);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateByte(String, byte)
		caught = false;
		try {
			rs.updateByte("id", (byte)20);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateBytes(int, byte[])
		byte[]	bytes = { 0x30, 0x4A };
		caught = false;
		try {
			rs.updateBytes(1, bytes);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateBytes(String, byte[])
		caught = false;
		try {
			rs.updateBytes("id", bytes);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateCharacterStream(int, java.io.Reader, int)
		caught = false;
		try {
			rs.updateCharacterStream(1, new java.io.FileReader("build.xml"), 100);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateCharacterStream(String, java.io.Reader, int)
		caught = false;
		try {
			rs.updateCharacterStream("id", new java.io.FileReader("build.xml"), 100);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateClob(int, java.sql.Clob)
		caught = false;
		try {
			rs.updateClob(1, new DummyClob());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateClob(String, java.sql.Clob)
		caught = false;
		try {
			rs.updateClob("id", new DummyClob());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateDate(int, java.sql.Date)
		caught = false;
		try {
			rs.updateDate(1, Date.valueOf("2004-12-28"));
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateDate(String, java.sql.Date)
		caught = false;
		try {
			rs.updateDate("id", Date.valueOf("2004-12-28"));
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateDouble(int, double)
		caught = false;
		try {
			rs.updateDouble(1, 493827.987);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateDouble(String, double)
		caught = false;
		try {
			rs.updateDouble("id", 929483.7);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateFloat(int, float)
		caught = false;
		try {
			rs.updateFloat(1, (float)3.8);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateFloat(String, float)
		caught = false;
		try {
			rs.updateFloat("id", (float)49.2);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateInt(int, int)
		caught = false;
		try {
			rs.updateInt(1, 328);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateInt(String, int)
		caught = false;
		try {
			rs.updateInt("id", 90);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateLong(int, long)
		caught = false;
		try {
			rs.updateLong(1, 9872341);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateLong(String, long)
		caught = false;
		try {
			rs.updateLong("id", 3979234);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateObject(int, Object)
		caught = false;
		try {
			rs.updateObject(1, new LanguageData("ja"));
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateObject(int, Object, int)
		caught = false;
		try {
			rs.updateObject(1, new Float(8.8743), 4);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateObject(String, Object)
		caught = false;
		try {
			rs.updateObject("id", new IntegerData(32));
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateObject(String, Object, int)
		caught = false;
		try {
			rs.updateObject("id", new Double(987432.87), 5);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateRef(int, ava.sql.Ref)
		caught = false;
		try {
			rs.updateRef(1, new DummyRef());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateRef(String, java.sql.Ref)
		caught = false;
		try {
			rs.updateRef("id", new DummyRef());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateShort(int, short)
		caught = false;
		try {
			rs.updateShort(1, (short)3);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateShort(String, short)
		caught = false;
		try {
			rs.updateShort("id", (short)5);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateString(int, String)
		caught = false;
		try {
			rs.updateString(1, "hogehoge");
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateString(String, String)
		caught = false;
		try {
			rs.updateString("id", "hogehoge");
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateTime(int, java.sql.Time)
		caught = false;
		try {
			rs.updateTime(1, Time.valueOf("17:37:20"));
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateTime(String, java.sql.Time)
		caught = false;
		try {
			rs.updateTime("id", Time.valueOf("17:38:04"));
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateTimestamp(int, java.sql.Timestamp)
		caught = false;
		try {
			rs.updateTimestamp(1, Timestamp.valueOf("2004-12-28 17:35:05.410"));
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.updateTimestamp(String, java.sql.Timestamp)
		caught = false;
		try {
			rs.updateTimestamp("id", Timestamp.valueOf("2004-12-28 17:36:41.001"));
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getAsciiStream(int)
		caught = false;
		try {
			rs.getAsciiStream(1);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getAsciiStream(String)
		caught = false;
		try {
			rs.getAsciiStream("id");
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getUnicodeStream(int)
		caught = false;
		try {
			rs.getUnicodeStream(1);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getUnicodeStream(String)
		caught = false;
		try {
			rs.getUnicodeStream("id");
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getRef(int)
		caught = false;
		try {
			rs.getRef(1);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getRef(String)
		caught = false;
		try {
			rs.getRef("id");
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getBlob(int)
		caught = false;
		try {
			rs.getBlob(1);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getBlob(String)
		caught = false;
		try {
			rs.getBlob("id");
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getClob(int)
		caught = false;
		try {
			rs.getClob(1);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getClob(String)
		caught = false;
		try {
			rs.getClob("id");
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getDate(int, java.util.Calendar)
		caught = false;
		try {
			rs.getDate(1, new java.util.GregorianCalendar());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getDate(String, java.util.Calendar)
		caught = false;
		try {
			rs.getDate("id", new java.util.GregorianCalendar());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getTime(int, java.util.Calendar)
		caught = false;
		try {
			rs.getTime(1, new java.util.GregorianCalendar());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getTime(String, java.util.Calendar)
		caught = false;
		try {
			rs.getTime("id", new java.util.GregorianCalendar());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getTimestamp(int, java.util.Calendar)
		caught = false;
		try {
			rs.getTimestamp(1, new java.util.GregorianCalendar());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getTimestamp(String, java.util.Calendar)
		caught = false;
		try {
			rs.getTimestamp("id", new java.util.GregorianCalendar());
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getURL(int)
		caught = false;
		try {
			rs.getURL(1);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getURL(String)
		caught = false;
		try {
			rs.getURL("id");
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.beforeFirst()
		caught = false;
		try {
			rs.beforeFirst();
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.afterLast()
		caught = false;
		try {
			rs.afterLast();
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.first()
		caught = false;
		try {
			rs.first();
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.last()
		caught = false;
		try {
			rs.last();
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.absolute(int)
		caught = false;
		try {
			rs.absolute(1);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.relative(int)
		caught = false;
		try {
			rs.relative(1);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.previous()
		caught = false;
		try {
			rs.previous();
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		// ResultSet.getObject(int, Class<T>)
		caught = false;
		try {
			rs.getObject(0, int.class);
		} catch (SQLException	sqle) {
			caught = true;
			assertNotSupported(sqle);
		}
		assertTrue(caught);

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
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

	private SQLException checkParseBigInteger(String	s)
	{
		SQLException	e = null;
		try {
			if (s != null) {
				java.math.BigInteger	bigInteger = new java.math.BigInteger(s);
			}
		} catch (NumberFormatException	nfe) {
			e = new ClassCast();
		}
		return e;
	}

	private SQLException checkParseInt(String	s)
	{
		SQLException	e = null;
		try {
			if (s != null) Integer.parseInt(s);
		} catch (NumberFormatException	nfe) {
			e = new ClassCast();
		}
		return e;
	}

	private SQLException checkParseByte(String	s)
	{
		SQLException	e = null;
		try {
			if (s != null) Byte.parseByte(s);
		} catch (NumberFormatException	nfe) {
			e = new ClassCast();
		}
		return e;
	}

	private SQLException checkParseDouble(String	s)
	{
		SQLException	e = null;
		try {
			if (s != null) Double.parseDouble(s);
		} catch (NumberFormatException	nfe) {
			e = new ClassCast();
		}
		return e;
	}

	private SQLException checkParseFloat(String	s)
	{
		SQLException	e = null;
		try {
			if (s != null) Float.parseFloat(s);
		} catch (NumberFormatException	nfe) {
			e = new ClassCast();
		}
		return e;
	}

	private SQLException checkParseLong(String	s)
	{
		SQLException	e = null;
		try {
			if (s != null) Long.parseLong(s);
		} catch (NumberFormatException	nfe) {
			e = new ClassCast();
		}
		return e;
	}

	private SQLException checkParseShort(String	s)
	{
		SQLException	e = null;
		try {
			if (s != null) Short.parseShort(s);
		} catch (NumberFormatException	nfe) {
			e = new ClassCast();
		}
		return e;
	}

	private SQLException checkParseDate(String	s)
	{
		SQLException	e = null;
		try {
			if (s != null) java.sql.Date.valueOf(s);
		} catch (NumberFormatException	nfe) {
			e = new ClassCast();
		} catch (IllegalArgumentException	iae) {
			e = new ClassCast();
		}
		return e;
	}

	private SQLException checkParseTime(String	s)
	{
		SQLException	e = null;
		try {
			if (s != null) java.sql.Time.valueOf(s);
		} catch (NumberFormatException	nfe) {
			e = new ClassCast();
		} catch (IllegalArgumentException	iae) {
			e = new ClassCast();
		}
		return e;
	}

	private SQLException checkParseTimestamp(String	s)
	{
		SQLException	e = null;
		try {
			if (s != null) java.sql.Timestamp.valueOf(s);
		} catch (NumberFormatException	nfe) {
			e = new ClassCast();
		} catch (IllegalArgumentException	iae) {
			e = new ClassCast();
		}
		return e;
	}

	// テスト用の表を作成する
	// ※ タプルの挿入は行わないので常に 0 を返す
	private void createTestTable(Connection	c) throws Exception
	{
		Statement	s = null;
		assertNotNull(s = c.createStatement());
		String	query =
			"create table t (																	" +
			"	f_int_not_null		int not null,												" +
			"	f_int1				int,														" +
			"	f_int2				int,														";

		query = query +
			"	f_bigint			bigint,														";
		query = query +
			"	f_decimal			decimal(10,5),													";

		query = query +
			"	f_char8_not_null	char(8) not null,											" +
			"	f_char8				char(8),													" +
			"	f_float				float,														" +
			"	f_datetime			datetime,													" +
			"	f_id				uniqueidentifier,											" +
			"	f_image				image,														" +
			"	f_language			language,													" +
			"	f_nchar6			nchar(6),													" +
			"	f_nvarchar256		nvarchar(256),												" +
			"	f_varchar128		varchar(128),												" +
			"	f_ntext				ntext,														" +
			"	f_ntext_compressed	ntext hint heap 'compressed',								" +
			"	f_fulltext			fulltext,													" +
			"	f_binary50			binary(50),													" +
			"	f_blob				blob,														" +
			"	f_nclob				nclob,														" +
			"	af_int				int					array[no limit],						";

		query = query +
			"	af_bigint			bigint				array[no limit],						";
		query = query +
			"	af_decimal			decimal(10,5)		array[no limit],						";

		query = query +
			"	af_char8			char(8)				array[no limit],						" +
			"	af_float			float				array[no limit],						" +
			"	af_datetime			datetime			array[no limit],						" +
			"	af_id				uniqueidentifier	array[no limit],						" +
			"	af_image			image				array[no limit],						" +
			"	af_language			language			array[no limit],						" +
			"	af_nchar6			nchar(6)			array[no limit],						" +
			"	af_nvarchar256		nvarchar(256)		array[no limit],						" +
			"	af_varchar128		varchar(128)		array[no limit],						" +
			"	af_ntext			ntext				array[no limit],						" +
			"	af_ntext_compressed	ntext				array[no limit] hint heap 'compressed',	" +
			"	af_fulltext			fulltext			array[no limit],						" +
			"	af_binary50			binary(50)			array[no limit],						" +
			"	primary key(f_int1)																" +
			")																					";
		s.executeUpdate(query);
		s.close();
	}

	private Integer[]	f_int_not_nulls = { new Integer(101), new Integer(102), new Integer(103), new Integer(104), new Integer(105), new Integer(106) };
						//                  ~~~~~~~~~~~~~~~~ f_int_not_null は not null 制約がついているので null にはできない

	private Integer[]	f_int1s = { new Integer(1), new Integer(2), new Integer(3), new Integer(4), new Integer(5), new Integer(6) };
						//          ~~~~~~~~~~~~~~ f_int1 は primary key なので null にはできない

	private Integer[]	f_int2s = { null, new Integer(3), new Integer(0), new Integer(1), new Integer(0), new Integer(10) };

	private Long[]		f_bigints = { null, new Long(549827687432L), new Long(38779870000L), new Long(-9057832987L), new Long(28701837412L), new Long(-700023982734L) };
	private BigDecimal[] f_decimals = { null, new BigDecimal("99999.99999"), new BigDecimal("0"), new BigDecimal("-876.543"), new BigDecimal("1"), new BigDecimal("-45678.90123") };

	private String[]	f_char8_not_nulls = { "53278913", "92846278", "00000000", "00000001", "32976438", "29837122" };
						//                    ~~~~~~~~~~ f_char8_not_null は not null 制約がついているので null にはできない

	private String[]	f_char8s = { null, "Abstract", "false   ", "TRUE    ", "False   ", "Tokenize" };

	private Double[]	f_floats = { null, new Double(532987.879), new Double(0), new Double(1), new Double(0.0000087832), new Double(532.87987) };

	private Timestamp[]	f_datetimes = { null, Timestamp.valueOf("2005-01-05 11:52:38.109"), Timestamp.valueOf("2005-01-06 13:44:01.337"), Timestamp.valueOf("2002-10-03 04:59:08.387"), Timestamp.valueOf("1980-04-03 12:58:49.387"), Timestamp.valueOf("2005-01-07 16:27:29.486") };

	private String[]	f_ids = { null, "053D535D-82D3-4DC8-92D0-35DFD8B25461", "35509791-DFC2-4390-A099-791E6C0A135E", "83851E53-7426-47B5-8F53-D2B6CC0D6803", "95A41B28-DEAD-4228-92A8-D700DDA30F19", "C7B757AB-871A-43BB-B6A0-752F08B9E7DE" };

	private byte[]		f_image_2 = { 0x3F, 0x29, 0x5A, 0x00, 0x3D, 0x01, 0x5F, 0x77, 0x34, 0x6C, 0x5D, 0x23, 0x52, 0x61, 0x44, 0x59, 0x47, 0x08, 0x4A, 0x5E, 0x43, 0x0D, 0x03, 0x45, 0x65, 0x1F, 0x5E };
	private byte[]		f_image_3 = { 0x4D, 0x1E, 0x6C, 0x50, 0x00, 0x0F, 0x2A, 0x72, 0x45, 0x0E, 0x38, 0x0A, 0x29, 0x1F, 0x30, 0x78, 0x2E, 0x1B, 0x5A, 0x2F, 0x1A, 0x06 };
	private byte[]		f_image_4 = { 0x38, 0x6C, 0x5E, 0x1B, 0x0C, 0x29, 0x4A, 0x08, 0x64, 0x0A, 0x5A, 0x1B, 0x49, 0x5D, 0x1B, 0x29, 0x0C, 0x43, 0x6C, 0x4A, 0x13, 0x1B, 0x0A, 0x0A, 0x04, 0x4C, 0x45, 0x18, 0x0F, 0x34, 0x1F, 0x2E, 0x28, 0x19, 0x6F, 0x53, 0x28, 0x59, 0x19, 0x13, 0x25, 0x6A, 0x5B, 0x2C, 0x29, 0x73, 0x3A, 0x1C, 0x1A, 0x08, 0x0A, 0x06, 0x48, 0x20, 0x3B, 0x19, 0x19, 0x37, 0x64, 0x1A, 0x73, 0x07, 0x26, 0x22 };
	private byte[]		f_image_5 = { 0x5C, 0x5D, 0x2F, 0x10, 0x3C, 0x1A, 0x37, 0x74, 0x0C, 0x1F, 0x19, 0x24, 0x3B, 0x66, 0x0D, 0x29, 0x09, 0x28, 0x1E, 0x2F, 0x33, 0x75, 0x18, 0x65, 0x61, 0x39, 0x0E, 0x43, 0x3F, 0x2B, 0x5B, 0x2B, 0x3B, 0x0A, 0x15, 0x30, 0x45, 0x1C, 0x24, 0x75 };
	private byte[]		f_image_6 = { 0x50, 0x76, 0x5A, 0x3C, 0x0A, 0x3A, 0x29, 0x5B, 0x2B, 0x4C, 0x1A, 0x69, 0x57, 0x7E, 0x1F, 0x1E, 0x6B, 0x67, 0x31, 0x0F, 0x37, 0x06, 0x1C, 0x56, 0x2C, 0x3F, 0x66, 0x0B, 0x64, 0x5E, 0x2C, 0x2D, 0x0A, 0x68, 0x1A, 0x59, 0x0A, 0x39, 0x1D, 0x79, 0x7E, 0x2E, 0x3B };
	private byte[][]	f_images = { null, f_image_2, f_image_3, f_image_4, f_image_5, f_image_6 };

	private String[]	f_languages = { null, "it", "en", "ja", "fr", "de" };

	private String[]	f_nchar6s = { null, "Stroke", "FALSE ", "true  ", "false ", "Reader" };

	private String[]	f_nvarchar256s = { null, "Java プログラミング言語の定数で、型コードとも呼ばれ、汎用の SQL 型 REF を識別します。", "2005-01-07", "true", "1", "空の文字列を表す新しい String オブジェクトが生成されて返されます。" };

	private String[]	f_varchar128s = { null, "A conversion from a type to that same type is permitted for any type.", "false", "0", "", "03:48:52" };

	private String[]	f_ntexts = { null, "バッチ更新オペレーション中にエラーが発生したときにスローされる例外です。", "2001-12-30 04:59:32.638", "FALSE", "0", "スタックトレース内の要素で、Throwable.getStackTrace() により返される値。各要素は単一のスタックフレームを表します。スタックの先頭にあるスタックフレームを除く、すべてのスタックフレームは、メソッド呼び出しを表します。" };

	private String[]	f_ntext_compresseds = { null, "接続がクローズされたあとで警告を取得しようとすると例外がスローされます。同様に、文がクローズされたあと、または結果セットがクローズされたあとで警告を取得しようとすると例外がスローされます。", "false", "1980-04-01 16:58:32.483", "スタックの先頭のフレームは、スタックトレースが生成された実行ポイントを表します。", "通常、これは、スタックトレースに対応するスロー可能オブジェクトが作成されたポイントになります。" };

	private String[]	f_fulltexts = { null, "1973-11-06", "このクラスは、java.util.Date の thin ラッパーで、このラッパーによって JDBC API はこれを SQL TIMESTAMP 値として識別できます。このクラスは、SQL TIMESTAMP の nanos 値 (ナノ秒の値) を保持する機能を追加し、タイムスタンプ値の JDBC エスケープ構文をサポートするためのフォーマットと構文解析操作を提供します。", "TRUE", "59", "FALSE" };

	private byte[]		f_binary50_2 = { 0x3D, 0x24, 0x0C, 0x7E, 0x3F, 0x3E, 0x2E, 0x0B, 0x10, 0x44, 0x4D, 0x08, 0x4B, 0x2F, 0x2D, 0x0F, 0x72, 0x73, 0x76, 0x1C, 0x3E, 0x18, 0x75, 0x7E, 0x1C, 0x1F, 0x70, 0x6B, 0x2D, 0x7C, 0x5F, 0x4D, 0x08, 0x08, 0x1B, 0x08, 0x39, 0x66, 0x51, 0x18, 0x6A, 0x0F, 0x48, 0x2A, 0x4A, 0x33, 0x20, 0x01, 0x5F, 0x3A };
	private byte[]		f_binary50_3 = { 0x24, 0x1E, 0x29, 0x59, 0x07, 0x44, 0x09, 0x74, 0x3A, 0x2F, 0x18, 0x27, 0x4B, 0x0F, 0x6F, 0x2D, 0x28, 0x09, 0x38, 0x1A, 0x48, 0x28, 0x1E, 0x55, 0x44, 0x3B, 0x7A, 0x34, 0x2C, 0x1F, 0x4F, 0x68, 0x5F, 0x1A, 0x0C, 0x28, 0x5E, 0x4E, 0x28, 0x37, 0x71, 0x31, 0x1A, 0x3E, 0x1D, 0x5A, 0x6F, 0x00, 0x47, 0x38 };
	private byte[]		f_binary50_4 = { 0x72, 0x2A, 0x2F, 0x35, 0x5C, 0x4E, 0x18, 0x39, 0x13, 0x48, 0x36, 0x28, 0x10, 0x3D, 0x28, 0x32, 0x09, 0x5E, 0x3D, 0x1D, 0x3A, 0x00, 0x2D, 0x3A, 0x28, 0x1A, 0x0A, 0x36, 0x4D, 0x47, 0x59, 0x18, 0x7D, 0x1F, 0x60, 0x0A, 0x20, 0x2D, 0x26, 0x7C, 0x38, 0x33, 0x41, 0x1B, 0x0A, 0x64, 0x42, 0x00, 0x38, 0x78 };
	private byte[]		f_binary50_5 = { 0x1B, 0x77, 0x18, 0x17, 0x3C, 0x19, 0x44, 0x41, 0x5F, 0x29, 0x28, 0x1A, 0x20, 0x40, 0x79, 0x0A, 0x3B, 0x29, 0x5F, 0x4C, 0x2F, 0x72, 0x2C, 0x39, 0x2E, 0x22, 0x16, 0x64, 0x61, 0x1C, 0x08, 0x5F, 0x3F, 0x1A, 0x3D, 0x2F, 0x4F, 0x3F, 0x2B, 0x6B, 0x3D, 0x28, 0x37, 0x39, 0x50, 0x29, 0x2F, 0x54, 0x39, 0x00 };
	private byte[]		f_binary50_6 = { 0x20, 0x1F, 0x4A, 0x6B, 0x3A, 0x4A, 0x0C, 0x38, 0x46, 0x23, 0x28, 0x19, 0x45, 0x0A, 0x5D, 0x1F, 0x3B, 0x29, 0x0B, 0x04, 0x3F, 0x3E, 0x5F, 0x5E, 0x79, 0x2C, 0x3D, 0x09, 0x18, 0x69, 0x59, 0x44, 0x4B, 0x2B, 0x48, 0x47, 0x53, 0x19, 0x1E, 0x09, 0x12, 0x5B, 0x2F, 0x18, 0x17, 0x0D, 0x0B, 0x77, 0x2F, 0x2F };
	private byte[][]	f_binary50s = { null, f_binary50_2, f_binary50_3, f_binary50_4, f_binary50_5, f_binary50_6 };

	private byte[]		f_blob_2 = { 0x0E, 0x3A, 0x1E, 0x5C, 0x40, 0x21, 0x0D, 0x3D, 0x1F, 0x45, 0x1D, 0x5A, 0x5D, 0x2B, 0x4E, 0x39, 0x53, 0x74, 0x28, 0x24, 0x5D, 0x4F, 0x48, 0x2B, 0x2C, 0x1F, 0x5B, 0x0C, 0x23, 0x40, 0x1B, 0x2A, 0x0B, 0x2E, 0x2B, 0x33, 0x49, 0x5A, 0x2C, 0x21, 0x7C, 0x08, 0x3C, 0x18, 0x7F, 0x76, 0x75, 0x0D, 0x1D, 0x4F, 0x3A, 0x29, 0x38, 0x1D, 0x34, 0x58, 0x21, 0x0D, 0x2F, 0x01, 0x28, 0x2F, 0x1D };
	private byte[]		f_blob_3 = { 0x38, 0x19, 0x4C, 0x2A, 0x28, 0x1A, 0x0A, 0x3A, 0x28, 0x24, 0x7E, 0x28, 0x4D, 0x5D, 0x64, 0x2C, 0x0A, 0x43, 0x1A, 0x78, 0x0D, 0x12, 0x38, 0x29, 0x0A, 0x1F, 0x3F, 0x28, 0x29, 0x4E, 0x34, 0x33, 0x3B, 0x5A, 0x70, 0x68, 0x75, 0x6E, 0x50, 0x69, 0x39, 0x0B };
	private byte[]		f_blob_4 = { 0x7D, 0x4E, 0x3D, 0x1B, 0x0D, 0x2C, 0x37, 0x40, 0x47, 0x0A, 0x7D, 0x3E, 0x5B, 0x2E, 0x3B, 0x4D, 0x1E, 0x2C, 0x1B, 0x1C, 0x04, 0x08 };
	private byte[]		f_blob_5 = { 0x5E, 0x0D, 0x77, 0x05, 0x3B, 0x3B, 0x24, 0x1F, 0x14, 0x77, 0x2F, 0x0A, 0x5E, 0x5B, 0x6C, 0x6B, 0x05, 0x11, 0x50, 0x1D, 0x0A, 0x0D, 0x11, 0x5E, 0x2E, 0x72, 0x2F, 0x34, 0x06, 0x3F, 0x59, 0x74, 0x0D, 0x4B, 0x2F, 0x61, 0x2E, 0x13, 0x0B, 0x0E, 0x20, 0x3B, 0x74, 0x3E, 0x28, 0x19, 0x1E, 0x09, 0x4A, 0x4E, 0x6C, 0x2A, 0x0C, 0x39, 0x0C, 0x26, 0x3D, 0x3D, 0x7D, 0x28, 0x57, 0x7A };
	private byte[]		f_blob_6 = { 0x4A, 0x1F, 0x0F, 0x2B, 0x25, 0x10, 0x4F, 0x38, 0x1E, 0x4F, 0x0C, 0x58, 0x2A, 0x64, 0x1A, 0x14, 0x78, 0x08, 0x04, 0x29, 0x5F, 0x1C, 0x48, 0x2F, 0x4E, 0x28, 0x08, 0x38, 0x34, 0x1B, 0x2D, 0x4A, 0x12, 0x09, 0x5F, 0x40, 0x60, 0x27, 0x28, 0x19, 0x62, 0x0A, 0x28, 0x5B, 0x39, 0x2F, 0x0C, 0x28, 0x34, 0x01, 0x3B, 0x59, 0x24, 0x0E, 0x2F, 0x18, 0x1E, 0x08, 0x5C, 0x3B, 0x49, 0x3A, 0x2F, 0x30, 0x1E, 0x5A, 0x11, 0x2E, 0x3A };
	private byte[][]	f_blobs = { null, f_blob_2, f_blob_3, f_blob_4, f_blob_5, f_blob_6 };

	private String[]	f_nclobs = { null, "ユーザ定義型の属性をデータベースに書き戻すための出力ストリームです。カスタムマッピングにだけ使用されるこのインタフェースはドライバによって使用され、そのメソッドをプログラマが直接呼び出すことはできません。", "2003-07-25 14:08:39.201", "FALSE", "", "false" };

	private Integer[]	af_int_2 = { new Integer(529), new Integer(3), new Integer(10) };
	private Integer[]	af_int_3 = { new Integer(98), new Integer(0), new Integer(30), new Integer(5) };
	private Integer[]	af_int_4 = { new Integer(128), new Integer(-387), new Integer(0), new Integer(66) };
	private Integer[]	af_int_5 = { new Integer(3) };
	private Integer[]	af_int_6 = { new Integer(39), new Integer(0), new Integer(519), new Integer(33), new Integer(803) };
	private Integer[][]	af_ints = { null, af_int_2, af_int_3, af_int_4, af_int_5, af_int_6 };

	private Long[]		af_bigint_2 = { new Long(149827349701L), new Long(38L), new Long(-80982387613L) };
	private Long[]		af_bigint_3 = { new Long(6297013877L), new Long(-42874189900L), new Long(9823691769080L), new Long(220881498273L) };
	private Long[]		af_bigint_4 = { new Long(30819870000L), new Long(59958701942L) };
	private Long[]		af_bigint_5 = { new Long(-2000010L), new Long(6987000000L), new Long(-6012981897432L), new Long(28017432987L), new Long(3702987302917L), new Long(-70138707000L) };
	private Long[]		af_bigint_6 = { new Long(178293740709L), new Long(20501870987L), new Long(3L), new Long(100000000000L) };
	private Long[][]	af_bigints = { null, af_bigint_2, af_bigint_3, af_bigint_4, af_bigint_5, af_bigint_6 };

	private BigDecimal[] af_decimal_2 = { new BigDecimal("3.14159"), new BigDecimal("0.002"), new BigDecimal("3776"), new BigDecimal("-56.78901"), new BigDecimal("99999.99999"), new BigDecimal("-99999.99999"), new BigDecimal("0") };
	private BigDecimal[] af_decimal_3 = { new BigDecimal("31415.9265"), new BigDecimal("-0.00001"), new BigDecimal("123.456"), new BigDecimal("0"), new BigDecimal("99999.99999") };
	private BigDecimal[] af_decimal_4 = { new BigDecimal("99999.99999"), new BigDecimal("0"), new BigDecimal("65432.10987"), new BigDecimal("1"), new BigDecimal("12345.67890"), new BigDecimal("-98765.43210"), new BigDecimal("-99999.99999"), new BigDecimal("-0.00001") };
	private BigDecimal[] af_decimal_5 = { new BigDecimal("0"), new BigDecimal("0.00001"), new BigDecimal("-0.00001"), new BigDecimal("0") };
	private BigDecimal[] af_decimal_6 = { new BigDecimal("-1"), new BigDecimal("0.99999"), new BigDecimal("99999"), new BigDecimal("12345"), new BigDecimal("-98765") };
	private BigDecimal[][] af_decimals = { null, af_decimal_2, af_decimal_3, af_decimal_4, af_decimal_5, af_decimal_6 };

	private String[]	af_char8_2 = { "AclEntry", "InputMap", "NotFound", "Runnable" };
	private String[]	af_char8_3 = { "Provider", "Receiver", "Registry" };
	private String[]	af_char8_4 = { "Security" };
	private String[]	af_char8_5 = { "SliderUI", "KeyStore", "ListView" };
	private String[]	af_char8_6 = { "BeanInfo", "Inflater" };
	private String[][]	af_char8s = { null, af_char8_2, af_char8_3, af_char8_4, af_char8_5, af_char8_6 };

	private Double[]	af_float_2 = { new Double(0.0087824), new Double(948372.987), new Double(8.9829873) };
	private Double[]	af_float_3 = { new Double(0), new Double(247983.8782), new Double(4672987.987), new Double(0.000148538) };
	private Double[]	af_float_4 = { new Double(5.37), new Double(9872341.639156) };
	private Double[]	af_float_5 = { new Double(1438741983.87), new Double(38149831.8), new Double(0.8439871) };
	private Double[]	af_float_6 = { new Double(3.08791), new Double(56.1973), new Double(0), new Double(87987432.8768) };
	private Double[][]	af_floats = { null, af_float_2, af_float_3, af_float_4, af_float_5, af_float_6 };

	private Timestamp[]	af_datetime_2 = { Timestamp.valueOf("2004-12-31 05:04:39.460"), Timestamp.valueOf("2005-01-05 13:12:49.008") };
	private Timestamp[]	af_datetime_3 = { Timestamp.valueOf("1980-05-14 23:58:05.438"), Timestamp.valueOf("2004-03-12 05:48:21.368"), Timestamp.valueOf("2005-01-06 15:09:38.420") };
	private Timestamp[]	af_datetime_4 = { Timestamp.valueOf("2002-12-04 14:49:23.152") };
	private Timestamp[]	af_datetime_5 = { Timestamp.valueOf("2003-01-21 01:14:36.287"), Timestamp.valueOf("2005-01-06 15:10:08.883") };
	private Timestamp[]	af_datetime_6 = { Timestamp.valueOf("2005-01-07 16:41:19.772") };
	private Timestamp[][]	af_datetimes = { null, af_datetime_2, af_datetime_3, af_datetime_4, af_datetime_5, af_datetime_6 };

	private String[]	af_id_2 = { "CD83C135-D203-44DE-991D-8FD6677A7C37", "5741A98E-32CC-4571-9C8C-C862529CDF93", "AFFF94DB-E4A0-4D04-AA65-7C518185B122", "7FD84A5D-2CFB-41E8-B4A7-F82EA6E3789E", "402C0C7E-8EDE-4C7C-8F72-EFD2130842EB" };
	private String[]	af_id_3 = { "47421E06-6640-459D-A495-E6FC6DFC95B9", "54B27B08-80D9-4EAA-B493-41A187192E73", "124CA827-748B-4D25-9A01-8BB02A68457A" };
	private String[]	af_id_4 = { "88464AEC-4F5C-4E79-BB72-36DC3D9F9D16", "AF64B9B9-FAA3-4CC4-B294-F94A775D12D9", "D0A66F92-4C6A-4B2A-B57B-E3F85C86BD08", "EDCFF74E-CC91-4A13-B5CB-61B38AA5176B", "B0B5F989-C472-4AF2-B9FE-4BE3E0CE898B", "1619BD43-7998-4EBF-A2B5-BC6CFBA6D27E", "C3CD5E43-C6B8-412D-B9D3-7E89819C4ABA", "75BBF84C-C0FC-4605-9762-1DDB6DB07FB1" };
	private String[]	af_id_5 = { "6604BB12-C5D9-4053-934F-D27119875551", "09B0C930-0D51-4D50-A347-0C00CB0F9541", "137F27DD-287F-4979-BFB1-24DAE9B76C72", "06AB932B-A43A-440E-8630-FC85861573D0" };
	private String[]	af_id_6 = { "126AA03D-4150-4F61-92F8-E2D2A4B4A1E1", "2FAF61BE-C59A-412E-AE25-D418DE2BF026", "D365262D-766B-4FE7-ACD5-1BE5C74C7953", "0D3F212E-A569-4184-BA66-4E007B4B0C80", "3EC4F573-2916-45CA-83C6-300216BAA06E", "7822D71A-CCCF-47C6-8974-0CBE90DEBB53" };
	private String[][]	af_ids = { null, af_id_2, af_id_3, af_id_4, af_id_5, af_id_6 };

	private byte[]	af_image_2c1 = { 0x08, 0x5B, 0x1E, 0x5C, 0x0F, 0x34, 0x24, 0x3B, 0x2C, 0x5B, 0x19, 0x7A, 0x0F, 0x1C, 0x50, 0x2E, 0x18, 0x49, 0x7E, 0x07, 0x5D, 0x4A, 0x19, 0x3F, 0x5D, 0x71, 0x2C, 0x1D, 0x0A, 0x24, 0x3C, 0x19, 0x2D, 0x1A, 0x0D, 0x5E, 0x46, 0x18, 0x4B, 0x70, 0x2D, 0x1C, 0x26, 0x08, 0x2C };
	private byte[]	af_image_2c2 = { 0x39, 0x0A, 0x5F, 0x5B, 0x59, 0x2E, 0x26, 0x4A, 0x1F, 0x14, 0x38, 0x38, 0x2B, 0x1F, 0x3D, 0x74, 0x79, 0x28, 0x1A, 0x2D, 0x3B, 0x58, 0x2F };
	private byte[][]	af_image_2 = { af_image_2c1, af_image_2c2 };
	private byte[]	af_image_3c1 = { 0x3E, 0x2E, 0x0B, 0x10, 0x44, 0x4D, 0x08, 0x4B, 0x2F, 0x0D, 0x3D, 0x1F, 0x45, 0x1D, 0x5A, 0x5D, 0x2B, 0x4E, 0x39, 0x53, 0x74, 0x5E, 0x0D, 0x77, 0x05, 0x3B, 0x3B, 0x24, 0x1F, 0x14, 0x77, 0x2F };
	private byte[][]	af_image_3 = { af_image_3c1 };
	private byte[]	af_image_4c1 = { 0x14, 0x77, 0x2F, 0x0A, 0x5E, 0x5B, 0x6C, 0x5F, 0x77, 0x34, 0x6C, 0x5D, 0x23, 0x52, 0x61, 0x44, 0x59, 0x47, 0x6C, 0x2A, 0x0C, 0x39, 0x0C, 0x26, 0x3D, 0x0A, 0x3A, 0x28, 0x24, 0x7E, 0x28, 0x4D, 0x5D, 0x60 };
	private byte[]	af_image_4c2 = { 0x5A, 0x1B, 0x49, 0x5D, 0x1B, 0x29, 0x0C, 0x24, 0x1F, 0x14, 0x77, 0x2F, 0x0A, 0x5E, 0x5B, 0x1E, 0x5C, 0x0F, 0x34, 0x24, 0x3B, 0x61, 0x2E, 0x13, 0x0B, 0x0E, 0x20, 0x3B, 0x64, 0x0A, 0x5A, 0x1B, 0x49, 0x5D, 0x1B, 0x29, 0x0C, 0x43, 0x6C, 0x4A, 0x13, 0x1B, 0x0A, 0x0A, 0x04, 0x4C };
	private byte[]	af_image_4c3 = { 0x0C, 0x7E, 0x3F, 0x0D, 0x2C, 0x47, 0x40, 0x47, 0x0A, 0x7D, 0x3E, 0x5B, 0x2E, 0x3B, 0x4D, 0x1E, 0x2C, 0x1B, 0x1C, 0x1F, 0x19, 0x24, 0x3B, 0x66, 0x0D, 0x29, 0x51, 0x18, 0x6A, 0x0F, 0x48 };
	private byte[][]	af_image_4 = { af_image_4c1, af_image_4c2, af_image_4c3 };
	private byte[]	af_image_5c1 = { 0x58, 0x21, 0x0D, 0x2F, 0x01, 0x28, 0x2F, 0x4F, 0x68, 0x5F, 0x1A, 0x0C, 0x28, 0x5E, 0x4E, 0x2F, 0x72, 0x2C, 0x39, 0x2E, 0x22, 0x16, 0x64, 0x61, 0x1C, 0x08, 0x76, 0x1C };
	private byte[]	af_image_5c2 = { 0x1B, 0x0D, 0x2C, 0x67, 0x40, 0x47, 0x0A, 0x7D, 0x3E, 0x5B, 0x2E, 0x0F, 0x2A, 0x72, 0x45, 0x0E, 0x38, 0x0A, 0x29, 0x1F, 0x30, 0x78, 0x2E, 0x2D, 0x7C, 0x5F, 0x4D, 0x08, 0x08, 0x1B, 0x08, 0x39, 0x5B, 0x2E, 0x3B, 0x4D, 0x1E, 0x2C, 0x1B, 0x1C };
	private byte[][]	af_image_5 = { af_image_5c1, af_image_5c2 };
	private byte[]	af_image_6c1 = { 0x1D, 0x5A, 0x5D, 0x2B, 0x4E, 0x39, 0x53, 0x3D, 0x0A, 0x3A, 0x28, 0x24, 0x24, 0x3B, 0x2C, 0x5B, 0x19, 0x7A, 0x0F, 0x1C, 0x50, 0x2E };
	private byte[]	af_image_6c2 = { 0x1B, 0x49, 0x5D, 0x1B, 0x29, 0x2F, 0x0D, 0x3D, 0x1F, 0x45, 0x1D, 0x5A, 0x5D, 0x2B, 0x4E, 0x39, 0x64, 0x0A, 0x5A, 0x1B, 0x49, 0x5D, 0x1B, 0x1B, 0x49, 0x5D, 0x1B, 0x29, 0x0C, 0x24, 0x1F, 0x23, 0x52, 0x61, 0x44, 0x59, 0x47, 0x6C, 0x2A };
	private byte[]	af_image_6c3 = { 0x0A, 0x5E, 0x5B, 0x77, 0x18, 0x17, 0x3C, 0x19, 0x44, 0x41, 0x5F, 0x29, 0x28, 0x7D, 0x3E, 0x5B, 0x2E, 0x3B, 0x4D, 0x1E, 0x2C, 0x1B, 0x1C, 0x04, 0x08, 0x0A, 0x20, 0x2D, 0x26, 0x7C, 0x38, 0x33, 0x41, 0x1B, 0x0A, 0x64, 0x42, 0x00 };
	private byte[][]	af_image_6 = { af_image_6c1, af_image_6c2, af_image_6c3 };
	private byte[][][]	af_images = { null, af_image_2, af_image_3, af_image_4, af_image_5, af_image_6 };

	private String[]	af_language_2 = { "fr", "gu", "id", "ms", "qu", "sg", "so", "uk", "vo", "zu" };
	private String[]	af_language_3 = { "tl", "uz", "km", "az", "ta", "yi" };
	private String[]	af_language_4 = { "ka", "af", "mn" };
	private String[]	af_language_5 = { "to", "so", "ps", "pt", "qu", "ja", "hi", "no", "oc"};
	private String[]	af_language_6 = { "en", "it" };
	private String[][]	af_languages = { null, af_language_2, af_language_3, af_language_4, af_language_5, af_language_6 };

	private String[]	af_nchar6_2 = { "Driver", "Action", "Entity", "Vector" };
	private String[]	af_nchar6_3 = { "ListUI", "Line2D", "Parser", "Helper", "Policy" };
	private String[]	af_nchar6_4 = { "RSAKey", "Remote", "Object", "Member", "MacSpi", "Kernel" };
	private String[]	af_nchar6_5 = { "Insets", "Inline", "DynAny" };
	private String[]	af_nchar6_6 = { "Policy", "Server", "DSAKey", "Filter", "NVList" };
	private String[][]	af_nchar6s = { null, af_nchar6_2, af_nchar6_3, af_nchar6_4, af_nchar6_5, af_nchar6_6 };

	private String[]	af_nvarchar256_2 = { "中国剰余定理 (CRT) の情報の値を使った、RSA 非公開鍵 (PKCS#1 標準の定義による) のインタフェースです。", "FloatControl.Type 内部クラスのインスタンスは、浮動小数点型のコントロールの種類を識別します。静的インスタンスは共通タイプに対して提供されます。", "CipherOutputStream は OutputStream と Cipher で構成されているので、write() メソッドはまずデータを処理してから基本となる OutputStream に書き込みます。" };
	private String[]	af_nvarchar256_3 = { "org.omg.CORBA.Any 値を、動的に解釈 (トラバーサル) および構築できるようにします。", "Any の値は、DynAny オブジェクトを通して動的に解釈 (トラバース) および構築することができます。", "オブジェクト直列化ストリームに書き込まれる定数です。", "JScrollPane コンポーネントとともに使われる定数です。"};
	private String[]	af_nvarchar256_4 = { "このクラスは、ExemptionMechanism クラスの Service Provider Interface (SPI) を定義します。", "パーサファクトリの構成に問題が存在する場合にスローされます。", "swing コンポーネントに HTML ビューを提供します。", "MenuBarUI のデフォルトの Look & Feel による実装です。この実装は、ビューとコントローラを統一したものです。", "関連する JEditorPane のフォントファミリを設定するアクションです。"};
	private String[]	af_nvarchar256_5 = { "予測可能な繰り返し順序を持つ Set インタフェースのハッシュテーブルとリンクリストの実装です。", "ユーザ認証に失敗したことを通知します。", "このクラスは、OID (Universal Object Identifier) とそれに関連付けられた操作を表します。", "このクラスは、Highlighter インタフェースを実装します。ソリッドカラーで描画する簡単なハイライトペインタを実装します。", "たとえば、Shift キーを押すと VK_SHIFT キーコードを伴う KEY_PRESSED イベントが発生します。" };
	private String[]	af_nvarchar256_6 = { "このクラスは、「PKCS #5」標準で定義されている、パスワードベースの暗号化 (PBE) で使用されるパラメータのセットを指定します。", "Permissions の異種コレクションを表します。" };
	private String[][]	af_nvarchar256s = { null, af_nvarchar256_2, af_nvarchar256_3, af_nvarchar256_4, af_nvarchar256_5, af_nvarchar256_6 };

	private String[]	af_varchar128_2 = { "As noted above, this specification often refers to classes of the Java and Java 2 platforms.", "Other useful constructors, methods," };
	private String[]	af_varchar128_3 = { "Execute the test method expecting that an Exception of class fExpected or one of its subclasses will be thrown", "Each test runs in its own fixture so there can be no side effects among test runs. Here is an example:", "A TestResult collects the results of executing a test case." };
	private String[]	af_varchar128_4 = { "Those members declared in the interface.", "If a type name is of the form Q.Id, then Q must be either a type name or a package name." };
	private String[]	af_varchar128_5 = { "The fully qualified name of a primitive type is the keyword for that primitive type", "The fully qualified name of a named package.", "The fully qualified name of the type long is long.", "a first identifier that begins with a lowercase letter," };
	private String[]	af_varchar128_6 = { "For type int, the default value is zero, that is, 0.", "For type long, the default value is zero, that is, 0L.", "For type float, the default value is positive zero, that is, 0.0f.", "For type double, the default value is positive zero, that is, 0.0." };
	private String[][]	af_varchar128s = { null, af_varchar128_2, af_varchar128_3, af_varchar128_4, af_varchar128_5, af_varchar128_6 };

	private String[]	af_ntext_2 = { "AWT パッケージの変更は、グラフィカルユーザインタフェースを表示するプログラムの堅牢さ、動作、およびパフォーマンスの向上に重点が置かれています。 これまでの実装は、新しい「フォーカスアーキテクチャ」に置き換わりました。", "ここではプラットフォームが異なるために生じるフォーカス関連のバグや、AWT コンポーネントと Swing コンポーネント間の非互換性について説明します。", "新しい持続モデルは、Bean のグラフと持続性形式の変換を処理するために設計されました。" };
	private String[]	af_ntext_3 = { "このクラスは、チャネルの非同期クローズと割り込みを実装するのに必要な低レベルの機構をカプセル化します。", "ViewportUI の結合に使用する多重 UI です。", "特に断らない限り、このクラスで定義されているメソッドはスレッドセーフではありません。", "この例外は、リンクを解決または構築するときに無効なリンクが見つかった場合にスローされます。" };
	private String[]	af_ntext_4 = { "ColorSelectionModel の汎用実装です。", "出力を通常の OutputStream に書き込む ImageOutputStream の実装です。メモリバッファには、少なくとも破棄位置と現在の書き込み位置との間のデータがキャッシュされます。OutputStream を使用するのはコンストラクタのみなので、このクラスは読み込み、変更、または書き込み操作に使用できない場合があります。読み込みは、キャッシュに書き込み済みでまだフラッシュされていないストリーム部分についてのみ行うことができます。" };
	private String[]	af_ntext_5 = { "このクラスは、アイデンティティを表します。アイデンティティは、人々、会社、組織などの実際の世界のオブジェクトで、そのアイデンティティがその公開鍵を使用して認証できるものです。アイデンティティはまた、デーモンスレッドやスマートカードのようなより抽象的、あるいはより具象的な構成概念であってもかまいません。", "", "" };
	private String[]	af_ntext_6 = { "関連したパラメータを使って Diffie-Hellman 公開鍵を指定します。", "一般キーストア例外です。", "パイプによる出力ストリームをパイプによる入力ストリームに接続すると、通信パイプを作成できます。パイプによる出力ストリームは、パイプの送端です。一般的に、PipedOutputStream オブジェクトにデータを書き込むスレッドと、接続された PipedInputStream オブジェクトからデータを読み込むスレッドは別々です。", "推奨できません。" };
	private String[][]	af_ntexts = { null, af_ntext_2, af_ntext_3, af_ntext_4, af_ntext_5, af_ntext_6 };

	private String[]	af_ntext_compressed_2 = { "Java Web Start 製品は、J2SE 1.4.0 に同梱されている新しいアプリケーション配備技術です。Java Web Start を使うと、Web ページのリンクをクリックするだけでアプリケーションを起動できます。 そのアプリケーションがコンピュータに存在しない場合は、Java Web Start により、必要なすべてのファイルが自動的にダウンロードされます。", "また、どの方法でアプリケーションを起動しても、常に最新バージョンのアプリケーションが起動されます。" };
	private String[]	af_ntext_compressed_3 = { "リバーブは、部屋の壁、天井、および床の音の反射をシミュレーションします。部屋の大きさや、部屋の表面の素材がサウンドを吸収または反射する度合によって、サウンドは消滅するまでに長時間跳ね返ることがあります。", "ReverbType によって提供されるリバーブパラメータは、アーリーリフレクションの遅延時間と強度、レイトリフレクションの遅延時間と強度、および全体的な減衰時間から構成されています。" };
	private String[]	af_ntext_compressed_4 = { "IDL の fixed 型に関連している DynAny オブジェクトを表します。", "DynStruct オブジェクトによって、IDL 構造体および例外値の操作がサポートされます。例外のメンバは、構造体のメンバと同じように扱われます。", "IDL-to-Java コンパイラ (ポータブル) バージョン 3.1 により ../../../../src/share/classes/org/omg/CosNaming/nameservice.idl から生成された org/omg/CosNaming/NamingContextPackage/CannotProceed.java。" };
	private String[]	af_ntext_compressed_5 = { "DefaultPersistenceDelegate は、抽象クラス PersistenceDelegate の固定実装であり、情報の得られないクラスがデフォルトで使用する委譲です。JavaBeans 規約に従うクラスは、DefaultPersistenceDelegate により、特別な構成を行わなくても、バージョンの違いによる影響を受けない公開 API ベースの持続性を確保することができます。" };
	private String[]	af_ntext_compressed_6 = { "Paper を生成する際、アプリケーションが用紙サイズとイメージング可能領域が互換性を持つことを確認します。たとえば、用紙サイズが 11 x 17 から 8.5 x 11 に変更された場合、印刷対象領域がページに適合するように、アプリケーションはイメージング可能領域を減少させる必要がある場合があります。", "Paper クラスは、用紙の物理的な性質を記述します。" };
	private String[][]	af_ntext_compresseds = { null, af_ntext_compressed_2, af_ntext_compressed_3, af_ntext_compressed_4, af_ntext_compressed_5, af_ntext_compressed_6 };

	private String[]	af_fulltext_2 = { "新しい持続モデルは、Bean のグラフと持続性形式の変換を処理するために設計されました。 新しい API は、プロパティを表すテキストとして JavaBeans コンポーネントのグラフのアーカイブを作成するのに適しています。", "JDBC 3.0 API は、パッケージの java.sql と javax.sql で構成されており、Java プログラミング言語からの一般的なデータアクセスを提供します。" };
	private String[]	af_fulltext_3 = { "このクラスは java.util.Vector API を柔軟に実装します。バージョン 1.1.x の java.util.Vector を実装しますが、コレクションクラスはサポートせず、変更発生時には ListDataListener に通知します。現在は Vector に委譲され、今後のリリースでは実際にコレクションが実装されます。", "ポップアップメニューおよびメニューバーのデフォルトのレイアウトマネージャです。" };
	private String[]	af_fulltext_4 = { "DataInput インタフェースを拡張してオブジェクトの読み込みができるようにします。", "要求や応答によって暗黙的に渡されるサービス固有の情報です。サービスコンテキストは、サービス ID と関連データから構成されます。", "読み込みに続いて書き込みするだけで、すべてのイメージを変換、つまりもともと保存されていたイメージとは違う形式で書き込むことができます。ただし、形式の違いのため、この処理中にデータが損失する可能性があります。" };
	private String[]	af_fulltext_5 = { "", "ImageOutputStream のサービスプロバイダインタフェース (SPI) です。サービスプロバイダインタフェースの詳細は、IIORegistry クラスのクラスコメントを参照してください。", "CSS 属性を保持する AttributeSet のキーとして使用される定義です。これは閉じたセット (仕様によって厳密に定義されているセット) なので、最終的なものであり、拡張することはできません。" };
	private String[]	af_fulltext_6 = { "ストリームが読み込み可能な状態かどうかを通知します。InputStreamReader は、入力バッファが空白ではないか、または基本となるバイトストリームからバイトデータを読み込める状態のときに読み込み可能です。", null, "ImagingOpException は、BufferedImageOp または RasterOp のフィルタメソッドのうちの 1 つがイメージを処理できない場合にスローされます。" };
	private String[][]	af_fulltexts = { null, af_fulltext_2, af_fulltext_3, af_fulltext_4, af_fulltext_5, af_fulltext_6 };

	private byte[]		af_binary50_2c1 = { 0x3A, 0x29, 0x11, 0x7B, 0x38, 0x39, 0x2E, 0x09, 0x1A, 0x4E, 0x4C, 0x40, 0x7B, 0x29, 0x59, 0x0C, 0x76, 0x7F, 0x7F, 0x0C, 0x3F, 0x1E, 0x71, 0x25, 0x18, 0x1D, 0x7D, 0x6F, 0x2F, 0x2A, 0x73, 0x28, 0x09, 0x41, 0x5A, 0x1A, 0x5D, 0x24, 0x2C, 0x0A, 0x51, 0x2E, 0x2B, 0x5E, 0x0A, 0x2C, 0x39, 0x0F, 0x48, 0x2A };
	private byte[]		af_binary50_2c2 = { 0x6F, 0x1F, 0x32, 0x28, 0x1D, 0x1E, 0x4A, 0x33, 0x1A, 0x4A, 0x0D, 0x1B, 0x0F, 0x3D, 0x4E, 0x33, 0x42, 0x28, 0x1B, 0x56, 0x1E, 0x5D, 0x6C, 0x1C, 0x49, 0x3D, 0x0D, 0x27, 0x1C, 0x28, 0x04, 0x0A, 0x2E, 0x0A, 0x3B, 0x15, 0x18, 0x33, 0x6C, 0x5E, 0x69, 0x1F, 0x2B, 0x07, 0x33, 0x1A, 0x00, 0x05, 0x4F, 0x22 };
	private byte[][]	af_binary50_2 = { af_binary50_2c1, af_binary50_2c2 };
	private byte[]		af_binary50_3c1 = { 0x3A, 0x6C, 0x5F, 0x77, 0x34, 0x6C, 0x5D, 0x23, 0x52, 0x5B, 0x19, 0x7A, 0x0F, 0x1C, 0x50, 0x2E, 0x0D, 0x2C, 0x67, 0x40, 0x47, 0x0A, 0x7D, 0x3E, 0x5B, 0x2E, 0x0F, 0x2A, 0x72, 0x38, 0x1A, 0x48, 0x28, 0x1E, 0x55, 0x44, 0x3B, 0x7A, 0x34, 0x1A, 0x20, 0x40, 0x79, 0x0A, 0x3B, 0x29, 0x5F, 0x4C, 0x28, 0x37 };
	private byte[]		af_binary50_3c2 = { 0x1A, 0x14, 0x77, 0x2F, 0x0A, 0x5E, 0x5B, 0x6C, 0x6B, 0x07, 0x44, 0x09, 0x74, 0x3A, 0x2F, 0x18, 0x27, 0x4B, 0x0F, 0x6F, 0x3D, 0x24, 0x0C, 0x7E, 0x3F, 0x3E, 0x2E, 0x0B, 0x10, 0x44, 0x4D, 0x08, 0x4B, 0x2F, 0x71, 0x31, 0x20, 0x3B, 0x64, 0x0A, 0x5A, 0x1B, 0x49, 0x5D, 0x1B, 0x29, 0x0C, 0x43, 0x6C, 0x4A };
	private byte[]		af_binary50_3c3 = { 0x7B, 0x24, 0x3B, 0x66, 0x0D, 0x29, 0x51, 0x18, 0x6A, 0x0F, 0x0E, 0x38, 0x0A, 0x29, 0x1F, 0x30, 0x78, 0x2E, 0x2D, 0x19, 0x1E, 0x09, 0x4A, 0x4E, 0x6C, 0x2A, 0x0C, 0x39, 0x0C, 0x26, 0x3D, 0x3D, 0x5A, 0x2C, 0x21, 0x7C, 0x08, 0x3C, 0x18, 0x7F, 0x76, 0x75, 0x0D, 0x1D, 0x4F, 0x3A, 0x29, 0x38, 0x66, 0x51 };
	private byte[][]	af_binary50_3 = { af_binary50_3c1, af_binary50_3c2, af_binary50_3c3 };
	private byte[]		af_binary50_4c1 = { 0x33, 0x0A, 0x7D, 0x3E, 0x5B, 0x2E, 0x0F, 0x14, 0x77, 0x2F, 0x0A, 0x5E, 0x5B, 0x6C, 0x6B, 0x29, 0x5F, 0x4C, 0x2F, 0x72, 0x2C, 0x39, 0x2E, 0x22, 0x16, 0x64, 0x61, 0x1C, 0x08, 0x5F, 0x3F, 0x1A, 0x3D, 0x2F, 0x0D, 0x2C, 0x37, 0x40, 0x47, 0x0A, 0x7D, 0x3E, 0x5B, 0x2E, 0x3B, 0x4D, 0x1E, 0x2C, 0x1B, 0x1C };
	private byte[]		af_binary50_4c2 = { 0x40, 0x5B, 0x1E, 0x5C, 0x0F, 0x34, 0x24, 0x3B, 0x2C, 0x5B, 0x19, 0x4C, 0x2A, 0x28, 0x1A, 0x0A, 0x3A, 0x28, 0x24, 0x6A, 0x5B, 0x2C, 0x29, 0x73, 0x48, 0x20, 0x3B, 0x19, 0x19, 0x37, 0x64, 0x1A, 0x73, 0x33, 0x75, 0x18, 0x65, 0x61, 0x39, 0x0E, 0x43, 0x3F, 0x2B, 0x5B, 0x2B, 0x3B, 0x0A, 0x15, 0x30, 0x33 };
	private byte[][]	af_binary50_4 = { af_binary50_4c1, af_binary50_4c2 };
	private byte[]		af_binary50_5c1 = { 0x7F, 0x6C, 0x5D, 0x23, 0x52, 0x5B, 0x19, 0x7A, 0x0F, 0x1C, 0x50, 0x08, 0x4B, 0x2F, 0x2D, 0x0F, 0x72, 0x72, 0x2A, 0x2F, 0x35, 0x5C, 0x4E, 0x18, 0x39, 0x13, 0x48, 0x36, 0x28, 0x10, 0x3D, 0x28, 0x32, 0x09, 0x3A, 0x1E, 0x5C, 0x40, 0x21, 0x0D, 0x3D, 0x1F, 0x45, 0x1D, 0x5A, 0x5D, 0x2B, 0x4E, 0x39, 0x53 };
	private byte[]		af_binary50_5c2 = { 0x11, 0x77, 0x2F, 0x0A, 0x5E, 0x5B, 0x6C, 0x6B, 0x07, 0x44, 0x09, 0x74, 0x3A, 0x2F, 0x18, 0x27, 0x1B, 0x0D, 0x2C, 0x37, 0x40, 0x47, 0x0A, 0x7D, 0x3E, 0x5B, 0x2E, 0x3B, 0x4D, 0x1E, 0x2C, 0x1B, 0x24, 0x0C, 0x7E, 0x3F, 0x3E, 0x2E, 0x0B, 0x10, 0x44, 0x4D, 0x08, 0x4B, 0x2F, 0x2D, 0x0F, 0x77, 0x05, 0x3B };
	private byte[]		af_binary50_5c3 = { 0x4C, 0x24, 0x0C, 0x7E, 0x3F, 0x3E, 0x2E, 0x0B, 0x10, 0x44, 0x4D, 0x3B, 0x24, 0x1F, 0x14, 0x77, 0x2F, 0x0A, 0x5E, 0x5B, 0x6C, 0x6B, 0x24, 0x3B, 0x2C, 0x5B, 0x19, 0x7A, 0x0F, 0x1C, 0x50, 0x0A, 0x5E, 0x5B, 0x6C, 0x5F, 0x77, 0x34, 0x6C, 0x5D, 0x23, 0x52, 0x61, 0x74, 0x3A, 0x2F, 0x18, 0x27, 0x4B, 0x7F };
	private byte[][]	af_binary50_5 = { af_binary50_5c1, af_binary50_5c2, af_binary50_5c3 };
	private byte[]		af_binary50_6c1 = { 0x2E, 0x0D, 0x2C, 0x67, 0x40, 0x47, 0x0A, 0x29, 0x5F, 0x4C, 0x2F, 0x72, 0x2C, 0x39, 0x6B, 0x07, 0x44, 0x09, 0x18, 0x1D, 0x7D, 0x6F, 0x2F, 0x2A, 0x73, 0x28, 0x09, 0x41, 0x5A, 0x1A, 0x5D, 0x24, 0x2C, 0x0A, 0x51, 0x2E, 0x2B, 0x5E, 0x0A, 0x2C, 0x39, 0x0F, 0x48, 0x22, 0x16, 0x64, 0x61, 0x1C, 0x08, 0x44 };
	private byte[]		af_binary50_6c2 = { 0x3D, 0x5A, 0x2C, 0x21, 0x7C, 0x08, 0x3C, 0x18, 0x7F, 0x76, 0x75, 0x0D, 0x1D, 0x4F, 0x6C, 0x5D, 0x23, 0x52, 0x5B, 0x19, 0x7A, 0x0F, 0x07, 0x44, 0x09, 0x74, 0x3A, 0x2F, 0x18, 0x27, 0x1B, 0x3E, 0x2E, 0x0B, 0x10, 0x44, 0x4D, 0x08, 0x4B, 0x2F, 0x2D, 0x0F, 0x1B, 0x0D, 0x2C, 0x37, 0x40, 0x47, 0x0A, 0x7D };
	private byte[][]	af_binary50_6 = { af_binary50_6c1, af_binary50_6c2 };
	private byte[][][]	af_binary50s = { null, af_binary50_2, af_binary50_3, af_binary50_4, af_binary50_5, af_binary50_6 };

	// テスト用の表を作成し、タプルの挿入を行う
	// 挿入したタプル数を返す
	private void createAndInsertTestTable(Connection	c) throws Exception
	{
		createTestTable(c);

		PreparedStatement	ps = null;
		String	query = "insert into t values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

		assertNotNull(ps = c.prepareStatement(query));

		for (int i = 0; i < f_int_not_nulls.length; i++) {

			int	columnIndex = 1;

			// f_int_not_null
			ps.setInt(columnIndex++, f_int_not_nulls[i].intValue());

			// f_int1
			ps.setInt(columnIndex++, f_int1s[i].intValue());

			// f_int2
			if (f_int2s[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else					ps.setInt(columnIndex++, f_int2s[i].intValue());

			// f_bigint
			if (f_bigints[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setLong(columnIndex++, f_bigints[i].longValue());

			// f_decimal
			if (f_decimals[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setBigDecimal(columnIndex++, f_decimals[i]);

			// f_char8_not_null
			ps.setString(columnIndex++, f_char8_not_nulls[i]);

			// f_char8
			if (f_char8s[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setString(columnIndex++, f_char8s[i]);

			// f_float
			if (f_floats[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setDouble(columnIndex++, f_floats[i].doubleValue());

			// f_datetime
			if (f_datetimes[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setTimestamp(columnIndex++, f_datetimes[i]);

			// f_id
			if (f_ids[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else					ps.setString(columnIndex++, f_ids[i]);

			// f_image
			if (f_images[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setBytes(columnIndex++, f_images[i]);

			// f_language
			if (f_languages[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setString(columnIndex++, f_languages[i]);

			// f_nchar6
			if (f_nchar6s[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setString(columnIndex++, f_nchar6s[i]);

			// f_nvarchar256
			if (f_nvarchar256s[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else							ps.setString(columnIndex++, f_nvarchar256s[i]);

			// f_varchar128
			if (f_varchar128s[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else							ps.setString(columnIndex++, f_varchar128s[i]);

			// f_ntext
			if (f_ntexts[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setString(columnIndex++, f_ntexts[i]);

			// f_ntext_compressed
			if (f_ntext_compresseds[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else								ps.setString(columnIndex++, f_ntext_compresseds[i]);

			// f_fulltext
			if (f_fulltexts[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setString(columnIndex++, f_fulltexts[i]);

			// f_binary50
			if (f_binary50s[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setBytes(columnIndex++, f_binary50s[i]);

			// f_blob
			if (f_blobs[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else					ps.setBytes(columnIndex++, f_blobs[i]);

			// f_nclob
			if (f_nclobs[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setString(columnIndex++, f_nclobs[i]);

			// af_int
			if (af_ints[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else					ps.setArray(columnIndex++, new TestArray(af_ints[i]));

			// af_bigint
			if (af_bigints[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setArray(columnIndex++, new TestArray(af_bigints[i]));

			// af_decimal
			if (af_decimals[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setArray(columnIndex++, new TestArray(af_decimals[i]));

			// af_char8
			if (af_char8s[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setArray(columnIndex++, new TestArray(af_char8s[i]));

			// af_float
			if (af_floats[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setArray(columnIndex++, new TestArray(af_floats[i]));

			// af_datetime
			if (af_datetimes[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else							ps.setArray(columnIndex++, new TestArray(af_datetimes[i]));

			// af_id
			if (af_ids[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else					ps.setArray(columnIndex++, new TestArray(af_ids[i]));

			// af_image
			if (af_images[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setArray(columnIndex++, new TestArray(af_images[i]));

			// af_language
			if (af_languages[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else							ps.setArray(columnIndex++, new TestArray(af_languages[i]));

			// af_nchar6
			if (af_nchar6s[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setArray(columnIndex++, new TestArray(af_nchar6s[i]));

			// af_nvarchar256
			if (af_nvarchar256s[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else							ps.setArray(columnIndex++, new TestArray(af_nvarchar256s[i]));

			// af_varchar128
			if (af_varchar128s[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else							ps.setArray(columnIndex++, new TestArray(af_varchar128s[i]));

			// af_ntext
			if (af_ntexts[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else						ps.setArray(columnIndex++, new TestArray(af_ntexts[i]));

			// af_ntext_compressed
			if (af_ntext_compresseds[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else									ps.setArray(columnIndex++, new TestArray(af_ntext_compresseds[i]));

			// af_fulltext
			if (af_fulltexts[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else							ps.setArray(columnIndex++, new TestArray(af_fulltexts[i]));

			// af_binary50
			if (af_binary50s[i] == null)	ps.setNull(columnIndex++, Types.NULL);
			else							ps.setArray(columnIndex++, new TestArray(af_binary50s[i]));

			ps.executeUpdate();
		}

		ps.close();
	}

	// テスト用の表を作成し、タプルの挿入を行う
	// 挿入したタプル数を返す
	private void createSimpleTable(	Connection	c,
									int			numTuple) throws Exception
	{
		if (numTuple > 10) throw new NotSupported();

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		s.executeUpdate("create table t (id int, title nvarchar(256), primary key(id))");

		String[]	queries = {
			"	insert into t values ( 1, 'RICOH')			",
			"	insert into t values ( 2, 'Sydney')			",
			"	insert into t values ( 3, 'Java')			",
			"	insert into t values ( 4, 'JDBC')			",
			"	insert into t values ( 5, 'DATABASE')		",
			"	insert into t values ( 6, 'C++')			",
			"	insert into t values ( 7, 'ricoh')			",
			"	insert into t values ( 8, 'SYDNEY')			",
			"	insert into t values ( 9, 'database')		",
			"	insert into t values (10, 'jdbc driver')	"	};

		for (int i = 0; i < numTuple; i++) s.executeUpdate(queries[i]);

		s.close();
	}

	// テスト用の表を作成する
	// タプルの挿入は行わない
	// 挿入したタプル数を返す
	private void createSimpleTable(Connection	c) throws Exception
	{
		createSimpleTable(c, 0);
	}

	// テスト用の表を作成し、タプルの挿入を行う
	// 挿入したタプル数を返す
	private void createSimpleTable(	Connection	c,
									int[]		ids,
									String[]	titles) throws Exception
	{
		createSimpleTable(c);
		PreparedStatement	ps = null;
		assertNotNull(ps = c.prepareStatement("insert into t values (?, ?)"));

		for (int i = 0; i < ids.length; i++) {

			ps.setInt(1, ids[i]);
			ps.setString(2, titles[i]);
			ps.executeUpdate();
		}

		ps.close();
	}

	private void dropTestTable(Connection	c) throws Exception
	{
		Statement	s = null;
		assertNotNull(s = c.createStatement());
		s.executeUpdate("drop table t");
		s.close();
	}

	private void assertGetArray(	ResultSet		rs,
									int				columnIndex,
									String			columnName,
									Object			expected,
									SQLException	e) throws Exception
	{
		if (expected == null) {
			if (columnName == null)	assertNull(rs.getArray(columnIndex));
			else					assertNull(rs.getArray(columnName));
			assertTrue(rs.wasNull());
		} else {
			if (e != null) {
				boolean	caught = false;
				try {
					if (columnName == null)	rs.getArray(columnIndex);
					else					rs.getArray(columnName);
				} catch (SQLException	sqle) {
					caught = true;
					assertEquals(e.getSQLState(), sqle.getSQLState());
				}
				assertTrue(caught);
			} else {
				Array	ary = null;
				if (columnName == null)	assertNotNull(ary = rs.getArray(columnIndex));
				else					assertNotNull(ary = rs.getArray(columnName));
				assertFalse(rs.wasNull());
				ResultSet	aryrs = ary.getResultSet();
				switch (ary.getBaseType()) {
				case Types.INTEGER:	// INT 配列の要素のチェック
					{
						Integer[]	exp = (Integer[])expected;
						for (int i = 0; i < exp.length; i++) {
							assertTrue(aryrs.next());
							assertEquals(i + 1, aryrs.getInt(1));				// 要素のインデックス
							assertEquals(exp[i].intValue(), aryrs.getInt(2));	// 要素の値
						}
					}
					break;
				case Types.BIGINT:	// BIGINT 配列の要素のチェック
					{
						Long[]	exp = (Long[])expected;
						for (int i = 0; i < exp.length; i++) {
							assertTrue(aryrs.next());
							assertEquals(i + 1, aryrs.getInt(1));				// 要素のインデックス
							assertEquals(exp[i].longValue(), aryrs.getLong(2));	// 要素の値
						}
					}
					break;
				case Types.DECIMAL:	// DECIMAL 配列の要素のチェック
					{
						BigDecimal[]	exp = (BigDecimal[])expected;
						for (int i = 0; i < exp.length; i++) {
							assertTrue(aryrs.next());
							assertEquals(i + 1, aryrs.getInt(1));			// 要素のインデックス
							assertEquals(exp[i].doubleValue(), aryrs.getBigDecimal(2).doubleValue());	// 要素の値
						}
					}
					break;
				case Types.CHAR:	// CHAR 配列などの要素のチェック
				case Types.VARCHAR:	// NVARCHAR 配列などの要素のチェック
				case Types.OTHER:	// LANGUAGE 配列の要素のチェック
					{
						String[]	exp = (String[])expected;
						for (int i = 0; i < exp.length; i++) {
							assertTrue(aryrs.next());
							assertEquals(i + 1, aryrs.getInt(1));		// 要素のインデックス
							assertEquals(exp[i], aryrs.getString(2));	// 要素の値
						}
					}
					break;
				case Types.DOUBLE:	// FLOAT 配列の要素のチェック
					{
						Double[]	exp = (Double[])expected;
						for (int i = 0; i < exp.length; i++) {
							assertTrue(aryrs.next());
							assertEquals(i + 1, aryrs.getInt(1));					// 要素のインデックス
							assertEquals(exp[i].doubleValue(), aryrs.getDouble(2));	// 要素の値
						}
					}
					break;
				case Types.TIMESTAMP:	// DATETIME 配列の要素のチェック
					{
						Timestamp[]	exp = (Timestamp[])expected;
						for (int i = 0; i < exp.length; i++) {
							assertTrue(aryrs.next());
							assertEquals(i + 1, aryrs.getInt(1));								// 要素のインデックス
							assertEquals(exp[i].getTime(), aryrs.getTimestamp(2).getTime());	// 要素の値
						}
					}
					break;
				case Types.VARBINARY:	// IMAGE 配列の要素のチェック
				case Types.BINARY:		// BINARY 配列の要素のチェック
					{
						byte[][]	exp = (byte[][])expected;
						for (int i = 0; i < exp.length; i++) {
							assertTrue(aryrs.next());
							assertEquals(i + 1, aryrs.getInt(1));		// 要素のインデックス
							assertEquals(exp[i], aryrs.getBytes(2));	// 要素の値
						}
					}
					break;
				default:
					System.out.println("??? assertGetArray");
					System.out.println("ary.getBaseType() = " + ary.getBaseType());
					break;
				}
				assertFalse(aryrs.next());
				aryrs.close();
			}
		}
	}

	private void assertGetArray(	ResultSet		rs,
									int				columnIndex,
									Object			expected,
									SQLException	e) throws Exception
	{
		assertGetArray(rs, columnIndex, null, expected, e);
	}

	private void assertGetArray(	ResultSet	rs,
									int			columnIndex,
									Object		expected) throws Exception
	{
		assertGetArray(rs, columnIndex, expected, null);
	}

	private void assertGetArray(	ResultSet		rs,
									String			columnName,
									Object			expected,
									SQLException	e) throws Exception
	{

		assertGetArray(rs, 0, columnName, expected, e);
	}

	private void assertGetArray(	ResultSet	rs,
									String		columnName,
									Object		expected) throws Exception
	{

		assertGetArray(rs, 0, columnName, expected, null);
	}

	private void assertGetBigDecimal(	ResultSet		rs,
										int				columnIndex,
										String			columnName,
										int				scale,
										Object			expected,
										SQLException	e) throws Exception
	{
		if (expected == null) {
			if (columnName == null)	{
				if (scale < 0)	assertNull(rs.getBigDecimal(columnIndex));
				else			assertNull(rs.getBigDecimal(columnIndex, scale));
			} else {
				if (scale < 0)	assertNull(rs.getBigDecimal(columnName));
				else			assertNull(rs.getBigDecimal(columnName, scale));
			}
			assertTrue(rs.wasNull());
		} else {
			if (e != null) {
				boolean	caught = false;
				try {
					if (columnName == null)	{
						if (scale < 0)	rs.getBigDecimal(columnIndex);
						else			rs.getBigDecimal(columnIndex, scale);
					} else {
						if (scale < 0)	rs.getBigDecimal(columnName);
						else			rs.getBigDecimal(columnName, scale);
					}
				} catch (SQLException	sqle) {
					caught = true;
					assertEquals(e.getSQLState(), sqle.getSQLState());
				}
				assertTrue(caught);
			} else {
				BigDecimal	actual = null;
				if (columnName == null)	{
					if (scale < 0)	assertNotNull(actual = rs.getBigDecimal(columnIndex));
					else			assertNotNull(actual = rs.getBigDecimal(columnIndex, scale));
				} else {
					if (scale < 0)	assertNotNull(actual = rs.getBigDecimal(columnName));
					else			assertNotNull(actual = rs.getBigDecimal(columnName, scale));
				}
				assertFalse(rs.wasNull());
				String	expectedClass = expected.getClass().getName();
				java.math.BigInteger	bi = null;
				BigDecimal	exp = null;
				if (expectedClass.compareTo("java.lang.String") == 0) {
					exp = new BigDecimal((String)expected);
					if (scale >= 0) exp = exp.setScale(scale, BigDecimal.ROUND_HALF_EVEN);
				} else if (expectedClass.compareTo("java.lang.Integer") == 0) {
					bi = java.math.BigInteger.valueOf(((Integer)expected).longValue());
					exp = new BigDecimal(bi);
					if (scale >= 0) exp = exp.setScale(scale, BigDecimal.ROUND_HALF_EVEN);
				} else if (expectedClass.compareTo("java.lang.Long") == 0) {
					bi = java.math.BigInteger.valueOf(((Long)expected).longValue());
					exp = new BigDecimal(bi);
					if (scale >= 0) exp = exp.setScale(scale, BigDecimal.ROUND_HALF_EVEN);
				} else if (expectedClass.compareTo("java.math.BigDecimal") == 0) {
					exp = (BigDecimal)expected;
					if (scale >= 0) exp = exp.setScale(scale, BigDecimal.ROUND_HALF_EVEN);
				} else if (expectedClass.compareTo("java.lang.Double") == 0 ) {
					exp = new BigDecimal(((Double)expected).toString());
					if (scale >= 0) exp = exp.setScale(scale, BigDecimal.ROUND_HALF_EVEN);
				} else {
					System.out.println("??? assertGetBigDecimal");
					System.out.println("expectedClass = expectedClass");
				}
				assertEquals(exp.doubleValue(), actual.doubleValue());
			}
		}
	}

	private void assertGetBigDecimal(	ResultSet		rs,
										int				columnIndex,
										Object			expected,
										SQLException	e) throws Exception
	{
		assertGetBigDecimal(rs, columnIndex, null, -1, expected, e);
	}

	private void assertGetBigDecimal(	ResultSet		rs,
										int				columnIndex,
										int				scale,
										Object			expected,
										SQLException	e) throws Exception
	{
		assertGetBigDecimal(rs, columnIndex, null, scale, expected, e);
	}

	private void assertGetBigDecimal(	ResultSet	rs,
										int			columnIndex,
										Object		expected) throws Exception
	{
		assertGetBigDecimal(rs, columnIndex, null, -1, expected, null);
	}

	private void assertGetBigDecimal(	ResultSet	rs,
										int			columnIndex,
										int			scale,
										Object		expected) throws Exception
	{
		assertGetBigDecimal(rs, columnIndex, null, scale, expected, null);
	}

	private void assertGetBigDecimal(	ResultSet		rs,
										String			columnName,
										Object			expected,
										SQLException	e) throws Exception
	{

		assertGetBigDecimal(rs, 0, columnName, -1, expected, e);
	}

	private void assertGetBigDecimal(	ResultSet		rs,
										String			columnName,
										int				scale,
										Object			expected,
										SQLException	e) throws Exception
	{

		assertGetBigDecimal(rs, 0, columnName, scale, expected, e);
	}

	private void assertGetBigDecimal(	ResultSet	rs,
										String		columnName,
										Object		expected) throws Exception
	{

		assertGetBigDecimal(rs, 0, columnName, -1, expected, null);
	}

	private void assertGetBigDecimal(	ResultSet	rs,
										String		columnName,
										int			scale,
										Object		expected) throws Exception
	{

		assertGetBigDecimal(rs, 0, columnName, scale, expected, null);
	}

	private void assertGetBinaryStream(	ResultSet		rs,
										int				columnIndex,
										String			columnName,
										Object			expected,
										SQLException	e) throws Exception
	{
		if (expected == null) {
			if (columnName == null)	assertNull(rs.getBinaryStream(columnIndex));
			else					assertNull(rs.getBinaryStream(columnName));
			assertTrue(rs.wasNull());
		} else {
			if (e != null) {
				boolean	caught = false;
				try {
					if (columnName == null)	rs.getBinaryStream(columnIndex);
					else					rs.getBinaryStream(columnName);
				} catch (SQLException	sqle) {
					caught = true;
					assertEquals(e.getSQLState(), sqle.getSQLState());
				}
				assertTrue(caught);
			} else {
				java.io.InputStream	stream = null;
				if (columnName == null)	assertNotNull(stream = rs.getBinaryStream(columnIndex));
				else					assertNotNull(stream = rs.getBinaryStream(columnName));
				assertFalse(rs.wasNull());
				assertNotNull(stream);
				int	len = stream.available();
				byte[]	actual = new byte[len];
				stream.read(actual, 0, len);
				assertEquals((byte[])expected, actual);
				stream.close();
			}
		}
	}

	private void assertGetBinaryStream(	ResultSet		rs,
										int				columnIndex,
										Object			expected,
										SQLException	e) throws Exception
	{
		assertGetBinaryStream(rs, columnIndex, null, expected, e);
	}

	private void assertGetBinaryStream(	ResultSet	rs,
										int			columnIndex,
										Object		expected) throws Exception
	{
		assertGetBinaryStream(rs, columnIndex, expected, null);
	}

	private void assertGetBinaryStream(	ResultSet		rs,
										String			columnName,
										Object			expected,
										SQLException	e) throws Exception
	{

		assertGetBinaryStream(rs, 0, columnName, expected, e);
	}

	private void assertGetBinaryStream(	ResultSet	rs,
										String		columnName,
										Object		expected) throws Exception
	{

		assertGetBinaryStream(rs, columnName, expected, null);
	}

	private void assertGetBoolean(	ResultSet		rs,
									int				columnIndex,
									String			columnName,
									Object			expected,
									SQLException	e) throws Exception
	{
		if (expected == null) {
			if (columnName == null)	assertFalse(rs.getBoolean(columnIndex));
			else					assertFalse(rs.getBoolean(columnName));
			assertTrue(rs.wasNull());
		} else {
			if (e != null) {
				boolean	caught = false;
				try {
					if (columnName == null)	rs.getBoolean(columnIndex);
					else					rs.getBoolean(columnName);
				} catch (SQLException	sqle) {
					caught = true;
					assertEquals(e.getSQLState(), sqle.getSQLState());
				}
				assertTrue(caught);
			} else {
				boolean	actual = false;
				if (columnName == null)	actual = rs.getBoolean(columnIndex);
				else					actual = rs.getBoolean(columnName);
				assertFalse(rs.wasNull());
				String	expectedClass = expected.getClass().getName();
				if (expectedClass.compareTo("java.lang.String") == 0) {
					String	exp = ((String)expected).trim();
					if ((exp.length() == 0) ||
						(exp.compareToIgnoreCase("false") == 0) ||
						(exp.compareToIgnoreCase("0") == 0)) {
						assertFalse(actual);
					} else {
						assertTrue(actual);
					}
				} else if (expectedClass.compareTo("java.lang.Integer") == 0) {
					int	exp = ((Integer)expected).intValue();
					if (exp == 0)	assertFalse(actual);
					else			assertTrue(actual);
				} else if (expectedClass.compareTo("java.lang.Long") == 0) {
					long	exp = ((Long)expected).longValue();
					if (exp == 0L)	assertFalse(actual);
					else			assertTrue(actual);
				} else if (expectedClass.compareTo("java.lang.Double") == 0) {
					double	exp = ((Double)expected).doubleValue();
					if (exp == 0.0)	assertFalse(actual);
					else			assertTrue(actual);
				} else {
					System.out.println("??? assertGetBoolean");
					System.out.println("expectedClass = expectedClass");
				}
			}
		}
	}

	private void assertGetBoolean(	ResultSet		rs,
									int				columnIndex,
									Object			expected,
									SQLException	e) throws Exception
	{
		assertGetBoolean(rs, columnIndex, null, expected, e);
	}

	private void assertGetBoolean(	ResultSet	rs,
									int			columnIndex,
									Object		expected) throws Exception
	{
		assertGetBoolean(rs, columnIndex, expected, null);
	}

	private void assertGetBoolean(	ResultSet		rs,
									String			columnName,
									Object			expected,
									SQLException	e) throws Exception
	{

		assertGetBoolean(rs, 0, columnName, expected, e);
	}

	private void assertGetBoolean(	ResultSet	rs,
									String		columnName,
									Object		expected) throws Exception
	{

		assertGetBoolean(rs, columnName, expected, null);
	}

	private void assertGetByte(	ResultSet		rs,
								int				columnIndex,
								String			columnName,
								Object			expected,
								SQLException	e) throws Exception
	{
		if (expected == null) {
			if (columnName == null)	assertZero(rs.getByte(columnIndex));
			else					assertZero(rs.getByte(columnName));
			assertTrue(rs.wasNull());
		} else {
			if (e != null) {
				boolean	caught = false;
				try {
					if (columnName == null)	rs.getByte(columnIndex);
					else					rs.getByte(columnName);
				} catch (SQLException	sqle) {
					caught = true;
					assertEquals(e.getSQLState(), sqle.getSQLState());
				}
				assertTrue(caught);
			} else {
				byte	exp = -1;
				byte	actual = -1;
				if (columnName == null)	actual = rs.getByte(columnIndex);
				else					actual = rs.getByte(columnName);
				assertFalse(rs.wasNull());
				if (exp == actual) {
					// 同じ値だったらわざと exp を違う値にしておく
					if (actual == Byte.MIN_VALUE)	exp++;
					else							exp--;
				}
				String	expectedClass = expected.getClass().getName();
				if (expectedClass.compareTo("java.lang.String") == 0) {
					boolean	caught = false;
					try {
						exp = Byte.parseByte((String)expected);
					} catch (NumberFormatException	nfe) {
						caught = true;
					}
					assertFalse(caught);
				} else if (expectedClass.compareTo("java.lang.Integer") == 0) {
					exp = ((Integer)expected).byteValue();
				} else if (expectedClass.compareTo("java.lang.Long") == 0) {
					exp = ((Long)expected).byteValue();
				} else if (expectedClass.compareTo("java.lang.Double") == 0) {
					exp = ((Double)expected).byteValue();
				} else if (expectedClass.compareTo("java.math.BigDecimal") == 0) {
					exp = ((BigDecimal)expected).byteValue();
				} else {
					System.out.println("??? assertGetByte");
					System.out.println("expectedClass = expectedClass");
				}
				assertEquals(exp, actual);
			}
		}
	}

	private void assertGetByte(	ResultSet		rs,
								int				columnIndex,
								Object			expected,
								SQLException	e) throws Exception
	{
		assertGetByte(rs, columnIndex, null, expected, e);
	}

	private void assertGetByte(	ResultSet	rs,
								int			columnIndex,
								Object		expected) throws Exception
	{
		assertGetByte(rs, columnIndex, expected, null);
	}

	private void assertGetByte(	ResultSet		rs,
								String			columnName,
								Object			expected,
								SQLException	e) throws Exception
	{

		assertGetByte(rs, 0, columnName, expected, e);
	}

	private void assertGetByte(	ResultSet	rs,
								String		columnName,
								Object		expected) throws Exception
	{

		assertGetByte(rs, 0, columnName, expected, null);
	}

	private void assertGetBytes(	ResultSet		rs,
									int				columnIndex,
									String			columnName,
									Object			expected,
									SQLException	e) throws Exception
	{
		if (expected == null) {
			if (columnName == null)	assertNull(rs.getBytes(columnIndex));
			else					assertNull(rs.getBytes(columnName));
			assertTrue(rs.wasNull());
		} else {
			if (e != null) {
				boolean	caught = false;
				try {
					if (columnName == null)	rs.getBytes(columnIndex);
					else					rs.getBytes(columnName);
				} catch (SQLException	sqle) {
					caught = true;
					assertEquals(e.getSQLState(), sqle.getSQLState());
				}
				assertTrue(caught);
			} else {
				byte[]	actual = null;
				if (columnName == null)	assertNotNull(actual = rs.getBytes(columnIndex));
				else					assertNotNull(actual = rs.getBytes(columnName));
				assertFalse(rs.wasNull());
				assertEquals((byte[])expected, actual);
			}
		}
	}

	private void assertGetBytes(	ResultSet		rs,
									int				columnIndex,
									Object			expected,
									SQLException	e) throws Exception
	{
		assertGetBytes(rs, columnIndex, null, expected, e);
	}

	private void assertGetBytes(	ResultSet	rs,
									int			columnIndex,
									Object		expected) throws Exception
	{
		assertGetBytes(rs, columnIndex, expected, null);
	}

	private void assertGetBytes(	ResultSet		rs,
									String			columnName,
									Object			expected,
									SQLException	e) throws Exception
	{

		assertGetBytes(rs, 0, columnName, expected, e);
	}

	private void assertGetBytes(	ResultSet		rs,
									String			columnName,
									Object			expected) throws Exception
	{

		assertGetBytes(rs, columnName, expected, null);
	}

	private void assertGetCharacterStream(	ResultSet		rs,
											int				columnIndex,
											String			columnName,
											Object			expected,
											SQLException	e) throws Exception
	{
		if (expected == null) {
			if (columnName == null)	assertNull(rs.getCharacterStream(columnIndex));
			else					assertNull(rs.getCharacterStream(columnName));
			assertTrue(rs.wasNull());
		} else {
			if (e != null) {
				boolean	caught = false;
				try {
					if (columnName == null)	rs.getCharacterStream(columnIndex);
					else					rs.getCharacterStream(columnName);
				} catch (SQLException	sqle) {
					caught = true;
					assertEquals(e.getSQLState(), sqle.getSQLState());
				}
				assertTrue(caught);
			} else {
				java.io.Reader	reader = null;
				if (columnName == null)	assertNotNull(reader = rs.getCharacterStream(columnIndex));
				else					assertNotNull(reader = rs.getCharacterStream(columnName));
				assertFalse(rs.wasNull());
				assertNotNull(reader);
				String	expectedClass = expected.getClass().getName();
				if (expectedClass.compareTo("java.lang.String") == 0 ||
					expectedClass.compareTo("[B") == 0) {
					StringBuffer	actual = new StringBuffer("");
					int	numChars = 4096;
					char[]	buff = new char[numChars];
					int	offset = 0;
					while (true) {
						int	readLen = reader.read(buff, offset, numChars);
						if (readLen > 0) actual.append(buff, 0, readLen);
						if (readLen < numChars) break;
						offset += readLen;
					}
					if (expectedClass.compareTo("java.lang.String") == 0) {
						assertEquals((String)expected, actual.toString());
					} else {
						assertEquals((byte[])expected, actual.toString().getBytes());
					}
					reader.close();
				}
			}
		}
	}

	private void assertGetCharacterStream(	ResultSet		rs,
											int				columnIndex,
											Object			expected,
											SQLException	e) throws Exception
	{
		assertGetCharacterStream(rs, columnIndex, null, expected, e);
	}

	private void assertGetCharacterStream(	ResultSet	rs,
											int			columnIndex,
											Object		expected) throws Exception
	{
		assertGetCharacterStream(rs, columnIndex, expected, null);
	}

	private void assertGetCharacterStream(	ResultSet		rs,
											String			columnName,
											Object			expected,
											SQLException	e) throws Exception
	{

		assertGetCharacterStream(rs, 0, columnName, expected, e);
	}

	private void assertGetCharacterStream(	ResultSet	rs,
											String		columnName,
											Object		expected) throws Exception
	{

		assertGetCharacterStream(rs, columnName, expected, null);
	}

	private void assertGetDate(	ResultSet		rs,
								int				columnIndex,
								String			columnName,
								Object			expected,
								SQLException	e) throws Exception
	{
		if (expected == null) {
			if (columnName == null)	assertNull(rs.getDate(columnIndex));
			else					assertNull(rs.getDate(columnName));
			assertTrue(rs.wasNull());
		} else {
			if (e != null) {
				boolean	caught = false;
				try {
					if (columnName == null)	rs.getDate(columnIndex);
					else					rs.getDate(columnName);
				} catch (SQLException	sqle) {
					caught = true;
					assertEquals(e.getSQLState(), sqle.getSQLState());
				}
				assertTrue(caught);
			} else {
				java.sql.Date	actual = null;
				if (columnName == null)	assertNotNull(actual = rs.getDate(columnIndex));
				else					assertNotNull(actual = rs.getDate(columnName));
				assertFalse(rs.wasNull());
				String	expectedClass = expected.getClass().getName();
				if (expectedClass.compareTo("java.lang.String") == 0) {
					assertEquals((String)expected, actual.toString());
				} else if (expectedClass.compareTo("java.sql.Timestamp") == 0) {
					java.sql.Date	exp = new java.sql.Date(0L);
					exp.setTime(((java.sql.Timestamp)expected).getTime());
					assertEquals(exp.toString(), actual.toString());
				} else {
					System.out.println("??? assertGetDate");
					System.out.println("expectedClass = expectedClass");
				}
			}
		}
	}

	private void assertGetDate(	ResultSet		rs,
								int				columnIndex,
								Object			expected,
								SQLException	e) throws Exception
	{
		assertGetDate(rs, columnIndex, null, expected, e);
	}

	private void assertGetDate(	ResultSet	rs,
								int			columnIndex,
								Object		expected) throws Exception
	{
		assertGetDate(rs, columnIndex, expected, null);
	}

	private void assertGetDate(	ResultSet		rs,
								String			columnName,
								Object			expected,
								SQLException	e) throws Exception
	{

		assertGetDate(rs, 0, columnName, expected, e);
	}

	private void assertGetDate(	ResultSet	rs,
								String		columnName,
								Object		expected) throws Exception
	{

		assertGetDate(rs, columnName, expected, null);
	}

	private void assertGetDouble(	ResultSet		rs,
									int				columnIndex,
									String			columnName,
									Object			expected,
									SQLException	e) throws Exception
	{
		if (expected == null) {
			if (columnName == null)	assertEquals(0, rs.getDouble(columnIndex));
			else					assertEquals(0, rs.getDouble(columnName));
			assertTrue(rs.wasNull());
		} else {
			if (e != null) {
				boolean	caught = false;
				try {
					if (columnName == null)	rs.getDouble(columnIndex);
					else					rs.getDouble(columnName);
				} catch (SQLException	sqle) {
					caught = true;
					assertEquals(e.getSQLState(), sqle.getSQLState());
				}
				assertTrue(caught);
			} else {
				double	exp = -1.0;
				double	actual = -1.0;
				if (columnName == null)	actual = rs.getDouble(columnIndex);
				else					actual = rs.getDouble(columnName);
				assertFalse(rs.wasNull());
				if (exp == actual) {
					// 同じ値だったらわざと exp を違う値にしておく
					if (actual == Double.MIN_VALUE)	exp += 0.1;
					else							exp += 0.1;
				}
				String	expectedClass = expected.getClass().getName();
				if (expectedClass.compareTo("java.lang.String") == 0) {
					boolean	caught = false;
					try {
						exp = Double.parseDouble((String)expected);
					} catch (NumberFormatException	nfe) {
						caught = true;
					}
					assertFalse(caught);
				} else if (expectedClass.compareTo("java.lang.Integer") == 0) {
					exp = ((Integer)expected).doubleValue();
				} else if (expectedClass.compareTo("java.lang.Long") == 0) {
					exp = ((Long)expected).doubleValue();
				} else if (expectedClass.compareTo("java.lang.Double") == 0) {
					exp = ((Double)expected).doubleValue();
				} else if (expectedClass.compareTo("java.math.BigDecimal") == 0) {
					exp = ((BigDecimal)expected).doubleValue();
				} else {
					System.out.println("??? assertGetDouble");
					System.out.println("expectedClass = expectedClass");
				}
				assertEquals(exp, actual);
			}
		}
	}

	private void assertGetDouble(	ResultSet		rs,
									int				columnIndex,
									Object			expected,
									SQLException	e) throws Exception
	{
		assertGetDouble(rs, columnIndex, null, expected, e);
	}

	private void assertGetDouble(	ResultSet	rs,
									int			columnIndex,
									Object		expected) throws Exception
	{
		assertGetDouble(rs, columnIndex, expected, null);
	}

	private void assertGetDouble(	ResultSet		rs,
									String			columnName,
									Object			expected,
									SQLException	e) throws Exception
	{

		assertGetDouble(rs, 0, columnName, expected, e);
	}

	private void assertGetDouble(	ResultSet	rs,
									String		columnName,
									Object		expected) throws Exception
	{

		assertGetDouble(rs, columnName, expected, null);
	}

	private void assertGetFloat(	ResultSet		rs,
									int				columnIndex,
									String			columnName,
									Object			expected,
									SQLException	e) throws Exception
	{
		if (expected == null) {
			if (columnName == null)	assertEquals(0, rs.getFloat(columnIndex));
			else					assertEquals(0, rs.getFloat(columnName));
			assertTrue(rs.wasNull());
		} else {
			if (e != null) {
				boolean	caught = false;
				try {
					if (columnName == null)	rs.getFloat(columnIndex);
					else					rs.getFloat(columnName);
				} catch (SQLException	sqle) {
					caught = true;
					assertEquals(e.getSQLState(), sqle.getSQLState());
				}
				assertTrue(caught);
			} else {
				float	exp = -1.0f;
				float	actual = -1.0f;
				if (columnName == null)	actual = rs.getFloat(columnIndex);
				else					actual = rs.getFloat(columnName);
				assertFalse(rs.wasNull());
				if (exp == actual) {
					// 同じ値だったらわざと exp を違う値にしておく
					if (actual == Float.MIN_VALUE)	exp += 0.1f;
					else							exp += 0.1f;
				}
				String	expectedClass = expected.getClass().getName();
				if (expectedClass.compareTo("java.lang.String") == 0) {
					boolean	caught = false;
					try {
						exp = Float.parseFloat((String)expected);
					} catch (NumberFormatException	nfe) {
						caught = true;
					}
					assertFalse(caught);
				} else if (expectedClass.compareTo("java.lang.Integer") == 0) {
					exp = ((Integer)expected).floatValue();
				} else if (expectedClass.compareTo("java.lang.Long") == 0) {
					exp = ((Long)expected).floatValue();
				} else if (expectedClass.compareTo("java.lang.Double") == 0) {
					exp = ((Double)expected).floatValue();
				} else if (expectedClass.compareTo("java.math.BigDecimal") == 0) {
					exp = ((BigDecimal)expected).floatValue();
				} else {
					System.out.println("??? assertGetFloat");
					System.out.println("expectedClass = expectedClass");
				}
				assertEquals(exp, actual);
			}
		}
	}

	private void assertGetFloat(	ResultSet		rs,
									int				columnIndex,
									Object			expected,
									SQLException	e) throws Exception
	{
		assertGetFloat(rs, columnIndex, null, expected, e);
	}

	private void assertGetFloat(	ResultSet	rs,
									int			columnIndex,
									Object		expected) throws Exception
	{
		assertGetFloat(rs, columnIndex, expected, null);
	}

	private void assertGetFloat(	ResultSet		rs,
									String			columnName,
									Object			expected,
									SQLException	e) throws Exception
	{

		assertGetFloat(rs, 0, columnName, expected, e);
	}

	private void assertGetFloat(	ResultSet	rs,
									String		columnName,
									Object		expected) throws Exception
	{

		assertGetFloat(rs, columnName, expected, null);
	}

	private void assertGetInt(	ResultSet		rs,
								int				columnIndex,
								String			columnName,
								Object			expected,
								SQLException	e) throws Exception
	{
		if (expected == null) {
			if (columnName == null)	assertZero(rs.getInt(columnIndex));
			else					assertZero(rs.getInt(columnName));
			assertTrue(rs.wasNull());
		} else {
			if (e != null) {
				boolean	caught = false;
				try {
					if (columnName == null)	rs.getInt(columnIndex);
					else					rs.getInt(columnName);
				} catch (SQLException	sqle) {
					caught = true;
					assertEquals(e.getSQLState(), sqle.getSQLState());
				}
				assertTrue(caught);
			} else {
				int	exp = -1;
				int	actual = -1;
				if (columnName == null)	actual = rs.getInt(columnIndex);
				else					actual = rs.getInt(columnName);
				assertFalse(rs.wasNull());
				if (exp == actual) {
					// 同じ値だったらわざと exp を違う値にしておく
					if (actual == Integer.MIN_VALUE)	exp++;
					else								exp--;
				}
				String	expectedClass = expected.getClass().getName();
				if (expectedClass.compareTo("java.lang.String") == 0) {
					boolean	caught = false;
					try {
						exp = Integer.parseInt((String)expected);
					} catch (NumberFormatException	nfe) {
						caught = true;
					}
					assertFalse(caught);
				} else if (expectedClass.compareTo("java.lang.Integer") == 0) {
					exp = ((Integer)expected).intValue();
				} else if (expectedClass.compareTo("java.lang.Long") == 0) {
					exp = ((Long)expected).intValue();
				} else if (expectedClass.compareTo("java.lang.Double") == 0) {
					exp = ((Double)expected).intValue();
				} else if (expectedClass.compareTo("java.math.BigDecimal") == 0) {
					exp = ((BigDecimal)expected).intValue();
				} else {
					System.out.println("??? assertGetInt");
					System.out.println("expectedClass = expectedClass");
				}
				assertEquals(exp, actual);
			}
		}
	}

	private void assertGetInt(	ResultSet		rs,
								int				columnIndex,
								Object			expected,
								SQLException	e) throws Exception
	{
		assertGetInt(rs, columnIndex, null, expected, e);
	}

	private void assertGetInt(	ResultSet	rs,
								int			columnIndex,
								Object		expected) throws Exception
	{
		assertGetInt(rs, columnIndex, expected, null);
	}

	private void assertGetInt(	ResultSet		rs,
								String			columnName,
								Object			expected,
								SQLException	e) throws Exception
	{

		assertGetInt(rs, 0, columnName, expected, e);
	}

	private void assertGetInt(	ResultSet	rs,
								String		columnName,
								Object		expected) throws Exception
	{

		assertGetInt(rs, columnName, expected, null);
	}

	private void assertGetLong(	ResultSet		rs,
								int				columnIndex,
								String			columnName,
								Object			expected,
								SQLException	e) throws Exception
	{
		if (expected == null) {
			if (columnName == null)	assertEquals(0, rs.getLong(columnIndex));
			else					assertEquals(0, rs.getLong(columnName));
			assertTrue(rs.wasNull());
		} else {
			if (e != null) {
				boolean	caught = false;
				try {
					if (columnName == null)	rs.getLong(columnIndex);
					else					rs.getLong(columnName);
				} catch (SQLException	sqle) {
					caught = true;
					assertEquals(e.getSQLState(), sqle.getSQLState());
				}
				assertTrue(caught);
			} else {
				long	exp = -1;
				long	actual = -1;
				if (columnName == null)	actual = rs.getLong(columnIndex);
				else					actual = rs.getLong(columnName);
				assertFalse(rs.wasNull());
				if (exp == actual) {
					// 同じ値だったらわざと exp を違う値にしておく
					if (actual == Long.MIN_VALUE)	exp++;
					else							exp--;
					String	expectedClass = expected.getClass().getName();
					if (expectedClass.compareTo("java.lang.String") == 0) {
						boolean	caught = false;
						try {
							exp = Long.parseLong((String)expected);
						} catch (NumberFormatException	nfe) {
							caught = true;
						}
						assertFalse(caught);
					} else if (expectedClass.compareTo("java.lang.Integer") == 0) {
						exp = ((Integer)expected).longValue();
					} else if (expectedClass.compareTo("java.lang.Long") == 0) {
						exp = ((Long)expected).longValue();
					} else if (expectedClass.compareTo("java.lang.Double") == 0) {
						exp = ((Double)expected).longValue();
					} else {
						System.out.println("??? assertGetLong");
						System.out.println("expectedClass = expectedClass");
					}
					assertEquals(exp, actual);
				}
			}
		}
	}

	private void assertGetLong(	ResultSet		rs,
								int				columnIndex,
								Object			expected,
								SQLException	e) throws Exception
	{
		assertGetLong(rs, columnIndex, null, expected, e);
	}

	private void assertGetLong(	ResultSet	rs,
								int			columnIndex,
								Object		expected) throws Exception
	{
		assertGetLong(rs, columnIndex, expected, null);
	}

	private void assertGetLong(	ResultSet		rs,
								String			columnName,
								Object			expected,
								SQLException	e) throws Exception
	{

		assertGetLong(rs, 0, columnName, expected, e);
	}

	private void assertGetLong(	ResultSet	rs,
								String		columnName,
								Object		expected) throws Exception
	{

		assertGetLong(rs, columnName, expected, null);
	}

	private void assertGetObject(	ResultSet		rs,
									int				columnIndex,
									String			columnName,
									Object			expected,
									java.util.Map	map) throws Exception
	{
		if (expected == null) {
			if (columnName == null) {
				if (map == null)	assertNull(rs.getObject(columnIndex));
				else				assertNull(rs.getObject(columnIndex, map));
			} else {
				if (map == null)	assertNull(rs.getObject(columnName));
				else				assertNull(rs.getObject(columnName, map));
			}
			assertTrue(rs.wasNull());
		} else {
			Object	actual = null;
			if (columnName == null) {
				if (map == null)	assertNotNull(actual = rs.getObject(columnIndex));
				else				assertNotNull(actual = rs.getObject(columnIndex, map));
			} else {
				if (map == null)	assertNotNull(actual = rs.getObject(columnName));
				else				assertNotNull(actual = rs.getObject(columnName, map));
			}
			assertFalse(rs.wasNull());
			String	actualClass = actual.getClass().getName();
			if (actualClass.compareTo("java.lang.Integer") == 0) {
				assertEquals(((Integer)expected).intValue(), ((Integer)actual).intValue());
			} else if (actualClass.compareTo("java.lang.Long") == 0) {
				assertEquals(((Long)expected).longValue(), ((Long)actual).longValue());
			} else if (actualClass.compareTo("java.lang.String") == 0) {
				assertEquals((String)expected, (String)actual);
			} else if (actualClass.compareTo("java.lang.Double") == 0) {
				assertEquals((Double)expected, (Double)actual);
			} else if (actualClass.compareTo("java.sql.Timestamp") == 0) {
				assertEquals(((java.sql.Timestamp)expected).toString(), ((java.sql.Timestamp)actual).toString());
			} else if (actualClass.compareTo("[B") == 0) {
				assertEquals((byte[])expected, (byte[])actual);
			} else if (actualClass.compareTo("jp.co.ricoh.doquedb.common.LanguageData") == 0) {
				assertEquals((String)expected, ((LanguageData)actual).toString());
			} else if (actualClass.compareTo("jp.co.ricoh.doquedb.jdbc.Array") == 0) {
				ResultSet	ary = ((java.sql.Array)actual).getResultSet();
				switch (((java.sql.Array)actual).getBaseType()) {
				case Types.INTEGER:	// INT 配列の要素のチェック
					{
						Integer[]	exp = (Integer[])expected;
						for (int i = 0; i < exp.length; i++) {
							assertTrue(ary.next());
							assertEquals(i + 1, ary.getInt(1));				// 要素のインデックス
							assertEquals(exp[i].intValue(), ary.getInt(2));	// 要素の値
						}
					}
					break;
				case Types.BIGINT:	// BIGINT 配列の要素のチェック
					{
						Long[]	exp = (Long[])expected;
						for (int i = 0; i < exp.length; i++) {
							assertTrue(ary.next());
							assertEquals(i + 1, ary.getInt(1));					// 要素のインデックス
							assertEquals(exp[i].longValue(), ary.getLong(2));	// 要素の値
						}
					}
					break;
				case Types.DECIMAL:	// DECIMAL 配列の要素のチェック
					{
						BigDecimal[]	exp = (BigDecimal[])expected;
						for (int i = 0; i < exp.length; i++) {
							assertTrue(ary.next());
							assertEquals(i + 1, ary.getInt(1));					// 要素のインデックス
							assertEquals(exp[i].doubleValue(), ary.getBigDecimal(2).doubleValue());			// 要素の値
						}
					}
					break;
				case Types.CHAR:	// CHAR 配列などの要素のチェック
				case Types.VARCHAR:	// NVARCHAR 配列などの要素のチェック
				case Types.OTHER:	// LANGUAGE 配列の要素のチェック
					{
						String[]	exp = (String[])expected;
						for (int i = 0; i < exp.length; i++) {
							assertTrue(ary.next());
							assertEquals(i + 1, ary.getInt(1));		// 要素のインデックス
							assertEquals(exp[i], ary.getString(2));	// 要素の値
						}
					}
					break;
				case Types.DOUBLE:
					{
						Double[]	exp = (Double[])expected;
						for (int i = 0; i < exp.length; i++) {
							assertTrue(ary.next());
							assertEquals(i + 1, ary.getInt(1));						// 要素のインデック
							assertEquals(exp[i].doubleValue(), ary.getDouble(2));	// 要素の値
						}
					}
					break;
				case Types.TIMESTAMP:
					{
						Timestamp[]	exp = (Timestamp[])expected;
						for (int i = 0; i < exp.length; i++) {
							assertTrue(ary.next());
							assertEquals(i + 1, ary.getInt(1));								// 要素のインデックス
							assertEquals(exp[i].getTime(), ary.getTimestamp(2).getTime());	// 要素の値
						}
					}
					break;
				case Types.VARBINARY:
				case Types.BINARY:
					{
						byte[][]	exp = (byte[][])expected;
						for (int i = 0; i < exp.length; i++) {
							assertTrue(ary.next());
							assertEquals(i + 1, ary.getInt(1));		// 要素のインデックス
							assertEquals(exp[i], ary.getBytes(2));	// 要素の値
						}
					}
					break;
				default:
					System.out.println("??? assertGetObject");
					System.out.println("expected class = " + expected.getClass().getName());
				}
				assertFalse(ary.next());
				ary.close();
			} else {
				System.out.println("??? assertGetObject");
				System.out.println("actual class = " + actual.getClass().getName());
			}
		}
	}

	private void assertGetObject(	ResultSet		rs,
									int				columnIndex,
									Object			expected,
									java.util.Map	map) throws Exception
	{
		assertGetObject(rs, columnIndex, null, expected, map);
	}

	private void assertGetObject(	ResultSet		rs,
									int				columnIndex,
									Object			expected) throws Exception
	{
		assertGetObject(rs, columnIndex, expected, null);
	}

	private void assertGetObject(	ResultSet		rs,
									String			columnName,
									Object			expected,
									java.util.Map	map) throws Exception
	{

		assertGetObject(rs, 0, columnName, expected, map);
	}

	private void assertGetObject(	ResultSet	rs,
									String		columnName,
									Object		expected) throws Exception
	{

		assertGetObject(rs, columnName, expected, null);
	}

	private void assertGetShort(	ResultSet		rs,
									int				columnIndex,
									String			columnName,
									Object			expected,
									SQLException	e) throws Exception
	{
		if (expected == null) {
			if (columnName == null)	assertZero(rs.getShort(columnIndex));
			else					assertZero(rs.getShort(columnName));
			assertTrue(rs.wasNull());
		} else {
			if (e != null) {
				boolean	caught = false;
				try {
					if (columnName == null)	rs.getShort(columnIndex);
					else					rs.getShort(columnName);
				} catch (SQLException	sqle) {
					caught = true;
					assertEquals(e.getSQLState(), sqle.getSQLState());
				}
				assertTrue(caught);
			} else {
				short	exp = -1;
				short	actual = -1;
				if (columnName == null)	actual = rs.getShort(columnIndex);
				else					actual = rs.getShort(columnName);
				assertFalse(rs.wasNull());
				if (exp == actual) {
					// 同じ値だったらわざと exp を違う値にしておく
					if (actual == Short.MIN_VALUE)	exp++;
					else							exp--;
				}
				String	expectedClass = expected.getClass().getName();
				if (expectedClass.compareTo("java.lang.String") == 0) {
					boolean	caught = false;
					try {
						exp = Short.parseShort((String)expected);
					} catch (NumberFormatException	nfe) {
						caught = true;
					}
					assertFalse(caught);
				} else if (expectedClass.compareTo("java.lang.Integer") == 0) {
					exp = ((Integer)expected).shortValue();
				} else if (expectedClass.compareTo("java.lang.Long") == 0) {
					exp = ((Long)expected).shortValue();
				} else if (expectedClass.compareTo("java.lang.Double") == 0) {
					exp = ((Double)expected).shortValue();
				} else if (expectedClass.compareTo("java.math.BigDecimal") == 0) {
					exp = ((BigDecimal)expected).shortValue();
				} else {
					System.out.println("??? assertGetShort");
					System.out.println("expectedClass = expectedClass");
				}
				assertEquals(exp, actual);
			}
		}
	}

	private void assertGetShort(	ResultSet		rs,
									int				columnIndex,
									Object			expected,
									SQLException	e) throws Exception
	{
		assertGetShort(rs, columnIndex, null, expected, e);
	}

	private void assertGetShort(	ResultSet	rs,
									int			columnIndex,
									Object		expected) throws Exception
	{
		assertGetShort(rs, columnIndex, expected, null);
	}

	private void assertGetShort(	ResultSet		rs,
									String			columnName,
									Object			expected,
									SQLException	e) throws Exception
	{

		assertGetShort(rs, 0, columnName, expected, e);
	}

	private void assertGetShort(	ResultSet	rs,
									String		columnName,
									Object		expected) throws Exception
	{

		assertGetShort(rs, columnName, expected, null);
	}

	private void assertGetString(	ResultSet		rs,
									int				columnIndex,
									String			columnName,
									Object			expected,
									SQLException	e) throws Exception
	{
		if (expected == null) {
			if (columnName == null)	assertNull(rs.getString(columnIndex));
			else					assertNull(rs.getString(columnName));
			assertTrue(rs.wasNull());
		} else {
			if (e != null) {
				boolean	caught = false;
				try {
					if (columnName == null)	rs.getString(columnIndex);
					else					rs.getString(columnName);
				} catch (SQLException	sqle) {
					caught = true;
					assertEquals(e.getSQLState(), sqle.getSQLState());
				}
				assertTrue(caught);
			} else {
				String	actual = null;
				if (columnName == null)	assertNotNull(actual = rs.getString(columnIndex));
				else					assertNotNull(actual = rs.getString(columnName));
				assertFalse(rs.wasNull());
				String	expectedClass = expected.getClass().getName();
				if (expectedClass.compareTo("[B") == 0) {
					String	exp = new String((byte[])expected, 0, ((byte[])expected).length);
					assertEquals(exp, actual);
				} else if ( expectedClass.compareTo("java.math.BigDecimal") == 0 ) {
					assertEquals(Double.valueOf(expected.toString()), Double.valueOf(actual));
				} else {
					assertEquals(expected.toString(), actual);
				}
			}
		}
	}

	private void assertGetString(	ResultSet		rs,
									int				columnIndex,
									Object			expected,
									SQLException	e) throws Exception
	{
		assertGetString(rs, columnIndex, null, expected, e);
	}

	private void assertGetString(	ResultSet	rs,
									int			columnIndex,
									Object		expected) throws Exception
	{
		assertGetString(rs, columnIndex, expected, null);
	}

	private void assertGetString(	ResultSet		rs,
									String			columnName,
									Object			expected,
									SQLException	e) throws Exception
	{

		assertGetString(rs, 0, columnName, expected, e);
	}

	private void assertGetString(	ResultSet	rs,
									String		columnName,
									Object		expected) throws Exception
	{

		assertGetString(rs, columnName, expected, null);
	}

	private void assertGetTime(	ResultSet		rs,
								int				columnIndex,
								String			columnName,
								Object			expected,
								SQLException	e) throws Exception
	{
		if (expected == null) {
			if (columnName == null)	assertNull(rs.getTime(columnIndex));
			else					assertNull(rs.getTime(columnName));
			assertTrue(rs.wasNull());
		} else {
			if (e != null) {
				boolean	caught = false;
				try {
					if (columnName == null)	rs.getTime(columnIndex);
					else					rs.getTime(columnName);
				} catch (SQLException	sqle) {
					caught = true;
					assertEquals(e.getSQLState(), sqle.getSQLState());
				}
				assertTrue(caught);
			} else {
				java.sql.Time	actual = null;
				if (columnName == null)	assertNotNull(actual = rs.getTime(columnIndex));
				else					assertNotNull(actual = rs.getTime(columnName));
				assertFalse(rs.wasNull());
				String	expectedClass = expected.getClass().getName();
				if (expectedClass.compareTo("java.lang.String") == 0) {
					assertEquals((String)expected, actual.toString());
				} else if (expectedClass.compareTo("java.sql.Timestamp") == 0) {
					java.sql.Time	exp = new java.sql.Time(0L);
					exp.setTime(((java.sql.Timestamp)expected).getTime());
					assertEquals(exp.toString(), actual.toString());
				} else {
					System.out.println("??? assertGetTime");
					System.out.println("expectedClass = expectedClass");
				}
			}
		}
	}

	private void assertGetTime(	ResultSet		rs,
								int				columnIndex,
								Object			expected,
								SQLException	e) throws Exception
	{
		assertGetTime(rs, columnIndex, null, expected, e);
	}

	private void assertGetTime(	ResultSet	rs,
								int			columnIndex,
								Object		expected) throws Exception
	{
		assertGetTime(rs, columnIndex, expected, null);
	}

	private void assertGetTime(	ResultSet		rs,
								String			columnName,
								Object			expected,
								SQLException	e) throws Exception
	{

		assertGetTime(rs, 0, columnName, expected, e);
	}

	private void assertGetTime(	ResultSet	rs,
								String		columnName,
								Object		expected) throws Exception
	{

		assertGetTime(rs, columnName, expected, null);
	}

	private void assertGetTimestamp(	ResultSet		rs,
										int				columnIndex,
										String			columnName,
										Object			expected,
										SQLException	e) throws Exception
	{
		if (expected == null) {
			if (columnName == null)	assertNull(rs.getTimestamp(columnIndex));
			else					assertNull(rs.getTimestamp(columnName));
			assertTrue(rs.wasNull());
		} else {
			if (e != null) {
				boolean	caught = false;
				try {
					if (columnName == null)	rs.getTimestamp(columnIndex);
					else					rs.getTimestamp(columnName);
				} catch (SQLException	sqle) {
					caught = true;
					assertEquals(e.getSQLState(), sqle.getSQLState());
				}
				assertTrue(caught);
			} else {
				java.sql.Timestamp	actual = null;
				if (columnName == null)	assertNotNull(actual = rs.getTimestamp(columnIndex));
				else					assertNotNull(actual = rs.getTimestamp(columnName));
				assertFalse(rs.wasNull());
				String	expectedClass = expected.getClass().getName();
				if (expectedClass.compareTo("java.lang.String") == 0) {
					assertEquals((String)expected, actual.toString());
				} else if (expectedClass.compareTo("java.sql.Timestamp") == 0) {
					assertEquals(((java.sql.Timestamp)expected).toString(), actual.toString());
				} else {
					System.out.println("??? assertGetTimestamp");
					System.out.println("expectedClass = " + expectedClass);
				}
			}
		}
	}

	private void assertGetTimestamp(	ResultSet		rs,
										int				columnIndex,
										Object			expected,
										SQLException	e) throws Exception
	{
		assertGetTimestamp(rs, columnIndex, null, expected, e);
	}

	private void assertGetTimestamp(	ResultSet	rs,
										int			columnIndex,
										Object		expected) throws Exception
	{
		assertGetTimestamp(rs, columnIndex, expected, null);
	}

	private void assertGetTimestamp(	ResultSet		rs,
										String			columnName,
										Object			expected,
										SQLException	e) throws Exception
	{

		assertGetTimestamp(rs, 0, columnName, expected, e);
	}

	private void assertGetTimestamp(	ResultSet	rs,
										String		columnName,
										Object		expected) throws Exception
	{

		assertGetTimestamp(rs, columnName, expected, null);
	}

	private void assertEquals(byte[] expected, byte[] actual) throws Exception
	{
		assertEquals(expected.length, actual.length);
		for (int i = 0; i < expected.length; i++) assertEquals(expected[i], actual[i]);
	}

	private void assertNotSupported(SQLException	e) throws Exception
	{
		assertEquals((new NotSupported()).getSQLState(), e.getSQLState());
	}

	private void assertEntryNotFound(SQLException	e) throws Exception
	{
		assertEquals((new EntryNotFound("dummy")).getSQLState(), e.getSQLState());
	}

	private void assertBadArgument(SQLException	e) throws Exception
	{
		assertEquals((new BadArgument()).getSQLState(), e.getSQLState());
	}

	private void assertUnexpected(SQLException	e) throws Exception
	{
		assertEquals((new Unexpected()).getSQLState(), e.getSQLState());
	}
}

//
// Copyright (c) 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
