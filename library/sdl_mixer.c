#include "sdl_mixer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

const char* WAV_PATH = "assets/movement.wav";
const char* MUS_PATH = "assets/Time-of-Horror.wav";
const char* JUMP_PATH = "assets/jump2.wav";


Mix_Chunk *grapple = NULL;
Mix_Chunk *jump_music = NULL;
Mix_Music *background_music = NULL;


int grappleSoundEffects() {
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1){
        return -1;
    }

    // Load our sound effect
    grapple = Mix_LoadWAV(WAV_PATH);
    if (grapple == NULL){
         return -1;
    } 

    if (Mix_PlayChannel(-1, grapple, 0) == -1){
         return -1;
    }
    return 0;
}

int jumpSoundEffects() {
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1){
        return -1;
    }

    // Load our sound effect
    jump_music = Mix_LoadWAV(JUMP_PATH);
    if (jump_music == NULL){
         return -1;
    } 

    if (Mix_PlayChannel(-1, jump_music, 0) == -1){
         return -1;
    }
    return 0;
}

int backgroundMusic(){
   if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1){
     printf("failed to open audio\n");
     return -1;
    }
     // Load our music
    background_music = Mix_LoadMUS(MUS_PATH);
    if (background_music == NULL){
      return -1;
    }

     if (Mix_PlayMusic(background_music, -1) == -1){
       return -1;
    }

    return 0;

}

void freeMusic(){
     Mix_FreeChunk(grapple);
     Mix_FreeMusic(background_music);
}


//citations for background music:

// Time of Horror by Magnetic Trailer | https://lesfm.net/
// Music promoted by https://www.chosic.com/free-music/all/
// Creative Commons CC BY 3.0
// https://creativecommons.org/licenses/by/3.0/