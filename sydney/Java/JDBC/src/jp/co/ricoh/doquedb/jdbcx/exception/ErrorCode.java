// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ErrorCode.java -- ErrorCodeクラス
//
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.jdbcx.exception;

import java.sql.SQLException;

import javax.transaction.xa.XAException;

/**
 * SQLExceptionのエラーコードをXAConnectionのエラーコードにマッピングさせ、
 * 該当するXAExceptionを生成しスローする。
 *
 */
public class ErrorCode {

	/**
	 * XAExceptionを生成しスローする。
	 *
	 * @param	ex_
	 * 			SQLException
	 * @throws	javax.transaction.xa.XAException
	 */
	public static void throwXAException(SQLException ex_) throws XAException {

		switch(ex_.getErrorCode()) {
		case jp.co.ricoh.doquedb.exception.ErrorCode.XA_DUPLICATE_IDENTIFIER:
			throw new XA_DuplicateIdentifier(ex_.getMessage());
		case jp.co.ricoh.doquedb.exception.ErrorCode.XA_HEUR_COMMIT:
			throw new XA_HeurCommit(ex_.getMessage());
		case jp.co.ricoh.doquedb.exception.ErrorCode.XA_HEUR_MIX:
			throw new XA_HeurMix(ex_.getMessage());
		case jp.co.ricoh.doquedb.exception.ErrorCode.XA_HEUR_ROLLBACK:
			throw new XA_HeurRollback(ex_.getMessage());
		case jp.co.ricoh.doquedb.exception.ErrorCode.XA_INSIDE_ACTIVE_BRANCH:
			throw new XA_InsideActiveBranch(ex_.getMessage());
		case jp.co.ricoh.doquedb.exception.ErrorCode.XA_INVALID_IDENTIFIER:
			throw new XA_InvalidIdentifier(ex_.getMessage());
		case jp.co.ricoh.doquedb.exception.ErrorCode.XA_PROTOCOL_ERROR:
			throw new XA_ProtocolError(ex_.getMessage());
		case jp.co.ricoh.doquedb.exception.ErrorCode.XA_UNKNOWN_IDENTIFIER:
			throw new XA_UnknownIdentifier(ex_.getMessage());
		default:
			// 該当するXAエラーがない場合
			XAException ex = new XAException(ex_.getMessage());
			ex.errorCode = XAException.XAER_RMERR;
			throw ex;
		}
	}
}

//
//Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//All rights reserved.
//
