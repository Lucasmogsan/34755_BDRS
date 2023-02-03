/*===================*/
/* NOTES DEFINITIONS */
/*===================*/

#ifndef MELODIES_h
#define MELODIES_h

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  205
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 2061
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

//for 12/8 -> change to 12/8 mode ... :/

//tunes is refenced to the length 4 beats (for 12/8)
#define d8 12  // 1/8 in 12/8
#define d16	32// 1/16 in 12/8
#define d38 4 //3/8
#define d12 2 //6/8
#define d28 6 // 2/8
#define d316 10 //3/16

/*============================*/
/* GAME OF THRONES THEME SONG */
/*============================*/

//game of thrones :D
//1 line = 12/8
uint16_t game_melody[]{
	NOTE_G4, NOTE_C3, NOTE_DS4, NOTE_F4,	NOTE_G4,NOTE_C3,NOTE_DS4,NOTE_F4,	NOTE_G4,NOTE_C3,NOTE_DS4,NOTE_F4,	NOTE_G4,NOTE_C3,NOTE_DS4,NOTE_F4, //same 4 tones repeated
	NOTE_G4, NOTE_C3, NOTE_E4, NOTE_F4,		NOTE_G4, NOTE_C3, NOTE_E4, NOTE_F4,	NOTE_G4, NOTE_C3, NOTE_E4, NOTE_F4,	NOTE_G4, NOTE_C3, NOTE_E4, NOTE_F4, //same 4 tones repeated

	//3
	NOTE_G4,	NOTE_C3,	NOTE_DS4, NOTE_F4, NOTE_G4,		NOTE_C3, NOTE_DS4, NOTE_F4,
	NOTE_D3, NOTE_G3, NOTE_AS3, NOTE_C3, NOTE_D3, NOTE_G3, NOTE_AS3, NOTE_C3,	NOTE_D3, NOTE_G3, NOTE_AS3, NOTE_C3,	NOTE_D3, NOTE_G3, NOTE_AS3, NOTE_C3, //same 4 tones repeated

	//5
	NOTE_F4, NOTE_AS3,	NOTE_DS4,NOTE_D4,NOTE_F4,NOTE_AS3,
	NOTE_DS4,NOTE_D4,NOTE_C3,NOTE_GS4,NOTE_AS4,		NOTE_C4, NOTE_G4,NOTE_GS4,NOTE_AS4,		 NOTE_C4,NOTE_G4,NOTE_GS4,NOTE_AS4,	   NOTE_C4,NOTE_G4,NOTE_GS4,NOTE_AS4, //last 3 groups repeated

	//7
	//repeat 1 octave over																																					 //repeat 1 octave over
	NOTE_G5,	NOTE_C4,	NOTE_DS5, NOTE_F5, NOTE_G5,		NOTE_C4, NOTE_DS5, NOTE_F5,
	NOTE_D4, NOTE_G4, NOTE_AS4, NOTE_C4, NOTE_D4, NOTE_G4, NOTE_AS4, NOTE_C4,	NOTE_D4, NOTE_G4, NOTE_AS4, NOTE_C4,	NOTE_D4, NOTE_G4, NOTE_AS4, NOTE_C4, //same 4 tones repeated

	//9
	NOTE_F5, NOTE_AS4,	NOTE_DS5,NOTE_D5,NOTE_F5,NOTE_AS4,
	NOTE_DS5,NOTE_D5,NOTE_C4,NOTE_GS5,NOTE_AS5,		NOTE_C5, NOTE_G5,NOTE_GS5,NOTE_AS5,   NOTE_C5, NOTE_G5,NOTE_GS5,NOTE_AS5,   NOTE_C5, NOTE_G5,NOTE_GS5,NOTE_AS5, //last 3 groups repeated

	//11
	NOTE_G6,	NOTE_C5,	NOTE_DS6, NOTE_F6, NOTE_G6,		NOTE_C5, NOTE_DS6, NOTE_F6,
	NOTE_D5, NOTE_G5, NOTE_AS5, NOTE_C5, NOTE_D5, NOTE_G5, NOTE_AS5, NOTE_C5,	NOTE_D5, NOTE_G5, NOTE_AS5, NOTE_C5,	NOTE_D5, NOTE_G5, NOTE_AS5, NOTE_C5, //same 4 tones repeated

	//13
	NOTE_F6,NOTE_AS5, NOTE_D6, NOTE_DS6,NOTE_D6,NOTE_AS5,
	NOTE_C5, NOTE_G3,0

};

uint8_t game_tempo[]{
	d8, d8, d16, d16,	d8, d8, d16, d16,	d8, d8, d16, d16,	d8, d8, d16, d16,
	d8, d8, d16, d16,	d8, d8, d16, d16,	d8, d8, d16, d16,	d8, d8, d16, d16,

	d38, d38, d16, d16, d28, d28, d16, d16,
	d8, d8, d16, d16,	d8, d8, d16, d16,	d8, d8, d16, d16,	d8, d8, d16, d16,

	d38, d38, d16, d16, d28, d38,
	d16, d16, d28, d16, d16,	d8, d8, d16, d16,	d8, d8, d16, d16,	d8, d8, d16, d16,

	d38, d38, d16, d16, d28, d28, d16, d16,
	d8, d8, d16, d16,	d8, d8, d16, d16,	d8, d8, d16, d16,	d8, d8, d16, d16,

	d38, d38, d16, d16, d28, d38,
	d16, d16, d28, d16, d16,	d8, d8, d16, d16,	d8, d8, d16, d16,	d8, d8, d16, d16,

	d38, d38, d16, d16, d28, d28, d16, d16,
	d8, d8, d16, d16,	d8, d8, d16, d16,	d8, d8, d16, d16,	d8, d8, d16, d16,

	//13
	d38, d38, d316, d316, d316, d316,
	d8, 4, d8
};

/*===============*/
/* BILLIE JEAN 1 */
/*===============*/

//billiejean1
uint16_t billie_melody_1[] = {
	NOTE_DS5,NOTE_AS4,NOTE_C5,NOTE_DS5,NOTE_C5,NOTE_AS4,NOTE_G4,NOTE_AS4,0
};
//billiejean1 tempo
uint8_t billie_tempo_1[] = {
	8,8,8,8, 8,8,8,8
};

//billiejean2
//billiejean2 tempo

/*====================*/
/* SWEET HOME ALABAMA */
/*====================*/

// Sweet Home Alabama melody
uint16_t sweetHomeAlabama_melody[] = {
	NOTE_D3, NOTE_D3, NOTE_D4, NOTE_A3, NOTE_D3, NOTE_C3, NOTE_C3, NOTE_D4, NOTE_G4, NOTE_D3,
	NOTE_G3, NOTE_G3, NOTE_G4, NOTE_G4, NOTE_A3, NOTE_B3, NOTE_D3, NOTE_E3, NOTE_D3, NOTE_B3, NOTE_A4, NOTE_G4, 0
};

// Sweet Home Alabama tempo
uint8_t sweetHomeAlabama_tempo[] = {
	8, 8, 16, 8, 16, 8, 8, 16, 8, 16,
	8, 8, 6, 16, 16, 16, 16, 16, 16, 16, 16, 16, 64
};

/*=================*/
/* EPIC SAX MELODY */
/*=================*/

// Epic Sax melody
uint16_t sax_melody[] = {
	NOTE_B4, 0, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_A4, NOTE_B4,
	NOTE_B4, 0, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_A4, NOTE_B4,
	NOTE_B4, 0, NOTE_D5, NOTE_B4, 0, NOTE_A4,
	NOTE_G4, 0, NOTE_E4, NOTE_E4, NOTE_FS4, NOTE_G4, NOTE_E4
};

// Epic Sax tempo
uint8_t sax_tempo[] = {
	5, 5, 10, 20, 20, 20, 8,
	5, 5, 10, 20, 20, 20, 8,
	5, 10, 5, 10, 10, 5,
	10, 10, 10, 10, 10, 10, 10
};

/*===================*/
/* MARIO MAIN MELODY */
/*===================*/

// Mario main theme melody
uint16_t mario_main_melody[] = {
	NOTE_E7, NOTE_E7, 0, NOTE_E7,
	0, NOTE_C7, NOTE_E7, 0,
	NOTE_G7, 0, 0,  0,
	NOTE_G6, 0, 0, 0,

	NOTE_C7, 0, 0, NOTE_G6,
	0, 0, NOTE_E6, 0,
	0, NOTE_A6, 0, NOTE_B6,
	0, NOTE_AS6, NOTE_A6, 0,

	NOTE_G6, NOTE_E7, NOTE_G7,
	NOTE_A7, 0, NOTE_F7, NOTE_G7,
	0, NOTE_E7, 0, NOTE_C7,
	NOTE_D7, NOTE_B6, 0, 0,

	NOTE_C7, 0, 0, NOTE_G6,
	0, 0, NOTE_E6, 0,
	0, NOTE_A6, 0, NOTE_B6,
	0, NOTE_AS6, NOTE_A6, 0,

	NOTE_G6, NOTE_E7, NOTE_G7,
	NOTE_A7, 0, NOTE_F7, NOTE_G7,
	0, NOTE_E7, 0, NOTE_C7,
	NOTE_D7, NOTE_B6, 0, 0
};

// Mario main them tempo
uint8_t mario_main_tempo[] = {
	12, 12, 12, 12,
	12, 12, 12, 12,
	12, 12, 12, 12,
	12, 12, 12, 12,

	12, 12, 12, 12,
	12, 12, 12, 12,
	12, 12, 12, 12,
	12, 12, 12, 12,

	9, 9, 9,
	12, 12, 12, 12,
	12, 12, 12, 12,
	12, 12, 12, 12,

	12, 12, 12, 12,
	12, 12, 12, 12,
	12, 12, 12, 12,
	12, 12, 12, 12,

	9, 9, 9,
	12, 12, 12, 12,
	12, 12, 12, 12,
	12, 12, 12, 12,
};

/*================*/
/* WARNING MELODY */
/*================*/

// Warning melody
uint16_t warning_melody[] = {
	NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
	NOTE_AS3, NOTE_AS4, 0, 0, 0, 0,
	0, 0
};

// Warning melody tempo
uint8_t warning_tempo[] = {
	12, 12, 12, 12,
	12, 12,
	12, 12, 12, 12,
	12, 12
};

/*=====================*/
/* STARTUP/SUPER MARIO */
/*=====================*/

// Startup melody "Super Mario"
uint16_t startup_melody[] = {
	NOTE_E7, NOTE_E7, 0, NOTE_E7,
	0, NOTE_C7, NOTE_E7, 0,
	NOTE_G7, 0, 0,  0,
	NOTE_G6, 0, 0, 0
};

// Super Mario tempo
uint8_t startup_tempo[] = {
	12, 12, 12, 12,
	12, 12, 12, 12,
	12, 12, 12, 12,
	12, 12, 12, 12
};

/*==========================*/
/* STARTUP 2/IMPERIAL MARCH */
/*==========================*/

// Startup melody2 "Imperial March"
uint16_t startup_melody2[] = {
	NOTE_G5, NOTE_G5, NOTE_G5,
	NOTE_DS5, NOTE_AS5, NOTE_G5, NOTE_DS5,
	NOTE_AS5, NOTE_G5, 0
};

// Imperial march tempo
uint8_t startup_tempo2[] = {
	6, 6, 6,
	9, 12, 6, 9,
	12, 3, 12
};

/*=======================*/
/* DER ER ET YNDIGT LAND */
/*=======================*/

// Startup melody3 "Der er et yndigt land"
uint16_t startup_melody3[] = {
	NOTE_D7, NOTE_A7, NOTE_A7, NOTE_F7,
	NOTE_D7, NOTE_B7
};

// Der er et yndigt land tempo
uint8_t startup_tempo3[] = {
	8, 6, 8, 6,
	6, 3
};

/*===================*/
/* UNDERWORLD MELODY */
/*===================*/

// Underworld melody
uint16_t underworld_melody[] = {
	NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
	NOTE_AS3, NOTE_AS4, 0,
	0,
	NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
	NOTE_AS3, NOTE_AS4, 0,
	0,
	NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
	NOTE_DS3, NOTE_DS4, 0,
	0,
	NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
	NOTE_DS3, NOTE_DS4, 0,
	0, NOTE_DS4, NOTE_CS4, NOTE_D4,
	NOTE_CS4, NOTE_DS4,
	NOTE_DS4, NOTE_GS3,
	NOTE_G3, NOTE_CS4,
	NOTE_C4, NOTE_FS4, NOTE_F4, NOTE_E3, NOTE_AS4, NOTE_A4,
	NOTE_GS4, NOTE_DS4, NOTE_B3,
	NOTE_AS3, NOTE_A3, NOTE_GS3,
	0, 0, 0
};

// Underworld tempo
uint8_t underworld_tempo[] = {
	12, 12, 12, 12,
	12, 12, 6,
	3,
	12, 12, 12, 12,
	12, 12, 6,
	3,
	12, 12, 12, 12,
	12, 12, 6,
	3,
	12, 12, 12, 12,
	12, 12, 6,
	6, 18, 18, 18,
	6, 6,
	6, 6,
	6, 6,
	18, 18, 18, 18, 18, 18,
	10, 10, 10,
	10, 10, 10,
	3, 3, 3
};

/*================*/
/* CANTINA MELODY */
/*================*/

// Cantina melody
uint16_t cantina_melody[] = {
	NOTE_A4, NOTE_D5, NOTE_A4, NOTE_D5, NOTE_A4, NOTE_D5,
	NOTE_A4, NOTE_GS4, NOTE_A4,
	NOTE_A4, NOTE_GS4, NOTE_A4, NOTE_G4, 0,
	NOTE_FS4, NOTE_G4, NOTE_FS4,
	NOTE_F4, NOTE_D4, 0
};

// Cantina tempo
uint8_t cantina_tempo[] = {
	12, 12, 12, 12, 12, 12,
	24, 24, 12,
	24, 24, 24, 24, 24,
	24, 24, 24,
	6, 12, 12
};

/*==============*/
/* RIVER MELODY */
/*==============*/

// River melody
uint16_t river_melody[] = {
	NOTE_A4, NOTE_GS4, NOTE_A4, NOTE_GS4,
	NOTE_A4, NOTE_E4, NOTE_A4, NOTE_D4,
	NOTE_A3, NOTE_CS3
};

// River tempo
uint8_t river_tempo[] = {
	6, 6, 6, 6,
	6, 6, 6, 3,
	12, 12
};

#endif