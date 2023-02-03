/*
 * tunes.h
 *
 * Created: 14/03/2017 15:20:50
 *  Author: Henning
 */


#ifndef TUNES_H_
#define TUNES_H_

#define FORMAT_BPM 1
#define FORMAT_OLD 0

enum SONG_IDS {
  MARIO_MAIN_MELODY_ID,
  SAX_MELODY_ID,
  WARNING_MELODY_ID,
  STARTUP_MELODY_ID,
  STARTUP_MELODY2_ID,
  UNDERWORLD_MELODY_ID,
  CANTINA_MELODY_ID,
  RIVER_MELODY_ID,
  BILLIE_JEAN_1_ID,
  GAME_MELODY_ID,
  SWEET_HOME_MELODY_ID
};

bool tunes_is_ready();
void sing(uint8_t songID);
void tunes_init(uint8_t initBuzPin);
void sendTunesStatus();

#endif /* TUNES_H_ */
