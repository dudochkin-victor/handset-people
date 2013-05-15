/*
 * meego-handset-people - Contacts application
 * Copyright © 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef SLIDER_H
#define SLIDER_H

#include <MWidgetController>

#include "seasideslidermodel.h"

class SeasideSlider: public MWidgetController
{
    Q_OBJECT
    M_CONTROLLER(SeasideSlider)

public:
    SeasideSlider(M::Orientation orientation = M::Landscape, MWidget *parent = NULL);
    virtual ~SeasideSlider();

    void setStops(const QStringList& list);

signals:
    void stopActivated(int index, const QString& stop);

private:
    Q_DISABLE_COPY(SeasideSlider);
};

#endif // SLIDER_H
