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

#include <QGraphicsSceneMouseEvent>

#include <MAction>

#include <seasidepersonmodel.h>
#include <seasidesyncmodel.h>

#include "peopleapp.h"

#include "personcard.h"

static MAction *createAction(const QString& text, SeasidePersonCard *card)
{
    MAction *action = new MAction(text, card);
    action->setLocation(MAction::ObjectMenuLocation);
    card->addAction(action);
    return action;
}

SeasidePersonCard::SeasidePersonCard(const QModelIndex& index, MWidget *parent):
        MWidgetController(SeasideSyncModel::createPersonModel(index), parent),
    m_index(new QPersistentModelIndex(index))
{
    setObjectName("SeasidePersonCard");

    SeasidePersonModel *pm = static_cast<SeasidePersonModel *>(model());

    //If we only have 1 phone #, call/sms (if type is mobile) it directly,
    // otherwise open up a "# to call"/"# to SMS" selection page
    MAction *action;
    if (pm->phones().count() == 1) {
        const SeasideDetail detail = pm->phones()[0];
        action = createAction(tr("Call"), this);
        m_actionToString[action] = detail.text();
        connect(action, SIGNAL(triggered()), this, SLOT(callNumber()));

        if (detail.location() == Seaside::LocationMobile) {
            action = createAction(tr("SMS"), this);
            m_actionToString[action] = detail.text();
            connect(action, SIGNAL(triggered()), this, SLOT(composeSMS()));
        }
    } else if (pm->phones().count() > 1) {
        action = new MAction(tr("Call"), this);
        action->setLocation(MAction::ObjectMenuLocation);
        addAction(action);
        connect(action, SIGNAL(triggered()),
                this, SLOT(showCallComms()));

        //If only 1 of our associated phone details is mobile,
        //then make SMS directly available, otherwise, if > 1,
        //then open up the "# to SMS" page
        QList<const SeasideDetail *> *mobileDetails = new QList<const SeasideDetail *>;
        foreach (const SeasideDetail& detail, pm->phones()) {
            if (detail.location() == Seaside::LocationMobile)
                mobileDetails->append(&detail);
        }
        if (mobileDetails->count() == 1) {
            action = createAction(tr("SMS", "Action for SMS"), this);
            m_actionToString[action] = mobileDetails->at(0)->text();
            connect(action, SIGNAL(triggered()), this, SLOT(composeSMS()));
        } else if (mobileDetails->count() > 1) {
            action = new MAction(tr("SMS","Action for SMS"), this);
            action->setLocation(MAction::ObjectMenuLocation);
            addAction(action);
            connect(action, SIGNAL(triggered()),
                    this, SLOT(showSMSComms()));
        }
    }


    //If we only have 1 associated email address, then directly
    //open up email app, otherwise open a "address to email" page
    if (pm->emails().count() == 1) {
        action = createAction(tr("Email","Action for email"), this);
        m_actionToString[action] = pm->emails()[0].text();
        connect(action, SIGNAL(triggered()), this, SLOT(composeEmail()));
    } else if (pm->emails().count() > 1) {
        action = new MAction(tr("Email","Action for email"), this);
        action->setLocation(MAction::ObjectMenuLocation);
        addAction(action);
        connect(action, SIGNAL(triggered()),
                this, SLOT(showEmailComms()));
    }

    if (pm->ims().count() == 1) {
        action = createAction(tr("IM","Action for IM"), this);
        m_actionToString[action] = pm->ims()[0].text();
        connect(action, SIGNAL(triggered()), this, SLOT(composeIM()));
    } else if (pm->ims().count() > 1) {
        action = new MAction(tr("IM","Action for IM"), this);
        action->setLocation(MAction::ObjectMenuLocation);
        addAction(action);
        connect(action, SIGNAL(triggered()),
                this, SLOT(showIMComms()));
    }

    //action = createAction(tr("Edit","Action for Edit"), this);
    //connect(action, SIGNAL(triggered()), this, SLOT(editPerson()));

    action = createAction(tr("View", "Action for going to Details view"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(viewDetails()));

    if (pm->favorite())
        action = createAction(tr("Unfavorite","Action to unmark contact as a favorite"), this);
    else
        action = createAction(tr("Favorite","Action to mark contact as a favorite"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(toggleFavorite()));

    action = createAction(tr("Delete","Action to delete a contact"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(deletePerson()));
}

SeasidePersonCard::~SeasidePersonCard()
{
    delete m_index;
}

QString SeasidePersonCard::name()
{
    SeasidePersonModel *pm = static_cast<SeasidePersonModel *>(model());
    return QObject::tr("%1 %2", "%1=firstname %2= lastname").arg(pm->firstname()).arg(pm->lastname());
}

void SeasidePersonCard::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
}

void SeasidePersonCard::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    if (sceneBoundingRect().contains(event->scenePos()))
        viewDetails();
}

void SeasidePersonCard::viewDetails()
{
    emit requestDetails(static_cast<const QModelIndex&>(*m_index));
}

void SeasidePersonCard::editPerson()
{
    emit requestEdit(static_cast<const QModelIndex&>(*m_index));
}

void SeasidePersonCard::toggleFavorite()
{
    SeasidePersonModel *pm = static_cast<SeasidePersonModel *>(model());
    emit requestSetFavorite(pm->uuid(), !pm->favorite());
}

void SeasidePersonCard::deletePerson()
{
    SeasidePersonModel *pm = static_cast<SeasidePersonModel *>(model());
    emit requestDelete(pm->uuid());
}

void SeasidePersonCard::callNumber()
{
    MAction *action = static_cast<MAction *>(sender());
    QString number = m_actionToString[action];
    if (!number.isEmpty())
        emit callNumber(number);
}

void SeasidePersonCard::composeSMS()
{
    MAction *action = static_cast<MAction *>(sender());
    QString number = m_actionToString[action];
    if (!number.isEmpty())
        emit composeSMS(number);
}

void SeasidePersonCard::composeIM()
{
  //    qWarning() << "SeasideCard: composeIM*()";
    MAction *action = static_cast<MAction *>(sender());
    QString chatcontact = m_actionToString[action];
    if (!chatcontact.isEmpty())
        emit composeIM(chatcontact);
}

void SeasidePersonCard::composeEmail()
{
    MAction *action = static_cast<MAction *>(sender());
    QString address = m_actionToString[action];
    if (!address.isEmpty())
        emit composeEmail(address);
}

void SeasidePersonCard::showCallComms()
{
    SeasidePersonModel *pm = static_cast<SeasidePersonModel *>(model());
    PeopleApplication::instance()->createCommPage(pm,
                                          PeopleApplication::CatCall);
}

void SeasidePersonCard::showIMComms()
{
    SeasidePersonModel *pm = static_cast<SeasidePersonModel *>(model());
    PeopleApplication::instance()->createCommPage(pm,
                                          PeopleApplication::CatIM);
}

void SeasidePersonCard::showSMSComms()
{
    SeasidePersonModel *pm = static_cast<SeasidePersonModel *>(model());
    PeopleApplication::instance()->createCommPage(pm,
                                          PeopleApplication::CatSMS);
}

void SeasidePersonCard::showEmailComms()
{
    SeasidePersonModel *pm = static_cast<SeasidePersonModel *>(model());
    PeopleApplication::instance()->createCommPage(pm,
                                          PeopleApplication::CatEmail);
}
