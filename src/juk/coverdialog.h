/***************************************************************************
    begin                : Sun May 15 2005
    copyright            : (C) 2005 by Michael Pyne
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

#ifndef COVERDIALOG_H
#define COVERDIALOG_H

#include "ui_coverdialogbase.h"

#include <QWidget>

class CoverDialog : public QWidget, public Ui::CoverDialogBase
{
    Q_OBJECT
public:
    CoverDialog(QWidget *parent);
    ~CoverDialog();

    virtual void show();

public slots:
    void slotArtistClicked(Q3ListViewItem *item);
    void slotContextRequested(Q3IconViewItem *item, const QPoint &pt);

private slots:
    void loadCovers();
    void removeSelectedCover();
};

#endif /* COVERDIALOG_H */

// vim: set et sw=4 tw=0 sta:
