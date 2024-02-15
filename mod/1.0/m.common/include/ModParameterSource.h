// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModParameterSource.h -- パラメーターの取得先関連のクラス定義
// 
// Copyright (c) 2000, 2002, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModParameterSource_H__
#define __ModParameterSource_H__

class ModParameter;

//	CLASS
//	ModParameterSource -- パラメーターの取得先を表すクラス
//
//	NOTES

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModParameterSource
{
	friend class ModParameter;
	friend class ModParameter::Object;
	friend class ModParameter::Object::SourceInfo;
public:
	// コンストラクター
	ModParameterSource(const char* parentPath = 0,
					   const char* environmentName = 0,
					   const char* environmentValue = 0,
					   const char* systemEnvironmentName = 0,
					   const char* systemEnvironmentValue = 0);
	// デストラクター
	~ModParameterSource()
	{}

private:
	const char*				_parentPath;
	const char*				_environmentName;
	const char*				_environmentValue;
	const char*				_systemEnvironmentName;
	const char*				_systemEnvironmentValue;
};

//	FUNCTION public
//	ModParameterSource::ModParameterSource --
//		パラメーターの取得先を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		char*				parentPath
//			0 以外の値
//				システムパラメーターファイルまたはデフォルトの
//				パラメーターファイルのパス名またはキー名を構成する
//				親ディレクトリー名または親キー名
//			0 または指定されないとき
//				ModParameter::defaultParentPath が指定されたものとみなす
//		char*				environmentName
//			0 以外の値
//				environmentValue に加えて、
//				パラメーターファイルのパス名またはキー名を得るための
//				環境変数の名前を格納する領域の先頭アドレス
//			0 または指定されないとき
//				ModParameter::defaultEnvironmentName が指定されたものとみなす
//		char*				environmentValue
//			0 以外の値
//				パラメーターファイルの
//				パス名またはキー名を格納する領域の先頭アドレス
//			0 または指定されないとき
//				ModParameter::defaultEnvironmentValue のみが
//				指定されたものとみなす
//		char*				systemEnvironmentName
//			0 以外の値
//				systemEnvironmentValue に代えて、
//				システムパラメーターファイルのパス名またはキー名を得るための
//				環境変数の名前を格納する領域の先頭アドレス
//			0 または指定されないとき
//				ModParameter::systemEnvironmentName が指定されたものとみなす
//		char*				systemEnvironmentValue
//			0 以外の値
//				システムパラメーターファイルの
//				パス名またはキー名を格納する領域の先頭アドレス
//			0 または指定されないとき
//				ModParameter::systemEnvironmentValue が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
ModParameterSource::ModParameterSource(const char* parentPath,
									   const char* environmentName,
									   const char* environmentValue,
									   const char* systemEnvironmentName,
									   const char* systemEnvironmentValue)
	: _parentPath(parentPath),
	  _environmentName(environmentName),
	  _environmentValue(environmentValue),
	  _systemEnvironmentName(systemEnvironmentName),
	  _systemEnvironmentValue(systemEnvironmentValue)
{ }

#endif	// __ModParameterSource_H__

//
// Copyright (c) 2000, 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
