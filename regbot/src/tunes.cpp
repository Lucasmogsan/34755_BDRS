/*
 * tunes.c
 *
 * Created: 14/03/2017 15:20:42
 *  Author: Henning
 */

/*==========*/
/* Includes */
/*==========*/

#include "Arduino.h"
// #include "pins_arduino.h"
#include "tunes.h"
#include "melodies.h"
#include "uusb.h"

/*=============*/
/* Definitions */
/*=============*/

#define LOOP_TIME 10	// [ms]

/*========================*/
/* Variables & Prototypes */
/*========================*/

void tunes_loop();

IntervalTimer tunes_timer;
volatile int tunes_progress = -1;
static int tunes_length = 0;
static uint8_t buzzerPin = 2;
static uint8_t *tempo_p;
static uint16_t *melody_p;
int next_tune_loop = 0;
int song = -1;

static uint8_t reference = 4;

uint16_t bpm = 100;
uint8_t format = FORMAT_OLD;



/*===========*/
/* FUNCTIONS */
/*===========*/

void tunes_init(uint8_t initBuzPin) {
	buzzerPin = initBuzPin;
	pinMode(buzzerPin, OUTPUT);
}

void setBPM(uint16_t BPM) {
	bpm = 2.3*BPM;
	format = FORMAT_BPM;
}

void sing(uint8_t songID) {
	/* Select song */
  song = songID;
  bool found = true;
	switch(songID) {
		case MARIO_MAIN_MELODY_ID:
			tunes_length = sizeof(mario_main_melody) / sizeof(uint16_t);
			tempo_p = mario_main_tempo;
			melody_p = mario_main_melody;
			format = FORMAT_OLD;
			reference = 4;
			break;
		case SAX_MELODY_ID:
			tunes_length = sizeof(sax_melody) / sizeof(uint16_t);
			tempo_p = sax_tempo;
			melody_p = sax_melody;
			format = FORMAT_OLD;
			break;
		case WARNING_MELODY_ID:
			tunes_length = sizeof(warning_melody) / sizeof(uint16_t);
			tempo_p = warning_tempo;
			melody_p = warning_melody;
			format = FORMAT_OLD;
			break;
		case STARTUP_MELODY_ID:
			tunes_length = sizeof(startup_melody) / sizeof(uint16_t);
			tempo_p = startup_tempo;
			melody_p = startup_melody;
			format = FORMAT_OLD;
			break;
		case STARTUP_MELODY2_ID:
			tunes_length = sizeof(startup_melody2) / sizeof(uint16_t);
			tempo_p = startup_tempo2;
			melody_p = startup_melody2;
			format = FORMAT_OLD;
			break;
		case UNDERWORLD_MELODY_ID:
			tunes_length = sizeof(underworld_melody) / sizeof(uint16_t);
			tempo_p = underworld_tempo;
			melody_p = underworld_melody;
			format = FORMAT_OLD;
			break;
		case CANTINA_MELODY_ID:
			tunes_length = sizeof(cantina_melody) / sizeof(uint16_t);
			tempo_p = cantina_tempo;
			melody_p = cantina_melody;
			format = FORMAT_OLD;
			break;
		case RIVER_MELODY_ID:
			tunes_length = sizeof(river_melody) / sizeof(uint16_t);
			tempo_p = river_tempo;
			melody_p = river_melody;
			format = FORMAT_OLD;
			break;
		case BILLIE_JEAN_1_ID:
			tunes_length = sizeof(billie_melody_1) / sizeof(uint16_t);
			tempo_p = billie_tempo_1;
			melody_p = billie_melody_1;
			setBPM(120);
			break;
		case GAME_MELODY_ID:
			tunes_length = sizeof(game_melody) / sizeof(uint16_t);
			tempo_p = game_tempo;
			melody_p = game_melody;
			reference = 4; // 1-2-3-4
			setBPM(61);
			break;
		case SWEET_HOME_MELODY_ID:
			tunes_length = sizeof(sweetHomeAlabama_melody) / sizeof(uint16_t);
			tempo_p = sweetHomeAlabama_tempo;
			melody_p = sweetHomeAlabama_melody;
			setBPM(98);
			break;
    default:
      found = false;
      break;
	}
  if (found)
  {
    /* Play melody */
    tunes_progress = 0;
    tunes_timer.begin(tunes_loop, LOOP_TIME * 1000);  // run every 0.010 seconds
    tunes_timer.priority(255);	// lowest priority
  }
  else
    usb.send("# melody not found\r\n");
}

bool tunes_is_ready() {
	return tunes_progress < 0;
}

void tunes_loop() {
	if(tunes_progress < 0) 
  {
		return;
	} 
	else if(tunes_progress >= tunes_length) 
  {
		noTone(buzzerPin);
		tunes_progress = -1;
		tunes_timer.end();
	}
	next_tune_loop--;
	if(next_tune_loop <= 0) 
  {
		int noteDuration;

		if (format == FORMAT_BPM)
		{
			noteDuration = (60000 * reference) / (tempo_p[tunes_progress] * bpm);
		}
		else {
			noteDuration = 1000/tempo_p[tunes_progress];
		}

		noTone(buzzerPin);
		tone(buzzerPin, melody_p[tunes_progress], noteDuration);
		tunes_progress++;
		next_tune_loop = noteDuration * 2.2 / LOOP_TIME;
	}
}

void sendTunesStatus()
{
  const int MSL = 100;
  char s[MSL];
  snprintf(s, MSL, "song %d %d %d\r\n", tunes_progress, song, tunes_length);
  usb.send(s);
}
