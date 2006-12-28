// Description:
//   Video subsystem.
//
// Copyright (C) 2001 Frank Becker
// Copyright (c) 2006 Milan Babuskov
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
#include <math.h>

#include <FPS.hpp>

#include <PNG.hpp>
#include <Video.hpp>
#include <Trace.hpp>
#include <Random.hpp>
#include <Config.hpp>
#include <GameState.hpp>
#include <Game.hpp>
#include <Constants.hpp>

#include <Hero.hpp>
#include <ParticleGroupManager.hpp>
#include <Timer.hpp>
#include <Camera.hpp>
#include <Starfield.hpp>
#include <MenuManager.hpp>
#include <StageManager.hpp>
#include <FontManager.hpp>
#include <BitmapManager.hpp>
#include <ModelManager.hpp>
#include <ScoreKeeper.hpp>
#include <TextureManager.hpp>
#include <GLExtension.hpp>

#include <ResourceManager.hpp>
#include <zrwops.hpp>

#include <gl++.hpp>
#include "SDL.h"

//Earlier versions of SDL don't have DISABLE defined
#ifndef SDL_DISABLE
#define SDL_DISABLE 0
#endif
//----------------------------------------------------------------------------
Video::Video():
    _isFullscreen( false),
    _showStarfield( true),
    _showNebulas( true),
    _maxFPS(0),
    _fpsStepSize(0.0),
    _bpp( 0),
    _width( VIDEO_DEFAULT_WIDTH),
    _height( VIDEO_DEFAULT_HEIGHT),
    _boardVisible( true),
    _boardPosX( 0),
    _titleA( 0),
    _titleB( 0),
    _angle(0.0),
    _prevAngle(0.0)
{
    XTRACE();
}
//----------------------------------------------------------------------------
Video::~Video()
{
    XTRACE();
    LOG_INFO << "Video shutdown..." << endl;

    BitmapManagerS::cleanup();
    FontManagerS::cleanup();
    TextureManagerS::cleanup();
    GLExtension::close();

    delete _titleA;
    delete _titleB;

    SkillS::cleanup();
    CameraS::cleanup();

    SDL_QuitSubSystem( SDL_INIT_VIDEO);
    SDL_Quit();
}
//----------------------------------------------------------------------------
void Video::reload( void)
{
    BitmapManagerS::instance()->reload();
    FontManagerS::instance()->reload();

    _titleA->reload();
    _titleB->reload();

    ModelManagerS::instance()->reload();
    PlanetManagerS::instance()->reload();
}
//----------------------------------------------------------------------------
void Video::grabAndWarpMouse()
{
    SDL_ShowCursor(SDL_DISABLE);
    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_WarpMouse( _width/2,  _height/2);
    SDL_Event event;
    while( SDL_PollEvent( &event))
    {
        //remove any queued up events due to warping, etc.
        ;
    }
}
//----------------------------------------------------------------------------
bool Video::init( void)
{
    XTRACE();

    LOG_INFO << "Initializing Video..." << endl;

    if( SDL_InitSubSystem( SDL_INIT_VIDEO) < 0 )
    {
        LOG_ERROR << "Init Video: failed # " << SDL_GetError() << endl;
        return false;
    }
    LOG_INFO << "Video: OK" << endl;

    ConfigS::instance()->getInteger( "maxFPS", _maxFPS);
    if( _maxFPS)
    {
        LOG_INFO << "Video: Restricting FPS to " << _maxFPS << endl;
        _fpsStepSize = 1.0f/(float)_maxFPS;
    }

    ConfigS::instance()->getBoolean( "fullscreen", _isFullscreen);

    LOG_INFO << "Setting program icon" << endl;
    SDL_Surface *icon = SDL_LoadBMP("icon.bmp");
    if (!icon)
    {
        LOG_ERROR << "Unable to load Icon" << endl;
        return false;
    }
    SDL_WM_SetIcon(icon, NULL);

    if( !setVideoMode())
    {
        return false;
    }

    _smallFont = FontManagerS::instance()->getFont( "bitmaps/arial-small");
    if( !_smallFont)
    {
        LOG_ERROR << "Unable to get font... (arial-small)" << endl;
        SDL_QuitSubSystem( SDL_INIT_VIDEO);
        return false;
    }

    _scoreFont = FontManagerS::instance()->getFont( "bitmaps/vipnaUpper");
    if( !_scoreFont)
    {
        LOG_ERROR << "Unable to get font... (vipnaUpper)" << endl;
        SDL_QuitSubSystem( SDL_INIT_VIDEO);
        return false;
    }

    _gameOFont = FontManagerS::instance()->getFont( "bitmaps/gameover");
    if( !_gameOFont)
    {
        LOG_ERROR << "Unable to get font... (gameover)" << endl;
        SDL_QuitSubSystem( SDL_INIT_VIDEO);
        return false;
    }

    _board = BitmapManagerS::instance()->getBitmap( "bitmaps/board");
    if( !_board)
    {
        LOG_ERROR << "Unable to load CritterBoard" << endl;
        SDL_QuitSubSystem( SDL_INIT_VIDEO);
        return false;
    }
    _boardIndex = _board->getIndex( "CritterBoard");

    _titleA = ResourceManagerS::instance()->getTexture("bitmaps/titleA.png");
    if (!_titleA)
        return false;
    _titleB = ResourceManagerS::instance()->getTexture("bitmaps/titleB.png");
    if (!_titleB)
        return false;

    grabAndWarpMouse();

    LOG_INFO << "OpenGL info follows..." << endl;
    string vendor = (char*)glGetString( GL_VENDOR);
    if( vendor.find( "Brian Paul") != string::npos)
    {
        LOG_WARNING << "*** Using MESA software rendering." << endl;
    }
    LOG_INFO << "  Vendor  : " << vendor << endl;
    LOG_INFO << "  Renderer: " <<  glGetString( GL_RENDERER) << endl;
    LOG_INFO << "  Version : " << glGetString( GL_VERSION) << endl;

    glViewport(0,0, _width, _height);

#if 0
    GLfloat mat_ambient[]  = { 0.7f, 0.7f, 0.7f, 0.0f };
    GLfloat mat_diffuse[]  = { 0.6f, 0.6f, 0.6f, 0.0f };
    GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 0.0f };
#else
    GLfloat mat_ambient[]  = { 0.5f, 0.5f, 0.5f, 0.5f };
    GLfloat mat_diffuse[]  = { 0.4f, 0.4f, 0.4f, 0.5f };
    GLfloat mat_specular[] = { 0.6f, 0.6f, 0.6f, 0.5f };
#endif
    GLfloat mat_shininess[] = { 60.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

    GLfloat mat_diffuse_b[]  = { 0.5f, 0.5f, 0.5f, 0.5f };
    GLfloat mat_ambient_b[]  = { 0.3f, 0.3f, 0.3f, 0.5f };
    GLfloat mat_specular_b[] = { 0.2f, 0.2f, 0.2f, 0.5f };
    GLfloat mat_shininess_b[] = { 10.0f };
    glMaterialfv(GL_BACK, GL_SPECULAR, mat_specular_b);
    glMaterialfv(GL_BACK, GL_SHININESS, mat_shininess_b);
    glMaterialfv(GL_BACK, GL_AMBIENT, mat_ambient_b);
    glMaterialfv(GL_BACK, GL_DIFFUSE, mat_diffuse_b);

    glClearColor(0.0, 0.0, 0.0, 0.0);

    return true;
}
//----------------------------------------------------------------------------
bool Video::setVideoMode( void)
{
    int videoFlags = SDL_OPENGL;
    if( _isFullscreen)
    {
        LOG_INFO << "Fullscreen request." << endl;
        videoFlags |= SDL_FULLSCREEN;
    }

    if( !ConfigS::instance()->getInteger( "width", _width))
    {
        Value *w = new Value( _width);
        ConfigS::instance()->updateTransitoryKeyword( "width", w);
    }
    if( !ConfigS::instance()->getInteger( "height", _height))
    {
        Value *h = new Value( _height);
        ConfigS::instance()->updateTransitoryKeyword( "height", h);
    }

    bool allowSkew = false;
    ConfigS::instance()->getBoolean( "allowAnyAspectRatio", allowSkew);
    if( !allowSkew && ((_height*4/3) != _width))
    {
        LOG_WARNING << "Adjusting height to be 3/4 of width (" << _height << " -> " << _width*3/4 << ")." << endl;
        LOG_WARNING << "To allow any aspect ratio set allowAnyAspectRatio to true in config file." << endl;
        _height = _width*3/4;

        Value *h = new Value( _height);
        ConfigS::instance()->updateTransitoryKeyword( "height", h);
    }

    if( ! ::init("libGL.so.1"))
    {
        LOG_ERROR << "SDL Error: " << SDL_GetError() << endl;
        SDL_QuitSubSystem( SDL_INIT_VIDEO);
        return false;
    }

    if( SDL_SetVideoMode( _width, _height, _bpp, videoFlags ) == NULL )
    {
        LOG_ERROR << "Video Mode: failed #" << SDL_GetError() << endl;
        SDL_QuitSubSystem( SDL_INIT_VIDEO);
        return false;
    }
    glViewport(0,0, _width, _height);

    //set title and icon name
    SDL_WM_SetCaption( "Scum of the Universe", "SotU" );

    SDL_Surface *surf = SDL_GetVideoSurface();
    LOG_INFO << "Video Mode: OK (" << surf->w << "x"
         << surf->h << "x" << (int)surf->format->BitsPerPixel << ")" << endl;

    return true;
}
//----------------------------------------------------------------------------
void  Video::updateSettings( void)
{
    bool fullscreen = _isFullscreen;
    ConfigS::instance()->getBoolean( "fullscreen", _isFullscreen);

    int width = 0;
    ConfigS::instance()->getInteger( "width", width);
    int height = 0;
    ConfigS::instance()->getInteger( "height", height);
    if( (fullscreen != _isFullscreen) || (width != _width) || (height != _height))
    {
#ifdef DYNAMIC_GL
        SDL_QuitSubSystem( SDL_INIT_VIDEO);
        if( SDL_InitSubSystem( SDL_INIT_VIDEO) < 0 )
        {
            LOG_ERROR << "Update Video: failed # " << SDL_GetError() << endl;
        }
#endif
        setVideoMode();
        reload();
#ifdef DYNAMIC_GL
        grabAndWarpMouse();
#endif
    }

    ConfigS::instance()->getBoolean( "showStarfield", _showStarfield);
    ConfigS::instance()->getBoolean( "showNebulas", _showNebulas);
}
//----------------------------------------------------------------------------
void Video::updateLogic( void)
{
    _prevAngle = _angle;
    _angle += 5.0f;
}
//----------------------------------------------------------------------------
bool Video::update( void)
{
    //    XTRACE();
    static float nextTime = Timer::getTime()+0.5f;
    float thisTime = Timer::getTime();
    if( thisTime > nextTime)
    {
        updateSettings();
        nextTime = thisTime+0.5f;
    }

    if( _maxFPS)
    {
        static float sTime = Timer::getTime();
        if( (thisTime-sTime) < _fpsStepSize)
        {
            return true;
        }
        while(( thisTime-sTime) > _fpsStepSize)
            sTime+=_fpsStepSize;
    }

    FPS::Update();

    GLBitmapFont &smallFont = *_smallFont;
    GLBitmapFont &scoreFont = *_scoreFont;
    GLBitmapFont &gameOFont = *_gameOFont;

    glEnable( GL_DEPTH_TEST);
    glEnable( GL_LIGHTING);
    glEnable( GL_LIGHT0);
    glShadeModel(GL_SMOOTH);

    glEnable( GL_NORMALIZE);
    //    glEnable( GL_RESCALE_NORMAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_COLOR_MATERIAL );

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    const float fov = 53.13f;
    glFrustum(
        (4.0/3.0)*(-2.0*tan(53.13 * M_PI / 360.0)),  //xmin
        (4.0/3.0)*( 2.0*tan(53.13 * M_PI / 360.0)),  //xmax
        -2.0*tan(53.13 * M_PI / 360.0),  //ymin
         2.0*tan(53.13 * M_PI / 360.0),  //ymax
         2.0,                            //znear
         1000.0);                        //zfar

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    CameraS::instance()->update();

    //place camera
    CameraS::instance()->place();

    GLfloat light_position[] = { 20.0, 0.0, -50.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    if( GameState::isDeveloper)
    {
        //highlight the playing field, useful when using mouselook
        glDisable( GL_DEPTH_TEST);
        glDisable( GL_LIGHTING);
        float y = tan(((float)M_PI*fov)/360.0f)*100;
        float x = y*4.0f/3.0f;
        glColor4f( 1.0f, 1.0f, 1.0f, 0.1f );
        glBegin(GL_QUADS);
            glVertex3f( -x, y, -100);
            glVertex3f(  x, y, -100);
            glVertex3f(  x,-y, -100);
            glVertex3f( -x,-y, -100);
        glEnd();
        glEnable( GL_LIGHTING);
        glEnable( GL_DEPTH_TEST);
    }

    // stars are not shown in planet menu
    ContextEnum context = GameS::instance()->getContext();
    if (context != ePlanetMenu)
        StarfieldS::instance()->draw( _showStarfield, _showNebulas);

    float hyspace = GameS::instance()->_hyperspaceCount;
    int countdown = -1;
    if (hyspace != 0)
        countdown = (int)(GameState::stopwatch.getTime() - hyspace);

    if (context == eInGame || context == ePaused)
    {
        if (HeroS::instance()->alive())
        {
            glPushMatrix();
            HeroS::instance()->draw();
            glPopMatrix();
        }
        if (hyspace == 0 || countdown < 10)
            ParticleGroupManagerS::instance()->draw();
    }

    //--- Ortho stuff from here on ---
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5,VIDEO_ORTHO_WIDTH+0.5,-0.5,VIDEO_ORTHO_HEIGHT+0.5, -1000.0, 1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    if (HeroS::instance()->alive() && (context == eInGame || context == ePaused))
    {
        if (hyspace != 0)
        {
            if (countdown < 10)
            {
                glColor4f(1.0f,0.5f,0.0f,0.4f);
                char buff[100];
                sprintf(buff, "Hyperspace jump in %d seconds", 10 - countdown);
                smallFont.DrawString(buff, 500, 400, 1.0f, 1.0f, GLBitmapFont::alCenter);
            }
            if (countdown > 15)
            {
                glColor4f(0.2f,0.9f,1.0f,0.8f);
                char buff[100];
                Planet *ph = PlanetManagerS::instance()->getHyperspaceTarget();
                sprintf(buff, "Reaching %s orbit", ph->_name.c_str());
                smallFont.DrawString(buff, 500, 400, 1.0f, 1.0f, GLBitmapFont::alCenter);
            }
        }
        else
        {
            if (GameS::instance()->_spaceStationApproach != 0)
            {
                glColor4f(1.0f,1.0f,0.3f,0.8f);
                smallFont.DrawString("Approaching Space Station",
                    500, 400, 1.0f, 1.0f, GLBitmapFont::alCenter);
            }
            else if (GameS::instance()->getHyperspaceAvailable() == "OK")
            {
                glColor4f(1.0f,1.0f,1.0f,0.6f);
                smallFont.DrawString("Press H for hyperspace jump",
                    500, 30, 1.0f, 1.0f, GLBitmapFont::alCenter);
            }
        }
    }

    glColor4f(1.0,1.0,1.0,1.0);
    if (context != ePlanetMenu)
    {
        bool showFPS = false;
        ConfigS::instance()->getBoolean( "showFPS", showFPS);
        if (showFPS)
            smallFont.DrawString( FPS::GetFPSString(), 0, 0,  0.6f, 0.6f);
    }

    if (context == eInGame || context == ePaused)
    {
        if (!HeroS::instance()->alive())
        {
            glColor4f(1.0f,1.0f,1.0f,0.8f);
            gameOFont.DrawString( "GAME OVER" , 80, 320, 1.3f, 1.3f);

            glColor4f(1.0f,0.0f,0.0f,0.8f);
            smallFont.DrawString( "You are fried by the Scum of the Universe" ,
            170, 260, 1.0f, 1.0f);

            /*
            if( ScoreKeeperS::instance()->currentIsTopTen())
            {
                if( !_textInput.isOn())
                {
                    _textInput.turnOn();
                }

                glColor4f(1.0f,1.0f,1.0f,1.0f);
                smallFont.DrawString( "Top Ten Finish! RIP",215,200, 1.0f, 1.0f);

                string pname = "Enter Pilot name: ";
                pname += _textInput.getText()+"_";
                glColor4f(1.0f,0.852f,0.0f,1.0f);
                smallFont.DrawString( pname.c_str() , 215, 140, 1.0f, 1.0f);

                ScoreKeeperS::instance()->setName( _textInput.getText());
            }*/
            //else
            //{
                string escmsg = "Press ESC to exit";
                glColor4f(1.0f,0.852f,0.0f,1.0f);
                smallFont.DrawString(escmsg.c_str() , 500, 140, 1.0f, 1.0f, GLBitmapFont::alCenter);
            //}
            glColor4f(1.0f,1.0f,1.0f,1.0f);
        }
    }

    char buff[128];
    if( GameState::isDeveloper)
    {
        static float nextShow = 0;
        static int aCount = 0;
        float thisTime = Timer::getTime();
        if( thisTime > nextShow)
        {
            nextShow = thisTime + 0.5f;
            aCount = ParticleGroupManagerS::instance()->getAliveCount();
        }
        sprintf( buff, "p=%d", aCount);
        smallFont.DrawString( buff, 0, 40, 1.0, 1.0);
    }

    if (context == eMenu)
    {
        glEnable(GL_TEXTURE_2D);
        float z=-1.0;
        float dx= 1.0/512.0;

        _titleA->bind();
        glBegin(GL_QUADS);
        glTexCoord2f( dx     ,dx );   glVertex3f(350,740, z);
        glTexCoord2f( 1.0f-dx,dx );   glVertex3f(500,740, z);
        glTexCoord2f( 1.0f-dx,1-dx ); glVertex3f(500,560, z);
        glTexCoord2f( dx     ,1-dx ); glVertex3f(350,560, z);
        glEnd();

        _titleB->bind();
        glBegin(GL_QUADS);
        glTexCoord2f( dx     ,dx );   glVertex3f(500,740, z);
        glTexCoord2f( 1.0f-dx,dx );   glVertex3f(650,740, z);
        glTexCoord2f( 1.0f-dx,1-dx ); glVertex3f(650,560, z);
        glTexCoord2f( dx     ,1-dx ); glVertex3f(500,560, z);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        MenuManagerS::instance()->draw();
    }
    else if (context == ePlanetMenu)
    {
        PlanetManagerS::instance()->draw();
    }
    else if (context == eMessageBox)
    {
        MessageBoxManagerS::instance()->draw();
    }
    else // InGame, Paused, MouseLook
    {
        if (_boardVisible)
        {
            float size = 0.48f;
            glColor4f( 1.0f, 0.9f, 0.5f, 0.7f );
            scoreFont.DrawString( "POINTS:", 5.0f, 725.0f, size, size);
            scoreFont.DrawString( "KILLS:", 5.0f,  700.0f, size, size);
            sprintf (buff, "%d", ScoreKeeperS::instance()->getCurrentScore());
            scoreFont.DrawString( buff, 100.0f, 725.0f, size, size);
            sprintf (buff, "%d", GameS::instance()->_kills);
            scoreFont.DrawString( buff, 100.0f, 700.0f, size, size);

            float he = HeroS::instance()->getEnergy();
            if (he < 0.0f)
                he = 0.0f;
            sprintf( buff, "%d", (int)he);

            glColor4f( 1.0f, 1.0f, 1.0f, 0.3f );
            glBegin(GL_QUADS);
                glVertex3f( 45        , 15, -1);
                glVertex3f( 142, 15, -1);
                glVertex3f( 142, 33, -1);
                glVertex3f( 45        , 33, -1);
            glEnd();
            if (he >= 100.0f)
                glColor4f( 0.0f, 0.5f, 1.0f, 0.7f );
            else
                glColor4f( 1.0f, 0.0f, 0.0f, 0.7f );
            glBegin(GL_QUADS);
                glVertex3f( 45        , 15, -1);
                glVertex3f( 45+he*.97f, 15, -1);
                glVertex3f( 45+he*.97f, 33, -1);
                glVertex3f( 45        , 33, -1);
            glEnd();
            glColor4f(1.0,1.0,1.0,0.9f);
            scoreFont.DrawString( buff, 45, 13, size, size);

            float se = HeroS::instance()->getShieldEnergy();
            float seclip = se;
            CargoItem *item = GameS::instance()->_cargo.findItem("Shield upgrade");
            if (item && item->_quantity > 0)
                seclip *= 0.5;
            sprintf( buff, "%d", (int)se);
            glColor4f( 1.0f, 1.0f, 1.0f, 0.3f );
            glBegin(GL_QUADS);
                glVertex3f( 45        , 47, -1);
                glVertex3f( 142, 47, -1);
                glVertex3f( 142, 65, -1);
                glVertex3f( 45        , 65, -1);
            glEnd();
            if (se >= 100.0f)
                glColor4f( 1.0f, 0.7f, 0.0f, 0.6f );
            else
                glColor4f( 1.0f, 0.0f, 0.0f, 0.7f );
            glBegin(GL_QUADS);
                glVertex3f( 45              , 47, -1);
                glVertex3f( 45+seclip*.97f  , 47, -1);
                glVertex3f( 45+seclip*.97f  , 65, -1);
                glVertex3f( 45              , 65, -1);
            glEnd();
            glColor4f(1.0,1.0,1.0,0.9f);
            scoreFont.DrawString( buff, 45, 45, size, size);

            const float spacing = 40.0f;
            const float redge = 1020.0f;
            const float top = 730.0f;
            int data[3];
            StageManagerS::instance()->getCounts(data[0], data[1], data[2]);
            glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
            for (int ix = 0; ix < 3; ++ix)
            {
                sprintf(buff, "%d", data[ix]);
                scoreFont.DrawString(buff, redge - (3-ix)*spacing,
                    top - 40, 0.48f, 0.48f, GLBitmapFont::alCenter);
            }

            /*
            CargoItem *bomb = GameS::instance()->_cargo.findItem("Space grenade");
            if (bomb && bomb->_quantity > 0)
            {
                sprintf(buff, "%d", bomb->_quantity);
                scoreFont.DrawString(buff, 500, 10, 0.48f, 0.48f, GLBitmapFont::alCenter);
            }
            CargoItem *rocket = GameS::instance()->_cargo.findItem("Stinger rocket");
            if (rocket && rocket->_quantity > 0)
            {
                sprintf(buff, "%d", rocket->_quantity);
                scoreFont.DrawString(buff, 550, 10, 0.48f, 0.48f, GLBitmapFont::alCenter);
            }*/

            glEnable( GL_LIGHTING);
            glEnable( GL_DEPTH_TEST);

            GLfloat light_position2[] = { 820.0, 620.0, 500.0, 0.0 };
            glLightfv(GL_LIGHT0, GL_POSITION, light_position2);
            glDisable(GL_TEXTURE_2D);

            float iAngle = _prevAngle+(_angle-_prevAngle)*GameState::frameFraction;

            // shield and energy
            static Model *energy = ModelManagerS::instance()->getModel("models/EnergyBlob");
            glPushMatrix();
                glTranslatef(22, 22, 1.0);
                //glRotatef(-90.0f, 1.0, 0.0, 0.0);
                glRotatef(iAngle * 0.5, 1.0, 0.0, 0.0);
                glScalef(4.5f, 4.5f, 4.5f);
                energy->draw();
            glPopMatrix();

            static Model *shield = ModelManagerS::instance()->getModel("models/ShieldBoost");
            glPushMatrix();
                glTranslatef(22, 50, 1.0);
                //glRotatef(-90.0f, 1.0, 0.0, 0.0);
                glRotatef(iAngle * 0.5, 0.0, 1.0, 0.0);
                glScalef(4.5f, 4.5f, 4.5f);
                shield->draw();
            glPopMatrix();

            /*
            // draw weapons at the bottom of the screen
            if (bomb && bomb->_quantity > 0)
            {
                static Model *bomba = ModelManagerS::instance()->getModel("models/WeaponUpgrade");
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                glPushMatrix();
                    glTranslatef(470, 20, 1.0);
                    //glRotatef(-90.0f, 1.0, 0.0, 0.0);
                    glRotatef(iAngle, 0.0, 0.0, 1.0);
                    glScalef(5.0f, 5.0f, 5.0f);
                    bomba->draw();
                glPopMatrix();
            }
            if (rocket && rocket->_quantity > 0)
            {
                static Model *rocketm = ModelManagerS::instance()->getModel("models/Stinger");
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                glPushMatrix();
                    glTranslatef(520, 20, 1.0);
                    //glRotatef(-90.0f, 1.0, 0.0, 0.0);
                    glRotatef(iAngle, 0.0, 1.0, 0.0);
                    //glScalef(5.0f, 5.0f, 5.0f);
                    rocketm->draw();
                glPopMatrix();
            }*/

            // draw images of incoming enemies -------------------------------
            static Model *alien = ModelManagerS::instance()->getModel("models/SixLegBugYellow");
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glPushMatrix();
                glTranslatef(redge - 3*spacing, top, 1.0);
                glRotatef(-90.0f, 1.0, 0.0, 0.0);
                glRotatef(iAngle, 0.0, 0.0, 1.0);
                glScalef(6.0f, 6.0f, 6.0f);
                alien->draw();
            glPopMatrix();

            static Model *empire = ModelManagerS::instance()->getModel("models/DarkAngel");
            glPushMatrix();
                glTranslatef(redge - 2*spacing, top, 1.0);
                glRotatef(-15.0f, 1.0, 0.0, 0.0);
                glRotatef(iAngle, 0.0, 1.0, 0.0);
                glScalef(6.0f, 6.0f, 6.0f);
                empire->draw();
            glPopMatrix();

            static Model *rebel = ModelManagerS::instance()->getModel("models/BigFoot");
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glPushMatrix();
                glTranslatef(redge - spacing, top, 1.0);
                glRotatef(iAngle, 0.0, 1.0, 0.0);
                glScalef(6.0f, 6.0f, 6.0f);
                rebel->draw();
            glPopMatrix();

            // draw weapons
            float yoffset = 22;
            bool weaponsStarted = false;
            std::vector<CargoItemInfo> *info = CargoItemInfo::getCargoInfo();
            for (std::vector<CargoItemInfo>::iterator it = info->begin();
                it != info->end(); ++it)
            {
                if ((*it)._name == "Shield upgrade")
                    break;
                if ((*it)._name == "Proton spread fire")
                    weaponsStarted = true;
                if (!weaponsStarted)
                    continue;
                CargoItem *c = GameS::instance()->_cargo.findItem((*it)._name);
                if (c && c->_quantity > 0)
                {
                    Model *m = ModelManagerS::instance()->getModel((*it)._modelName);
                    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                    glPushMatrix();
                        glTranslatef(975, yoffset, 1.0);
                        glRotatef(iAngle * 0.5f + yoffset, 0.0, 1.0, 0.0);
                        glScalef((*it)._scale, (*it)._scale, (*it)._scale);
                        m->draw();
                    glPopMatrix();
                    if ((*it)._maxQty != 1 && c->_quantity > 1)
                    {
                        sprintf(buff, "%d", c->_quantity);
                        scoreFont.DrawString(buff, 952, yoffset - 13, 0.48f, 0.48f, GLBitmapFont::alRight);
                    }
                    yoffset += 45;
                }
            }
        }
    }

    SDL_GL_SwapBuffers( );
    return true;
}
//----------------------------------------------------------------------------
void Video::takeSnapshot( void)
{
    static int count = 0;

    int width = VideoS::instance()->getWidth();
    int height = VideoS::instance()->getHeight();
    char filename[128];
    sprintf( filename, "snap%02d.png", count++);
    SDL_Surface *img;

    img = SDL_CreateRGBSurface( SDL_SWSURFACE, width, height, 24,
        0xFF000000, 0x00FF0000, 0x0000FF00,0);

    glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, img->pixels);

    LOG_INFO << "Writing snapshot: " << filename << endl;
    if( !PNG::Snapshot( img, filename))
    {
        LOG_ERROR << "Failed to save snapshot." << endl;
    }

    SDL_FreeSurface( img);
}
//----------------------------------------------------------------------------
