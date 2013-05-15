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

#include "peopledbusadaptor.h"

PeopleDBusAdaptor::PeopleDBusAdaptor(PeopleApplication *app) :
    QDBusAbstractAdaptor(app), mApp(app) 
{}

void PeopleDBusAdaptor::showDetailsPage(uint id)
{
  QContactLocalId contact = qdbus_cast<QContactLocalId>(id);
  mApp->showDetailsPage(contact);
}

void PeopleDBusAdaptor::setThumbnailForContact(QString id, QString path)
{
  QUuid contact(id);
  mApp->setThumbnailPath(contact, path);
}

