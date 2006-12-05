// Description:
//   main. Ready, set, go!
//
// Copyright (C) 2001 Frank Becker
// Copyright (C) 2006 Milan Babuskov
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
#include "SDL.h" //needed for SDL_main

#include <Trace.hpp>
#include <Constants.hpp>
#include <Config.hpp>
#include <Input.hpp>
#include <Game.hpp>
#include <GameState.hpp>
#include <Endian.hpp>
#include <ResourceManager.hpp>
#include <GetDataPath.hpp>
#include <OnlineUpdate.hpp>

#include <sys/stat.h>
#include <sys/types.h>
//----------------------------------------------------------------------------
void migrateConfig( void)
{
    //if onlineCheck is not set, default it to true
    bool dummy;
    if( ! ConfigS::instance()->getBoolean( "onlineCheck", dummy))
    {
    ConfigS::instance()->updateKeyword( "onlineCheck", "1");
    }

#ifdef WIN32
    rename( "config.txt-scores" , "leaderboard");
#else
    struct stat statInfo;

    string configFileName = ConfigS::instance()->getConfigFileName();

    if( (stat( configFileName.c_str(), &statInfo) != -1) )
    {
    LOG_INFO << "Found config file - no need to migrate\n";
    return;
    }

    string::size_type start = configFileName.find_last_of( '/');
    if( start > 0)
    {
    string fileName = configFileName.substr(start+1);
//  LOG_INFO << "filename = " << fileName << endl;
    string path = configFileName.substr( 0, start);
//  LOG_INFO << "path = " << path << endl;
    if( (stat( path.c_str(), &statInfo) != -1) )
    {
        if( S_ISDIR(statInfo.st_mode))
        {
        LOG_INFO << "New config dir found - OK\n";
        return;
        }
        else
        {
        //ok, we got some work to do
        string bak = path + ".bak";
        if( rename( path.c_str(), bak.c_str()) == 0)
        {
            mkdir( path.c_str(), 0777);
            if( rename( bak.c_str(), configFileName.c_str()) != 0)
            {
            LOG_ERROR << "Unable to rename: " << bak << " to " << configFileName << endl;
            return;
            }
            string leaderboard = path + "/leaderboard";
            string oldleaderboard = path + "-scores";
            if( rename( oldleaderboard.c_str(), leaderboard.c_str()) != 0)
            {
            LOG_WARNING << "Unable to rename: " << oldleaderboard << " to " << leaderboard << endl;
            return;
            }
            LOG_INFO << "config file migration OK\n";
        }
        }
    }
    else
    {
        //.critter subdir doesn't exists at all, create it
        mkdir( path.c_str(), 0777);
    }
    }
#endif
}
//----------------------------------------------------------------------------
void addOtherResourcePacks()
{
    string configDir = ConfigS::instance()->getConfigDirectory();

    int i=0;
    char num[10];
    bool done = false;
    while( ! done)
    {
    sprintf( num, "%03d", i);
    string filename = string("resource") + num + string(".dat");
    string fullPath = configDir + filename;
    struct stat statInfo;
    LOG_INFO  << "Looking for " << fullPath << endl;
    if( stat( fullPath.c_str(), &statInfo) == -1)
    {
        done = true;
    }
    else
    {
        ResourceManagerS::instance()->addResourcePack(filename, configDir);
    }
    i++;
    }
}

void checkEndian( void)
{
    if( ::isLittleEndian())
    {
    LOG_INFO << "Setting up for little endian." << endl;
    }
    else
    {
    LOG_INFO << "Setting up for big endian." << endl;
    }
}
//----------------------------------------------------------------------------
void showInfo()
{
    LOG_INFO << "----------------------------------" << endl;
    LOG_INFO << GAMETITLE << " " << GAMEVERSION
             << " - "__TIME__" "__DATE__
             << endl;
    LOG_INFO << "Copyright (C) 2006 Milan Babuskov, 2001-2005 Frank Becker" << endl;
    LOG_INFO << "Visit http://sotu.berlios.de/" << endl;
    LOG_INFO << "----------------------------------" << endl;
}
//----------------------------------------------------------------------------
#include <png.h>
void showVersions( void)
{
    const SDL_version *vsdl = SDL_Linked_Version();
    LOG_INFO << "SDL Version "
             << (int)vsdl->major  << "."
             << (int)vsdl->minor  << "."
             << (int)vsdl->patch  << endl;
     LOG_INFO << "zlib Version " << zlibVersion() << endl;
     LOG_INFO << "PNG Version " << png_get_header_version(NULL) << endl;
}
//----------------------------------------------------------------------------
int main( int argc, char *argv[])
{
    XTRACE();

    showInfo();
    showVersions();

    checkEndian();
    if( !ResourceManagerS::instance()->
      addResourcePack("resource.dat", getDataPath()))
    {
        LOG_WARNING << "resource.dat not found. Trying data directory." << endl;
#ifdef VCPP
        ResourceManagerS::instance()->addResourceDirectory( "..\\data");
#else
        ResourceManagerS::instance()->addResourceDirectory( "data");
#endif
        if( ResourceManagerS::instance()->getResourceSize("system/config.txt") < 0)
        {
            LOG_ERROR << "Sorry, unable to find game data!" << endl;
            return -1;
        }
    }

    Config *cfg = ConfigS::instance();
#ifndef WIN32
    cfg->setSubdirectory( ".critter");
#endif
    migrateConfig();

    addOtherResourcePacks();

    // register config handler(s)
    cfg->registerConfigHandler( InputS::instance()->preinit());

    // read config file...
    cfg->updateFromFile();

    // process command line arguments...
    cfg->updateFromCommandLine( argc, argv);

    // to dump or not to dump...
    cfg->getBoolean( "developer", GameState::isDeveloper);
    if( GameState::isDeveloper)
    {
        cfg->dump();
    }

    OnlineUpdateS::instance()->init("http://sotu.berlios.de/update.php");
    //TODO: Napraviti da berlios vraca podatke o updateu (obican string)
    //OnlineUpdateS::instance()->getUpdate();

    // get ready!
    if( GameS::instance()->init())
    {
        // let's go!
        GameS::instance()->run();
    }

    // Fun is over. Cleanup time...
    GameS::cleanup();
    OnlineUpdateS::cleanup();
    ConfigS::cleanup();

    LOG_INFO << "Cleanup complete. Ready to Exit." << endl;

    showInfo();

    // See ya!
    return 0;
}
//----------------------------------------------------------------------------
