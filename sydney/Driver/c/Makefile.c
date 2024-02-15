/*
 * Makefile.c --- 
 * 
 * Copyright (c) 1997, 2023 Ricoh Company, Ltd.
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

#ifdef SYD_CPU_SPARC
SUBDIRS = \
	FileCommon \
	Record \
	Record2 \
	Btree2 \
	Vector2 \
	Inverted \
	FullText \
	FullText2 \
	Lob \
	Bitmap \
	Array \
	KdTree
#else
SUBDIRS = \
	FileCommon \
	Record \
	Record2 \
	Btree \
	Btree2 \
	Vector \
	Vector2 \
	Inverted \
	FullText \
	FullText2 \
	Lob \
	Bitmap \
	Array \
	KdTree
#endif

#ifdef OS_RHLINUX6_0
.NOTPARALLEL:
#endif

#ifdef SYD_COVERAGE
driver: all-r install-r dll-r installdll-r
#else
driver: installh-r all-r install-r dll-r installdll-r
#endif

/* no makefile.h */
MAKEFILE_H =

/*
  Copyright 1997, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
