// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SetFromArray.java --
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

import java.sql.Array;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.SQLFeatureNotSupportedException;
import java.sql.Timestamp;
import java.sql.Types;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.SortedMap;

/**
 * Hibernate type to store a java array using SQL ARRAY.
 *
 * References :
 * http://forum.hibernate.org/viewtopic.php?t=946973
 */
class SqlArray<T> implements Array
{
	private List<T> data;
	private int baseType;
	private String baseTypeName = null;

	protected SqlArray(int baseType, String baseTypeName) {
		this.data = null;
		this.baseType = baseType;
		this.baseTypeName = baseTypeName;
	}

	public static class INTEGER extends SqlArray<Integer>
	{
		public INTEGER(List<Integer> data) {
			super(Types.INTEGER, "int");
			setData(data);
		}
		public INTEGER(Set<Integer> data) {
			super(Types.INTEGER, "int");
			setData(new ArrayList<Integer>(data));
		}
	}

	public static class DOUBLE extends SqlArray<Double>
	{
		public DOUBLE(List<Double> data) {
			super(Types.DOUBLE, "float");
			setData(data);
		}
		public DOUBLE(Set<Double> data) {
			super(Types.DOUBLE, "float");
			setData(new ArrayList<Double>(data));
		}
	}

	public static class STRING extends SqlArray<String>
	{
		public STRING(List<String> data){
			super(Types.VARCHAR, "text");
			setData(data);
		}
		public STRING(Set<String> data){
			super(Types.VARCHAR, "text");
			setData(new ArrayList<String>(data));
		}
	}

	public static class DATE extends SqlArray<Timestamp>
	{
		public DATE(List<Date> data){
			super(Types.TIMESTAMP, "datetime");
			// java.util.Date -> java.sql.Timestamp
			List<Timestamp> n = new ArrayList<Timestamp>(data.size());
			for (Date d : data) {
				if (d == null)
					n.add(null);
				else
					n.add(new Timestamp(d.getTime()));
			}
			setData(n);
		}
		public DATE(Set<Date> data){
			super(Types.TIMESTAMP, "datetime");
			// java.util.Date -> java.sql.Timestamp
			List<Timestamp> n = new ArrayList<Timestamp>(data.size());
			for (Date d : data) {
				if (d == null)
					n.add(null);
				else
					n.add(new Timestamp(d.getTime()));
			}
			setData(n);
		}
	}

	public String getBaseTypeName() {
		return baseTypeName;
	}

	public int getBaseType() {
		return baseType;
	}

	public Object getArray() {
		return data.toArray();
	}

	public Object getArray(long index, int count) {
		int lastIndex = count - (int)index;
		if (lastIndex > data.size())
			lastIndex = data.size();

		return data.subList((int)(index-1), lastIndex).toArray();
	}

	public Object getArray(SortedMap<String, Class<?>> arg0) {
		throw new UnsupportedOperationException();
	}

	public Object getArray(long arg0, int arg1,
						   SortedMap<String, Class<?>> arg2) {
		throw new UnsupportedOperationException();
	}

	public ResultSet getResultSet() {
		throw new UnsupportedOperationException();
	}

	public ResultSet getResultSet(SortedMap<String, Class<?>> arg0) {
		throw new UnsupportedOperationException();
	}

	public ResultSet getResultSet(long index, int count) {
		throw new UnsupportedOperationException();
	}

	public ResultSet getResultSet(long arg0, int arg1,
								  SortedMap<String, Class<?>> arg2) {
		throw new UnsupportedOperationException();
	}

	public String toString() {
		StringBuilder result = new StringBuilder();
		result.append('{');
		boolean first = true;

		for (T t: data) {
			if (first)
				first = false;
			else
				result.append(',');

			if (t == null) {
				result.append("null");
				continue;
			}

			switch (baseType) {
            case Types.BIT:
            case Types.BOOLEAN:
				result.append(((Boolean)t).booleanValue() ? "true" : "false");
				break;

            case Types.INTEGER:
            case Types.FLOAT:
            case Types.DOUBLE:
            case Types.REAL:
            case Types.NUMERIC:
            case Types.DECIMAL:
				result.append(t);
                break;

            case Types.VARCHAR:
				String s = (String)t;
				// Escape the string
				result.append('\"');
				for (int p = 0; p < s.length(); ++p) {
					char ch = s.charAt(p);
					if (ch == '\0')
						throw new IllegalArgumentException(
							"Zero bytes may not occur in string parameters.");
					if (ch == '\\' || ch == '"')
						result.append('\\');
					result.append(ch);
				}
				result.append('\"');
				break;

            case Types.TIMESTAMP:
				Date d = (Date)t;
				result.append('\'');
				appendDate(result, d);
				result.append( d );
				result.append('\'');
				break;

            default:
				throw new UnsupportedOperationException(
					"Unsupported type " + baseType + " / " + getBaseTypeName());
			}
		}

		result.append('}');

		return result.toString();
	}

	protected void appendDate(StringBuilder sb, Date date) {
		SimpleDateFormat f = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
		sb.append(f.format(date));
		if (date instanceof Timestamp) {
			Timestamp t = (Timestamp)date;
			int m = t.getNanos() / 1000 / 1000;	// nanosecond -> millisecond
			sb.append(".");
			if (m < 100) sb.append("0");
			if (m < 10) sb.append("0");
			sb.append(m);
		}
	}

	public Object getArray(Map<String, Class<?>> arg0)
		throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	public Object getArray(long arg0, int arg1, Map<String, Class<?>> arg2)
		throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	public ResultSet getResultSet(Map<String, Class<?>> arg0)
		throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	public ResultSet getResultSet(long arg0, int arg1,
								  Map<String, Class<?>> arg2)
		throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	protected void setData(List<T> data) {
		this.data = data;
	}

	@Override
	public void free() throws SQLException {
		throw new SQLFeatureNotSupportedException();
	}
}

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
