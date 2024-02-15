// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ArrayTest.java -- jp.co.ricoh.doquedb.jdbc.Array クラスのテスト
// 
// Copyright (c) 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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

import jp.co.ricoh.doquedb.common.*;
import jp.co.ricoh.doquedb.exception.*;

public class ArrayTest extends TestBase
{
	public ArrayTest(String nickname)
	{
		super(nickname);
	}

	// Array.getArray() のテスト
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

		int	rowIndex = 0;

		while (rs.next()) {

			int	columnIndex = 1;

			java.sql.Array	ary = null;
			assertArray(af_ints[rowIndex],				((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray());	// af_int				int					array[no limit]

			// bigint 列は v15.0 からサポート
			assertArray(af_bigints[rowIndex],		((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray());	// af_bigint			bigint				array[no limit]

			// decimal 列は v16.1 からサポート
			assertArray(af_decimals[rowIndex],		((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray());	// af_decimal			decimal				array[no limit]

			assertArray(af_char8s[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray());	// af_char8				char(8)				array[no limit]
			assertArray(af_floats[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray());	// af_float				float				array[no limit]
			assertArray(af_datetimes[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray());	// af_datetime			datetime			array[no limit]
			assertArray(af_ids[rowIndex],				((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray());	// af_id				uniqueidentifier	array[no limit]
			assertArray(af_images[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray());	// af_image				image				array[no limit]
			assertArray(af_languages[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray());	// af_language			language			array[no limit]
			assertArray(af_nchar6s[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray());	// af_nchar6			nchar(6)			array[no limit]
			assertArray(af_nvarchar256s[rowIndex],		((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray());	// af_nvarchar256		nvarchar(256)		array[no limit]
			assertArray(af_varchar128s[rowIndex],		((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray());	// af_varchar128		varchar(128)		array[no limit]
			assertArray(af_ntexts[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray());	// af_ntext				ntext				array[no limit]
			assertArray(af_ntext_compresseds[rowIndex],	((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray());	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
			assertArray(af_fulltexts[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray());	// af_fulltext			fulltext			array[no limit]
			assertArray(af_binary50s[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray());	// af_binary50			binary(50)			array[no limit]

			rowIndex++;
		}
		assertFalse(rs.next());

		rs.close();

		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// Array.getArray(long, int) のテスト
	public void test_getArray2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("Select * from t"));

		int	rowIndex = 0;

		int		index = 2;
		long	indexL = index;
		int		count = 3;

		while (rs.next()) {

			int	columnIndex = 1;

			java.sql.Array	ary = null;
			assertArray(af_ints[rowIndex],				((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray(indexL, count), index, count);	// af_int				int					array[no limit]

			// bigint 列は v15.0 からサポート
			assertArray(af_bigints[rowIndex],		((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray(indexL, count), index, count);	// af_bigint			bigint				array[no limit]

			// decimal 列は v16.1 からサポート
			assertArray(af_decimals[rowIndex],		((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray(indexL, count), index, count);	// af_decimal			decimal				array[no limit]

			assertArray(af_char8s[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray(indexL, count), index, count);	// af_char8				char(8)				array[no limit]
			assertArray(af_floats[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray(indexL, count), index, count);	// af_float				float				array[no limit]
			assertArray(af_datetimes[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray(indexL, count), index, count);	// af_datetime			datetime			array[no limit]
			assertArray(af_ids[rowIndex],				((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray(indexL, count), index, count);	// af_id				uniqueidentifier	array[no limit]
			assertArray(af_images[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray(indexL, count), index, count);	// af_image				image				array[no limit]
			assertArray(af_languages[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray(indexL, count), index, count);	// af_language			language			array[no limit]
			assertArray(af_nchar6s[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray(indexL, count), index, count);	// af_nchar6			nchar(6)			array[no limit]
			assertArray(af_nvarchar256s[rowIndex],		((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray(indexL, count), index, count);	// af_nvarchar256		nvarchar(256)		array[no limit]
			assertArray(af_varchar128s[rowIndex],		((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray(indexL, count), index, count);	// af_varchar128		varchar(128)		array[no limit]
			assertArray(af_ntexts[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray(indexL, count), index, count);	// af_ntext				ntext				array[no limit]
			assertArray(af_ntext_compresseds[rowIndex],	((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray(indexL, count), index, count);	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
			assertArray(af_fulltexts[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray(indexL, count), index, count);	// af_fulltext			fulltext			array[no limit]
			assertArray(af_binary50s[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getArray(indexL, count), index, count);	// af_binary50			binary(50)			array[no limit]

			rowIndex++;
		}
		assertFalse(rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// Array.getBaseType() のテスト
	public void test_getBaseType() throws Exception
	{
		// ColumnMetaData は v15.0 からサポート

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * From t"));

		assertTrue(rs.next());	// null ばっかりのタプルを飛ばす
		assertTrue(rs.next());	// ふたつめのタプルは null の列がないはず

		int	columnIndex = 1;

		assertEquals(Types.INTEGER,		rs.getArray(columnIndex++).getBaseType());	// af_int				int					array[no limit]
		assertEquals(Types.BIGINT,		rs.getArray(columnIndex++).getBaseType());	// af_bigint			bigint				array[no limit]
		assertEquals(Types.DECIMAL,		rs.getArray(columnIndex++).getBaseType());	// af_decimal			decimal				array[no limit]
		assertEquals(Types.CHAR,		rs.getArray(columnIndex++).getBaseType());	// af_char8				char(8)				array[no limit]
		assertEquals(Types.DOUBLE,		rs.getArray(columnIndex++).getBaseType());	// af_float				float				array[no limit]
		assertEquals(Types.TIMESTAMP,	rs.getArray(columnIndex++).getBaseType());	// af_datetime			datetime			array[no limit]
		assertEquals(Types.CHAR,		rs.getArray(columnIndex++).getBaseType());	// af_id				uniqueidentifier	array[no limit]
		assertEquals(Types.VARBINARY,	rs.getArray(columnIndex++).getBaseType());	// af_image				image				array[no limit]
		assertEquals(Types.OTHER,		rs.getArray(columnIndex++).getBaseType());	// af_language			language			array[no limit]
		assertEquals(Types.CHAR,		rs.getArray(columnIndex++).getBaseType());	// af_nchar6			nchar(6)			array[no limit]
		assertEquals(Types.VARCHAR,		rs.getArray(columnIndex++).getBaseType());	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertEquals(Types.VARCHAR,		rs.getArray(columnIndex++).getBaseType());	// af_varchar128		varchar(128)		array[no limit]
		assertEquals(Types.VARCHAR,		rs.getArray(columnIndex++).getBaseType());	// af_ntext				ntext				array[no limit]
		assertEquals(Types.VARCHAR,		rs.getArray(columnIndex++).getBaseType());	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertEquals(Types.VARCHAR,		rs.getArray(columnIndex++).getBaseType());	// af_fulltext			fulltext			array[no limit]
		assertEquals(Types.BINARY,		rs.getArray(columnIndex++).getBaseType());	// af_binary50			binary(50)			array[no limit]

		// （すべての ResultSet を next で取得しないで close すると、Sydney に cancel request が飛ぶので、
		// 　[INFO] ではあるが syslog.csv にそれが出てしまいちょっと気持ち悪いので、すべて next で取得する。）
		while (rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// Array.getBaseTypeName() のテスト
	public void test_getBaseTypeName() throws Exception
	{
		// ColumnMetaData は v15.0 からサポート

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("SELECT * from t"));

		assertTrue(rs.next());	// null ばっかりのタプルを飛ばす
		assertTrue(rs.next());	// ふたつめのタプルは null の列がないはず

		int	columnIndex = 1;

		assertEquals("int",			rs.getArray(columnIndex++).getBaseTypeName());	// af_int				int					array[no limit]
		assertEquals("bigint",		rs.getArray(columnIndex++).getBaseTypeName());	// af_bigint			bigint				array[no limit]
		assertEquals("decimal",		rs.getArray(columnIndex++).getBaseTypeName());	// af_decimal			decimal				array[no limit]
		assertEquals("char",		rs.getArray(columnIndex++).getBaseTypeName());	// af_char8				char(8)				array[no limit]
		assertEquals("float",		rs.getArray(columnIndex++).getBaseTypeName());	// af_float				float				array[no limit]
		assertEquals("datetime",	rs.getArray(columnIndex++).getBaseTypeName());	// af_datetime			datetime			array[no limit]
		assertEquals("char",		rs.getArray(columnIndex++).getBaseTypeName());	// af_id				uniqueidentifier	array[no limit]
		assertEquals("varbinary",	rs.getArray(columnIndex++).getBaseTypeName());	// af_image				image				array[no limit]
		assertEquals("language",	rs.getArray(columnIndex++).getBaseTypeName());	// af_language			language			array[no limit]
		assertEquals("nchar",		rs.getArray(columnIndex++).getBaseTypeName());	// af_nchar6			nchar(6)			array[no limit]
		assertEquals("nvarchar",	rs.getArray(columnIndex++).getBaseTypeName());	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertEquals("varchar",		rs.getArray(columnIndex++).getBaseTypeName());	// af_varchar128		varchar(128)		array[no limit]
		assertEquals("nvarchar",	rs.getArray(columnIndex++).getBaseTypeName());	// af_ntext				ntext				array[no limit]
		assertEquals("nvarchar",	rs.getArray(columnIndex++).getBaseTypeName());	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertEquals("nvarchar",	rs.getArray(columnIndex++).getBaseTypeName());	// af_fulltext			fulltext			array[no limit]
		assertEquals("binary",		rs.getArray(columnIndex++).getBaseTypeName());	// af_binary50			binary(50)			array[no limit]

		// （すべての ResultSet を next で取得しないで close すると、Sydney に cancel request が飛ぶので、
		// 　[INFO] ではあるが syslog.csv にそれが出てしまいちょっと気持ち悪いので、すべて next で取得する。）
		while (rs.next());

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// Array.getResultSet() のテスト
	public void test_getResultSet1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * FROM t"));

		int	rowIndex = 0;

		while (rs.next()) {

			int	columnIndex = 1;

			java.sql.Array	ary = null;
			assertArray(af_ints[rowIndex],				((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(), Types.INTEGER);	// af_int				int					array[no limit]

			// bigint 列は v15.0 からサポート
			assertArray(af_bigints[rowIndex],		((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(), Types.BIGINT);	// af_bigint			bigint				array[no limit]

			// decimal 列は v16.1 からサポート
			assertArray(af_decimals[rowIndex],		((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(), Types.DECIMAL);	// af_decimal			decimal				array[no limit]

			assertArray(af_char8s[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(), Types.CHAR);		// af_char8				char(8)				array[no limit]
			assertArray(af_floats[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(), Types.DOUBLE);	// af_float				float				array[no limit]
			assertArray(af_datetimes[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(), Types.TIMESTAMP);	// af_datetime			datetime			array[no limit]
			assertArray(af_ids[rowIndex],				((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(), Types.CHAR);		// af_id				uniqueidentifier	array[no limit]
			assertArray(af_images[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(), Types.VARBINARY);	// af_image				image				array[no limit]
			assertArray(af_languages[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(), Types.OTHER);		// af_language			language			array[no limit]
			assertArray(af_nchar6s[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(), Types.CHAR);		// af_nchar6			nchar(6)			array[no limit]
			assertArray(af_nvarchar256s[rowIndex],		((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(), Types.VARCHAR);	// af_nvarchar256		nvarchar(256)		array[no limit]
			assertArray(af_varchar128s[rowIndex],		((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(), Types.VARCHAR);	// af_varchar128		varchar(128)		array[no limit]
			assertArray(af_ntexts[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(), Types.VARCHAR);	// af_ntext				ntext				array[no limit]
			assertArray(af_ntext_compresseds[rowIndex],	((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(), Types.VARCHAR);	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
			assertArray(af_fulltexts[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(), Types.VARCHAR);	// af_fulltext			fulltext			array[no limit]
			assertArray(af_binary50s[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(), Types.BINARY);	// af_binary50			binary(50)			array[no limit]

			rowIndex++;
		}
		assertFalse(rs.next());

		rs.close();

		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	// Array.getResultSet(long, int) のテスト
	public void test_getResultSet2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("SELECT * FROM t"));

		int	rowIndex = 0;

		int		index = 2;
		long	indexL = index;
		int		count = 2;

		while (rs.next()) {

			int	columnIndex = 1;

			java.sql.Array	ary = null;
			assertArray(af_ints[rowIndex],				((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(indexL, count), Types.INTEGER, index, count);	// af_int				int					array[no limit]

			// bigint 列は v15.0 からサポート
			assertArray(af_bigints[rowIndex],		((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(indexL, count), Types.BIGINT, index, count);		// af_bigint			bigint				array[no limit]	

			// decimal 列は v16.1 からサポート
			assertArray(af_decimals[rowIndex],		((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(indexL, count), Types.DECIMAL, index, count);		// af_decimal			decimal				array[no limit]	

			assertArray(af_char8s[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(indexL, count), Types.CHAR, index, count);		// af_char8				char(8)				array[no limit]
			assertArray(af_floats[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(indexL, count), Types.DOUBLE, index, count);		// af_float				float				array[no limit]
			assertArray(af_datetimes[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(indexL, count), Types.TIMESTAMP, index, count);	// af_datetime			datetime			array[no limit]
			assertArray(af_ids[rowIndex],				((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(indexL, count), Types.CHAR, index, count);		// af_id				uniqueidentifier	array[no limit]
			assertArray(af_images[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(indexL, count), Types.VARBINARY, index, count);	// af_image				image				array[no limit]
			assertArray(af_languages[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(indexL, count), Types.OTHER, index, count);		// af_language			language			array[no limit]
			assertArray(af_nchar6s[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(indexL, count), Types.CHAR, index, count);		// af_nchar6			nchar(6)			array[no limit]
			assertArray(af_nvarchar256s[rowIndex],		((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(indexL, count), Types.VARCHAR, index, count);	// af_nvarchar256		nvarchar(256)		array[no limit]
			assertArray(af_varchar128s[rowIndex],		((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(indexL, count), Types.VARCHAR, index, count);	// af_varchar128		varchar(128)		array[no limit]
			assertArray(af_ntexts[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(indexL, count), Types.VARCHAR, index, count);	// af_ntext				ntext				array[no limit]
			assertArray(af_ntext_compresseds[rowIndex],	((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(indexL, count), Types.VARCHAR, index, count);	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
			assertArray(af_fulltexts[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(indexL, count), Types.VARCHAR, index, count);	// af_fulltext			fulltext			array[no limit]
			assertArray(af_binary50s[rowIndex],			((ary = rs.getArray(columnIndex++)) == null) ? null : ary.getResultSet(indexL, count), Types.BINARY, index, count);		// af_binary50			binary(50)			array[no limit]

			rowIndex++;
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
		createAndInsertTestTable(c);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select af_int from t"));

		java.util.HashMap	hashMap = new java.util.HashMap();

		while (rs.next()) {

			java.sql.Array	ary = rs.getArray(1);
			if (rs.wasNull() == false) {

				assertNotNull(ary);

				// Array.getArray(long, int, java.util.Map)
				boolean	caught = false;
				try {
					ary.getArray(1L, 1, hashMap);
				} catch (SQLException	sqle) {
					caught = true;
					assertNotSupported(sqle);
				}
				assertTrue(caught);

				// Array.getArray(java.util.Map)
				caught = false;
				try {
					ary.getArray(hashMap);
				} catch (SQLException	sqle) {
					caught = true;
					assertNotSupported(sqle);
				}
				assertTrue(caught);

				// Array.getResultSet(long, int, java.util.Map) のテスト
				caught = false;
				try {
					ary.getResultSet(1L, 1, hashMap);
				} catch (SQLException	sqle) {
					caught = true;
					assertNotSupported(sqle);
				}
				assertTrue(caught);

				// Array.getResultSet(java.util.Map) のテスト
				caught = false;
				try {
					ary.getResultSet(hashMap);
				} catch (SQLException	sqle) {
					caught = true;
					assertNotSupported(sqle);
				}
				assertTrue(caught);

				break;
			}
		}

		rs.close();
		s.close();

		// 後始末
		dropTestTable(c);

		c.close();
	}

	public void setUp() throws Exception
	{
		super.setUp();
	}

	public void tearDown() throws Exception
	{
		super.tearDown();
	}

	private void createTestTable(Connection	c) throws Exception
	{
		Statement	s = null;
		assertNotNull(s = c.createStatement());
		String	query =
			"create table t (																	" +
			"	af_int				int					array[no limit],						";

		query = query +
		"	af_bigint			bigint				array[no limit],						";

		query = query +
		"	af_decimal			decimal(10, 5)		array[no limit],						";

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
			"	af_binary50			binary(50)			array[no limit]							" +
			")																					";
		assertEquals(0, s.executeUpdate(query));
		s.close();
	}

	private Integer[]	af_int_2 = { new Integer(529), new Integer(3), new Integer(10), new Integer(3), new Integer(20), new Integer(5) };
	private Integer[]	af_int_3 = { new Integer(98), new Integer(0), new Integer(30), new Integer(5), new Integer(-30), new Integer(1) };
	private Integer[]	af_int_4 = { new Integer(128), new Integer(-387), new Integer(0), new Integer(66), new Integer(910), new Integer(2), new Integer(5) };
	private Integer[]	af_int_5 = { new Integer(3), new Integer(60), new Integer(3), new Integer(9178), new Integer(8) };
	private Integer[]	af_int_6 = { new Integer(39), new Integer(0), new Integer(519), new Integer(33), new Integer(803) };
	private Integer[][]	af_ints = { null, af_int_2, af_int_3, af_int_4, af_int_5, af_int_6 };

	private Long[]		af_bigint_2 = { new Long(151154805176933L), new Long(439827478987L), new Long(38L), new Long(-826876431987L), new Long(7183383824504L), new Long(-4371068808L), new Long(79286346787L) };
	private Long[]		af_bigint_3 = { new Long(30987873287L), new Long(-908439878137L), new Long(66174398714L), new Long(0L), new Long(9015867382748L) };
	private Long[]		af_bigint_4 = { new Long(104938792678L), new Long(38L), new Long(33894739286871L), new Long(1L), new Long(9518984328787L), new Long(-486573686211L), new Long(-4916578L), new Long(6108674876238764L) };
	private Long[]		af_bigint_5 = { new Long(-7984682374L), new Long(360873723674L), new Long(-9832876141873L), new Long(18397676432L) };
	private Long[]		af_bigint_6 = { new Long(-1L), new Long(48368173688L), new Long(6238521743234L), new Long(13847632716546L), new Long(-1847367613857682176L) };
	private Long[][]	af_bigints = { null, af_bigint_2, af_bigint_3, af_bigint_4, af_bigint_5, af_bigint_6 };

	private BigDecimal[] af_decimal_2 = { new BigDecimal("3.14159"), new BigDecimal("0.002"), new BigDecimal("3776"), new BigDecimal("-56.78901"), new BigDecimal("99999.99999"), new BigDecimal("-99999.99999"), new BigDecimal("0") };
	private BigDecimal[] af_decimal_3 = { new BigDecimal("31415.9265"), new BigDecimal("-0.00001"), new BigDecimal("123.456"), new BigDecimal("0"), new BigDecimal("99999.99999") };
	private BigDecimal[] af_decimal_4 = { new BigDecimal("99999.99999"), new BigDecimal("0"), new BigDecimal("65432.10987"), new BigDecimal("1"), new BigDecimal("12345.67890"), new BigDecimal("-98765.43210"), new BigDecimal("-99999.99999"), new BigDecimal("-0.00001") };
	private BigDecimal[] af_decimal_5 = { new BigDecimal("0"), new BigDecimal("0.00001"), new BigDecimal("-0.00001"), new BigDecimal("0") };
	private BigDecimal[] af_decimal_6 = { new BigDecimal("-1"), new BigDecimal("0.99999"), new BigDecimal("99999"), new BigDecimal("12345"), new BigDecimal("-98765") };
	private BigDecimal[][] af_decimals = { null, af_decimal_2, af_decimal_3, af_decimal_4, af_decimal_5, af_decimal_6 };

	private String[]	af_char8_2 = { "AclEntry", "InputMap", "NotFound", "Runnable", "Observer" };
	private String[]	af_char8_3 = { "Provider", "Receiver", "Registry", "Position", "ZipEntry" };
	private String[]	af_char8_4 = { "Security", "FlowView", "Deflater", "BeanInfo", "ButtonUI", "CertPath", "Checksum" };
	private String[]	af_char8_5 = { "SliderUI", "KeyStore", "ListView", "NodeList", "Notation", "NotEmpty" };
	private String[]	af_char8_6 = { "BeanInfo", "Inflater", "Pageable", "PrintJob", "Provider", "Receiver" };
	private String[][]	af_char8s = { null, af_char8_2, af_char8_3, af_char8_4, af_char8_5, af_char8_6 };

	private Double[]	af_float_2 = { new Double(0.0087824), new Double(948372.987), new Double(8.9829873), new Double(0.005872), new Double(43879.987), new Double(0), new Double(787) };
	private Double[]	af_float_3 = { new Double(0), new Double(247983.8782), new Double(4672987.987), new Double(0.000148538), new Double(3.29871), new Double(7498723) };
	private Double[]	af_float_4 = { new Double(5.37), new Double(9872341.639156), new Double(539871.879), new Double(1.28980003), new Double(0.00087438) };
	private Double[]	af_float_5 = { new Double(1438741983.87), new Double(38149831.8), new Double(0.8439871), new Double(4324.987), new Double(5987324.987), new Double(987432.0) };
	private Double[]	af_float_6 = { new Double(3.08791), new Double(56.1973), new Double(0), new Double(87987432.8768), new Double(98732.87), new Double(49173.87432) };
	private Double[][]	af_floats = { null, af_float_2, af_float_3, af_float_4, af_float_5, af_float_6 };

	private Timestamp[]	af_datetime_2 = { Timestamp.valueOf("2004-12-31 05:04:39.460"), Timestamp.valueOf("2005-01-05 13:12:49.008"), Timestamp.valueOf("2005-01-12 16:21:01.329"), Timestamp.valueOf("2005-01-01 14:58:39.005") };
	private Timestamp[]	af_datetime_3 = { Timestamp.valueOf("1980-05-14 23:58:05.438"), Timestamp.valueOf("2004-03-12 05:48:21.368"), Timestamp.valueOf("2005-01-06 15:09:38.420"), Timestamp.valueOf("2004-09-21 05:08:44.332") };
	private Timestamp[]	af_datetime_4 = { Timestamp.valueOf("2002-12-04 14:49:23.152"), Timestamp.valueOf("1990-03-15 21:22:15.332"), Timestamp.valueOf("2001-03-24 03:14:05.118"), Timestamp.valueOf("1970-11-14 23:58:05.438"), Timestamp.valueOf("2005-03-21 14:12:51.838") };
	private Timestamp[]	af_datetime_5 = { Timestamp.valueOf("2003-01-21 01:14:36.287"), Timestamp.valueOf("2005-01-06 15:10:08.883"), Timestamp.valueOf("2005-01-05 13:12:49.008"), Timestamp.valueOf("2005-01-12 16:21:01.329") };
	private Timestamp[]	af_datetime_6 = { Timestamp.valueOf("2005-01-07 16:41:19.772"), Timestamp.valueOf("2005-01-06 15:09:38.420"), Timestamp.valueOf("2004-09-21 05:08:44.332") };
	private Timestamp[][]	af_datetimes = { null, af_datetime_2, af_datetime_3, af_datetime_4, af_datetime_5, af_datetime_6 };

	private String[]	af_id_2 = { "CD83C135-D203-44DE-991D-8FD6677A7C37", "5741A98E-32CC-4571-9C8C-C862529CDF93", "AFFF94DB-E4A0-4D04-AA65-7C518185B122", "7FD84A5D-2CFB-41E8-B4A7-F82EA6E3789E", "402C0C7E-8EDE-4C7C-8F72-EFD2130842EB" };
	private String[]	af_id_3 = { "47421E06-6640-459D-A495-E6FC6DFC95B9", "54B27B08-80D9-4EAA-B493-41A187192E73", "124CA827-748B-4D25-9A01-8BB02A68457A" };
	private String[]	af_id_4 = { "88464AEC-4F5C-4E79-BB72-36DC3D9F9D16", "AF64B9B9-FAA3-4CC4-B294-F94A775D12D9", "D0A66F92-4C6A-4B2A-B57B-E3F85C86BD08", "EDCFF74E-CC91-4A13-B5CB-61B38AA5176B", "B0B5F989-C472-4AF2-B9FE-4BE3E0CE898B", "1619BD43-7998-4EBF-A2B5-BC6CFBA6D27E", "C3CD5E43-C6B8-412D-B9D3-7E89819C4ABA", "75BBF84C-C0FC-4605-9762-1DDB6DB07FB1" };
	private String[]	af_id_5 = { "6604BB12-C5D9-4053-934F-D27119875551", "09B0C930-0D51-4D50-A347-0C00CB0F9541", "137F27DD-287F-4979-BFB1-24DAE9B76C72", "06AB932B-A43A-440E-8630-FC85861573D0" };
	private String[]	af_id_6 = { "126AA03D-4150-4F61-92F8-E2D2A4B4A1E1", "2FAF61BE-C59A-412E-AE25-D418DE2BF026", "D365262D-766B-4FE7-ACD5-1BE5C74C7953", "0D3F212E-A569-4184-BA66-4E007B4B0C80", "3EC4F573-2916-45CA-83C6-300216BAA06E", "7822D71A-CCCF-47C6-8974-0CBE90DEBB53" };
	private String[][]	af_ids = { null, af_id_2, af_id_3, af_id_4, af_id_5, af_id_6 };

	private byte[]	af_image_2c1 = { 0x08, 0x5B, 0x1E, 0x5C, 0x0F, 0x34, 0x24, 0x3B, 0x2C, 0x5B, 0x19, 0x7A, 0x0F, 0x1C, 0x50, 0x2E, 0x18, 0x49, 0x7E, 0x07, 0x5D, 0x4A, 0x19, 0x3F, 0x5D, 0x71, 0x2C, 0x1D, 0x0A, 0x24, 0x3C, 0x19, 0x2D, 0x1A, 0x0D, 0x5E, 0x46, 0x18, 0x4B, 0x70, 0x2D, 0x1C, 0x26, 0x08, 0x2C };
	private byte[]	af_image_2c2 = { 0x39, 0x0A, 0x5F, 0x5B, 0x59, 0x2E, 0x26, 0x4A, 0x1F, 0x14, 0x38, 0x38, 0x2B, 0x1F, 0x3D, 0x74, 0x79, 0x28, 0x1A, 0x2D, 0x3B, 0x58, 0x2F };
	private byte[]	af_image_2c3 = { 0x23, 0x3A, 0x42, 0x1F, 0x2B, 0x3B, 0x11, 0x0F, 0x45, 0x2F, 0x78, 0x38, 0x30, 0x1D, 0x3D, 0x5E, 0x0A, 0x69, 0x64, 0x78, 0x5D, 0x2C, 0x3D, 0x29, 0x1D, 0x08, 0x1E, 0x2A, 0x5F, 0x38, 0x2B, 0x40, 0x1D, 0x38, 0x2A, 0x39, 0x54, 0x1A, 0x03, 0x2F, 0x08, 0x2B, 0x3D, 0x62, 0x29, 0x74 };
	private byte[]	af_image_2c4 = { 0x27, 0x6C, 0x18, 0x29, 0x0B, 0x2F, 0x1C, 0x24, 0x1D, 0x1C, 0x3B, 0x61, 0x59, 0x69, 0x0A, 0x79, 0x59, 0x18, 0x2D, 0x09, 0x03, 0x2B, 0x0E, 0x52, 0x2D, 0x44, 0x2B, 0x69, 0x0D, 0x46, 0x57, 0x5B, 0x13, 0x7D, 0x28, 0x2E, 0x61, 0x2D, 0x7E, 0x40, 0x1C };
	private byte[]	af_image_2c5 = { 0x11, 0x7F, 0x1B, 0x0F, 0x3E, 0x7C, 0x4E, 0x45, 0x66, 0x38, 0x4F, 0x01, 0x16, 0x03, 0x2B, 0x3C, 0x5D, 0x0D, 0x7D, 0x2B };
	private byte[][]	af_image_2 = { af_image_2c1, af_image_2c2, af_image_2c3, af_image_2c4, af_image_2c5 };
	private byte[]	af_image_3c1 = { 0x3E, 0x2E, 0x0B, 0x10, 0x44, 0x4D, 0x08, 0x4B, 0x2F, 0x0D, 0x3D, 0x1F, 0x45, 0x1D, 0x5A, 0x5D, 0x2B, 0x4E, 0x39, 0x53, 0x74, 0x5E, 0x0D, 0x77, 0x05, 0x3B, 0x3B, 0x24, 0x1F, 0x14, 0x77, 0x2F };
	private byte[]	af_image_3c2 = { 0x1A, 0x5B, 0x7A, 0x0F, 0x78, 0x59, 0x3D, 0x2F, 0x74, 0x28, 0x5A, 0x18, 0x01, 0x2A, 0x5B, 0x0B, 0x2F, 0x3C, 0x5E, 0x68, 0x1A, 0x19, 0x2C, 0x2D };
	private byte[]	af_image_3c3 = { 0x75, 0x18, 0x70, 0x09, 0x5F, 0x48, 0x79, 0x2E, 0x38, 0x4F, 0x1E, 0x28, 0x28, 0x56, 0x1E, 0x0B, 0x0A, 0x00, 0x2C, 0x51, 0x2A, 0x21, 0x2D, 0x71, 0x1E, 0x3D, 0x3E, 0x5D, 0x2E, 0x1C, 0x29, 0x63, 0x44, 0x1B, 0x79, 0x28, 0x05, 0x07, 0x18, 0x0E, 0x1A, 0x0C, 0x5A, 0x0D, 0x2A, 0x25, 0x29, 0x4C, 0x28, 0x3E, 0x1C, 0x58, 0x79, 0x19, 0x28, 0x46, 0x18, 0x1A, 0x23, 0x65, 0x71, 0x1D, 0x58, 0x4E, 0x4D, 0x0E, 0x3B, 0x2C, 0x4D };
	private byte[]	af_image_3c4 = { 0x5D, 0x23, 0x52, 0x61, 0x44, 0x59, 0x47, 0x2C, 0x39, 0x2E, 0x22, 0x16, 0x64, 0x61, 0x38, 0x2B, 0x1F, 0x3D, 0x74, 0x79, 0x28, 0x1A, 0x2B, 0x4E, 0x39, 0x64, 0x0A, 0x5A, 0x1B, 0x49, 0x5D, 0x1B, 0x1B, 0x49, 0x5D };
	private byte[][]	af_image_3 = { af_image_3c1, af_image_3c2, af_image_3c3, af_image_3c4 };
	private byte[]	af_image_4c1 = { 0x14, 0x77, 0x2F, 0x0A, 0x5E, 0x5B, 0x6C, 0x5F, 0x77, 0x34, 0x6C, 0x5D, 0x23, 0x52, 0x61, 0x44, 0x59, 0x47, 0x6C, 0x2A, 0x0C, 0x39, 0x0C, 0x26, 0x3D, 0x0A, 0x3A, 0x28, 0x24, 0x7E, 0x28, 0x4D, 0x5D, 0x60 };
	private byte[]	af_image_4c2 = { 0x5A, 0x1B, 0x49, 0x5D, 0x1B, 0x29, 0x0C, 0x24, 0x1F, 0x14, 0x77, 0x2F, 0x0A, 0x5E, 0x5B, 0x1E, 0x5C, 0x0F, 0x34, 0x24, 0x3B, 0x61, 0x2E, 0x13, 0x0B, 0x0E, 0x20, 0x3B, 0x64, 0x0A, 0x5A, 0x1B, 0x49, 0x5D, 0x1B, 0x29, 0x0C, 0x43, 0x6C, 0x4A, 0x13, 0x1B, 0x0A, 0x0A, 0x04, 0x4C };
	private byte[]	af_image_4c3 = { 0x0C, 0x7E, 0x3F, 0x0D, 0x2C, 0x47, 0x40, 0x47, 0x0A, 0x7D, 0x3E, 0x5B, 0x2E, 0x3B, 0x4D, 0x1E, 0x2C, 0x1B, 0x1C, 0x1F, 0x19, 0x24, 0x3B, 0x66, 0x0D, 0x29, 0x51, 0x18, 0x6A, 0x0F, 0x48 };
	private byte[]	af_image_4c4 = { 0x18, 0x0E, 0x1A, 0x0C, 0x5A, 0x0D, 0x2A, 0x25, 0x29, 0x4C, 0x28, 0x3E, 0x0A, 0x3A, 0x28, 0x24, 0x24, 0x3B, 0x2C, 0x5B, 0x19, 0x7A, 0x0F, 0x2C, 0x67, 0x40, 0x47, 0x0A, 0x7D, 0x3E, 0x5B, 0x30, 0x1D, 0x3D, 0x5E, 0x0A, 0x69, 0x64, 0x78, 0x5D };
	private byte[]	af_image_4c5 = { 0x3D, 0x1F, 0x45, 0x1D, 0x5A, 0x5D, 0x2B, 0x4E, 0x74, 0x28, 0x5A };
	private byte[][]	af_image_4 = { af_image_4c1, af_image_4c2, af_image_4c3, af_image_4c4, af_image_4c5 };
	private byte[]	af_image_5c1 = { 0x58, 0x21, 0x0D, 0x2F, 0x01, 0x28, 0x2F, 0x4F, 0x68, 0x5F, 0x1A, 0x0C, 0x28, 0x5E, 0x4E, 0x2F, 0x72, 0x2C, 0x39, 0x2E, 0x22, 0x16, 0x64, 0x61, 0x1C, 0x08, 0x76, 0x1C };
	private byte[]	af_image_5c2 = { 0x1B, 0x0D, 0x2C, 0x67, 0x40, 0x47, 0x0A, 0x7D, 0x3E, 0x5B, 0x2E, 0x0F, 0x2A, 0x72, 0x45, 0x0E, 0x38, 0x0A, 0x29, 0x1F, 0x30, 0x78, 0x2E, 0x2D, 0x7C, 0x5F, 0x4D, 0x08, 0x08, 0x1B, 0x08, 0x39, 0x5B, 0x2E, 0x3B, 0x4D, 0x1E, 0x2C, 0x1B, 0x1C };
	private byte[]	af_image_5c3 = { 0x3E, 0x5D, 0x2E, 0x1C, 0x29, 0x63, 0x44, 0x1B, 0x79, 0x28, 0x05, 0x07, 0x18, 0x0E, 0x1A, 0x0C, 0x7D, 0x3E, 0x5B, 0x2E, 0x3B, 0x4D, 0x1E, 0x2C, 0x1B, 0x1C, 0x59, 0x3D, 0x2F, 0x74, 0x28, 0x5A, 0x18, 0x01, 0x2A, 0x5B, 0x0B, 0x2F, 0x3C, 0x5E, 0x68, 0x1A, 0x19, 0x2C, 0x2D };
	private byte[]	af_image_5c4 = { 0x0C, 0x24, 0x1F, 0x14, 0x77, 0x2F, 0x0A, 0x5E, 0x49, 0x5D, 0x1B, 0x29, 0x2F, 0x0D, 0x3D, 0x1F, 0x4A, 0x1F, 0x14, 0x38, 0x38, 0x2B, 0x1F, 0x3D, 0x74, 0x79, 0x7A, 0x0F, 0x78, 0x59, 0x3D, 0x2F, 0x74, 0x28, 0x5A, 0x18, 0x01, 0x2A, 0x5B, 0x0B, 0x2F, 0x3C, 0x5E, 0x68 };
	private byte[]	af_image_5c5 = { 0x2F, 0x08, 0x2B, 0x3D, 0x62, 0x29, 0x5A, 0x0D, 0x2A, 0x25, 0x29, 0x4C, 0x28, 0x3E, 0x1C, 0x28, 0x5A, 0x18, 0x01, 0x2A, 0x5B, 0x0B, 0x2F, 0x3C, 0x5E, 0x68, 0x1A, 0x19, 0x2C, 0x2D };
	private byte[]	af_image_5c6 = { 0x1D, 0x5A, 0x5D, 0x2B, 0x4E, 0x39, 0x64, 0x0A, 0x5A, 0x1B, 0x49, 0x5D, 0x1B, 0x1B, 0x49, 0x5D, 0x3A, 0x42, 0x1F, 0x2B, 0x3B, 0x11, 0x0F, 0x45, 0x2F, 0x78, 0x38, 0x30, 0x1D, 0x3D, 0x5E, 0x0A, 0x69, 0x64, 0x78, 0x1B, 0x29, 0x0C, 0x24, 0x1F, 0x14, 0x77, 0x2F, 0x0A, 0x5E, 0x5B, 0x1E, 0x5C, 0x0F, 0x34, 0x24, 0x3B, 0x61, 0x2E, 0x13, 0x0B, 0x0E };
	private byte[][]	af_image_5 = { af_image_5c1, af_image_5c2, af_image_5c3, af_image_5c4, af_image_5c5, af_image_5c6 };
	private byte[]	af_image_6c1 = { 0x1D, 0x5A, 0x5D, 0x2B, 0x4E, 0x39, 0x53, 0x3D, 0x0A, 0x3A, 0x28, 0x24, 0x24, 0x3B, 0x2C, 0x5B, 0x19, 0x7A, 0x0F, 0x1C, 0x50, 0x2E };
	private byte[]	af_image_6c2 = { 0x1B, 0x49, 0x5D, 0x1B, 0x29, 0x2F, 0x0D, 0x3D, 0x1F, 0x45, 0x1D, 0x5A, 0x5D, 0x2B, 0x4E, 0x39, 0x64, 0x0A, 0x5A, 0x1B, 0x49, 0x5D, 0x1B, 0x1B, 0x49, 0x5D, 0x1B, 0x29, 0x0C, 0x24, 0x1F, 0x23, 0x52, 0x61, 0x44, 0x59, 0x47, 0x6C, 0x2A };
	private byte[]	af_image_6c3 = { 0x0A, 0x5E, 0x5B, 0x77, 0x18, 0x17, 0x3C, 0x19, 0x44, 0x41, 0x5F, 0x29, 0x28, 0x7D, 0x3E, 0x5B, 0x2E, 0x3B, 0x4D, 0x1E, 0x2C, 0x1B, 0x1C, 0x04, 0x08, 0x0A, 0x20, 0x2D, 0x26, 0x7C, 0x38, 0x33, 0x41, 0x1B, 0x0A, 0x64, 0x42, 0x00 };
	private byte[]	af_image_6c4 = { 0x1F, 0x2B, 0x3B, 0x11, 0x0F, 0x45, 0x2F, 0x78, 0x38, 0x30, 0x1D, 0x3D, 0x5E, 0x0A, 0x69, 0x64, 0x78, 0x5D, 0x2C, 0x3D };
	private byte[]	af_image_6c5 = { 0x74, 0x28, 0x5A, 0x18, 0x01, 0x2A, 0x5B, 0x0B, 0x2F, 0x3C, 0x5E, 0x68, 0x1A, 0x19, 0x2C, 0x2D, 0x0D, 0x2C, 0x47, 0x40, 0x47, 0x0A, 0x7D, 0x3E };
	private byte[][]	af_image_6 = { af_image_6c1, af_image_6c2, af_image_6c3, af_image_6c4, af_image_6c5 };
	private byte[][][]	af_images = { null, af_image_2, af_image_3, af_image_4, af_image_5, af_image_6 };

	private String[]	af_language_2 = { "fr", "gu", "id", "ms", "qu", "sg", "so", "uk", "vo", "zu" };
	private String[]	af_language_3 = { "tl", "uz", "km", "az", "ta", "yi" };
	private String[]	af_language_4 = { "ka", "af", "mn", "ik", "or" };
	private String[]	af_language_5 = { "to", "so", "ps", "pt", "qu", "ja", "hi", "no", "oc"};
	private String[]	af_language_6 = { "en", "it", "qu", "si", "dz", "jw", "km" };
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
	private String[]	af_nvarchar256_6 = { "このクラスは、「PKCS #5」標準で定義されている、パスワードベースの暗号化 (PBE) で使用されるパラメータのセットを指定します。", "Permissions の異種コレクションを表します。", "初期コンテキストが構築されると、コンストラクタに渡される環境パラメータ、および任意のアプリケーションリソースファイルで定義されたプロパティによって環境が初期化されます。", "CSS 属性に基づいて「インライン要素」スタイルを表示します。" };
	private String[][]	af_nvarchar256s = { null, af_nvarchar256_2, af_nvarchar256_3, af_nvarchar256_4, af_nvarchar256_5, af_nvarchar256_6 };

	private String[]	af_varchar128_2 = { "As noted above, this specification often refers to classes of the Java and Java 2 platforms.", "Other useful constructors, methods,", "Counts the number of test cases that will be run by this test.", "Returns a short description of the failure." };
	private String[]	af_varchar128_3 = { "Execute the test method expecting that an Exception of class fExpected or one of its subclasses will be thrown", "Each test runs in its own fixture so there can be no side effects among test runs. Here is an example:", "A TestResult collects the results of executing a test case." };
	private String[]	af_varchar128_4 = { "Those members declared in the interface.", "If a type name is of the form Q.Id, then Q must be either a type name or a package name.", "Runs the tests and collects their result in a TestResult." };
	private String[]	af_varchar128_5 = { "The fully qualified name of a primitive type is the keyword for that primitive type", "The fully qualified name of a named package.", "The fully qualified name of the type long is long.", "a first identifier that begins with a lowercase letter," };
	private String[]	af_varchar128_6 = { "For type int, the default value is zero, that is, 0.", "For type long, the default value is zero, that is, 0L.", "For type float, the default value is positive zero, that is, 0.0f.", "For type double, the default value is positive zero, that is, 0.0." };
	private String[][]	af_varchar128s = { null, af_varchar128_2, af_varchar128_3, af_varchar128_4, af_varchar128_5, af_varchar128_6 };

	private String[]	af_ntext_2 = { "AWT パッケージの変更は、グラフィカルユーザインタフェースを表示するプログラムの堅牢さ、動作、およびパフォーマンスの向上に重点が置かれています。 これまでの実装は、新しい「フォーカスアーキテクチャ」に置き換わりました。", "ここではプラットフォームが異なるために生じるフォーカス関連のバグや、AWT コンポーネントと Swing コンポーネント間の非互換性について説明します。", "新しい持続モデルは、Bean のグラフと持続性形式の変換を処理するために設計されました。" };
	private String[]	af_ntext_3 = { "このクラスは、チャネルの非同期クローズと割り込みを実装するのに必要な低レベルの機構をカプセル化します。", "ViewportUI の結合に使用する多重 UI です。", "特に断らない限り、このクラスで定義されているメソッドはスレッドセーフではありません。", "この例外は、リンクを解決または構築するときに無効なリンクが見つかった場合にスローされます。" };
	private String[]	af_ntext_4 = { "ColorSelectionModel の汎用実装です。", "出力を通常の OutputStream に書き込む ImageOutputStream の実装です。メモリバッファには、少なくとも破棄位置と現在の書き込み位置との間のデータがキャッシュされます。OutputStream を使用するのはコンストラクタのみなので、このクラスは読み込み、変更、または書き込み操作に使用できない場合があります。", "読み込みは、キャッシュに書き込み済みでまだフラッシュされていないストリーム部分についてのみ行うことができます。" };
	private String[]	af_ntext_5 = { "アイデンティティは、人々、会社、組織などの実際の世界のオブジェクトで、そのアイデンティティがその公開鍵を使用して認証できるものです。アイデンティティはまた、デーモンスレッドやスマートカードのようなより抽象的、あるいはより具象的な構成概念であってもかまいません。", "サービスコンテキストリストを形成するサービスコンテキストの配列です。", "固定のインデックスを持つ値のリストを表示するコンポーネントの、現在の選択状態を表します。" };
	private String[]	af_ntext_6 = { "関連したパラメータを使って Diffie-Hellman 公開鍵を指定します。", "一般キーストア例外です。", "パイプによる出力ストリームをパイプによる入力ストリームに接続すると、通信パイプを作成できます。パイプによる出力ストリームは、パイプの送端です。一般的に、PipedOutputStream オブジェクトにデータを書き込むスレッドと、接続された PipedInputStream オブジェクトからデータを読み込むスレッドは別々です。", "推奨できません。" };
	private String[][]	af_ntexts = { null, af_ntext_2, af_ntext_3, af_ntext_4, af_ntext_5, af_ntext_6 };

	private String[]	af_ntext_compressed_2 = { "Java Web Start 製品は、J2SE 1.4.0 に同梱されている新しいアプリケーション配備技術です。そのアプリケーションがコンピュータに存在しない場合は、Java Web Start により、必要なすべてのファイルが自動的にダウンロードされます。", "また、どの方法でアプリケーションを起動しても、常に最新バージョンのアプリケーションが起動されます。", "印刷や表示に使用する、指定された列の推奨タイトルを取得します。", "指定された列の名前を取得します。" };
	private String[]	af_ntext_compressed_3 = { "リバーブは、部屋の壁、天井、および床の音の反射をシミュレーションします。部屋の大きさや、部屋の表面の素材がサウンドを吸収または反射する度合によって、サウンドは消滅するまでに長時間跳ね返ることがあります。", "ReverbType によって提供されるリバーブパラメータは、アーリーリフレクションの遅延時間と強度、レイトリフレクションの遅延時間と強度、および全体的な減衰時間から構成されています。", "指定された列の SQL 型を取得します。" };
	private String[]	af_ntext_compressed_4 = { "IDL の fixed 型に関連している DynAny オブジェクトを表します。", "例外のメンバは、構造体のメンバと同じように扱われます。", "IDL-to-Java コンパイラ (ポータブル) バージョン 3.1 により ../../../../src/share/classes/org/omg/CosNaming/nameservice.idl から生成された org/omg/CosNaming/NamingContextPackage/CannotProceed.java。", "ストリーム内の次の属性を読み込み、それを Unicode 文字のストリームとして返します。" };
	private String[]	af_ntext_compressed_5 = { "DefaultPersistenceDelegate は、抽象クラス PersistenceDelegate の固定実装であり、情報の得られないクラスがデフォルトで使用する委譲です。", "ResultSet オブジェクトの列の型とプロパティに関する情報を取得するのに使用できるオブジェクトです。", "SQL ストアドプロシージャを実行するのに使用されるインタフェースです。" };
	private String[]	af_ntext_compressed_6 = { "Paper を生成する際、アプリケーションが用紙サイズとイメージング可能領域が互換性を持つことを確認します。", "たとえば、用紙サイズが 11 x 17 から 8.5 x 11 に変更された場合、印刷対象領域がページに適合するように、アプリケーションはイメージング可能領域を減少させる必要がある場合があります。", "Paper クラスは、用紙の物理的な性質を記述します。" };
	private String[][]	af_ntext_compresseds = { null, af_ntext_compressed_2, af_ntext_compressed_3, af_ntext_compressed_4, af_ntext_compressed_5, af_ntext_compressed_6 };

	private String[]	af_fulltext_2 = { "新しい持続モデルは、Bean のグラフと持続性形式の変換を処理するために設計されました。 新しい API は、プロパティを表すテキストとして JavaBeans コンポーネントのグラフのアーカイブを作成するのに適しています。", "JDBC 3.0 API は、パッケージの java.sql と javax.sql で構成されており、Java プログラミング言語からの一般的なデータアクセスを提供します。", "ほとんどの SPI メソッドの実装では、設定ノードで情報の読み取りまたは書き込みを行う必要があります。" };
	private String[]	af_fulltext_3 = { "バージョン 1.1.x の java.util.Vector を実装しますが、コレクションクラスはサポートせず、変更発生時には ListDataListener に通知します。現在は Vector に委譲され、今後のリリースでは実際にコレクションが実装されます。", "ポップアップメニューおよびメニューバーのデフォルトのレイアウトマネージャです。", "指定された名前 (キー) を持つ属性を、この Map から削除します。以前からある値を返します。値がない場合は null を返します。" };
	private String[]	af_fulltext_4 = { "DataInput インタフェースを拡張してオブジェクトの読み込みができるようにします。", "要求や応答によって暗黙的に渡されるサービス固有の情報です。サービスコンテキストは、サービス ID と関連データから構成されます。", "読み込みに続いて書き込みするだけで、すべてのイメージを変換、つまりもともと保存されていたイメージとは違う形式で書き込むことができます。ただし、形式の違いのため、この処理中にデータが損失する可能性があります。" };
	private String[]	af_fulltext_5 = { "", "ImageOutputStream のサービスプロバイダインタフェース (SPI) です。サービスプロバイダインタフェースの詳細は、IIORegistry クラスのクラスコメントを参照してください。", "CSS 属性を保持する AttributeSet のキーとして使用される定義です。これは閉じたセット (仕様によって厳密に定義されているセット) なので、最終的なものであり、拡張することはできません。" };
	private String[]	af_fulltext_6 = { "ストリームが読み込み可能な状態かどうかを通知します。InputStreamReader は、入力バッファが空白ではないか、または基本となるバイトストリームからバイトデータを読み込める状態のときに読み込み可能です。", null, "ImagingOpException は、BufferedImageOp または RasterOp のフィルタメソッドのうちの 1 つがイメージを処理できない場合にスローされます。" };
	private String[][]	af_fulltexts = { null, af_fulltext_2, af_fulltext_3, af_fulltext_4, af_fulltext_5, af_fulltext_6 };

	private byte[]		af_binary50_2c1 = { 0x3A, 0x29, 0x11, 0x7B, 0x38, 0x39, 0x2E, 0x09, 0x1A, 0x4E, 0x4C, 0x40, 0x7B, 0x29, 0x59, 0x0C, 0x76, 0x7F, 0x7F, 0x0C, 0x3F, 0x1E, 0x71, 0x25, 0x18, 0x1D, 0x7D, 0x6F, 0x2F, 0x2A, 0x73, 0x28, 0x09, 0x41, 0x5A, 0x1A, 0x5D, 0x24, 0x2C, 0x0A, 0x51, 0x2E, 0x2B, 0x5E, 0x0A, 0x2C, 0x39, 0x0F, 0x48, 0x2A };
	private byte[]		af_binary50_2c2 = { 0x6F, 0x1F, 0x32, 0x28, 0x1D, 0x1E, 0x4A, 0x33, 0x1A, 0x4A, 0x0D, 0x1B, 0x0F, 0x3D, 0x4E, 0x33, 0x42, 0x28, 0x1B, 0x56, 0x1E, 0x5D, 0x6C, 0x1C, 0x49, 0x3D, 0x0D, 0x27, 0x1C, 0x28, 0x04, 0x0A, 0x2E, 0x0A, 0x3B, 0x15, 0x18, 0x33, 0x6C, 0x5E, 0x69, 0x1F, 0x2B, 0x07, 0x33, 0x1A, 0x00, 0x05, 0x4F, 0x22 };
	private byte[]		af_binary50_2c3 = { 0x24, 0x1F, 0x14, 0x77, 0x2F, 0x0A, 0x5E, 0x5B, 0x1E, 0x5C, 0x0F, 0x34, 0x24, 0x3B, 0x61, 0x2E, 0x13, 0x0B, 0x0E, 0x14, 0x77, 0x2F, 0x0A, 0x5E, 0x49, 0x5D, 0x1B, 0x3C, 0x19, 0x44, 0x41, 0x5F, 0x29, 0x28, 0x7D, 0x3E, 0x5B, 0x2E, 0x3B, 0x4D, 0x1E, 0x2C, 0x1B, 0x1C, 0x04, 0x08, 0x0A, 0x20, 0x2D, 0x0F };
	private byte[]		af_binary50_2c4 = { 0x2A, 0x5B, 0x0B, 0x2F, 0x3C, 0x5E, 0x68, 0x1A, 0x19, 0x5A, 0x0D, 0x2A, 0x25, 0x29, 0x4C, 0x28, 0x0D, 0x3D, 0x1F, 0x45, 0x1D, 0x5A, 0x5D, 0x2B, 0x4E, 0x39, 0x64, 0x0A, 0x5A, 0x1B, 0x49, 0x5D, 0x1B, 0x29, 0x0C, 0x24, 0x1F, 0x14, 0x77, 0x2F, 0x0A, 0x5E, 0x5B, 0x1E, 0x5D, 0x1B, 0x29, 0x2F, 0x0D, 0x3D };
	private byte[]		af_binary50_2c5 = { 0x2C, 0x5B, 0x19, 0x7A, 0x0F, 0x2C, 0x67, 0x40, 0x47, 0x0A, 0x7D, 0x3E, 0x5B, 0x30, 0x1D, 0x3D, 0x5E, 0x0E, 0x1A, 0x0C, 0x7D, 0x3E, 0x5B, 0x2E, 0x3B, 0x4D, 0x1E, 0x2C, 0x1B, 0x1C, 0x59, 0x3D, 0x2F, 0x11, 0x0F, 0x45, 0x2F, 0x78, 0x38, 0x30, 0x1D, 0x3D, 0x5E, 0x0A, 0x69, 0x64, 0x78, 0x5D, 0x2C, 0x3D };
	private byte[][]	af_binary50_2 = { af_binary50_2c1, af_binary50_2c2, af_binary50_2c3, af_binary50_2c4, af_binary50_2c5 };
	private byte[]		af_binary50_3c1 = { 0x3A, 0x6C, 0x5F, 0x77, 0x34, 0x6C, 0x5D, 0x23, 0x52, 0x5B, 0x19, 0x7A, 0x0F, 0x1C, 0x50, 0x2E, 0x0D, 0x2C, 0x67, 0x40, 0x47, 0x0A, 0x7D, 0x3E, 0x5B, 0x2E, 0x0F, 0x2A, 0x72, 0x38, 0x1A, 0x48, 0x28, 0x1E, 0x55, 0x44, 0x3B, 0x7A, 0x34, 0x1A, 0x20, 0x40, 0x79, 0x0A, 0x3B, 0x29, 0x5F, 0x4C, 0x28, 0x37 };
	private byte[]		af_binary50_3c2 = { 0x1A, 0x14, 0x77, 0x2F, 0x0A, 0x5E, 0x5B, 0x6C, 0x6B, 0x07, 0x44, 0x09, 0x74, 0x3A, 0x2F, 0x18, 0x27, 0x4B, 0x0F, 0x6F, 0x3D, 0x24, 0x0C, 0x7E, 0x3F, 0x3E, 0x2E, 0x0B, 0x10, 0x44, 0x4D, 0x08, 0x4B, 0x2F, 0x71, 0x31, 0x20, 0x3B, 0x64, 0x0A, 0x5A, 0x1B, 0x49, 0x5D, 0x1B, 0x29, 0x0C, 0x43, 0x6C, 0x4A };
	private byte[]		af_binary50_3c3 = { 0x7B, 0x24, 0x3B, 0x66, 0x0D, 0x29, 0x51, 0x18, 0x6A, 0x0F, 0x0E, 0x38, 0x0A, 0x29, 0x1F, 0x30, 0x78, 0x2E, 0x2D, 0x19, 0x1E, 0x09, 0x4A, 0x4E, 0x6C, 0x2A, 0x0C, 0x39, 0x0C, 0x26, 0x3D, 0x3D, 0x5A, 0x2C, 0x21, 0x7C, 0x08, 0x3C, 0x18, 0x7F, 0x76, 0x75, 0x0D, 0x1D, 0x4F, 0x3A, 0x29, 0x38, 0x66, 0x51 };
	private byte[][]	af_binary50_3 = { af_binary50_3c1, af_binary50_3c2, af_binary50_3c3 };
	private byte[]		af_binary50_4c1 = { 0x33, 0x0A, 0x7D, 0x3E, 0x5B, 0x2E, 0x0F, 0x14, 0x77, 0x2F, 0x0A, 0x5E, 0x5B, 0x6C, 0x6B, 0x29, 0x5F, 0x4C, 0x2F, 0x72, 0x2C, 0x39, 0x2E, 0x22, 0x16, 0x64, 0x61, 0x1C, 0x08, 0x5F, 0x3F, 0x1A, 0x3D, 0x2F, 0x0D, 0x2C, 0x37, 0x40, 0x47, 0x0A, 0x7D, 0x3E, 0x5B, 0x2E, 0x3B, 0x4D, 0x1E, 0x2C, 0x1B, 0x1C };
	private byte[]		af_binary50_4c2 = { 0x40, 0x5B, 0x1E, 0x5C, 0x0F, 0x34, 0x24, 0x3B, 0x2C, 0x5B, 0x19, 0x4C, 0x2A, 0x28, 0x1A, 0x0A, 0x3A, 0x28, 0x24, 0x6A, 0x5B, 0x2C, 0x29, 0x73, 0x48, 0x20, 0x3B, 0x19, 0x19, 0x37, 0x64, 0x1A, 0x73, 0x33, 0x75, 0x18, 0x65, 0x61, 0x39, 0x0E, 0x43, 0x3F, 0x2B, 0x5B, 0x2B, 0x3B, 0x0A, 0x15, 0x30, 0x33 };
	private byte[]		af_binary50_4c3 = { 0x2A, 0x25, 0x29, 0x4C, 0x28, 0x3E, 0x1C, 0x58, 0x79, 0x19, 0x28, 0x46, 0x18, 0x1A, 0x23, 0x65, 0x59, 0x3D, 0x2F, 0x74, 0x28, 0x5A, 0x18, 0x01, 0x2A, 0x5B, 0x0B, 0x2F, 0x3C, 0x5E, 0x68, 0x0F, 0x34, 0x24, 0x3B, 0x61, 0x2E, 0x13, 0x0B, 0x0E, 0x20, 0x3B, 0x64, 0x0A, 0x5A, 0x1B, 0x49, 0x5D, 0x1B, 0x29 };
	private byte[]		af_binary50_4c4 = { 0x76, 0x7F, 0x7F, 0x0C, 0x3F, 0x1E, 0x71, 0x25, 0x18, 0x1D, 0x7D, 0x0E, 0x38, 0x0A, 0x29, 0x1F, 0x30, 0x78, 0x2E, 0x2D, 0x7C, 0x5F, 0x4D, 0x08, 0x08, 0x5B, 0x77, 0x18, 0x17, 0x3C, 0x19, 0x44, 0x41, 0x5F, 0x29, 0x28, 0x5F, 0x29, 0x28, 0x7D, 0x3E, 0x5B, 0x2E, 0x3B, 0x4D, 0x1E, 0x2C, 0x1B, 0x1C, 0x04 };
	private byte[][]	af_binary50_4 = { af_binary50_4c1, af_binary50_4c2, af_binary50_4c3, af_binary50_4c4 };
	private byte[]		af_binary50_5c1 = { 0x7F, 0x6C, 0x5D, 0x23, 0x52, 0x5B, 0x19, 0x7A, 0x0F, 0x1C, 0x50, 0x08, 0x4B, 0x2F, 0x2D, 0x0F, 0x72, 0x72, 0x2A, 0x2F, 0x35, 0x5C, 0x4E, 0x18, 0x39, 0x13, 0x48, 0x36, 0x28, 0x10, 0x3D, 0x28, 0x32, 0x09, 0x3A, 0x1E, 0x5C, 0x40, 0x21, 0x0D, 0x3D, 0x1F, 0x45, 0x1D, 0x5A, 0x5D, 0x2B, 0x4E, 0x39, 0x53 };
	private byte[]		af_binary50_5c2 = { 0x11, 0x77, 0x2F, 0x0A, 0x5E, 0x5B, 0x6C, 0x6B, 0x07, 0x44, 0x09, 0x74, 0x3A, 0x2F, 0x18, 0x27, 0x1B, 0x0D, 0x2C, 0x37, 0x40, 0x47, 0x0A, 0x7D, 0x3E, 0x5B, 0x2E, 0x3B, 0x4D, 0x1E, 0x2C, 0x1B, 0x24, 0x0C, 0x7E, 0x3F, 0x3E, 0x2E, 0x0B, 0x10, 0x44, 0x4D, 0x08, 0x4B, 0x2F, 0x2D, 0x0F, 0x77, 0x05, 0x3B };
	private byte[]		af_binary50_5c3 = { 0x4C, 0x24, 0x0C, 0x7E, 0x3F, 0x3E, 0x2E, 0x0B, 0x10, 0x44, 0x4D, 0x3B, 0x24, 0x1F, 0x14, 0x77, 0x2F, 0x0A, 0x5E, 0x5B, 0x6C, 0x6B, 0x24, 0x3B, 0x2C, 0x5B, 0x19, 0x7A, 0x0F, 0x1C, 0x50, 0x0A, 0x5E, 0x5B, 0x6C, 0x5F, 0x77, 0x34, 0x6C, 0x5D, 0x23, 0x52, 0x61, 0x74, 0x3A, 0x2F, 0x18, 0x27, 0x4B, 0x7F };
	private byte[][]	af_binary50_5 = { af_binary50_5c1, af_binary50_5c2, af_binary50_5c3 };
	private byte[]		af_binary50_6c1 = { 0x2E, 0x0D, 0x2C, 0x67, 0x40, 0x47, 0x0A, 0x29, 0x5F, 0x4C, 0x2F, 0x72, 0x2C, 0x39, 0x6B, 0x07, 0x44, 0x09, 0x18, 0x1D, 0x7D, 0x6F, 0x2F, 0x2A, 0x73, 0x28, 0x09, 0x41, 0x5A, 0x1A, 0x5D, 0x24, 0x2C, 0x0A, 0x51, 0x2E, 0x2B, 0x5E, 0x0A, 0x2C, 0x39, 0x0F, 0x48, 0x22, 0x16, 0x64, 0x61, 0x1C, 0x08, 0x44 };
	private byte[]		af_binary50_6c2 = { 0x3D, 0x5A, 0x2C, 0x21, 0x7C, 0x08, 0x3C, 0x18, 0x7F, 0x76, 0x75, 0x0D, 0x1D, 0x4F, 0x6C, 0x5D, 0x23, 0x52, 0x5B, 0x19, 0x7A, 0x0F, 0x07, 0x44, 0x09, 0x74, 0x3A, 0x2F, 0x18, 0x27, 0x1B, 0x3E, 0x2E, 0x0B, 0x10, 0x44, 0x4D, 0x08, 0x4B, 0x2F, 0x2D, 0x0F, 0x1B, 0x0D, 0x2C, 0x37, 0x40, 0x47, 0x0A, 0x7D };
	private byte[]		af_binary50_6c3 = { 0x65, 0x59, 0x3D, 0x2F, 0x74, 0x28, 0x5A, 0x18, 0x09, 0x1A, 0x4E, 0x4C, 0x40, 0x7B, 0x25, 0x29, 0x4C, 0x28, 0x3E, 0x1C, 0x28, 0x1A, 0x0A, 0x3A, 0x28, 0x24, 0x6A, 0x0D, 0x3D, 0x1F, 0x45, 0x1D, 0x5A, 0x5D, 0x2B, 0x4E, 0x39, 0x64, 0x17, 0x3C, 0x19, 0x44, 0x41, 0x5F, 0x29, 0x28, 0x2C, 0x47, 0x40, 0x47 };
	private byte[][]	af_binary50_6 = { af_binary50_6c1, af_binary50_6c2, af_binary50_6c3 };
	private byte[][][]	af_binary50s = { null, af_binary50_2, af_binary50_3, af_binary50_4, af_binary50_5, af_binary50_6 };

	// テスト用の表を作成し、タプルの挿入を行う
	// 挿入したタプル数を返す
	private void createAndInsertTestTable(Connection	c) throws Exception
	{
		createTestTable(c);

		PreparedStatement	ps = null;
		String	query;

		query = "insert into t values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

		assertNotNull(ps = c.prepareStatement(query));

		for (int i = 0; i < af_ints.length; i++) {

			int	columnIndex = 1;

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

			int	expected = 1;
			assertEquals(expected, ps.executeUpdate());
		}

		ps.close();
	}

	private void dropTestTable(Connection	c) throws Exception
	{
		Statement	s = null;
		assertNotNull(s = c.createStatement());
		assertEquals(0, s.executeUpdate("drop table t"));
		s.close();
	}

	private void assertArray(	Object	expected,
								Object	actual) throws Exception
	{
		if (expected == null) {
			assertNull(actual);
		} else {
			assertNotNull(actual);
			String	actualClass = actual.getClass().getName();
			if (actualClass.compareTo("[Ljava.lang.Integer;") == 0) {
				Integer[]	exp = (Integer[])expected;
				Integer[]	act = (Integer[])actual;
				assertEquals(exp.length, act.length);
				for (int i = 0; i < exp.length; i++) {
					assertEquals(exp[i].intValue(), act[i].intValue());
				}
			} else if (actualClass.compareTo("[Ljava.lang.Long;") == 0) {
				Long[]	exp = (Long[])expected;
				Long[]	act = (Long[])actual;
				assertEquals(exp.length, act.length);
				for (int i = 0; i < exp.length; i++) {
					assertEquals(exp[i].longValue(), act[i].longValue());
				}
			} else if (actualClass.compareTo("[Ljava.math.BigDecimal;") == 0) {
				BigDecimal[]	exp = (BigDecimal[])expected;
				BigDecimal[]	act = (BigDecimal[])actual;
				assertEquals(exp.length, act.length);
				for (int i = 0; i < exp.length; i++) {
					assertEquals(exp[i].setScale(5), act[i]);
				}
			} else if (actualClass.compareTo("[Ljava.lang.String;") == 0) {
				String[]	exp = (String[])expected;
				String[]	act = (String[])actual;
				assertEquals(exp.length, act.length);
				for (int i = 0; i < exp.length; i++) {
					assertEquals(exp[i], act[i]);
				}
			} else if (actualClass.compareTo("[Ljava.lang.Double;") == 0) {
				Double[]	exp = (Double[])expected;
				Double[]	act = (Double[])actual;
				assertEquals(exp.length, act.length);
				for (int i = 0; i < exp.length; i++) {
					assertEquals(exp[i].doubleValue(), act[i].doubleValue());
				}
			} else if (actualClass.compareTo("[Ljava.sql.Timestamp;") == 0) {
				Timestamp[]	exp = (Timestamp[])expected;
				Timestamp[]	act = (Timestamp[])actual;
				assertEquals(exp.length, act.length);
				for (int i = 0; i < exp.length; i++) {
					assertEquals(exp[i].toString(), act[i].toString());
				}
			} else if (actualClass.compareTo("[[B") == 0) {
				byte[][]	exp = (byte[][])expected;
				byte[][]	act = (byte[][])actual;
				assertEquals(exp.length, act.length);
				for (int i = 0; i < exp.length; i++) {
					assertEquals(exp[i], act[i]);
				}
			} else if (actualClass.compareTo("[Ljp.co.ricoh.doquedb.common.LanguageData;") == 0) {
				String[]		exp = (String[])expected;
				LanguageData[]	act = (LanguageData[])actual;
				assertEquals(exp.length, act.length);
				for (int i = 0; i < exp.length; i++) {
					assertEquals(exp[i], act[i].toString());
				}
			} else {
				System.out.println("??? assertArray");
				System.out.println("EXPECTED CLASS : \"" + expected.getClass().getName() + "\"");
				System.out.println("ACTUAL CLASS   : \"" + actual.getClass().getName() + "\"");
			}
		}
	}

	private void assertArray(	Object	expected,
								Object	actual,
								int		index,
								int		count) throws Exception
	{
		if (expected == null) {
			assertNull(actual);
		} else {
			assertNotNull(actual);
			String	actualClass = actual.getClass().getName();
			if (actualClass.compareTo("[Ljava.lang.Integer;") == 0) {
				Integer[]	exp = (Integer[])expected;
				Integer[]	act = (Integer[])actual;
				for (int expIndex = index - 1, actIndex = 0; actIndex < act.length; expIndex++, actIndex++) {
					assertEquals(exp[expIndex].intValue(), act[actIndex].intValue());
				}
			} else if (actualClass.compareTo("[Ljava.lang.Long;") == 0) {
				Long[]	exp = (Long[])expected;
				Long[]	act = (Long[])actual;
				for (int expIndex = index - 1, actIndex = 0; actIndex < act.length; expIndex++, actIndex++) {
					assertEquals(exp[expIndex].longValue(), act[actIndex].longValue());
				}
			} else if (actualClass.compareTo("[Ljava.math.BigDecimal;") == 0) {
				BigDecimal[]	exp = (BigDecimal[])expected;
				BigDecimal[]	act = (BigDecimal[])actual;
				for (int expIndex = index - 1, actIndex = 0; actIndex < act.length; expIndex++, actIndex++) {
					assertEquals(exp[expIndex].setScale(5), act[actIndex]);
				}
			} else if (actualClass.compareTo("[Ljava.lang.String;") == 0) {
				String[]	exp = (String[])expected;
				String[]	act = (String[])actual;
				for (int expIndex = index - 1, actIndex = 0; actIndex < act.length; expIndex++, actIndex++) {
					assertEquals(exp[expIndex], act[actIndex]);
				}
			} else if (actualClass.compareTo("[Ljava.lang.Double;") == 0) {
				Double[]	exp = (Double[])expected;
				Double[]	act = (Double[])actual;
				for (int expIndex = index - 1, actIndex = 0; actIndex < act.length; expIndex++, actIndex++) {
					assertEquals(exp[expIndex].doubleValue(), act[actIndex].doubleValue());
				}
			} else if (actualClass.compareTo("[Ljava.sql.Timestamp;") == 0) {
				Timestamp[]	exp = (Timestamp[])expected;
				Timestamp[]	act = (Timestamp[])actual;
				for (int expIndex = index - 1, actIndex = 0; actIndex < act.length; expIndex++, actIndex++) {
					assertEquals(exp[expIndex].toString(), act[actIndex].toString());
				}
			} else if (actualClass.compareTo("[[B") == 0) {
				byte[][]	exp = (byte[][])expected;
				byte[][]	act = (byte[][])actual;
				for (int expIndex = index - 1, actIndex = 0; actIndex < act.length; expIndex++, actIndex++) {
					assertEquals(exp[expIndex], act[actIndex]);
				}
			} else if (actualClass.compareTo("[Ljp.co.ricoh.doquedb.common.LanguageData;") == 0) {
				String[]		exp = (String[])expected;
				LanguageData[]	act = (LanguageData[])actual;
				for (int expIndex = index - 1, actIndex = 0; actIndex < act.length; expIndex++, actIndex++) {
					assertEquals(exp[expIndex], act[actIndex].toString());
				}
			} else {
				System.out.println("??? assertArray");
				System.out.println("EXPECTED CLASS : \"" + expected.getClass().getName() + "\"");
				System.out.println("ACTUAL CLASS   : \"" + actual.getClass().getName() + "\"");
			}
		}
	}

	private void assertArray(	Object		expected,
								ResultSet	actual,
								int			baseType) throws Exception
	{
		if (expected == null) {
			assertNull(actual);
		} else {
			assertNotNull(actual);
			switch (baseType) {
			case Types.INTEGER:
				{
					Integer[]	exp = (Integer[])expected;
					for (int i = 0; i < exp.length; i++) {
						assertTrue(actual.next());
						assertEquals(i + 1, actual.getInt(1));				// 要素のインデックス
						assertEquals(exp[i].intValue(), actual.getInt(2));	// 要素の値
					}
				}
				break;
			case Types.BIGINT:
				{
					Long[]	exp = (Long[])expected;
					for (int i = 0; i < exp.length; i++) {
						assertTrue(actual.next());
						assertEquals(i + 1, actual.getInt(1));					// 要素のインデックス
						assertEquals(exp[i].longValue(), actual.getLong(2));	// 要素の値
					}
				}
				break;
			case Types.DECIMAL:
				{
					BigDecimal[]	exp = (BigDecimal[])expected;
					for (int i = 0; i < exp.length; i++) {
						assertTrue(actual.next());
						assertEquals(i + 1, actual.getInt(1));				// 要素のインデックス
						assertEquals(exp[i].setScale(5), actual.getBigDecimal(2));		// 要素の値
					}
				}
				break;
			case Types.CHAR:
			case Types.VARCHAR:
			case Types.OTHER:
				{
					String[]	exp = (String[])expected;
					for (int i = 0; i < exp.length; i++) {
						assertTrue(actual.next());
						assertEquals(i + 1, actual.getInt(1));		// 要素のインデックス
						assertEquals(exp[i], actual.getString(2));	// 要素の値
					}
				}
				break;
			case Types.DOUBLE:
				{
					Double[]	exp = (Double[])expected;
					for (int i = 0; i < exp.length; i++) {
						assertTrue(actual.next());
						assertEquals(i + 1, actual.getInt(1));						// 要素のインデックス
						assertEquals(exp[i].doubleValue(), actual.getDouble(2));	// 要素の値
					}
				}
				break;
			case Types.TIMESTAMP:
				{
					Timestamp[]	exp = (Timestamp[])expected;
					for (int i = 0; i < exp.length; i++) {
						assertTrue(actual.next());
						assertEquals(i + 1, actual.getInt(1));								// 要素のインデックス
						assertEquals(exp[i].getTime(), actual.getTimestamp(2).getTime());	// 要素の値
					}
				}
				break;
			case Types.VARBINARY:
			case Types.BINARY:
				{
					byte[][]	exp = (byte[][])expected;
					for (int i = 0; i < exp.length; i++) {
						assertTrue(actual.next());
						assertEquals(i + 1, actual.getInt(1));		// 要素のインデックス
						assertEquals(exp[i], actual.getBytes(2));	// 要素の値
					}
				}
				break;
			default:
				System.out.println("??? assertArray");
				System.out.println("baseType = " + baseType);
			}
			assertFalse(actual.next());
			actual.close();
		}
	}

	private void assertArray(	Object		expected,
								ResultSet	actual,
								int			baseType,
								int			index,
								int			count) throws Exception
	{
		if (expected == null) {
			assertNull(actual);
		} else {
			assertNotNull(actual);
			int	start = index - 1;
			int	end = start + count;
			switch (baseType) {
			case Types.INTEGER:
				{
					Integer[]	exp = (Integer[])expected;
					for (int i = 0, expIndex = start; expIndex < end; i++, expIndex++) {
						assertTrue(actual.next());
						assertEquals(index + i, actual.getInt(1));					// 要素のインデックス
						assertEquals(exp[expIndex].intValue(), actual.getInt(2));	// 要素の値
					}
				}
				break;
			case Types.BIGINT:
				{
					Long[]	exp = (Long[])expected;
					for (int i = 0, expIndex = start; expIndex < end; i++, expIndex++) {
						assertTrue(actual.next());
						assertEquals(index + i, actual.getInt(1));					// 要素のインデックス
						assertEquals(exp[expIndex].longValue(), actual.getLong(2));	// 要素の値
					}
				}
				break;
			case Types.DECIMAL:
				{
					BigDecimal[]	exp = (BigDecimal[])expected;
					for (int i = 0, expIndex = start; expIndex < end; i++, expIndex++) {
						assertTrue(actual.next());
						assertEquals(index + i, actual.getInt(1));				// 要素のインデックス
						assertEquals(exp[expIndex].setScale(5), actual.getBigDecimal(2));	// 要素の値
					}
				}
				break;
			case Types.CHAR:
			case Types.VARCHAR:
			case Types.OTHER:
				{
					String[]	exp = (String[])expected;
					for (int i = 0, expIndex = start; expIndex < end; i++, expIndex++) {
						assertTrue(actual.next());
						assertEquals(index + i, actual.getInt(1));			// 要素のインデックス
						assertEquals(exp[expIndex], actual.getString(2));	// 要素の値
					}
				}
				break;
			case Types.DOUBLE:
				{
					Double[]	exp = (Double[])expected;
					for (int i = 0, expIndex = start; expIndex < end; i++, expIndex++) {
						assertTrue(actual.next());
						assertEquals(index + i, actual.getInt(1));						// 要素のインデックス
						assertEquals(exp[expIndex].doubleValue(), actual.getDouble(2));	// 要素の値
					}
				}
				break;
			case Types.TIMESTAMP:
				{
					Timestamp[]	exp = (Timestamp[])expected;
					for (int i = 0, expIndex = start; expIndex < end; i++, expIndex++) {
						assertTrue(actual.next());
						assertEquals(index + i, actual.getInt(1));									// 要素のインデックス
						assertEquals(exp[expIndex].getTime(), actual.getTimestamp(2).getTime());	// 要素の値
					}
				}
				break;
			case Types.VARBINARY:
			case Types.BINARY:
				{
					byte[][]	exp = (byte[][])expected;
					for (int i = 0, expIndex = start; expIndex < end; i++, expIndex++) {
						assertTrue(actual.next());
						assertEquals(index + i, actual.getInt(1));			// 要素のインデックス
						assertEquals(exp[expIndex], actual.getBytes(2));	// 要素の値
					}
				}
				break;
			default:
				System.out.println("??? assertArray");
				System.out.println("baseType = " + baseType);
			}
			assertFalse(actual.next());
			actual.close();
		}
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
}

//
// Copyright (c) 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
