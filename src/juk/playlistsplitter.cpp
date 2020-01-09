/***************************************************************************
    begin                : Fri Sep 13 2002
    copyright            : (C) 2002 - 2004 by Scott Wheeler
    email                : wheeler@kde.org

    copyright            : (C) 2009 by Michael Pyne
    email                : michael.pyne@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "playlistsplitter.h"

#include <kicon.h>
#include <kaction.h>
#include <kglobal.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <ktoggleaction.h>
#include <kconfiggroup.h>

#include <QEvent>
#include <QVBoxLayout>
#include <QList>
#include <QStackedWidget>
#include <QSizePolicy>

#include "searchwidget.h"
#include "playlistsearch.h"
#include "actioncollection.h"
#include "tageditor.h"
#include "collectionlist.h"
#include "playermanager.h"
#include "nowplaying.h"
#include "playlistbox.h"

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

PlaylistSplitter::PlaylistSplitter(QWidget *parent, const char *name) :
    QSplitter(Qt::Horizontal, parent),
    m_newVisible(0),
    m_playlistBox(0),
    m_searchWidget(0),
    m_playlistStack(0),
    m_editor(0),
    m_nowPlaying(0)
{
    setObjectName(name);

    setupActions();
    setupLayout();
    readConfig();

    m_editor->slotUpdateCollection();
    m_editor->setupObservers();
}

PlaylistSplitter::~PlaylistSplitter()
{
    saveConfig();

    // NowPlaying depends on the PlaylistCollection, so kill it now.
    delete m_nowPlaying;
    m_nowPlaying = 0;

    delete m_searchWidget; // Take no chances here either.

    // Since we want to ensure that the shutdown process for the PlaylistCollection
    // (a base class for PlaylistBox) has a chance to write the playlists to disk
    // before they are deleted we're explicitly deleting the PlaylistBox here.

    delete m_playlistBox;
}

PlaylistInterface *PlaylistSplitter::playlist() const
{
    return m_playlistBox;
}

bool PlaylistSplitter::eventFilter(QObject *, QEvent *event)
{
    if(event->type() == FocusUpEvent::id) {
        m_searchWidget->setFocus();
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// public slots
////////////////////////////////////////////////////////////////////////////////

void PlaylistSplitter::setFocus()
{
    m_searchWidget->setFocus();
}

void PlaylistSplitter::slotFocusCurrentPlaylist()
{
    Playlist *playlist = m_playlistBox->visiblePlaylist();

    if(playlist) {
        playlist->setFocus();
        playlist->K3ListView::selectAll(false);

        // Select the top visible (and matching) item.

        PlaylistItem *item = static_cast<PlaylistItem *>(playlist->itemAt(QPoint(0, 0)));

        if(!item)
            return;

        // A little bit of a hack to make QListView repaint things properly.  Switch
        // to single selection mode, set the selection and then switch back.

        playlist->setSelectionMode(Q3ListView::Single);

        playlist->markItemSelected(item, true);
        playlist->setCurrentItem(item);

        playlist->setSelectionMode(Q3ListView::Extended);
    }
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

Playlist *PlaylistSplitter::visiblePlaylist() const
{
    return m_newVisible ? m_newVisible : m_playlistBox->visiblePlaylist();
}

void PlaylistSplitter::setupActions()
{
    KActionCollection* coll = ActionCollection::actions();
    KToggleAction *showSearch =
	 new KToggleAction(KIcon("edit-find"), i18n("Show &Search Bar"), this);
    coll->addAction("showSearch", showSearch);

    KAction *act = new KAction(KIcon("edit-clear"), i18n("Edit Track Search"), this);
    coll->addAction("editTrackSearch", act);
    act->setShortcut(Qt::Key_F6);
    connect(act, SIGNAL(triggered(bool)), SLOT(setFocus()));
}

void PlaylistSplitter::setupLayout()
{
    setOpaqueResize(false);

    // Disable the GUI until startup is complete (as indicated by PlaylistBox)

    setEnabled(false);

    // Create a splitter to go between the playlists and the editor.

    QSplitter *editorSplitter = new QSplitter(Qt::Vertical, this);
    editorSplitter->setObjectName("editorSplitter");

    // Create the playlist and the editor.

    QWidget *top = new QWidget(editorSplitter);
    QVBoxLayout *topLayout = new QVBoxLayout(top);
    topLayout->setMargin(0);
    topLayout->setSpacing(0);

    m_playlistStack = new QStackedWidget(top);
    m_playlistStack->setObjectName("playlistStack");
    m_playlistStack->installEventFilter(this);
    m_playlistStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_playlistStack->hide(); // Will be shown after CollectionList filled.

    m_editor = new TagEditor(editorSplitter);
    m_editor->setObjectName("TagEditor");

    // Make the editor as small as possible (or at least as small as recommended)

    editorSplitter->setStretchFactor(editorSplitter->indexOf(m_editor), 0);
    editorSplitter->setStretchFactor(editorSplitter->indexOf(top), 1);

    // Create the PlaylistBox

    m_playlistBox = new PlaylistBox(this, m_playlistStack);
    m_playlistBox->setObjectName( "playlistBox" );

    connect(m_playlistBox->object(), SIGNAL(signalSelectedItemsChanged()),
            this, SLOT(slotPlaylistSelectionChanged()));
    connect(m_playlistBox, SIGNAL(signalPlaylistDestroyed(Playlist *)),
            m_editor, SLOT(slotPlaylistDestroyed(Playlist *)));
    connect(m_playlistBox, SIGNAL(startupComplete()), SLOT(slotEnable()));

    // Let interested parties know we're ready
    connect(m_playlistBox, SIGNAL(startupComplete()), SIGNAL(guiReady()));

    insertWidget(0, m_playlistBox);

    m_nowPlaying = new NowPlaying(top, m_playlistBox);

    // Create the search widget -- this must be done after the CollectionList is created.

    m_searchWidget = new SearchWidget(top);
    connect(m_searchWidget, SIGNAL(signalQueryChanged()),
            this, SLOT(slotShowSearchResults()));
    connect(m_searchWidget, SIGNAL(signalDownPressed()),
            this, SLOT(slotFocusCurrentPlaylist()));
    connect(m_searchWidget, SIGNAL(signalAdvancedSearchClicked()),
            m_playlistBox->object(), SLOT(slotCreateSearchPlaylist()));
    connect(m_searchWidget, SIGNAL(signalShown(bool)),
            m_playlistBox->object(), SLOT(slotSetSearchEnabled(bool)));
    connect(m_searchWidget, SIGNAL(returnPressed()),
            m_playlistBox->object(), SLOT(slotPlayFirst()));
    connect(ActionCollection::action<KToggleAction>("showSearch"), SIGNAL(toggled(bool)),
            m_searchWidget, SLOT(setEnabled(bool)));

    topLayout->addWidget(m_nowPlaying);
    topLayout->addWidget(m_searchWidget);
    topLayout->addWidget(m_playlistStack, 1);

    // Now that GUI setup is complete, add some auto-update signals.
    connect(CollectionList::instance(), SIGNAL(signalCollectionChanged()),
            m_editor, SLOT(slotUpdateCollection()));
    connect(m_playlistStack, SIGNAL(currentChanged(int)), this, SLOT(slotPlaylistChanged(int)));

    // Show the collection on startup.
    m_playlistBox->setSelected(0, true);
}

void PlaylistSplitter::readConfig()
{
    KConfigGroup config(KGlobal::config(), "Splitter");

    QList<int> splitterSizes = config.readEntry("PlaylistSplitterSizes",QList<int>());
    if(splitterSizes.isEmpty()) {
        splitterSizes.append(100);
        splitterSizes.append(640);
    }
    setSizes(splitterSizes);

    bool showSearch = config.readEntry("ShowSearch", true);
    ActionCollection::action<KToggleAction>("showSearch")->setChecked(showSearch);
    m_searchWidget->setHidden(!showSearch);
}

void PlaylistSplitter::saveConfig()
{
    KConfigGroup config(KGlobal::config(), "Splitter");
    config.writeEntry("PlaylistSplitterSizes", sizes());
    config.writeEntry("ShowSearch", ActionCollection::action<KToggleAction>("showSearch")->isChecked());
}

void PlaylistSplitter::slotShowSearchResults()
{
    PlaylistList playlists;
    playlists.append(visiblePlaylist());
    PlaylistSearch search = m_searchWidget->search(playlists);
    visiblePlaylist()->setSearch(search);
}

void PlaylistSplitter::slotPlaylistSelectionChanged()
{
    m_editor->slotSetItems(visiblePlaylist()->selectedItems());
}

void PlaylistSplitter::slotPlaylistChanged(int i)
{
    Playlist *p = qobject_cast<Playlist *>(m_playlistStack->widget(i));

    if(!p)
        return;

    m_newVisible = p;
    m_searchWidget->setSearch(p->search());
    m_newVisible = 0;
}

void PlaylistSplitter::slotEnable()
{
    setEnabled(true); // Ready to go.
    m_playlistStack->show();
}

#include "playlistsplitter.moc"

// vim: set et sw=4 tw=0 sta:
