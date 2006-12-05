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
#include <defines.h>
#include <Audio.hpp>
#include <Trace.hpp>
#include <Config.hpp>
#include <ResourceManager.hpp>
#include <zrwops.hpp>
#include <SampleManager.hpp>
#include <Timer.hpp>
#include <GetDataPath.hpp>

#include <sys/types.h>
#include <sys/stat.h>

#include "SDL.h"
#include "SDL_mixer.h"

Audio::Audio():
  _sampleManager(0),
  _soundTrack(0),
  _playDefaultSoundtrack( true),
  _playMusic( true),
  _isPlaying( false),
  _unloadMusic(false),
  _musicVolume(0.8),
  _effectsVolume(0.8)
{
    XTRACE();

    bool dummy;
    //if config variables don't exist (upgrade, etc) update 
    if( ! ConfigS::instance()->getBoolean( "playMusic", dummy))
    {
	Value *v = new Value( _playMusic);
	ConfigS::instance()->updateKeyword( "playMusic", v);
    }
    if( ! ConfigS::instance()->getBoolean( 
	"playDefaultSoundtrack", dummy))
    {
	Value *v = new Value( _playDefaultSoundtrack);
	ConfigS::instance()->updateKeyword( "playDefaultSoundtrack", v);
    }
}

Audio::~Audio()
{
    XTRACE();
    LOG_INFO << "Audio shutdown..." << endl;

    turnMusicOff();

    Mix_CloseAudio();
    delete _sampleManager;

    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

static void musicFinished( void)
{
    AudioS::instance()->musicFinished();
}

void Audio::musicFinished( void)
{
    _unloadMusic = true;
}

void Audio::loadMusic( const string &mod)
{
    XTRACE();
    if( mod == "") return;

    LOG_INFO << "Loading soundtrack...[" << mod << "]" << endl;

#if 0
//SDL_mixer doesn't have RWops support for MUS yet :(
    if( !ResourceManagerS::instance()->selectResource( mod))
    {
	LOG_ERROR << "Music file [" << mod << "] not found." << endl;
	return;
    }

    ziStream &infile = ResourceManagerS::instance()->getInputStream();
    SDL_RWops *src = RWops_from_ziStream( infile);

    _soundTrack = Mix_LoadMUS_RW(src);
    SDL_RWclose( src);
#else
    _soundTrack = Mix_LoadMUS( mod.c_str());
#endif

    if( !_soundTrack)
    { 
	LOG_ERROR << "Failed to load soundtrack: [" << mod << "]" << endl;
	LOG_ERROR << SDL_GetError() << endl;
    }
    else
    {
	//loop soundtrack
	Mix_FadeInMusic( _soundTrack, -1, 500);
	Mix_HookMusicFinished(::musicFinished);
	_isPlaying = true;
    }
}

bool Audio::init( void)
{
    XTRACE();
    LOG_INFO << "Initializing Audio..." << endl;

    bool audio;
    if( !ConfigS::instance()->getBoolean( "audio", audio))
    {
        audio = false;
    }
    if( audio)
    {
	if( SDL_InitSubSystem( SDL_INIT_AUDIO) < 0 )
	{
	    LOG_ERROR << "Init Audio: failed # " << SDL_GetError() << endl;
	    return false;
	}

        if( Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) < 0 ) 
        {
	    LOG_ERROR << "Init Audio (Mixer): failed # " 
                      << SDL_GetError() << endl;
	    return false;
        }

	_sampleManager = new SampleManager();

	//make sure we have default volumes
	float dummy;
	if( ! ConfigS::instance()->getFloat( "musicVolume", dummy))
	{
	    Value *v = new Value( 0.8f);
	    ConfigS::instance()->updateKeyword( "musicVolume", v);
	}
	if( !ConfigS::instance()->getFloat( "effectsVolume", dummy))
	{
	    Value *v = new Value( 0.8f);
	    ConfigS::instance()->updateKeyword( "effectsVolume", v);
	}

	ConfigS::instance()->getBoolean( "playMusic", _playMusic);
	ConfigS::instance()->getBoolean( 
	    "playDefaultSoundtrack", _playDefaultSoundtrack);
	startMusic();
	updateVolume();

        LOG_INFO << "Audio ON." << endl;
    }
    else
    {
        LOG_INFO << "Audio OFF." << endl;
    }

    return true;
}

void Audio::playSample( const string &sampleName)
{
    if( !_sampleManager) return;

    Mix_Chunk *sample = _sampleManager->getSample( sampleName);
    if( sample)
    {
	//any channel, no loop
	Mix_PlayChannel(-1, sample, 0);
    }
}

void Audio::turnMusicOff( void)
{
    if( _isPlaying)
    {
	Mix_FadeOutMusic(500);
	_isPlaying = false;
    }
}

void Audio::startMusic( void)
{
    if( _playMusic)
    {
	string soundtrack = "";

	if( _playDefaultSoundtrack)
	{
	    //look in developer file tree first
	    soundtrack = string("../data/music/lg-criti.xm");

	    struct stat statInfo;
	    if( stat( soundtrack.c_str(), &statInfo) == -1)
	    {
		//try global
		soundtrack = getDataPath()+string("lg-criti.xm");
	    }
	}
	else
	{
	    ConfigS::instance()->getString( "soundtrack", soundtrack);
	}

        turnMusicOff();
	loadMusic( soundtrack);
    }
    else
    {
        turnMusicOff();
    }
}

void Audio::updateSettings( void)
{
    if( Mix_FadingMusic() != MIX_NO_FADING) return;

    bool oldPlayMusic = _playMusic;
    bool oldPlayDefaultSoundtrack = _playDefaultSoundtrack;

    ConfigS::instance()->getBoolean( "playMusic", _playMusic);
    ConfigS::instance()->getBoolean( 
	"playDefaultSoundtrack", _playDefaultSoundtrack);

    if( (oldPlayMusic == _playMusic) &&
	(oldPlayDefaultSoundtrack == _playDefaultSoundtrack))
    {
	//no changes...
	return;
    }

    startMusic();
}

void Audio::updateVolume( void)
{
    int newVolume;

    float musicVolume = 0.8;
    ConfigS::instance()->getFloat( "musicVolume", musicVolume);
    newVolume = (int)(MIX_MAX_VOLUME * musicVolume);
    if( _musicVolume != musicVolume)
    {
	_musicVolume = musicVolume;
	Mix_VolumeMusic( newVolume);
    }

    float effectsVolume = 0.8;
    ConfigS::instance()->getFloat( "effectsVolume", effectsVolume);
    newVolume = (int)(MIX_MAX_VOLUME * effectsVolume);
    if( _effectsVolume != effectsVolume)
    {
	_effectsVolume = effectsVolume;
	Mix_Volume( -1, newVolume);
	playSample( "sounds/beep.wav");
    }
}

bool Audio::update( void)
{
    if( !_sampleManager) return true;

    if( _unloadMusic)
    {
	if( _soundTrack)
	{
	    Mix_FreeMusic( _soundTrack);
	    _soundTrack = 0;
	}
	_unloadMusic = false;
    }

    updateVolume();

    static float nextTime = Timer::getTime()+0.5f;
    float thisTime = Timer::getTime();
    if( thisTime > nextTime)
    {
	updateSettings();
	nextTime = thisTime+0.5f;
    }

    return true;
}
