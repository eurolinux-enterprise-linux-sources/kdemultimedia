/***************************************************************************
    copyright            : (C) 2002 by Daniel Molkentin
    email                : molkentin@kde.org

    copyright            : (C) 2002 - 2004 by Scott Wheeler
    email                : wheeler@kde.org

    copyright            : (C) 2004 - 2008 by Michael Pyne
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

#include "systemtray.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <kaction.h>
#include <kdebug.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <ktoggleaction.h>
#include <kvbox.h>

#include <QTimer>
#include <QColor>
#include <QPushButton>
#include <QPalette>
#include <QWheelEvent>
#include <QPixmap>
#include <QMenu>
#include <Q3MimeSourceFactory>
#include <QLabel>
#include <QPainter>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QTextDocument> // Qt::escape()

#ifdef Q_WS_X11
#include <netwm.h>
#include <QX11Info>
#endif

#include "tag.h"
#include "actioncollection.h"
#include "playermanager.h"
#include "collectionlist.h"
#include "coverinfo.h"

using namespace ActionCollection;

PassiveInfo::PassiveInfo(QSystemTrayIcon *parent) :
    QFrame(static_cast<QWidget *>(0), 
        Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
    ),
    m_icon(parent),
    m_timer(new QTimer),
    m_layout(new QVBoxLayout(this)),
    m_view(0),
    m_justDie(false)
{
    connect(m_timer, SIGNAL(timeout()), SLOT(timerExpired()));
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    // I'd like StyledPanel but it doesn't work in the default KDE style...
    setFrameStyle(Box | Plain);
    setLineWidth(3);
}

void PassiveInfo::startTimer(int delay)
{
    m_timer->start(delay);
}

void PassiveInfo::show()
{
    m_timer->start(3500);
    setWindowOpacity(1.0);
    QFrame::show();
}

QWidget *PassiveInfo::view() const
{
    return m_view;
}

void PassiveInfo::setView(QWidget *view)
{
    m_layout->addWidget(view);
    view->show(); // We are still hidden though.
    adjustSize();
    positionSelf();
}

void PassiveInfo::timerExpired()
{
    // If m_justDie is set, we should just go, otherwise we should emit the
    // signal and wait for the system tray to delete us.
    if(m_justDie)
        hide();
    else
        emit timeExpired();
}

void PassiveInfo::enterEvent(QEvent *)
{
    m_timer->stop();
    emit mouseEntered();
}

void PassiveInfo::leaveEvent(QEvent *)
{
    m_justDie = true;
    m_timer->start(50);
}

void PassiveInfo::hideEvent(QHideEvent *)
{
}

void PassiveInfo::positionSelf()
{
    // Start with a QRect of our size, move it to the right spot.
    QRect r(rect());
    QPoint iconCenter(m_icon->geometry().center());
    QRect iconRect(m_icon->geometry());
    QRect curScreen(QApplication::desktop()->availableGeometry(iconCenter));

    // Decide if going on left or right.  We will be somewhere in availableGeometry, we want
    // to be on the side that opposite the systray icon.  i.e. if icon on right side of
    // screen, we go left and have our right side flush with the left edge.
    bool onRight = iconCenter.x() < curScreen.center().x();

    // For top or bottom, we want to be near the systray icon.  Assume that the systray icon
    // is near an edge and therefore it's on the top if its y() is less than the center.
    bool onTop = iconCenter.y() < curScreen.center().y();

    QPoint anchor;

    if(onRight)
        anchor.setX(iconRect.right());
    else
        anchor.setX(iconRect.left());

    if(onTop)
        anchor.setY(qMax(iconRect.bottom(), curScreen.top()));
    else
        anchor.setY(qMax(iconRect.top(), curScreen.bottom()));

    // Now make our rect hit that anchor.
    if(onTop && onRight)
        r.moveTopLeft(anchor);
    else if(onTop && !onRight)
        r.moveTopRight(anchor);
    else if(!onTop && onRight)
        r.moveBottomLeft(anchor);
    else
        r.moveBottomRight(anchor);

    move(r.topLeft());
}

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

SystemTray::SystemTray(QWidget *parent) : KSystemTrayIcon(parent),
                                          m_popup(0),
                                          m_fadeTimer(0),
                                          m_fade(true),
                                          m_hasCompositionManager(false)

{
    // This should be initialized to the number of labels that are used.
    m_labels.fill(0, 3);

    m_appPix = loadIcon("juk");

    m_playPix = createPixmap("media-playback-start");
    m_pausePix = createPixmap("media-playback-pause");

    m_forwardPix = SmallIcon("media-skip-forward");
    m_backPix = SmallIcon("media-skip-backward");

    setIcon(m_appPix);

    setToolTip();

    // Just create this here so that it show up in the DCOP interface and the key
    // bindings dialog.

    KAction *rpaction = new KAction(i18n("Redisplay Popup"), this);
    ActionCollection::actions()->addAction("showPopup", rpaction);
    connect(rpaction, SIGNAL(triggered(bool) ), SLOT(slotPlay()));

    QMenu *cm = contextMenu();

    connect(PlayerManager::instance(), SIGNAL(signalPlay()), this, SLOT(slotPlay()));
    connect(PlayerManager::instance(), SIGNAL(signalPause()), this, SLOT(slotPause()));
    connect(PlayerManager::instance(), SIGNAL(signalStop()), this, SLOT(slotStop()));

    cm->addAction( action("play") );
    cm->addAction( action("pause") );
    cm->addAction( action("stop") );
    cm->addAction( action("forward") );
    cm->addAction( action("back") );

    cm->addSeparator();

    // Pity the actionCollection doesn't keep track of what sub-menus it has.

    KActionMenu *menu = new KActionMenu(i18n("&Random Play"), this);
    actionCollection()->addAction("randomplay", menu);
    menu->addAction(action("disableRandomPlay"));
    menu->addAction(action("randomPlay"));
    menu->addAction(action("albumRandomPlay"));
    cm->addAction( menu );

    cm->addAction( action("togglePopups") );

    m_fadeTimer = new QTimer(this);
    m_fadeTimer->setObjectName("systrayFadeTimer");
    connect(m_fadeTimer, SIGNAL(timeout()), SLOT(slotNextStep()));
    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(slotActivated(QSystemTrayIcon::ActivationReason)));

    if(PlayerManager::instance()->playing())
        slotPlay();
    else if(PlayerManager::instance()->paused())
        slotPause();
}

SystemTray::~SystemTray()
{

}

////////////////////////////////////////////////////////////////////////////////
// public slots
////////////////////////////////////////////////////////////////////////////////

void SystemTray::slotPlay()
{
    if(!PlayerManager::instance()->playing())
        return;

    QPixmap cover = PlayerManager::instance()->playingFile().coverInfo()->pixmap(CoverInfo::Thumbnail);

    setIcon(m_playPix);
    setToolTip(PlayerManager::instance()->playingString(), cover);
    createPopup();
}

void SystemTray::slotTogglePopup()
{
    if(m_popup && m_popup->view()->isVisible())
        m_popup->startTimer(50);
    else
        slotPlay();
}

void  SystemTray::slotPopupLargeCover()
{
    if(!PlayerManager::instance()->playing())
        return;

    FileHandle playingFile = PlayerManager::instance()->playingFile();
    playingFile.coverInfo()->popup();
}

void SystemTray::slotStop()
{
    setIcon(m_appPix);
    setToolTip();

    delete m_popup;
    m_popup = 0;
    m_fadeTimer->stop();
}

void SystemTray::slotPopupDestroyed()
{
    for(int i = 0; i < m_labels.size(); ++i)
        m_labels[i] = 0;
}

void SystemTray::slotNextStep()
{
    // Could happen I guess if the timeout event were queued while we're deleting m_popup
    if(!m_popup)
        return;

    ++m_step;

    // If we're not fading, immediately stop the fadeout
    if(!m_fade || m_step == STEPS) {
        m_step = 0;
        m_fadeTimer->stop();
        emit fadeDone();
        return;
    }

    if(m_hasCompositionManager) {
        m_popup->setWindowOpacity((1.0 * STEPS - m_step) / STEPS);
    }
    else {
        QColor result = interpolateColor(m_step);

        for(int i = 0; i < m_labels.size() && m_labels[i]; ++i) {
            QPalette palette;
            palette.setColor(m_labels[i]->foregroundRole(), result);
            m_labels[i]->setPalette(palette);
        }
    }
}

void SystemTray::slotFadeOut()
{
    m_startColor = m_labels[0]->palette().color( QPalette::Text ); //textColor();
    m_endColor = m_labels[0]->palette().color( QPalette::Window ); //backgroundColor();

#ifdef Q_WS_X11
    m_hasCompositionManager = QX11Info::isCompositingManagerRunning();
#else
    m_hasCompositionManager = true; // Assume yes for non-X11
#endif

    connect(this, SIGNAL(fadeDone()), m_popup, SLOT(hide()));
    connect(m_popup, SIGNAL(mouseEntered()), this, SLOT(slotMouseInPopup()));
    m_fadeTimer->start(1500 / STEPS);
}

// If we receive this signal, it's because we were called during fade out.
// That means there is a single shot timer about to call slotNextStep, so we
// don't have to do it ourselves.
void SystemTray::slotMouseInPopup()
{
    m_endColor = m_labels[0]->palette().color( QPalette::Text ); //textColor();
    disconnect(SIGNAL(fadeDone()));

    if(m_hasCompositionManager)
        m_popup->setWindowOpacity(1.0);

    m_step = STEPS - 1; // Simulate end of fade to solid text
    slotNextStep();
}

////////////////////////////////////////////////////////////////////////////////
// private methods
////////////////////////////////////////////////////////////////////////////////

KVBox *SystemTray::createPopupLayout(QWidget *parent, const FileHandle &file)
{
    KVBox *infoBox = 0;

    if(buttonsToLeft()) {

        // They go to the left because JuK is on that side

        createButtonBox(parent);
        addSeparatorLine(parent);

        infoBox = new KVBox(parent);

        // Another line, and the cover, if there's a cover, and if
        // it's selected to be shown

        if(file.coverInfo()->hasCover()) {
            addSeparatorLine(parent);
            addCoverButton(parent, file.coverInfo()->pixmap(CoverInfo::Thumbnail));
        }
    }
    else {

        // Like above, but reversed.

        if(file.coverInfo()->hasCover()) {
            addCoverButton(parent, file.coverInfo()->pixmap(CoverInfo::Thumbnail));
            addSeparatorLine(parent);
        }

        infoBox = new KVBox(parent);

        addSeparatorLine(parent);
        createButtonBox(parent);
    }

    infoBox->setSpacing(3);
    infoBox->setMargin(3);
    return infoBox;
}

void SystemTray::createPopup()
{
    FileHandle playingFile = PlayerManager::instance()->playingFile();
    Tag *playingInfo = playingFile.tag();

    // If the action exists and it's checked, do our stuff

    if(!ActionCollection::action<KToggleAction>("togglePopups")->isChecked())
        return;

    delete m_popup;
    m_popup = 0;
    m_fadeTimer->stop();

    // This will be reset after this function call by slot(Forward|Back)
    // so it's safe to set it true here.
    m_fade = true;
    m_step = 0;

    m_popup = new PassiveInfo(this);
    connect(m_popup, SIGNAL(destroyed()), SLOT(slotPopupDestroyed()));
    connect(m_popup, SIGNAL(timeExpired()), SLOT(slotFadeOut()));

    KHBox *box = new KHBox(m_popup);
    box->setSpacing(15); // Add space between text and buttons

    KVBox *infoBox = createPopupLayout(box, playingFile);

    for(int i = 0; i < m_labels.size(); ++i) {
        QLabel *l = new QLabel(" ", infoBox);
        l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_labels[i] = l;
    }

    // We have to set the text of the labels after all of the
    // widgets have been added in order for the width to be calculated
    // correctly.

    int labelCount = 0;

    QString title = Qt::escape(playingInfo->title());
    m_labels[labelCount++]->setText(QString("<qt><nobr><h2>%1</h2></nobr></qt>").arg(title));

    if(!playingInfo->artist().isEmpty())
        m_labels[labelCount++]->setText(playingInfo->artist());

    if(!playingInfo->album().isEmpty()) {
        QString album = Qt::escape(playingInfo->album());
        QString s = playingInfo->year() > 0
            ? QString("<qt><nobr>%1 (%2)</nobr></qt>").arg(album).arg(playingInfo->year())
            : QString("<qt><nobr>%1</nobr></qt>").arg(album);
        m_labels[labelCount++]->setText(s);
    }

    m_popup->setView(box);
    m_popup->show();
}

bool SystemTray::buttonsToLeft() const
{
    QPoint iconCenter(geometry().center());
    QRect curScreen(QApplication::desktop()->availableGeometry(iconCenter));

    // See PassivePopup::positionSelf()
    return iconCenter.x() < curScreen.center().x();
}

QPixmap SystemTray::createPixmap(const QString &pixName)
{
    QImage bgImage = m_appPix.pixmap(22).toImage(); // 22x22
    QImage fgImage = SmallIcon(pixName).toImage(); // Should be 16x16

    KIconEffect::semiTransparent(bgImage);

    // Center smaller image inside larger one.
    QRect rect = fgImage.rect();
    rect.moveCenter(bgImage.rect().center());

    // Must use begin()/end() (or scope in this case) to avoid returning a
    // QPixmap constructed from a QImage being painted on.
    {
        QPainter painter(&bgImage);
        painter.drawImage(rect.topLeft(), fgImage);
    }

    return QPixmap::fromImage(bgImage);
}

void SystemTray::createButtonBox(QWidget *parent)
{
    KVBox *buttonBox = new KVBox(parent);

    buttonBox->setSpacing(3);

    QPushButton *forwardButton = new QPushButton(m_forwardPix, 0, buttonBox);
    forwardButton->setObjectName("popup_forward");
    connect(forwardButton, SIGNAL(clicked()), SLOT(slotForward()));

    QPushButton *backButton = new QPushButton(m_backPix, 0, buttonBox);
    backButton->setObjectName("popup_back");
    connect(backButton, SIGNAL(clicked()), SLOT(slotBack()));
}

/**
 * What happens here is that the action->trigger() call will end up invoking
 * createPopup(), which sets m_fade to true.  Before the text starts fading
 * control returns to this function, which resets m_fade to false.
 */
void SystemTray::slotBack()
{
    action("back")->trigger();
    m_fade = false;
}

void SystemTray::slotForward()
{
    action("forward")->trigger();
    m_fade = false;
}

void SystemTray::addSeparatorLine(QWidget *parent)
{
    QFrame *line = new QFrame(parent);
    line->setFrameShape(QFrame::VLine);

    // Cover art takes up 80 pixels, make sure we take up at least 80 pixels
    // even if we don't show the cover art for consistency.

    line->setMinimumHeight(80);
}

void SystemTray::addCoverButton(QWidget *parent, const QPixmap &cover)
{
    QPushButton *coverButton = new QPushButton(parent);

    coverButton->setIconSize(cover.size());
    coverButton->setIcon(cover);
    coverButton->setFixedSize(cover.size());
    coverButton->setFlat(true);

    connect(coverButton, SIGNAL(clicked()), this, SLOT(slotPopupLargeCover()));
}

QColor SystemTray::interpolateColor(int step, int steps)
{
    if(step < 0)
        return m_startColor;
    if(step >= steps)
        return m_endColor;

    // TODO: Perhaps the algorithm here could be better?  For example, it might
    // make sense to go rather quickly from start to end and then slow down
    // the progression.
    return QColor(
            (step * m_endColor.red() + (steps - step) * m_startColor.red()) / steps,
            (step * m_endColor.green() + (steps - step) * m_startColor.green()) / steps,
            (step * m_endColor.blue() + (steps - step) * m_startColor.blue()) / steps
           );
}

void SystemTray::setToolTip(const QString &tip, const QPixmap &cover)
{
    if(tip.isEmpty())
        KSystemTrayIcon::setToolTip(i18n("JuK"));
    else {
        QPixmap myCover = cover;
        if(cover.isNull())
            myCover = DesktopIcon("juk");

        QImage coverImage = myCover.toImage();
        QSize iconSize(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop));
        QSize newIconSize = coverImage.size();
        newIconSize.scale(iconSize, Qt::KeepAspectRatio);

        if(newIconSize != iconSize)
            coverImage = coverImage.scaled(newIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        Q3MimeSourceFactory::defaultFactory()->setImage("tipCover", coverImage);

        QString html = i18nc("%1 is Cover Art, %2 is the playing track, %3 is the appname",
                            "<table style=\"font-size:14pt;\" cellspacing=\"2em\">"
                            "<tr><td rowspan=\"2\">%1</td><td align=\"center\" valign=\"middle\">%2</td></tr>"
                            "<tr><td align=\"center\" valign=\"middle\"><em>%3</em></td></tr></table>",
                            QString("<img valign=\"middle\" src=\"tipCover\">"),
                            QString("<nobr>%1</nobr>").arg(tip), i18n("JuK"));

        KSystemTrayIcon::setToolTip( html);
    }
}

void SystemTray::wheelEvent(QWheelEvent *e)
{
    if(e->orientation() == Qt::Horizontal)
        return;

    // I already know the type here, but this file doesn't (and I don't want it
    // to) know about the JuK class, so a static_cast won't work, and I was told
    // that a reinterpret_cast isn't portable when combined with multiple
    // inheritance.  (This is why I don't check the result.)

    switch(e->modifiers()) {
    case Qt::ShiftButton:
        if(e->delta() > 0)
            action("volumeUp")->trigger();
        else
            action("volumeDown")->trigger();
        break;
    default:
        if(e->delta() > 0)
            action("forward")->trigger();
        else
            action("back")->trigger();
        break;
    }
    e->accept();
}

/*
 * Reimplemented this in order to use the middle mouse button
 */
void SystemTray::slotActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason != MiddleClick)
        return;

    if(action("pause")->isEnabled())
        action("pause")->trigger();
    else
        action("play")->trigger();
}

#include "systemtray.moc"

// vim: set et sw=4 tw=0 sta:
