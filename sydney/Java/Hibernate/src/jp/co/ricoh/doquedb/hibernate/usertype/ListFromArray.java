// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ListFromArray.java --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.hibernate.usertype;

import java.io.Serializable;
import java.sql.Array;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Types;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.hibernate.Hibernate;
import org.hibernate.HibernateException;
import org.hibernate.engine.spi.SessionImplementor;
import org.hibernate.type.DateType;
import org.hibernate.type.DoubleType;
import org.hibernate.type.IntegerType;
import org.hibernate.type.StringType;
import org.hibernate.type.Type;

/**
 * Hibernate type to store Lists of primitives using SQL ARRAY.
 *
 * References :
 * http://forum.hibernate.org/viewtopic.php?t=946973
 */
public abstract class ListFromArray<T> implements CollectionType
{
	private static final int SQL_TYPE = Types.ARRAY;
	private static final int[] SQL_TYPES = { SQL_TYPE };

	abstract protected Array getDataAsArray(Object value);

	protected List<T> getDataFromArray(Object array) {
		T[] dataarray = (T[]) array;
		ArrayList<T> result = new ArrayList<T>(dataarray.length);
		for (int i = 0; i < dataarray.length; ++i) {
			result.add(dataarray[i]);
		}
		return result;
	}

	/**
	 * int array[no limit]
	 */
	public static class INTEGER extends ListFromArray<Integer>
	{
		protected Array getDataAsArray(Object value) {
			return new SqlArray.INTEGER((List<Integer>)value);
		}

		public Type getElementType() {
			return new IntegerType();
		}
	}

	/**
	 * float array[no limit]
	 */
	public static class DOUBLE extends ListFromArray<Double>
	{
		protected Array getDataAsArray(Object value) {
			return new SqlArray.DOUBLE((List<Double>)value);
		}

		public Type getElementType() {
			return new DoubleType();
		}
	}

	/**
	 * ntext array[no limit] など
	 * language array[no limit] もこれ
	 */
	public static class STRING extends ListFromArray<String>
	{
		protected Array getDataAsArray(Object value) {
			return new SqlArray.STRING((List<String>)value);
		}

		public Type getElementType() {
			return new StringType();
		}
	}

	/**
	 * datetime array[no limit]
	 */
	public static class DATE extends ListFromArray<Date>
	{
		protected Array getDataAsArray(Object value) {
			return new SqlArray.DATE((List<Date>)value);
		}

		public Type getElementType() {
			return new DateType();
		}
	}

	public Class returnedClass() {
		return List.class;
	}

	public int[] sqlTypes() {
		return SQL_TYPES;
	}

    public Object deepCopy(Object value) {
		// メンバー変数を持っていないので、引数をそのまま返す
		return value;
    }

    public boolean isMutable() {
		return true;
    }

    @Override
    public Object nullSafeGet(ResultSet resultSet, String[] names, SessionImplementor sessionImplementor, Object owner)
        throws HibernateException, SQLException {

		Array sqlArray = resultSet.getArray(names[0]);
        if (resultSet.wasNull())
			return null;

        return getDataFromArray(sqlArray.getArray());
    }

    @Override
    public void nullSafeSet(PreparedStatement preparedStatement,
							Object value, int index, SessionImplementor sessionImplementor)
		throws HibernateException, SQLException {
        if (null == value)
            preparedStatement.setNull(index, SQL_TYPE);
        else
			preparedStatement.setArray(index, getDataAsArray(value));
    }

    public int hashCode(Object x) throws HibernateException {
        return x.hashCode();
    }

    public boolean equals(Object x, Object y) throws HibernateException {
        if (x == y)
            return true;
        if (null == x || null == y)
            return false;
        Class javaClass = returnedClass();
        if (!javaClass.equals(x.getClass()) || !javaClass.equals(y.getClass()))
			return false;

        return x.equals(y);
    }

    public Object assemble(Serializable cached, Object owner)
		throws HibernateException {
		return cached;
    }

    public Serializable disassemble(Object value) throws HibernateException {
        return (Serializable)value;
    }

    public Object replace(Object original, Object target, Object owner)
		throws HibernateException {
        return original;
    }
}

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
