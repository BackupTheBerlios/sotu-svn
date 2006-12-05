// Description:
//   Audio subsysytem.
//
// Copyright (C) 2001 Frank Becker
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation;  either version 2 of the License,  or (at your option) any  later
// version.
//
// This program is distributed in the hope that it will be useful,  but  WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details
//
#ifndef _Audio_hpp_
#define _Audio_hpp_

#include <string>

#include "SDL_mixer.h"
#include <Singleton.hpp>

using std::string;

class SampleManager;

class Audio
{
friend class Singleton<Audio>;
public:
    bool init( void);
    bool update( void);
    void playSample( const string &sampleName);

    void musicFinished( void);
private:
    ~Audio();
    Audio( void);
    Audio( const Audio&);
    Audio &operator=(const Audio&);

    void loadMusic( const string &mod);
    void turnMusicOff( void);
    void startMusic( void);
    void updateSettings( void);
    void updateVolume( void);

    SampleManager *_sampleManager;
    Mix_Music *_soundTrack;
    bool _playDefaultSoundtrack;
    bool _playMusic;
    bool _isPlaying;
    bool _unloadMusic;
    float _musicVolume;
    float _effectsVolume;
};

typedef Singleton<Audio> AudioS;

#endif
