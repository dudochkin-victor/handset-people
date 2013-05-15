/*
 * meego-handset-people - Contacts application
 * Copyright © 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef PERSONCARD_H
#define PERSONCARD_H

#include <QString>
#include <QDateTime>
#include <QModelIndex>

#include <MWidgetController>

#include <seaside.h>

class QPersistentModelIndex;
class MAction;
class SeasidePersonModel;

class SeasidePersonCard: public MWidgetController
{
    Q_OBJECT

public:
    SeasidePersonCard(const QModelIndex& index, MWidget *parent = NULL);
    virtual ~SeasidePersonCard();

    QString name();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

signals:
    void requestDetails(const QModelIndex& index);
    void requestEdit(const QModelIndex& index);
    void requestSetFavorite(const QUuid& uuid, bool favorite);
    void requestDelete(const QUuid& uuid);
    void callNumber(const QString& number);
    void composeSMS(const QString& number);
    void composeIM(const QString& number);
    void composeEmail(const QString& address);

protected slots:
    void viewDetails();
    void editPerson();
    void toggleFavorite();
    void deletePerson();
    void callNumber();
    void composeSMS();
    void composeIM();
    void composeEmail();
    void showSMSComms();
    void showEmailComms();
    void showCallComms();
    void showIMComms();

private:
    QPersistentModelIndex *m_index;

    QMap<MAction *, QString> m_actionToString;
};

#endif // PERSONCARD_H
