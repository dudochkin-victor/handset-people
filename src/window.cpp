/*
 * meego-handset-people - Contacts application
 * Copyright © 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include <QKeyEvent>

#include "window.h"

// temporary debug keystrokes to add and remove random contacts

bool SeasideWindow::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *kev = static_cast<QKeyEvent *>(event);
        if ((kev->key() == Qt::Key_A) && (kev->modifiers() & Qt::ControlModifier))
            emit debugAddContact();
        else if ((kev->key() == Qt::Key_Z) && (kev->modifiers() & Qt::ControlModifier))
            emit debugDeleteContact();
    }
    return MApplicationWindow::event(event);
}
