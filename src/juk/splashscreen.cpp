/***************************************************************************
    begin                : Sun Dec 8 2002
    copyright            : (C) 2002 - 2004 by Scott Wheeler
    email                : wheeler@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "splashscreen.h"

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <QPixmap>
#include <QLabel>
#include <QPalette>

SplashScreen *SplashScreen::splash = 0;
bool SplashScreen::done = false;
int SplashScreen::count = 0;

static QString loadedText(int i)
{
    static QString loading = i18n("Loading").toUpper();
    return loading + ": " + QString::number(i);;
}

////////////////////////////////////////////////////////////////////////////////
// pubic members
////////////////////////////////////////////////////////////////////////////////

SplashScreen *SplashScreen::instance()
{
    if(!splash && !done)
        splash = new SplashScreen();
    return splash;
}

void SplashScreen::finishedLoading()
{
    done = true;
    delete splash;
    splash = 0;
}

void SplashScreen::increment()
{
    if(splash) {
        count++;
        if(( count & 63 ) == 0)
            splash->processEvents();
    }
}

void SplashScreen::update()
{
    if(splash)
        splash->processEvents();
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

SplashScreen::SplashScreen() : QLabel(0, Qt::WStyle_Splash)
{
    setObjectName("splashScreen");

    QPixmap background = UserIcon("splash");
    resize(background.size());
    QPalette palette;
    palette.setBrush(backgroundRole(), QBrush(background));

    setMargin(7);
    setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    palette.setColor(foregroundRole(), QColor(107, 158, 194));
    setPalette(palette);

    QFont f = font();
    f.setPixelSize(10);
    setFont(f);

    setText(loadedText(0));
}

SplashScreen::~SplashScreen()
{

}

////////////////////////////////////////////////////////////////////////////////
// private methods
////////////////////////////////////////////////////////////////////////////////

void SplashScreen::processEvents()
{
    setText(loadedText(count));
    kapp->processEvents();
}

// vim: set et sw=4 tw=0 sta:
