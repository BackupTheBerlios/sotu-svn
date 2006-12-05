// Description:
//   Online update check.
//
// Copyright (C) 2005 Frank Becker
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
#ifndef _OnlineCheck_hpp_
#define _OnlineCheck_hpp_

#include <list>

#include <Trace.hpp>
#include <Singleton.hpp>
#include "SDL_thread.h"

class OnlineUpdate
{
friend class Singleton<OnlineUpdate>;
public:
    enum Status
    {
	eNotChecking,
	eDownloading,
	eFailure,
	eSuccess,
	eLAST
    };

    struct NewsItem
    {
	string title;
	string date;
	string text;
	float r;
	float g;
	float b;
    };

    void init( const string &defaultUpdateURL);
    void getUpdate( void);
    Status getStatus( void) { return _status; }

    bool isLatest( void) { return _isLatest; }
    const string getLatestVersion( void) { return _latestVersion; }
    const string getLatestVersionDate( void) { return _latestVersionDate; }
    const string getLatestVersionText( void) { return _latestVersionText; }

    list<NewsItem*>& getNewsItems( void) { return _newsItemList; }

private:
    ~OnlineUpdate();
    OnlineUpdate( void);
    OnlineUpdate( const OnlineUpdate&);
    OnlineUpdate &operator=( const OnlineUpdate&);

    static int run(void *data);
    static size_t write(void *buffer, size_t size, size_t nmemb, void *data);

    bool process( void);
    void loadUpdateCache(void);
    void loadUpdateCache(const string &updateCache);

    string _updateURL;
    SDL_Thread *_thread;
    string _text;
    Status _status;
    bool _isLatest;
    string _latestVersion;
    string _latestVersionDate;
    string _latestVersionText;
    list<NewsItem*> _newsItemList;
};

typedef Singleton<OnlineUpdate> OnlineUpdateS;

#endif
