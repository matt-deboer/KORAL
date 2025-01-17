/*******************************************************************
*   CLATCH.h
*   KORAL
*
*	Author: Kareem Omar
*	kareem.omar@uah.edu
*	https://github.com/komrad36
*
*	Last updated Dec 27, 2016
*******************************************************************/
//
// ## Summary ##
// KORAL is a novel, extremely fast, highly accurate, scale- and
// rotation-invariant, CPU-GPU cooperative detector-descriptor.
//
// Detection is based on the author's custom multi-scale KFAST corner
// detector, with rapid bilinear interpolation performed by the GPU
// asynchronously while the CPU works on KFAST.
//
// ## Usage ##
// Basic use of KORAL is extremely easy, although, of course, for a
// larger high-performance pipeline, users will benefit from
// calling KORAL functions directly and modifying it to suit their needs.
//
// To detect and describe, simply #include "KORAL.h" and
// then do:
//
// 	    KORAL koral(scale_factor, scale_levels);
//      koral.go(image, width, height, KFAST_threshold);
//
// where scale_factor is the factor by which each scale leve
// is reduced from the previous, scale_levels is the total
// number of such scale levels used, image is a pointer to
// uint8_t (grayscale) image data, and KFAST_threshold
// is the threshold supplied to the KFAST feature detector.
//
// After this call, keypoints are avaiable in a vector at 
// koral.kps, while descriptors are available at
// koral.desc.
//
// Portions of KORAL require SSE, AVX, AVX2, and CUDA.
// The author is working on reduced-performance versions
// with lesser requirements, but as the intent of this work
// is primarily novel performance capability, modern
// hardware and this full version are highly recommended.
//
// Description is performed by the GPU using the novel CLATCH
// (CUDA LATCH) binary descriptor kernel.
//
// Rotation invariance is provided by a novel vectorized
// SSE angle weight detector.
//
// All components have been written and carefully tuned by the author
// for maximum performance and have no external dependencies. Some have
// been modified for integration into KORAL,
// but the original standalone projects are all availble on
// the author's GitHub (https://github.com/komrad36).
//
// These individual components are:
// -KFAST        (https://github.com/komrad36/KFAST)
// -CUDALERP     (https://github.com/komrad36/CUDALERP)
// -FeatureAngle (https://github.com/komrad36/FeatureAngle)
// -CLATCH       (https://github.com/komrad36/CLATCH)
//
// In addition, the natural next step of matching descriptors
// is available in the author's currently separate
// project, CUDAK2NN (https://github.com/komrad36/CUDAK2NN).
//
// A key insight responsible for much of the performance of
// this insanely fast system is due to Christopher Parker
// (https://github.com/csp256), to whom I am extremely grateful.
// 
// The file 'main.cpp' is a simple test driver
// illustrating example usage.It requires OpenCV
// for image read and keypoint display.KORAL itself,
// however, does not require OpenCV or any other
// external dependencies.
//
// Note that KORAL is a work in progress.
// Suggestions and improvements are welcomed.
//
// ## License ##
// The FAST detector was created by Edward Rosten and Tom Drummond
// as described in the 2006 paper by Rosten and Drummond:
// "Machine learning for high-speed corner detection"
//         Edward Rosten and Tom Drummond
// https://www.edwardrosten.com/work/rosten_2006_machine.pdf
//
// The FAST detector is BSD licensed:
// 
// Copyright(c) 2006, 2008, 2009, 2010 Edward Rosten
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met :
// 
// 
// *Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 
// *Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and / or other materials provided with the distribution.
// 
// *Neither the name of the University of Cambridge nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO,
// 	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// 	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// 	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING
// 		NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// 	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
//
//
// KORAL is licensed under the MIT License : https://opensource.org/licenses/mit-license.php
// 
// Copyright(c) 2016 Kareem Omar, Christopher Parker
// 
// Permission is hereby granted, free of charge,
// to any person obtaining a copy of this software and associated documentation
// files(the "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish, distribute,
// sublicense, and / or sell copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// 
// Note again that KORAL is a work in progress.
// Suggestions and improvements are welcomed.
//

#ifndef KORAL_CLATCH
#define KORAL_CLATCH

#pragma once

#include "cuda_runtime.h"

#include <cstdint>

#ifdef __INTELLISENSE__
#define asm(x)
#include <algorithm>
#include "device_launch_parameters.h"
#define __CUDACC__
#include "device_functions.h"
#undef __CUDACC__
#endif

#include "Keypoint.h"

void CLATCH(cudaTextureObject_t d_all_tex[8], const cudaTextureObject_t d_triplets, const koral::Keypoint* const __restrict d_kps, const int num_kps, uint64_t* const __restrict d_desc);

constexpr uint16_t triplets[2048]={2184,3253,596,0,3262,1099,2762,0,2560,3204,1328,0,2239,2969,3607,0,1414,2839,835,0,1131,675,2500,0,3273,1349,609,0,1698,3504,1852,0,2643,1542,2539,0,3408,2928,959,0,1481,3581,697,0,1106,610,3495,0,2762,2028,744,0,1980,483,3570,0,3000,748,1484,0,2242,1467,3823,0,603,810,2618,0,2716,2625,766,0,3429,471,2425,0,3695,1324,1115,0,1013,1124,1302,0,1205,1127,763,0,2820,1242,872,0,1848,2696,3483,0,1883,1029,2459,0,1175,3270,3767,0,1519,1753,3542,0,3062,455,3207,0,3001,400,3710,0,1810,2476,2241,0,2381,980,1738,0,2814,2466,3469,0,2474,3751,526,0,2246,3620,3536,0,2891,1413,2393,0,1615,1195,3563,0,3200,484,1474,0,2694,1828,2618,0,3247,3484,1269,0,1267,3062,3000,0,2282,871,2348,0,2477,544,3421,0,2750,2346,875,0,2825,2485,800,0,3360,3413,1601,0,663,1614,1961,0,3221,536,2605,0,3036,1133,365,0,454,2833,3556,0,1173,3556,528,0,546,3285,1109,0,2971,1469,956,0,3217,747,1056,0,3466,3795,1959,0,3245,1911,2037,0,1015,3791,3606,0,2846,470,410,0,732,473,1529,0,3360,965,731,0,3610,3550,2319,0,1993,2125,1414,0,3830,2267,512,0,2963,3493,1085,0,2061,952,845,0,2068,1401,3721,0,1131,535,2925,0,735,989,1676,0,1768,3845,1233,0,3286,2180,1055,0,1412,3245,2933,0,2338,962,1977,0,3494,806,756,0,2560,3696,1411,0,1893,3862,3322,0,3282,701,1838,0,1692,2925,2197,0,3776,3215,409,0,3219,3415,1385,0,3038,2776,2752,0,1459,3070,1168,0,3219,2610,1489,0,2135,3258,2926,0,843,1882,1061,0,3579,3264,480,0,3146,1541,2244,0,2126,2335,2988,0,3420,2681,402,0,1320,1382,2743,0,3864,2569,1349,0,2539,1975,588,0,912,3650,1202,0,2322,1737,1532,0,674,3766,1468,0,3626,1275,2332,0,1810,1903,3317,0,2821,2191,3538,0,3620,873,3684,0,2603,3476,1745,0,2571,1621,3712,0,2813,1042,653,0,3567,1807,686,0,2525,1181,1014,0,699,3842,2496,0,471,1468,1765,0,1676,2140,3031,0,1604,1204,660,0,2641,3176,1779,0,3869,3677,1265,0,1197,2891,1694,0,1459,3199,3431,0,2315,3846,3533,0,2526,1231,2604,0,802,3683,3051,0,3823,2697,2239,0,1260,3495,3649,0,1595,2696,2461,0,1089,1473,1524,0,1678,3393,1881,0,2405,1545,1559,0,806,534,2820,0,3061,3362,1925,0,2851,395,1414,0,2671,1553,3755,0,1528,3434,2894,0,3361,2628,394,0,1246,3126,3350,0,2925,1949,3653,0,2775,3182,3290,0,2323,1339,2671,0,1203,2383,2347,0,3628,1132,891,0,1629,454,2568,0,554,2534,1835,0,2632,604,1819,0,451,3771,2464,0,368,2927,2469,0,371,988,3248,0,2541,1200,3750,0,437,3037,1324,0,1374,2201,2237,0,663,1327,3825,0,1828,553,743,0,2273,978,2204,0,3064,2813,3653,0,1243,2845,3832,0,2860,1096,2853,0,3701,3536,3413,0,2645,2608,3503,0,379,653,1744,0,1848,3629,3290,0,1037,3832,2763,0,3473,2539,959,0,2395,529,1527,0,1637,3827,2065,0,2639,2325,1348,0,1665,3330,744,0,2129,1326,977,0,2067,1899,3677,0,3358,3334,1270,0,1663,3108,2903,0,1170,1465,819,0,3245,1769,3037,0,3753,1412,951,0,3074,2099,1265,0,3107,513,2963,0,3613,1904,1020,0,3209,764,2410,0,1556,3541,3790,0,3464,1980,2182,0,3031,3506,1376,0,2126,482,395,0,3029,3273,1157,0,813,3788,3762,0,2855,3775,2564,0,1095,2787,3320,0,1256,2093,1899,0,3431,2612,3360,0,1345,2097,2416,0,1618,544,1775,0,446,3725,3396,0,1734,1094,2902,0,1610,3344,1177,0,768,3843,2206,0,607,3565,1264,0,2931,1100,1837,0,2242,1129,3751,0,902,509,3350,0,611,2116,3409,0,1189,452,3065,0,1980,2464,1773,0,442,3782,2755,0,894,413,2337,0,1767,2043,3209,0,2931,3563,1463,0,557,3781,1390,0,2696,1690,3127,0,1266,2899,2916,0,1197,1606,1274,0,1550,3427,1474,0,545,3051,3637,0,2048,1970,3475,0,1528,1764,872,0,1125,529,3359,0,2274,1996,2489,0,2177,2761,1959,0,3684,1475,1169,0,2252,2067,1373,0,3111,2526,1888,0,1230,2988,3246,0,3551,3705,680,0,1042,2995,1184,0,552,1242,701,0,3350,3617,3861,0,1449,3259,591,0,670,2814,1537,0,2567,3831,2857,0,603,969,1983,0,1235,3114,590,0,1627,878,1991,0,3425,2116,2641,0,3112,3761,3396,0,1634,1032,1709,0,2190,2781,3408,0,2355,3409,2715,0,2755,2411,373,0,826,1249,371,0,733,3407,3754,0,1848,1374,1204,0,2706,3479,1704,0,901,2100,3713,0,3709,540,555,0,483,874,3288,0,2981,3650,546,0,2314,950,2177,0,388,3507,3052,0,2612,3859,2039,0,1883,522,1739,0,2752,3414,2636,0,2976,399,3725,0,1030,2333,3844,0,986,1734,2712,0,3431,953,1771,0,3465,1989,729,0,1161,3651,1878,0,768,3829,2644,0,2458,3556,3215,0,1338,2355,1133,0,1450,2995,3318,0,2039,547,1460,0,840,3772,1376,0,2211,1270,475,0,1548,2693,3626,0,2285,1683,1277,0,3133,1903,2558,0,379,3410,1746,0,2969,1737,1678,0,588,1035,1020,0,1042,482,1262,0,2827,2415,2613,0,2428,2906,2414,0,604,3692,1808,0,1889,408,2102,0,2741,3333,1594,0,409,1309,3506,0,1120,3752,624,0,2644,3332,1705,0,3611,888,2914,0,962,1166,2888,0,2347,2259,873,0,589,1920,3185,0,3578,1704,2050,0,3002,3782,1470,0,2630,2543,2139,0,609,1843,3436,0,383,1169,2742,0,3643,771,1475,0,3699,957,3768,0,3482,2860,818,0,3607,2988,1234,0,1676,412,2605,0,3630,3203,1915,0,3417,2835,3546,0,680,1204,3796,0,1264,3506,1561,0,1843,3715,1122,0,1267,2644,3639,0,679,1185,1336,0,1195,3853,1125,0,1532,1918,3616,0,3252,2121,3828,0,3391,374,2692,0,1381,3694,2463,0,390,1480,2836,0,2772,1817,3427,0,1922,476,1762,0,3861,1333,3791,0,1270,3052,3503,0,2963,1119,3831,0,3553,2976,3548,0,1557,672,1628,0,2813,1330,1669,0,1016,3137,1013,0,2570,1756,3504,0,2861,2993,1040,0,1159,1830,2388,0,1272,3117,483,0,1840,1388,2277,0,3605,597,3130,0,1339,1520,555,0,1734,1324,3072,0,1266,3418,393,0,3688,2767,3471,0,3714,3470,902,0,2474,1977,1106,0,616,1039,2265,0,2028,3192,661,0,1628,677,2124,0,2979,772,1396,0,818,3210,480,0,2816,2338,3394,0,1236,3494,3829,0,369,532,1445,0,1769,733,1703,0,449,2703,1317,0,3646,1173,2924,0,3136,485,2560,0,3716,468,3144,0,3409,2642,2475,0,2860,413,2847,0,3831,2329,2240,0,1687,2645,3851,0,1555,1021,2927,0,2096,3117,3753,0,3678,2632,3677,0,3492,2141,3782,0,1483,2615,481,0,2209,2774,3642,0,1314,3196,3782,0,2269,2828,3652,0,514,1977,3538,0,475,2209,3792,0,3713,3052,806,0,2933,2544,3002,0,2488,3195,2633,0,1165,2847,3326,0,1739,2318,2674,0,1481,3569,1267,0,808,1518,3031,0,836,3391,3141,0,1476,2284,985,0,444,2758,3749,0,542,960,1330,0,2887,1962,3105,0,400,3421,1125,0,2998,2622,3407,0,753,2413,3420,0,3475,460,442,0,2476,3076,390,0,733,3643,514,0,2468,3246,2324,0,2921,3218,3786,0,1232,412,872,0,591,3045,2040,0,3502,1853,2559,0,554,1949,1554,0,1708,1741,1058,0,1529,1699,2466,0,2760,470,3861,0,2705,3461,2283,0,3763,2333,551,0,658,3200,2537,0,2454,1920,3750,0,2861,3853,1905,0,469,1988,3564,0,2424,407,2572,0,3260,1950,2255,0,2055,3636,2633,0,3263,2479,2569,0,1120,2768,1249,0,546,3756,3208,0,3712,2406,1918,0,2927,1100,482,0,1892,3139,2391,0,2129,385,3290,0,1016,467,661,0,691,1159,2189,0,2124,1025,1338,0,3357,2045,3639,0,835,3847,1748,0,1673,2716,2755,0,3861,1397,951,0,2960,591,3030,0,368,2282,1016,0,2694,2712,474,0,741,395,2638,0,2813,3575,368,0,2120,3712,1249,0,1909,3346,2855,0,3392,551,1664,0,3836,895,2400,0,2479,395,3634,0,913,3245,1906,0,2467,531,1474,0,1250,2392,3752,0,3851,3061,1544,0,1690,2856,3712,0,1379,838,2819,0,3361,1922,3434,0,2238,620,1378,0,805,2283,3683,0,3487,1541,2626,0,2410,1088,2265,0,3469,3198,2423,0,1670,819,1521,0,586,2830,461,0,3686,2622,2101,0,1273,3425,2211,0,1996,3787,1986,0,2050,1629,2768,0,1314,688,448,0,2239,470,1881,0,2335,3844,3502,0,2322,3558,3533,0,2966,1101,1305,0,1129,1101,2784,0,509,2780,1821,0,1635,3845,1901,0,912,2172,2932,0,797,3200,1597,0,2127,3763,2414,0,2774,679,476,0,765,2704,3780,0,910,807,1130,0,379,2272,3043,0,3221,1908,2907,0,3609,2423,1604,0,2832,1667,2886,0,2054,389,1266,0,2062,471,3198,0,1997,1821,3149,0,2531,389,479,0,1485,802,2056,0,1877,3696,3174,0,1014,2765,2962,0,1400,3690,608,0,1034,656,3471,0,627,1333,3774,0,884,1465,1963,0,3572,409,2342,0,949,2136,586,0,2782,1265,2917,0,3000,2238,2129,0,989,522,773,0,1991,2478,454,0,615,2212,1399,0,1340,2381,2205,0,1197,1665,3067,0,2561,1268,687,0,1734,3642,2171,0,2026,598,1014,0,1520,1604,1019,0,893,1271,2835,0,3607,1107,3720,0,2213,3635,767,0,2995,2319,3356,0,1167,2474,2985,0,1337,1491,485,0,2765,688,2200,0,3281,1841,2783,0,912,2907,3678,0,1844,3354,1923,0,1964,1120,1744,0,1834,3401,1267,0,2714,463,2674,0,883,2981,3493,0,839,607,3063,0,2418,746,2208,0,1376,2053,437,0,2783,3566,2342,0,378,479,2328,0,1166,2998,3466,0,1122,1040,474,0,1407,1950,3285,0,591,3462,1679,0,3624,1527,2907,0,2562,405,3140,0,584,2181,3124,0,768,2491,3638,0,1693,2963,2206,0,1306,1195,3752,0,833,890,1118,0,3281,1346,3504,0,3678,386,3571,0,1750,834,443,0,1015,812,3259,0,3408,2825,961,0,1406,2534,3286,0,1701,1612,657,0,2021,2675,1960,0,549,2915,1271,0,3273,662,2550,0,3790,3199,1850,0,2354,3605,2706,0,2041,3489,1601,0,1264,610,1179,0,3538,1962,369,0,3568,1828,458,0,2272,2620,1987,0,3757,1540,1092,0,1058,3491,399,0,476,1697,2910,0,3322,2136,1523,0};


#endif /* KORAL_CLATCH */
