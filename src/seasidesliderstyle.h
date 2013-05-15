/*
 * meego-handset-people - Contacts application
 * Copyright © 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef SLIDERSTYLE_H
#define SLIDERSTYLE_H

#include <MWidgetStyle>
#include <QPoint>
#include <QFont>

class SeasideSliderStyle: public MWidgetStyle
{
    Q_OBJECT
    M_STYLE(SeasideSliderStyle)

    M_STYLE_ATTRIBUTE(QColor, color, Color);
    M_STYLE_ATTRIBUTE(QColor, shadowColor, ShadowColor);
    M_STYLE_ATTRIBUTE(QColor, shadowColored, ShadowColored);
    M_STYLE_ATTRIBUTE(QPoint, shadowOffset, ShadowOffset);
    M_STYLE_ATTRIBUTE(QFont, font, Font);
    M_STYLE_ATTRIBUTE(QString, fontWeight, FontWeight);
};

class SeasideSliderStyleContainer: public MWidgetStyleContainer
{
    M_STYLE_CONTAINER(SeasideSliderStyle)
};

#endif // SLIDERSTYLE_H
