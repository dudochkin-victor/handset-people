/*
 * meego-handset-people - Contacts application
 * Copyright © 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef SLIDERMODEL_H
#define SLIDERMODEL_H

#include <MWidgetModel>

class SeasideSliderModel: public MWidgetModel
{
    Q_OBJECT
    M_MODEL(SeasideSliderModel)

public:
    M_MODEL_PROPERTY(QStringList, stops, Stops, true, QStringList())
    M_MODEL_PROPERTY(M::Orientation, orientation, Orientation, true, M::Landscape)
};

#endif // SLIDERMODEL_H
