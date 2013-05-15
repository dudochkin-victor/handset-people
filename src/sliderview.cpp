/*
 * meego-handset-people - Contacts application
 * Copyright © 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include <QDebug>

#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>

#include <MApplication>

#include "sliderview.h"
#include "slider.h"

#define DUMP(x) qDebug() << #x ":" << x;

SeasideSliderView::SeasideSliderView(SeasideSlider *controller):
        MWidgetView(controller)
{
    m_controller = controller;

    m_layout = new QGraphicsLinearLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_controller->setLayout(m_layout);
    m_controller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_shortSeparator = QChar(0x00b7);
    m_longSeparator = QString(" %1 ").arg(m_shortSeparator);

    m_separatorHeight = 0;
    m_separatorOffset = 0;

    m_lineToSep = 0.0;
    m_lineDiff = 0.0;
    m_separatorCenter = 0.0;

    m_totalWidth = 0;
    m_lastIndex = 0;

    connect(this, SIGNAL(stopActivated(int,QString)),
            m_controller, SIGNAL(stopActivated(int,QString)));
}

SeasideSliderView::~SeasideSliderView()
{
}

void SeasideSliderView::calculateRatios()
{
    if (model()->stops().isEmpty())
        return;

    QFont font = style()->font();
    QRectF rect = m_controller->contentsRect();

    QFontMetrics fm(font);
    int textWidth = fm.width(model()->stops().join(m_longSeparator));
    if (textWidth) {
        int spaceWidth = rect.width();
        int percent = spaceWidth * 100 / textWidth;
        if (percent < 100)
            m_separator = m_shortSeparator;
        else
            m_separator = m_longSeparator;
    }
    m_text = model()->stops().join(m_separator);

    int count = model()->stops().size();

    m_stopWidths.clear();
    m_stopWidths.resize(count);

    float sepWidth = fm.width(m_separator);

    // give first stop only half a separator width
    float totalWidth = - sepWidth / 2;

    int i=0;
    foreach (const QString& stop, model()->stops()) {
        totalWidth += fm.width(stop) + sepWidth;
        m_stopWidths[i++] = totalWidth;
    }

    // remove extra half a separator width included in the last stop
    m_totalWidth = totalWidth - sepWidth / 2;
}

void SeasideSliderView::calculateSize()
{
    QFontMetrics fm(style()->font());
    int height = fm.height();

    int count = model()->stops().count();
    if (model()->orientation() == M::Landscape) {
        int totalWidth = 0;
        foreach (const QString& stop, model()->stops())
            totalWidth += fm.width(stop);

        int sepWidth = fm.width(m_shortSeparator);
        if (count >= 1)
            totalWidth += sepWidth * (count - 1);
        m_size = QSizeF(totalWidth, height);
    }
    else {
        int maxWidth = 0;
        foreach (const QString& stop, model()->stops()) {
            int width = fm.width(stop);
            if (width > maxWidth)
                maxWidth = width;
        }

        int totalHeight = count * height;

        int xmin, xmax, ymin, ymax;
        xmin = ymin = INT_MAX;
        xmax = ymax = INT_MIN;
        foreach (QChar ch, m_shortSeparator) {
            QRectF rect = fm.boundingRect(ch);
            if (rect.top() < ymin)
                ymin = rect.top();
            if (rect.bottom() > ymax)
                ymax = rect.bottom();
            if (rect.left() < xmin)
                xmin = rect.left();
            if (rect.right() > xmax)
                xmax = rect.right();
        }

        QRectF rect = fm.boundingRect(m_shortSeparator);
        int before = 0;
        int after = 0;
        if (rect.left() < xmin)
            before = xmin - rect.left();
        if (rect.right() > xmax)
            after = rect.right() - xmax;

        int above = 0;
        int below = 0;
        if (rect.top() < ymin)
            above = ymin - rect.top();
        if (rect.bottom() > ymax)
            below = rect.bottom() - ymax;

        // give the separator at least as much space above and below ink as it
        //   has before and after
        if (above > before)
            above -= before;
        if (below > after)
            below -= after;

        m_separatorHeight = height - above - below;
        m_separatorOffset = above;
        if (count >= 1)
            totalHeight += m_separatorHeight * (count - 1);

        m_size = QSizeF(maxWidth, totalHeight);
    }
}

QSizeF SeasideSliderView::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(which)
    Q_UNUSED(constraint)

    if (which == Qt::PreferredSize) {
        QSizeF padding(style()->paddingLeft() + style()->paddingRight(),
                       style()->paddingTop() + style()->paddingBottom());
        return m_size + padding;
    }

    return MWidgetView::sizeHint(which, constraint);
}

void SeasideSliderView::applyStyle()
{
    MWidgetView::applyStyle();

    // font could have changed, recalculate
    calculateRatios();
    calculateSize();
}

void SeasideSliderView::setupModel()
{
    MWidgetView::setupModel();

    // stops could have changed, recalculate
    calculateRatios();
    calculateSize();
}

void SeasideSliderView::resizeEvent(QGraphicsSceneResizeEvent *event) {
    if (model()->orientation() == M::Landscape)
        return;

    QFontMetrics fm(style()->font());
    int lineHeight = fm.height();

    qreal padLeft = style()->paddingLeft();
    qreal padRight = style()->paddingRight();
    qreal padTop = style()->paddingTop();
    qreal padBottom = style()->paddingBottom();

    QSizeF paddingSize(padLeft + padRight, padTop + padBottom);
    QSizeF size = event->newSize() - paddingSize;

    m_drawRect = QRectF(padLeft, padTop, size.width(), lineHeight);

    int count = model()->stops().count();
    if (count > 1)
        m_lineDiff = (size.height() - lineHeight) / (count - 1);
    else
        m_lineDiff = 0.0;

    qreal space = m_lineDiff - lineHeight;
    m_separatorCenter = padTop + lineHeight + space / 2;
    m_lineToSep = lineHeight + space / 2 - m_separatorHeight / 2.0 -
                  m_separatorOffset;
}

void SeasideSliderView::drawContents(QPainter *painter,
                                     const QStyleOptionGraphicsItem *option) const
{
    Q_UNUSED(option)

    QFont font = style()->font();
    painter->setFont(font);

    QPoint offset = style()->shadowOffset();
    bool drawShadow = !offset.isNull();

    if (model()->orientation() == M::Landscape) {
        // adjust content rect for margins
        QRectF rect = m_controller->contentsRect();
        rect.moveTop(rect.top() - style()->marginTop());
        rect.moveLeft(rect.left() - style()->marginLeft());

        QFontMetrics fm(font);

        int textWidth = fm.width(m_text);
        if (textWidth) {
            int spaceWidth = rect.width();
            int percent = spaceWidth * 100 / textWidth;
            if (percent < 100)
                font.setStretch(percent - 1);  // subtract one to fix clipping bug
        }

        // draw the shadow text
        if (drawShadow) {
            QRectF shadowRect;
            shadowRect.setTopLeft(rect.topLeft() + offset);
            shadowRect.setBottomRight(rect.bottomRight() + offset);
            painter->setPen(style()->shadowColor());
            painter->drawText(shadowRect, Qt::AlignJustify | Qt::TextJustificationForced,
                              m_text);
        }

        // draw the regular text
        painter->setPen(style()->color());
        painter->drawText(rect, Qt::AlignJustify | Qt::TextJustificationForced, m_text);
    }
    else {
        QRectF drawRect, shadowRect;
        drawRect = m_drawRect;
        shadowRect = m_drawRect;
        shadowRect.moveTopLeft(drawRect.topLeft() + offset);

        qreal sepToLine = m_lineDiff - m_lineToSep;
        painter->setPen(style()->color());

        bool first = true;
        foreach (const QString& stop, model()->stops()) {
            if (first)
                first = false;
            else {
                if (drawShadow) {
                    // draw the separator shadow
                    painter->setPen(style()->shadowColor());
                    painter->drawText(shadowRect, Qt::AlignCenter, m_separator);
                    painter->setPen(style()->color());
                    shadowRect.moveTop(shadowRect.top() + sepToLine);
                }

                // draw the separator
                painter->drawText(drawRect, Qt::AlignCenter, m_separator);
                drawRect.moveTop(drawRect.top() + sepToLine);
            }

            if (drawShadow) {
                // draw the shadow text
                painter->setPen(style()->shadowColor());
                painter->drawText(shadowRect, Qt::AlignCenter, stop);
                painter->setPen(style()->color());
                shadowRect.moveTop(shadowRect.top() + m_lineToSep);
            }

            // draw the text
            painter->drawText(drawRect, Qt::AlignCenter, stop);
            drawRect.moveTop(drawRect.top() + m_lineToSep);
        }
    }
}

int SeasideSliderView::findStop(const QPointF& point)
{
    if (model()->orientation() == M::Landscape) {
        int width = m_controller->size().width() -
                    style()->marginLeft() - style()->paddingLeft() -
                    style()->marginRight() - style()->paddingRight();

        float ratio = m_totalWidth * 1.0 / width;

        qreal pos = point.x();
        pos -= style()->marginLeft() + style()->paddingLeft();
        pos *= ratio;

        int count = m_stopWidths.count();
        int i;
        for (i=0; i<count-1; i++) {
            if (pos < m_stopWidths[i])
                break;
        }
        return i;
    }
    else {
        qreal pos = point.y();
        qreal sep = m_separatorCenter;

        int count = model()->stops().count();
        int i;
        for (i=0; i<count-1; i++) {
            if (pos < sep)
                break;
            sep += m_lineDiff;
        }
        return i;
    }
}

void SeasideSliderView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    int index = findStop(event->pos());
    emit stopActivated(index, model()->stops()[index]);
    m_lastIndex = index;
}

void SeasideSliderView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QRectF rect = m_controller->sceneBoundingRect();
    if (rect.contains(event->scenePos())) {
        int index = findStop(event->pos());
        if (index != m_lastIndex) {
            emit stopActivated(index, model()->stops()[index]);
            m_lastIndex = index;
        }
    }
}

M_REGISTER_VIEW(SeasideSliderView, SeasideSlider)
