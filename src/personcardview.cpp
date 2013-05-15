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

#include <MImageWidget>

#include <MApplication>
#include <MApplicationWindow>

#include <MLayout>
#include <MGridLayoutPolicy>

#include <seasidepersonmodel.h>
#include <seasidelabel.h>

#include "personcardview.h"
//meegotouch-separator-background-horizontal/vertical
PersonCardView::PersonCardView(MWidgetController *controller): MWidgetView(controller)
{
    m_controller = controller;
    setObjectName("SeasideCard");
}

PersonCardView::~PersonCardView()
{
}

static MImageWidget *createCommIcon(Seaside::CommType type, const QSize& size)
{
    QString id = "empty";
    switch (type) {
    case Seaside::CommCallDialed:
        id = "icon-m-telephony-call-initiated";
        break;

    case Seaside::CommCallReceived:
        id = "icon-m-telephony-call-received";
        break;

    case Seaside::CommCallMissed:
        id = "icon-m-telephony-call-missed";
        break;

    case Seaside::CommSmsSent:
    case Seaside::CommEmailSent:
        id = "email-sent";
        break;

    case Seaside::CommSmsReceived:
    case Seaside::CommEmailReceived:
        id = "email-received";
        break;

    default:
        break;
    }

    MImageWidget *image = new MImageWidget;
    image->setImage(id, size);
    image->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    return image;
}

static MImageWidget *createPresenceIcon(Seaside::Presence presence)
{
    QString id;
    if (presence == Seaside::PresenceAvailable)
        id = "icon-m-common-presence-online";
    else if (presence == Seaside::PresenceAway)
        id = "icon-m-common-presence-away";
    else if (presence == Seaside::PresenceOffline)
        id = "icon-m-common-presence-offline";
    else
        id = "icon-m-common-presence-unknown";

    QSize size(36, 36);
    MImageWidget *image = new MImageWidget;
    image->setImage(id, size);
    image->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    return image;
}

static MImageWidget *createFavoriteIcon(bool favorite)
{
    QSize size(36, 36);
    MImageWidget *image = new MImageWidget;
    image->setImage(favorite ? "icon-s-common-favorite-mark":"icon-s-common-favorite-unmark", size);

    image->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    return image;
}

void PersonCardView::setupModel()
{
    MWidgetView::setupModel();

    // NOTE: need to clear out the old layout here if we start handling updates

    MLayout *layout = new MLayout(m_controller);
    MGridLayoutPolicy *landscapePolicy = new MGridLayoutPolicy(layout);
    MGridLayoutPolicy *portraitPolicy = new MGridLayoutPolicy(layout);
    layout->setLandscapePolicy(landscapePolicy);
    layout->setPortraitPolicy(portraitPolicy);

    SeasidePersonModel *pm = model();

    QString thumbnail = pm->thumbnail();
    MWidget *image;
    if (!thumbnail.isEmpty()) {
        image = new MImageWidget(MTheme::pixmap(pm->thumbnail()));
        image->setObjectName("SeasideContactThumbnail");
    }
    else {
        image = new MWidget;
    }
    image->setPreferredSize(64, 64);
    image->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    landscapePolicy->addItem(image, 0, 0, 2, 2, Qt::AlignTop);
    portraitPolicy->addItem(image, 0, 0, 2, 2, Qt::AlignTop);

    // handle recent communication
    MWidget *icon = NULL;
    MWidget *label = NULL;
    bool attention = false;
    foreach (SeasideCommEvent event, pm->events()) {
        Seaside::CommType type = event.type();
        if (type >= Seaside::CommCallDialed && type <= Seaside::CommCallMissed) {
            icon = createCommIcon(type, QSize(36, 36));
            icon->setObjectName("SeasideCardCommIcon");

            label = new SeasideLabel(event.getFriendlyDateString());
            if (type == Seaside::CommCallMissed) {
                attention = true;
                label->setObjectName("SeasideCardCommLabelAttention");
            }
            else
                label->setObjectName("SeasideCardCommLabel");

            MImageWidget *smallIcon = createCommIcon(type, QSize(24, 24));
            smallIcon->setObjectName("SeasideCardCommIcon");

            portraitPolicy->addItem(smallIcon, 1, 2, Qt::AlignCenter | Qt::AlignVCenter);
            break;
        }
    }

    if (!icon)
        icon = new MWidget;
    if (!label)
        label = new MWidget;

    landscapePolicy->addItem(icon, 2, 2, Qt::AlignCenter | Qt::AlignTop);
    landscapePolicy->addItem(label, 0, 4, Qt::AlignCenter | Qt::AlignTop);
    portraitPolicy->addItem(label, 1, 3, Qt::AlignCenter | Qt::AlignTop);

    int w = MApplication::instance()->activeApplicationWindow()->width();
    int h = MApplication::instance()->activeApplicationWindow()->height();

    MWidget *phone = NULL;
    const QVector<SeasideDetail>& phones = pm->phones();
    if (!phones.isEmpty()) {
        const SeasideDetail& detail = pm->phones()[0];
        phone = new SeasideLabel(detail.text());
        if (attention)
            phone->setObjectName("SeasideCardPhoneLabelAttention");
        else
            phone->setObjectName("SeasideCardPhoneLabel");
    }

    if (phone) {
        landscapePolicy->addItem(phone, 0, 3, Qt::AlignLeft | Qt::AlignTop);
        portraitPolicy->addItem(phone, 1, 2, Qt::AlignLeft);
    }

    SeasideLabel *name = new SeasideLabel(QObject::tr("%1 %2").arg(pm->firstname()).arg(pm->lastname()));

    if (attention)
        name->setObjectName("SeasideCardNameLabelAttention");
    else
        name->setObjectName("SeasideCardNameLabel");
    landscapePolicy->addItem(name, 0, 2, Qt::AlignLeft | Qt::AlignTop);
    portraitPolicy->addItem(name, 0, 2, Qt::AlignLeft | Qt::AlignTop);

    MImageWidget *presence = createPresenceIcon(pm->presence());
    landscapePolicy->addItem(presence, 2, 0, Qt::AlignLeft | Qt::AlignTop);
    portraitPolicy->addItem(presence, 2, 0, Qt::AlignLeft | Qt::AlignTop);

    MImageWidget *favorite = createFavoriteIcon(pm->favorite());
    landscapePolicy->addItem(favorite, 2, 1, Qt::AlignLeft | Qt::AlignTop);
    portraitPolicy->addItem(favorite, 2, 1, Qt::AlignLeft | Qt::AlignTop);

    landscapePolicy->setColumnFixedWidth(2, w * 0.25);
    landscapePolicy->setColumnFixedWidth(3, w * 0.25);
    landscapePolicy->setRowFixedHeight(1, h*0.004);
    landscapePolicy->setRowFixedHeight(2, h*0.004);

    portraitPolicy->setColumnFixedWidth(2, w * 0.25);
    portraitPolicy->setColumnFixedWidth(3, w * 0.25);
    portraitPolicy->setRowFixedHeight(1, w*0.004);
    portraitPolicy->setRowFixedHeight(2, w*0.004);

}

M_REGISTER_VIEW(PersonCardView, MWidgetController)
