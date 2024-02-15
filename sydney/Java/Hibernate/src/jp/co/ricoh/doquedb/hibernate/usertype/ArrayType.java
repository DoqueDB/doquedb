// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ArrayType.java -- ArrayType class for Hibernate3
// 
// Copyright (c) 2006, 2007, 2010, 2023 Ricoh Company, Ltd.
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

import org.hibernate.HibernateException;
import org.hibernate.engine.spi.SessionImplementor;


/**
 * Array class for Hibernate3 of Doquedb
 *
 * It can be regarded as a bridge between hibernate and JDBC driver.
 * User declears it in POJO and uses it solution instead of co.jp.ricoh.doquedb.jdbc.Array
 * Actually, jdbc.Array is take as members in this class and be used to gain trully data.
 *
 * @author Xie Xuansong
 */

public class ArrayType implements org.hibernate.usertype.UserType
{
 	// Field type
 	public static final int[] TYPES = new int[]{ Types.ARRAY };

 	/*
 	 *
 	 * Return the SQL type codes for the columns mapped by this type.
 	 * return type: java.sql.Types.ARRAY
 	 */
 	public int[] sqlTypes() {
  		return TYPES;
 	}

 	/*
	 * The class returned by <nullSafeGet()>.
	 * return class type
	 */
 	public Class returnedClass() {
  		return ArrayList.class;
 	}

	/*
  	* Compare two instances of the class mapped by this type for persistence "equality".
  	* Equality of the persistent state.
  	* equal function, should realize DataArrayData comparison, but it's private, so...
  	* @param	Object x : which type is jdbc.Array or DoquedbArray?
  	* 			Object y : as forward
  	*/
 	public boolean equals(Object x, Object y) throws HibernateException {
  		if (x == y) return true;
		if (x == null || y == null) return false;

		return x.equals(y);
 	}

 	/*
 	 * Get a hashcode for the instance, consistent with persistence "equality"
 	 * if exists hash code.
 	 */
 	public int hashCode(Object x) throws HibernateException {
		return x.hashCode();
 	}

 	/*
  	* Retrieve an instance of the mapped class from a JDBC resultset.
  	* Should handle possibility of null values.
  	* be called by hibernate, it's the main entrance of JOPO getter.
  	* for type matching, the return type should be jdbc.Array, not DoquedbArray.
  	* @param	ResultSet rs : input record set;
  	*			java.lang.String[] names: column names, normally has 1 column only.
  	*			java.lang.Object owner:  owner the containing entity
  	*/
 	@Override
 	public Object nullSafeGet(java.sql.ResultSet rs,
							  java.lang.String[] names,
							  SessionImplementor sessionImplementor,
							  java.lang.Object owner)
  		throws HibernateException, SQLException
	{
		Array a = rs.getArray(names[0]);
		if (rs.wasNull()) return null;

		Object[] o = (Object[])a.getArray();
		ArrayList arr = new ArrayList();
		for (int i = 0; i < o.length; ++i) {
			arr.add(o[i]);
		}
		return arr;
 	}

 	/*
  	* Write an instance of the mapped class to a prepared statement.
  	* Should handle possibility of null values.
  	* be called by hibernate, it's the main entrance of JOPO setter.
 	* @param PreparedStatement st: a JDBC prepared statement
	* 		 Object value: the object to write
	* 		 int index: statement parameter index
	*					A multi-column type should be written to parameters starting from <index>.
  	*/
 	@Override
 	public void nullSafeSet(java.sql.PreparedStatement st,
							java.lang.Object value,
							int index,
							SessionImplementor sessionImplementor)
  		throws HibernateException, SQLException
	{
		if (value == null) {
			st.setNull(index, sqlTypes()[0]);
		} else if (value instanceof ArrayList) {
			ArrayList a = (ArrayList)value;
			st.setArray(index,
						new jp.co.ricoh.doquedb.jdbc.Array(a.toArray()));
		} else {
			st.setObject(index, value);
		}
 	}

	/*
	 * Return a deep copy of the persistent state, stopping at entities and at
	 * collections. It is not necessary to copy immutable objects, or null
	 * values, in which case it is safe to simply return the argument.
	 * @param Object value : the object to be cloned, which may be null
	 * @return Object a copy
	 */
 	public Object deepCopy(Object value) throws HibernateException
	{
		if (value == null) return null;
		return ((ArrayList)value).clone();
 	}

	/**
	 * Are objects of this type mutable?
	 *
	 * @return boolean
	 */
	public boolean isMutable()
	{
		return true;
	}


	/*
	 * Transform the object into its cacheable representation. At the very least this
	 * method should perform a deep copy if the type is mutable. That may not be enough
	 * for some implementations, however; for example, associations must be cached as
	 * identifier values. (optional operation)
	 */
 	public Serializable disassemble(Object value) throws HibernateException
	{
		return (Serializable)deepCopy(value);
 	}

	/*
	 * Reconstruct an object from the cacheable representation. At the very least this
	 * method should perform a deep copy if the type is mutable. (optional operation)
	 *
	 * @param Serializable cached: the object to be cached
	 * 		Object owner: the owner of the cached object
	 * @return a reconstructed object from the cachable representation
	 */
 	public Object assemble(Serializable cached, Object owner)
  		throws HibernateException
	{
  		return deepCopy(cached);
 	}


	/*
	 * During merge, replace the existing (target) value in the entity we are merging to
	 * with a new (original) value from the detached entity we are merging. For immutable
	 * objects, or null values, it is safe to simply return the first parameter. For
	 * mutable objects, it is safe to return a copy of the first parameter. For objects
	 * with component values, it might make sense to recursively replace component values.
	 *
	 * @param Object original: the value from the detached entity being merged
	 * 			Object target: the value in the managed entity
	 * @return the value to be merged
	 */
 	public Object replace(Object original, Object target, Object owner)
  		throws HibernateException
	{
  		return deepCopy(original);
 	}
}

//
// Copyright (c) 2006, 2007, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
