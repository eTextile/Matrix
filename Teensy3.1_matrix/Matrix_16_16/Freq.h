// freqs to midi notes : http://newt.phys.unsw.edu.au/jw/notes.html
// notes to freqs : http://www.phy.mtu.edu/~suits/notefreqs.html

#if defined( STANDALONE )
  #define C1  32.70
  #define D1  36.71
  #define E1  41.20
  #define F1  43.65
  #define G1  49.00
  #define A_1 55.00
  #define B1  61.74
  #define C2  65.41
  #define D2  73.42
  #define E2  82.41
  #define F2  87.31
  #define G2  98.00
  #define A_2 110.00
  #define B2  123.47
  #define C3  130.81
  #define D3  146.83
  #define E3  164.81
  #define F3  174.61
  #define G3  196.00
  #define A_3 220.00
  #define B3  246.94
  #define C4  261.63
  #define D4  293.66
  #define E4  329.63
  #define F4  349.23
  #define G4  392.00
  #define A_4 440.00
  #define B4  493.88
  #define C5  523.25
  #define D5  587.33
  #define E5  659.25
  #define F5  698.46
  #define G5  783.99
  #define A_5 880.00
  #define B5  987.77
  #define C6  1046.50
  #define D6  1174.66
  #define E6  1318.51
  #define F6  1396.91
  #define G6  1567.98
  #define A_6 1760.00
  #define B6  1975.53
  #define C7  2093.00
  #define D7  2349.32
  #define E7  2637.02
  #define F7  2793.83
  #define G7  3135.96
  #define A_7 3520.00
  #define B7  3951.07
  #define C8  4186.01
  #define D8  4698.63
  #define E8  5274.04
  #define F8  5587.65
  #define G8  6271.93
  #define A_8 7040.00
  #define B8  7902.13

float freq[ROW][COLUMN] = {
  { C8, D8, E8, F8, G8, A_8, B8, C8, D8, E8, F8, G8, A_8, B8, -1, -1 },
  { C7, D7, E7, F7, G7, A_7, B7, C7, D7, E7, F7, G7, A_7, B7, -1, -1 },
  { C6, D6, E6, F6, G6, A_6, B6, C6, D6, E6, F6, G6, A_6, B6, -1, -1 },
  { C5, D5, E5, F5, G5, A_5, B5, C5, D5, E5, F5, G5, A_5, B5, -1, -1 },
  { C4, D4, E4, F4, G4, A_4, B4, C4, D4, E4, F4, G4, A_4, B4, -1, -1 },
  { C3, D3, E3, F3, G3, A_3, B3, C3, D3, E3, F3, G3, A_3, B3, -1, -1 },
  { C2, D2, E2, F2, G2, A_2, B2, C2, D2, E2, F2, G2, A_2, B2, -1, -1 },
  { C1, D1, E1, F1, G1, A_1, B1, C1, D1, E1, F1, G1, A_1, B1, -1, -1 },
  { C8, D8, E8, F8, G8, A_8, B8, C8, D8, E8, F8, G8, A_8, B8, -1, -1 },
  { C7, D7, E7, F7, G7, A_7, B7, C7, D7, E7, F7, G7, A_7, B7, -1, -1 },
  { C6, D6, E6, F6, G6, A_6, B6, C6, D6, E6, F6, G6, A_6, B6, -1, -1 },
  { C5, D5, E5, F5, G5, A_5, B5, C5, D5, E5, F5, G5, A_5, B5, -1, -1 },
  { C4, D4, E4, F4, G4, A_4, B4, C4, D4, E4, F4, G4, A_4, B4, -1, -1 },
  { C3, D3, E3, F3, G3, A_3, B3, C3, D3, E3, F3, G3, A_3, B3, -1, -1 },
  { C2, D2, E2, F2, G2, A_2, B2, C2, D2, E2, F2, G2, A_2, B2, -1, -1 },
  { C1, D1, E1, F1, G1, A_1, B1, C1, D1, E1, F1, G1, A_1, B1, -1, -1 }
};

// WAVEFORM_SINE      0
// WAVEFORM_SAWTOOTH  1
// WAVEFORM_SQUARE    2
// WAVEFORM_TRIANGLE  3
// WAVEFORM_ARBITRARY 4
// WAVEFORM_PULSE     5

short int wave[ROW][COLUMN] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
  { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
  { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
  { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
  { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 },
  { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 },
  { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 },
  { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 },
  { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 },
  { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 },
  { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 },
  { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 },
  { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 },
};

#endif
