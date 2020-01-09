/***************************************************************************
    begin                : Sat Sep 7 2002
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

#include "tageditor.h"
#include "collectionlist.h"
#include "playlistitem.h"
#include "tag.h"
#include "actioncollection.h"
#include "tagtransactionmanager.h"

#include <kactioncollection.h>
#include <kconfiggroup.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <knuminput.h>
#include <ktextedit.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kicon.h>
#include <ktoggleaction.h>
#include <kshortcut.h>

#include <QLabel>
#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QValidator>
#include <QToolTip>
#include <QEventLoop>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>

#include <id3v1genres.h>

#undef KeyRelease

class FileNameValidator : public QValidator
{
public:
    FileNameValidator(QObject *parent, const char *name = 0) :
        QValidator(parent)
    {
        setObjectName(name);
    }

    virtual void fixup(QString &s) const
    {
        s.remove('/');
    }

    virtual State validate(QString &s, int &) const
    {
        if(s.contains('/'))
           return Invalid;
        return Acceptable;
    }
};

#if 0 // Qt4 porting: this needs to be in the ::event() for m_fileNameBox (i.e. it needs a KLineEdit-derived class)
class FileBoxToolTip : public QToolTip
{
public:
    FileBoxToolTip(TagEditor *editor, QWidget *widget) :
        QToolTip(widget), m_editor(editor) {}
protected:
    virtual void maybeTip(const QPoint &)
    {
        tip(parentWidget()->rect(), m_editor->items().first()->file().absFilePath());
    }
private:
    TagEditor *m_editor;
};
#endif

class FixedHLayout : public QHBoxLayout
{
public:
    FixedHLayout(QWidget *parent, int margin = 0, int spacing = -1, const char *name = 0) :
        QHBoxLayout(parent),
        m_width(-1)
    {
        setMargin( margin );
        setSpacing( spacing );
        setObjectName( name );
    }
    FixedHLayout(QLayout *parentLayout, int spacing = -1, const char *name = 0) :
        QHBoxLayout(),
        m_width(-1)
    {
        parentLayout->addItem( this );
        setSpacing( spacing );
        setObjectName( name );
    }
    void setWidth(int w = -1)
    {
        m_width = w == -1 ? QHBoxLayout::minimumSize().width() : w;
    }
    virtual QSize minimumSize() const
    {
        QSize s = QHBoxLayout::minimumSize();
        s.setWidth(m_width);
        return s;
    }
private:
    int m_width;
};

class CollectionObserver : public PlaylistObserver
{
public:
    CollectionObserver(TagEditor *parent) :
        PlaylistObserver(CollectionList::instance()),
        m_parent(parent)
    {
    }

    virtual void updateData()
    {
        if(m_parent && m_parent->m_currentPlaylist && m_parent->isVisible())
            m_parent->slotSetItems(m_parent->m_currentPlaylist->selectedItems());
    }

    virtual void updateCurrent() {}

private:
    TagEditor *m_parent;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

TagEditor::TagEditor(QWidget *parent) :
    QWidget(parent),
    m_currentPlaylist(0),
    m_observer(0),
    m_performingSave(false)
{
    setupActions();
    setupLayout();
    readConfig();
    m_dataChanged = false;
    m_collectionChanged = false;

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

TagEditor::~TagEditor()
{
    delete m_observer;
    saveConfig();
}

void TagEditor::setupObservers()
{
    m_observer = new CollectionObserver(this);
}

////////////////////////////////////////////////////////////////////////////////
// public slots
////////////////////////////////////////////////////////////////////////////////

void TagEditor::slotSetItems(const PlaylistItemList &list)
{
    if(m_performingSave)
        return;

    // Store the playlist that we're setting because saveChangesPrompt
    // can delete the PlaylistItems in list.

    Playlist *itemPlaylist = 0;
    if(!list.isEmpty())
        itemPlaylist = list.first()->playlist();

    bool hadPlaylist = m_currentPlaylist != 0;

    saveChangesPrompt();

    if(m_currentPlaylist) {
        disconnect(m_currentPlaylist, SIGNAL(signalAboutToRemove(PlaylistItem *)),
                   this, SLOT(slotItemRemoved(PlaylistItem *)));
    }

    if((hadPlaylist && !m_currentPlaylist) || !itemPlaylist) {
        m_currentPlaylist = 0;
        m_items.clear();
    }
    else {
        m_currentPlaylist = itemPlaylist;

        // We can't use list here, it may not be valid

        m_items = itemPlaylist->selectedItems();
    }

    if(m_currentPlaylist) {
        connect(m_currentPlaylist, SIGNAL(signalAboutToRemove(PlaylistItem *)),
                this, SLOT(slotItemRemoved(PlaylistItem *)));
        connect(m_currentPlaylist, SIGNAL(destroyed()), this, SLOT(slotPlaylistRemoved()));
    }

    if(isVisible())
        slotRefresh();
    else
        m_collectionChanged = true;
}

void TagEditor::slotRefresh()
{
    // This method takes the list of currently selected m_items and tries to
    // figure out how to show that in the tag editor.  The current strategy --
    // the most common case -- is to just process the first item.  Then we
    // check after that to see if there are other m_items and adjust accordingly.

    if(m_items.isEmpty() || !m_items.first()->file().tag()) {
        slotClear();
        setEnabled(false);
        return;
    }

    setEnabled(true);

    PlaylistItem *item = m_items.first();

    Q_ASSERT(item);

    Tag *tag = item->file().tag();

    QFileInfo fi(item->file().absFilePath());
    if(!fi.isWritable() && m_items.count() == 1)
        setEnabled(false);

    m_artistNameBox->setEditText(tag->artist());
    m_trackNameBox->setText(tag->title());
    m_albumNameBox->setEditText(tag->album());

    m_fileNameBox->setText(item->file().fileInfo().fileName());
    //// TODO: new FileBoxToolTip(this, m_fileNameBox);

    m_bitrateBox->setText(QString::number(tag->bitrate()));
    m_lengthBox->setText(tag->lengthString());

    if(m_genreList.indexOf(tag->genre()) >= 0)
        m_genreBox->setCurrentIndex(m_genreList.indexOf(tag->genre()) + 1);
    else {
        m_genreBox->setCurrentIndex(0);
        m_genreBox->setEditText(tag->genre());
    }

    m_trackSpin->setValue(tag->track());
    m_yearSpin->setValue(tag->year());

    m_commentBox->setPlainText(tag->comment());

    // Start at the second item, since we've already processed the first.

    PlaylistItemList::Iterator it = m_items.begin();
    ++it;

    // If there is more than one item in the m_items that we're dealing with...

    if(it != m_items.end()) {

        foreach(QWidget *w, m_hideList)
            w->hide();

        foreach(QCheckBox *box, m_enableBoxes) {
            box->setChecked(true);
            box->show();
        }

        // Yep, this is ugly.  Loop through all of the files checking to see
        // if their fields are the same.  If so, by default, enable their
        // checkbox.

        // Also, if there are more than 50 m_items, don't scan all of them.

        if(m_items.count() > 50) {
            m_enableBoxes[m_artistNameBox]->setChecked(false);
            m_enableBoxes[m_trackNameBox]->setChecked(false);
            m_enableBoxes[m_albumNameBox]->setChecked(false);
            m_enableBoxes[m_genreBox]->setChecked(false);
            m_enableBoxes[m_trackSpin]->setChecked(false);
            m_enableBoxes[m_yearSpin]->setChecked(false);
            m_enableBoxes[m_commentBox]->setChecked(false);
        }
        else {
            for(; it != m_items.end(); ++it) {
                tag = (*it)->file().tag();

                if(tag) {

                    if(m_artistNameBox->currentText() != tag->artist() &&
                       m_enableBoxes.contains(m_artistNameBox))
                    {
                        m_artistNameBox->lineEdit()->clear();
                        m_enableBoxes[m_artistNameBox]->setChecked(false);
                    }
                    if(m_trackNameBox->text() != tag->title() &&
                       m_enableBoxes.contains(m_trackNameBox))
                    {
                        m_trackNameBox->clear();
                        m_enableBoxes[m_trackNameBox]->setChecked(false);
                    }
                    if(m_albumNameBox->currentText() != tag->album() &&
                       m_enableBoxes.contains(m_albumNameBox))
                    {
                        m_albumNameBox->lineEdit()->clear();
                        m_enableBoxes[m_albumNameBox]->setChecked(false);
                    }
                    if(m_genreBox->currentText() != tag->genre() &&
                       m_enableBoxes.contains(m_genreBox))
                    {
                        m_genreBox->lineEdit()->clear();
                        m_enableBoxes[m_genreBox]->setChecked(false);
                    }
                    if(m_trackSpin->value() != tag->track() &&
                       m_enableBoxes.contains(m_trackSpin))
                    {
                        m_trackSpin->setValue(0);
                        m_enableBoxes[m_trackSpin]->setChecked(false);
                    }
                    if(m_yearSpin->value() != tag->year() &&
                       m_enableBoxes.contains(m_yearSpin))
                    {
                        m_yearSpin->setValue(0);
                        m_enableBoxes[m_yearSpin]->setChecked(false);
                    }
                    if(m_commentBox->toPlainText() != tag->comment() &&
                       m_enableBoxes.contains(m_commentBox))
                    {
                        m_commentBox->clear();
                        m_enableBoxes[m_commentBox]->setChecked(false);
                    }
                }
            }
        }
    }
    else {
        // Clean up in the case that we are only handling one item.

        foreach(QWidget *w, m_hideList)
            w->show();

        foreach(QCheckBox *box, m_enableBoxes) {
            box->setChecked(true);
            box->hide();
        }
    }
    m_dataChanged = false;
}

void TagEditor::slotClear()
{
    m_artistNameBox->lineEdit()->clear();
    m_trackNameBox->clear();
    m_albumNameBox->lineEdit()->clear();
    m_genreBox->setCurrentIndex(0);
    m_fileNameBox->clear();
    m_trackSpin->setValue(0);
    m_yearSpin->setValue(0);
    m_lengthBox->clear();
    m_bitrateBox->clear();
    m_commentBox->clear();
}

void TagEditor::slotUpdateCollection()
{
    if(isVisible())
        updateCollection();
    else
        m_collectionChanged = true;
}

void TagEditor::updateCollection()
{
    m_collectionChanged = false;

    CollectionList *list = CollectionList::instance();

    if(!list)
        return;

    QStringList artistList = list->uniqueSet(CollectionList::Artists);
    artistList.sort();
    m_artistNameBox->clear();
    m_artistNameBox->addItems(artistList);
    m_artistNameBox->completionObject()->setItems(artistList);

    QStringList albumList = list->uniqueSet(CollectionList::Albums);
    albumList.sort();
    m_albumNameBox->clear();
    m_albumNameBox->addItems(albumList);
    m_albumNameBox->completionObject()->setItems(albumList);

    // Merge the list of genres found in tags with the standard ID3v1 set.

    StringHash genreHash;

    m_genreList = list->uniqueSet(CollectionList::Genres);

    for(QStringList::ConstIterator it = m_genreList.constBegin(); it != m_genreList.constEnd(); ++it)
        genreHash.insert(*it);

    TagLib::StringList genres = TagLib::ID3v1::genreList();

    for(TagLib::StringList::Iterator it = genres.begin(); it != genres.end(); ++it)
        genreHash.insert(TStringToQString((*it)));

    m_genreList = genreHash.values();
    m_genreList.sort();

    m_genreBox->clear();
    m_genreBox->addItem(QString());
    m_genreBox->addItems(m_genreList);
    m_genreBox->completionObject()->setItems(m_genreList);
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void TagEditor::readConfig()
{
    // combo box completion modes

    KConfigGroup config(KGlobal::config(), "TagEditor");
    if(m_artistNameBox && m_albumNameBox) {
        readCompletionMode(config, m_artistNameBox, "ArtistNameBoxMode");
        readCompletionMode(config, m_albumNameBox, "AlbumNameBoxMode");
        readCompletionMode(config, m_genreBox, "GenreBoxMode");
    }

    bool show = config.readEntry("Show", false);
    ActionCollection::action<KToggleAction>("showEditor")->setChecked(show);
    setVisible(show);

    TagLib::StringList genres = TagLib::ID3v1::genreList();

    for(TagLib::StringList::ConstIterator it = genres.begin(); it != genres.end(); ++it)
        m_genreList.append(TStringToQString((*it)));
    m_genreList.sort();

    m_genreBox->clear();
    m_genreBox->addItem(QString());
    m_genreBox->addItems(m_genreList);
    m_genreBox->completionObject()->setItems(m_genreList);
}

void TagEditor::readCompletionMode(const KConfigGroup &config, KComboBox *box, const QString &key)
{
    KGlobalSettings::Completion mode =
        KGlobalSettings::Completion(config.readEntry(key, (int)KGlobalSettings::CompletionAuto));

    box->setCompletionMode(mode);
}

void TagEditor::saveConfig()
{
    // combo box completion modes

    KConfigGroup config(KGlobal::config(), "TagEditor");

    if(m_artistNameBox && m_albumNameBox) {
        config.writeEntry("ArtistNameBoxMode", (int)m_artistNameBox->completionMode());
        config.writeEntry("AlbumNameBoxMode", (int)m_albumNameBox->completionMode());
        config.writeEntry("GenreBoxMode", (int)m_genreBox->completionMode());
    }
    config.writeEntry("Show", ActionCollection::action<KToggleAction>("showEditor")->isChecked());
}

void TagEditor::setupActions()
{
    KToggleAction *show = new KToggleAction(KIcon("document-properties"), i18n("Show &Tag Editor"), this);
    ActionCollection::actions()->addAction("showEditor", show);
    connect(show, SIGNAL(toggled(bool)), this, SLOT(setShown(bool)));

    KAction *act = new KAction(KIcon("document-save"), i18n("&Save"), this);
    ActionCollection::actions()->addAction("saveItem", act);
    act->setShortcut(Qt::CTRL + Qt::Key_T);
    connect(act, SIGNAL(triggered(bool)), SLOT(slotSave()));
}

void TagEditor::setupLayout()
{
    static const int horizontalSpacing = 2;
    static const int verticalSpacing = 2;

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(horizontalSpacing);
    layout->setMargin(0);

    //////////////////////////////////////////////////////////////////////////////
    // define two columns of the bottem layout
    //////////////////////////////////////////////////////////////////////////////
    QVBoxLayout *leftColumnLayout = new QVBoxLayout();
    layout->addItem( leftColumnLayout );
    leftColumnLayout->setSpacing(verticalSpacing);
    leftColumnLayout->setMargin(0);
    QVBoxLayout *rightColumnLayout = new QVBoxLayout();
    layout->addItem( rightColumnLayout );
    rightColumnLayout->setSpacing(verticalSpacing);
    rightColumnLayout->setMargin(0);

    layout->setStretchFactor(leftColumnLayout, 2);
    layout->setStretchFactor(rightColumnLayout, 3);

    //////////////////////////////////////////////////////////////////////////////
    // put stuff in the left column -- all of the field names are class wide
    //////////////////////////////////////////////////////////////////////////////
    { // just for organization

        m_artistNameBox = new KComboBox( true, this );
        m_artistNameBox->setObjectName( "artistNameBox" );
        m_artistNameBox->setCompletionMode(KGlobalSettings::CompletionAuto);
        addItem(i18n("&Artist name:"), m_artistNameBox, leftColumnLayout, "view-media-artist");

        m_trackNameBox = new KLineEdit(this);
        m_trackNameBox->setObjectName( "trackNameBox" );
        addItem(i18n("&Track name:"), m_trackNameBox, leftColumnLayout, "media-playback-start");

        m_albumNameBox = new KComboBox( true, this );
        m_albumNameBox->setObjectName( "albumNameBox" );
        m_albumNameBox->setCompletionMode(KGlobalSettings::CompletionAuto);
        addItem(i18n("Album &name:"), m_albumNameBox, leftColumnLayout, "media-optical-audio");

        m_genreBox = new KComboBox( true, this );
        m_genreBox->setObjectName( "genreBox" );
        addItem(i18n("&Genre:"), m_genreBox, leftColumnLayout, "help-about");

        // this fills the space at the bottem of the left column
        leftColumnLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum,
                                                  QSizePolicy::Expanding));
    }
    //////////////////////////////////////////////////////////////////////////////
    // put stuff in the right column
    //////////////////////////////////////////////////////////////////////////////
    { // just for organization

        QHBoxLayout *fileNameLayout = new QHBoxLayout();
        rightColumnLayout->addItem(fileNameLayout);
        fileNameLayout->setSpacing(horizontalSpacing);

        m_fileNameBox = new KLineEdit(this);
        m_fileNameBox->setObjectName( "fileNameBox" );
        m_fileNameBox->setValidator(new FileNameValidator(m_fileNameBox));

        QLabel *fileNameIcon = new QLabel(this);
        fileNameIcon->setPixmap(SmallIcon("audio-x-generic"));
        QLabel * tmp = new QLabel(i18n("&File name:"), this);
        tmp->setBuddy( m_fileNameBox );
        QWidget *fileNameLabel = addHidden(tmp);

        fileNameLayout->addWidget(addHidden(fileNameIcon));
        fileNameLayout->addWidget(fileNameLabel);
        fileNameLayout->setStretchFactor(fileNameIcon, 0);
        fileNameLayout->setStretchFactor(fileNameLabel, 0);
        fileNameLayout->insertStretch(-1, 1);
        rightColumnLayout->addWidget(addHidden(m_fileNameBox));

        { // lay out the track row
            FixedHLayout *trackRowLayout = new FixedHLayout(rightColumnLayout,
                                                            horizontalSpacing);

            m_trackSpin = new KIntSpinBox(0, 9999, 1, 0, this);
            m_trackSpin->setObjectName( "trackSpin" );
            addItem(i18nc("cd track number", "T&rack:"), m_trackSpin, trackRowLayout);
            m_trackSpin->installEventFilter(this);

            m_yearSpin = new KIntSpinBox(0, 9999, 1, 0, this );
            m_yearSpin->setObjectName( "yearSpin" );
            addItem(i18n("&Year:"), m_yearSpin, trackRowLayout);
            m_yearSpin->installEventFilter(this);

            trackRowLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding,
                                                    QSizePolicy::Minimum));

            trackRowLayout->setWidth();
        }
        {
            FixedHLayout *trackRowLayout = new FixedHLayout(rightColumnLayout,
                                                            horizontalSpacing);

            m_lengthBox = new KLineEdit(this);
            m_lengthBox->setObjectName( "lengthBox" );
            // addItem(i18n("Length:"), m_lengthBox, trackRowLayout);
            m_lengthBox->setMinimumWidth(fontMetrics().width("00:00") + trackRowLayout->spacing());
            m_lengthBox->setMaximumWidth(60);
            m_lengthBox->setAlignment(Qt::AlignRight);
            m_lengthBox->setReadOnly(true);
            addItem(i18n("Length:"), m_lengthBox, trackRowLayout);

            m_bitrateBox = new KLineEdit(this);
            m_bitrateBox->setObjectName( "bitrateBox" );
            // addItem(i18n("Bitrate:"), m_bitrateBox, trackRowLayout);
            m_bitrateBox->setMinimumWidth(fontMetrics().width("000") + trackRowLayout->spacing());
            m_bitrateBox->setMaximumWidth(60);
            m_bitrateBox->setAlignment(Qt::AlignRight);
            m_bitrateBox->setReadOnly(true);
            addItem(i18n("Bitrate:"), m_bitrateBox, trackRowLayout);

            trackRowLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding,
                                                    QSizePolicy::Minimum));

            trackRowLayout->setWidth();
        }

        m_commentBox = new KTextEdit(this);
        m_commentBox->setObjectName( "commentBox" );
        addItem(i18n("&Comment:"), m_commentBox, rightColumnLayout, "document-properties");
        fileNameLabel->setMinimumHeight(m_trackSpin->height());

    }

    connect(m_artistNameBox, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotDataChanged()));

    connect(m_trackNameBox, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotDataChanged()));

    connect(m_albumNameBox, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotDataChanged()));

    connect(m_genreBox, SIGNAL(activated(int)),
            this, SLOT(slotDataChanged()));

    connect(m_genreBox, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotDataChanged()));

    connect(m_fileNameBox, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotDataChanged()));

    connect(m_yearSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotDataChanged()));

    connect(m_trackSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotDataChanged()));

    connect(m_commentBox, SIGNAL(textChanged()),
            this, SLOT(slotDataChanged()));
}

void TagEditor::save(const PlaylistItemList &list)
{
    if(!list.isEmpty() && m_dataChanged) {

        KApplication::setOverrideCursor(Qt::WaitCursor);
        m_dataChanged = false;
        m_performingSave = true;

        // The list variable can become corrupted if the playlist holding its
        // items dies, which is possible as we edit tags.  So we need to copy
        // the end marker.

        PlaylistItemList::ConstIterator end = list.end();

        for(PlaylistItemList::ConstIterator it = list.begin(); it != end; /* Deliberatly missing */ ) {

            // Process items before we being modifying tags, as the dynamic
            // playlists will try to modify the file we edit if the tag changes
            // due to our alterations here.

            qApp->processEvents(QEventLoop::ExcludeUserInput);

            PlaylistItem *item = *it;

            // The playlist can be deleted from under us if this is the last
            // item and we edit it so that it doesn't match the search, which
            // means we can't increment the iterator, so let's do it now.

            ++it;

            QString fileName = item->file().fileInfo().path() + QDir::separator() +
                               m_fileNameBox->text();
            if(list.count() > 1)
                fileName = item->file().fileInfo().absoluteFilePath();

            Tag *tag = TagTransactionManager::duplicateTag(item->file().tag(), fileName);

            // A bit more ugliness.  If there are multiple files that are
            // being modified, they each have a "enabled" checkbox that
            // says if that field is to be respected for the multiple
            // files.  We have to check to see if that is enabled before
            // each field that we write.

            if(m_enableBoxes[m_artistNameBox]->isChecked())
                tag->setArtist(m_artistNameBox->currentText());
            if(m_enableBoxes[m_trackNameBox]->isChecked())
                tag->setTitle(m_trackNameBox->text());
            if(m_enableBoxes[m_albumNameBox]->isChecked())
                tag->setAlbum(m_albumNameBox->currentText());
            if(m_enableBoxes[m_trackSpin]->isChecked()) {
                if(m_trackSpin->text().isEmpty())
                    m_trackSpin->setValue(0);
                tag->setTrack(m_trackSpin->value());
            }
            if(m_enableBoxes[m_yearSpin]->isChecked()) {
                if(m_yearSpin->text().isEmpty())
                    m_yearSpin->setValue(0);
                tag->setYear(m_yearSpin->value());
            }
            if(m_enableBoxes[m_commentBox]->isChecked())
                tag->setComment(m_commentBox->toPlainText());

            if(m_enableBoxes[m_genreBox]->isChecked())
                tag->setGenre(m_genreBox->currentText());

            TagTransactionManager::instance()->changeTagOnItem(item, tag);
        }

        TagTransactionManager::instance()->commit();
        CollectionList::instance()->dataChanged();
        m_performingSave = false;
        KApplication::restoreOverrideCursor();
    }
}

void TagEditor::saveChangesPrompt()
{
    if(!isVisible() || !m_dataChanged || m_items.isEmpty())
        return;

    QStringList files;

    for(PlaylistItemList::Iterator it = m_items.begin(); it != m_items.end(); ++it)
        files.append((*it)->file().absFilePath());

    if(KMessageBox::questionYesNoList(this,
                                      i18n("Do you want to save your changes to:\n"),
                                      files,
                                      i18n("Save Changes"),
                                      KStandardGuiItem::save(),
                                      KStandardGuiItem::discard(),
                                      "tagEditor_showSaveChangesBox") == KMessageBox::Yes)
    {
        save(m_items);
    }
}

void TagEditor::addItem(const QString &text, QWidget *item, QBoxLayout *layout, const QString &iconName)
{
    if(!item || !layout)
        return;

    QLabel *label = new QLabel(text, this);
    label->setBuddy( item );
    QLabel *iconLabel = new QLabel(0, this);
    iconLabel->setBuddy( item );

    if(!iconName.isNull())
        iconLabel->setPixmap(SmallIcon(iconName));

    QCheckBox *enableBox = new QCheckBox(i18n("Enable"), this);
    enableBox->setChecked(true);

    label->setMinimumHeight(enableBox->height());

    if(layout->direction() == QBoxLayout::LeftToRight) {
        layout->addWidget(iconLabel);
        layout->addWidget(label);
        layout->addWidget(item);
        layout->addWidget(enableBox);
    }
    else {
        QHBoxLayout *l = new QHBoxLayout();
        l->setMargin(0);
        layout->addItem(l);

        l->addWidget(iconLabel);
        l->addWidget(label);
        l->setStretchFactor(label, 1);

        l->insertStretch(-1, 1);

        l->addWidget(enableBox);
        l->setStretchFactor(enableBox, 0);

        layout->addWidget(item);
    }

    enableBox->hide();

    connect(enableBox, SIGNAL(toggled(bool)), item, SLOT(setEnabled(bool)));
    m_enableBoxes.insert(item, enableBox);
}

void TagEditor::showEvent(QShowEvent *e)
{
    if(m_collectionChanged) {
        updateCollection();
        slotRefresh();
    }

    QWidget::showEvent(e);
}

bool TagEditor::eventFilter(QObject *watched, QEvent *e)
{
    QKeyEvent *ke = static_cast<QKeyEvent*>(e);
    if(watched->inherits("QSpinBox") && e->type() == QEvent::KeyRelease && ke->modifiers() == 0)
        slotDataChanged();

    return false;
}

////////////////////////////////////////////////////////////////////////////////
// private slots
////////////////////////////////////////////////////////////////////////////////

void TagEditor::slotDataChanged(bool c)
{
    m_dataChanged = c;
}

void TagEditor::slotItemRemoved(PlaylistItem *item)
{
    m_items.removeAll(item);
    if(m_items.isEmpty())
        slotRefresh();
}

void TagEditor::slotPlaylistDestroyed(Playlist *p)
{
    if(m_currentPlaylist == p) {
        m_currentPlaylist = 0;
        slotSetItems(PlaylistItemList());
    }
}

#include "tageditor.moc"

// vim: set et sw=4 tw=0 sta:
