/*
 * meego-handset-people - Meego Handset People application
 *
 * Copyright (c) 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef PEOPLEDBUSADAPTOR_H
#define PEOPLEDBUSADAPTOR_H

#include <QDBusAbstractAdaptor>
#include <QDBusArgument>
#include <QUuid>
 
#include "peopleapp.h"

using namespace QtMobility;

#define CLIENTNAME "people"
#define DBUSINTFNAME QString("com.meego.").append(CLIENTNAME)
#define DBUSOBJPATH QString("/com/meego/").append(CLIENTNAME)

/*QT_DECLARE_METATYPE(QContactLocalId)
QDBusArgument &operator<<(QDBusArgument &argument, const QContactLocalId &localid)
{
  argument.beginStructure();
  argument << localid;
  argument.endStructure();
  return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QContactLocalId &localid )
{
  argument.beginStructure();
  argument >> localid;
  argument.endStructure();
  return argument;
}
*/
class PeopleDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.meego.people")

public:
    PeopleDBusAdaptor(PeopleApplication *app);

public slots:
    void showDetailsPage(uint id);
    void setThumbnailForContact(QString id, QString path);

private:
    PeopleApplication *mApp;

};


#endif // PEOPLEDBUSADAPTOR_H
