// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AuthorizeMode.java -- 認証方式
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

package jp.co.ricoh.doquedb.port;

/**
 * 認証方式
 *
 */
public final class AuthorizeMode
{
	/** なし */
	public final static int NONE			= 0;

	/** パスワード認証 */
	public final static int PASSWORD		= 0x01000000;

	/** Mask */
	/** マスターID取得用 */
	public final static int MaskMasterID	= 0x0000FFFF;
	/** 認証方式取得用 */
	public final static int MaskMode		= 0x0F000000;
}

//
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
