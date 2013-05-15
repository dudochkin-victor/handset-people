/*
 * meego-handset-people - Contacts application
 * Copyright © 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef PEOPLEGROUPEDVIEW_H
#define PEOPLEGROUPEDVIEW_H

#include <MWidgetView>

class QModelIndex;
class QGraphicsGridLayout;
class QAbstractItemModel;
class SeasidePeople;

class PeopleGroupedView: public MWidgetView
{
    Q_OBJECT
    M_VIEW(MWidgetModel, MWidgetStyle)

public:
    PeopleGroupedView(SeasidePeople *controller);
    virtual ~PeopleGroupedView();

protected:
    virtual void setupModel();
    virtual void orientationChangeEvent(MOrientationChangeEvent *event);

    virtual void insertCard(const QModelIndex& index);
    virtual void removeCard(int row);

protected slots:
    void rowsInserted(const QModelIndex& parent, int start, int end);
    void rowsRemoved(const QModelIndex& parent, int start, int end);
    void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void rowsMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd,
                   const QModelIndex& destParent, int destRow);
    void layoutChanged();
    void modelReset();
    void scrollTo(const QString& name);
    void adjustGeometry();

private:
    QGraphicsGridLayout *m_mainLayout;
    QAbstractItemModel *m_itemModel;

    SeasidePeople *m_controller;

    QStringList m_headings;
    QStringList m_displayHeadings;
    int m_numSections;
    QVector<int> m_sectionCounts;

    Q_DISABLE_COPY(PeopleGroupedView)
};

#endif // PEOPLEGROUPEDVIEW_H
