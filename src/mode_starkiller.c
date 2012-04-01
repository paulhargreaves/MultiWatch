/* Copyright (C) 2012 Allerta
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// (C)2012 Paul Hargreaves (paul.hargreaves@technowizardry.co.uk)
// Refactored into MultiWatch with other functional adjustments for
// safety, reducing globals, code size etc.
//
// Legal disclaimer: This software is provided "as-is". Use at your own peril.
//

#include "pulse_app.h"
#include "mode_starkiller.h"

#define MODE_STARKILLER_BALL_SIZE  5 // careful! Check the draw function
#define MODE_STARKILLER_ENEMY_SIZE 6 // careful! Check the draw function

#define MODE_STARKILLER_ENEMY_DELAY 10 // 1/10 the speed of the player to move

#define MODE_STARKILLER_ENEMY_COUNT   7 // how many enemies
#define MODE_STARKILLER_BULLET_COUNT 11 // how many bullets in flight
#define MODE_STARKILLER_BULLET_FREQUENCY 10 // about 400ms...

//#define MODE_STARKILLER_ENEMY_MOVED_MAX_TOCK 33 // lower means faster moving

int  modeStarkillerScore; // current score
int  modeStarkillerHighScore = 0; // safe to be used here
int  modeStarkillerBallX;
int  modeStarkillerBallY;
int  modeStarkillerEnemyY[MODE_STARKILLER_ENEMY_COUNT];
int  modeStarkillerEnemyX[MODE_STARKILLER_ENEMY_COUNT];
int  modeStarkillerBulletX[MODE_STARKILLER_BULLET_COUNT];
int  modeStarkillerBulletY[MODE_STARKILLER_BULLET_COUNT];
int  modeStarkillerEnemyBulletX[MODE_STARKILLER_BULLET_COUNT];
int  modeStarkillerEnemyBulletY[MODE_STARKILLER_BULLET_COUNT];
bool modeStarkillerPlaying; // Is a game in process?
int  modeStarkillerPlayerDirection; // -1 or 1, depending on button
int  modeStarkillerEnemiesAlive; // how many enemies are alive?
int  modeStarkillerBulletSpeed;
int  modeStarkillerEnemyMoveTock;
int  modeStarkillerFireBulletsTock;

int32_t modeStarkillerRessurectEnemiesTimerID;
int32_t modeStarkillerBulletMoveTimerID;

color24_t mode_starkiller_color_blue  = { 0x00, 0x40, 0xFF, 0x00 };
color24_t mode_starkiller_color_red   = { 0xFF, 0x20, 0x20, 0x00 };

void mode_starkiller_move_enemies();
void mode_starkiller_move_bullets();

// Put the enemies at random positions
// They are all evenly spaced out on the x axis but at random y axis positions
void mode_starkiller_reset_enemies() {
  for (int i = 0; i < MODE_STARKILLER_ENEMY_COUNT; i++) {
    modeStarkillerEnemyX[i] = (i + 1) * (MODE_STARKILLER_ENEMY_SIZE * 2);
    modeStarkillerEnemyY[i] = 10 + ((i % 3) * 10);
  }
  modeStarkillerEnemiesAlive = MODE_STARKILLER_ENEMY_COUNT;
  modeStarkillerEnemyMoveTock = 0;
  modeStarkillerFireBulletsTock = 0;
  if (modeStarkillerBulletSpeed > 10) {
    modeStarkillerBulletSpeed = modeStarkillerBulletSpeed - 2;
  }
  mode_starkiller_move_enemies(); // so they display
}

// Start a new game
void mode_starkiller_start_game()
{
  multiModeChangePressTime = 4000; // 4 seconds
  multi_update_power_down_timer(99000); // 99 seconds
  pulse_blank_canvas();
  modeStarkillerPlaying = true;
  modeStarkillerBulletSpeed = 40;
  multiLoopTimeMS = 40; // 40ms
  modeStarkillerScore = 0;
  modeStarkillerBallX = 48;
  modeStarkillerBallY = SCREEN_HEIGHT-10;
  mode_starkiller_reset_enemies();
  for (int i = 0; i < MODE_STARKILLER_BULLET_COUNT; i++) {
    modeStarkillerBulletY[i] = 0;
    modeStarkillerEnemyBulletY[i] = SCREEN_HEIGHT;
  }
  modeStarkillerPlayerDirection = -1; // left
  mode_starkiller_move_bullets(); // start the bullet timer
}

void mode_starkiller_game_over() {
  multi_vibe_for_ms(60);
  pulse_blank_canvas();
  printf("GAME OVER\n\nScore: %d\n\nHighscore: %d\n\nClick to restart",
         modeStarkillerScore, modeStarkillerHighScore);
  multi_update_power_down_timer(8000); // 8 seconds
  modeStarkillerPlaying = false;
  multiModeChangePressTime = 1400; // 1.4 seconds
  multi_cancel_timer(&modeStarkillerBulletMoveTimerID);
}

void mode_starkiller_move_enemies() {
  for (int i = 0; i < MODE_STARKILLER_ENEMY_COUNT; i++) {
    if (modeStarkillerEnemyY[i] != 0) {
      int direction = rand()%2;
      if (direction == 0) {
        direction = -1;
      }
      // At this point direction is either 1 or -1

      modeStarkillerEnemyX[i] = modeStarkillerEnemyX[i] + direction;
      if ((modeStarkillerEnemyX[i] < 1) ||
                   (modeStarkillerEnemyX[i] > 
                   SCREEN_WIDTH - MODE_STARKILLER_ENEMY_SIZE - 1)) {
        modeStarkillerEnemyX[i] = modeStarkillerEnemyX[i] - direction;
      }
      pulse_draw_image(IMAGE_MODE_STARKILLER_INVADER, modeStarkillerEnemyX[i]-1,
                       modeStarkillerEnemyY[i]);
    }
  }
}

void mode_starkiller_draw_bullet(int x, int y, color24_t color) {
  pulse_set_draw_window(x, y, x, y);
  pulse_draw_point24(color);
}

void mode_starkiller_move_player_bullets() {
  for (int i = 0; i < MODE_STARKILLER_BULLET_COUNT; i++) {
    mode_starkiller_draw_bullet(modeStarkillerBulletX[i],
                                modeStarkillerBulletY[i], COLOR_BLACK24);
    // Move the bullet
    if (modeStarkillerBulletY[i] != 0) { 
      modeStarkillerBulletY[i]--;
    }
    // Has it hit the enemy?
    if (modeStarkillerBulletY[i] != 0) {
      mode_starkiller_draw_bullet(modeStarkillerBulletX[i], 
                     modeStarkillerBulletY[i], mode_starkiller_color_blue);
      for (int j = 0; j < MODE_STARKILLER_ENEMY_COUNT; j++) {
        if ((modeStarkillerBulletX[i] > modeStarkillerEnemyX[j]) && 
               (modeStarkillerBulletX[i] < modeStarkillerEnemyX[j]+
                            MODE_STARKILLER_ENEMY_SIZE) && 
               (modeStarkillerBulletY[i] > modeStarkillerEnemyY[j]) &&
               (modeStarkillerBulletY[i] < modeStarkillerEnemyY[j]+
                            MODE_STARKILLER_ENEMY_SIZE) &&
               modeStarkillerEnemyY[j] != 0) {
          multi_debug("HIT!\n");
          pulse_draw_image(IMAGE_MODE_STARKILLER_BLANK_INVADER,
                           modeStarkillerEnemyX[j]-1, modeStarkillerEnemyY[j]);
          modeStarkillerBulletY[i] = 0; // remove the bullet
          modeStarkillerEnemyY[j]  = 0; // remove the enemy
          modeStarkillerScore = modeStarkillerScore + 10;
          if (modeStarkillerScore > modeStarkillerHighScore) {
            // Increment high score here in case the user changes mode mid-game
            modeStarkillerHighScore = modeStarkillerScore;
          }
          modeStarkillerEnemiesAlive--;
          if ( modeStarkillerEnemiesAlive == 0) {
            multi_register_timer(&modeStarkillerRessurectEnemiesTimerID, 4000,
                      (PulseCallback) &mode_starkiller_reset_enemies, 0);
          }
        }
      }
    }
  }
}

void mode_starkiller_move_enemy_bullets() {
  for (int i = 0; i < MODE_STARKILLER_BULLET_COUNT; i++) {
    mode_starkiller_draw_bullet(modeStarkillerEnemyBulletX[i], 
                             modeStarkillerEnemyBulletY[i], COLOR_BLACK24);
    if (modeStarkillerEnemyBulletY[i] < SCREEN_HEIGHT) {
      modeStarkillerEnemyBulletY[i]++;
    }
    if (modeStarkillerEnemyBulletY[i] < SCREEN_HEIGHT) {
      mode_starkiller_draw_bullet(modeStarkillerEnemyBulletX[i],
                modeStarkillerEnemyBulletY[i], mode_starkiller_color_red);
      if ((modeStarkillerEnemyBulletX[i] > modeStarkillerBallX) &&
               (modeStarkillerEnemyBulletX[i] < modeStarkillerBallX +
                     MODE_STARKILLER_BALL_SIZE) &&
               (modeStarkillerEnemyBulletY[i] > modeStarkillerBallY) &&
               (modeStarkillerEnemyBulletY[i] < modeStarkillerBallY +
                     MODE_STARKILLER_BALL_SIZE)) {
        mode_starkiller_game_over();
        return; // do not bother with remaining bullets
      }
    }
  }
}

// Fire bullets from both the player and the enemies
void mode_starkiller_fire_bullets() {
  if ( modeStarkillerEnemiesAlive ) {
    // PLAYER
    if ((rand() % 3)) { 
      for (int i = 0; i < MODE_STARKILLER_BULLET_COUNT; i++) {
        if (modeStarkillerBulletY[i] == 0) {
          modeStarkillerBulletX[i] = modeStarkillerBallX + 2;
          modeStarkillerBulletY[i] = SCREEN_HEIGHT - 11;
          break;
        }
      }
    }

    // ENEMIES
    for (int i = 0; i < MODE_STARKILLER_BULLET_COUNT; i++) {
      if (modeStarkillerEnemyBulletY[i] >= SCREEN_HEIGHT) {
        // We look for an available enemy 
        int j = rand() % MODE_STARKILLER_ENEMY_COUNT;
        if (modeStarkillerEnemyY[j] == 0) { continue; } // not alive;try again

        modeStarkillerEnemyBulletX[i] = modeStarkillerEnemyX[j] + 3;
        modeStarkillerEnemyBulletY[i] = modeStarkillerEnemyY[j] +
                                        MODE_STARKILLER_ENEMY_SIZE + 1;
        break;
      }
    }
  }
}

void mode_starkiller_move_bullets() {
  multi_register_timer(&modeStarkillerBulletMoveTimerID, 
                       modeStarkillerBulletSpeed,
                       (PulseCallback) &mode_starkiller_move_bullets, 0);

  // Move the players bullets
  mode_starkiller_move_player_bullets();

  // Lastly, move the enemy bullets. Needs to be at the end as we could die
  // and display the game_over message
  mode_starkiller_move_enemy_bullets(); 
}

void mode_starkiller_main_app_loop() {
  multi_debug("direction = %i\n", modeStarkillerPlayerDirection);
  multi_debug("position before move %i\n", modeStarkillerBallX);

  // Move the players position
  modeStarkillerBallX = modeStarkillerBallX + modeStarkillerPlayerDirection;
  multi_debug("position after move %i\n", modeStarkillerBallX);
  if (modeStarkillerBallX > SCREEN_WIDTH - (MODE_STARKILLER_BALL_SIZE + 1) ||
                                              modeStarkillerBallX < 1) {
    multi_debug("ball x > SCREEN_WIDTH || < 1\n");
    // undo the move
    modeStarkillerBallX = modeStarkillerBallX - modeStarkillerPlayerDirection;
  }
  assert(modeStarkillerBallX >= 1);
  assert(modeStarkillerBallX <= SCREEN_WIDTH - (MODE_STARKILLER_BALL_SIZE + 1));

  // Draw the player
  pulse_draw_image(IMAGE_MODE_STARKILLER_BALL, modeStarkillerBallX - 1,
                   modeStarkillerBallY);

  // Move the enemies
  if (modeStarkillerEnemyMoveTock++ % MODE_STARKILLER_ENEMY_DELAY == 0) {
    mode_starkiller_move_enemies();
  }

  // Fire new bullets?
  if (modeStarkillerFireBulletsTock++ % MODE_STARKILLER_BULLET_FREQUENCY == 0) {
    mode_starkiller_fire_bullets();
  }

}

// The main init function
void mode_starkiller_watch_functions(const enum multi_function_table iFunc,
                                       ...) {
  multi_debug("enum %i\n", iFunc);
  switch (iFunc) {
    case MODEINIT:
      break;
    case BUTTONWAKE:
      modeStarkillerPlaying = false; // new game
      modeStarkillerRessurectEnemiesTimerID = -1;
      modeStarkillerBulletMoveTimerID = -1;
      mode_starkiller_watch_functions(BUTTONDOWN);
      break;
    case MAINLOOP:
      if (modeStarkillerPlaying) {
        mode_starkiller_main_app_loop();
      }
      break;
    case BUTTONDOWN:
      modeStarkillerPlayerDirection = -modeStarkillerPlayerDirection;
      if ( !modeStarkillerPlaying ) {
        mode_starkiller_start_game(); // start playing!
      }
      break;
    default: // ignore features we do not use
      break;
  }
}

