/*
 * Makefile.c --- 
 * 
 * Copyright (c) 2023 Ricoh Company, Ltd.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if defined(SYD_ARCH64) && defined(SYD_OS_LINUX)
SUBDIRS = \
	SydServer \
	Sqli \
	UserAdd \
	TRBackup

/* [YET] add sqli without inprocess option */
CLIENTSUBDIRS = \
	Sqli \
	UserAdd \
	TRBackup
#else
SUBDIRS = \
	SydServer \
	Sqli \
	UserAdd

/* [YET] add sqli without inprocess option */
CLIENTSUBDIRS = \
	Sqli \
	UserAdd
#endif

/*
ANTDIRS = \
	SydAdmin
*/

utility: all-r
clientutility: clientall-r

/*
AntTarget(all, dist, $(ANTDIRS))
AntTarget(clean, clean, $(ANTDIRS))
*/

/* no makefile.h */
MAKEFILE_H =
