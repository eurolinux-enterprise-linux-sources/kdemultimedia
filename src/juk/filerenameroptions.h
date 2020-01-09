/***************************************************************************
    begin                : Thu Oct 28 2004
    copyright            : (C) 2004 by Michael Pyne
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

#ifndef FILERENAMEROPTIONS_H
#define FILERENAMEROPTIONS_H

#include <kdialog.h>
#include "ui_filerenameroptionsbase.h"
#include "tagrenameroptions.h"

/**
 * Base widget implementing the options for a particular tag type.
 *
 * @author Michael Pyne <michael.pyne@kdemail.net>
 */
class FileRenamerTagOptions : public QWidget, public Ui::FileRenamerTagOptionsBase
{
    Q_OBJECT
public:
    FileRenamerTagOptions(QWidget *parent, const TagRenamerOptions &options);
    virtual ~FileRenamerTagOptions() {}

    const TagRenamerOptions &options() const { return m_options; }

protected slots:
    virtual void slotBracketsChanged();
    virtual void slotTrackWidthChanged();
    virtual void slotEmptyActionChanged();

private:
    TagRenamerOptions m_options;
};

/**
 * This defines the dialog that actually gets the options from the user.
 *
 * @author Michael Pyne <michael.pyne@kdemail.net>
 */
class TagOptionsDialog : public KDialog
{
    Q_OBJECT

public:
    TagOptionsDialog(QWidget *parent, const TagRenamerOptions &options, unsigned categoryNumber);

    const TagRenamerOptions &options() const { return m_options; }

    protected slots:
    virtual void accept();

private:

    // Private methods

    void loadConfig(); // Loads m_options from KConfig
    void saveConfig(); // Saves m_options to KConfig

    // Private members

    FileRenamerTagOptions *m_widget;
    TagRenamerOptions m_options;
    unsigned m_categoryNumber;
};

#endif /* FILERENAMEROPTIONS_H */

// vim: set et sw=4 tw=0 sta:
