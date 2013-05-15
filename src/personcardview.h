/*
 * meego-handset-people - Contacts application
 * Copyright © 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef PERSONCARDVIEW_H
#define PERSONCARDVIEW_H

#include <MWidgetView>

#include <seasidepersonmodel.h>

class QGraphicsLinearLayout;
class MLayout;
class MGridLayoutPolicy;
class SeasideLabel;

class PersonCardView: public MWidgetView
{
    Q_OBJECT
    M_VIEW(SeasidePersonModel, MWidgetStyle)

public:
    PersonCardView(MWidgetController *controller);
    virtual ~PersonCardView();

protected:
    virtual void setupModel();

private:
    MWidgetController *m_controller;

    Q_DISABLE_COPY(PersonCardView)
};

#endif // PERSONCARDVIEW_H
