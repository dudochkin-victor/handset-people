/*
 * meego-handset-people - Contacts application
 * Copyright © 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "slider.h"

SeasideSlider::SeasideSlider(M::Orientation orientation, MWidget *parent):
        MWidgetController(new SeasideSliderModel, parent)
{
    setObjectName("SeasideSlider");
    model()->setOrientation(orientation);
}

SeasideSlider::~SeasideSlider()
{
}

void SeasideSlider::setStops(const QStringList& list)
{
    model()->setStops(list);
}
