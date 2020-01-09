/***************************************************************************
    begin                : Fri Sep 13 2002
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

#include "collectionlist.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kmenu.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kactioncollection.h>
#include <ktoolbarpopupaction.h>
#include <kdirwatch.h>

#include <QList>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QApplication>
#include <QClipboard>
#include <QFileInfo>

#include "playlistcollection.h"
#include "splashscreen.h"
#include "stringshare.h"
#include "cache.h"
#include "actioncollection.h"
#include "tag.h"
#include "viewmode.h"

using ActionCollection::action;

////////////////////////////////////////////////////////////////////////////////
// static methods
////////////////////////////////////////////////////////////////////////////////

CollectionList *CollectionList::m_list = 0;

CollectionList *CollectionList::instance()
{
    return m_list;
}

void CollectionList::loadCachedItems()
{
    if(!m_list)
        return;

    FileHandleHash::ConstIterator end = Cache::instance()->constEnd();
    for(FileHandleHash::ConstIterator it = Cache::instance()->constBegin(); it != end; ++it) {
        // This may have already been created via a loaded playlist.
        if(!m_itemsDict.contains(it.key()))
            new CollectionListItem(*it);
    }

    SplashScreen::update();

    // The CollectionList is created with sorting disabled for speed.  Re-enable
    // it here, and perform the sort.
    KConfigGroup config(KGlobal::config(), "Playlists");

    Qt::SortOrder order = Qt::DescendingOrder;
    if(config.readEntry("CollectionListSortAscending", true))
        order = Qt::AscendingOrder;

    m_list->setSortOrder(order);
    m_list->setSortColumn(config.readEntry("CollectionListSortColumn", 1));

    m_list->sort();

    SplashScreen::finishedLoading();
}

void CollectionList::initialize(PlaylistCollection *collection)
{
    if(m_list)
        return;

    // We have to delay initialization here because dynamic_cast or comparing to
    // the collection instance won't work in the PlaylistBox::Item initialization
    // won't work until the CollectionList is fully constructed.

    m_list = new CollectionList(collection);
    m_list->setName(i18n("Collection List"));

    collection->setupPlaylist(m_list, "folder-sound");
}

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

PlaylistItem *CollectionList::createItem(const FileHandle &file, Q3ListViewItem *, bool)
{
    // It's probably possible to optimize the line below away, but, well, right
    // now it's more important to not load duplicate items.

    if(m_itemsDict.contains(file.absFilePath()))
        return 0;

    PlaylistItem *item = new CollectionListItem(file);

    if(!item->isValid()) {
        kError() << "CollectionList::createItem() -- A valid tag was not created for \""
                  << file.absFilePath() << "\"" << endl;
        delete item;
        return 0;
    }

    setupItem(item);

    return item;
}

void CollectionList::clearItems(const PlaylistItemList &items)
{
    foreach(PlaylistItem *item, items) {
        Cache::instance()->remove(item->file());
        delete item;
    }

    dataChanged();
}

void CollectionList::setupTreeViewEntries(ViewMode *viewMode) const
{
    TreeViewMode *treeViewMode = dynamic_cast<TreeViewMode *>(viewMode);
    if(!treeViewMode) {
        kWarning(65432) << "Can't setup entries on a non-tree-view mode!\n";
        return;
    }

    QList<int> columnList;
    columnList << PlaylistItem::ArtistColumn;
    columnList << PlaylistItem::GenreColumn;
    columnList << PlaylistItem::AlbumColumn;

    foreach(int column, columnList)
        treeViewMode->addItems(m_columnTags[column]->keys(), column);
}

void CollectionList::slotNewItems(const KFileItemList &items)
{
    QStringList files;

    for(KFileItemList::ConstIterator it = items.constBegin(); it != items.constEnd(); ++it)
        files.append((*it).url().path());

    addFiles(files);
    update();
}

void CollectionList::slotRefreshItems(const QList<QPair<KFileItem, KFileItem> > &items)
{
    for(int i = 0; i < items.count(); ++i) {
        const KFileItem fileItem = items[i].second;
        CollectionListItem *item = lookup(fileItem.url().path());

        if(item) {
            item->refreshFromDisk();

            // If the item is no longer on disk, remove it from the collection.

            if(item->file().fileInfo().exists())
                item->repaint();
            else
                delete item;
        }
    }

    update();
}

void CollectionList::slotDeleteItem(const KFileItem &item)
{
    delete lookup(item.url().path());
}

////////////////////////////////////////////////////////////////////////////////
// public slots
////////////////////////////////////////////////////////////////////////////////

void CollectionList::paste()
{
    decode(QApplication::clipboard()->mimeData());
}

void CollectionList::clear()
{
    int result = KMessageBox::warningContinueCancel(this,
        i18n("Removing an item from the collection will also remove it from "
             "all of your playlists. Are you sure you want to continue?\n\n"
             "Note, however, that if the directory that these files are in is in "
             "your \"scan on startup\" list, they will be readded on startup."));

    if(result == KMessageBox::Continue) {
        Playlist::clear();
        emit signalCollectionChanged();
    }
}

void CollectionList::slotCheckCache()
{
    loadCachedItems();

    PlaylistItemList invalidItems;

    foreach(CollectionListItem *item, m_itemsDict) {
        if(!item->checkCurrent())
            invalidItems.append(item);
        processEvents();
    }

    clearItems(invalidItems);
}

void CollectionList::slotRemoveItem(const QString &file)
{
    delete m_itemsDict[file];
}

void CollectionList::slotRefreshItem(const QString &file)
{
    if(m_itemsDict[file])
        m_itemsDict[file]->refresh();
}

////////////////////////////////////////////////////////////////////////////////
// protected methods
////////////////////////////////////////////////////////////////////////////////

CollectionList::CollectionList(PlaylistCollection *collection) :
    Playlist(collection, true),
    m_columnTags(15, 0)
{
    QAction *spaction = ActionCollection::actions()->addAction("showPlaying");
    spaction->setText(i18n("Show Playing"));
    connect(spaction, SIGNAL(triggered(bool) ), SLOT(slotShowPlaying()));

    connect(action<KToolBarPopupAction>("back")->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotPopulateBackMenu()));
    connect(action<KToolBarPopupAction>("back")->menu(), SIGNAL(triggered(QAction *)),
            this, SLOT(slotPlayFromBackMenu(QAction *)));
    setSorting(-1); // Temporarily disable sorting to add items faster.

    m_columnTags[PlaylistItem::ArtistColumn] = new TagCountDict;
    m_columnTags[PlaylistItem::AlbumColumn] = new TagCountDict;
    m_columnTags[PlaylistItem::GenreColumn] = new TagCountDict;
}

CollectionList::~CollectionList()
{
    KConfigGroup config(KGlobal::config(), "Playlists");
    config.writeEntry("CollectionListSortColumn", sortColumn());
    config.writeEntry("CollectionListSortAscending", sortOrder() == Qt::AscendingOrder);

    // In some situations the dataChanged signal from clearItems will cause observers to
    // subsequently try to access a deleted item.  Since we're going away just remove all
    // observers.

    clearObservers();

    // The CollectionListItems will try to remove themselves from the
    // m_columnTags member, so we must make sure they're gone before we
    // are.

    clearItems(items());

    qDeleteAll(m_columnTags);
    m_columnTags.clear();
}

void CollectionList::contentsDropEvent(QDropEvent *e)
{
    if(e->source() == this)
        return; // Don't rearrange in the CollectionList.
    else
        Playlist::contentsDropEvent(e);
}

void CollectionList::contentsDragMoveEvent(QDragMoveEvent *e)
{
    if(e->source() != this)
        Playlist::contentsDragMoveEvent(e);
    else
        e->setAccepted(false);
}

QString CollectionList::addStringToDict(const QString &value, int column)
{
    if(column > m_columnTags.count() || value.trimmed().isEmpty())
        return QString();

    if(m_columnTags[column]->contains(value))
        ++((*m_columnTags[column])[value]);
    else {
        m_columnTags[column]->insert(value, 1);
        emit signalNewTag(value, column);
    }

    return value;
}

QStringList CollectionList::uniqueSet(UniqueSetType t) const
{
    int column;

    switch(t)
    {
    case Artists:
        column = PlaylistItem::ArtistColumn;
    break;

    case Albums:
        column = PlaylistItem::AlbumColumn;
    break;

    case Genres:
        column = PlaylistItem::GenreColumn;
    break;

    default:
        return QStringList();
    }

    return m_columnTags[column]->keys();
}

CollectionListItem *CollectionList::lookup(const QString &file) const
{
    return m_itemsDict.value(file, 0);
}

void CollectionList::removeStringFromDict(const QString &value, int column)
{
    if(column > m_columnTags.count() || value.trimmed().isEmpty())
        return;

    if(m_columnTags[column]->contains(value) &&
       --((*m_columnTags[column])[value])) // If the decrement goes to 0...
    {
        emit signalRemovedTag(value, column);
        m_columnTags[column]->remove(value);
    }
}

void CollectionList::addWatched(const QString &file)
{
    m_dirWatch->addFile(file);
}

void CollectionList::removeWatched(const QString &file)
{
    m_dirWatch->removeFile(file);
}

////////////////////////////////////////////////////////////////////////////////
// CollectionListItem public methods
////////////////////////////////////////////////////////////////////////////////

void CollectionListItem::refresh()
{
    int offset = static_cast<Playlist *>(listView())->columnOffset();
    int columns = lastColumn() + offset + 1;

    data()->metadata.resize(columns);
    data()->cachedWidths.resize(columns);

    for(int i = offset; i < columns; i++) {
        int id = i - offset;
        if(id != TrackNumberColumn && id != LengthColumn) {
            // All columns other than track num and length need local-encoded data for sorting

            QString toLower = text(i).toLower();

            // For some columns, we may be able to share some strings

            if((id == ArtistColumn) || (id == AlbumColumn) ||
               (id == GenreColumn)  || (id == YearColumn)  ||
               (id == CommentColumn))
            {
                toLower = StringShare::tryShare(toLower);

                if(id != YearColumn && id != CommentColumn && data()->metadata[id] != toLower) {
                    CollectionList::instance()->removeStringFromDict(data()->metadata[id], id);
                    CollectionList::instance()->addStringToDict(text(i), id);
                }
            }

            data()->metadata[id] = toLower;
        }

        int newWidth = width(listView()->fontMetrics(), listView(), i);
        data()->cachedWidths[i] = newWidth;

        if(newWidth != data()->cachedWidths[i])
            playlist()->slotWeightDirty(i);
    }

    if(listView()->isVisible())
        repaint();

    for(PlaylistItemList::Iterator it = m_children.begin(); it != m_children.end(); ++it) {
        (*it)->playlist()->update();
        (*it)->playlist()->dataChanged();
        if((*it)->listView()->isVisible())
            (*it)->repaint();
    }

    CollectionList::instance()->dataChanged();
    emit CollectionList::instance()->signalCollectionChanged();
}

PlaylistItem *CollectionListItem::itemForPlaylist(const Playlist *playlist)
{
    if(playlist == CollectionList::instance())
        return this;

    PlaylistItemList::ConstIterator it;
    for(it = m_children.constBegin(); it != m_children.constEnd(); ++it)
        if((*it)->playlist() == playlist)
            return *it;
    return 0;
}

void CollectionListItem::updateCollectionDict(const QString &oldPath, const QString &newPath)
{
    CollectionList *collection = CollectionList::instance();

    if(!collection)
        return;

    collection->removeFromDict(oldPath);
    collection->addToDict(newPath, this);
}

void CollectionListItem::repaint() const
{
    Q3ListViewItem::repaint();
    for(PlaylistItemList::ConstIterator it = m_children.constBegin(); it != m_children.constEnd(); ++it)
        (*it)->repaint();
}

////////////////////////////////////////////////////////////////////////////////
// CollectionListItem protected methods
////////////////////////////////////////////////////////////////////////////////

CollectionListItem::CollectionListItem(const FileHandle &file) :
    PlaylistItem(CollectionList::instance()),
    m_shuttingDown(false)
{
    CollectionList *l = CollectionList::instance();
    if(l) {
        l->addToDict(file.absFilePath(), this);

        data()->fileHandle = file;

        if(file.tag()) {
            refresh();
            l->dataChanged();
            // l->addWatched(m_path);
        }
        else
            kError() << "CollectionListItem::CollectionListItem() -- Tag() could not be created." << endl;
    }
    else
        kError(65432) << "CollectionListItems should not be created before "
                       << "CollectionList::initialize() has been called." << endl;

    SplashScreen::increment();
}

CollectionListItem::~CollectionListItem()
{
    m_shuttingDown = true;

    foreach(PlaylistItem *item, m_children)
        delete item;

    CollectionList *l = CollectionList::instance();
    if(l) {
        l->removeFromDict(file().absFilePath());
        l->removeStringFromDict(file().tag()->album(), AlbumColumn);
        l->removeStringFromDict(file().tag()->artist(), ArtistColumn);
        l->removeStringFromDict(file().tag()->genre(), GenreColumn);
    }
}

void CollectionListItem::addChildItem(PlaylistItem *child)
{
    m_children.append(child);
}

void CollectionListItem::removeChildItem(PlaylistItem *child)
{
    if(!m_shuttingDown)
        m_children.removeAll(child);
}

bool CollectionListItem::checkCurrent()
{
    if(!file().fileInfo().exists() || !file().fileInfo().isFile())
        return false;

    if(!file().current()) {
        file().refresh();
        refresh();
    }

    return true;
}

#include "collectionlist.moc"

// vim: set et sw=4 tw=0 sta:
