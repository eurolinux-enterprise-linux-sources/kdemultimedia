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

#include "exampleoptions.h"

#include <QHideEvent>
#include <QShowEvent>
#include <QVBoxLayout>

#include <kurlrequester.h>
#include <klocale.h>

ExampleOptions::ExampleOptions(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    setObjectName("example options widget");
}

void ExampleOptions::exampleSelectionChanged()
{
    if(m_fileTagsButton->isChecked())
        emit fileChanged();
    else
        emit dataChanged();
}

void ExampleOptions::exampleDataChanged()
{
    emit dataChanged();
}

void ExampleOptions::exampleFileChanged()
{
    emit fileChanged();
}

ExampleOptionsDialog::ExampleOptionsDialog(QWidget *parent) :
    QDialog(parent)
{
    setObjectName("example options dialog");
    setWindowTitle(i18n("JuK"));
    QVBoxLayout *l = new QVBoxLayout(this);

    m_options = new ExampleOptions(this);
    m_options->m_exampleFile->setMode(KFile::ExistingOnly|KFile::LocalOnly);
    l->addWidget(m_options);

    // Forward signals

    connect(m_options, SIGNAL(fileChanged()), SLOT(fileModeSelected()));
    connect(m_options, SIGNAL(dataChanged()), SIGNAL(dataChanged()));
    connect(m_options->m_exampleFile, SIGNAL(urlSelected(const KUrl &)),
            this,                     SLOT(fileChanged(const KUrl &)));
    connect(m_options->m_exampleFile, SIGNAL(returnPressed(const QString &)),
            this,                     SIGNAL(fileChanged(const QString &)));
}

void ExampleOptionsDialog::fileChanged(const KUrl &url)
{
    emit fileChanged(url.path());
}

void ExampleOptionsDialog::hideEvent(QHideEvent *)
{
    emit signalHidden();
}

void ExampleOptionsDialog::showEvent(QShowEvent *)
{
    emit signalShown();
}

void ExampleOptionsDialog::fileModeSelected()
{
    emit fileChanged(m_options->m_exampleFile->url());
}

#include "exampleoptions.moc"

// vim: set et sw=4 tw=0 sta:
