// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNlpResourceUnaJp.cpp --
// 
// Copyright (c) 2005, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#include "ModLanguageSet.h"
#include "ModFile.h"
#include "UnaParam.h"
#include "UNA_UNIFY_TAG.h"
#include "ModNlpUnaJp/Module.h"
#include "LibUna/DicSet.h"
#include "ModNlpUnaJp/ModNlpResourceUnaJp.h"

_UNA_USING
_UNA_UNAJP_USING
	
namespace
{
	// UNA品詞番号からUNA統合品詞番号に変換するテーブル
	const int convTable[] =
	{
		 UNA_OTHER							//	0	UNA_HIN_NOTHING
		,UNA_NOUN|UNA_UNKNOWN				//  1	UNA_HIN_UNKNOWN_KATAKANA
		,UNA_SYMBOL							//  2	UNA_HIN_KUTEN
		,UNA_SYMBOL							//  3	UNA_HIN_TOUTEN
		,UNA_NUMBER							//  4	UNA_HIN_SUUSHI
		,UNA_NOUN							//  5	UNA_HIN_MEISHI_IPPAN
		,UNA_NOUN|UNA_PROPER				//  6	UNA_HIN_MEISHI_KOYUU
		,UNA_NOUN|UNA_PRO					//  7	UNA_HIN_MEISHI_DAI
		,UNA_NOUN|UNA_VERB					//  8	UNA_HIN_MEISHI_SAHEN
		,UNA_OTHER							//  9	UNA_HIN_JOSUU
		,UNA_QUALIFIER						//  10	UNA_HIN_KEIDOU_KANGO
		,UNA_QUALIFIER						//  11	UNA_HIN_RENTAI
		,UNA_QUALIFIER|UNA_ADV				//  12	UNA_HIN_FUKUSHI
		,UNA_CONJ							//  13	UNA_HIN_SETSUZOKU
		,UNA_FILLER							//  14	UNA_HIN_KANDOU
		,UNA_PFX							//  15	UNA_HIN_SETTOUTSUJI_IPPAN
		,UNA_SFX							//  16	UNA_HIN_SETSUBIJI_IPPAN
		,UNA_SYMBOL							//  17	UNA_HIN_KIGOU_IPPAN
		,UNA_VERB|UNA_QUALIFIER				//  18	UNA_HIN_DOUSHI_5_SHUU_KA
		,UNA_VERB							//  19	UNA_HIN_DOUSHI_5_I_KA
		,UNA_VERB							//  20	UNA_HIN_DOUSHI_5_MI_KA
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  21	UNA_HIN_DOUSHI_5_YOU_KA
		,UNA_VERB							//  22	UNA_HIN_DOUSHI_5_KARI_KA
		,UNA_VERB							//  23	UNA_HIN_DOUSHI_5_U_KA
		,UNA_VERB|UNA_QUALIFIER				//  24	UNA_HIN_DOUSHI_5_SHUU_GA
		,UNA_VERB							//  25	UNA_HIN_DOUSHI_5_I_GA
		,UNA_VERB							//  26	UNA_HIN_DOUSHI_5_MI_GA
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  27	UNA_HIN_DOUSHI_5_YOU_GA
		,UNA_VERB							//  28	UNA_HIN_DOUSHI_5_KA_GA
		,UNA_VERB							//  29	UNA_HIN_DOUSHI_5_U_GA
		,UNA_VERB|UNA_QUALIFIER				//  30	UNA_HIN_DOUSHI_5_SHUU_SA
		,UNA_VERB							//  31	UNA_HIN_DOUSHI_5_MI_SA
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  32	UNA_HIN_DOUSHI_5_YOU_SA
		,UNA_VERB							//  33	UNA_HIN_DOUSHI_5_KA_SA
		,UNA_VERB							//  34	UNA_HIN_DOUSHI_5_U_SA
		,UNA_VERB|UNA_QUALIFIER				//  35	UNA_HIN_DOUSHI_5_SHUU_TA
		,UNA_VERB							//  36	UNA_HIN_DOUSHI_5_TSU_TA
		,UNA_VERB							//  37	UNA_HIN_DOUSHI_5_MI_TA
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  38	UNA_HIN_DOUSHI_5_YOU_TA
		,UNA_VERB							//  39	UNA_HIN_DOUSHI_5_KA_TA
		,UNA_VERB							//  40	UNA_HIN_DOUSHI_5_U_TA
		,UNA_VERB|UNA_QUALIFIER				//  41	UNA_HIN_DOUSHI_5_SHUU_NA
		,UNA_VERB							//  42	UNA_HIN_DOUSHI_5_MI_NA
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  43	UNA_HIN_DOUSHI_5_YOU_NA
		,UNA_VERB							//  44	UNA_HIN_DOUSHI_5_KA_NA
		,UNA_VERB							//  45	UNA_HIN_DOUSHI_5_U_NA
		,UNA_VERB							//  46	UNA_HIN_DOUSHI_5_N_NA
		,UNA_VERB|UNA_QUALIFIER				//  47	UNA_HIN_DOUSHI_5_SHUU_BA
		,UNA_VERB							//  48	UNA_HIN_DOUSHI_5_MI_BA
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  49	UNA_HIN_DOUSHI_5_YOU_BA
		,UNA_VERB							//  50	UNA_HIN_DOUSHI_5_KA_BA
		,UNA_VERB							//  51	UNA_HIN_DOUSHI_5_U_BA
		,UNA_VERB							//  52	UNA_HIN_DOUSHI_5_N_BA
		,UNA_VERB|UNA_QUALIFIER				//  53	UNA_HIN_DOUSHI_5_SHUU_MA
		,UNA_VERB							//  54	UNA_HIN_DOUSHI_5_MI_MA
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  55	UNA_HIN_DOUSHI_5_YOU_MA
		,UNA_VERB							//  56	UNA_HIN_DOUSHI_5_KA_MA
		,UNA_VERB							//  57	UNA_HIN_DOUSHI_5_U_MA
		,UNA_VERB							//  58	UNA_HIN_DOUSHI_5_N_MA
		,UNA_VERB|UNA_QUALIFIER				//  59	UNA_HIN_DOUSHI_5_SHUU_RA
		,UNA_VERB							//  60	UNA_HIN_DOUSHI_5_TSU_RA
		,UNA_VERB							//  61	UNA_HIN_DOUSHI_5_MI_RA
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  62	UNA_HIN_DOUSHI_5_YOU_RA
		,UNA_VERB							//  63	UNA_HIN_DOUSHI_5_KA_RA
		,UNA_VERB							//  64	UNA_HIN_DOUSHI_5_KA_RA
		,UNA_VERB|UNA_QUALIFIER				//  65	UNA_HIN_DOUSHI_5_SHUU_WA
		,UNA_VERB							//  66	UNA_HIN_DOUSHI_5_TSU_WA
		,UNA_VERB							//  67	UNA_HIN_DOUSHI_5_MI_WA
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  68	UNA_HIN_DOUSHI_5_YOU_WA
		,UNA_VERB							//  69	UNA_HIN_DOUSHI_5_KA_WA
		,UNA_VERB							//  70	UNA_HIN_DOUSHI_5_U_WA
		,UNA_VERB|UNA_QUALIFIER				//  71	UNA_HIN_DOUSHI_1_SHUU
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  72	UNA_HIN_DOUSHI_1_YOU
		,UNA_VERB							//  73	UNA_HIN_DOUSHI_1_KA
		,UNA_VERB							//  74	UNA_HIN_DOUSHI_1_MEI
		,UNA_VERB							//  75	UNA_HIN_DOUSHI_1_MEIYO
		,UNA_QUALIFIER|UNA_A				//  76	UNA_HIN_KEIYOU_SHUU
		,UNA_QUALIFIER|UNA_A				//  77	UNA_HIN_KEIYOU_GOKAN
		,UNA_QUALIFIER|UNA_A				//  78	UNA_HIN_KEIYOU_TSU
		,UNA_QUALIFIER|UNA_A				//  79	UNA_HIN_KEIYOU_MI
		,UNA_QUALIFIER|UNA_A				//  80	UNA_HIN_KEIYOU_YOU
		,UNA_QUALIFIER|UNA_A				//  81	UNA_HIN_KEIYOU_KA
		,UNA_QUALIFIER|UNA_A				//  82	UNA_HIN_KEIYOU_U
		,UNA_QUALIFIER|UNA_A				//  83	UNA_HIN_KEIYOU_KI
		,UNA_QUALIFIER|UNA_A				//  84	UNA_HIN_KEIYOU_MEI
		,UNA_NOUN|UNA_VERB|UNA_UNKNOWN		//  85	UNA_HIN_UNKNOWN_MEISHI_SAHEN
		,UNA_NOUN|UNA_PROPER|UNA_UNKNOWN	//  86	UNA_HIN_UNKNOWN_MEISHI_KOYUU
		,UNA_SYMBOL|UNA_UNKNOWN				//  87	UNA_HIN_UNKNOWN_KIGOU
		,UNA_SYMBOL|UNA_UNKNOWN				//  88	UNA_HIN_UNKNOWN_KIGOU_ALPHABET
		,UNA_UNKNOWN						//  89	UNA_HIN_UNKNOWN_KATSUYOUGO
		,UNA_UNKNOWN						//  90	UNA_HIN_UNKNOWN_IPPAN
		,UNA_UNKNOWN						//  91	UNA_HIN_USER_DEFINED_1
		,UNA_UNKNOWN						//  92	UNA_HIN_USER_DEFINED_2
		,UNA_UNKNOWN						//  93	UNA_HIN_USER_DEFINED_3
		,UNA_UNKNOWN						//  94	UNA_HIN_USER_DEFINED_4
		,UNA_UNKNOWN						//  95	UNA_HIN_USER_DEFINED_5
		,UNA_UNKNOWN						//  96	UNA_HIN_USER_DEFINED_6
		,UNA_UNKNOWN						//  97	UNA_HIN_USER_DEFINED_7
		,UNA_UNKNOWN						//  98	UNA_HIN_USER_DEFINED_8
		,UNA_OTHER							//  99
		,UNA_OTHER							//  100
		,UNA_OTHER							//  101
		,UNA_OTHER							//  102
		,UNA_OTHER							//  103
		,UNA_OTHER							//  104
		,UNA_OTHER							//  105
		,UNA_OTHER							//  106
		,UNA_OTHER							//  107
		,UNA_OTHER							//  108
		,UNA_OTHER							//  109
		,UNA_OTHER							//  110
		,UNA_OTHER							//  111
		,UNA_OTHER							//  112
		,UNA_OTHER							//  113
		,UNA_OTHER							//  114
		,UNA_OTHER							//  115
		,UNA_OTHER							//  116
		,UNA_OTHER							//  117
		,UNA_OTHER							//  118
		,UNA_OTHER							//  119
		,UNA_OTHER							//  120
		,UNA_OTHER							//  121
		,UNA_OTHER							//  122
		,UNA_OTHER							//  123
		,UNA_OTHER							//  124
		,UNA_OTHER							//  125
		,UNA_OTHER							//  126
		,UNA_OTHER							//  127
		,UNA_OTHER							//  128
		,UNA_OTHER							//  129
		,UNA_OTHER							//  130
		,UNA_OTHER							//  131
		,UNA_OTHER							//  132
		,UNA_ADV							//  133
		,UNA_ADV							//  134
		,UNA_ADV							//  135
		,UNA_ADV							//  136
		,UNA_ADV							//  137
		,UNA_ADV							//  138
		,UNA_ADV							//  139
		,UNA_ADV							//  140
		,UNA_ADV							//  141
		,UNA_ADV							//  142
		,UNA_ADV							//  143
		,UNA_ADV							//  144
		,UNA_ADV							//  145
		,UNA_ADV							//  146
		,UNA_ADV							//  147
		,UNA_ADV							//  148
		,UNA_ADV							//  149
		,UNA_ADV							//  150
		,UNA_ADV							//  151
		,UNA_ADV							//  152
		,UNA_ADV							//  153
		,UNA_ADV							//  154
		,UNA_ADV							//  155
		,UNA_ADV							//  156
		,UNA_ADV							//  157
		,UNA_ADV							//  158
		,UNA_ADV							//  159
		,UNA_ADV							//  160
		,UNA_ADV							//  161
		,UNA_ADV							//  162
		,UNA_ADV							//  163
		,UNA_ADV							//  164
		,UNA_ADV							//  165
		,UNA_ADV							//  166
		,UNA_ADV							//  167
		,UNA_ADV							//  168
		,UNA_ADV							//  169
		,UNA_ADV							//  170
		,UNA_ADV							//  171
		,UNA_ADV							//  172
		,UNA_ADV							//  173
		,UNA_ADV							//  174
		,UNA_ADV							//  175
		,UNA_ADV							//  176
		,UNA_ADV							//  177
		,UNA_ADV							//  178
		,UNA_ADV							//  179
		,UNA_ADV							//  180
		,UNA_ADV							//  181
		,UNA_ADV							//  182
		,UNA_ADV							//  183
		,UNA_ADV							//  184
		,UNA_ADV							//  185
		,UNA_ADV							//  186
		,UNA_ADV							//  187
		,UNA_ADV							//  188
		,UNA_ADV							//  189
		,UNA_ADV							//  190
		,UNA_ADV							//  191
		,UNA_ADV							//  192
		,UNA_ADV							//  193
		,UNA_ADV							//  194
		,UNA_ADV							//  195
		,UNA_ADV							//  196
		,UNA_OTHER							//  197
		,UNA_OTHER							//  198
		,UNA_OTHER							//  199
		,UNA_OTHER							//  200
		,UNA_OTHER							//  201
		,UNA_OTHER							//  202
		,UNA_OTHER							//  203
		,UNA_OTHER							//  204
		,UNA_OTHER							//  205
		,UNA_OTHER							//  206
		,UNA_OTHER							//  207
		,UNA_OTHER							//  208
		,UNA_OTHER							//  209
		,UNA_OTHER							//  210
		,UNA_OTHER							//  211
		,UNA_OTHER							//  212
		,UNA_OTHER							//  213
		,UNA_OTHER							//  214
		,UNA_OTHER							//  215
		,UNA_OTHER							//  216
		,UNA_OTHER							//  217
		,UNA_OTHER							//  218
		,UNA_OTHER							//  219
		,UNA_OTHER							//  220
		,UNA_OTHER							//  221
		,UNA_OTHER							//  222
		,UNA_OTHER							//  223
		,UNA_OTHER							//  224
		,UNA_OTHER							//  225
		,UNA_OTHER							//  226
		,UNA_OTHER							//  227
		,UNA_OTHER							//  228
		,UNA_OTHER							//  229
		,UNA_OTHER							//  230
		,UNA_OTHER							//  231
		,UNA_OTHER							//  232
		,UNA_OTHER							//  233
		,UNA_OTHER							//  234
		,UNA_OTHER							//  235
		,UNA_OTHER							//  236
		,UNA_OTHER							//  237
		,UNA_OTHER							//  238
		,UNA_OTHER							//  239
		,UNA_OTHER							//  240
		,UNA_OTHER							//  241
		,UNA_OTHER							//  242
		,UNA_OTHER							//  243
		,UNA_OTHER							//  244
		,UNA_OTHER							//  245
		,UNA_OTHER							//  246
		,UNA_OTHER							//  247
		,UNA_OTHER							//  248
		,UNA_OTHER							//  249
		,UNA_OTHER							//  250
		,UNA_OTHER							//  251
		,UNA_OTHER							//  252
		,UNA_OTHER							//  253
		,UNA_OTHER							//  254
		,UNA_OTHER							//  255
		,UNA_OTHER							//  256
		,UNA_OTHER							//  257
		,UNA_OTHER							//  258
		,UNA_OTHER							//  259
		,UNA_OTHER							//  260
		,UNA_OTHER							//  261
		,UNA_OTHER							//  262
		,UNA_OTHER							//  263
		,UNA_OTHER							//  264
		,UNA_OTHER							//  265
		,UNA_OTHER							//  266
		,UNA_OTHER							//  267
		,UNA_OTHER							//  268
		,UNA_OTHER							//  269
		,UNA_OTHER							//  270
		,UNA_OTHER							//  271
		,UNA_OTHER							//  272
		,UNA_OTHER							//  273
		,UNA_OTHER							//  274
		,UNA_OTHER							//  275
		,UNA_OTHER							//  276
		,UNA_OTHER							//  277
		,UNA_OTHER							//  278
		,UNA_OTHER							//  279
		,UNA_OTHER							//  280
		,UNA_OTHER							//  281
		,UNA_OTHER							//  282
		,UNA_OTHER							//  283
		,UNA_OTHER							//  284
		,UNA_OTHER							//  285
		,UNA_OTHER							//  286
		,UNA_OTHER							//  287
		,UNA_OTHER							//  288
		,UNA_OTHER							//  289
		,UNA_OTHER							//  290
		,UNA_OTHER							//  291
		,UNA_OTHER							//  292
		,UNA_OTHER							//  293
		,UNA_OTHER							//  294
		,UNA_OTHER							//  295
		,UNA_OTHER							//  296
		,UNA_OTHER							//  297
		,UNA_OTHER							//  298
		,UNA_OTHER							//  299
		,UNA_OTHER							//  300
		,UNA_OTHER							//  301
		,UNA_OTHER							//  302
		,UNA_OTHER							//  303
		,UNA_OTHER							//  304
		,UNA_OTHER							//  305
		,UNA_OTHER							//  306
		,UNA_OTHER							//  307
		,UNA_OTHER							//  308
		,UNA_OTHER							//  309
		,UNA_OTHER							//  310
		,UNA_OTHER							//  311
		,UNA_OTHER							//  312
		,UNA_OTHER							//  313
		,UNA_OTHER							//  314
		,UNA_OTHER							//  315
		,UNA_OTHER							//  316
		,UNA_OTHER							//  317
		,UNA_OTHER							//  318
		,UNA_OTHER							//  319
		,UNA_OTHER							//  320
		,UNA_OTHER							//  321
		,UNA_OTHER							//  322
		,UNA_OTHER							//  323
		,UNA_OTHER							//  324
		,UNA_OTHER							//  325
		,UNA_OTHER							//  326
		,UNA_OTHER							//  327
		,UNA_OTHER							//  328
		,UNA_OTHER							//  329
		,UNA_OTHER							//  330
		,UNA_OTHER							//  331
		,UNA_OTHER							//  332
		,UNA_OTHER							//  333
		,UNA_OTHER							//  334
		,UNA_OTHER							//  335
		,UNA_OTHER							//  336
		,UNA_OTHER							//  337
		,UNA_OTHER							//  338
		,UNA_OTHER							//  339
		,UNA_OTHER							//  340
		,UNA_OTHER							//  341
		,UNA_OTHER							//  342
		,UNA_OTHER							//  343
		,UNA_OTHER							//  344
		,UNA_OTHER							//  345
		,UNA_OTHER							//  346
		,UNA_OTHER							//  347
		,UNA_OTHER							//  348
		,UNA_OTHER							//  349
		,UNA_OTHER							//  350
		,UNA_OTHER							//  351
		,UNA_OTHER							//  352
		,UNA_OTHER							//  353
		,UNA_OTHER							//  354
		,UNA_OTHER							//  355
		,UNA_OTHER							//  356
		,UNA_OTHER							//  357
		,UNA_OTHER							//  358
		,UNA_OTHER							//  359
		,UNA_OTHER							//  360
		,UNA_OTHER							//  361
		,UNA_OTHER							//  362
		,UNA_OTHER							//  363
		,UNA_OTHER							//  364
		,UNA_OTHER							//  365
		,UNA_OTHER							//  366
		,UNA_OTHER							//  367
		,UNA_OTHER							//  368
		,UNA_OTHER							//  369
		,UNA_OTHER							//  370
		,UNA_OTHER							//  371
		,UNA_OTHER							//  372
		,UNA_OTHER							//  373
		,UNA_OTHER							//  374
		,UNA_OTHER							//  375
		,UNA_OTHER							//  376
		,UNA_OTHER							//  377
		,UNA_OTHER							//  378
		,UNA_OTHER							//  379
		,UNA_OTHER							//  380
		,UNA_OTHER							//  381
		,UNA_OTHER							//  382
		,UNA_OTHER							//  383
		,UNA_OTHER							//  384
		,UNA_OTHER							//  385
		,UNA_OTHER							//  386
		,UNA_OTHER							//  387
		,UNA_OTHER							//  388
		,UNA_OTHER							//  389
		,UNA_OTHER							//  390
		,UNA_OTHER							//  391
		,UNA_OTHER							//  392
		,UNA_OTHER							//  393
		,UNA_OTHER							//  394
		,UNA_OTHER							//  395
		,UNA_OTHER							//  396
		,UNA_OTHER							//  397
		,UNA_OTHER							//  398
		,UNA_OTHER							//  399
		,UNA_OTHER							//  400
		,UNA_OTHER							//  401
		,UNA_OTHER							//  402
		,UNA_OTHER							//  403
		,UNA_OTHER							//  404
		,UNA_OTHER							//  405
		,UNA_OTHER							//  406
		,UNA_OTHER							//  407
		,UNA_OTHER							//  408
		,UNA_OTHER							//  409
		,UNA_OTHER							//  410
		,UNA_OTHER							//  411
		,UNA_OTHER							//  412
		,UNA_OTHER							//  413
		,UNA_OTHER							//  414
		,UNA_OTHER							//  415
		,UNA_OTHER							//  416
		,UNA_OTHER							//  417
		,UNA_OTHER							//  418
		,UNA_OTHER							//  419
		,UNA_OTHER							//  420
		,UNA_OTHER							//  421
		,UNA_OTHER							//  422
		,UNA_OTHER							//  423
		,UNA_OTHER							//  424
		,UNA_OTHER							//  425
		,UNA_OTHER							//  426
		,UNA_OTHER							//  427
		,UNA_OTHER							//  428
		,UNA_OTHER							//  429
		,UNA_OTHER							//  430
		,UNA_OTHER							//  431
		,UNA_OTHER							//  432
		,UNA_OTHER							//  433
		,UNA_OTHER							//  434
		,UNA_OTHER							//  435
		,UNA_OTHER							//  436
		,UNA_OTHER							//  437
		,UNA_OTHER							//  438
		,UNA_OTHER							//  439
		,UNA_OTHER							//  440
		,UNA_OTHER							//  441
		,UNA_OTHER							//  442
		,UNA_OTHER							//  443
		,UNA_OTHER							//  444
		,UNA_OTHER							//  445
		,UNA_VERB							//  446
		,UNA_VERB							//  447
		,UNA_VERB							//  448
		,UNA_VERB							//  449
		,UNA_VERB							//  450
		,UNA_VERB							//  451
		,UNA_VERB							//  452
		,UNA_VERB							//  453
		,UNA_VERB							//  454
		,UNA_VERB							//  455
		,UNA_VERB							//  456
		,UNA_VERB							//  457
		,UNA_VERB							//  458
		,UNA_VERB							//  459
		,UNA_VERB							//  460
		,UNA_VERB							//  461
		,UNA_VERB							//  462
		,UNA_VERB							//  463
		,UNA_VERB							//  464
		,UNA_VERB							//  465
		,UNA_VERB							//  466
		,UNA_VERB							//  467
		,UNA_VERB							//  468
		,UNA_VERB							//  469
		,UNA_VERB							//  470
		,UNA_VERB							//  471
		,UNA_VERB							//  472
		,UNA_VERB							//  473
		,UNA_VERB							//  474
		,UNA_VERB							//  475
		,UNA_VERB							//  476
		,UNA_VERB							//  477
		,UNA_VERB							//  478
		,UNA_VERB							//  479
		,UNA_VERB							//  480
		,UNA_VERB							//  481
		,UNA_VERB							//  482
		,UNA_VERB							//  483
		,UNA_VERB							//  484
		,UNA_VERB							//  485
		,UNA_VERB							//  486
		,UNA_VERB							//  487
		,UNA_VERB							//  488
		,UNA_VERB							//  489
		,UNA_VERB							//  490
		,UNA_VERB							//  491
		,UNA_VERB							//  492
		,UNA_VERB							//  493
		,UNA_VERB							//  494
		,UNA_VERB							//  495
		,UNA_VERB							//  496
		,UNA_VERB							//  497
		,UNA_VERB							//  498
		,UNA_VERB							//  499
		,UNA_VERB							//  500
		,UNA_VERB							//  501
		,UNA_VERB							//  502
		,UNA_VERB							//  503
		,UNA_VERB							//  504
		,UNA_VERB							//  505
		,UNA_VERB							//  506
		,UNA_VERB							//  507
		,UNA_VERB							//  508
		,UNA_VERB							//  509
		,UNA_VERB							//  510
		,UNA_VERB							//  511
		,UNA_VERB							//  512
		,UNA_VERB							//  513
		,UNA_VERB							//  514
		,UNA_VERB							//  515
		,UNA_VERB							//  516
		,UNA_VERB							//  517
		,UNA_VERB							//  518
		,UNA_VERB							//  519
		,UNA_VERB							//  520
		,UNA_VERB							//  521
		,UNA_VERB							//  522
		,UNA_VERB							//  523
		,UNA_VERB							//  524
		,UNA_VERB							//  525
		,UNA_VERB							//  526
		,UNA_VERB							//  527
		,UNA_VERB							//  528
		,UNA_VERB							//  529
		,UNA_VERB							//  530
		,UNA_VERB							//  531
		,UNA_VERB							//  532
		,UNA_VERB							//  533
		,UNA_VERB							//  534
		,UNA_VERB							//  535
		,UNA_VERB							//  536
		,UNA_VERB							//  537
		,UNA_VERB							//  538
		,UNA_VERB							//  539
		,UNA_VERB							//  540
		,UNA_VERB							//  541
		,UNA_VERB							//  542
		,UNA_VERB							//  543
		,UNA_VERB							//  544
		,UNA_VERB							//  545
		,UNA_VERB							//  546
		,UNA_VERB							//  547
		,UNA_VERB							//  548
		,UNA_VERB							//  549
		,UNA_VERB							//  550
		,UNA_VERB							//  551
		,UNA_VERB							//  552
		,UNA_VERB							//  553
		,UNA_VERB|UNA_QUALIFIER				//  554
		,UNA_VERB|UNA_QUALIFIER				//  555
		,UNA_VERB|UNA_QUALIFIER				//  556
		,UNA_VERB|UNA_QUALIFIER				//  557
		,UNA_VERB|UNA_QUALIFIER				//  558
		,UNA_VERB|UNA_QUALIFIER				//  559
		,UNA_VERB|UNA_QUALIFIER				//  560
		,UNA_VERB|UNA_QUALIFIER				//  561
		,UNA_VERB|UNA_QUALIFIER				//  562
		,UNA_VERB|UNA_QUALIFIER				//  563
		,UNA_VERB|UNA_QUALIFIER				//  564
		,UNA_VERB|UNA_QUALIFIER				//  565
		,UNA_VERB|UNA_QUALIFIER				//  566
		,UNA_VERB|UNA_QUALIFIER				//  567
		,UNA_VERB|UNA_QUALIFIER				//  568
		,UNA_VERB|UNA_QUALIFIER				//  569
		,UNA_VERB|UNA_QUALIFIER				//  570
		,UNA_VERB|UNA_QUALIFIER				//  571
		,UNA_VERB|UNA_QUALIFIER				//  572
		,UNA_VERB|UNA_QUALIFIER				//  573
		,UNA_VERB|UNA_QUALIFIER				//  574
		,UNA_VERB|UNA_QUALIFIER				//  575
		,UNA_VERB|UNA_QUALIFIER				//  576
		,UNA_VERB|UNA_QUALIFIER				//  577
		,UNA_VERB|UNA_QUALIFIER				//  578
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  579
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  580
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  581
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  582
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  583
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  584
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  585
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  586
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  587
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  588
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  589
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  590
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  591
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  592
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  593
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  594
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  595
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  596
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  597
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  598
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  599
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  600
		,UNA_PRO|UNA_VERB|UNA_QUALIFIER		//  601
		,UNA_VERB							//  602
		,UNA_VERB							//  603
		,UNA_VERB							//  604
		,UNA_VERB							//  605
		,UNA_VERB							//  606
		,UNA_VERB							//  607
		,UNA_VERB							//  608
		,UNA_VERB							//  609
		,UNA_VERB							//  610
		,UNA_VERB							//  611
		,UNA_VERB							//  612
		,UNA_NOUN|UNA_VERB					//  613
		,UNA_NOUN|UNA_VERB					//  614
		,UNA_NOUN|UNA_VERB					//  615
		,UNA_NOUN|UNA_VERB					//  616
		,UNA_NOUN|UNA_VERB					//  617
		,UNA_NOUN|UNA_VERB					//  618
		,UNA_NOUN|UNA_VERB					//  619
		,UNA_NOUN|UNA_VERB					//  620
		,UNA_NOUN|UNA_VERB					//  621
		,UNA_NOUN|UNA_VERB					//  622
		,UNA_NOUN|UNA_VERB					//  623
		,UNA_NOUN|UNA_VERB					//  624
		,UNA_NOUN|UNA_VERB					//  625
		,UNA_NOUN|UNA_VERB					//  626
		,UNA_NOUN|UNA_VERB					//  627
		,UNA_NOUN|UNA_VERB					//  628
		,UNA_NOUN|UNA_VERB					//  629
		,UNA_NOUN|UNA_VERB					//  630
		,UNA_NOUN|UNA_VERB					//  631
		,UNA_NOUN|UNA_VERB					//  632
		,UNA_NOUN							//  633
		,UNA_NOUN							//  634
		,UNA_NOUN							//  635
		,UNA_NOUN							//  636
		,UNA_NOUN							//  637
		,UNA_NOUN							//  638
		,UNA_NOUN							//  639
		,UNA_NOUN							//  640
		,UNA_NOUN							//  641
		,UNA_NOUN							//  642
		,UNA_NOUN							//  643
		,UNA_NOUN							//  644
		,UNA_NOUN							//  645
		,UNA_NOUN							//  646
		,UNA_NOUN							//  647
		,UNA_NOUN							//  648
		,UNA_NOUN							//  649
		,UNA_NOUN							//  650
		,UNA_NOUN							//  651
		,UNA_NOUN							//  652
		,UNA_NOUN							//  653
		,UNA_NOUN							//  654
		,UNA_NOUN							//  655
		,UNA_NOUN							//  656
		,UNA_NOUN							//  657
		,UNA_NOUN							//  658
		,UNA_NOUN							//  659
		,UNA_NOUN							//  660
		,UNA_NOUN							//  661
		,UNA_NOUN							//  662
		,UNA_NOUN							//  663
		,UNA_NOUN							//  664
		,UNA_NOUN							//  665
		,UNA_NOUN							//  666
		,UNA_NOUN							//  667
		,UNA_NOUN							//  668
		,UNA_NOUN							//  669
		,UNA_NOUN							//  670
		,UNA_NOUN							//  671
		,UNA_NOUN							//  672
		,UNA_NOUN							//  673
		,UNA_NOUN							//  674
		,UNA_NOUN							//  675
		,UNA_NOUN							//  676
		,UNA_NOUN							//  677
		,UNA_NOUN							//  678
		,UNA_NOUN							//  679
		,UNA_NOUN							//  680
		,UNA_NOUN							//  681
		,UNA_NOUN							//  682
		,UNA_NOUN							//  683
		,UNA_NOUN							//  684
		,UNA_NOUN							//  685
		,UNA_NOUN							//  686
		,UNA_NOUN							//  687
		,UNA_NOUN							//  688
		,UNA_NOUN							//  689
		,UNA_NOUN							//  690
		,UNA_NOUN							//  691
		,UNA_NOUN							//  692
		,UNA_NOUN							//  693
		,UNA_NOUN							//  694
		,UNA_NOUN							//  695
		,UNA_NOUN							//  696
		,UNA_NOUN							//  697
		,UNA_NOUN							//  698
		,UNA_NOUN							//  699
		,UNA_NOUN							//  700
		,UNA_NOUN							//  701
		,UNA_NOUN							//  702
		,UNA_NOUN							//  703
		,UNA_NOUN							//  704
		,UNA_NOUN							//  705
		,UNA_NOUN							//  706
		,UNA_NOUN							//  707
		,UNA_NOUN							//  708
		,UNA_NOUN							//  709
		,UNA_NOUN							//  710
		,UNA_NOUN							//  711
		,UNA_NOUN							//  712
		,UNA_NOUN							//  713
		,UNA_NOUN							//  714
		,UNA_NOUN							//  715
		,UNA_NOUN							//  716
		,UNA_NOUN							//  717
		,UNA_NOUN							//  718
		,UNA_NOUN							//  719
		,UNA_NOUN							//  720
		,UNA_NOUN							//  721
		,UNA_NOUN							//  722
		,UNA_NOUN							//  723
		,UNA_NOUN							//  724
		,UNA_NOUN							//  725
		,UNA_NOUN|UNA_ADV					//  726
		,UNA_NOUN|UNA_ADV					//  727
		,UNA_NOUN|UNA_ADV					//  728
		,UNA_NOUN|UNA_ADV					//  729
		,UNA_NOUN|UNA_ADV					//  730
		,UNA_NOUN|UNA_ADV					//  731
		,UNA_NOUN|UNA_ADV					//  732
		,UNA_NOUN|UNA_ADV					//  733
		,UNA_NOUN|UNA_ADV					//  734
		,UNA_NOUN|UNA_ADV					//  735
		,UNA_NOUN|UNA_ADV					//  736
		,UNA_NOUN|UNA_ADV					//  737
		,UNA_NOUN|UNA_ADV					//  738
		,UNA_NOUN|UNA_ADV					//  739
		,UNA_NOUN|UNA_ADV					//  740
		,UNA_NOUN|UNA_ADV					//  741
		,UNA_NOUN|UNA_ADV					//  742
		,UNA_NOUN|UNA_ADV					//  743
		,UNA_NOUN|UNA_ADV					//  744
		,UNA_NOUN|UNA_ADV					//  745
		,UNA_NOUN|UNA_ADV					//  746
		,UNA_NOUN|UNA_PROPER				//  747
		,UNA_NOUN|UNA_PROPER				//  748
		,UNA_NOUN|UNA_PROPER				//  749
		,UNA_NOUN|UNA_QUALIFIER				//  750
		,UNA_NOUN|UNA_QUALIFIER				//  715
		,UNA_NOUN|UNA_QUALIFIER				//  752
		,UNA_NOUN|UNA_QUALIFIER				//  753
		,UNA_NOUN|UNA_QUALIFIER				//  754
		,UNA_NOUN|UNA_QUALIFIER				//  755
		,UNA_NOUN|UNA_QUALIFIER				//  756
		,UNA_NOUN|UNA_QUALIFIER				//  757
		,UNA_NOUN|UNA_QUALIFIER				//  758
		,UNA_NOUN|UNA_QUALIFIER				//  759
		,UNA_NOUN|UNA_QUALIFIER				//  760
		,UNA_NOUN|UNA_QUALIFIER				//  761
		,UNA_NOUN|UNA_QUALIFIER				//  762
		,UNA_NOUN|UNA_QUALIFIER				//  763
		,UNA_NOUN|UNA_QUALIFIER				//  764
		,UNA_NOUN|UNA_QUALIFIER				//  765
		,UNA_NOUN|UNA_QUALIFIER				//  766
		,UNA_NOUN|UNA_QUALIFIER				//  767
		,UNA_NOUN|UNA_QUALIFIER				//  768
		,UNA_NOUN|UNA_QUALIFIER				//  769
		,UNA_NOUN|UNA_QUALIFIER				//  770
		,UNA_NOUN							//  771
		,UNA_NOUN							//  772
		,UNA_NOUN							//  773
		,UNA_NOUN							//  774
		,UNA_NOUN							//  775
		,UNA_NOUN|UNA_OTHER					//  776
		,UNA_NOUN|UNA_OTHER					//  777
		,UNA_NOUN|UNA_OTHER					//  778
		,UNA_NOUN|UNA_OTHER					//  779
		,UNA_NOUN|UNA_SFX					//  780
		,UNA_NOUN|UNA_SFX					//  781
		,UNA_NOUN|UNA_SFX					//  782
		,UNA_NOUN							//  783
		,UNA_QUALIFIER						//  784
		,UNA_QUALIFIER						//  785
		,UNA_QUALIFIER						//  786
		,UNA_QUALIFIER						//  787
		,UNA_QUALIFIER						//  788
		,UNA_QUALIFIER						//  789
		,UNA_QUALIFIER						//  790
		,UNA_QUALIFIER						//  791
		,UNA_QUALIFIER						//  792
		,UNA_QUALIFIER						//  793
		,UNA_QUALIFIER						//  794
		,UNA_QUALIFIER						//  795
		,UNA_QUALIFIER						//  796
		,UNA_QUALIFIER						//  797
		,UNA_QUALIFIER						//  798
		,UNA_QUALIFIER						//  799
		,UNA_QUALIFIER						//  800
		,UNA_QUALIFIER						//  801
		,UNA_QUALIFIER						//  802
		,UNA_QUALIFIER						//  803
		,UNA_QUALIFIER						//  804
		,UNA_QUALIFIER						//  805
		,UNA_QUALIFIER						//  806
		,UNA_QUALIFIER						//  807
		,UNA_QUALIFIER						//  808
		,UNA_QUALIFIER						//  809
		,UNA_QUALIFIER						//  810
		,UNA_QUALIFIER						//  811
		,UNA_QUALIFIER						//  812
		,UNA_QUALIFIER						//  813
		,UNA_QUALIFIER						//  814
		,UNA_QUALIFIER						//  815
		,UNA_QUALIFIER						//  816
		,UNA_QUALIFIER						//  817
		,UNA_QUALIFIER						//  818
		,UNA_QUALIFIER						//  819
		,UNA_QUALIFIER						//  820
		,UNA_QUALIFIER						//  821
		,UNA_QUALIFIER						//  822
		,UNA_QUALIFIER						//  823
		,UNA_QUALIFIER						//  824
		,UNA_QUALIFIER						//  825
		,UNA_QUALIFIER						//  826
		,UNA_QUALIFIER						//  827
		,UNA_QUALIFIER						//  828
		,UNA_QUALIFIER						//  829
		,UNA_QUALIFIER						//  830
		,UNA_QUALIFIER						//  831
		,UNA_QUALIFIER						//  832
		,UNA_QUALIFIER						//  833
		,UNA_QUALIFIER						//  834
		,UNA_QUALIFIER						//  835
		,UNA_QUALIFIER						//  836
		,UNA_QUALIFIER						//  837
		,UNA_QUALIFIER						//  838
		,UNA_QUALIFIER						//  839
		,UNA_QUALIFIER						//  840
		,UNA_QUALIFIER						//  841
		,UNA_QUALIFIER|UNA_A				//  842
		,UNA_QUALIFIER|UNA_A				//  843
		,UNA_QUALIFIER|UNA_A				//  844
		,UNA_QUALIFIER|UNA_A				//  845
		,UNA_QUALIFIER|UNA_A				//  846
		,UNA_QUALIFIER|UNA_A				//  847
		,UNA_QUALIFIER|UNA_A				//  848
		,UNA_QUALIFIER|UNA_A				//  849
		,UNA_QUALIFIER|UNA_A				//  850
		,UNA_QUALIFIER|UNA_A				//  851
		,UNA_QUALIFIER|UNA_A				//  852
		,UNA_QUALIFIER|UNA_A				//  853
		,UNA_QUALIFIER|UNA_A				//  854
		,UNA_QUALIFIER|UNA_A				//  855
		,UNA_QUALIFIER|UNA_A				//  856
		,UNA_QUALIFIER|UNA_A				//  857
		,UNA_QUALIFIER|UNA_A				//  858
		,UNA_QUALIFIER|UNA_A				//  859
		,UNA_QUALIFIER|UNA_A				//  860
		,UNA_QUALIFIER|UNA_A				//  861
		,UNA_QUALIFIER|UNA_A				//  862
		,UNA_FILLER							//  863
		,UNA_SFX							//  864
		,UNA_SFX							//  865
		,UNA_SFX							//  866
		,UNA_SFX							//  867
		,UNA_SFX							//  868
		,UNA_SFX							//  869
		,UNA_SFX							//  870
		,UNA_SFX							//  871
		,UNA_SFX							//  872
		,UNA_SFX							//  873
		,UNA_SFX							//  874
		,UNA_OTHER							//  875
		,UNA_OTHER							//  876
		,UNA_OTHER							//  877
		,UNA_OTHER							//  878
		,UNA_OTHER							//  879
		,UNA_OTHER							//  880
		,UNA_OTHER							//  881
		,UNA_OTHER							//  882
		,UNA_OTHER							//  883
		,UNA_OTHER							//  884
		,UNA_OTHER							//  885
		,UNA_OTHER							//  886
		,UNA_OTHER							//  887
		,UNA_OTHER							//  888
		,UNA_OTHER							//  889
		,UNA_OTHER							//  890
		,UNA_OTHER							//  891
		,UNA_OTHER							//  892
		,UNA_OTHER							//  893
		,UNA_OTHER							//  894
		,UNA_OTHER							//  895
		,UNA_OTHER							//  896
		,UNA_OTHER							//  897
		,UNA_OTHER							//  898
		,UNA_OTHER							//  899
		,UNA_OTHER							//  900
		,UNA_CONJ							//  901
		,UNA_CONJ							//  902
		,UNA_CONJ							//  903
		,UNA_CONJ							//  904
		,UNA_CONJ							//  905
		,UNA_CONJ							//  906
		,UNA_CONJ							//  907
		,UNA_CONJ							//  908
		,UNA_CONJ							//  909
		,UNA_PFX							//  910
		,UNA_PFX							//  911
		,UNA_NUMBER							//  912
		,UNA_OTHER							//  913
		,UNA_OTHER							//  914
		,UNA_OTHER							//  915
		,UNA_OTHER							//  916
		,UNA_OTHER							//  917
		,UNA_OTHER							//  918
		,UNA_OTHER							//  919
		,UNA_OTHER							//  920
		,UNA_OTHER							//  921
		,UNA_OTHER							//  922
		,UNA_OTHER							//  923
		,UNA_OTHER							//  924
		,UNA_OTHER							//  925
		,UNA_OTHER							//  926
		,UNA_OTHER							//  927
		,UNA_OTHER							//  928
		,UNA_OTHER							//  929
		,UNA_OTHER							//  930
		,UNA_OTHER							//  931
		,UNA_OTHER							//  932
		,UNA_OTHER							//  933
		,UNA_OTHER							//  934
		,UNA_OTHER							//  935
		,UNA_OTHER							//  936
		,UNA_VERB							//  937
		,UNA_VERB							//  938
		,UNA_VERB							//  939
		,UNA_VERB							//  940
		,UNA_VERB							//  941
		,UNA_VERB							//  942
		,UNA_VERB							//  943
		,UNA_VERB							//  944
		,UNA_VERB							//  945
		,UNA_VERB							//  946
		,UNA_VERB							//  947
		,UNA_VERB							//  948
		,UNA_VERB							//  949
		,UNA_VERB							//  950
		,UNA_VERB							//  951
		,UNA_VERB							//  952
		,UNA_VERB							//  953
		,UNA_VERB							//  954
		,UNA_VERB							//  955
		,UNA_VERB							//  956
		,UNA_VERB							//  957
		,UNA_VERB							//  958
		,UNA_VERB							//  959
		,UNA_VERB							//  960
		,UNA_VERB							//  961
		,UNA_VERB							//  962
		,UNA_VERB							//  963
		,UNA_VERB							//  964
		,UNA_VERB							//  965
		,UNA_VERB							//  966
		,UNA_VERB							//  967
		,UNA_VERB							//  968
		,UNA_VERB							//  969
		,UNA_VERB							//  970
		,UNA_VERB							//  971
		,UNA_VERB							//  972
		,UNA_VERB							//  973
		,UNA_VERB							//  974
		,UNA_VERB							//  975
		,UNA_VERB							//  976
		,UNA_VERB							//  977
		,UNA_VERB							//  978
		,UNA_VERB							//  979
		,UNA_VERB							//  980
		,UNA_VERB							//  981
		,UNA_VERB							//  982
		,UNA_VERB							//  983
		,UNA_VERB							//  984
		,UNA_VERB|UNA_QUALIFIER				//  985
		,UNA_VERB|UNA_QUALIFIER				//  986
		,UNA_VERB|UNA_QUALIFIER				//  987
		,UNA_VERB|UNA_QUALIFIER				//  988
		,UNA_VERB|UNA_QUALIFIER				//  989
		,UNA_VERB|UNA_QUALIFIER				//  990
		,UNA_VERB|UNA_QUALIFIER				//  991
		,UNA_VERB|UNA_QUALIFIER				//  992
		,UNA_VERB|UNA_QUALIFIER				//  993
		,UNA_VERB|UNA_QUALIFIER				//  994
		,UNA_VERB|UNA_QUALIFIER				//  995
		,UNA_VERB|UNA_QUALIFIER				//  996
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  997
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  998
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  999
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  1000
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  1001
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  1002
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  1003
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  1004
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  1005
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  1006
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  1007
		,UNA_VERB							//  1008
		,UNA_VERB							//  1009
		,UNA_VERB							//  1010
		,UNA_VERB							//  1011
		,UNA_VERB							//  1012
		,UNA_VERB							//  1013
		,UNA_VERB							//  1014
		,UNA_QUALIFIER|UNA_A				//  1015
		,UNA_QUALIFIER|UNA_A				//  1016
		,UNA_QUALIFIER|UNA_A				//  1017
		,UNA_QUALIFIER|UNA_A				//  1018
		,UNA_QUALIFIER|UNA_A				//  1019
		,UNA_QUALIFIER|UNA_A				//  1020
		,UNA_QUALIFIER|UNA_A				//  1021
		,UNA_QUALIFIER|UNA_A				//  1022
		,UNA_QUALIFIER|UNA_A				//  1023
		,UNA_QUALIFIER|UNA_A				//  1024
		,UNA_QUALIFIER|UNA_A				//  1025
		,UNA_QUALIFIER|UNA_A				//  1026
		,UNA_QUALIFIER|UNA_A				//  1027
		,UNA_QUALIFIER|UNA_A				//  1028
		,UNA_QUALIFIER|UNA_A				//  1029
		,UNA_QUALIFIER|UNA_A				//  1030
		,UNA_QUALIFIER|UNA_A				//  1031
		,UNA_QUALIFIER|UNA_A				//  1032
		,UNA_QUALIFIER|UNA_A				//  1033
		,UNA_QUALIFIER|UNA_A				//  1034
		,UNA_QUALIFIER|UNA_A				//  1035
		,UNA_QUALIFIER|UNA_A				//  1036
		,UNA_QUALIFIER|UNA_A				//  1037
		,UNA_QUALIFIER|UNA_A				//  1038
		,UNA_QUALIFIER|UNA_A				//  1039
		,UNA_QUALIFIER|UNA_A				//  1040
		,UNA_QUALIFIER|UNA_A				//  1041
		,UNA_QUALIFIER|UNA_A				//  1042
		,UNA_QUALIFIER|UNA_A				//  1043
		,UNA_SYMBOL							//  1044
		,UNA_SYMBOL							//  1045
		,UNA_SYMBOL							//  1046
		,UNA_SYMBOL							//  1047
		,UNA_SYMBOL							//  1048
		,UNA_SYMBOL							//  1049
		,UNA_QUALIFIER						//  1050
		,UNA_VERB							//  1051
		,UNA_VERB							//  1052
		,UNA_VERB							//  1053
		,UNA_VERB							//  1054
		,UNA_VERB							//  1055
		,UNA_VERB							//  1056
		,UNA_VERB							//  1057
		,UNA_VERB							//  1058
		,UNA_VERB							//  1059
		,UNA_VERB							//  1060
		,UNA_VERB							//  1061
		,UNA_VERB							//  1062
		,UNA_VERB							//  1063
		,UNA_VERB							//  1064
		,UNA_VERB							//  1065
		,UNA_VERB							//  1066
		,UNA_VERB|UNA_QUALIFIER				//  1067
		,UNA_VERB|UNA_QUALIFIER				//  1068
		,UNA_VERB|UNA_QUALIFIER				//  1069
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  1070
		,UNA_NOUN|UNA_VERB|UNA_QUALIFIER	//  1071
		,UNA_VERB							//  1072
		,UNA_VERB							//  1073
		,UNA_NOUN|UNA_VERB					//  1074
		,UNA_SFX							//  1075
		,UNA_SYMBOL							//  1076
	};

	const TransTable JpTransTable [] = {
		 {	 0, "ダミー" }
		,{	 0x1000|0x0200 	,  "未登録語.カタカナ"	} //  1
		,{	 0x2000|0x0900|0x0003 	,  "記号.句点"	} //  2
		,{	 0x2000|0x0900|0x0004 	,  "記号.読点"	} //  3
		,{	 0x2000|0x0a00 	,  "数詞"	} //  4
		,{	 0x2000|0x0100 	,  "名詞.一般"	} //  5
		,{	 0x2000|0x0b00 	,  "名詞.固有"	} //  6
		,{	 0x2000|0x0700 	,  "名詞.指示"	} //  7
		,{	 0x2000|0x0200 	,  "名詞.サ変"	} //  8
		,{	 0xc600 	,  "助数詞"	} //  9
		,{	 0x2000|0x0800 	,  "形容動詞.一般"	} //  10
		,{	 0x5000 	,  "連体詞"	} //  11
		,{	 0x3000 	,  "副詞"	} //  12
		,{	 0x6000 	,  "接続詞"	} //  13
		,{	 0x4000 	,  "感動詞"	} //  14
		,{	 0xb000 	,  "接頭辞.一般"	} //  15
		,{	 0xc000 	,  "接尾辞"	} //  16
		,{	 0x2000|0x0900 	,  "記号.一般"	} //  17
		,{	 0x7000|0x0020|0x0900 	,  "動詞.終止連体,カ行五段"	} //  18
		,{	 0x7000|0x0020|0x0100 	,  "動詞.音便,カ行五段"	} //  19
		,{	 0x7000|0x0020|0x0400 	,  "動詞.未然,カ行五段"	} //  20
		,{	 0x7000|0x0020|0x0100 	,  "動詞.連用,カ行五段"	} //  21
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,カ行五段"	} //  22
		,{	 0x7000|0x0020|0x0400 	,  "動詞.ウ接続,カ行五段"	} //  23
		,{	 0x7000|0x0020|0x0900 	,  "動詞.終止連体,ガ行五段"	} //  24
		,{	 0x7000|0x0020|0x0100 	,  "動詞.音便,ガ行五段"	} //  25
		,{	 0x7000|0x0020|0x0400 	,  "動詞.未然,ガ行五段"	} //  26
		,{	 0x7000|0x0020|0x0100 	,  "動詞.連用,ガ行五段"	} //  27
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,ガ行五段"	} //  28
		,{	 0x7000|0x0020|0x0400 	,  "動詞.ウ接続,ガ行五段"	} //  29
		,{	 0x7000|0x0020|0x0900 	,  "動詞.終止連体,サ行五段"	} //  30
		,{	 0x7000|0x0020|0x0400 	,  "動詞.未然,サ行五段"	} //  31
		,{	 0x7000|0x0020|0x0100 	,  "動詞.連用,サ行五段"	} //  32
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,サ行五段"	} //  33
		,{	 0x7000|0x0020|0x0400 	,  "動詞.ウ接続,サ行五段"	} //  34
		,{	 0x7000|0x0020|0x0900 	,  "動詞.終止連体,タ行五段"	} //  35
		,{	 0x7000|0x0020|0x0100 	,  "動詞.音便,タ行五段"	} //  36
		,{	 0x7000|0x0020|0x0400 	,  "動詞.未然,タ行五段"	} //  37
		,{	 0x7000|0x0020|0x0100 	,  "動詞.連用,タ行五段"	} //  38
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,タ行五段"	} //  39
		,{	 0x7000|0x0020|0x0400 	,  "動詞.ウ接続,タ行五段"	} //  40
		,{	 0x7000|0x0020|0x0900 	,  "動詞.終止連体,ナ行五段"	} //  41
		,{	 0x7000|0x0020|0x0400 	,  "動詞.未然,ナ行五段"	} //  42
		,{	 0x7000|0x0020|0x0100 	,  "動詞.連用,ナ行五段"	} //  43
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,ナ行五段"	} //  44
		,{	 0x7000|0x0020|0x0400 	,  "動詞.ウ接続,ナ行五段"	} //  45
		,{	 0x7000|0x0020|0x0100 	,  "動詞.音便,ナ行五段"	} //  46
		,{	 0x7000|0x0020|0x0900 	,  "動詞.終止連体,バ行五段"	} //  47
		,{	 0x7000|0x0020|0x0400 	,  "動詞.未然,バ行五段"	} //  48
		,{	 0x7000|0x0020|0x0100 	,  "動詞.連用,バ行五段"	} //  49
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,バ行五段"	} //  50
		,{	 0x7000|0x0020|0x0400 	,  "動詞.ウ接続,バ行五段"	} //  51
		,{	 0x7000|0x0020|0x0100 	,  "動詞.音便,バ行五段"	} //  52
		,{	 0x7000|0x0020|0x0900 	,  "動詞.終止連体,マ行五段"	} //  53
		,{	 0x7000|0x0020|0x0400 	,  "動詞.未然,マ行五段"	} //  54
		,{	 0x7000|0x0020|0x0100 	,  "動詞.連用,マ行五段"	} //  55
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,マ行五段"	} //  56
		,{	 0x7000|0x0020|0x0400 	,  "動詞.ウ接続,マ行五段"	} //  57
		,{	 0x7000|0x0020|0x0100 	,  "動詞.音便,マ行五段"	} //  58
		,{	 0x7000|0x0020|0x0900 	,  "動詞.終止連体,ラ行五段"	} //  59
		,{	 0x7000|0x0020|0x0100 	,  "動詞.音便,ラ行五段"	} //  60
		,{	 0x7000|0x0020|0x0400 	,  "動詞.未然,ラ行五段"	} //  61
		,{	 0x7000|0x0020|0x0100 	,  "動詞.連用,ラ行五段"	} //  62
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,ラ行五段"	} //  63
		,{	 0x7000|0x0020|0x0400 	,  "動詞.ウ接続,ラ行五段"	} //  64
		,{	 0x7000|0x0020|0x0900 	,  "動詞.終止連体,ワ行五段"	} //  65
		,{	 0x7000|0x0020|0x0100 	,  "動詞.音便,ワ行五段"	} //  66
		,{	 0x7000|0x0020|0x0400 	,  "動詞.未然,ワ行五段"	} //  67
		,{	 0x7000|0x0020|0x0100 	,  "動詞.連用,ワ行五段"	} //  68
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,ワ行五段"	} //  69
		,{	 0x7000|0x0020|0x0400 	,  "動詞.ウ接続,ワ行五段"	} //  70
		,{	 0x7000|0x0010|0x0900 	,  "動詞.終止連体,一段"	} //  71
		,{	 0x7000|0x0010|0x0100 	,  "動詞.連用,一段"	} //  72
		,{	 0x7000|0x0010|0x0b00 	,  "動詞.仮定,一段"	} //  73
		,{	 0x7000|0x0010|0x0600 	,  "動詞.命令,一段.ロ"	} //  74
		,{	 0x7000|0x0010|0x0600 	,  "動詞.命令,一段.ヨ"	} //  75
		,{	 0x8000|0x0900 	,  "形容詞.終止連体"	} //  76
		,{	 0x8000|0x0c00 	,  "形容詞.語幹"	} //  77
		,{	 0x8000|0x0100 	,  "形容詞.音便"	} //  78
		,{	 0x8000|0x0400 	,  "形容詞.ズ接続"	} //  79
		,{	 0x8000|0x0300 	,  "形容詞.未然連用"	} //  80
		,{	 0x8000|0x0b00 	,  "形容詞.仮定"	} //  81
		,{	 0x8000|0x0100 	,  "形容詞.ウ接続"	} //  82
		,{	 0x8000|0x0800 	,  "形容詞.連体,文語.キ"	} //  83
		,{	 0x8000|0x0600 	,  "形容詞.命令,文語"	} //  84
		,{	 0x1000|0x0200 	,  "未登録語.名詞,サ変"	} //  85
		,{	 0x1000|0x0210 	,  "未登録語.名詞,固有"	} //  86
		,{	 0x1000|0x0220 	,  "未登録語.記号"	} //  87
		,{	 0x1000|0x0240 	,  "未登録語.記号,アルファベット"	} //  88
		,{	 0x1000|0x0180 	,  "未登録語.活用語"	} //  89
		,{	 0x1000 	,  "未登録語.一般"	} //  90
		,{	 0x2000 	,  "ユーザ定義1"	} //  91
		,{	 0x2000 	,  "ユーザ定義2"	} //  92
		,{	 0x2000 	,  "ユーザ定義3"	} //  93
		,{	 0x2000 	,  "ユーザ定義4"	} //  94
		,{	 0x2000 	,  "ユーザ定義5"	} //  95
		,{	 0x2000 	,  "ユーザ定義6"	} //  96
		,{	 0x2000 	,  "ユーザ定義7"	} //  97
		,{	 0x2000 	,  "ユーザ定義8"	} //  98
		,{	 0xa100|0x0105 	,  "副助詞.カ"	} //  99
		,{	 0xa100|0x0106 	,  "副助詞.キリ"	} //  100
		,{	 0xa202 	,  "副助詞.クライ"	} //  101
		,{	 0xa202 	,  "副助詞.クライ,濁音"	} //  102
		,{	 0xa100|0x0107 	,  "副助詞.コソ"	} //  103
		,{	 0xa100|0x010d 	,  "副助詞.サエ"	} //  104
		,{	 0xa100|0x010e 	,  "副助詞.シカ"	} //  105
		,{	 0xa100|0x010f 	,  "副助詞.シモ"	} //  106
		,{	 0xa100|0x0110 	,  "副助詞.スラ"	} //  107
		,{	 0xa100|0x0113 	,  "副助詞.ズツ"	} //  108
		,{	 0xa100|0x0102 	,  "副助詞.ダケ"	} //  109
		,{	 0xa100|0x0101 	,  "副助詞.ダノ"	} //  110
		,{	 0xa100|0x0103 	,  "副助詞.デモ"	} //  111
		,{	 0xa100|0x0111 	,  "副助詞.トテ"	} //  112
		,{	 0xa100|0x010a 	,  "副助詞.ナガラ"	} //  113
		,{	 0xa100|0x0101 	,  "副助詞.ナクシテ"	} //  114
		,{	 0xa100|0x0109 	,  "副助詞.ナド"	} //  115
		,{	 0xa100|0x010b 	,  "副助詞.ナリ"	} //  116
		,{	 0xa100|0x010c 	,  "副助詞.ノミ"	} //  117
		,{	 0xa100|0x0104 	,  "副助詞.ハ"	} //  118
		,{	 0xa204 	,  "副助詞.バカリ"	} //  119
		,{	 0xa201 	,  "副助詞.ホド"	} //  120
		,{	 0xa203 	,  "副助詞.マデ"	} //  121
		,{	 0xa100|0x0108 	,  "副助詞.モ"	} //  122
		,{	 0xa100|0x0101 	,  "副助詞.ヤ"	} //  123
		,{	 0xa100|0x0112 	,  "副助詞.ヤラ"	} //  124
		,{	 0xa100|0x0101 	,  "副助詞.相当;副助詞.カ＊副助詞.カ"	} //  125
		,{	 0xa100|0x0101 	,  "副助詞.相当;助動詞.ダ.仮定＊副助詞.ハ"	} //  126
		,{	 0xa100|0x0101 	,  "副助詞.相当;格助詞.デ＊副助詞.ハ"	} //  127
		,{	 0xa100|0x0101 	,  "副助詞.相当;格助詞.ト＊助動詞.ナイ.連用"	} //  128
		,{	 0xa100|0x0101 	,  "副助詞.相当;格助詞.ニ＊動詞.命令,サ変.ロ"	} //  129
		,{	 0xa100|0x0101 	,  "副助詞.相当;格助詞.ニ＊動詞.連用,一段"	} //  130
		,{	 0xa100|0x0101 	,  "副助詞.相当;格助詞.ニ＊接続助詞.テ"	} //  131
		,{	 0xa100|0x0101 	,  "副助詞.相当;格助詞.ヨリ＊名詞.一般"	} //  132
		,{	 0x3000 	,  "副詞;〜副助詞は"	} //  133
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞.ダ"	} //  134
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞.ダ;〜助動詞と"	} //  135
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞.ダ;〜助動詞と;〜格助詞の"	} //  136
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞.ダ;〜助動詞と;〜格助詞の;！未登録語カタ"	} //  137
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞.ダ;〜格助詞の"	} //  138
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞だ;〜助動詞と;〜助動詞に;〜格助詞の"	} //  139
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞だ;〜助動詞な;〜助動詞に;〜格助詞の"	} //  140
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞だ;〜助動詞に;〜格助詞の"	} //  141
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞だ;〜格助詞の"	} //  142
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞と"	} //  143
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞と;〜助動詞に"	} //  144
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞と;〜助動詞に;〜格助詞の"	} //  145
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞と;〜助動詞に;〜格助詞の;！未登録語カタ"	} //  146
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞と;〜助動詞に;！未登録語カタ"	} //  147
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞と;〜格助詞の"	} //  148
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞と;！未登録語カタ"	} //  149
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞に"	} //  150
		,{	 0x3000 	,  "副詞;〜副助詞は;〜助動詞に;〜格助詞の"	} //  151
		,{	 0x3000 	,  "副詞;〜副助詞は;〜格助詞の"	} //  152
		,{	 0x3000 	,  "副詞;〜副助詞は;〜格助詞の;！未登録語カタ"	} //  153
		,{	 0x3000 	,  "副詞;〜助動詞.ダ"	} //  154
		,{	 0x3000 	,  "副詞;〜助動詞.ダ;〜助動詞と"	} //  155
		,{	 0x3000 	,  "副詞;〜助動詞.ダ;〜助動詞と;〜格助詞の"	} //  156
		,{	 0x3000 	,  "副詞;〜助動詞.ダ;〜助動詞と;〜格助詞の;！未登録語カタ"	} //  157
		,{	 0x3000 	,  "副詞;〜助動詞.ダ;〜助動詞と;！未登録語カタ"	} //  158
		,{	 0x3000 	,  "副詞;〜助動詞.ダ;〜格助詞の"	} //  159
		,{	 0x3000 	,  "副詞;〜助動詞.ダ;〜格助詞の;！未登録語カタ"	} //  160
		,{	 0x3000 	,  "副詞;〜助動詞だ"	} //  161
		,{	 0x3000 	,  "副詞;〜助動詞だ;〜助動詞と"	} //  162
		,{	 0x3000 	,  "副詞;〜助動詞だ;〜助動詞と;〜助動詞に"	} //  163
		,{	 0x3000 	,  "副詞;〜助動詞だ;〜助動詞と;〜助動詞に;〜格助詞の"	} //  164
		,{	 0x3000 	,  "副詞;〜助動詞だ;〜助動詞と;〜助動詞に;〜格助詞の;！未登録語カタ"	} //  165
		,{	 0x3000 	,  "副詞;〜助動詞だ;〜助動詞と;〜格助詞の"	} //  166
		,{	 0x3000 	,  "副詞;〜助動詞だ;〜助動詞に"	} //  167
		,{	 0x3000 	,  "副詞;〜助動詞だ;〜助動詞に;〜格助詞の"	} //  168
		,{	 0x3000 	,  "副詞;〜助動詞だ;〜格助詞の"	} //  169
		,{	 0x3000 	,  "副詞;〜助動詞と"	} //  170
		,{	 0x3000 	,  "副詞;〜助動詞と;〜助動詞に"	} //  171
		,{	 0x3000 	,  "副詞;〜助動詞と;〜助動詞に;〜格助詞の"	} //  172
		,{	 0x3000 	,  "副詞;〜助動詞と;〜助動詞に;〜格助詞の;！未登録語カタ"	} //  173
		,{	 0x3000 	,  "副詞;〜助動詞と;〜助動詞に;！未登録語カタ"	} //  174
		,{	 0x3000 	,  "副詞;〜助動詞と;〜格助詞の"	} //  175
		,{	 0x3000 	,  "副詞;〜助動詞と;〜格助詞の;！未登録語カタ"	} //  176
		,{	 0x3000 	,  "副詞;〜助動詞と;！未登録語カタ"	} //  177
		,{	 0x3000 	,  "副詞;〜助動詞に"	} //  178
		,{	 0x3000 	,  "副詞;〜助動詞に;〜格助詞の"	} //  179
		,{	 0x3000 	,  "副詞;〜助動詞に;〜格助詞の;！未登録語カタ"	} //  180
		,{	 0x3000 	,  "副詞;〜助動詞に;！未登録語カタ"	} //  181
		,{	 0x3000 	,  "副詞;〜形容性"	} //  182
		,{	 0x3000 	,  "副詞;〜格助詞の"	} //  183
		,{	 0x3000 	,  "副詞;副詞＊名詞.副詞"	} //  184
		,{	 0x3000 	,  "副詞;副詞＊接尾辞;＝副詞"	} //  185
		,{	 0x3000 	,  "副詞;副詞＊終助詞"	} //  186
		,{	 0x3000 	,  "副詞;動詞.終止連体,ワ行五段＊名詞.形動;！助動詞な"	} //  187
		,{	 0x3000 	,  "副詞;名詞.サ変＊名詞.接尾辞"	} //  188
		,{	 0x3000 	,  "副詞;名詞.サ変＊接尾辞;＝形動"	} //  189
		,{	 0x3000 	,  "副詞;名詞.一般＊副助詞.ダケ"	} //  190
		,{	 0x3000 	,  "副詞;名詞.一般＊副助詞.バカリ"	} //  191
		,{	 0x3000 	,  "副詞;名詞.一般＊副助詞.ホド"	} //  192
		,{	 0x3000 	,  "副詞;名詞.副詞＊副助詞.ナガラ"	} //  193
		,{	 0x3000 	,  "副詞;名詞.副詞＊接尾辞;＝副詞"	} //  194
		,{	 0x3000 	,  "副詞;！名詞"	} //  195
		,{	 0x3000 	,  "副詞;！未登録語カタ"	} //  196
		,{	 0x9000|0x0026|0x0a00 	,  "助動詞.ウ.終止"	} //  197
		,{	 0x9000|0x0006|0x0400 	,  "助動詞.ガチダ.ウ接続"	} //  198
		,{	 0x9000|0x0006|0x0300 	,  "助動詞.ガチダ.未然連用"	} //  199
		,{	 0x9000|0x0006|0x0a00 	,  "助動詞.ガチダ.終止"	} //  200
		,{	 0x9000|0x0006|0x0c00 	,  "助動詞.ガチダ.語幹"	} //  201
		,{	 0x9000|0x0006|0x0800 	,  "助動詞.ガチダ.連体"	} //  202
		,{	 0x9000|0x0006|0x0800 	,  "助動詞.ガチダ.連体,ノ"	} //  203
		,{	 0x9000|0x0006|0x0100 	,  "助動詞.ガチダ.連用"	} //  204
		,{	 0x9000|0x0006|0x0100 	,  "助動詞.ガチダ.音便"	} //  205
		,{	 0x9000|0x0007|0x0400 	,  "助動詞.ガル.ウ接続"	} //  206
		,{	 0x9000|0x0007|0x0700 	,  "助動詞.ガル.仮定命令"	} //  207
		,{	 0x9000|0x0007|0x0400 	,  "助動詞.ガル.未然"	} //  208
		,{	 0x9000|0x0007|0x0900 	,  "助動詞.ガル.終止連体"	} //  209
		,{	 0x9000|0x0007|0x0100 	,  "助動詞.ガル.連用"	} //  210
		,{	 0x9000|0x0007|0x0100 	,  "助動詞.ガル.音便"	} //  211
		,{	 0x9000|0x0008|0x0a00 	,  "助動詞.ゴトシ.終止"	} //  212
		,{	 0x9000|0x0008|0x0800 	,  "助動詞.ゴトシ.連体"	} //  213
		,{	 0x9000|0x0008|0x0100 	,  "助動詞.ゴトシ.連用"	} //  214
		,{	 0x9000|0x001c|0x0b00 	,  "助動詞.サセル.仮定"	} //  215
		,{	 0x9000|0x001c|0x0600 	,  "助動詞.サセル.命令,ヨ"	} //  216
		,{	 0x9000|0x001c|0x0600 	,  "助動詞.サセル.命令,ロ"	} //  217
		,{	 0x9000|0x001c|0x0900 	,  "助動詞.サセル.終止連体"	} //  218
		,{	 0x9000|0x001c|0x0100 	,  "助動詞.サセル.連用"	} //  219
		,{	 0x9000|0x001e|0x0b00 	,  "助動詞.シメル.仮定"	} //  220
		,{	 0x9000|0x001e|0x0600 	,  "助動詞.シメル.命令,ヨ"	} //  221
		,{	 0x9000|0x001e|0x0600 	,  "助動詞.シメル.命令,ロ"	} //  222
		,{	 0x9000|0x001e|0x0900 	,  "助動詞.シメル.終止連体"	} //  223
		,{	 0x9000|0x001e|0x0100 	,  "助動詞.シメル.連用"	} //  224
		,{	 0x9000|0x0029|0x0b00 	,  "助動詞.ズ.仮定"	} //  225
		,{	 0x9000|0x0029|0x0b00 	,  "助動詞.ズ.仮定,文語"	} //  226
		,{	 0x9000|0x0029|0x0100 	,  "助動詞.ズ.仮定,文語.音便"	} //  227
		,{	 0x9000|0x0029|0x0900 	,  "助動詞.ズ.終止連体"	} //  228
		,{	 0x9000|0x0029|0x0900 	,  "助動詞.ズ.終止連体,音便"	} //  229
		,{	 0x9000|0x0029|0x0100 	,  "助動詞.ズ.終止連用"	} //  230
		,{	 0x9000|0x0029|0x0800 	,  "助動詞.ズ.連体"	} //  231
		,{	 0x9000|0x001d|0x0400 	,  "助動詞.セル.ウ接続"	} //  232
		,{	 0x9000|0x001d|0x0b00 	,  "助動詞.セル.仮定"	} //  233
		,{	 0x9000|0x001d|0x0600 	,  "助動詞.セル.命令,ヨ"	} //  234
		,{	 0x9000|0x001d|0x0600 	,  "助動詞.セル.命令,ロ"	} //  235
		,{	 0x9000|0x001d|0x0400 	,  "助動詞.セル.未然"	} //  236
		,{	 0x9000|0x001d|0x0300 	,  "助動詞.セル.未然連用仮定"	} //  237
		,{	 0x9000|0x001d|0x0900 	,  "助動詞.セル.終止連体"	} //  238
		,{	 0x9000|0x001d|0x0900 	,  "助動詞.セル.終止連体,ス"	} //  239
		,{	 0x9000|0x001f|0x0a00 	,  "助動詞.ソウダ伝.終止"	} //  240
		,{	 0x9000|0x001f|0x0a00 	,  "助動詞.ソウダ伝.終止,文語"	} //  241
		,{	 0x9000|0x001f|0x0c00 	,  "助動詞.ソウダ伝.語幹"	} //  242
		,{	 0x9000|0x001f|0x0100 	,  "助動詞.ソウダ伝.連用"	} //  243
		,{	 0x9000|0x0020|0x0400 	,  "助動詞.ソウダ推.ウ接続"	} //  244
		,{	 0x9000|0x0020|0x0300 	,  "助動詞.ソウダ推.未然連用"	} //  245
		,{	 0x9000|0x0020|0x0a00 	,  "助動詞.ソウダ推.終止"	} //  246
		,{	 0x9000|0x0020|0x0c00 	,  "助動詞.ソウダ推.語幹"	} //  247
		,{	 0x9000|0x0020|0x0800 	,  "助動詞.ソウダ推.連体"	} //  248
		,{	 0x9000|0x0020|0x0100 	,  "助動詞.ソウダ推.連用"	} //  249
		,{	 0x9000|0x0020|0x0100 	,  "助動詞.ソウダ推.音便"	} //  250
		,{	 0x9000|0x0022|0x0400 	,  "助動詞.タ.ウ接続"	} //  251
		,{	 0x9000|0x0022|0x0400 	,  "助動詞.タ.ウ接続,濁音"	} //  252
		,{	 0x9000|0x0022|0x0b00 	,  "助動詞.タ.仮定"	} //  253
		,{	 0x9000|0x0022|0x0b00 	,  "助動詞.タ.仮定,濁音"	} //  254
		,{	 0x9000|0x0022|0x0900 	,  "助動詞.タ.終止連体"	} //  255
		,{	 0x9000|0x0022|0x0900 	,  "助動詞.タ.終止連体,濁音"	} //  256
		,{	 0x9000|0x0023|0x0400 	,  "助動詞.タイ.ウ接続"	} //  257
		,{	 0x9000|0x0023|0x0b00 	,  "助動詞.タイ.仮定"	} //  258
		,{	 0x9000|0x0023|0x0b00 	,  "助動詞.タイ.仮定,口語"	} //  259
		,{	 0x9000|0x0023|0x0b00 	,  "助動詞.タイ.仮定ケ,口語"	} //  260
		,{	 0x9000|0x0023|0x0a00 	,  "助動詞.タイ.終止,シ"	} //  261
		,{	 0x9000|0x0023|0x0900 	,  "助動詞.タイ.終止連体"	} //  262
		,{	 0x9000|0x0023|0x0c00 	,  "助動詞.タイ.語幹"	} //  263
		,{	 0x9000|0x0023|0x0800 	,  "助動詞.タイ.連体,文語"	} //  264
		,{	 0x9000|0x0023|0x0100 	,  "助動詞.タイ.連用"	} //  265
		,{	 0x9000|0x0023|0x0100 	,  "助動詞.タイ.音便"	} //  266
		,{	 0x9000|0x0024|0x0700 	,  "助動詞.タリ.仮定命令"	} //  267
		,{	 0x9000|0x0024|0x0400 	,  "助動詞.タリ.未然"	} //  268
		,{	 0x9000|0x0024|0x0100 	,  "助動詞.タリ.終止連用"	} //  269
		,{	 0x9000|0x0024|0x0800 	,  "助動詞.タリ.連体"	} //  270
		,{	 0x9000|0x0024|0x0100 	,  "助動詞.タリ.連用"	} //  271
		,{	 0x9000|0x0002|0x0400 	,  "助動詞.ダ.ウ接続"	} //  272
		,{	 0x9000|0x0002|0x0b00 	,  "助動詞.ダ.仮定"	} //  273
		,{	 0x9000|0x0002|0x0a00 	,  "助動詞.ダ.終止"	} //  274
		,{	 0x9000|0x0002|0x0800 	,  "助動詞.ダ.連体"	} //  275
		,{	 0x9000|0x0002|0x0800 	,  "助動詞.ダ.連体,文語"	} //  276
		,{	 0x9000|0x0002|0x0100 	,  "助動詞.ダ.連用"	} //  277
		,{	 0x9000|0x0002|0x0100 	,  "助動詞.ダ.連用,ニ"	} //  278
		,{	 0x9000|0x0002|0x0100 	,  "助動詞.ダ.音便"	} //  279
		,{	 0x9000|0x0005|0x0400 	,  "助動詞.デス.ウ接続"	} //  280
		,{	 0x9000|0x0005|0x0a00 	,  "助動詞.デス.終止"	} //  281
		,{	 0x9000|0x0005|0x0100 	,  "助動詞.デス.連用"	} //  282
		,{	 0x9000|0x0011|0x0400 	,  "助動詞.ナイ.ウ接続"	} //  283
		,{	 0x9000|0x0011|0x0400 	,  "助動詞.ナイ.ソウ接続"	} //  284
		,{	 0x9000|0x0011|0x0b00 	,  "助動詞.ナイ.仮定"	} //  285
		,{	 0x9000|0x0011|0x0b00 	,  "助動詞.ナイ.仮定,口語.キ"	} //  286
		,{	 0x9000|0x0011|0x0b00 	,  "助動詞.ナイ.仮定,口語.ケ"	} //  287
		,{	 0x9000|0x0011|0x0900 	,  "助動詞.ナイ.終止連体"	} //  288
		,{	 0x9000|0x0011|0x0c00 	,  "助動詞.ナイ.語幹"	} //  289
		,{	 0x9000|0x0011|0x0800 	,  "助動詞.ナイ.連体,文語"	} //  290
		,{	 0x9000|0x0011|0x0100 	,  "助動詞.ナイ.連用"	} //  291
		,{	 0x9000|0x0011|0x0100 	,  "助動詞.ナイ.音便"	} //  292
		,{	 0x9000|0x002d|0x0400 	,  "助動詞.ベシ.未然"	} //  293
		,{	 0x9000|0x002d|0x0a00 	,  "助動詞.ベシ.終止"	} //  294
		,{	 0x9000|0x002d|0x0800 	,  "助動詞.ベシ.連体"	} //  295
		,{	 0x9000|0x002d|0x0100 	,  "助動詞.ベシ.連用"	} //  296
		,{	 0x9000|0x000d|0x0a00 	,  "助動詞.マイ.終止"	} //  297
		,{	 0x9000|0x000e|0x0400 	,  "助動詞.マス.ウ接続"	} //  298
		,{	 0x9000|0x000e 	,  "助動詞.マス.ン接続"	} //  299
		,{	 0x9000|0x000e|0x0b00 	,  "助動詞.マス.仮定"	} //  300
		,{	 0x9000|0x000e|0x0900 	,  "助動詞.マス.終止連体"	} //  301
		,{	 0x9000|0x000e|0x0100 	,  "助動詞.マス.連用"	} //  302
		,{	 0x9000|0x000f|0x0400 	,  "助動詞.ミタイダ.ウ接続"	} //  303
		,{	 0x9000|0x000f|0x0300 	,  "助動詞.ミタイダ.未然連用"	} //  304
		,{	 0x9000|0x000f|0x0a00 	,  "助動詞.ミタイダ.終止"	} //  305
		,{	 0x9000|0x000f|0x0c00 	,  "助動詞.ミタイダ.語幹"	} //  306
		,{	 0x9000|0x000f|0x0800 	,  "助動詞.ミタイダ.連体"	} //  307
		,{	 0x9000|0x000f|0x0100 	,  "助動詞.ミタイダ.連用"	} //  308
		,{	 0x9000|0x000f|0x0100 	,  "助動詞.ミタイダ.音便"	} //  309
		,{	 0x9000|0x0028|0x0a00 	,  "助動詞.ヨウ.終止"	} //  310
		,{	 0x9000|0x0000 	,  "助動詞.ラシイ.ヌ接続"	} //  311
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.ラシイ.仮定"	} //  312
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.ラシイ.仮定,口語"	} //  313
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.ラシイ.仮定,口語.ケ"	} //  314
		,{	 0x9000|0x0000|0x0900 	,  "助動詞.ラシイ.終止連体"	} //  315
		,{	 0x9000|0x0000|0x0c00 	,  "助動詞.ラシイ.語幹"	} //  316
		,{	 0x9000|0x0000|0x0800 	,  "助動詞.ラシイ.連体"	} //  317
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.ラシイ.連用"	} //  318
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.ラシイ.連用.文語"	} //  319
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.ラシイ.音便"	} //  320
		,{	 0x9000|0x0018|0x0b00 	,  "助動詞.ラレル.仮定"	} //  321
		,{	 0x9000|0x0018|0x0600 	,  "助動詞.ラレル.命令,ヨ"	} //  322
		,{	 0x9000|0x0018|0x0600 	,  "助動詞.ラレル.命令,ロ"	} //  323
		,{	 0x9000|0x0018|0x0900 	,  "助動詞.ラレル.終止連体"	} //  324
		,{	 0x9000|0x0018|0x0100 	,  "助動詞.ラレル.連用"	} //  325
		,{	 0x9000|0x001a|0x0b00 	,  "助動詞.ル.仮定"	} //  326
		,{	 0x9000|0x001a|0x0900 	,  "助動詞.ル.終止連体"	} //  327
		,{	 0x9000|0x0019|0x0b00 	,  "助動詞.レル.仮定"	} //  328
		,{	 0x9000|0x0019|0x0600 	,  "助動詞.レル.命令,ヨ"	} //  329
		,{	 0x9000|0x0019|0x0600 	,  "助動詞.レル.命令,ロ"	} //  330
		,{	 0x9000|0x0019|0x0900 	,  "助動詞.レル.終止連体"	} //  331
		,{	 0x9000|0x0019|0x0100 	,  "助動詞.レル.連用"	} //  332
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.ウ接続;副助詞.カ＊助動詞.ナイ.ウ接続"	} //  333
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.ウ接続;助動詞.ズ.連体＊助動詞.ナイ.ウ接続"	} //  334
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.ウ接続;助動詞.ダ.連用＊助動詞.ナイ.ウ接続"	} //  335
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.ウ接続;助動詞.ダ.連用＊補助動詞.ウ接続,ラ行五段"	} //  336
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.ウ接続;接続助詞.テ,濁音＊助動詞.ナイ.ウ接続"	} //  337
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.ウ接続;接続助詞.テ＊助動詞.ナイ.ウ接続"	} //  338
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.ウ接続;接続助詞.テ＊補助動詞.ウ接続,カ行五段"	} //  339
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.ウ接続;接続助詞.テ＊補助動詞.ウ接続,カ行五段促音"	} //  340
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.ウ接続;接続助詞.テ＊補助動詞.ウ接続,ラ行五段"	} //  341
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.ウ接続;接続助詞.ト＊補助形容詞.ウ接続,ヨイ"	} //  342
		,{	 0x9000|0x0000 	,  "助動詞.相当.ズ接続;接続助詞.ト＊補助形容詞.ズ接続,ヨイ"	} //  343
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.ソウ接続;副助詞.カ＊助動詞.ナイ.ソウ接続"	} //  344
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.ソウ接続;助動詞.ズ.連体＊助動詞.ナイ.ソウ接続"	} //  345
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.ソウ接続;助動詞.ダ.連用＊助動詞.ナイ.ソウ接続"	} //  346
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.ソウ接続;接続助詞.テ,濁音＊助動詞.ナイ.ソウ接続"	} //  347
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.ソウ接続;接続助詞.テ＊助動詞.ナイ.ソウ接続"	} //  348
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.ソウ接続;接続助詞.ト＊補助形容詞.ソウ接続,ヨイ"	} //  349
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;副助詞.カ＊助動詞.ナイ.仮定"	} //  350
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;副助詞.カ＊助動詞.ナイ.仮定,口語.キ"	} //  351
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;副助詞.カ＊助動詞.ナイ.仮定,口語.ケ"	} //  352
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;助動詞.ズ.仮定＊助動詞.ズ.仮定"	} //  353
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;助動詞.ズ.仮定＊助動詞.ズ.仮定,文語"	} //  354
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;助動詞.ズ.仮定＊助動詞.ズ.仮定,文語.音便"	} //  355
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;助動詞.ズ.連体＊助動詞.ナイ.仮定"	} //  356
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;助動詞.ズ.連体＊助動詞.ナイ.仮定,口語.キ"	} //  357
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;助動詞.ズ.連体＊助動詞.ナイ.仮定,口語.ケ"	} //  358
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;助動詞.ダ.連用＊助動詞.ナイ.仮定"	} //  359
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;助動詞.ダ.連用＊助動詞.ナイ.仮定,口語.キ"	} //  360
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;助動詞.ダ.連用＊助動詞.ナイ.仮定,口語.ケ"	} //  361
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;助動詞.ダ.連用＊補助動詞.仮定,ラ行五段.口語"	} //  362
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;接続助詞.テ,濁音＊助動詞.ナイ.仮定"	} //  363
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;接続助詞.テ,濁音＊助動詞.ナイ.仮定,口語.キ"	} //  364
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;接続助詞.テ,濁音＊助動詞.ナイ.仮定,口語.ケ"	} //  365
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;接続助詞.テ,濁音＊補助動詞.仮定,一段"	} //  366
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;接続助詞.テ＊助動詞.ナイ.仮定"	} //  367
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;接続助詞.テ＊助動詞.ナイ.仮定,口語.キ"	} //  368
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;接続助詞.テ＊助動詞.ナイ.仮定,口語.ケ"	} //  369
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;接続助詞.テ＊補助動詞.仮定,カ変"	} //  370
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;接続助詞.テ＊補助動詞.仮定,カ行五段.口語"	} //  371
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;接続助詞.テ＊補助動詞.仮定,カ行五段促音.口語"	} //  372
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;接続助詞.テ＊補助動詞.仮定,ラ行五段.口語"	} //  373
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;接続助詞.テ＊補助動詞.仮定,一段"	} //  374
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;接続助詞.ト＊補助形容詞.仮定,ヨイ"	} //  375
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;接続助詞.ト＊補助形容詞.仮定,ヨイ.口語.キ"	} //  376
		,{	 0x9000|0x0000|0x0b00 	,  "助動詞.相当.仮定;接続助詞.ト＊補助形容詞.仮定,ヨイ.口語.ケ"	} //  377
		,{	 0x9000|0x0000|0x0700 	,  "助動詞.相当.仮定命令;助動詞.ダ.連用＊補助動詞.仮定命令,ラ行五段"	} //  378
		,{	 0x9000|0x0000|0x0700 	,  "助動詞.相当.仮定命令;接続助詞.テ＊補助動詞.仮定命令,カ行五段"	} //  379
		,{	 0x9000|0x0000|0x0700 	,  "助動詞.相当.仮定命令;接続助詞.テ＊補助動詞.仮定命令,カ行五段促音"	} //  380
		,{	 0x9000|0x0000|0x0700 	,  "助動詞.相当.仮定命令;接続助詞.テ＊補助動詞.仮定命令,ラ行五段"	} //  381
		,{	 0x9000|0x0000|0x0600 	,  "助動詞.相当.命令;接続助詞.テ,濁音＊補助動詞.命令,一段.ヨ"	} //  382
		,{	 0x9000|0x0000|0x0600 	,  "助動詞.相当.命令;接続助詞.テ,濁音＊補助動詞.命令,一段.ロ"	} //  383
		,{	 0x9000|0x0000|0x0600 	,  "助動詞.相当.命令;接続助詞.テ＊補助動詞.命令,カ変"	} //  384
		,{	 0x9000|0x0000|0x0600 	,  "助動詞.相当.命令;接続助詞.テ＊補助動詞.命令,一段.ヨ"	} //  385
		,{	 0x9000|0x0000|0x0600 	,  "助動詞.相当.命令;接続助詞.テ＊補助動詞.命令,一段.ロ"	} //  386
		,{	 0x9000|0x0000|0x0600 	,  "助動詞.相当.命令;接続助詞.ト＊補助形容詞.命令,ヨイ.文語"	} //  387
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.未然;助動詞.ダ.連用＊補助動詞.未然,ラ行五段"	} //  388
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.未然;接続助詞.テ＊補助動詞.未然,カ変"	} //  389
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.未然;接続助詞.テ＊補助動詞.未然,カ行五段"	} //  390
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.未然;接続助詞.テ＊補助動詞.未然,カ行五段促音"	} //  391
		,{	 0x9000|0x0000|0x0400 	,  "助動詞.相当.未然;接続助詞.テ＊補助動詞.未然,ラ行五段"	} //  392
		,{	 0x9000|0x0000|0x0300 	,  "助動詞.相当.未然連用;接続助詞.ト＊補助形容詞.未然連用,ヨイ"	} //  393
		,{	 0x9000|0x0000|0x0900 	,  "助動詞.相当.終止連体;副助詞.カ＊助動詞.ナイ.終止連体"	} //  394
		,{	 0x9000|0x0000|0x0900 	,  "助動詞.相当.終止連体;助動詞.ズ.仮定＊助動詞.ズ.終止連体"	} //  395
		,{	 0x9000|0x0000|0x0900 	,  "助動詞.相当.終止連体;助動詞.ズ.仮定＊助動詞.ズ.終止連体,音便"	} //  396
		,{	 0x9000|0x0000|0x0900 	,  "助動詞.相当.終止連体;助動詞.ズ.連体＊助動詞.ナイ.終止連体"	} //  397
		,{	 0x9000|0x0000|0x0900 	,  "助動詞.相当.終止連体;助動詞.ダ.連用＊助動詞.ナイ.終止連体"	} //  398
		,{	 0x9000|0x0000|0x0900 	,  "助動詞.相当.終止連体;助動詞.ダ.連用＊補助動詞.終止連体,ラ行五段"	} //  399
		,{	 0x9000|0x0000|0x0900 	,  "助動詞.相当.終止連体;接続助詞.テ,濁音＊助動詞.ナイ.終止連体"	} //  400
		,{	 0x9000|0x0000|0x0900 	,  "助動詞.相当.終止連体;接続助詞.テ,濁音＊補助動詞.終止連体,一段"	} //  401
		,{	 0x9000|0x0000|0x0900 	,  "助動詞.相当.終止連体;接続助詞.テ＊助動詞.ナイ.終止連体"	} //  402
		,{	 0x9000|0x0000|0x0900 	,  "助動詞.相当.終止連体;接続助詞.テ＊補助動詞.終止連体,カ変"	} //  403
		,{	 0x9000|0x0000|0x0900 	,  "助動詞.相当.終止連体;接続助詞.テ＊補助動詞.終止連体,カ行五段"	} //  404
		,{	 0x9000|0x0000|0x0900 	,  "助動詞.相当.終止連体;接続助詞.テ＊補助動詞.終止連体,カ行五段促音"	} //  405
		,{	 0x9000|0x0000|0x0900 	,  "助動詞.相当.終止連体;接続助詞.テ＊補助動詞.終止連体,ラ行五段"	} //  406
		,{	 0x9000|0x0000|0x0900 	,  "助動詞.相当.終止連体;接続助詞.テ＊補助動詞.終止連体,一段"	} //  407
		,{	 0x9000|0x0000|0x0900 	,  "助動詞.相当.終止連体;接続助詞.ト＊補助形容詞.終止連体,ヨイ"	} //  408
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.終止連用;助動詞.ズ.仮定＊助動詞.ズ.終止連用"	} //  409
		,{	 0x9000|0x0000|0x0c00 	,  "助動詞.相当.語幹;副助詞.カ＊助動詞.ナイ.語幹"	} //  410
		,{	 0x9000|0x0000|0x0c00 	,  "助動詞.相当.語幹;助動詞.ズ.連体＊助動詞.ナイ.語幹"	} //  411
		,{	 0x9000|0x0000|0x0c00 	,  "助動詞.相当.語幹;助動詞.ダ.連用＊助動詞.ナイ.語幹"	} //  412
		,{	 0x9000|0x0000|0x0c00 	,  "助動詞.相当.語幹;接続助詞.テ,濁音＊助動詞.ナイ.語幹"	} //  413
		,{	 0x9000|0x0000|0x0c00 	,  "助動詞.相当.語幹;接続助詞.テ＊助動詞.ナイ.語幹"	} //  414
		,{	 0x9000|0x0000|0x0c00 	,  "助動詞.相当.語幹;接続助詞.ト＊補助形容詞.語幹,ヨイ"	} //  415
		,{	 0x9000|0x0000|0x0800 	,  "助動詞.相当.連体;副助詞.カ＊助動詞.ナイ.連体,文語"	} //  416
		,{	 0x9000|0x0000|0x0800 	,  "助動詞.相当.連体;助動詞.ズ.仮定＊助動詞.ズ.連体"	} //  417
		,{	 0x9000|0x0000|0x0800 	,  "助動詞.相当.連体;助動詞.ズ.連体＊助動詞.ナイ.連体,文語"	} //  418
		,{	 0x9000|0x0000|0x0800 	,  "助動詞.相当.連体;助動詞.ダ.連用＊助動詞.ナイ.連体,文語"	} //  419
		,{	 0x9000|0x0000|0x0800 	,  "助動詞.相当.連体;接続助詞.テ,濁音＊助動詞.ナイ.連体,文語"	} //  420
		,{	 0x9000|0x0000|0x0800 	,  "助動詞.相当.連体;接続助詞.テ＊助動詞.ナイ.連体,文語"	} //  421
		,{	 0x9000|0x0000|0x0800 	,  "助動詞.相当.連体;接続助詞.ト＊補助形容詞.連体,ヨイ.文語.キ"	} //  422
		,{	 0x9000|0x0000|0x0800 	,  "助動詞.相当.連体;接続助詞.ト＊補助形容詞.連体,ヨイ.文語.シ"	} //  423
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.連用;副助詞.カ＊助動詞.ナイ.連用"	} //  424
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.連用;助動詞.ズ.連体＊助動詞.ナイ.連用"	} //  425
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.連用;助動詞.ダ.連用＊助動詞.ナイ.連用"	} //  426
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.連用;助動詞.ダ.連用＊補助動詞.連用,ラ行五段"	} //  427
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.連用;接続助詞.テ,濁音＊助動詞.ナイ.連用"	} //  428
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.連用;接続助詞.テ,濁音＊補助動詞.連用,一段"	} //  429
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.連用;接続助詞.テ＊助動詞.ナイ.連用"	} //  430
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.連用;接続助詞.テ＊補助動詞.連用,カ変"	} //  431
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.連用;接続助詞.テ＊補助動詞.連用,カ行五段"	} //  432
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.連用;接続助詞.テ＊補助動詞.連用,カ行五段促音"	} //  433
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.連用;接続助詞.テ＊補助動詞.連用,ラ行五段"	} //  434
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.連用;接続助詞.テ＊補助動詞.連用,一段"	} //  435
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.音便;副助詞.カ＊助動詞.ナイ.音便"	} //  436
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.音便;助動詞.ズ.連体＊助動詞.ナイ.音便"	} //  437
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.音便;助動詞.ダ.連用＊助動詞.ナイ.音便"	} //  438
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.音便;助動詞.ダ.連用＊補助動詞.音便,ラ行五段"	} //  439
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.音便;接続助詞.テ,濁音＊助動詞.ナイ.音便"	} //  440
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.音便;接続助詞.テ＊助動詞.ナイ.音便"	} //  441
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.音便;接続助詞.テ＊補助動詞.音便,カ行五段"	} //  442
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.音便;接続助詞.テ＊補助動詞.音便,カ行五段促音"	} //  443
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.音便;接続助詞.テ＊補助動詞.音便,ラ行五段"	} //  444
		,{	 0x9000|0x0000|0x0100 	,  "助動詞.相当.音便;接続助詞.ト＊補助形容詞.音便,ヨイ"	} //  445
		,{	 0x7000|0x0020|0x0400 	,  "動詞.ウ接続,カ行五段促音"	} //  446
		,{	 0x7000|0x0005|0x0400 	,  "動詞.ウ接続,サ変一字"	} //  447
		,{	 0x7000|0x0020|0x0400 	,  "動詞.ウ接続,ラ行五段イ"	} //  448
		,{	 0x7000|0x0020|0x0400 	,  "動詞.ウ接続,ワ行五段ウ"	} //  449
		,{	 0x7000|0x0001|0x0400 	,  "動詞.ウ接続;動詞.終止連体,カ行五段＊動詞.ウ接続,カ行五段"	} //  450
		,{	 0x7000|0x0001|0x0400 	,  "動詞.ウ接続;動詞.終止連体,ガ行五段＊動詞.ウ接続,ガ行五段"	} //  451
		,{	 0x7000|0x0001|0x0400 	,  "動詞.ウ接続;動詞.終止連体,サ行五段＊動詞.ウ接続,サ行五段"	} //  452
		,{	 0x7000|0x0001|0x0400 	,  "動詞.ウ接続;動詞.終止連体,タ行五段＊動詞.ウ接続,タ行五段"	} //  453
		,{	 0x7000|0x0001|0x0400 	,  "動詞.ウ接続;動詞.終止連体,バ行五段＊動詞.ウ接続,バ行五段"	} //  454
		,{	 0x7000|0x0001|0x0400 	,  "動詞.ウ接続;動詞.終止連体,マ行五段＊動詞.ウ接続,マ行五段"	} //  455
		,{	 0x7000|0x0001|0x0400 	,  "動詞.ウ接続;動詞.終止連体,ラ行五段＊動詞.ウ接続,ラ行五段"	} //  456
		,{	 0x7000|0x0001|0x0400 	,  "動詞.ウ接続;動詞.終止連体,ワ行五段＊動詞.ウ接続,ワ行五段"	} //  457
		,{	 0x7000|0x0005|0x0400 	,  "動詞.ズ接続,サ変"	} //  458
		,{	 0x7000|0x0005|0x0400 	,  "動詞.ズ接続,ザ変一字"	} //  459
		,{	 0x7000|0x0005|0x0400 	,  "動詞.レル接続,サ変"	} //  460
		,{	 0x7000|0x0001|0x0b00 	,  "動詞.仮定,カ変"	} //  461
		,{	 0x7000|0x0020|0x0b00 	,  "動詞.仮定,カ行五段.口語"	} //  462
		,{	 0x7000|0x0020|0x0b00 	,  "動詞.仮定,カ行五段促音.口語"	} //  463
		,{	 0x7000|0x0020|0x0b00 	,  "動詞.仮定,ガ行五段.口語"	} //  464
		,{	 0x7000|0x0005|0x0b00 	,  "動詞.仮定,サ変"	} //  465
		,{	 0x7000|0x0005|0x0b00 	,  "動詞.仮定,サ変一字"	} //  466
		,{	 0x7000|0x0020|0x0b00 	,  "動詞.仮定,サ行五段.口語"	} //  467
		,{	 0x7000|0x0005|0x0b00 	,  "動詞.仮定,ザ変一字.ジ"	} //  468
		,{	 0x7000|0x0005|0x0b00 	,  "動詞.仮定,ザ変一字.ズ"	} //  469
		,{	 0x7000|0x0020|0x0b00 	,  "動詞.仮定,タ行五段.口語"	} //  470
		,{	 0x7000|0x0020|0x0b00 	,  "動詞.仮定,ナ行五段.口語"	} //  471
		,{	 0x7000|0x0020|0x0b00 	,  "動詞.仮定,バ行五段.口語"	} //  472
		,{	 0x7000|0x0020|0x0b00 	,  "動詞.仮定,マ行五段.口語"	} //  473
		,{	 0x7000|0x0020|0x0b00 	,  "動詞.仮定,ラ行五段.口語"	} //  474
		,{	 0x7000|0x0020|0x0b00 	,  "動詞.仮定,ラ行五段イ.口語"	} //  475
		,{	 0x7000|0x0020|0x0b00 	,  "動詞.仮定,ワ行五段.口語"	} //  476
		,{	 0x7000|0x0020|0x0b00 	,  "動詞.仮定,ワ行五段ウ.口語"	} //  477
		,{	 0x7000|0x0010|0x0b00 	,  "動詞.仮定,一段;＝可能形"	} //  478
		,{	 0x7000|0x0001|0x0b00 	,  "動詞.仮定;動詞.終止連体,カ行五段＊動詞.仮定,カ行五段.口語"	} //  479
		,{	 0x7000|0x0001|0x0b00 	,  "動詞.仮定;動詞.終止連体,ガ行五段＊動詞.仮定,ガ行五段.口語"	} //  480
		,{	 0x7000|0x0001|0x0b00 	,  "動詞.仮定;動詞.終止連体,サ行五段＊動詞.仮定,サ行五段.口語"	} //  481
		,{	 0x7000|0x0001|0x0b00 	,  "動詞.仮定;動詞.終止連体,タ行五段＊動詞.仮定,タ行五段.口語"	} //  482
		,{	 0x7000|0x0001|0x0b00 	,  "動詞.仮定;動詞.終止連体,バ行五段＊動詞.仮定,バ行五段.口語"	} //  483
		,{	 0x7000|0x0001|0x0b00 	,  "動詞.仮定;動詞.終止連体,マ行五段＊動詞.仮定,マ行五段.口語"	} //  484
		,{	 0x7000|0x0001|0x0b00 	,  "動詞.仮定;動詞.終止連体,ラ行五段＊動詞.仮定,ラ行五段.口語"	} //  485
		,{	 0x7000|0x0001|0x0b00 	,  "動詞.仮定;動詞.終止連体,ワ行五段＊動詞.仮定,ワ行五段.口語"	} //  486
		,{	 0x7000|0x0001|0x0b00 	,  "動詞.仮定;動詞.終止連体,一段＊動詞.仮定,一段"	} //  487
		,{	 0x7000|0x0001|0x0b00 	,  "動詞.仮定;動詞.連用,ラ行五段＊補助動詞.仮定,一段"	} //  488
		,{	 0x7000|0x0001|0x0b00 	,  "動詞.仮定;動詞.連用,一段＊補助動詞.仮定,一段"	} //  489
		,{	 0x7000|0x0001|0x0b00 	,  "動詞.仮定;名詞.一般＊動詞.仮定,一段"	} //  490
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,カ行五段;＝可能形"	} //  491
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,カ行五段促音"	} //  492
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,カ行五段促音;＝可能形"	} //  493
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,ガ行五段;＝可能形"	} //  494
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,サ行五段;＝可能形"	} //  495
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,タ行五段;＝可能形"	} //  496
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,ナ行五段;＝可能形"	} //  497
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,バ行五段;＝可能形"	} //  498
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,マ行五段;＝可能形"	} //  499
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,ラ行五段;＝可能形"	} //  500
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,ラ行五段イ"	} //  501
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,ワ行五段;＝可能形"	} //  502
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,ワ行五段ウ"	} //  503
		,{	 0x7000|0x0020|0x0700 	,  "動詞.仮定命令,ワ行五段ウ;＝可能形"	} //  504
		,{	 0x7000|0x0001|0x0700 	,  "動詞.仮定命令;動詞.終止連体,カ行五段＊動詞.仮定命令,カ行五段"	} //  505
		,{	 0x7000|0x0001|0x0700 	,  "動詞.仮定命令;動詞.終止連体,カ行五段＊動詞.仮定命令,カ行五段;＝可能形"	} //  506
		,{	 0x7000|0x0001|0x0700 	,  "動詞.仮定命令;動詞.終止連体,ガ行五段＊動詞.仮定命令,ガ行五段;＝可能形"	} //  507
		,{	 0x7000|0x0001|0x0700 	,  "動詞.仮定命令;動詞.終止連体,サ行五段＊動詞.仮定命令,サ行五段"	} //  508
		,{	 0x7000|0x0001|0x0700 	,  "動詞.仮定命令;動詞.終止連体,サ行五段＊動詞.仮定命令,サ行五段;＝可能形"	} //  509
		,{	 0x7000|0x0001|0x0700 	,  "動詞.仮定命令;動詞.終止連体,タ行五段＊動詞.仮定命令,タ行五段"	} //  510
		,{	 0x7000|0x0001|0x0700 	,  "動詞.仮定命令;動詞.終止連体,バ行五段＊動詞.仮定命令,バ行五段"	} //  511
		,{	 0x7000|0x0001|0x0700 	,  "動詞.仮定命令;動詞.終止連体,マ行五段＊動詞.仮定命令,マ行五段"	} //  512
		,{	 0x7000|0x0001|0x0700 	,  "動詞.仮定命令;動詞.終止連体,マ行五段＊動詞.仮定命令,マ行五段;＝可能形"	} //  513
		,{	 0x7000|0x0001|0x0700 	,  "動詞.仮定命令;動詞.終止連体,ラ行五段＊動詞.仮定命令,ラ行五段"	} //  514
		,{	 0x7000|0x0001|0x0700 	,  "動詞.仮定命令;動詞.終止連体,ラ行五段＊動詞.仮定命令,ラ行五段;＝可能形"	} //  515
		,{	 0x7000|0x0001|0x0700 	,  "動詞.仮定命令;動詞.終止連体,ワ行五段＊動詞.仮定命令,ワ行五段"	} //  516
		,{	 0x7000|0x0001|0x0700 	,  "動詞.仮定命令;動詞.終止連体,ワ行五段＊動詞.仮定命令,ワ行五段;＝可能形"	} //  517
		,{	 0x7000|0x0001|0x0600 	,  "動詞.命令,カ変"	} //  518
		,{	 0x7000|0x0005|0x0600 	,  "動詞.命令,サ変.ヨ"	} //  519
		,{	 0x7000|0x0005|0x0600 	,  "動詞.命令,サ変.ロ"	} //  520
		,{	 0x7000|0x0005|0x0600 	,  "動詞.命令,サ変一字.セ"	} //  521
		,{	 0x7000|0x0005|0x0600 	,  "動詞.命令,サ変一字.ヨ"	} //  522
		,{	 0x7000|0x0005|0x0600 	,  "動詞.命令,サ変一字.ロ"	} //  523
		,{	 0x7000|0x0005|0x0600 	,  "動詞.命令,ザ変一字.ヨ"	} //  524
		,{	 0x7000|0x0005|0x0600 	,  "動詞.命令,ザ変一字.ロ"	} //  525
		,{	 0x7000|0x0020|0x0600 	,  "動詞.命令,ラ行五段イ"	} //  526
		,{	 0x7000|0x0001|0x0600 	,  "動詞.命令;動詞.終止連体,一段＊動詞.命令,一段.ヨ"	} //  527
		,{	 0x7000|0x0001|0x0600 	,  "動詞.命令;動詞.終止連体,一段＊動詞.命令,一段.ロ"	} //  528
		,{	 0x7000|0x0001|0x0600 	,  "動詞.命令;動詞.連用,ラ行五段＊補助動詞.命令,一段.ヨ"	} //  529
		,{	 0x7000|0x0001|0x0600 	,  "動詞.命令;動詞.連用,ラ行五段＊補助動詞.命令,一段.ロ"	} //  530
		,{	 0x7000|0x0001|0x0600 	,  "動詞.命令;動詞.連用,一段＊補助動詞.命令,一段.ヨ"	} //  531
		,{	 0x7000|0x0001|0x0600 	,  "動詞.命令;動詞.連用,一段＊補助動詞.命令,一段.ロ"	} //  532
		,{	 0x7000|0x0001|0x0600 	,  "動詞.命令;名詞.一般＊動詞.命令,一段.ヨ"	} //  533
		,{	 0x7000|0x0001|0x0600 	,  "動詞.命令;名詞.一般＊動詞.命令,一段.ロ"	} //  534
		,{	 0x7000|0x0001|0x0400 	,  "動詞.未然,カ変"	} //  535
		,{	 0x7000|0x0020|0x0400 	,  "動詞.未然,カ行五段;！助動詞れる"	} //  536
		,{	 0x7000|0x0020|0x0400 	,  "動詞.未然,カ行五段促音"	} //  537
		,{	 0x7000|0x0005|0x0400 	,  "動詞.未然,サ変一字"	} //  538
		,{	 0x7000|0x0020|0x0400 	,  "動詞.未然,ラ行五段イ"	} //  539
		,{	 0x7000|0x0020|0x0400 	,  "動詞.未然,ワ行五段ウ"	} //  540
		,{	 0x7000|0x0001|0x0400 	,  "動詞.未然;動詞.終止連体,カ行五段＊動詞.未然,カ行五段"	} //  541
		,{	 0x7000|0x0001|0x0400 	,  "動詞.未然;動詞.終止連体,ガ行五段＊動詞.未然,ガ行五段"	} //  542
		,{	 0x7000|0x0001|0x0400 	,  "動詞.未然;動詞.終止連体,サ行五段＊動詞.未然,サ行五段"	} //  543
		,{	 0x7000|0x0001|0x0400 	,  "動詞.未然;動詞.終止連体,タ行五段＊動詞.未然,タ行五段"	} //  544
		,{	 0x7000|0x0001|0x0400 	,  "動詞.未然;動詞.終止連体,バ行五段＊動詞.未然,バ行五段"	} //  545
		,{	 0x7000|0x0001|0x0400 	,  "動詞.未然;動詞.終止連体,マ行五段＊動詞.未然,マ行五段"	} //  546
		,{	 0x7000|0x0001|0x0400 	,  "動詞.未然;動詞.終止連体,ラ行五段＊動詞.未然,ラ行五段"	} //  547
		,{	 0x7000|0x0001|0x0400 	,  "動詞.未然;動詞.終止連体,ワ行五段＊動詞.未然,ワ行五段"	} //  548
		,{	 0x7000|0x0005|0x0300 	,  "動詞.未然連用,サ変"	} //  549
		,{	 0x7000|0x0005|0x0300 	,  "動詞.未然連用,ザ変一字"	} //  550
		,{	 0x7000|0x0005|0x0a00 	,  "動詞.終止,サ変"	} //  551
		,{	 0x7000|0x0005|0x0a00 	,  "動詞.終止,サ変一字.文語"	} //  552
		,{	 0x7000|0x0005|0x0a00 	,  "動詞.終止,ザ変一字.文語"	} //  553
		,{	 0x7000|0x0001|0x0900 	,  "動詞.終止連体,カ変"	} //  554
		,{	 0x7000|0x0020|0x0900 	,  "動詞.終止連体,カ行五段促音"	} //  555
		,{	 0x7000|0x0005|0x0900 	,  "動詞.終止連体,サ変"	} //  556
		,{	 0x7000|0x0005|0x0900 	,  "動詞.終止連体,サ変一字"	} //  557
		,{	 0x7000|0x0005|0x0900 	,  "動詞.終止連体,ザ変一字.ジル"	} //  558
		,{	 0x7000|0x0005|0x0900 	,  "動詞.終止連体,ザ変一字.ズル"	} //  559
		,{	 0x7000|0x0020|0x0900 	,  "動詞.終止連体,ラ行五段イ"	} //  560
		,{	 0x7000|0x0020|0x0900 	,  "動詞.終止連体,ワ行五段ウ"	} //  561
		,{	 0x7000|0x0010|0x0900 	,  "動詞.終止連体,一段.仮定;動詞.終止連体,一段＊動詞.仮定,一段"	} //  562
		,{	 0x7000|0x0010|0x0900 	,  "動詞.終止連体,一段.命令;動詞.終止連体,一段＊動詞.命令,一段.ヨ"	} //  563
		,{	 0x7000|0x0010|0x0900 	,  "動詞.終止連体,一段.命令;動詞.終止連体,一段＊動詞.命令,一段.ロ"	} //  564
		,{	 0x7000|0x0010|0x0900 	,  "動詞.終止連体,一段.終止連体;動詞.終止連体,一段＊動詞.終止連体,一段"	} //  565
		,{	 0x7000|0x0010|0x0900 	,  "動詞.終止連体,一段.連用;動詞.終止連体,一段＊動詞.連用,一段"	} //  566
		,{	 0x7000|0x0001|0x0900 	,  "動詞.終止連体;動詞.終止連体,カ行五段＊動詞.終止連体,カ行五段"	} //  567
		,{	 0x7000|0x0001|0x0900 	,  "動詞.終止連体;動詞.終止連体,ガ行五段＊動詞.終止連体,ガ行五段"	} //  568
		,{	 0x7000|0x0001|0x0900 	,  "動詞.終止連体;動詞.終止連体,サ行五段＊動詞.終止連体,サ行五段"	} //  569
		,{	 0x7000|0x0001|0x0900 	,  "動詞.終止連体;動詞.終止連体,タ行五段＊動詞.終止連体,タ行五段"	} //  570
		,{	 0x7000|0x0001|0x0900 	,  "動詞.終止連体;動詞.終止連体,バ行五段＊動詞.終止連体,バ行五段"	} //  571
		,{	 0x7000|0x0001|0x0900 	,  "動詞.終止連体;動詞.終止連体,マ行五段＊動詞.終止連体,マ行五段"	} //  572
		,{	 0x7000|0x0001|0x0900 	,  "動詞.終止連体;動詞.終止連体,ラ行五段＊動詞.終止連体,ラ行五段"	} //  573
		,{	 0x7000|0x0001|0x0900 	,  "動詞.終止連体;動詞.終止連体,ワ行五段＊動詞.終止連体,ワ行五段"	} //  574
		,{	 0x7000|0x0001|0x0900 	,  "動詞.終止連体;動詞.終止連体,一段＊動詞.終止連体,一段"	} //  575
		,{	 0x7000|0x0001|0x0900 	,  "動詞.終止連体;動詞.連用,ラ行五段＊補助動詞.終止連体,一段"	} //  576
		,{	 0x7000|0x0001|0x0900 	,  "動詞.終止連体;動詞.連用,一段＊補助動詞.終止連体,一段"	} //  577
		,{	 0x7000|0x0001|0x0900 	,  "動詞.終止連体;名詞.一般＊動詞.終止連体,一段"	} //  578
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用,カ変"	} //  579
		,{	 0x7000|0x0020|0x0100 	,  "動詞.連用,カ行五段促音"	} //  580
		,{	 0x7000|0x0005|0x0100 	,  "動詞.連用,サ変一字"	} //  581
		,{	 0x7000|0x0020|0x0100 	,  "動詞.連用,ラ行五段イ"	} //  582
		,{	 0x7000|0x0020|0x0100 	,  "動詞.連用,ワ行五段ウ"	} //  583
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;動詞.終止連体,カ行五段＊動詞.連用,カ行五段"	} //  584
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;動詞.終止連体,カ行五段＊動詞.連用,カ行五段;＝名詞"	} //  585
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;動詞.終止連体,ガ行五段＊動詞.連用,ガ行五段;＝名詞"	} //  586
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;動詞.終止連体,サ行五段＊動詞.連用,サ行五段"	} //  587
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;動詞.終止連体,サ行五段＊動詞.連用,サ行五段;＝名詞"	} //  588
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;動詞.終止連体,タ行五段＊動詞.連用,タ行五段"	} //  589
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;動詞.終止連体,バ行五段＊動詞.連用,バ行五段"	} //  590
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;動詞.終止連体,マ行五段＊動詞.連用,マ行五段"	} //  591
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;動詞.終止連体,マ行五段＊動詞.連用,マ行五段;＝名詞"	} //  592
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;動詞.終止連体,ラ行五段＊動詞.連用,ラ行五段"	} //  593
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;動詞.終止連体,ラ行五段＊動詞.連用,ラ行五段;＝名詞"	} //  594
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;動詞.終止連体,ワ行五段＊動詞.連用,ワ行五段"	} //  595
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;動詞.終止連体,ワ行五段＊動詞.連用,ワ行五段;＝名詞"	} //  596
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;動詞.終止連体,一段＊動詞.連用,一段"	} //  597
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;動詞.終止連体,一段＊動詞.連用,一段;＝名詞"	} //  598
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;動詞.連用,ラ行五段＊補助動詞.連用,一段"	} //  599
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;動詞.連用,一段＊補助動詞.連用,一段"	} //  600
		,{	 0x7000|0x0001|0x0100 	,  "動詞.連用;名詞.一般＊動詞.連用,一段"	} //  601
		,{	 0x7000|0x0020|0x0100 	,  "動詞.音便,カ行五段促音"	} //  602
		,{	 0x7000|0x0020|0x0100 	,  "動詞.音便,ラ行五段イ"	} //  603
		,{	 0x7000|0x0020|0x0100 	,  "動詞.音便,ワ行五段ウ"	} //  604
		,{	 0x7000|0x0001|0x0100 	,  "動詞.音便;動詞.終止連体,カ行五段＊動詞.音便,カ行五段"	} //  605
		,{	 0x7000|0x0001|0x0100 	,  "動詞.音便;動詞.終止連体,ガ行五段＊動詞.音便,ガ行五段"	} //  606
		,{	 0x7000|0x0001|0x0100 	,  "動詞.音便;動詞.終止連体,タ行五段＊動詞.音便,タ行五段"	} //  607
		,{	 0x7000|0x0001|0x0100 	,  "動詞.音便;動詞.終止連体,バ行五段＊動詞.音便,バ行五段"	} //  608
		,{	 0x7000|0x0001|0x0100 	,  "動詞.音便;動詞.終止連体,マ行五段＊動詞.音便,マ行五段"	} //  609
		,{	 0x7000|0x0001|0x0100 	,  "動詞.音便;動詞.終止連体,ラ行五段＊動詞.音便,ラ行五段"	} //  610
		,{	 0x7000|0x0001|0x0100 	,  "動詞.音便;動詞.終止連体,ワ行五段＊動詞.音便,ワ行五段"	} //  611
		,{	 0x7000|0x0001 	,  "動詞;接頭辞.一般＊補助動詞.終止連体,サ変"	} //  612
		,{	 0x2000|0x0200 	,  "名詞.サ変;動詞.連用,サ行五段＊名詞.形式"	} //  613
		,{	 0x2000|0x0200 	,  "名詞.サ変;動詞.連用,マ行五段＊動詞.連用,一段"	} //  614
		,{	 0x2000|0x0200 	,  "名詞.サ変;動詞.連用,一段＊名詞.一般"	} //  615
		,{	 0x2000|0x0200 	,  "名詞.サ変;名詞.サ変＊名詞.サ変"	} //  616
		,{	 0x2000|0x0200 	,  "名詞.サ変;名詞.サ変＊接尾辞;＝サ変"	} //  617
		,{	 0x2000|0x0200 	,  "名詞.サ変;名詞.一般＊動詞.連用,一段"	} //  618
		,{	 0x2000|0x0200 	,  "名詞.サ変;名詞.一般＊名詞.サ変"	} //  619
		,{	 0x2000|0x0200 	,  "名詞.サ変;名詞.一般＊名詞.サ変;！未登録語カタ"	} //  620
		,{	 0x2000|0x0200 	,  "名詞.サ変;名詞.一般＊名詞.一般"	} //  621
		,{	 0x2000|0x0200 	,  "名詞.サ変;名詞.一般＊接尾辞;＝サ変"	} //  622
		,{	 0x2000|0x0200 	,  "名詞.サ変;名詞.形動＊接尾辞;＝サ変"	} //  623
		,{	 0x2000|0x0200 	,  "名詞.サ変;名詞.接尾辞＊名詞.サ変"	} //  624
		,{	 0x2000|0x0200 	,  "名詞.サ変;名詞.接尾辞＊名詞.一般;！未登録語カタ"	} //  625
		,{	 0x2000|0x0200 	,  "名詞.サ変;形容動詞.一般＊名詞.サ変"	} //  626
		,{	 0x2000|0x0200 	,  "名詞.サ変;形容動詞.一般＊接尾辞;＝サ変"	} //  627
		,{	 0x2000|0x0200 	,  "名詞.サ変;接頭辞.一般＊名詞.サ変"	} //  628
		,{	 0x2000|0x0200 	,  "名詞.サ変;接頭辞.一般＊名詞.一般"	} //  629
		,{	 0x2000|0x0200 	,  "名詞.サ変;接頭辞.一般＊接尾辞;＝サ変"	} //  630
		,{	 0x2000|0x0200 	,  "名詞.サ変;！未登録語カタ"	} //  631
		,{	 0x2000|0x0200 	,  "名詞.サ変;！未登録語カタ;＝形動"	} //  632
		,{	 0x2000|0x0100 	,  "名詞.一般;副詞＊名詞.接尾辞"	} //  633
		,{	 0x2000|0x0100 	,  "名詞.一般;動詞.連用,カ行五段＊名詞.形式"	} //  634
		,{	 0x2000|0x0100 	,  "名詞.一般;動詞.連用,カ行五段＊名詞.接尾辞"	} //  635
		,{	 0x2000|0x0100 	,  "名詞.一般;動詞.連用,バ行五段＊名詞.形式"	} //  636
		,{	 0x2000|0x0100 	,  "名詞.一般;動詞.連用,バ行五段＊名詞.接尾辞"	} //  637
		,{	 0x2000|0x0100 	,  "名詞.一般;動詞.連用,ラ行五段＊名詞.サ変;！未登録語カタ"	} //  638
		,{	 0x2000|0x0100 	,  "名詞.一般;動詞.連用,ラ行五段＊名詞.一般"	} //  639
		,{	 0x2000|0x0100 	,  "名詞.一般;動詞.連用,ラ行五段＊名詞.形式"	} //  640
		,{	 0x2000|0x0100 	,  "名詞.一般;動詞.連用,ラ行五段＊名詞.接尾辞"	} //  641
		,{	 0x2000|0x0100 	,  "名詞.一般;動詞.連用,ラ行五段＊接尾辞"	} //  642
		,{	 0x2000|0x0100 	,  "名詞.一般;動詞.連用,ワ行五段＊名詞.形式"	} //  643
		,{	 0x2000|0x0100 	,  "名詞.一般;動詞.連用,ワ行五段＊名詞.接尾辞"	} //  644
		,{	 0x2000|0x0100 	,  "名詞.一般;動詞.連用,一段＊名詞.サ変"	} //  645
		,{	 0x2000|0x0100 	,  "名詞.一般;動詞.連用,一段＊名詞.一般"	} //  646
		,{	 0x2000|0x0100 	,  "名詞.一般;動詞.連用,一段＊名詞.形式"	} //  647
		,{	 0x2000|0x0100 	,  "名詞.一般;動詞.連用,一段＊名詞.接尾辞"	} //  648
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.サ変＊名詞.サ変"	} //  649
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.サ変＊名詞.一般"	} //  650
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.サ変＊名詞.一般;数量〜"	} //  651
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.サ変＊名詞.一般;！未登録語カタ"	} //  652
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.サ変＊名詞.接尾辞"	} //  653
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.サ変＊名詞.接尾辞;数量〜"	} //  654
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.サ変＊接尾辞"	} //  655
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.サ変＊接尾辞;＝副詞"	} //  656
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.一般＊動詞.連用,カ行五段"	} //  657
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.一般＊動詞.連用,一段"	} //  658
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.一般＊名詞.サ変"	} //  659
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.一般＊名詞.一般"	} //  660
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.一般＊名詞.一般;！未登録語カタ"	} //  661
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.一般＊名詞.副詞"	} //  662
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.一般＊名詞.形動;〜助動詞.ガル"	} //  663
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.一般＊名詞.形式"	} //  664
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.一般＊名詞.接助"	} //  665
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.一般＊名詞.接尾辞"	} //  666
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.一般＊形容動詞.文語"	} //  667
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.一般＊接尾辞"	} //  668
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.一般＊接尾辞;数量〜"	} //  669
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.一般＊接尾辞;＝サ変"	} //  670
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.一般＊接尾辞;＝副詞"	} //  671
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.副詞＊名詞.接尾辞"	} //  672
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.副詞＊接尾辞"	} //  673
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.固有＊名詞.サ変"	} //  674
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.固有＊名詞.一般"	} //  675
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.固有＊名詞.一般;！未登録語カタ"	} //  676
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.固有＊名詞.接助"	} //  677
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.固有＊名詞.接尾辞"	} //  678
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.固有＊接尾辞"	} //  679
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.固有＊接尾辞;＝副詞"	} //  680
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.形動＊名詞.一般"	} //  681
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.形動＊名詞.接尾辞"	} //  682
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.形動＊接尾辞"	} //  683
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.形動＊接尾辞;＝副詞"	} //  684
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.形式＊名詞.一般"	} //  685
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.指示＊接尾辞"	} //  686
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.接助＊名詞.一般"	} //  687
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.接助＊名詞.一般;！未登録語カタ"	} //  688
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.接助＊名詞.接尾辞"	} //  689
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.接尾辞＊名詞.サ変"	} //  690
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.接尾辞＊名詞.一般"	} //  691
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.接尾辞＊名詞.一般;！未登録語カタ"	} //  692
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.接尾辞＊名詞.固有"	} //  693
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.接尾辞＊名詞.接助"	} //  694
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.接尾辞＊名詞.接尾辞"	} //  695
		,{	 0x2000|0x0100 	,  "名詞.一般;名詞.接尾辞＊接尾辞"	} //  696
		,{	 0x2000|0x0100 	,  "名詞.一般;形容動詞.一般＊名詞.サ変"	} //  697
		,{	 0x2000|0x0100 	,  "名詞.一般;形容動詞.一般＊名詞.一般"	} //  698
		,{	 0x2000|0x0100 	,  "名詞.一般;形容動詞.一般＊名詞.形動"	} //  699
		,{	 0x2000|0x0100 	,  "名詞.一般;形容動詞.一般＊名詞.接尾辞"	} //  700
		,{	 0x2000|0x0100 	,  "名詞.一般;形容動詞.一般＊名詞.接尾辞;＝サ変"	} //  701
		,{	 0x2000|0x0100 	,  "名詞.一般;形容動詞.一般＊接尾辞"	} //  702
		,{	 0x2000|0x0100 	,  "名詞.一般;形容動詞.文語＊名詞.一般"	} //  703
		,{	 0x2000|0x0100 	,  "名詞.一般;形容詞.語幹,ナイ＊名詞.一般"	} //  704
		,{	 0x2000|0x0100 	,  "名詞.一般;形容詞.語幹＊名詞.サ変"	} //  705
		,{	 0x2000|0x0100 	,  "名詞.一般;形容詞.語幹＊名詞.一般"	} //  706
		,{	 0x2000|0x0100 	,  "名詞.一般;形容詞.語幹＊名詞.一般;！未登録語カタ"	} //  707
		,{	 0x2000|0x0100 	,  "名詞.一般;形容詞.語幹＊接尾辞"	} //  708
		,{	 0x2000|0x0100 	,  "名詞.一般;接尾辞＊名詞.サ変"	} //  709
		,{	 0x2000|0x0100 	,  "名詞.一般;接頭辞.一般＊名詞.サ変"	} //  710
		,{	 0x2000|0x0100 	,  "名詞.一般;接頭辞.一般＊名詞.一般"	} //  711
		,{	 0x2000|0x0100 	,  "名詞.一般;接頭辞.一般＊名詞.一般;！未登録語カタ"	} //  712
		,{	 0x2000|0x0100 	,  "名詞.一般;接頭辞.一般＊名詞.形動"	} //  713
		,{	 0x2000|0x0100 	,  "名詞.一般;接頭辞.一般＊名詞.形動;〜助動詞.ガル"	} //  714
		,{	 0x2000|0x0100 	,  "名詞.一般;接頭辞.一般＊名詞.接助"	} //  715
		,{	 0x2000|0x0100 	,  "名詞.一般;接頭辞.一般＊名詞.接尾辞"	} //  716
		,{	 0x2000|0x0100 	,  "名詞.一般;接頭辞.一般＊接尾辞"	} //  717
		,{	 0x2000|0x0100 	,  "名詞.一般;数詞＊名詞.一般"	} //  718
		,{	 0x2000|0x0100 	,  "名詞.一般;数量〜"	} //  719
		,{	 0x2000|0x0100 	,  "名詞.一般;連体詞＊名詞.接助"	} //  720
		,{	 0x2000|0x0100 	,  "名詞.一般;連体詞＊名詞.接尾辞"	} //  721
		,{	 0x2000|0x0100 	,  "名詞.一般;連体詞＊接尾辞"	} //  722
		,{	 0x2000|0x0100 	,  "名詞.一般;！未登録語カタ"	} //  723
		,{	 0x2000|0x0100 	,  "名詞.一般;！未登録語カタ;＝形動カタ"	} //  724
		,{	 0x2000|0x0100 	,  "名詞.一般;＝数量"	} //  725
		,{	 0x2000|0x0300 	,  "名詞.副詞"	} //  726
		,{	 0x2000|0x0300 	,  "名詞.副詞;〜副助詞は"	} //  727
		,{	 0x2000|0x0300 	,  "名詞.副詞;〜助動詞な"	} //  728
		,{	 0x2000|0x0300 	,  "名詞.副詞;〜助動詞な;！助動詞に"	} //  729
		,{	 0x2000|0x0300 	,  "名詞.副詞;〜数詞"	} //  730
		,{	 0x2000|0x0300 	,  "名詞.副詞;名詞.サ変＊名詞.接尾辞"	} //  731
		,{	 0x2000|0x0300 	,  "名詞.副詞;名詞.一般＊名詞.接助"	} //  732
		,{	 0x2000|0x0300 	,  "名詞.副詞;名詞.一般＊接尾辞"	} //  733
		,{	 0x2000|0x0300 	,  "名詞.副詞;名詞.一般＊接尾辞;＝副詞"	} //  734
		,{	 0x2000|0x0300 	,  "名詞.副詞;名詞.副詞＊名詞.接尾辞"	} //  735
		,{	 0x2000|0x0300 	,  "名詞.副詞;名詞.副詞＊接尾辞"	} //  736
		,{	 0x2000|0x0300 	,  "名詞.副詞;名詞.副詞＊接尾辞;＝副詞"	} //  737
		,{	 0x2000|0x0300 	,  "名詞.副詞;名詞.接尾辞＊名詞.一般"	} //  738
		,{	 0x2000|0x0300 	,  "名詞.副詞;接頭辞.一般＊名詞.一般"	} //  739
		,{	 0x2000|0x0300 	,  "名詞.副詞;接頭辞.一般＊名詞.副詞;〜数詞"	} //  740
		,{	 0x2000|0x0300 	,  "名詞.副詞;直接指示＝"	} //  741
		,{	 0x2000|0x0300 	,  "名詞.副詞;連体詞＊名詞.副詞;〜数詞"	} //  742
		,{	 0x2000|0x0300 	,  "名詞.副詞;連体詞＊名詞.接助"	} //  743
		,{	 0x2000|0x0300 	,  "名詞.副詞;連体詞＊名詞.接尾辞"	} //  744
		,{	 0x2000|0x0300 	,  "名詞.副詞;！助動詞な;！助動詞に"	} //  745
		,{	 0x2000|0x0300 	,  "名詞.副詞;！名詞"	} //  746
		,{	 0x2000|0x0b00 	,  "名詞.固有;名詞.一般＊名詞.固有;！未登録語カタ"	} //  747
		,{	 0x2000|0x0b00 	,  "名詞.固有;名詞.一般＊名詞.接尾辞"	} //  748
		,{	 0x2000|0x0b00 	,  "名詞.固有;！未登録語カタ"	} //  749
		,{	 0x2000|0x0100 	,  "名詞.形動"	} //  750
		,{	 0x2000|0x0100 	,  "名詞.形動;〜助動詞.ガル"	} //  751
		,{	 0x2000|0x0100 	,  "名詞.形動;〜助動詞.ガル;〜助動詞に"	} //  752
		,{	 0x2000|0x0100 	,  "名詞.形動;〜助動詞.ガル;！助動詞に"	} //  753
		,{	 0x2000|0x0100 	,  "名詞.形動;〜助動詞だ;〜助動詞に"	} //  754
		,{	 0x2000|0x0100 	,  "名詞.形動;〜助動詞だ;！助動詞な"	} //  755
		,{	 0x2000|0x0100 	,  "名詞.形動;〜助動詞な;〜助動詞に"	} //  756
		,{	 0x2000|0x0100 	,  "名詞.形動;〜助動詞に"	} //  757
		,{	 0x2000|0x0100 	,  "名詞.形動;名詞.サ変＊接尾辞;＝形動"	} //  758
		,{	 0x2000|0x0100 	,  "名詞.形動;名詞.一般＊名詞.一般"	} //  759
		,{	 0x2000|0x0100 	,  "名詞.形動;名詞.形動＊副詞;〜助動詞.ダ"	} //  760
		,{	 0x2000|0x0100 	,  "名詞.形動;名詞.接尾辞＊名詞.サ変"	} //  761
		,{	 0x2000|0x0100 	,  "名詞.形動;名詞.接尾辞＊名詞.形動"	} //  762
		,{	 0x2000|0x0100 	,  "名詞.形動;形容詞.語幹,ナイ＊名詞.サ変;＝形動"	} //  763
		,{	 0x2000|0x0100 	,  "名詞.形動;形容詞.語幹,ナイ＊名詞.一般"	} //  764
		,{	 0x2000|0x0100 	,  "名詞.形動;接頭辞.一般＊名詞.サ変"	} //  765
		,{	 0x2000|0x0100 	,  "名詞.形動;接頭辞.一般＊名詞.サ変;＝形動"	} //  766
		,{	 0x2000|0x0100 	,  "名詞.形動;接頭辞.一般＊形容動詞.一般;〜格助詞の"	} //  767
		,{	 0x2000|0x0100 	,  "名詞.形動;！助動詞な"	} //  768
		,{	 0x2000|0x0100 	,  "名詞.形動;！助動詞な;！助動詞に"	} //  769
		,{	 0x2000|0x0100 	,  "名詞.形動;！助動詞に"	} //  770
		,{	 0x2000|0x0400 	,  "名詞.形式"	} //  771
		,{	 0x2000|0x0700 	,  "名詞.指示;直接指示＝"	} //  772
		,{	 0x2000|0x0700 	,  "名詞.指示;直接指示＝;＝副詞"	} //  773
		,{	 0x2000|0x0700 	,  "名詞.指示;連体詞＊名詞.形式"	} //  774
		,{	 0x2000|0x0700 	,  "名詞.指示;＝副詞"	} //  775
		,{	 0x2000|0x0300|0x0003 	,  "名詞.接助"	} //  776
		,{	 0x2000|0x0300|0x0003 	,  "名詞.接助;〜助動詞な"	} //  777
		,{	 0x2000|0x0300|0x0003 	,  "名詞.接助;指示〜"	} //  778
		,{	 0x2000|0x0300|0x0003 	,  "名詞.接助;直接指示＝"	} //  779
		,{	 0x2000|0x0500 	,  "名詞.接尾辞"	} //  780
		,{	 0x2000|0x0500 	,  "名詞.接尾辞;名詞.指示＊名詞.接尾辞"	} //  781
		,{	 0x2000|0x0500 	,  "名詞.接尾辞;数量〜"	} //  782
		,{	 0x2000|0x0100 	,  "名詞.複合"	} //  783
		,{	 0x2000|0x0800 	,  "形容動詞.一般;〜助動詞.ガル"	} //  784
		,{	 0x2000|0x0800 	,  "形容動詞.一般;〜助動詞.ガル;〜格助詞の"	} //  785
		,{	 0x2000|0x0800 	,  "形容動詞.一般;〜助動詞な"	} //  786
		,{	 0x2000|0x0800 	,  "形容動詞.一般;〜名詞"	} //  787
		,{	 0x2000|0x0800 	,  "形容動詞.一般;〜格助詞の"	} //  788
		,{	 0x2000|0x0800 	,  "形容動詞.一般;〜格助詞の;！助動詞な"	} //  789
		,{	 0x2000|0x0800 	,  "形容動詞.一般;〜格助詞の;！助動詞な;！助動詞に"	} //  790
		,{	 0x2000|0x0800 	,  "形容動詞.一般;〜格助詞の;！助動詞に"	} //  791
		,{	 0x2000|0x0800 	,  "形容動詞.一般;〜格助詞の;！未登録語カタ"	} //  792
		,{	 0x2000|0x0800 	,  "形容動詞.一般;動詞.連用,サ行五段＊名詞.形動"	} //  793
		,{	 0x2000|0x0800 	,  "形容動詞.一般;動詞.連用,一段＊名詞.サ変;＝形動"	} //  794
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.サ変＊名詞.サ変"	} //  795
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.サ変＊名詞.形動"	} //  796
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.サ変＊接尾辞;＝形動"	} //  797
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.一般＊名詞.一般"	} //  798
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.一般＊名詞.副詞"	} //  799
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.一般＊名詞.接尾辞"	} //  800
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.一般＊形容動詞.一般"	} //  801
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.一般＊形容動詞.一般;〜格助詞の"	} //  802
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.一般＊形容動詞.一般;〜格助詞の;！助動詞に"	} //  803
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.一般＊形容動詞.一般;！名詞"	} //  804
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.一般＊接尾辞;＝形動"	} //  805
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.副詞＊接尾辞;＝形動"	} //  806
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.固有＊名詞.接尾辞"	} //  807
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.形動＊名詞.サ変"	} //  808
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.形動＊名詞.一般"	} //  809
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.形動＊名詞.形動"	} //  810
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.形動＊接尾辞;＝形動"	} //  811
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.接助＊接尾辞;＝形動"	} //  812
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.接尾辞＊名詞.サ変;＝形動"	} //  813
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.接尾辞＊名詞.形動"	} //  814
		,{	 0x2000|0x0800 	,  "形容動詞.一般;名詞.接尾辞＊形容動詞.一般"	} //  815
		,{	 0x2000|0x0800 	,  "形容動詞.一般;形容動詞.一般＊形容動詞.一般;〜格助詞の"	} //  816
		,{	 0x2000|0x0800 	,  "形容動詞.一般;形容動詞.一般＊接尾辞;＝形動"	} //  817
		,{	 0x2000|0x0800 	,  "形容動詞.一般;形容詞.語幹,ナイ＊名詞.一般"	} //  818
		,{	 0x2000|0x0800 	,  "形容動詞.一般;形容詞.語幹＊名詞.一般"	} //  819
		,{	 0x2000|0x0800 	,  "形容動詞.一般;形容詞.語幹＊名詞.副詞"	} //  820
		,{	 0x2000|0x0800 	,  "形容動詞.一般;接頭辞.一般＊副詞;〜助動詞.ダ"	} //  821
		,{	 0x2000|0x0800 	,  "形容動詞.一般;接頭辞.一般＊名詞.サ変"	} //  822
		,{	 0x2000|0x0800 	,  "形容動詞.一般;接頭辞.一般＊名詞.サ変;＝形動"	} //  823
		,{	 0x2000|0x0800 	,  "形容動詞.一般;接頭辞.一般＊名詞.一般"	} //  824
		,{	 0x2000|0x0800 	,  "形容動詞.一般;接頭辞.一般＊名詞.形動"	} //  825
		,{	 0x2000|0x0800 	,  "形容動詞.一般;接頭辞.一般＊名詞.形動;〜助動詞.ガル"	} //  826
		,{	 0x2000|0x0800 	,  "形容動詞.一般;接頭辞.一般＊形容動詞.一般"	} //  827
		,{	 0x2000|0x0800 	,  "形容動詞.一般;接頭辞.一般＊形容動詞.一般;〜格助詞の"	} //  828
		,{	 0x2000|0x0800 	,  "形容動詞.一般;接頭辞.一般＊形容動詞.一般;！名詞"	} //  829
		,{	 0x2000|0x0800 	,  "形容動詞.一般;数詞＊形容動詞.一般;〜格助詞の"	} //  830
		,{	 0x2000|0x0800 	,  "形容動詞.一般;！助動詞な"	} //  831
		,{	 0x2000|0x0800 	,  "形容動詞.一般;！助動詞な;！助動詞に"	} //  832
		,{	 0x2000|0x0800 	,  "形容動詞.一般;！助動詞に"	} //  833
		,{	 0x2000|0x0800 	,  "形容動詞.一般;！名詞"	} //  834
		,{	 0x2000|0x0800 	,  "形容動詞.一般;！名詞;！未登録語カタ"	} //  835
		,{	 0x2000|0x0800 	,  "形容動詞.一般;！未登録語カタ"	} //  836
		,{	 0x2000|0x0800 	,  "形容動詞.一般;＝サ変"	} //  837
		,{	 0x2000|0x0800 	,  "形容動詞.文語"	} //  838
		,{	 0x2000|0x0800 	,  "形容動詞.文語;名詞.サ変＊形容動詞.文語;！助動詞な"	} //  839
		,{	 0x2000|0x0800 	,  "形容動詞.文語;！助動詞な"	} //  840
		,{	 0x2000|0x0800 	,  "形容動詞.文語;！未登録語カタ"	} //  841
		,{	 0x8000|0x0100 	,  "形容詞.ウ接続,ナイ"	} //  842
		,{	 0x8000|0x0400 	,  "形容詞.ズ接続,ナイ"	} //  843
		,{	 0x8000|0x0100 	,  "形容詞.ソウ接続,ナイ"	} //  844
		,{	 0x8000|0x0b00 	,  "形容詞.仮定,ナイ"	} //  845
		,{	 0x8000|0x0b00 	,  "形容詞.仮定,ナイ.口語.キ"	} //  846
		,{	 0x8000|0x0b00 	,  "形容詞.仮定,ナイ.口語.ケ"	} //  847
		,{	 0x8000|0x0b00 	,  "形容詞.仮定,口語.キ"	} //  848
		,{	 0x8000|0x0b00 	,  "形容詞.仮定,口語.ケ"	} //  849
		,{	 0x8000|0x0600 	,  "形容詞.命令,ナイ.文語"	} //  850
		,{	 0x8000|0x0300 	,  "形容詞.未然連用,ナイ"	} //  851
		,{	 0x8000|0x0a00 	,  "形容詞.終止,文語.シ"	} //  852
		,{	 0x8000|0x0900 	,  "形容詞.終止連体,イイ"	} //  853
		,{	 0x8000|0x0900 	,  "形容詞.終止連体,ナイ"	} //  854
		,{	 0x8000|0x0c00 	,  "形容詞.語幹,ナイ"	} //  855
		,{	 0x8000|0x0c00 	,  "形容詞.語幹;〜助動詞.ガル"	} //  856
		,{	 0x8000|0x0800 	,  "形容詞.連体,ナイ.文語.キ"	} //  857
		,{	 0x8000|0x0800 	,  "形容詞.連体,ナイ.文語.シ"	} //  858
		,{	 0x8000|0x0800 	,  "形容詞.連体,文語.カル"	} //  859
		,{	 0x8000|0x0100 	,  "形容詞.連用,文語.ウ"	} //  860
		,{	 0x8000|0x0100 	,  "形容詞.連用,文語.ュウ"	} //  861
		,{	 0x8000|0x0100 	,  "形容詞.音便,ナイ"	} //  862
		,{	 0x4000 	,  "感動詞;〜動詞"	} //  863
		,{	 0xc000 	,  "接尾辞;〜助動詞.ダ"	} //  864
		,{	 0xc000 	,  "接尾辞;助数詞〜"	} //  865
		,{	 0xc000 	,  "接尾辞;名詞.形式〜;指示〜;数量〜;＝副詞"	} //  866
		,{	 0xc000 	,  "接尾辞;名詞.接尾辞＊接尾辞;＝副詞"	} //  867
		,{	 0xc000 	,  "接尾辞;形動形容語幹〜"	} //  868
		,{	 0xc000 	,  "接尾辞;形動形容語幹〜;＝形動"	} //  869
		,{	 0xc000 	,  "接尾辞;数量〜"	} //  870
		,{	 0xc000 	,  "接尾辞;！未登録語カタ"	} //  871
		,{	 0xc000 	,  "接尾辞;＝サ変"	} //  872
		,{	 0xc000 	,  "接尾辞;＝副詞"	} //  873
		,{	 0xc000 	,  "接尾辞;＝形動"	} //  874
		,{	 0xa100|0x002f 	,  "接続助詞.カラ"	} //  875
		,{	 0xa100|0x002d 	,  "接続助詞.ガ"	} //  876
		,{	 0xa100|0x002e 	,  "接続助詞.ガテラ"	} //  877
		,{	 0xa100|0x0029 	,  "接続助詞.ケレド"	} //  878
		,{	 0xa100|0x0037 	,  "接続助詞.シ"	} //  879
		,{	 0xa100|0x0039 	,  "接続助詞.タリ"	} //  880
		,{	 0xa100|0x0039 	,  "接続助詞.タリ,濁音"	} //  881
		,{	 0xa100|0x003d 	,  "接続助詞.ツツ"	} //  882
		,{	 0xa100|0x003a 	,  "接続助詞.テ"	} //  883
		,{	 0xa100|0x003a 	,  "接続助詞.テ,濁音"	} //  884
		,{	 0xa100|0x003b 	,  "接続助詞.テモ"	} //  885
		,{	 0xa100|0x003b 	,  "接続助詞.テモ,濁音"	} //  886
		,{	 0xa100|0x003c 	,  "接続助詞.ト"	} //  887
		,{	 0xa100|0x0034 	,  "接続助詞.ニ"	} //  888
		,{	 0xa100|0x0035 	,  "接続助詞.ノデ"	} //  889
		,{	 0xa100|0x0036 	,  "接続助詞.ノニ"	} //  890
		,{	 0xa100|0x002a 	,  "接続助詞.バ"	} //  891
		,{	 0xa100|0x0029 	,  "接続助詞.相当;副助詞.クライ,濁音＊助動詞.ダ.仮定"	} //  892
		,{	 0xa100|0x0029 	,  "接続助詞.相当;副助詞.クライ＊助動詞.ダ.仮定"	} //  893
		,{	 0xa100|0x0029 	,  "接続助詞.相当;副助詞.ヤ＊副助詞.ヤ"	} //  894
		,{	 0xa100|0x0029 	,  "接続助詞.相当;助動詞.ヨウ.終止＊助動詞.ダ.仮定"	} //  895
		,{	 0xa100|0x0029 	,  "接続助詞.相当;接続助詞.カラ＊副助詞.ハ"	} //  896
		,{	 0xa100|0x0029 	,  "接続助詞.相当;格助詞.ガ＊格助詞.ニ"	} //  897
		,{	 0xa100|0x0029 	,  "接続助詞.相当;格助詞.ト＊格助詞.ニ"	} //  898
		,{	 0xa100|0x0029 	,  "接続助詞.相当;格助詞.ニ＊助動詞.ズ.終止連用"	} //  899
		,{	 0xa100|0x0029 	,  "接続助詞.相当;格助詞.ニ＊動詞.命令,サ変.ヨ"	} //  900
		,{	 0x6000 	,  "接続詞;副助詞.ノミ＊助動詞.ズ.終止連用"	} //  901
		,{	 0x6000 	,  "接続詞;副詞＊助動詞.ダ.仮定"	} //  902
		,{	 0x6000 	,  "接続詞;副詞＊接続助詞.バ"	} //  903
		,{	 0x6000 	,  "接続詞;名詞.一般＊副助詞.カ"	} //  904
		,{	 0x6000 	,  "接続詞;名詞.一般＊名詞.一般"	} //  905
		,{	 0x6000 	,  "接続詞;名詞.一般＊接続助詞.テモ"	} //  906
		,{	 0x6000 	,  "接続詞;接続詞＊副助詞.ナガラ"	} //  907
		,{	 0x6000 	,  "接続詞;文頭＝"	} //  908
		,{	 0x6000 	,  "接続詞;連体詞＊名詞.一般"	} //  909
		,{	 0xb000 	,  "接頭辞.一般;！未登録語カタ"	} //  910
		,{	 0xb000 	,  "接頭辞.数詞"	} //  911
		,{	 0x2000|0x0a00 	,  "数詞.複合"	} //  912
		,{	 0xa205 	,  "格助詞.カラ"	} //  913
		,{	 0xa100|0x0120 	,  "格助詞.ガ"	} //  914
		,{	 0xa100|0x0124 	,  "格助詞.デ"	} //  915
		,{	 0xa100|0x0121 	,  "格助詞.ト"	} //  916
		,{	 0xa206 	,  "格助詞.ニ"	} //  917
		,{	 0xa207 	,  "格助詞.ノ"	} //  918
		,{	 0xa100|0x0122 	,  "格助詞.ヘ"	} //  919
		,{	 0xa208 	,  "格助詞.ヨリ"	} //  920
		,{	 0xa100|0x0123 	,  "格助詞.ヲ"	} //  921
		,{	 0xa100|0x011f 	,  "格助詞.相当;格助詞.ト＊格助詞.ニ"	} //  922
		,{	 0xa100|0x011f 	,  "格助詞.相当;格助詞.ニ＊助動詞.ル.終止連体"	} //  923
		,{	 0xa100|0x011f 	,  "格助詞.相当;格助詞.ニ＊動詞.仮定命令,カ行五段"	} //  924
		,{	 0xa100|0x011f 	,  "格助詞.相当;格助詞.ニ＊動詞.終止連体,ラ行五段"	} //  925
		,{	 0xa100|0x011f 	,  "格助詞.相当;格助詞.ニ＊動詞.連用,カ行五段"	} //  926
		,{	 0xa100|0x011f 	,  "格助詞.相当;格助詞.ニ＊動詞.連用,ラ行五段"	} //  927
		,{	 0xa100|0x011f 	,  "格助詞.相当;格助詞.ニ＊接続助詞.テ"	} //  928
		,{	 0xa100|0x011f 	,  "格助詞.相当;格助詞.ヲ＊接続助詞.テ"	} //  929
		,{	 0xa300 	,  "準体助詞.ノ"	} //  930
		,{	 0xa500 	,  "終助詞"	} //  931
		,{	 0xa500 	,  "終助詞;助詞て〜;名詞類〜"	} //  932
		,{	 0xa500 	,  "終助詞;助詞て〜;名詞類〜;終助詞〜"	} //  933
		,{	 0xa500 	,  "終助詞;名詞類〜"	} //  934
		,{	 0xa500 	,  "終助詞;名詞類〜;〜句点"	} //  935
		,{	 0xa500 	,  "終助詞;終助詞〜"	} //  936
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.ウ接続,カ行五段"	} //  937
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.ウ接続,カ行五段;体言動詞化＝"	} //  938
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.ウ接続,カ行五段促音"	} //  939
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.ウ接続,サ行五段;体言動詞化＝"	} //  940
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.ウ接続,ラ行五段"	} //  941
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.ウ接続,ラ行五段イ;体言動詞化＝"	} //  942
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.ウ接続,ワ行五段"	} //  943
		,{	 0x7000|0x0005|0x0400 	,  "補助動詞.ズ接続,サ変;体言動詞化＝"	} //  944
		,{	 0x7000|0x0005|0x0400 	,  "補助動詞.レル接続,サ変;体言動詞化＝"	} //  945
		,{	 0x7000|0x0001|0x0b00 	,  "補助動詞.仮定,カ変"	} //  946
		,{	 0x7000|0x0020|0x0b00 	,  "補助動詞.仮定,カ行五段.口語"	} //  947
		,{	 0x7000|0x0020|0x0b00 	,  "補助動詞.仮定,カ行五段.口語;体言動詞化＝"	} //  948
		,{	 0x7000|0x0020|0x0b00 	,  "補助動詞.仮定,カ行五段促音.口語"	} //  949
		,{	 0x7000|0x0005|0x0b00 	,  "補助動詞.仮定,サ変;体言動詞化＝"	} //  950
		,{	 0x7000|0x0020|0x0b00 	,  "補助動詞.仮定,サ行五段.口語;体言動詞化＝"	} //  951
		,{	 0x7000|0x0020|0x0b00 	,  "補助動詞.仮定,ラ行五段.口語"	} //  952
		,{	 0x7000|0x0020|0x0b00 	,  "補助動詞.仮定,ラ行五段イ.口語;体言動詞化＝"	} //  953
		,{	 0x7000|0x0020|0x0b00 	,  "補助動詞.仮定,ラ行五段イ;体言動詞化＝"	} //  954
		,{	 0x7000|0x0020|0x0b00 	,  "補助動詞.仮定,ワ行五段.口語"	} //  955
		,{	 0x7000|0x0010|0x0b00 	,  "補助動詞.仮定,一段"	} //  956
		,{	 0x7000|0x0010|0x0b00 	,  "補助動詞.仮定,一段;体言動詞化＝"	} //  957
		,{	 0x7000|0x0010|0x0b00 	,  "補助動詞.仮定,一段;形動形容語幹〜"	} //  958
		,{	 0x7000|0x0020|0x0700 	,  "補助動詞.仮定命令,カ行五段"	} //  959
		,{	 0x7000|0x0020|0x0700 	,  "補助動詞.仮定命令,カ行五段;体言動詞化＝"	} //  960
		,{	 0x7000|0x0020|0x0700 	,  "補助動詞.仮定命令,カ行五段促音"	} //  961
		,{	 0x7000|0x0020|0x0700 	,  "補助動詞.仮定命令,サ行五段;体言動詞化＝"	} //  962
		,{	 0x7000|0x0020|0x0700 	,  "補助動詞.仮定命令,ラ行五段"	} //  963
		,{	 0x7000|0x0020|0x0700 	,  "補助動詞.仮定命令,ワ行五段"	} //  964
		,{	 0x7000|0x0001|0x0600 	,  "補助動詞.命令,カ変"	} //  965
		,{	 0x7000|0x0005|0x0600 	,  "補助動詞.命令,サ変.ヨ;体言動詞化＝"	} //  966
		,{	 0x7000|0x0005|0x0600 	,  "補助動詞.命令,サ変.ロ;体言動詞化＝"	} //  967
		,{	 0x7000|0x0020|0x0600 	,  "補助動詞.命令,ラ行五段イ;体言動詞化＝"	} //  968
		,{	 0x7000|0x0010|0x0600 	,  "補助動詞.命令,一段.ヨ"	} //  969
		,{	 0x7000|0x0010|0x0600 	,  "補助動詞.命令,一段.ヨ;体言動詞化＝"	} //  970
		,{	 0x7000|0x0010|0x0600 	,  "補助動詞.命令,一段.ヨ;形動形容語幹〜"	} //  971
		,{	 0x7000|0x0010|0x0600 	,  "補助動詞.命令,一段.ロ"	} //  972
		,{	 0x7000|0x0010|0x0600 	,  "補助動詞.命令,一段.ロ;体言動詞化＝"	} //  973
		,{	 0x7000|0x0010|0x0600 	,  "補助動詞.命令,一段.ロ;形動形容語幹〜"	} //  974
		,{	 0x7000|0x0001|0x0400 	,  "補助動詞.未然,カ変"	} //  975
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.未然,カ行五段"	} //  976
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.未然,カ行五段;体言動詞化＝"	} //  977
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.未然,カ行五段促音"	} //  978
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.未然,サ行五段;体言動詞化＝"	} //  979
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.未然,ラ行五段"	} //  980
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.未然,ラ行五段イ;体言動詞化＝"	} //  981
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.未然,ワ行五段"	} //  982
		,{	 0x7000|0x0005|0x0300 	,  "補助動詞.未然連用,サ変;体言動詞化＝"	} //  983
		,{	 0x7000|0x0005|0x0a00 	,  "補助動詞.終止,サ変;体言動詞化＝"	} //  984
		,{	 0x7000|0x0001|0x0900 	,  "補助動詞.終止連体,カ変"	} //  985
		,{	 0x7000|0x0020|0x0900 	,  "補助動詞.終止連体,カ行五段"	} //  986
		,{	 0x7000|0x0020|0x0900 	,  "補助動詞.終止連体,カ行五段;体言動詞化＝"	} //  987
		,{	 0x7000|0x0020|0x0900 	,  "補助動詞.終止連体,カ行五段促音"	} //  988
		,{	 0x7000|0x0005|0x0900 	,  "補助動詞.終止連体,サ変;体言動詞化＝"	} //  989
		,{	 0x7000|0x0020|0x0900 	,  "補助動詞.終止連体,サ行五段;体言動詞化＝"	} //  990
		,{	 0x7000|0x0020|0x0900 	,  "補助動詞.終止連体,ラ行五段"	} //  991
		,{	 0x7000|0x0020|0x0900 	,  "補助動詞.終止連体,ラ行五段イ;体言動詞化＝"	} //  992
		,{	 0x7000|0x0020|0x0900 	,  "補助動詞.終止連体,ワ行五段"	} //  993
		,{	 0x7000|0x0010|0x0900 	,  "補助動詞.終止連体,一段"	} //  994
		,{	 0x7000|0x0010|0x0900 	,  "補助動詞.終止連体,一段;体言動詞化＝"	} //  995
		,{	 0x7000|0x0010|0x0900 	,  "補助動詞.終止連体,一段;形動形容語幹〜"	} //  996
		,{	 0x7000|0x0001|0x0100 	,  "補助動詞.連用,カ変"	} //  997
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.連用,カ行五段"	} //  998
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.連用,カ行五段;体言動詞化＝"	} //  999
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.連用,カ行五段促音"	} //  1000
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.連用,サ行五段;体言動詞化＝"	} //  1001
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.連用,ラ行五段"	} //  1002
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.連用,ラ行五段イ;体言動詞化＝"	} //  1003
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.連用,ワ行五段"	} //  1004
		,{	 0x7000|0x0010|0x0100 	,  "補助動詞.連用,一段"	} //  1005
		,{	 0x7000|0x0010|0x0100 	,  "補助動詞.連用,一段;体言動詞化＝"	} //  1006
		,{	 0x7000|0x0010|0x0100 	,  "補助動詞.連用,一段;形動形容語幹〜"	} //  1007
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.音便,カ行五段"	} //  1008
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.音便,カ行五段;体言動詞化＝"	} //  1009
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.音便,カ行五段促音"	} //  1010
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.音便,サ行五段;体言動詞化＝"	} //  1011
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.音便,ラ行五段"	} //  1012
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.音便,ラ行五段イ;体言動詞化＝"	} //  1013
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.音便,ワ行五段"	} //  1014
		,{	 0x8000|0x0100 	,  "補助形容詞.ウ接続"	} //  1015
		,{	 0x8000|0x0100 	,  "補助形容詞.ウ接続,ヨイ"	} //  1016
		,{	 0x8000|0x0400 	,  "補助形容詞.ズ接続"	} //  1017
		,{	 0x8000|0x0400 	,  "補助形容詞.ズ接続,ヨイ"	} //  1018
		,{	 0x8000|0x0100 	,  "補助形容詞.ソウ接続,ヨイ"	} //  1019
		,{	 0x8000|0x0b00 	,  "補助形容詞.仮定"	} //  1020
		,{	 0x8000|0x0b00 	,  "補助形容詞.仮定,ヨイ"	} //  1021
		,{	 0x8000|0x0b00 	,  "補助形容詞.仮定,ヨイ.口語.キ"	} //  1022
		,{	 0x8000|0x0b00 	,  "補助形容詞.仮定,ヨイ.口語.ケ"	} //  1023
		,{	 0x8000|0x0b00 	,  "補助形容詞.仮定,口語.キ"	} //  1024
		,{	 0x8000|0x0b00 	,  "補助形容詞.仮定,口語.ケ"	} //  1025
		,{	 0x8000|0x0600 	,  "補助形容詞.命令,ヨイ.文語"	} //  1026
		,{	 0x8000|0x0600 	,  "補助形容詞.命令,文語"	} //  1027
		,{	 0x8000|0x0300 	,  "補助形容詞.未然連用"	} //  1028
		,{	 0x8000|0x0300 	,  "補助形容詞.未然連用,ヨイ"	} //  1029
		,{	 0x8000|0x0a00 	,  "補助形容詞.終止,文語.シ"	} //  1030
		,{	 0x8000|0x0900 	,  "補助形容詞.終止連体"	} //  1031
		,{	 0x8000|0x0900 	,  "補助形容詞.終止連体,イイ"	} //  1032
		,{	 0x8000|0x0900 	,  "補助形容詞.終止連体,ヨイ"	} //  1033
		,{	 0x8000|0x0c00 	,  "補助形容詞.語幹"	} //  1034
		,{	 0x8000|0x0c00 	,  "補助形容詞.語幹,ヨイ"	} //  1035
		,{	 0x8000|0x0800 	,  "補助形容詞.連体,ヨイ.文語.キ"	} //  1036
		,{	 0x8000|0x0800 	,  "補助形容詞.連体,ヨイ.文語.シ"	} //  1037
		,{	 0x8000|0x0800 	,  "補助形容詞.連体,文語.カル"	} //  1038
		,{	 0x8000|0x0800 	,  "補助形容詞.連体,文語.キ"	} //  1039
		,{	 0x8000|0x0100 	,  "補助形容詞.連用,文語.ウ"	} //  1040
		,{	 0x8000|0x0100 	,  "補助形容詞.連用,文語.ュウ"	} //  1041
		,{	 0x8000|0x0100 	,  "補助形容詞.音便"	} //  1042
		,{	 0x8000|0x0100 	,  "補助形容詞.音便,ヨイ"	} //  1043
		,{	 0x2000|0x0900 	,  "記号.コンマ"	} //  1044
		,{	 0x2000|0x0900 	,  "記号.ピリオド"	} //  1045
		,{	 0x2000|0x0900 	,  "記号.一般;未登録語カタ！;！未登録語カタ"	} //  1046
		,{	 0x2000|0x0900 	,  "記号.中点"	} //  1047
		,{	 0x2000|0x0900|0x0002 	,  "記号.閉じ括弧,一般"	} //  1048
		,{	 0x2000|0x0900|0x0001 	,  "記号.開き括弧,一般"	} //  1049
		,{	 0x5000 	,  "連体詞;〜助動詞に"	} //  1050
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.ウ接続,サ行五段"	} //  1051
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.ウ接続,ラ行五段イ"	} //  1052
		,{	 0x7000|0x0005|0x0400 	,  "補助動詞.ズ接続,サ変"	} //  1053
		,{	 0x7000|0x0005|0x0400 	,  "補助動詞.レル接続,サ変"	} //  1054
		,{	 0x7000|0x0005|0x0b00 	,  "補助動詞.仮定,サ変"	} //  1055
		,{	 0x7000|0x0020|0x0b00 	,  "補助動詞.仮定,サ行五段.口語"	} //  1056
		,{	 0x7000|0x0020|0x0b00 	,  "補助動詞.仮定,ラ行五段イ"	} //  1057
		,{	 0x7000|0x0020|0x0b00 	,  "補助動詞.仮定,ラ行五段イ.口語"	} //  1058
		,{	 0x7000|0x0020|0x0700 	,  "補助動詞.仮定命令,サ行五段"	} //  1059
		,{	 0x7000|0x0005|0x0600 	,  "補助動詞.命令,サ変.ヨ"	} //  1060
		,{	 0x7000|0x0005|0x0600 	,  "補助動詞.命令,サ変.ロ"	} //  1061
		,{	 0x7000|0x0020|0x0600 	,  "補助動詞.命令,ラ行五段イ"	} //  1062
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.未然,サ行五段"	} //  1063
		,{	 0x7000|0x0020|0x0400 	,  "補助動詞.未然,ラ行五段イ"	} //  1064
		,{	 0x7000|0x0005|0x0300 	,  "補助動詞.未然連用,サ変"	} //  1065
		,{	 0x7000|0x0005|0x0a00 	,  "補助動詞.終止,サ変"	} //  1066
		,{	 0x7000|0x0005|0x0900 	,  "補助動詞.終止連体,サ変"	} //  1067
		,{	 0x7000|0x0020|0x0900 	,  "補助動詞.終止連体,サ行五段"	} //  1068
		,{	 0x7000|0x0020|0x0900 	,  "補助動詞.終止連体,ラ行五段イ"	}	// 1069
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.連用,サ行五段"	}			// 1070
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.連用,ラ行五段イ"	}	 	// 1071
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.音便,サ行五段"	} 			// 1072
		,{	 0x7000|0x0020|0x0100 	,  "補助動詞.音便,ラ行五段イ"	} 		// 1073
		,{	 0x2000|0x0200 	,  "名詞.サ変;数量〜"	}						// 1074
		,{	 0xc000 	,  "接尾辞;名詞.形式〜;指示〜;数量〜"	} 			// 1075
		,{	 0x2000|0x0900 	,  "記号.一般;未登録語カタ！"	}		 		// 1076
		,{	0, 0 }
	};
}

////////////////////////////////////////////////////////
//  class UnaJpDicSet
//


UnaJpDicSet::UnaJpDicSet(const char *rule_,ModLanguageSet lang_):DicSet(rule_,lang_)
{
}

int UnaJpDicSet::getTypeNoFromTypeString(const ModUnicodeString& typeString_, int startpos_)
{
	const ModVector< TypeInfo >& typeinfo = getTypeInfoVector();
	// 開始位置チェック
	if ( startpos_ == 0 || startpos_ > (int)(typeinfo.getSize() - 1) ) {
		startpos_ = typeinfo.getSize() - 1;
	}
	// 指定位置から先頭に向けて探す
	for( ; startpos_ >= 0; startpos_--) {
		const ModUnicodeString& tgtstr = typeinfo[startpos_]._name;
	
		// 比較する文字数を取得する
		// 比較対象は先頭文字から ";" まで。無ければ全文字を比較する
		ModUnicodeChar* findPos = ModUnicodeCharTrait::find(tgtstr, ';');
		int compLen = ( findPos == 0 ) ? 0 : (int)(findPos - tgtstr);
		if ( ModUnicodeCharTrait::compare( typeString_, tgtstr, compLen ) == 0 ) {
			// 品詞の発見成功
			return startpos_;
		}
	}

	// 発見に失敗したら 0 を返す
	return 0;
}

void UnaJpDicSet::makeSameTypeList(const ModUnicodeString& typename_)
{
	// マップが無ければ作成する
	if ( l_SameTypeBank == 0 )
		l_SameTypeBank = new SameTypeBank;
	// 一致する文字列の最初の品詞番号を取得する

	int headno = getTypeCodeFromTypeString(typename_);
	// 最後から先頭に向けて処理する

	int tgtpos = sizeof(JpTransTable)/sizeof(JpTransTable[0]);
	while ( tgtpos > headno ) {
		// 品詞名が一致する最初の品詞番号を取得する

		tgtpos = getTypeNoFromTypeString(typename_, tgtpos);
		if ( tgtpos <= headno ) break;
		// リストに登録済みのグループなら登録しない
		if ( l_SameTypeBank->find(JpTransTable[tgtpos].code)
			  != l_SameTypeBank->end() )
			return;
		// 品詞番号よりグループの先頭の品詞番号を取得する

		tgtpos = getGroupHeadNoFromTypeNo(tgtpos);
		// リストに登録する
	
		l_SameTypeBank->insert(JpTransTable[tgtpos].code, headno);

		// イテレータを勧める
		tgtpos--;
	}
}

const ModVector< TypeInfo >& UnaJpDicSet::getTypeInfoVector()
{
	ModAutoMutex<ModCriticalSection> autoCS(&l_TypeNameInfoCS);
	autoCS.lock();
	// make the vector at first time
	if ( l_TypeNameInfo.getSize() == 0 ){	
		// 領域を予約してから…
		l_TypeNameSize = sizeof(JpTransTable) / sizeof(JpTransTable[0]) - 1;
		l_TypeNameInfo.reserve(l_TypeNameSize);

		for ( int i = 0; i < l_TypeNameSize; ++i ) {
			// push back the vector in turn
			l_TypeNameInfo.pushBack(TypeInfo(JpTransTable[i].code,
					ModUnicodeString(JpTransTable[i].name, LiteralCode)));
		}
	}
	return l_TypeNameInfo;
}

int UnaJpDicSet::getGroupHeadNoFromTypeNo(int startpos_)
{
	// 開始位置チェック
	if ( startpos_ == 0 ) {
		startpos_ = sizeof(JpTransTable)/sizeof(JpTransTable[0]);
	}

	// 指定位置から先頭に向けて同グループの先頭番号を探す
	unsigned short group = JpTransTable[startpos_].code;
	for ( ; startpos_ >= 0; startpos_--)
		if ( JpTransTable[startpos_].code != group )
			return startpos_ + 1;
	// failure, return 0
	return 0;
}

unsigned short UnaJpDicSet::getGroupFromTypeNo(unsigned short unahin_)
{
	return getTypeInfoVector()[unahin_]._code;
}

int UnaJpDicSet::getTypeCodeFromTypeString(const ModUnicodeString& typeString_)
{
	const ModVector< TypeInfo >& typeinfo = getTypeInfoVector();
	// UNA の品詞データより検索
	int iMax = typeinfo.getSize();
	for ( int i = 1; i < iMax; ++i ) {
		const ModUnicodeString& tgtstr = typeinfo[i]._name;

	// 比較する文字数を取得する
	// 比較対象は先頭文字から ";" まで。無ければ全文字を比較する
		ModUnicodeChar* findPos
			= ModUnicodeCharTrait::find(tgtstr, UnicodeChar::usSemiColon);
		int compLen = ( findPos == 0 ) ? 0 : (int)(findPos - tgtstr);

		if ( ModUnicodeCharTrait::compare(typeString_, tgtstr, compLen ) == 0 ) {
			// 共通品詞種別の発見成功
			// この品詞が品詞グループの先頭である保証は無いので、
			// 品詞グループの先頭を求める
			return (int)getHeadNoFromGroup(i);
		}
	}
	return 0;
}

unsigned short
UnaJpDicSet::getHeadNoFromGroup(unsigned short unahin_)
{
// #T45#
	// 同品詞グループが無ければ unahin_ を直接返してよい
	if ( l_SameTypeBank.get() == 0 )
		return unahin_;

// #T46#
	// unahin_ のグループマスクを取得する
	
	unsigned short groupmask = JpTransTable[unahin_].code;

// #T47#
	// まず、同品詞グループに登録されている品詞かチェックする
	
	SameTypeBank::Iterator findresult = (*l_SameTypeBank).find(groupmask);
	if ( findresult != (*l_SameTypeBank).end() ) {
// #T48#
		// 同品詞グループに登録されているなら、その品詞番号を返す
		return (*findresult).second;
	}

// #T49#
	// 対象位置から前に進みながらマスク値が同じものを探す
	for ( ; (short)unahin_ >= 0; unahin_-- ) {

		if ( JpTransTable[unahin_].code != groupmask ) {
// #T50#
			// 異なるグループに進入したので次の品詞番号を返す
			return (unahin_ + 1);
		}
	}

// #T51#
	// ここにはこないと思うが、もし来たら unahin_ を返す
	return unahin_;
}

void UnaJpDicSet::registSameType(const ModUnicodeString& type_)
{
	makeSameTypeList(type_);
}

TagCode UnaJpDicSet::getTypeCode( ModUnicodeString& typeString_)
{
	TagCode code;
	code.strict = getTypeCodeFromTypeString(typeString_);
	code.common = UNA_OTHER;
	return code;
}

int UnaJpDicSet::getTypeCode(int pos)
{
	if(pos >= 0 && pos <= sizeof(convTable)/sizeof(convTable[0]))
		return convTable[pos];
	return UNA_OTHER;
}

////////////////////////////////////////////////////////
//  class ModNlpResourceUnaJp
//
ModNlpResourceUnaJp::ModNlpResourceUnaJp()
	: _cResourceX(0),
	JpDicSet("JpExRule.txt",ModLanguageSet( "ja+en" ))
{
	DEBUGPRINT("ModNlpResourceUnaJp::ModNlpResourceUnaJp\n",0);
}

ModNlpResourceUnaJp::~ModNlpResourceUnaJp()
{
	DEBUGPRINT("ModNlpResourceUnaJp::~ModNlpResourceUnaJp\n",0);  
	this->unload();
}
DicSet *
ModNlpResourceUnaJp::getJapaneseDic()
{
	return &JpDicSet;
}
ModBoolean
ModNlpResourceUnaJp::load(const ModUnicodeString& resourcePath_, 
						  const ModBoolean memorySave_)
{
	DEBUGPRINT("ModNlpResourceUnaJp::load\n",0);
	ModFile file(resourcePath_);
	m_resourcePath = file.getFullPathNameW();
	_cResourceX = new ModNlpResourceX(m_resourcePath,
									  ModLanguageSet(),
									  memorySave_ == ModTrue ? 1 : 0);
	m_ulResType = checkFile(resourcePath_);
	return ModTrue;
}

ModSize
ModNlpResourceUnaJp::checkFile(const ModUnicodeString& resourcePath_)
{
	ModSize ulResType = 3400; // data340-for unknown

	if (!ModFile::doesExist(resourcePath_+"norm/")
	 || !ModFile::doesExist(resourcePath_+"una/"))
	{
		ModMessage << "Resource data structure error. Please check whether norm and una directory." << ModEndl;
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument,ModErrorLevelError);
	}
	else if (ModFile::doesExist(resourcePath_+"norm/metaDef.tbl"))
	{
		ulResType = 3402; // data340-for foreign
	}
	else
	{
		ulResType = 3401; // data340-for Japanese
	}
	return ulResType;
}

ModBoolean 
ModNlpResourceUnaJp::unload()
{
	DEBUGPRINT("ModNlpResourceUnaJp::unload\n",0);  
	delete _cResourceX,_cResourceX=0;
	return ModTrue;
}

ModNlpResourceX*
ModNlpResourceUnaJp::getResourceX()
{
	return _cResourceX;
}
//
// Copyright (c) 2005, 2009, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
