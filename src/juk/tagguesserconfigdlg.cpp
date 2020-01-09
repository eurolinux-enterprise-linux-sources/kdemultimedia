/*
 * tagguesserconfigdlg.cpp - Copyright (c) 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include "tagguesserconfigdlg.h"
#include "tagguesser.h"

#include <kicon.h>
#include <k3listview.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <kapplication.h>

#include <QKeyEvent>

TagGuesserConfigDlg::TagGuesserConfigDlg(QWidget *parent, const char *name)
    : KDialog(parent)
{
    setObjectName(name);
    setModal(true);
    setCaption(i18n("Tag Guesser Configuration"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    showButtonSeparator(true);

    m_child = new TagGuesserConfigDlgWidget(this);
    setMainWidget(m_child);

    m_child->lvSchemes->setItemsRenameable(true);
    m_child->lvSchemes->setSorting(-1);
    m_child->lvSchemes->setDefaultRenameAction(Q3ListView::Accept);
    m_child->bMoveUp->setIcon(KIcon("arrow-up"));
    m_child->bMoveDown->setIcon(KIcon("arrow-down"));

    const QStringList schemes = TagGuesser::schemeStrings();
    QStringList::ConstIterator it = schemes.begin();
    QStringList::ConstIterator end = schemes.end();
    for (; it != end; ++it) {
        K3ListViewItem *item = new K3ListViewItem(m_child->lvSchemes, *it);
        item->moveItem(m_child->lvSchemes->lastItem());
    }

    connect(m_child->lvSchemes, SIGNAL(currentChanged(Q3ListViewItem *)),
            this, SLOT(slotCurrentChanged(Q3ListViewItem *)));
    connect(m_child->lvSchemes, SIGNAL(doubleClicked(Q3ListViewItem *, const QPoint &, int)),
            this, SLOT(slotRenameItem(Q3ListViewItem *, const QPoint &, int)));
    connect(m_child->bMoveUp, SIGNAL(clicked()), this, SLOT(slotMoveUpClicked()));
    connect(m_child->bMoveDown, SIGNAL(clicked()), this, SLOT(slotMoveDownClicked()));
    connect(m_child->bAdd, SIGNAL(clicked()), this, SLOT(slotAddClicked()));
    connect(m_child->bModify, SIGNAL(clicked()), this, SLOT(slotModifyClicked()));
    connect(m_child->bRemove, SIGNAL(clicked()), this, SLOT(slotRemoveClicked()));

    m_child->lvSchemes->setSelected(m_child->lvSchemes->firstChild(), true);
    slotCurrentChanged(m_child->lvSchemes->currentItem());

    resize( 400, 300 );
}

void TagGuesserConfigDlg::accept()
{
    if(m_child->lvSchemes->renameLineEdit()) {
        QKeyEvent returnKeyPress(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
        KApplication::sendEvent(m_child->lvSchemes->renameLineEdit(), &returnKeyPress);
    }

    QStringList schemes;
    for (Q3ListViewItem *it = m_child->lvSchemes->firstChild(); it; it = it->nextSibling())
        schemes += it->text(0);
    TagGuesser::setSchemeStrings(schemes);
    KDialog::accept();
}

void TagGuesserConfigDlg::slotCurrentChanged(Q3ListViewItem *item)
{
    m_child->bMoveUp->setEnabled(item != 0 && item->itemAbove() != 0);
    m_child->bMoveDown->setEnabled(item != 0 && item->itemBelow() != 0);
    m_child->bModify->setEnabled(item != 0);
    m_child->bRemove->setEnabled(item != 0);
}

void TagGuesserConfigDlg::slotRenameItem(Q3ListViewItem *item, const QPoint &, int c)
{
    m_child->lvSchemes->rename(item, c);
}

void TagGuesserConfigDlg::slotMoveUpClicked()
{
    Q3ListViewItem *item = m_child->lvSchemes->currentItem();
    if(item->itemAbove() == m_child->lvSchemes->firstChild())
        item->itemAbove()->moveItem(item);
    else
      item->moveItem(item->itemAbove()->itemAbove());
    m_child->lvSchemes->ensureItemVisible(item);
    slotCurrentChanged(item);
}

void TagGuesserConfigDlg::slotMoveDownClicked()
{
    Q3ListViewItem *item = m_child->lvSchemes->currentItem();
    item->moveItem(item->itemBelow());
    m_child->lvSchemes->ensureItemVisible(item);
    slotCurrentChanged(item);
}

void TagGuesserConfigDlg::slotAddClicked()
{
    K3ListViewItem *item = new K3ListViewItem(m_child->lvSchemes);
    m_child->lvSchemes->rename(item, 0);
}

void TagGuesserConfigDlg::slotModifyClicked()
{
    m_child->lvSchemes->rename(m_child->lvSchemes->currentItem(), 0);
}

void TagGuesserConfigDlg::slotRemoveClicked()
{
    delete m_child->lvSchemes->currentItem();
}

#include "tagguesserconfigdlg.moc"

// vim: set et sw=4 tw=0 sta:
