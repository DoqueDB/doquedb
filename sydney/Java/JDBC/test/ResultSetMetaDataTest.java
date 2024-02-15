// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ResultSetMetaDataTest.java -- jp.co.ricoh.doquedb.jdbc.ResultSetMetaData クラスのテスト
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
import java.math.BigDecimal;

import jp.co.ricoh.doquedb.common.*;
import jp.co.ricoh.doquedb.exception.*;

public class ResultSetMetaDataTest extends TestBase
{
	public ResultSetMetaDataTest(String nickname)
	{
		super(nickname);
	}

	// ResultSetMetaData.getColumnCount() のテスト
	public void test_getColumnCount() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 2;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		java.sql.ResultSetMetaData	rsm = null;
		assertNotNull(rsm = rs.getMetaData());
		assertEquals(numberOfColumns, rsm.getColumnCount());
		rs.close();

		assertNotNull(rs = s.executeQuery("select * from t1, t2"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(numberOfColumns * numberOfTables, rsm.getColumnCount());
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(1, rsm.getColumnCount());
		rs.close();

		assertNotNull(rs = s.executeQuery("select RowID from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(1, rsm.getColumnCount());
		rs.close();

		assertNotNull(rs = s.executeQuery("select RowID as rid from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(1, rsm.getColumnCount());
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_int1, f_int2 from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(2, rsm.getColumnCount());
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_int1 as primarykey, f_int2, f_char8 from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(3, rsm.getColumnCount());
		rs.close();

		assertNotNull(rs = s.executeQuery("select t1.f_language, t2.f_id, t1.f_char8, t1.af_int, t2.f_blob from t1, t2"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(5, rsm.getColumnCount());
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.isAutoIncrement() のテスト
	public void test_isAutoIncrement() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		assertTrue(rs.next());
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	cc = rsm.getColumnCount();
		assertEquals(numberOfColumns, cc);
		for (int i = 1; i <= cc; i++) assertFalse(rsm.isAutoIncrement(i));
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_int1 from t1"));
		assertTrue(rs.next());

		assertNotNull(rsm = rs.getMetaData());
		cc = rsm.getColumnCount();
		assertEquals(1, cc);
		for (int i = 1; i <= cc; i++) assertFalse(rsm.isAutoIncrement(i));
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) from t1"));
		assertTrue(rs.next());

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isAutoIncrement(1));
		rs.close();

		assertNotNull(rs = s.executeQuery("select RowID from t1"));
		assertTrue(rs.next());

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rsm.isAutoIncrement(1));
		rs.close();

		assertNotNull(rs = s.executeQuery("select RowID as rid from t1"));
		assertTrue(rs.next());

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rsm.isAutoIncrement(1));
		rs.close();

		assertNotNull(rs = s.executeQuery("select RowID, f_int1 from t1"));
		assertTrue(rs.next());

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rsm.isAutoIncrement(1));
		assertFalse(rsm.isAutoIncrement(2));
		rs.close();

		assertNotNull(rs = s.executeQuery("select RowID as rid, f_int1 as primarykey from t1"));
		assertTrue(rs.next());

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rsm.isAutoIncrement(1));
		assertFalse(rsm.isAutoIncrement(2));
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.isCaseSensitive() のテスト
	public void test_isCaseSensitive() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	columnIndex = 1;
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_int_not_null		int not null
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_int1				int
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_int2				int
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_bigint				bigint
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_decimal			decimal(10,5)
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_char8_not_null		char(8) not null
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_char8				char(8)
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_float				float
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_datetime			datetime		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_id					uniqueidentifier
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_image				image
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_language			language
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_nchar6				nchar(6)
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_nvarchar256		nvarchar(256)
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_varchar128			varchar(128)
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_ntext				ntext
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_ntext_compressed	ntext hint heap 'compressed'
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_fulltext			fulltext
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_binary50			binary(50)
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_blob				blob
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_nclob				nclob
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// af_int				int					array[no limit]
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// af_bigint			bigint				array[no limit]
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// af_decimal			decimal(10,5)		array[no limit]
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// af_char8				char(8)				array[no limit]
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// af_float				float				array[no limit]
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// af_datetime			datetime			array[no limit]
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// af_id				uniqueidentifier	array[no limit]
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// af_image				image				array[no limit]
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// af_language			language			array[no limit]
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// af_nchar6			nchar(6)			array[no limit]
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// af_varchar128		varchar(128)		array[no limit]
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// af_ntext				ntext				array[no limit]
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// af_fulltext			fulltext			array[no limit]
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// af_binary50			binary(50)			array[no limit]
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_int2, f_blob, af_id, f_nvarchar256 from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_int2
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_blob
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// af_id
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_nvarchar256
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rsm.isCaseSensitive(1));	// count(*)
		rs.close();

		assertNotNull(rs = s.executeQuery("select max(f_int1) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rsm.isCaseSensitive(1));	// max(f_int1)
		rs.close();

		assertNotNull(rs = s.executeQuery("select min(f_int2) as minv from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rsm.isCaseSensitive(1));	// min(f_int2) as minv
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_blob as binarydata, f_int1 as keydata, f_nchar6, f_id as xid from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_blob as binarydata
 		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_int1 as keydata
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_nchar
		assertTrue(rsm.isCaseSensitive(columnIndex++));	// f_id as xid
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.isSearchable() のテスト
	public void test_isSearchable() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		assertTrue(rs.next());
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	columnIndex = 1;
		assertTrue(rsm.isSearchable(columnIndex++));	// f_int_not_null		int not null
		assertTrue(rsm.isSearchable(columnIndex++));	// f_int1				int
		assertTrue(rsm.isSearchable(columnIndex++));	// f_int2				int
		assertTrue(rsm.isSearchable(columnIndex++));	// f_bigint				bigint
		assertTrue(rsm.isSearchable(columnIndex++));	// f_decimal			decimal(10,5)
		assertTrue(rsm.isSearchable(columnIndex++));	// f_char8_not_null		char(8) not null
		assertTrue(rsm.isSearchable(columnIndex++));	// f_char8				char(8)
		assertTrue(rsm.isSearchable(columnIndex++));	// f_float				float
		assertTrue(rsm.isSearchable(columnIndex++));	// f_datetime			datetime
		assertTrue(rsm.isSearchable(columnIndex++));	// f_id					uniqueidentifier
		assertTrue(rsm.isSearchable(columnIndex++));	// f_image				image
		assertTrue(rsm.isSearchable(columnIndex++));	// f_language			language
		assertTrue(rsm.isSearchable(columnIndex++));	// f_nchar6				nchar(6)
		assertTrue(rsm.isSearchable(columnIndex++));	// f_nvarchar256		nvarchar(256)
		assertTrue(rsm.isSearchable(columnIndex++));	// f_varchar128			varchar(128)
		assertTrue(rsm.isSearchable(columnIndex++));	// f_ntext				ntext
		assertTrue(rsm.isSearchable(columnIndex++));	// f_ntext_compressed	ntext hint heap 'compressed'
		assertTrue(rsm.isSearchable(columnIndex++));	// f_fulltext			fulltext
		assertTrue(rsm.isSearchable(columnIndex++));	// f_binary50			binary(50)
		assertTrue(rsm.isSearchable(columnIndex++));	// f_blob				blob
		assertTrue(rsm.isSearchable(columnIndex++));	// f_nclob				nclob
		assertTrue(rsm.isSearchable(columnIndex++));	// af_int				int					array[no limit]
		assertTrue(rsm.isSearchable(columnIndex++));	// af_bigint			bigint				array[no limit]
		assertTrue(rsm.isSearchable(columnIndex++));	// af_decimal			decimal(10,5)		array[no limit]
		assertTrue(rsm.isSearchable(columnIndex++));	// af_char8				char(8)				array[no limit]
		assertTrue(rsm.isSearchable(columnIndex++));	// af_float				float				array[no limit]
		assertTrue(rsm.isSearchable(columnIndex++));	// af_datetime			datetime			array[no limit]
		assertTrue(rsm.isSearchable(columnIndex++));	// af_id				uniqueidentifier	array[no limit]
		assertTrue(rsm.isSearchable(columnIndex++));	// af_image				image				array[no limit]
		assertTrue(rsm.isSearchable(columnIndex++));	// af_language			language			array[no limit]
		assertTrue(rsm.isSearchable(columnIndex++));	// af_nchar6			nchar(6)			array[no limit]
		assertTrue(rsm.isSearchable(columnIndex++));	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertTrue(rsm.isSearchable(columnIndex++));	// af_varchar128		varchar(128)		array[no limit]
		assertTrue(rsm.isSearchable(columnIndex++));	// af_ntext				ntext				array[no limit]
		assertTrue(rsm.isSearchable(columnIndex++));	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertTrue(rsm.isSearchable(columnIndex++));	// af_fulltext			fulltext			array[no limit]
		assertTrue(rsm.isSearchable(columnIndex++));	// af_binary50			binary(50)			array[no limit]
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_float, f_varchar128, f_int1, af_ntext, f_blob from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertTrue(rsm.isSearchable(columnIndex++));	// f_float
		assertTrue(rsm.isSearchable(columnIndex++));	// f_varchar128
		assertTrue(rsm.isSearchable(columnIndex++));	// f_int1
		assertTrue(rsm.isSearchable(columnIndex++));	// af_ntext
		assertTrue(rsm.isSearchable(columnIndex++));	// f_blob
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_int1 as n1, f_int2 as n2 from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertTrue(rsm.isSearchable(columnIndex++));	// f_int1 as n1
		assertTrue(rsm.isSearchable(columnIndex++));	// f_int2 as n2
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isSearchable(1));	// count(*)
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) as numtuple from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isSearchable(1));	// count(*) as numtuple
		rs.close();

		assertNotNull(rs = s.executeQuery("select max(f_int1) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isSearchable(1));	// max(f_int1)
		rs.close();

		assertNotNull(rs = s.executeQuery("select max(f_int1) as maxv from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isSearchable(1));	// max(f_int1) as maxv
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rsm.isSearchable(1));	// ROWID
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID as rid from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rsm.isSearchable(1));	// ROWID as rid
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.isCurrency() のテスト
	public void test_isCurrency() throws Exception
	{
		//
		// isCurrency() は、常に false
		//

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	cc = rsm.getColumnCount();
		for (int i = 1; i <= cc; i++) assertFalse(rsm.isCurrency(i));
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_ntext, f_blob, f_id, f_int1 from t1"));

		assertNotNull(rsm = rs.getMetaData());
		for (int i = 1; i <= 4; i++) assertFalse(rsm.isCurrency(i));
		rs.close();

		assertNotNull(rs = s.executeQuery("select max(f_int2) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isCurrency(1));
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.isNullable() のテスト
	public void test_isNullable() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	cc = rsm.getColumnCount();
		for (int i = 1; i <= cc; i++) {
			int	expected = java.sql.ResultSetMetaData.columnNullable;
			switch (i) {
			case 1:	// f_int_not_null	int not null
			case 2:	// f_int1			int					(primary key)
			case 6:	// f_char8_not_null	char(8) not null
				expected = java.sql.ResultSetMetaData.columnNoNulls;
				break;
			}
			assertEquals(expected, rsm.isNullable(i));
		}
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_int_not_null, f_int2, f_char8_not_null, f_char8, f_int1 from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rs.next());
		for (int i = 1; i <= 5; i++) {
			int	expected = java.sql.ResultSetMetaData.columnNullable;
			switch (i) {
			case 1:	// f_int_not_null
			case 3:	// f_char8_not_null
			case 5:	// f_int1
				expected = java.sql.ResultSetMetaData.columnNoNulls;
				break;
			}
			assertEquals(expected, rsm.isNullable(i));
		}
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rs.next());
		assertEquals(java.sql.ResultSetMetaData.columnNoNulls, rsm.isNullable(1));	// ROWID
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID as rid from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(java.sql.ResultSetMetaData.columnNoNulls, rsm.isNullable(1));	// ROWID as rid
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(java.sql.ResultSetMetaData.columnNullable, rsm.isNullable(1));	// count(*)
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) as numtuple from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(java.sql.ResultSetMetaData.columnNullable, rsm.isNullable(1));	// count(*) as numtuple
		rs.close();

		//
		// タプルが 0 件の場合には null を返すので max() は nullable
		//

		assertNotNull(rs = s.executeQuery("select max(f_int1) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(java.sql.ResultSetMetaData.columnNullable, rsm.isNullable(1));	// max(f_int1)
		rs.close();

		assertNotNull(rs = s.executeQuery("select max(f_int1) as maxv from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(java.sql.ResultSetMetaData.columnNullable, rsm.isNullable(1));	// max(f_int1) as maxv
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.isSigned() のテスト
	public void test_isSigned() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		assertTrue(rs.next());
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	columnIndex = 1;
		assertTrue(rsm.isSigned(columnIndex++));	// f_int_not_null		int not null
		assertTrue(rsm.isSigned(columnIndex++));	// f_int1				int
		assertTrue(rsm.isSigned(columnIndex++));	// f_int2				int
		assertTrue(rsm.isSigned(columnIndex++));	// f_bigint				bigint
		assertTrue(rsm.isSigned(columnIndex++));	// f_decimal			decimal(10,5)
		assertTrue(rsm.isSigned(columnIndex++));	// f_char8_not_null		char(8) not null
		assertTrue(rsm.isSigned(columnIndex++));	// f_char8				char(8)
		assertTrue(rsm.isSigned(columnIndex++));	// f_float				float
		assertTrue(rsm.isSigned(columnIndex++));	// f_datetime			datetime
		assertTrue(rsm.isSigned(columnIndex++));	// f_id					uniqueidentifier
		assertTrue(rsm.isSigned(columnIndex++));	// f_image				image
		assertTrue(rsm.isSigned(columnIndex++));	// f_language			language
		assertTrue(rsm.isSigned(columnIndex++));	// f_nchar6				nchar(6)
		assertTrue(rsm.isSigned(columnIndex++));	// f_nvarchar256		nvarchar(256)
		assertTrue(rsm.isSigned(columnIndex++));	// f_varchar128			varchar(128)
		assertTrue(rsm.isSigned(columnIndex++));	// f_ntext				ntext
		assertTrue(rsm.isSigned(columnIndex++));	// f_ntext_compressed	ntext hint heap 'compressed'
		assertTrue(rsm.isSigned(columnIndex++));	// f_fulltext			fulltext
		assertTrue(rsm.isSigned(columnIndex++));	// f_binary50			binary(50)
		assertTrue(rsm.isSigned(columnIndex++));	// f_blob				blob
		assertTrue(rsm.isSigned(columnIndex++));	// f_nclob				nclob
		assertTrue(rsm.isSigned(columnIndex++));	// af_int				int					array[no limit]
		assertTrue(rsm.isSigned(columnIndex++));	// af_bigint			bigint				array[no limit]
		assertTrue(rsm.isSigned(columnIndex++));	// af_decimal			decimal(10,5)		array[no limit]
		assertTrue(rsm.isSigned(columnIndex++));	// af_char8				char(8)				array[no limit]
		assertTrue(rsm.isSigned(columnIndex++));	// af_float				float				array[no limit]
		assertTrue(rsm.isSigned(columnIndex++));	// af_datetime			datetime			array[no limit]
		assertTrue(rsm.isSigned(columnIndex++));	// af_id				uniqueidentifier	array[no limit]
		assertTrue(rsm.isSigned(columnIndex++));	// af_image				image				array[no limit]
		assertTrue(rsm.isSigned(columnIndex++));	// af_language			language			array[no limit]
		assertTrue(rsm.isSigned(columnIndex++));	// af_nchar6			nchar(6)			array[no limit]
		assertTrue(rsm.isSigned(columnIndex++));	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertTrue(rsm.isSigned(columnIndex++));	// af_varchar128		varchar(128)		array[no limit]
		assertTrue(rsm.isSigned(columnIndex++));	// af_ntext				ntext				array[no limit]
		assertTrue(rsm.isSigned(columnIndex++));	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertTrue(rsm.isSigned(columnIndex++));	// af_fulltext			fulltext			array[no limit]
		assertTrue(rsm.isSigned(columnIndex++));	// af_binary50			binary(50)			array[no limit]
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_id, f_int1, af_int, f_nchar6, f_language from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertTrue(rsm.isSigned(columnIndex++));	// f_id
		assertTrue(rsm.isSigned(columnIndex++));	// f_int1
		assertTrue(rsm.isSigned(columnIndex++));	// af_int
		assertTrue(rsm.isSigned(columnIndex++));	// f_nchar6
		assertTrue(rsm.isSigned(columnIndex++));	// f_language
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isSigned(1));	// count(*)
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) as numtuple from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isSigned(1));	// count(*) as numtuple
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isSigned(1));	// ROWID
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID as rid from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isSigned(1));	// ROWID as rid
		rs.close();

		assertNotNull(rs = s.executeQuery("select max(f_int1) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rsm.isSigned(1));	// max(f_int1)
		rs.close();

		assertNotNull(rs = s.executeQuery("select max(f_int1) as maxnumber from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rsm.isSigned(1));	// max(f_int1) as maxnumber
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.getColumnDisplaySize() のテスト
	public void test_getColumnDisplaySize() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	columnIndex = 1;
		assertEquals(10,			rsm.getColumnDisplaySize(columnIndex++));	// f_int_not_null		int not null
		assertEquals(10, 			rsm.getColumnDisplaySize(columnIndex++));	// f_int1				int
		assertEquals(10, 			rsm.getColumnDisplaySize(columnIndex++));	// f_int2				int
		assertEquals(19,			rsm.getColumnDisplaySize(columnIndex++));	// f_bigint				bigint
		assertEquals(10,			rsm.getColumnDisplaySize(columnIndex++));	// f_decimal			decimal(10,5)
		assertEquals(8,				rsm.getColumnDisplaySize(columnIndex++));	// f_char8_not_null		char(8) not null
		assertEquals(8,				rsm.getColumnDisplaySize(columnIndex++));	// f_char8				char(8)
		assertEquals(22,			rsm.getColumnDisplaySize(columnIndex++));	// f_float				float
		assertEquals(23,			rsm.getColumnDisplaySize(columnIndex++));	// f_datetime			datetime
		assertEquals(36,			rsm.getColumnDisplaySize(columnIndex++));	// f_id					uniqueidentifier
		assertEquals(0x7FFFFFFF,	rsm.getColumnDisplaySize(columnIndex++));	// f_image				image
		assertEquals(8,				rsm.getColumnDisplaySize(columnIndex++));	// f_language			language
		assertEquals(6,				rsm.getColumnDisplaySize(columnIndex++));	// f_nchar6				nchar(6)
		assertEquals(256,			rsm.getColumnDisplaySize(columnIndex++));	// f_nvarchar256		nvarchar(256)
		assertEquals(128,			rsm.getColumnDisplaySize(columnIndex++));	// f_varchar128			varchar(128)
		assertEquals(0x7FFFFFFF,	rsm.getColumnDisplaySize(columnIndex++));	// f_ntext				ntext
		assertEquals(0x7FFFFFFF,	rsm.getColumnDisplaySize(columnIndex++));	// f_ntext_compressed	ntext hint heap 'compressed'
		assertEquals(0x7FFFFFFF,	rsm.getColumnDisplaySize(columnIndex++));	// f_fulltext			fulltext
		assertEquals(50,			rsm.getColumnDisplaySize(columnIndex++));	// f_binary50			binary(50)
		assertEquals(0x7FFFFFFF,	rsm.getColumnDisplaySize(columnIndex++));	// f_blob				blob
		assertEquals(0x7FFFFFFF,	rsm.getColumnDisplaySize(columnIndex++));	// f_nclob				nclob
		assertEquals(10,			rsm.getColumnDisplaySize(columnIndex++));	// af_int				int					array[no limit]
		assertEquals(19,			rsm.getColumnDisplaySize(columnIndex++));	// af_bigint			bigint				array[no limit]
		assertEquals(10,			rsm.getColumnDisplaySize(columnIndex++));	// af_decimal			decimal(10,5)		array[no limit]
		assertEquals(8,				rsm.getColumnDisplaySize(columnIndex++));	// af_char8				char(8)				array[no limit]
		assertEquals(22,			rsm.getColumnDisplaySize(columnIndex++));	// af_float				float				array[no limit]
		assertEquals(23,			rsm.getColumnDisplaySize(columnIndex++));	// af_datetime			datetime			array[no limit]
		assertEquals(36,			rsm.getColumnDisplaySize(columnIndex++));	// af_id				uniqueidentifier	array[no limit]
		assertEquals(0x7FFFFFFF,	rsm.getColumnDisplaySize(columnIndex++));	// af_image				image				array[no limit]
		assertEquals(8,				rsm.getColumnDisplaySize(columnIndex++));	// af_language			language			array[no limit]
		assertEquals(6,				rsm.getColumnDisplaySize(columnIndex++));	// af_nchar6			nchar(6)			array[no limit]
		assertEquals(256,			rsm.getColumnDisplaySize(columnIndex++));	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertEquals(128,			rsm.getColumnDisplaySize(columnIndex++));	// af_varchar128		varchar(128)		array[no limit]
		assertEquals(0x7FFFFFFF,	rsm.getColumnDisplaySize(columnIndex++));	// af_ntext				ntext				array[no limit]
		assertEquals(0x7FFFFFFF,	rsm.getColumnDisplaySize(columnIndex++));	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertEquals(0x7FFFFFFF,	rsm.getColumnDisplaySize(columnIndex++));	// af_fulltext			fulltext			array[no limit]
		assertEquals(50,			rsm.getColumnDisplaySize(columnIndex++));	// af_binary50			binary(50)			array[no limit]
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_int1, f_float, f_nchar6, f_nclob, f_id, f_language, af_int, af_char8, af_ntext from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals(10,			rsm.getColumnDisplaySize(columnIndex++));	// f_int1
		assertEquals(22,			rsm.getColumnDisplaySize(columnIndex++));	// f_float
		assertEquals(6,				rsm.getColumnDisplaySize(columnIndex++));	// f_nchar6
		assertEquals(0x7FFFFFFF,	rsm.getColumnDisplaySize(columnIndex++));	// f_nclob
		assertEquals(36,			rsm.getColumnDisplaySize(columnIndex++));	// f_id
		assertEquals(8,				rsm.getColumnDisplaySize(columnIndex++));	// f_language
		assertEquals(10,			rsm.getColumnDisplaySize(columnIndex++));	// af_int
		assertEquals(8,				rsm.getColumnDisplaySize(columnIndex++));	// af_char8
		assertEquals(0x7FFFFFFF,	rsm.getColumnDisplaySize(columnIndex++));	// af_ntext
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_int2 as xid, f_varchar128 as textdata, f_binary50 as binarydata from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals(10,	rsm.getColumnDisplaySize(columnIndex++));	// f_int2 as xid
		assertEquals(128,	rsm.getColumnDisplaySize(columnIndex++));	// f_varchar128 as textdata
		assertEquals(50,	rsm.getColumnDisplaySize(columnIndex++));	// f_binary50 as binarydata
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(10, rsm.getColumnDisplaySize(1));	// count(*)
		rs.close();

		assertNotNull(rs = s.executeQuery("select max(f_int2) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(10, rsm.getColumnDisplaySize(1));	// max(f_int2)
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(10, rsm.getColumnDisplaySize(1));	// ROWID
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.getColumnLabel() のテスト
	public void test_getColumnLabel() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 2;
		createAndInsertTestTable(c, numberOfTables);

		String	databaseName = "TEST";

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1, t2"));
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	columnIndex = 1;
		assertEquals("f_int_not_null",		rsm.getColumnLabel(columnIndex++));	// f_int_not_null		int not null
		assertEquals("f_int1",				rsm.getColumnLabel(columnIndex++));	// f_int1				int
		assertEquals("f_int2",				rsm.getColumnLabel(columnIndex++));	// f_int2				int
		assertEquals("f_bigint",			rsm.getColumnLabel(columnIndex++));	// f_bigint				bigint
		assertEquals("f_decimal",			rsm.getColumnLabel(columnIndex++));	// f_decimal			decimal(10,5)
		assertEquals("f_char8_not_null",	rsm.getColumnLabel(columnIndex++));	// f_char8_not_null		char(8) not null
		assertEquals("f_char8",				rsm.getColumnLabel(columnIndex++));	// f_char8				char(8)
		assertEquals("f_float",				rsm.getColumnLabel(columnIndex++));	// f_float				float
		assertEquals("f_datetime",			rsm.getColumnLabel(columnIndex++));	// f_datetime			datetime
		assertEquals("f_id",				rsm.getColumnLabel(columnIndex++));	// f_id					uniqueidentifier
		assertEquals("f_image",				rsm.getColumnLabel(columnIndex++));	// f_image				image
		assertEquals("f_language",			rsm.getColumnLabel(columnIndex++));	// f_language			language
		assertEquals("f_nchar6",			rsm.getColumnLabel(columnIndex++));	// f_nchar6				nchar(6)
		assertEquals("f_nvarchar256",		rsm.getColumnLabel(columnIndex++));	// f_nvarchar256		nvarchar(256)
		assertEquals("f_varchar128",		rsm.getColumnLabel(columnIndex++));	// f_varchar128			varchar(128)
		assertEquals("f_ntext",				rsm.getColumnLabel(columnIndex++));	// f_ntext				ntext
		assertEquals("f_ntext_compressed",	rsm.getColumnLabel(columnIndex++));	// f_ntext_compressed	ntext hint heap 'compressed'
		assertEquals("f_fulltext",			rsm.getColumnLabel(columnIndex++));	// f_fulltext			fulltext
		assertEquals("f_binary50",			rsm.getColumnLabel(columnIndex++));	// f_binary50			binary(50)
		assertEquals("f_blob",				rsm.getColumnLabel(columnIndex++));	// f_blob				blob
		assertEquals("f_nclob",				rsm.getColumnLabel(columnIndex++));	// f_nclob				nclob
		assertEquals("af_int",				rsm.getColumnLabel(columnIndex++));	// af_int				int					array[no limit]
		assertEquals("af_bigint",			rsm.getColumnLabel(columnIndex++));	// af_bigint			bigint				array[no limit]
		assertEquals("af_decimal",			rsm.getColumnLabel(columnIndex++));	// af_decimal			decimal(10,5)		array[no limit]
		assertEquals("af_char8",			rsm.getColumnLabel(columnIndex++));	// af_char8				char(8)				array[no limit]
		assertEquals("af_float",			rsm.getColumnLabel(columnIndex++));	// af_float				float				array[no limit]
		assertEquals("af_datetime",			rsm.getColumnLabel(columnIndex++));	// af_datetime			datetime			array[no limit]
		assertEquals("af_id",				rsm.getColumnLabel(columnIndex++));	// af_id				uniqueidentifier	array[no limit]
		assertEquals("af_image",			rsm.getColumnLabel(columnIndex++));	// af_image				image				array[no limit]
		assertEquals("af_language",			rsm.getColumnLabel(columnIndex++));	// af_language			language			array[no limit]
		assertEquals("af_nchar6",			rsm.getColumnLabel(columnIndex++));	// af_nchar6			nchar(6)			array[no limit]
		assertEquals("af_nvarchar256",		rsm.getColumnLabel(columnIndex++));	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertEquals("af_varchar128",		rsm.getColumnLabel(columnIndex++));	// af_varchar128		varchar(128)		array[no limit]
		assertEquals("af_ntext",			rsm.getColumnLabel(columnIndex++));	// af_ntext				ntext				array[no limit]
		assertEquals("af_ntext_compressed",	rsm.getColumnLabel(columnIndex++));	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertEquals("af_fulltext",			rsm.getColumnLabel(columnIndex++));	// af_fulltext			fulltext			array[no limit]
		assertEquals("af_binary50",			rsm.getColumnLabel(columnIndex++));	// af_binary50			binary(50)			array[no limit]
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_int1 as tkey, f_char8 as name, f_nchar6 as position, f_language as lang from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals("tkey",		rsm.getColumnLabel(columnIndex++));	// f_int1 as tkey
		assertEquals("name",		rsm.getColumnLabel(columnIndex++));	// f_char8 as name
		assertEquals("position",	rsm.getColumnLabel(columnIndex++));	// f_nchar6 as position
		assertEquals("lang",		rsm.getColumnLabel(columnIndex++));	// f_language as lang
		rs.close();

		assertNotNull(rs = s.executeQuery("select t2.f_image, t2.f_nchar6, t1.f_ntext, t1.af_float, t2.f_char8_not_null, t1.f_int1 from t1, t2"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals("f_image",				rsm.getColumnLabel(columnIndex++));	// t2.f_image
		assertEquals("f_nchar6",			rsm.getColumnLabel(columnIndex++));	// t2.f_nchar6
		assertEquals("f_ntext",				rsm.getColumnLabel(columnIndex++));	// t1.f_ntext
		assertEquals("af_float",			rsm.getColumnLabel(columnIndex++));	// t1.af_float
		assertEquals("f_char8_not_null",	rsm.getColumnLabel(columnIndex++));	// f5.f_char8_not_null
		assertEquals("f_int1",				rsm.getColumnLabel(columnIndex++));	// f1.f_int1
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(f_int1) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals("count(f_int1)", rsm.getColumnLabel(1));	// count(*)
		rs.close();

		assertNotNull(rs = s.executeQuery("select max(f_int2) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals("max(f_int2)", rsm.getColumnLabel(1));	// max(f_int2)
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID from t2"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals("ROWID", rsm.getColumnLabel(1));	// ROWID
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.getColumnName() のテスト
	public void test_getColumnName() throws Exception
	{
		//
		// Sydney では SQL 文にエイリアスが指定されていればエイリアスを、指定されていなければ列名を返す
		//

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	columnIndex = 1;
		assertEquals("f_int_not_null",		rsm.getColumnName(columnIndex++));	// f_int_not_null		int not null
		assertEquals("f_int1", 				rsm.getColumnName(columnIndex++));	// f_int1				int
		assertEquals("f_int2", 				rsm.getColumnName(columnIndex++));	// f_int2				int
		assertEquals("f_bigint",			rsm.getColumnName(columnIndex++));	// f_bigint				bigint
		assertEquals("f_decimal",			rsm.getColumnName(columnIndex++));	// f_decimal			decimal(10,5)
		assertEquals("f_char8_not_null",	rsm.getColumnName(columnIndex++));	// f_char8_not_null		char(8) not null
		assertEquals("f_char8",				rsm.getColumnName(columnIndex++));	// f_char8				char(8)
		assertEquals("f_float",				rsm.getColumnName(columnIndex++));	// f_float				float
		assertEquals("f_datetime",			rsm.getColumnName(columnIndex++));	// f_datetime			datetime
		assertEquals("f_id",				rsm.getColumnName(columnIndex++));	// f_id					uniqueidentifier
		assertEquals("f_image",				rsm.getColumnName(columnIndex++));	// f_image				image
		assertEquals("f_language",			rsm.getColumnName(columnIndex++));	// f_language			language
		assertEquals("f_nchar6",			rsm.getColumnName(columnIndex++));	// f_nchar6				nchar(6)
		assertEquals("f_nvarchar256",		rsm.getColumnName(columnIndex++));	// f_nvarchar256		nvarchar(256)
		assertEquals("f_varchar128",		rsm.getColumnName(columnIndex++));	// f_varchar128			varchar(128)
		assertEquals("f_ntext",				rsm.getColumnName(columnIndex++));	// f_ntext				ntext
		assertEquals("f_ntext_compressed",	rsm.getColumnName(columnIndex++));	// f_ntext_compressed	ntext hint heap 'compressed'
		assertEquals("f_fulltext",			rsm.getColumnName(columnIndex++));	// f_fulltext			fulltext
		assertEquals("f_binary50",			rsm.getColumnName(columnIndex++));	// f_binary50			binary(50)
		assertEquals("f_blob",				rsm.getColumnName(columnIndex++));	// f_blob				blob
		assertEquals("f_nclob",				rsm.getColumnName(columnIndex++));	// f_nclob				nclob
		assertEquals("af_int",				rsm.getColumnName(columnIndex++));	// af_int				int					array[no limit]
		assertEquals("af_bigint",			rsm.getColumnName(columnIndex++));	// af_bigint			bigint				array[no limit]
		assertEquals("af_decimal",			rsm.getColumnName(columnIndex++));	// af_decimal			decimal(10,5)		array[no limit]
		assertEquals("af_char8",			rsm.getColumnName(columnIndex++));	// af_char8				char(8)				array[no limit]
		assertEquals("af_float",			rsm.getColumnName(columnIndex++));	// af_float				float				array[no limit]
		assertEquals("af_datetime",			rsm.getColumnName(columnIndex++));	// af_datetime			datetime			array[no limit]
		assertEquals("af_id",				rsm.getColumnName(columnIndex++));	// af_id				uniqueidentifier	array[no limit]
		assertEquals("af_image",			rsm.getColumnName(columnIndex++));	// af_image				image				array[no limit]
		assertEquals("af_language",			rsm.getColumnName(columnIndex++));	// af_language			language			array[no limit]
		assertEquals("af_nchar6",			rsm.getColumnName(columnIndex++));	// af_nchar6			nchar(6)			array[no limit]
		assertEquals("af_nvarchar256",		rsm.getColumnName(columnIndex++));	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertEquals("af_varchar128",		rsm.getColumnName(columnIndex++));	// af_varchar128		varchar(128)		array[no limit]
		assertEquals("af_ntext",			rsm.getColumnName(columnIndex++));	// af_ntext				ntext				array[no limit]
		assertEquals("af_ntext_compressed",	rsm.getColumnName(columnIndex++));	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertEquals("af_fulltext",			rsm.getColumnName(columnIndex++));	// af_fulltext			fulltext			array[no limit]
		assertEquals("af_binary50",			rsm.getColumnName(columnIndex++));	// af_binary50			binary(50)			array[no limit]
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_int2, f_blob, af_id, f_nvarchar256 from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals("f_int2",			rsm.getColumnName(columnIndex++));	// f_int2
		assertEquals("f_blob",			rsm.getColumnName(columnIndex++));	// f_blob
		assertEquals("af_id",			rsm.getColumnName(columnIndex++));	// af_id
		assertEquals("f_nvarchar256",	rsm.getColumnName(columnIndex++));	// f_nvarchar256
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals("count(*)", rsm.getColumnName(1));	// count(*)
		rs.close();

		assertNotNull(rs = s.executeQuery("select max(f_int1) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals("max(f_int1)", rsm.getColumnName(1));	// max(f_int1)
		rs.close();

		assertNotNull(rs = s.executeQuery("select min(f_int2) as minv from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals("minv", rsm.getColumnName(1));	// min(f_int2) as minv
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_blob as binarydata, f_int1 as keydata, f_nchar6, f_id as xid from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals("binarydata",	rsm.getColumnName(columnIndex++));	// f_blob as binarydata
 		assertEquals("keydata",		rsm.getColumnName(columnIndex++));	// f_int1 as keydata
		assertEquals("f_nchar6",	rsm.getColumnName(columnIndex++));	// f_nchar
		assertEquals("xid",			rsm.getColumnName(columnIndex++));	// f_id as xid
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.getSchemaName() のテスト
	public void test_getSchemaName() throws Exception
	{
		//
		// Sydney では、データベース名が返る
		//

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 2;
		createAndInsertTestTable(c, numberOfTables);

		String	databaseName = "TEST";

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1, t2"));
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	columnIndex = 1;
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_int_not_null		int not null
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_int1				int
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_int2				int
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_bigint				bigint
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_decimal			decimal(10,5)
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_char8_not_null		char(8) not null
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_char8				char(8)
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_float				float
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_datetime			datetime
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_id					uniqueidentifier
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_image				image
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_language			language
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_nchar6				nchar(6)
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_nvarchar256		nvarchar(256)
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_varchar128			varchar(128)
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_ntext				ntext
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_ntext_compressed	ntext hint heap 'compressed'
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_fulltext			fulltext
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_binary50			binary(50)
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_blob				blob
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f_nclob				nclob
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// af_int				int					array[no limit]
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// af_bigint			bigint				array[no limit]
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// af_decimal			decimal(10,5)		array[no limit]
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// af_char8				char(8)				array[no limit]
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// af_float				float				array[no limit]
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// af_datetime			datetime			array[no limit]
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// af_id				uniqueidentifier	array[no limit]
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// af_image				image				array[no limit]
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// af_language			language			array[no limit]
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// af_nchar6			nchar(6)			array[no limit]
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// af_varchar128		varchar(128)		array[no limit]
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// af_ntext				ntext				array[no limit]
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// af_fulltext			fulltext			array[no limit]
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// af_binary50			binary(50)			array[no limit]
		rs.close();

		assertNotNull(rs = s.executeQuery("select t2.f_image, t2.f_nchar6, t1.f_ntext, t1.af_float, t2.f_char8_not_null, t1.f_int1 from t1, t2"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// t2.f_image
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// t2.f_nchar6
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// t1.f_ntext
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// t1.af_float
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f5.f_char8_not_null
		assertEquals(databaseName, rsm.getSchemaName(columnIndex++));	// f1.f_int1
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.getPrecision() のテスト
	public void test_getPrecision() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		String	databaseName = "TEST";

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	columnIndex = 1;
		assertEquals(10,	rsm.getPrecision(columnIndex++));	// f_int_not_null		int not null
		assertEquals(10,	rsm.getPrecision(columnIndex++));	// f_int1				int
		assertEquals(10,	rsm.getPrecision(columnIndex++));	// f_int2				int
		assertEquals(19,	rsm.getPrecision(columnIndex++));	// f_bigint				bigint
		assertEquals(10,	rsm.getPrecision(columnIndex++));	// f_decimal			decimal(10,5)
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_char8_not_null		char(8) not null
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_char8				char(8)
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_float				float
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_datetime			datetime
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_id					uniqueidentifier
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_image				image
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_language			language
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_nchar6				nchar(6)
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_nvarchar256		nvarchar(256)
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_varchar128			varchar(128)
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_ntext				ntext
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_ntext_compressed	ntext hint heap 'compressed'
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_fulltext			fulltext
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_binary50			binary(50)
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_blob				blob
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_nclob				nclob
		assertEquals(10,	rsm.getPrecision(columnIndex++));	// af_int				int					array[no limit]
		assertEquals(19,	rsm.getPrecision(columnIndex++));	// af_bigint			bigint				array[no limit]
		assertEquals(10,	rsm.getPrecision(columnIndex++));	// af_decimal			decimal(10,5)		array[no limit]
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// af_char8				char(8)				array[no limit]
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// af_float				float				array[no limit]
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// af_datetime			datetime			array[no limit]
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// af_id				uniqueidentifier	array[no limit]
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// af_image				image				array[no limit]
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// af_language			language			array[no limit]
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// af_nchar6			nchar(6)			array[no limit]
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// af_varchar128		varchar(128)		array[no limit]
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// af_ntext				ntext				array[no limit]
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// af_fulltext			fulltext			array[no limit]
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// af_binary50			binary(50)			array[no limit]
		rs.close();

		assertNotNull(rs = s.executeQuery("select af_nchar6, f_id, f_float, f_int2 from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// af_nchar6
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_id
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_float
		assertEquals(10,	rsm.getPrecision(columnIndex++));	// f_int2
		rs.close();

		assertNotNull(rs = s.executeQuery("select af_int as intarray, f_char8 as textvalue, f_int1 as pkey from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals(10,	rsm.getPrecision(columnIndex++));	// af_int as intarray
		assertEquals(0,		rsm.getPrecision(columnIndex++));	// f_char8 as textvalue
		assertEquals(10,	rsm.getPrecision(columnIndex++));	// f_int1 as pkey
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(f_int1) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(10, rsm.getPrecision(1));	// count(f_int1)
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(f_int1) as numvalue from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(10, rsm.getPrecision(1));	// count(f_int1) as numvalue
		rs.close();

		assertNotNull(rs = s.executeQuery("select min(f_int2) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(10, rsm.getPrecision(1));	// min(f_int2)
		rs.close();

		assertNotNull(rs = s.executeQuery("select min(f_int2) as minv from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(10, rsm.getPrecision(1));	// min(f_int2) as minv
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(10, rsm.getPrecision(1));	// ROWID
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID as rid from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(10, rsm.getPrecision(1));	// ROWID as rid

		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.getScale() のテスト
	public void test_getScale() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		String	databaseName = "TEST";

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	columnIndex = 1;
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_int_not_null		int not null
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_int1				int
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_int2				int
		assertEquals(0, rsm.getScale(columnIndex++));	// f_bigint				bigint
		assertEquals(5, rsm.getScale(columnIndex++));	// f_decimal			decimal(10,5)
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_char8_not_null		char(8) not null
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_char8				char(8)
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_float				float
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_datetime			datetime
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_id					uniqueidentifier
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_image				image
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_language			language
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_nchar6				nchar(6)
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_nvarchar256		nvarchar(256)
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_varchar128			varchar(128)
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_ntext				ntext
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_ntext_compressed	ntext hint heap 'compressed'
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_fulltext			fulltext
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_binary50			binary(50)
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_blob				blob
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_nclob				nclob
		assertEquals(0,	rsm.getScale(columnIndex++));	// af_int				int					array[no limit]
		assertEquals(0, rsm.getScale(columnIndex++));	// af_bigint			bigint				array[no limit]
		assertEquals(5, rsm.getScale(columnIndex++));	// af_decimal			decimal(10,5)		array[no limit]
		assertEquals(0,	rsm.getScale(columnIndex++));	// af_char8				char(8)				array[no limit]
		assertEquals(0,	rsm.getScale(columnIndex++));	// af_float				float				array[no limit]
		assertEquals(0,	rsm.getScale(columnIndex++));	// af_datetime			datetime			array[no limit]
		assertEquals(0,	rsm.getScale(columnIndex++));	// af_id				uniqueidentifier	array[no limit]
		assertEquals(0,	rsm.getScale(columnIndex++));	// af_image				image				array[no limit]
		assertEquals(0,	rsm.getScale(columnIndex++));	// af_language			language			array[no limit]
		assertEquals(0,	rsm.getScale(columnIndex++));	// af_nchar6			nchar(6)			array[no limit]
		assertEquals(0,	rsm.getScale(columnIndex++));	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertEquals(0,	rsm.getScale(columnIndex++));	// af_varchar128		varchar(128)		array[no limit]
		assertEquals(0,	rsm.getScale(columnIndex++));	// af_ntext				ntext				array[no limit]
		assertEquals(0,	rsm.getScale(columnIndex++));	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertEquals(0,	rsm.getScale(columnIndex++));	// af_fulltext			fulltext			array[no limit]
		assertEquals(0,	rsm.getScale(columnIndex++));	// af_binary50			binary(50)			array[no limit]
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_int_not_null, f_float, f_image, f_blob from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals(0,	rsm.getScale(columnIndex++));	// f_int_not_null
		assertEquals(0, rsm.getScale(columnIndex++));	// f_float
		assertEquals(0, rsm.getScale(columnIndex++));	// f_image
		assertEquals(0, rsm.getScale(columnIndex++));	// f_blob
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(0, rsm.getScale(1));	// count(*)
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(0, rsm.getScale(1));	// ROWID
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.getTableName() のテスト
	public void test_getTableName() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 3;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		assertTableName(rsm, "t1");
		rs.close();

		assertNotNull(rs = s.executeQuery("select * from t1, t2"));

		assertNotNull(rsm = rs.getMetaData());
			String[]	tableNames = {
					"t1", "t1","t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1", "t1",
					"t2", "t2","t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2", "t2" };
		assertTableName(rsm, tableNames);
		rs.close();

		assertNotNull(rs = s.executeQuery("select t2.f_int2, t1.f_id, t3.f_language from t1, t2, t3"));

		assertNotNull(rsm = rs.getMetaData());
			String[]	tableNames2 = { "t2", "t1", "t3" };
		assertTableName(rsm, tableNames2);
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.getCatalogName() のテスト
	public void test_getCatalogName() throws Exception
	{
		//
		// カタログをサポートしていないので常に "" が返るはず
		//

		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	columnIndex = 1;
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_int_not_null		int not null
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_int1				int
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_int2				int
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_bigint				bigint
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_decimal			decimal(10,5)
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_char8_not_null		char(8) not null
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_char8				char(8)
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_float				float
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_datetime			datetime
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_id					uniqueidentifier
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_image				image
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_language			language
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_nchar6				nchar(6)
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_nvarchar256		nvarchar(256)
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_varchar128			varchar(128)
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_ntext				ntext
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_ntext_compressed	ntext hint heap 'compressed'
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_fulltext			fulltext
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_binary50			binary(50)
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_blob				blob
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_nclob				nclob
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_int				int					array[no limit]
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_bigint			bigint				array[no limit]
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_decimal			decimal(10,5)		array[no limit]
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_char8				char(8)				array[no limit]
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_float				float				array[no limit]
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_datetime			datetime			array[no limit]
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_id				uniqueidentifier	array[no limit]
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_image				image				array[no limit]
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_language			language			array[no limit]
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_nchar6			nchar(6)			array[no limit]
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_varchar128		varchar(128)		array[no limit]
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_ntext				ntext				array[no limit]
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_fulltext			fulltext			array[no limit]
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_binary50			binary(50)			array[no limit]
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_int1, f_image, f_ntext_compressed, af_image, af_binary50, f_float from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_int1
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_image
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_ntext_compressed
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_image
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_binary50
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_float
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals("", rsm.getCatalogName(1));	// count(*)
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals("", rsm.getCatalogName(1));	// ROWID
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_ntext as singlecontents, af_ntext as multicontents from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals("", rsm.getCatalogName(columnIndex++));	// f_ntext as singlecontents
		assertEquals("", rsm.getCatalogName(columnIndex++));	// af_ntext as multicontents
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.getColumnType() のテスト
	public void test_getColumnType1() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));

		// next 前でもメタデータが得られるはず
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());

		assertTrue(rs.next());


		assertNotNull(rsm = rs.getMetaData());
		int	columnIndex = 1;
		assertEquals(Types.INTEGER,		rsm.getColumnType(columnIndex++));	// f_int_not_null		int not null
		assertEquals(Types.INTEGER, 	rsm.getColumnType(columnIndex++));	// f_int1				int
		assertEquals(Types.INTEGER, 	rsm.getColumnType(columnIndex++));	// f_int2				int
		assertEquals(Types.BIGINT,		rsm.getColumnType(columnIndex++));	// f_bigint				bigint
		assertEquals(Types.DECIMAL,		rsm.getColumnType(columnIndex++));	// f_decimal			decimal(10,5)
		assertEquals(Types.CHAR,		rsm.getColumnType(columnIndex++));	// f_char8_not_null		char(8) not null
		assertEquals(Types.CHAR,		rsm.getColumnType(columnIndex++));	// f_char8				char(8)
		assertEquals(Types.DOUBLE,		rsm.getColumnType(columnIndex++));	// f_float				float
		assertEquals(Types.TIMESTAMP,	rsm.getColumnType(columnIndex++));	// f_datetime			datetime
		assertEquals(Types.CHAR,		rsm.getColumnType(columnIndex++));	// f_id					uniqueidentifier
		assertEquals(Types.VARBINARY,	rsm.getColumnType(columnIndex++));	// f_image				image
		assertEquals(Types.OTHER,		rsm.getColumnType(columnIndex++));	// f_language			language
		assertEquals(Types.CHAR,		rsm.getColumnType(columnIndex++));	// f_nchar6				nchar(6)
		assertEquals(Types.VARCHAR,		rsm.getColumnType(columnIndex++));	// f_nvarchar256		nvarchar(256)
		assertEquals(Types.VARCHAR,		rsm.getColumnType(columnIndex++));	// f_varchar128			varchar(128)
		assertEquals(Types.VARCHAR,		rsm.getColumnType(columnIndex++));	// f_ntext				ntext
		assertEquals(Types.VARCHAR,		rsm.getColumnType(columnIndex++));	// f_ntext_compressed	ntext hint heap 'compressed'
		assertEquals(Types.VARCHAR,		rsm.getColumnType(columnIndex++));	// f_fulltext			fulltext
		assertEquals(Types.BINARY,		rsm.getColumnType(columnIndex++));	// f_binary50			binary(50)
		assertEquals(Types.BLOB,		rsm.getColumnType(columnIndex++));	// f_blob				blob
		assertEquals(Types.CLOB,		rsm.getColumnType(columnIndex++));	// f_nclob				nclob
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_int				int					array[no limit]
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_bigint			bigint				array[no limit]
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_decimal			decimal(10,5)		array[no limit]
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_char8				char(8)				array[no limit]
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_float				float				array[no limit]
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_datetime			datetime			array[no limit]
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_id				uniqueidentifier	array[no limit]
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_image				image				array[no limit]
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_language			language			array[no limit]
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_nchar6			nchar(6)			array[no limit]
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_varchar128		varchar(128)		array[no limit]
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_ntext				ntext				array[no limit]
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_fulltext			fulltext			array[no limit]
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_binary50			binary(50)			array[no limit]
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_int1, f_image, f_ntext_compressed, af_image, af_binary50, f_float from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals(Types.INTEGER,		rsm.getColumnType(columnIndex++));	// f_int1
		assertEquals(Types.VARBINARY,	rsm.getColumnType(columnIndex++));	// f_image
		assertEquals(Types.VARCHAR,		rsm.getColumnType(columnIndex++));	// f_ntext_compressed
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_image
		assertEquals(Types.ARRAY,		rsm.getColumnType(columnIndex++));	// af_binary50
		assertEquals(Types.DOUBLE,		rsm.getColumnType(columnIndex++));	// f_float
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(Types.INTEGER, rsm.getColumnType(1));	// count(*)
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals(Types.INTEGER, rsm.getColumnType(1));	// ROWID
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_ntext as singlecontents, af_ntext as multicontents from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals(Types.VARCHAR,	rsm.getColumnType(columnIndex++));	// f_ntext as singlecontents
		assertEquals(Types.ARRAY,	rsm.getColumnType(columnIndex++));	// af_ntext as multicontents
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.getColumnType() のテスト
	// 『count(*)のデータ型がNULLになる』の再現
	public void test_getColumnType2() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());
		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select count(*) from t1"));

		// next 前でもメタデータが得られるはず
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());

		assertTrue(rs.next());


		assertNotNull(rsm = rs.getMetaData());
		assertEquals(1, rsm.getColumnCount());
		assertEquals(Types.INTEGER,	rsm.getColumnType(1));	// count(*)
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.getColumnTypeName() のテスト
	public void test_getColumnTypeName() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		assertTrue(rs.next());
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	columnIndex = 1;
		assertEquals("int",				rsm.getColumnTypeName(columnIndex++));	// f_int_not_null		int not null
		assertEquals("int", 			rsm.getColumnTypeName(columnIndex++));	// f_int1				int
		assertEquals("int", 			rsm.getColumnTypeName(columnIndex++));	// f_int2				int
		assertEquals("bigint",			rsm.getColumnTypeName(columnIndex++));	// f_bigint				bigint
		assertEquals("decimal",			rsm.getColumnTypeName(columnIndex++));	// f_decimal			decimal(10,5)
		assertEquals("char",			rsm.getColumnTypeName(columnIndex++));	// f_char8_not_null		char(8) not null
		assertEquals("char",			rsm.getColumnTypeName(columnIndex++));	// f_char8				char(8)
		assertEquals("float",			rsm.getColumnTypeName(columnIndex++));	// f_float				float
		assertEquals("datetime",		rsm.getColumnTypeName(columnIndex++));	// f_datetime			datetime
		assertEquals("char",			rsm.getColumnTypeName(columnIndex++));	// f_id					uniqueidentifier
		assertEquals("varbinary",		rsm.getColumnTypeName(columnIndex++));	// f_image				image
		assertEquals("language",		rsm.getColumnTypeName(columnIndex++));	// f_language			language
		assertEquals("nchar",			rsm.getColumnTypeName(columnIndex++));	// f_nchar6				nchar(6)
		assertEquals("nvarchar",		rsm.getColumnTypeName(columnIndex++));	// f_nvarchar256		nvarchar(256)
		assertEquals("varchar",			rsm.getColumnTypeName(columnIndex++));	// f_varchar128			varchar(128)
		assertEquals("nvarchar",		rsm.getColumnTypeName(columnIndex++));	// f_ntext				ntext
		assertEquals("nvarchar",		rsm.getColumnTypeName(columnIndex++));	// f_ntext_compressed	ntext hint heap 'compressed'
		assertEquals("nvarchar",		rsm.getColumnTypeName(columnIndex++));	// f_fulltext			fulltext
		assertEquals("binary",			rsm.getColumnTypeName(columnIndex++));	// f_binary50			binary(50)
		assertEquals("blob",			rsm.getColumnTypeName(columnIndex++));	// f_blob				blob
		assertEquals("nclob",			rsm.getColumnTypeName(columnIndex++));	// f_nclob				nclob
		assertEquals("int array",		rsm.getColumnTypeName(columnIndex++));	// af_int				int					array[no limit]
		assertEquals("bigint array",	rsm.getColumnTypeName(columnIndex++));	// af_bigint			bigint				array[no limit]
		assertEquals("decimal array",	rsm.getColumnTypeName(columnIndex++));	// af_decimal			decimal(10,5)		array[no limit]
		assertEquals("char array",		rsm.getColumnTypeName(columnIndex++));	// af_char8				char(8)				array[no limit]
		assertEquals("float array",		rsm.getColumnTypeName(columnIndex++));	// af_float				float				array[no limit]
		assertEquals("datetime array",	rsm.getColumnTypeName(columnIndex++));	// af_datetime			datetime			array[no limit]
		assertEquals("char array",		rsm.getColumnTypeName(columnIndex++));	// af_id				uniqueidentifier	array[no limit]
		assertEquals("varbinary array",	rsm.getColumnTypeName(columnIndex++));	// af_image				image				array[no limit]
		assertEquals("language array",	rsm.getColumnTypeName(columnIndex++));	// af_language			language			array[no limit]
		assertEquals("nchar array",		rsm.getColumnTypeName(columnIndex++));	// af_nchar6			nchar(6)			array[no limit]
		assertEquals("nvarchar array",	rsm.getColumnTypeName(columnIndex++));	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertEquals("varchar array",	rsm.getColumnTypeName(columnIndex++));	// af_varchar128		varchar(128)		array[no limit]
		assertEquals("nvarchar array",	rsm.getColumnTypeName(columnIndex++));	// af_ntext				ntext				array[no limit]
		assertEquals("nvarchar array",	rsm.getColumnTypeName(columnIndex++));	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertEquals("nvarchar array",	rsm.getColumnTypeName(columnIndex++));	// af_fulltext			fulltext			array[no limit]
		assertEquals("binary array",	rsm.getColumnTypeName(columnIndex++));	// af_binary50			binary(50)			array[no limit]
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_char8, f_id, f_language, f_fulltext, af_binary50, af_int, f_nclob from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals("char",			rsm.getColumnTypeName(columnIndex++));	// f_char8
		assertEquals("char",			rsm.getColumnTypeName(columnIndex++));	// f_id
		assertEquals("language",		rsm.getColumnTypeName(columnIndex++));	// f_language
		assertEquals("nvarchar",		rsm.getColumnTypeName(columnIndex++));	// f_fulltext
		assertEquals("binary array",	rsm.getColumnTypeName(columnIndex++));	// af_binary50
		assertEquals("int array",		rsm.getColumnTypeName(columnIndex++));	// af_int
		assertEquals("nclob",			rsm.getColumnTypeName(columnIndex++));	// f_nclob
		rs.close();

		assertNotNull(rs = s.executeQuery("select max(f_int1) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals("int", rsm.getColumnTypeName(1));	// max(f_int1)
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(f_nchar6) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals("uint", rsm.getColumnTypeName(1));	// count(f_nchar6)
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals("uint", rsm.getColumnTypeName(1));	// ROWID
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_int1 as xid, f_nchar6, f_blob as binarydata from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals("int",		rsm.getColumnTypeName(columnIndex++));	// f_int1 as xid
		assertEquals("nchar",	rsm.getColumnTypeName(columnIndex++));	// f_nchar6
		assertEquals("blob",	rsm.getColumnTypeName(columnIndex++));	// f_blob as binarydata
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.isReadOnly() のテスト
	public void test_isReadOnly() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		assertTrue(rs.next());
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	columnIndex = 1;
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_int_not_null		int not null
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_int1				int
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_int2				int
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_bigint				bigint
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_decimal			decimal(10,5)
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_char8_not_null		char(8) not null
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_char8				char(8)
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_float				float
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_datetime			datetime
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_id					uniqueidentifier
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_image				image
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_language			language
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_nchar6				nchar(6)
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_nvarchar256		nvarchar(256)
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_varchar128			varchar(128)
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_ntext				ntext
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_ntext_compressed	ntext hint heap 'compressed'
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_fulltext			fulltext
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_binary50			binary(50)
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_blob				blob
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_nclob				nclob
		assertFalse(rsm.isReadOnly(columnIndex++));	// af_int				int					array[no limit]
		assertFalse(rsm.isReadOnly(columnIndex++));	// af_bigint			bigint				array[no limit]
		assertFalse(rsm.isReadOnly(columnIndex++));	// af_decimal			decimal(10,5)		array[no limit]
		assertFalse(rsm.isReadOnly(columnIndex++));	// af_char8				char(8)				array[no limit]
		assertFalse(rsm.isReadOnly(columnIndex++));	// af_float				float				array[no limit]
		assertFalse(rsm.isReadOnly(columnIndex++));	// af_datetime			datetime			array[no limit]
		assertFalse(rsm.isReadOnly(columnIndex++));	// af_id				uniqueidentifier	array[no limit]
		assertFalse(rsm.isReadOnly(columnIndex++));	// af_image				image				array[no limit]
		assertFalse(rsm.isReadOnly(columnIndex++));	// af_language			language			array[no limit]
		assertFalse(rsm.isReadOnly(columnIndex++));	// af_nchar6			nchar(6)			array[no limit]
		assertFalse(rsm.isReadOnly(columnIndex++));	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertFalse(rsm.isReadOnly(columnIndex++));	// af_varchar128		varchar(128)		array[no limit]
		assertFalse(rsm.isReadOnly(columnIndex++));	// af_ntext				ntext				array[no limit]
		assertFalse(rsm.isReadOnly(columnIndex++));	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertFalse(rsm.isReadOnly(columnIndex++));	// af_fulltext			fulltext			array[no limit]
		assertFalse(rsm.isReadOnly(columnIndex++));	// af_binary50			binary(50)			array[no limit]
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_id, f_float, f_int2, f_datetime from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_id
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_float
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_int2
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_datetime
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_image as imagedata, f_fulltext as textdata, f_int1 as tid from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_image as imagedata
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_fulltext as textdata
		assertFalse(rsm.isReadOnly(columnIndex++));	// f_int1 as tid
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rsm.isReadOnly(1));	// ROWID
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID as rid from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rsm.isReadOnly(1));	// ROWID as rid
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rsm.isReadOnly(1));	// count(*)
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) as tuplenum from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rsm.isReadOnly(1));	// count(*) as tuplenum
		rs.close();

		assertNotNull(rs = s.executeQuery("select max(f_int1) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rsm.isReadOnly(1));	// max(f_int1)
		rs.close();

		assertNotNull(rs = s.executeQuery("select max(f_int2) as maxv from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertTrue(rsm.isReadOnly(1));	// max(f_int2) as maxv
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.isWritable() のテスト
	public void test_isWritable() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		assertTrue(rs.next());
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	columnIndex = 1;
		assertTrue(rsm.isWritable(columnIndex++));	// f_int_not_null		int not null
		assertTrue(rsm.isWritable(columnIndex++));	// f_int1				int
		assertTrue(rsm.isWritable(columnIndex++));	// f_int2				int
		assertTrue(rsm.isWritable(columnIndex++));	// f_bigint				bigint
		assertTrue(rsm.isWritable(columnIndex++));	// f_decimal			decimal(10,5)
		assertTrue(rsm.isWritable(columnIndex++));	// f_char8_not_null		char(8) not null
		assertTrue(rsm.isWritable(columnIndex++));	// f_char8				char(8)
		assertTrue(rsm.isWritable(columnIndex++));	// f_float				float
		assertTrue(rsm.isWritable(columnIndex++));	// f_datetime			datetime
		assertTrue(rsm.isWritable(columnIndex++));	// f_id					uniqueidentifier
		assertTrue(rsm.isWritable(columnIndex++));	// f_image				image
		assertTrue(rsm.isWritable(columnIndex++));	// f_language			language
		assertTrue(rsm.isWritable(columnIndex++));	// f_nchar6				nchar(6)
		assertTrue(rsm.isWritable(columnIndex++));	// f_nvarchar256		nvarchar(256)
		assertTrue(rsm.isWritable(columnIndex++));	// f_varchar128			varchar(128)
		assertTrue(rsm.isWritable(columnIndex++));	// f_ntext				ntext
		assertTrue(rsm.isWritable(columnIndex++));	// f_ntext_compressed	ntext hint heap 'compressed'
		assertTrue(rsm.isWritable(columnIndex++));	// f_fulltext			fulltext
		assertTrue(rsm.isWritable(columnIndex++));	// f_binary50			binary(50)
		assertTrue(rsm.isWritable(columnIndex++));	// f_blob				blob
		assertTrue(rsm.isWritable(columnIndex++));	// f_nclob				nclob
		assertTrue(rsm.isWritable(columnIndex++));	// af_int				int					array[no limit]
		assertTrue(rsm.isWritable(columnIndex++));	// af_bigint			bigint				array[no limit]
		assertTrue(rsm.isWritable(columnIndex++));	// af_decimal			decimal(10,5)		array[no limit]
		assertTrue(rsm.isWritable(columnIndex++));	// af_char8				char(8)				array[no limit]
		assertTrue(rsm.isWritable(columnIndex++));	// af_float				float				array[no limit]
		assertTrue(rsm.isWritable(columnIndex++));	// af_datetime			datetime			array[no limit]
		assertTrue(rsm.isWritable(columnIndex++));	// af_id				uniqueidentifier	array[no limit]
		assertTrue(rsm.isWritable(columnIndex++));	// af_image				image				array[no limit]
		assertTrue(rsm.isWritable(columnIndex++));	// af_language			language			array[no limit]
		assertTrue(rsm.isWritable(columnIndex++));	// af_nchar6			nchar(6)			array[no limit]
		assertTrue(rsm.isWritable(columnIndex++));	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertTrue(rsm.isWritable(columnIndex++));	// af_varchar128		varchar(128)		array[no limit]
		assertTrue(rsm.isWritable(columnIndex++));	// af_ntext				ntext				array[no limit]
		assertTrue(rsm.isWritable(columnIndex++));	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertTrue(rsm.isWritable(columnIndex++));	// af_fulltext			fulltext			array[no limit]
		assertTrue(rsm.isWritable(columnIndex++));	// af_binary50			binary(50)			array[no limit]
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_float, f_image, f_fulltext, f_int1, af_nchar6 from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertTrue(rsm.isWritable(columnIndex++));	// f_float
		assertTrue(rsm.isWritable(columnIndex++));	// f_image
		assertTrue(rsm.isWritable(columnIndex++));	// f_fulltext
		assertTrue(rsm.isWritable(columnIndex++));	// f_int1
		assertTrue(rsm.isWritable(columnIndex++));	// af_nchar6
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isWritable(1));	// ROWID
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID as rid from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isWritable(1));	// ROWID as rid
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isWritable(1));	// count(*)
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) as numtuple from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isWritable(1));	// count(*) as numtuple
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.isDefinitelyWritable() のテスト
	public void test_isDefinitelyWritable() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		assertTrue(rs.next());
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	columnIndex = 1;
		assertFalse(rsm.isDefinitelyWritable(columnIndex++));	// f_int_not_null		int not null
		assertFalse(rsm.isDefinitelyWritable(columnIndex++));	// f_int1				int
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_int2				int
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_bigint				bigint
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_decimal			decimal(10,5)
		assertFalse(rsm.isDefinitelyWritable(columnIndex++));	// f_char8_not_null		char(8) not null
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_char8				char(8)
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_float				float
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_datetime			datetime
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_id					uniqueidentifier
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_image				image
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_language			language
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_nchar6				nchar(6)
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_nvarchar256		nvarchar(256)
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_varchar128			varchar(128)
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_ntext				ntext
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_ntext_compressed	ntext hint heap 'compressed'
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_fulltext			fulltext
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_binary50			binary(50)
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_blob				blob
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_nclob				nclob
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// af_int				int					array[no limit]
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// af_bigint			bigint				array[no limit]
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// af_decimal			decimal(10,5)		array[no limit]
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// af_char8				char(8)				array[no limit]
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// af_float				float				array[no limit]
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// af_datetime			datetime			array[no limit]
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// af_id				uniqueidentifier	array[no limit]
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// af_image				image				array[no limit]
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// af_language			language			array[no limit]
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// af_nchar6			nchar(6)			array[no limit]
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// af_varchar128		varchar(128)		array[no limit]
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// af_ntext				ntext				array[no limit]
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// af_fulltext			fulltext			array[no limit]
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// af_binary50			binary(50)			array[no limit]
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_int2, f_int1, f_int_not_null, f_id, f_char8, f_char8_not_null from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_int2
		assertFalse(rsm.isDefinitelyWritable(columnIndex++));	// f_int1			(primary key)
		assertFalse(rsm.isDefinitelyWritable(columnIndex++));	// f_int_not_null
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_id
		assertTrue(rsm.isDefinitelyWritable(columnIndex++));	// f_char8
		assertFalse(rsm.isDefinitelyWritable(columnIndex++));	// f_char8_not_null
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isDefinitelyWritable(1));
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(*) as tuplenum from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isDefinitelyWritable(1));
		rs.close();

		assertNotNull(rs = s.executeQuery("select max(f_int1) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isDefinitelyWritable(1));
		rs.close();

		assertNotNull(rs = s.executeQuery("select max(f_int1) as maxv from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isDefinitelyWritable(1));
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isDefinitelyWritable(1));
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID as rid from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertFalse(rsm.isDefinitelyWritable(1));
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

		c.close();
	}

	// ResultSetMetaData.getColumnClassName() のテスト
	public void test_getColumnClassName() throws Exception
	{
		Connection	c = null;
		assertNotNull(c = getConnection());

		// 下準備
		int	numberOfTables = 1;
		createAndInsertTestTable(c, numberOfTables);

		Statement	s = null;
		assertNotNull(s = c.createStatement());

		ResultSet	rs = null;
		assertNotNull(rs = s.executeQuery("select * from t1"));
		assertTrue(rs.next());
		java.sql.ResultSetMetaData	rsm = null;

		assertNotNull(rsm = rs.getMetaData());
		int	columnIndex = 1;
		assertEquals("java.lang.Integer",						rsm.getColumnClassName(columnIndex++));	// f_int_not_null		int not null
		assertEquals("java.lang.Integer", 						rsm.getColumnClassName(columnIndex++));	// f_int1				int
		assertEquals("java.lang.Integer", 						rsm.getColumnClassName(columnIndex++));	// f_int2				int
		assertEquals("java.lang.Long",							rsm.getColumnClassName(columnIndex++));	// f_bigint				bigint
		assertEquals("java.math.BigDecimal",				rsm.getColumnClassName(columnIndex++));	// f_decimal			decimal(10,5)
		assertEquals("java.lang.String",						rsm.getColumnClassName(columnIndex++));	// f_char8_not_null		char(8) not null
		assertEquals("java.lang.String",						rsm.getColumnClassName(columnIndex++));	// f_char8				char(8)
		assertEquals("java.lang.Double",						rsm.getColumnClassName(columnIndex++));	// f_float				float
		assertEquals("java.sql.Timestamp",						rsm.getColumnClassName(columnIndex++));	// f_datetime			datetime
		assertEquals("java.lang.String",						rsm.getColumnClassName(columnIndex++));	// f_id					uniqueidentifier
		assertEquals("[B",										rsm.getColumnClassName(columnIndex++));	// f_image				image
		assertEquals("jp.co.ricoh.doquedb.common.LanguageData",	rsm.getColumnClassName(columnIndex++));	// f_language			language
		assertEquals("java.lang.String",						rsm.getColumnClassName(columnIndex++));	// f_nchar6				nchar(6)
		assertEquals("java.lang.String",						rsm.getColumnClassName(columnIndex++));	// f_nvarchar256		nvarchar(256)
		assertEquals("java.lang.String",						rsm.getColumnClassName(columnIndex++));	// f_varchar128			varchar(128)
		assertEquals("java.lang.String",						rsm.getColumnClassName(columnIndex++));	// f_ntext				ntext
		assertEquals("java.lang.String",						rsm.getColumnClassName(columnIndex++));	// f_ntext_compressed	ntext hint heap 'compressed'
		assertEquals("java.lang.String",						rsm.getColumnClassName(columnIndex++));	// f_fulltext			fulltext
		assertEquals("[B",										rsm.getColumnClassName(columnIndex++));	// f_binary50			binary(50)
		assertEquals("[B",										rsm.getColumnClassName(columnIndex++));	// f_blob				blob
		assertEquals("java.lang.String",						rsm.getColumnClassName(columnIndex++));	// f_nclob				nclob
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_int				int					array[no limit]
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_bigint			bigint				array[no limit]
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_decimal			decimal(10,5)		array[no limit]
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_char8				char(8)				array[no limit]
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_float				float				array[no limit]
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_datetime			datetime			array[no limit]
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_id				uniqueidentifier	array[no limit]
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_image				image				array[no limit]
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_language			language			array[no limit]
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_nchar6			nchar(6)			array[no limit]
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_nvarchar256		nvarchar(256)		array[no limit]
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_varchar128		varchar(128)		array[no limit]
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_ntext				ntext				array[no limit]
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_ntext_compressed	ntext				array[no limit] hint heap 'compressed'
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_fulltext			fulltext			array[no limit]
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_binary50			binary(50)			array[no limit]
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_char8, f_id, f_language, f_fulltext, af_binary50, af_int, f_nclob from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals("java.lang.String",						rsm.getColumnClassName(columnIndex++));	// f_char8
		assertEquals("java.lang.String",						rsm.getColumnClassName(columnIndex++));	// f_id
		assertEquals("jp.co.ricoh.doquedb.common.LanguageData",	rsm.getColumnClassName(columnIndex++));	// f_language
		assertEquals("java.lang.String",						rsm.getColumnClassName(columnIndex++));	// f_fulltext
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_binary50
		assertEquals("java.sql.Array",							rsm.getColumnClassName(columnIndex++));	// af_int
		assertEquals("java.lang.String",						rsm.getColumnClassName(columnIndex++));	// f_nclob
		rs.close();

		assertNotNull(rs = s.executeQuery("select max(f_int1) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals("java.lang.Integer", rsm.getColumnClassName(1));	// max(f_int1)
		rs.close();

		assertNotNull(rs = s.executeQuery("select count(f_nchar6) from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals("java.lang.Integer", rsm.getColumnClassName(1));	// count(f_nchar6)
		rs.close();

		assertNotNull(rs = s.executeQuery("select ROWID from t1"));

		assertNotNull(rsm = rs.getMetaData());
		assertEquals("java.lang.Integer", rsm.getColumnClassName(1));	// ROWID
		rs.close();

		assertNotNull(rs = s.executeQuery("select f_int1 as xid, f_nchar6, f_blob as binarydata from t1"));

		assertNotNull(rsm = rs.getMetaData());
		columnIndex = 1;
		assertEquals("java.lang.Integer",	rsm.getColumnClassName(columnIndex++));	// f_int1 as xid
		assertEquals("java.lang.String",	rsm.getColumnClassName(columnIndex++));	// f_nchar6
		assertEquals("[B",					rsm.getColumnClassName(columnIndex++));	// f_blob as binarydata
		rs.close();

		s.close();

		// 後始末
		dropTestTable(c, numberOfTables);

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

	private void createTestTable(	Connection	c,
									int			numberOfTables) throws Exception
	{
		Statement	s = null;
		assertNotNull(s = c.createStatement());
		for (int tid = 1; tid <= numberOfTables; tid++) {

			String	query =
				"create table t" + tid + " (														" +
				"	f_int_not_null		int not null,												" +
				"	f_int1				int,														" +
				"	f_int2				int,														" +
				"	f_bigint			bigint,														" +
				"	f_decimal			decimal(10,5),												" +
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
				"	af_int				int					array[no limit],						" +
				"	af_bigint			bigint				array[no limit],						" +
				"	af_decimal			decimal(10,5)		array[no limit],						" +
				"	af_char8			char(8)				array[no limit],						" +
				"	af_float			float				array[50],								" +
				"	af_datetime			datetime			array[no limit],						" +
				"	af_id				uniqueidentifier	array[no limit],						" +
				"	af_image			image				array[no limit],						" +
				"	af_language			language			array[30],								" +
				"	af_nchar6			nchar(6)			array[no limit],						" +
				"	af_nvarchar256		nvarchar(256)		array[no limit],						" +
				"	af_varchar128		varchar(128)		array[no limit],						" +
				"	af_ntext			ntext				array[10],								" +
				"	af_ntext_compressed	ntext				array[no limit] hint heap 'compressed',	" +
				"	af_fulltext			fulltext			array[no limit],						" +
				"	af_binary50			binary(50)			array[no limit],						" +
				"	primary key(f_int1)																" +
				")																					";
			s.executeUpdate(query);
		}

		s.close();
	}

	private void createTestTable(Connection	c) throws Exception
	{
		createTestTable(c, 1);
	}

	private static final int	numberOfColumns = 37;

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
	private void createAndInsertTestTable(	Connection	c,
											int			numberOfTables) throws Exception
	{
		createTestTable(c, numberOfTables);

		for (int tid = 1; tid <= numberOfTables; tid++) {

			PreparedStatement	ps = null;
			String	query;
			query = "insert into t" + tid + " values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
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
	}

	private void createAndInsertTestTable(Connection	c) throws Exception
	{
		createAndInsertTestTable(c, 1);
	}

	private void dropTestTable(	Connection	c,
								int			numberOfTables) throws Exception
	{
		Statement	s = c.createStatement();
		for (int tid = 1; tid <= numberOfTables; tid++) s.executeUpdate("drop table t" + tid);
		s.close();
	}

	private void dropTestTable(Connection	c) throws Exception
	{
		dropTestTable(c, 1);
	}

	private void assertTableName(	java.sql.ResultSetMetaData	rsm,
									String						tableName) throws Exception
	{
		for (int i = 0; i < rsm.getColumnCount(); i++) {
			assertEquals(tableName, rsm.getTableName(i + 1));
		}
	}

	private void assertTableName(	java.sql.ResultSetMetaData	rsm,
									String[]					tableNames) throws Exception
	{
		int	cc = rsm.getColumnCount();
		assertEquals(tableNames.length, cc);
		for (int i = 0; i < cc; i++) {
			assertEquals(tableNames[i], rsm.getTableName(i + 1));
		}
	}

}

//
// Copyright (c) 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
