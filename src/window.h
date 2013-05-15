/*
 * meego-handset-people - Contacts application
 * Copyright © 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef WINDOW_H
#define WINDOW_H

#include <MApplicationWindow>

class QEvent;

class SeasideWindow: public MApplicationWindow
{
    Q_OBJECT

signals:
    void debugAddContact();
    void debugDeleteContact();

protected:
    bool event(QEvent *event);
};

#endif // WINDOW_H
