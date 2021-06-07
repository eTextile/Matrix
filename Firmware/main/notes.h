/*
  This file is part of the E256 - eTextile matrix sensor project - http://matrix.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __NOTES_H__
#define __NOTES_H__

#include "config.h"
#if HARDWARE_MIDI
#define  Cm1   0         // 8.18 Hz
#define  Cdm1  1         // 8.66 Hz
#define  Dm1   2         // 9.18 Hz
#define  Ddm1  3         // 9.72 Hz
#define  Em1   4         // 10.30 Hz
#define  Fm1   5         // 10.91 Hz
#define  Fdm1  6         // 11.56 Hz
#define  Gm1   7         // 12.25 Hz
#define  Gdm1  8         // 12.98 Hz
#define  Am1   9         // 13.75 Hz
#define  Adm1  10        // 14.57 Hz
#define  Bm1   11        // 15.43 Hz
#define  C0    12        // 16.35 Hz
#define  Cd0   13        // 17.32 Hz
#define  D0    14        // 18.35 Hz
#define  Dd0   15        // 19.44 Hz
#define  E0    16        // 20.60 Hz
#define  F0    17        // 21.83 Hz
#define  Fd0   18        // 23.13 Hz
#define  G0    19        // 24.50 Hz
#define  Gd0   20        // 25.98 Hz
#define  A0_   21        // 27.50 Hz
#define  Ad0   22        // 29.14 Hz
#define  B0_   23        // 30.87 Hz
#define  C1    24        // 32.70 Hz
#define  Cd1   25        // 34.65 Hz
#define  D1    26        // 36.71 Hz
#define  Dd1   27        // 38.89 Hz
#define  E1    28        // 41.20 Hz
#define  F1    29        // 43.65 Hz
#define  Fd1   30        // 46.25 Hz
#define  G1    31        // 40.00 Hz
#define  Gd1   32        // 51.91 Hz
#define  A1_   33        // 55.00 Hz
#define  Ad1   34        // 58.27 Hz
#define  B1_   35        // 61.74 Hz
#define  C2    36        // 65.41 Hz
#define  Cd2   37        // 69.30 Hz
#define  D2    38        // 73.42 Hz
#define  Dd2   39        // 77.78 Hz
#define  E2    40        // 82.41 Hz
#define  F2    41        // 87.31 Hz
#define  Fd2   42        // 92.50 Hz
#define  G2    43        // 98.00 Hz
#define  Gd2   44        // 103.83 Hz
#define  A2_   45        // 110.00 Hz
#define  Ad2   46        // 116.54 Hz
#define  B2    47        // 123.47 Hz
#define  C3    48        // 130.81 Hz
#define  Cd3   49        // 138.59 Hz
#define  D3    50        // 146.83 Hz
#define  Dd3   51        // 155.56 Hz
#define  E3    52        // 164.81 Hz
#define  F3    53        // 174.61 Hz
#define  Fd3   54        // 185.00 Hz
#define  G3    55        // 196.00 Hz
#define  Gd3   56        // 207.65 Hz
#define  A3_   57        // 220.00 Hz
#define  Ad3   58        // 233.08 Hz
#define  B3    59        // 246.94 Hz
#define  C4    60        // 261.63 Hz
#define  Cd4   61        // 277.18 Hz
#define  D4    62        // 293.67 Hz
#define  Dd4   63        // 311.13 Hz
#define  E4    64        // 329.63 Hz
#define  F4    65        // 349.23 Hz
#define  Fd4   66        // 369.99 Hz
#define  G4    67        // 392.00 Hz
#define  Gd4   68        // 415.31 Hz
#define  A4_   69        // 440.00 Hz
#define  Ad4   70        // 466.16 Hz
#define  B4    71        // 493.88 Hz
#define  C5    72        // 523.25 Hz
#define  Cd5   73        // 554.37 Hz
#define  D5    74        // 587.33 Hz
#define  Dd5   75        // 622.25 Hz
#define  E5    76        // 659.26 Hz
#define  F5    77        // 698.46 Hz
#define  Fd5   78        // 739.99 Hz
#define  G5    79        // 784.00
#define  Gd5   80        // 830.61
#define  A5_   81        // 880.00
#define  Ad5   82        // 932.33
#define  B5    83        // 987.77
#define  C6    84        // 1046.50
#define  Cd6   85        // 1108.73
#define  D6    86        // 1174.66
#define  Dd6   87        // 1244.51
#define  E6    88        // 1318.51
#define  F6    89        // 1396.91
#define  Fd6   90        // 1479.98
#define  G6    91        // 1567.98
#define  Gd6   92        // 1661.22
#define  A6_   93        // 1760.00
#define  Ad6   94        // 1864.66
#define  B6    95        // 1975.53
#define  C7    96        // 2093.01
#define  Cd7   97        // 2217.46
#define  D7    98        // 2349.32
#define  Dd7   99        // 2489.02
#define  E7    100       // 2637.02
#define  F7    101       // 2793.83
#define  Fd7   102       // 2959.96
#define  G7    103       // 3135.96
#define  Gd7   104       // 3322.44
#define  A7_   105       // 3520.00
#define  Ad7   106       // 3729.31
#define  B7    107       // 3951.07
#define  C8    108       // 4186.01
#define  Cd8   109       // 4434.92
#define  D8    110       // 4698.64
#define  Dd8   111       // 4978.03
#define  E8    112       // 5274.04
#define  F8    113       // 5587.65
#define  Fd8   114       // 5919.91
#define  G8    115       // 6271.93
#define  Ad8   116       // 6644.88
#define  A8_   117       // 7040.00
#define  Ad8   118       // 7458.62
#define  B8    119       // 7902.13
#define  C9    120       // 8372.02
#define  Cd9   121       // 8869.84
#define  D9    122       // 9397.27
#define  Dd9   123       // 9956.06
#define  E9    124       // 10548.08
#define  F9    125       // 11175.30
#define  Fd9   126       // 11839.82
#define  G9    127       // 12543.85

const int8_t note[128] = {
  Cm1, Cdm1, Dm1, Ddm1, Em1, Fm1, Fdm1, Gm1, Gdm1, Am1, Adm1, Bm1,
  C0,  Cd0,  D0,  Dd0,  E0,  F0,  Fd0,  G0,  Gd0,  A0_, Ad0,  B0_,
  C1,  Cd1,  D1,  Dd1,  E1,  F1,  Fd1,  G1,  Gd1,  A1_, Ad1,  B1_,
  C2,  Cd2,  D2,  Dd2,  E2,  F2,  Fd2,  G2,  Gd2,  A2_, Ad2,  B2,
  C3,  Cd3,  D3,  Dd3,  E3,  F3,  Fd3,  G3,  Gd3,  A3_, Ad3,  B3,
  C4,  Cd4,  D4,  Dd4,  E4,  F4,  Fd4,  G4,  Gd4,  A4_, Ad4,  B4,
  C5,  Cd5,  D5,  Dd5,  E5,  F5,  Fd5,  G5,  Gd5,  A5_, Ad5,  B5,
  C6,  Cd6,  D6,  Dd6,  E6,  F6,  Fd6,  G6,  Gd6,  A6_, Ad6,  B6,
  C7,  Cd7,  D7,  Dd7,  E7,  F7,  Fd7,  G7,  Gd7,  A7_, Ad7,  B7,
  C8,  Cd8,  D8,  Dd8,  E8,  F8,  Fd8,  G8,  Ad8,  A8_, Ad8,  B8,
  C9,  Cd9,  D9,  Dd9,  E9,  F9,  Fd9,  G9
};

//See: https://www.c-thru-music.com/cgi/?page=layout_octaves
const int8_t harmonicKeyboard[128] = {
  Gd7, F7,  A7_, Fd7, Ad7, G7,  B7,  Gd7, C8,  A7_, Cd8, Ad7, D8,  B7,
  Cd7, Ad6, D7,  B6,  Dd7, C7,  E7,  Cd7, F7,  D7,  Fd7, Dd7, G7,  E7,
  Fd6, Dd6, G6,  E6,  Gd6, F6,  A6_, Fd6, Ad6, G6,  B6,  Gd6, C7,  A6_,
  B5,  Gd5, C6,  A5_, Cd6, Ad5, D6,  B5,  Dd6, C6,  E6,  Cd6, F6,  D6,
  E5,  Cd5, F5,  D5,  Fd5, Dd5, G5,  E5,  Gd5, F5,  A5_, Fd5, Ad5, G5,
  A4_, Fd4, Ad4, G4,  B4,  Gd4, C5,  A4_, Cd5, Ad4, D5,  B4,  Dd5, C5,
  D4,  B3,  Dd4, C4,  E4,  Cd4, F4,  D4,  Fd4, Dd4, G4,  E4,  Gd4, F4,
  G3,  E3,  Gd3, F3,  A3_, Fd3, Ad3, G3,  B3,  Gd3, C4,  A3_, Cd4, Ad3,
  C3, A2_,  Cd3, Ad2, D3,  B2,  Dd3, C3,  E3,  Cd3, F3,  D3,  Fd3, Dd3,
};
#endif

#if SYNTH_PLAYER
#define  Cm1   8.18      // note[0]
#define  Cdm1  8.66      // note[1]
#define  Dm1   9.18      // note[2]
#define  Ddm1  9.72      // note[3]
#define  Em1   10.30     // note[4]
#define  Fm1   10.91     // note[5]
#define  Fdm1  11.56     // note[6]
#define  Gm1   12.25     // note[7]
#define  Gdm1  12.98     // note[8]
#define  Am1   13.75     // note[9]
#define  Adm1  14.57     // note[10]
#define  Bm1   15.43     // note[11]
#define  C0    16.35     // note[12]
#define  Cd0   17.32     // note[13]
#define  D0    18.35     // note[14]
#define  Dd0   19.44     // note[15]
#define  E0    20.60     // note[16]
#define  F0    21.83     // note[17]
#define  Fd0   23.13     // note[18]
#define  G0    24.50     // note[19]
#define  Gd0   25.98     // note[20]
#define  A0_   27.50     // note[21]
#define  Ad0   29.14     // note[22]
#define  B0_   30.87     // note[23]
#define  C1    32.70     // note[24]
#define  Cd1   34.65     // note[25]
#define  D1    36.71     // note[26]
#define  Dd1   38.89     // note[27]
#define  E1    41.20     // note[28]
#define  F1    43.65     // note[29]
#define  Fd1   46.25     // note[30]
#define  G1    40.00     // note[31]
#define  Gd1   51.91     // note[32]
#define  A1_   55.00     // note[33]
#define  Ad1   58.27     // note[34]
#define  B1_   61.74     // note[35]
#define  C2    65.41     // note[36]
#define  Cd2   69.30     // note[37]
#define  D2    73.42     // note[38]
#define  Dd2   77.78     // note[39]
#define  E2    82.41     // note[40]
#define  F2    87.31     // note[41]
#define  Fd2   92.50     // note[42]
#define  G2    98.00     // note[43]
#define  Gd2   103.83    // note[44]
#define  A2_   110.00    // note[45]
#define  Ad2   116.54    // note[46]
#define  B2    123.47    // note[47]
#define  C3    130.81    // note[48]
#define  Cd3   138.59    // note[49]
#define  D3    146.83    // note[50]
#define  Dd3   155.56    // note[51]
#define  E3    164.81    // note[52]
#define  F3    174.61    // note[53]
#define  Fd3   185.00    // note[54]
#define  G3    196.00    // note[55]
#define  Gd3   207.65    // note[56]
#define  A3_   220.00    // note[57]
#define  Ad3   233.08    // note[58]
#define  B3    246.94    // note[59]
#define  C4    261.63    // note[60]
#define  Cd4   277.18    // note[61]
#define  D4    293.67    // note[62]
#define  Dd4   311.13    // note[63]
#define  E4    329.63    // note[64]
#define  F4    349.23    // note[65]
#define  Fd4   369.99    // note[66]
#define  G4    392.00    // note[67]
#define  Gd4   415.31    // note[68]
#define  A4_   440.00    // note[69]
#define  Ad4   466.16    // note[70]
#define  B4    493.88    // note[71]
#define  C5    523.25    // note[72]
#define  Cd5   554.37    // note[73]
#define  D5    587.33    // note[74]
#define  Dd5   622.25    // note[75]
#define  E5    659.26    // note[76]
#define  F5    698.46    // note[77]
#define  Fd5   739.99    // note[78]
#define  G5    784.00    // note[79]
#define  Gd5   830.61    // note[80]
#define  A5_   880.00    // note[81]
#define  Ad5   932.33    // note[82]
#define  B5    987.77    // note[83]
#define  C6    1046.50   // note[84]
#define  Cd6   1108.73   // note[85]
#define  D6    1174.66   // note[86]
#define  Dd6   1244.51   // note[87]
#define  E6    1318.51   // note[88]
#define  F6    1396.91   // note[89]
#define  Fd6   1479.98   // note[90]
#define  G6    1567.98   // note[91]
#define  Gd6   1661.22   // note[92]
#define  A6_   1760.00   // note[93]
#define  Ad6   1864.66   // note[94]
#define  B6    1975.53   // note[95]
#define  C7    2093.01   // note[96]
#define  Cd7   2217.46   // note[97]
#define  D7    2349.32   // note[98]
#define  Dd7   2489.02   // note[99]
#define  E7    2637.02   // note[100]
#define  F7    2793.83   // note[101]
#define  Fd7   2959.96   // note[102]
#define  G7    3135.96   // note[103]
#define  Gd7   3322.44   // note[104]
#define  A7_   3520.00   // note[105]
#define  Ad7   3729.31   // note[106]
#define  B7    3951.07   // note[107]
#define  C8    4186.01   // note[108]
#define  Cd8   4434.92   // note[109]
#define  D8    4698.64   // note[110]
#define  Dd8   4978.03   // note[111]
#define  E8    5274.04   // note[112]
#define  F8    5587.65   // note[113]
#define  Fd8   5919.91   // note[114]
#define  G8    6271.93   // note[115]
#define  Ad8   6644.88   // note[116]
#define  A8_   7040.00   // note[117]
#define  Ad8   7458.62   // note[118]
#define  B8    7902.13   // note[119]
#define  C9    8372.02   // note[120]
#define  Cd9   8869.84   // note[121]
#define  D9    9397.27   // note[122]
#define  Dd9   9956.06   // note[123]
#define  E9    10548.08  // note[124]
#define  F9    11175.30  // note[125]
#define  Fd9   11839.82  // note[126]
#define  G9    12543.85  // note[127]

const float note[128] = {
  Cm1, Cdm1, Dm1, Ddm1, Em1, Fm1, Fdm1, Gm1, Gdm1, Am1, Adm1, Bm1,
  C0,  Cd0,  D0,  Dd0,  E0,  F0,  Fd0,  G0,  Gd0,  A0_, Ad0,  B0_,
  C1,  Cd1,  D1,  Dd1,  E1,  F1,  Fd1,  G1,  Gd1,  A1_, Ad1,  B1_,
  C2,  Cd2,  D2,  Dd2,  E2,  F2,  Fd2,  G2,  Gd2,  A2_, Ad2,  B2,
  C3,  Cd3,  D3,  Dd3,  E3,  F3,  Fd3,  G3,  Gd3,  A3_, Ad3,  B3,
  C4,  Cd4,  D4,  Dd4,  E4,  F4,  Fd4,  G4,  Gd4,  A4_, Ad4,  B4,
  C5,  Cd5,  D5,  Dd5,  E5,  F5,  Fd5,  G5,  Gd5,  A5_, Ad5,  B5,
  C6,  Cd6,  D6,  Dd6,  E6,  F6,  Fd6,  G6,  Gd6,  A6_, Ad6,  B6,
  C7,  Cd7,  D7,  Dd7,  E7,  F7,  Fd7,  G7,  Gd7,  A7_, Ad7,  B7,
  C8,  Cd8,  D8,  Dd8,  E8,  F8,  Fd8,  G8,  Ad8,  A8_, Ad8,  B8,
  C9,  Cd9,  D9,  Dd9,  E9,  F9,  Fd9,  G9
};

//See: https://www.c-thru-music.com/cgi/?page=layout_octaves
const float harmonicKeyboard[128] = {
  Gd7, F7,  A7_, Fd7, Ad7, G7,  B7,  Gd7, C8,  A7_, Cd8, Ad7, D8,  B7,
  Cd7, Ad6, D7,  B6,  Dd7, C7,  E7,  Cd7, F7,  D7,  Fd7, Dd7, G7,  E7,
  Fd6, Dd6, G6,  E6,  Gd6, F6,  A6_, Fd6, Ad6, G6,  B6,  Gd6, C7,  A6_,
  B5,  Gd5, C6,  A5_, Cd6, Ad5, D6,  B5,  Dd6, C6,  E6,  Cd6, F6,  D6,
  E5,  Cd5, F5,  D5,  Fd5, Dd5, G5,  E5,  Gd5, F5,  A5_, Fd5, Ad5, G5,
  A4_, Fd4, Ad4, G4,  B4,  Gd4, C5,  A4_, Cd5, Ad4, D5,  B4,  Dd5, C5,
  D4,  B3,  Dd4, C4,  E4,  Cd4, F4,  D4,  Fd4, Dd4, G4,  E4,  Gd4, F4,
  G3,  E3,  Gd3, F3,  A3_, Fd3, Ad3, G3,  B3,  Gd3, C4,  A3_, Cd4, Ad3,
  C3, A2_,  Cd3, Ad2, D3,  B2,  Dd3, C3,  E3,  Cd3, F3,  D3,  Fd3, Dd3,
};
#endif

#endif /*__NOTES_H__*/
