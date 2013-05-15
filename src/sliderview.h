/*
 * meego-handset-people - Contacts application
 * Copyright © 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef SLIDERVIEW_H
#define SLIDERVIEW_H

#include <MWidgetView>

#include "seasideslidermodel.h"
#include "seasidesliderstyle.h"

class QGraphicsLinearLayout;
class SeasideSlider;

class SeasideSliderView: public MWidgetView
{
    Q_OBJECT
    M_VIEW(SeasideSliderModel, SeasideSliderStyle)

public:
    SeasideSliderView(SeasideSlider *controller);
    virtual ~SeasideSliderView();

signals:
    void stopActivated(int index, const QString& stop);

protected:
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint=QSizeF()) const;
    virtual void applyStyle();
    virtual void setupModel();
    virtual void resizeEvent (QGraphicsSceneResizeEvent *event);
    virtual void drawContents(QPainter *painter, const QStyleOptionGraphicsItem *option) const;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    virtual void calculateRatios();
    virtual void calculateSize();

private:
    int findStop(const QPointF& point);

    SeasideSlider *m_controller;

    QGraphicsLinearLayout *m_layout;
    QString m_text;
    QString m_separator;
    QString m_shortSeparator;
    QString m_longSeparator;

    int m_separatorHeight;
    int m_separatorOffset;

    qreal m_lineToSep;
    qreal m_lineDiff;
    qreal m_separatorCenter;
    QRectF m_drawRect;

    QVector<int> m_stopWidths;
    int m_totalWidth;
    int m_lastIndex;

    QSizeF m_size;

    Q_DISABLE_COPY(SeasideSliderView)
};

#endif // SLIDERVIEW_H
