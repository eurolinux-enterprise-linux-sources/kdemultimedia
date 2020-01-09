/***************************************************************************
    begin                : Sat Jul 9 2005
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

#ifndef COVERICONVIEW_H
#define COVERICONVIEW_H

#include <k3iconview.h>

#include "covermanager.h"

// The WebImageFetcher dialog also has a class named CoverIconViewItem and I
// don't like the idea of naming it "CoverIVI" or something, so just namespace
// it out.  I would merge them except for webimagefetcher's dependence on KIO
// and such.

namespace CoverUtility
{
    class CoverIconViewItem : public K3IconViewItem
    {
    public:
        CoverIconViewItem(coverKey id, Q3IconView *parent);

        coverKey id() const { return m_id; }

    private:
        coverKey m_id;
    };
}

using CoverUtility::CoverIconViewItem;

/**
 * This class subclasses K3IconView in order to provide cover drag-and-drop
 * support.
 *
 * @author Michael Pyne <michael.pyne@kdemail.net>
 */
class CoverIconView : public K3IconView
{
public:
    explicit CoverIconView(QWidget *parent, const char *name = 0);

    CoverIconViewItem *currentItem() const;

protected:
    virtual Q3DragObject *dragObject();
};

#endif /* COVERICONVIEW_H */

// vim: set et sw=4 tw=0 sta:
