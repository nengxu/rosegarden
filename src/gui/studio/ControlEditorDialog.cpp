/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2009 the Rosegarden development team.
 
    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/


#include "ControlEditorDialog.h"

#include "misc/Debug.h"
#include "misc/Strings.h"
#include "base/Colour.h"
#include "base/Composition.h"
#include "base/ControlParameter.h"
#include "base/Device.h"
#include "base/Event.h"
#include "base/MidiDevice.h"
#include "base/MidiTypes.h"
#include "base/Studio.h"
#include "commands/studio/AddControlParameterCommand.h"
#include "commands/studio/ModifyControlParameterCommand.h"
#include "commands/studio/RemoveControlParameterCommand.h"
#include "ControlParameterEditDialog.h"
#include "ControlParameterItem.h"
#include "document/RosegardenDocument.h"
#include "misc/ConfigGroups.h"
#include "document/Command.h"
#include "document/CommandHistory.h"

#include <QShortcut>
#include <QMainWindow>
#include <QLayout>
#include <QApplication>
#include <QAction>
#include <QTreeWidget>
#include <QColor>
#include <QDialog>
#include <QFrame>
#include <QLabel>
#include <QTreeWidget>
#include <QPixmap>
#include <QPushButton>
#include <QSizePolicy>
#include <QString>
#include <QToolTip>
#include <QWidget>
#include <QVBoxLayout>
#include <QList>


namespace Rosegarden
{


ControlEditorDialog::ControlEditorDialog
        (
            QWidget *parent,
            RosegardenDocument *doc,
            DeviceId device
       ):
        QMainWindow(parent, "controleditordialog"),
        m_studio(&doc->getStudio()),
        m_doc(doc),
        m_device(device),
        m_modified(false)
{
    RG_DEBUG << "ControlEditorDialog::ControlEditorDialog: device is " << m_device << endl;

    QWidget *mainFrame = new QWidget(this);
    QVBoxLayout *mainFrameLayout = new QVBoxLayout;
    setCentralWidget(mainFrame);

    setWindowTitle(tr("Manage Controllers"));

    QString deviceName(tr("<no device>"));
    MidiDevice *md =
        dynamic_cast<MidiDevice *>(m_studio->getDevice(m_device));
    if (md)
        deviceName = strtoqstr(md->getName());

    // spacing hack!
    new QLabel("", mainFrame);
    new QLabel(tr("  Controllers for %1 (device %2)")
           .arg(deviceName)
           .arg(device), mainFrame);
    new QLabel("", mainFrame);
    
    QStringList sl;
    sl  << tr("Name  ")
        << tr("Type  ")
        << tr("Number  ")
        << tr("Description  ")
        << tr("Min. value  ")
        << tr("Max. value  ")
        << tr("Default value  ")
        << tr("Color  ")
        << tr("Position on instrument panel");
    
    m_treeWidget = new QTreeWidget(mainFrame);
    m_treeWidget->setHeaderLabels(sl);
    
//     m_treeWidget->setColumnAlignment(0, Qt::AlignLeft);    //&&& align per item now:
//     m_treeWidgetItem->setTextAlignment(0, Qt::AlignLeft);    
    
        
    mainFrameLayout->addWidget(m_treeWidget);
    
    // Align remaining columns centrally
//     for (int i = 1; i < 9; ++i)
//         m_treeWidget->setColumnAlignment(i, Qt::AlignHCenter);    //&&& align per item now
    
    
//     m_treeWidget->restoreLayout(ControlEditorConfigGroup);    //&&&
    
    QFrame *btnBox = new QFrame(mainFrame);
    mainFrameLayout->addWidget(btnBox);
    mainFrame->setLayout(mainFrameLayout);

    btnBox->setSizePolicy(
        QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));

    QHBoxLayout* layout = new QHBoxLayout(btnBox, 4, 10);

    m_addButton = new QPushButton(tr("Add"), btnBox);
    m_deleteButton = new QPushButton(tr("Delete"), btnBox);

    m_closeButton = new QPushButton(tr("Close"), btnBox);

    m_addButton->setToolTip(tr("Add a Control Parameter to the Studio"));

    m_deleteButton->setToolTip(tr("Delete a Control Parameter from the Studio"));

    m_closeButton->setToolTip(tr("Close the Control Parameter editor"));

    layout->addStretch(10);
    layout->addWidget(m_addButton);
    layout->addWidget(m_deleteButton);
    layout->addSpacing(30);

    layout->addWidget(m_closeButton);
    layout->addSpacing(5);

    connect(m_addButton, SIGNAL(released()),
            SLOT(slotAdd()));

    connect(m_deleteButton, SIGNAL(released()),
            SLOT(slotDelete()));

    setupActions();

    connect(CommandHistory::getInstance(), SIGNAL(commandExecuted()),
            this, SLOT(slotUpdate()));

    connect(m_treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
            SLOT(slotEdit(QTreeWidgetItem *, int)));

    // Highlight all columns - enable extended selection mode
    //
    m_treeWidget->setAllColumnsShowFocus(true);
    
    m_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
//     m_treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    initDialog();

//     setAutoSaveSettings(ControlEditorConfigGroup, true);    //&&&
}

ControlEditorDialog::~ControlEditorDialog()
{
    RG_DEBUG << "\n*** ControlEditorDialog::~ControlEditorDialog\n" << endl;

//     m_treeWidget->saveLayout(ControlEditorConfigGroup);    //&&&
    
}

void
ControlEditorDialog::initDialog()
{
    RG_DEBUG << "ControlEditorDialog::initDialog" << endl;
    slotUpdate();
}

void
ControlEditorDialog::slotUpdate(bool added)
{
    RG_DEBUG << "ControlEditorDialog::slotUpdate" << endl;

    //QPtrList<QTreeWidgetItem> selection = m_treeWidget->selectedItems();

    MidiDevice *md =
        dynamic_cast<MidiDevice *>(m_studio->getDevice(m_device));
    if (!md)
        return ;

    ControlList::const_iterator it = md->beginControllers();
    QTreeWidgetItem *item;
    int i = 0;

    m_treeWidget->clear();

    for (; it != md->endControllers(); ++it) {
        Composition &comp = m_doc->getComposition();
        
        // TODO: fix following segFault
//         if(! it){
//             continue;
//             RG_DEBUG << "WARNING: it is NULL in ControlEditorDialog::slotUpdate() ";
//         }

        QString colour =
            strtoqstr(comp.getGeneralColourMap().getNameByIndex(it->getColourIndex()));

        if (colour == "")
            colour = tr("<default>");

        QString position = QString("%1").arg(it->getIPBPosition());
        if (position.toInt() == -1)
            position = tr("<not showing>");

        QString value;
        value.sprintf("%d (0x%x)", it->getControllerValue(),
                      it->getControllerValue());

        if (it->getType() == PitchBend::EventType) {
            item = new ControlParameterItem(
                                            i++,
                                            m_treeWidget,
                                            QStringList()
                                                << strtoqstr(it->getName())
                                                << strtoqstr(it->getType())
                                                << QString("-")
                                                << strtoqstr(it->getDescription())
                                                << QString("%1").arg(it->getMin())
                                                << QString("%1").arg(it->getMax())
                                                << QString("%1").arg(it->getDefault())
                                                << colour
                                                << position 
                                          );
        } else {
            item = new ControlParameterItem(
                            i++,
                            m_treeWidget,
                            QStringList()
                                << strtoqstr(it->getName())
                                << strtoqstr(it->getType())
                                << value
                                << strtoqstr(it->getDescription())
                                << QString("%1").arg(it->getMin())
                                << QString("%1").arg(it->getMax())
                                << QString("%1").arg(it->getDefault())
                                << colour
                                << position
                           );
        }


        // create and set a colour pixmap
        //
        QPixmap colourPixmap(16, 16);
        Colour c = comp.getGeneralColourMap().getColourByIndex(it->getColourIndex());
        colourPixmap.fill(QColor(c.getRed(), c.getGreen(), c.getBlue()));
        
//         item->setPixmap(7, colourPixmap);
        item->setIcon(7, QIcon(colourPixmap));

        m_treeWidget->addTopLevelItem(item);
    }

    if(m_treeWidget->topLevelItemCount() == 0) {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_treeWidget, QStringList(tr("<none>")));
        m_treeWidget->addTopLevelItem(item);

        m_treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
    } else {
        m_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }
    
    // This logic is kind of frigged up, and may be too fragile.  It assumes
    // that if you added an item, the last thing iterated through will be that
    // new item, so the value of the variable item will be the last thing
    // iterated through, and therefore the newest item you just added and want
    // to edit.
    //
    // I got substantially far along the way to making this quick and dirty hack
    // work before I thought it would be a lot cleaner to have the
    // AddControlParameterCommand itself launch the dialog and allow the user to
    // edit the parameters before ever adding it to the list at all.  That would
    // be a lot cleaner, but it would also require going against the flow of how
    // this logic always worked, so it would require a lot more thought to
    // achieve the same end result that way.  Instead, I just used this hack
    // overloaded slotUpdate() to tell it when a new controller was added, so we
    // could grab it here and launch the dialog on the generic blah we just added
    // to the list right before this slot got called with the optional bool set
    // true.
    //
    if (added) {
        RG_DEBUG << "ControlEditorDialog: detected new item entered; launching editor" << endl;
        slotEdit(item, 0);
    }
}

/*
void
ControlEditorDialog::slotEditCopy()
{
    RG_DEBUG << "ControlEditorDialog::slotEditCopy" << endl;
}

void
ControlEditorDialog::slotEditPaste()
{
    RG_DEBUG << "ControlEditorDialog::slotEditPaste" << endl;
}
*/

void
ControlEditorDialog::slotAdd()
{
    RG_DEBUG << "ControlEditorDialog::slotAdd to device " << m_device << endl;

    AddControlParameterCommand *command =
        new AddControlParameterCommand(m_studio, m_device,
                                       ControlParameter());

    addCommandToHistory(command);
    slotUpdate(true);
}

void
ControlEditorDialog::slotDelete()
{
    RG_DEBUG << "ControlEditorDialog::slotDelete" << endl;

//     if (!m_treeWidget->currentIndex())
    if(! m_treeWidget->currentItem())
        return ;

    ControlParameterItem *item =
        dynamic_cast<ControlParameterItem*>(m_treeWidget->currentItem());

    if (item) {
        RemoveControlParameterCommand *command =
            new RemoveControlParameterCommand(m_studio, m_device, item->getId());

        addCommandToHistory(command);
    }
}

void
ControlEditorDialog::slotClose()
{
    RG_DEBUG << "ControlEditorDialog::slotClose" << endl;

    m_doc = 0;

    close();
}

void
ControlEditorDialog::setupActions()
{
    createAction("file_close", SLOT(slotClose()));
    m_closeButton->setText(tr("Close"));
    connect(m_closeButton, SIGNAL(released()), this, SLOT(slotClose()));

    createGUI("controleditor.rc");
}

void
ControlEditorDialog::addCommandToHistory(Command *command)
{
    CommandHistory::getInstance()->addCommand(command);
    setModified(false);
}

void
ControlEditorDialog::setModified(bool modified)
{
    RG_DEBUG << "ControlEditorDialog::setModified(" << modified << ")" << endl;

    if (modified) {}
    else {}

    m_modified = modified;
}

void
ControlEditorDialog::checkModified()
{
    RG_DEBUG << "ControlEditorDialog::checkModified(" << m_modified << ")"
    << endl;

}

void
ControlEditorDialog::slotEdit(QTreeWidgetItem *i, int)
{
    RG_DEBUG << "ControlEditorDialog::slotEdit" << endl;

    ControlParameterItem *item =
        dynamic_cast<ControlParameterItem*>(i);

    MidiDevice *md =
        dynamic_cast<MidiDevice *>(m_studio->getDevice(m_device));

    if (item && md) {
        ControlParameterEditDialog dialog
        (this,
         md->getControlParameter(item->getId()), m_doc);

        if (dialog.exec() == QDialog::Accepted) {
            ModifyControlParameterCommand *command =
                new ModifyControlParameterCommand(m_studio,
                                                  m_device,
                                                  dialog.getControl(),
                                                  item->getId());

            addCommandToHistory(command);
        }
    }
}

void
ControlEditorDialog::closeEvent(QCloseEvent *e)
{
    emit closing();
    close();
}

void
ControlEditorDialog::setDocument(RosegardenDocument *doc)
{
    // reset our pointers
    m_doc = doc;
    m_studio = &doc->getStudio();
    m_modified = false;

    slotUpdate();
}

}
#include "ControlEditorDialog.moc"