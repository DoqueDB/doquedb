// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AvailabilityTest.java -- 利用可能性のテスト
// 
// Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
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

public class AvailabilityTest extends TestBase
{
	public AvailabilityTest(String	nickname_)
	{
		super(nickname_);
	}

	public void test_isServerAvailable() throws Exception
	{
		jp.co.ricoh.doquedb.client.DataSource	dataSource =
			new jp.co.ricoh.doquedb.client.DataSource("localhost", 54321);
		dataSource.open(3);
		assertTrue(dataSource.isServerAvailable());
		dataSource.close();
	}

	public void test_isDatabaseAvailable() throws Exception
	{
		jp.co.ricoh.doquedb.client.DataSource	dataSource =
			new jp.co.ricoh.doquedb.client.DataSource("localhost", 54321);
		dataSource.open(3);
		assertTrue(dataSource.isDatabaseAvailable());
		dataSource.close();
	}
}

//
// Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
