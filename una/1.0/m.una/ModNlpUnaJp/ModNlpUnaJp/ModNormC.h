/* -*-Mode: C++; tab-width: 4;-*-
 * vi:set ts=4 sw=4:    
 *
 *	ModNormC.h -- C language interface of ModNormalizer
 * 
 * Copyright (c) 2000-2005, 2023 Ricoh Company, Ltd.
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

#ifndef __ModNormC_H__
#define __ModNormC_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__STDC__) || defined(__cplusplus)
#define P_(arg_list) arg_list
#else
#define P_(arg_list) ()
#endif

UNA_UNAJP_FUNCTION void*	modNormInit P_((const char* dataDirPath, const int normalizeEnglish));
UNA_UNAJP_FUNCTION void*	modNormInit2 P_((const char* ruleDicPath, const char* ruleAppPath, const char* exDicPath, const char* exAppPath, const char* connPath, const char* unkTblPath, const char* unkCstPath, const char* nrmTblPath, const char* prePath, const char* postPath, const char* combiPath, const int normalizeEnglish));
UNA_UNAJP_FUNCTION void	modNormTerm P_((void* result_of_modNormInit));

UNA_UNAJP_FUNCTION void*	modNormCreateNormalizer P_((const void* result_of_modNormInit));
UNA_UNAJP_FUNCTION void	modNormDestroyNormalizer P_((void* result_of_modNormCreateNormalizer));

UNA_UNAJP_FUNCTION void*	modNormNormalize P_((void* normalizer, const char*  str));
UNA_UNAJP_FUNCTION void	modNormDestroyResult P_((void* result_of_modNormNormalize));
UNA_UNAJP_FUNCTION const char* modNormGetString P_((void* result_of_modNormNormalize));

UNA_UNAJP_FUNCTION void*	modNormExpand P_((void* normalizer, const char* string, int* sizep));
UNA_UNAJP_FUNCTION void	modNormDestroyExpanded P_((void* result_of_modNormExpand));
UNA_UNAJP_FUNCTION const char* modNormGetExpandedString P_((void* result_of_modNormExpand, int n));

#ifdef __cplusplus
}
#endif

#undef P_

#endif /* __ModNormC_H__ */

/*
 * Copyright (c) 2000-2005, 2023 RICOH Company, Ltd.
 * All rights reserved.
 */
