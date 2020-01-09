// Copyright Frerich Raabe <raabe@kde.org>.
// This notice was added by Michael Pyne <michael.pyne@kdemail.net>
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "tagguesser.h"
//#include <kaboutdata.h>
//#include <kcmdlineargs.h>
#include <QCoreApplication>
#include <QDir>
#include <iostream>

#include <stdlib.h>

using std::cout;
using std::endl;

void check( const QString &filename, const QString &title,
            const QString &artist, const QString &track,
            const QString &comment, const QString &album = QString() )
{
    cout << "Checking " << qPrintable(filename) << "...";
    TagGuesser guesser( filename );
    if ( guesser.title() != title ) {
        cout << "Error: In filename " << qPrintable(filename) << ", expected title " <<
            qPrintable(title) << ", got title " << qPrintable(guesser.title()) << endl;
        exit( 1 );
    }
    if ( guesser.artist() != artist ) {
        cout << "Error: In filename " << qPrintable(filename) << ", expected artist " <<
            qPrintable(artist) << ", got artist " << qPrintable(guesser.artist()) << endl;
        exit( 1 );
    }
    if ( guesser.track() != track ) {
        cout << "Error: In filename " << qPrintable(filename) << ", expected track " <<
            qPrintable(track) << ", got track " << qPrintable(guesser.track()) << endl;
        exit( 1 );
    }
    if ( guesser.comment() != comment ) {
        cout << "Error: In filename " << qPrintable(filename) << ", expected comment " <<
            qPrintable(comment) << ", got comment " << qPrintable(guesser.comment()) << endl;
        exit( 1 );
    }
    if ( guesser.album() != album ) {
        cout << "Error: In filename " << qPrintable(filename) << ", expected album " <<
            qPrintable(album) << ", got album " << qPrintable(guesser.album()) << endl;
        exit( 1 );
    }
    cout << "OK" << endl;
}

int main( int argc, char **argv )
{
    //KAboutData aboutData("tagguessertest", 0, ki18n("tagguessertest"), "0.1");
    //KComponentData componentData(&aboutData);
    QCoreApplication app( argc, argv );

    check( "/home/frerich/Chemical Brothers - (01) - Block rockin' beats [Live].mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", "Live" );
    check( "/home/frerich/Chemical Brothers - (01) - Block rockin' beats (Live).mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", "Live" );
    check( "/home/frerich/Chemical Brothers - (01) - Block rockin' beats.mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", QString() );
    check( "/home/frerich/Chemical Brothers - [01] - Block rockin' beats [Live].mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", "Live" );
    check( "/home/frerich/Chemical Brothers - [01] - Block rockin' beats (Live).mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", "Live" );
    check( "/home/frerich/Chemical Brothers - [01] - Block rockin' beats.mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", QString() );
    check( "/home/frerich/Chemical Brothers - 01 - Block rockin' beats [Live].mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", "Live" );
    check( "/home/frerich/Chemical Brothers - 01 - Block rockin' beats (Live).mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", "Live" );
    check( "/home/frerich/Chemical Brothers - 01 - Block rockin' beats.mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", QString() );
    check( "/home/frerich/(01) Chemical Brothers - Block rockin' beats [Live].mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", "Live" );
    check( "/home/frerich/(01) Chemical Brothers - Block rockin' beats (Live).mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", "Live" );
    check( "/home/frerich/(01) Chemical Brothers - Block rockin' beats.mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", QString() );
    check( "/home/frerich/[01] Chemical Brothers - Block rockin' beats [Live].mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", "Live" );
    check( "/home/frerich/[01] Chemical Brothers - Block rockin' beats (Live).mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", "Live" );
    check( "/home/frerich/[01] Chemical Brothers - Block rockin' beats.mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", QString() );
    check( "/home/frerich/01 Chemical Brothers - Block rockin' beats [Live].mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", "Live" );
    check( "/home/frerich/01 Chemical Brothers - Block rockin' beats (Live).mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", "Live" );
    check( "/home/frerich/01 Chemical Brothers - Block rockin' beats.mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", QString() );
    check( "/home/frerich/(Chemical Brothers) Block rockin' beats [Live].mp3",
            "Block Rockin' Beats", "Chemical Brothers", QString(), "Live" );
    check( "/home/frerich/(Chemical Brothers) Block rockin' beats (Live).mp3",
            "Block Rockin' Beats", "Chemical Brothers", QString(), "Live" );
    check( "/home/frerich/(Chemical Brothers) Block rockin' beats.mp3",
            "Block Rockin' Beats", "Chemical Brothers", QString(), QString() );
    check( "/home/frerich/Chemical Brothers - Block rockin' beats [Live].mp3",
            "Block Rockin' Beats", "Chemical Brothers", QString(), "Live" );
    check( "/home/frerich/Chemical Brothers - Block rockin' beats (Live).mp3",
            "Block Rockin' Beats", "Chemical Brothers", QString(), "Live" );
    check( "/home/frerich/Chemical Brothers - Block rockin' beats.mp3",
            "Block Rockin' Beats", "Chemical Brothers", QString(), QString() );
    check( "/home/frerich/mp3/Chemical Brothers/Dig your own hole/[01] Block rockin' beats.mp3",
            "Block Rockin' Beats", "Chemical Brothers", "01", QString(), "Dig Your Own Hole");
    check( QDir::homePath() + "/[01] Randy - Religion, religion.mp3",
            "Religion, Religion", "Randy", "01", QString(), QString() );
    check( QDir::homePath() + "/(3) Mr. Doe - Punk.mp3",
            "Punk", "Mr. Doe", "3", QString(), QString() );
    check( "c:\\music\\mp3s\\(3) Mr. Doe - Punk.mp3",
            "Punk", "Mr. Doe", "3", QString(), QString() );
    cout << "All OK" << endl;
    return 0;
}

// vim: set et sw=4 tw=0 sta:
