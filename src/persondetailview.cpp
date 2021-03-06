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

#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>

#include <MLabel>
#include <MImageWidget>
#include <MButton>

#include <MLayout>
#include <MGridLayoutPolicy>
#include <MBasicLayoutAnimation>

#include <seasidelabel.h>

#include "persondetailview.h"
#include "imagelabel.h"
#include "person.h"

enum Section {
    SectionRecent,
    SectionPhone,
    SectionEmail,
    SectionAddress,
    SectionIM,
    SectionEnd
};

void PersonDetailView::clearArrays()
{
    for (int i=0; i<SectionEnd; ++i) {
        m_visible[i] = true;
        m_headers[i] = NULL;
        m_widgets[i] = NULL;
        m_layouts[i] = NULL;
    }
}

PersonDetailView::PersonDetailView(MWidgetController *controller): MWidgetView(controller)
{
    m_controller = controller;
    m_mainPolicy = NULL;

    m_visible = new bool[SectionEnd];
    m_headers = new SeasideLabel *[SectionEnd];
    m_widgets = new MWidgetController *[SectionEnd];
    m_layouts = new QGraphicsLinearLayout *[SectionEnd];
    clearArrays();

    SeasidePerson *person = dynamic_cast<SeasidePerson *>(m_controller);
    if (person) {
        connect(this, SIGNAL(callNumber(QString)), person, SIGNAL(callNumber(QString)));
        connect(this, SIGNAL(composeSMS(QString)), person, SIGNAL(composeSMS(QString)));
        connect(this, SIGNAL(composeIM(QString)), person, SIGNAL(composeIM(QString)));
        connect(this, SIGNAL(composeEmail(QString)), person, SIGNAL(composeEmail(QString)));
        connect(this, SIGNAL(viewRequest(qreal,qreal)),
                person, SIGNAL(viewRequest(qreal,qreal)));
    }
}

PersonDetailView::~PersonDetailView()
{
    delete [] m_visible;
    delete [] m_headers;
    delete [] m_widgets;
    delete [] m_layouts;
}

static const char *getPresenceIconName(Seaside::Presence presence) 
{
    switch (presence) {
    case Seaside::PresenceAvailable:
        return "icon-m-common-presence-online";

    case Seaside::PresenceAway:
        return "icon-m-common-presence-away";

    case Seaside::PresenceBusy: //REVISIT
        return "icon-m-common-presence-busy";

    case Seaside::PresenceOffline:
        return "icon-m-common-presence-offline";

    default:
        return "icon-m-common-presence-unknown";
    }
}

void PersonDetailView::setupModel()
{
    MWidgetView::setupModel();

    QGraphicsLayout *layout = m_controller->layout();
    if (layout) {
        int count = layout->count();
        for (int i=0; i<count; i++) {
            QGraphicsLayoutItem *item = layout->itemAt(0);
            layout->removeAt(0);
            delete item;
        }
    }

    MLayout *mainLayout = new MLayout(m_controller);
    new MBasicLayoutAnimation(mainLayout);
    m_mainPolicy = new MGridLayoutPolicy(mainLayout);
    clearArrays();

    SeasidePersonModel *pm = model();

    MWidgetController *widget = new MWidgetController;
    widget->setViewType("background");
    widget->setObjectName("SeasideDetailHeaderBox");
    widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_mainPolicy->addItem(widget, 0, 0);

    QGraphicsGridLayout *grid = new QGraphicsGridLayout(widget);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(0);

    MImageWidget *image = new MImageWidget(MTheme::pixmap(pm->thumbnail()));
    image->setObjectName("SeasideContactThumbnail");
    image->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    grid->addItem(image, 0, 0, Qt::AlignVCenter);

    QGraphicsLinearLayout *linear = new QGraphicsLinearLayout(Qt::Vertical);
    linear->setContentsMargins(0, 0, 0, 0);
    linear->setSpacing(0);
    grid->addItem(linear, 0, 1, Qt::AlignVCenter);

    QString fullname;
    if(!(pm->lastname()).isEmpty())
      { 
	fullname = QObject::tr("%1 %2").arg(pm->firstname()).arg(pm->lastname());
      }
    else
      {
	fullname = QObject::tr("%1").arg(pm->firstname());
      }
    const QString& name = fullname;
    if (!name.isEmpty()) {
        MLabel *label = new MLabel(QString("<b>%1</b>").arg(name));
        label->setObjectName("SeasideContactName");
        linear->addItem(label);
    }

    const QString& company = pm->company();
    if (!company.isEmpty()) {
        MLabel *label = new MLabel(pm->company());
        label->setObjectName("SeasideContactCompany");
        linear->addItem(label);
    }

    const char *presenceIcon = getPresenceIconName(pm->presence());
    image = new MImageWidget(MTheme::pixmap(presenceIcon));
    image->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    grid->addItem(image, 0, 2, Qt::AlignTop);

    const char *fav = pm->favorite() ? "icon-s-common-favorite-mark":"icon-s-common-favorite-unmark";
    image = new MImageWidget(MTheme::pixmap(fav));
    image->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    grid->addItem(image, 0, 3, Qt::AlignTop);

    foreach (const SeasideDetail& detail, pm->phones())
        addPhone(detail);

    foreach (const SeasideDetail& detail, pm->emails())
        addEmail(detail);

    foreach (const SeasideDetail& detail, pm->ims())
        addIM(detail);

    foreach (const SeasideDetail& detail, pm->addresses())
        addAddress(detail);
}

static SeasideLabel *createHeader(const QString &text) {
    SeasideLabel *label = new SeasideLabel(text);
    label->setObjectName("DetailHeader");
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    label->setMouseEnabled(true);
    return label;
}

static ImageLabel *createImageLabel(const QString &name, const QString &text)
{
    ImageLabel *label = new ImageLabel;
    label->setObjectName(name);
    label->setText(text);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    return label;
}

void PersonDetailView::addSection(int index, const QString& title)
{
    m_headers[index] = createHeader(title);
    m_widgets[index] = new MWidgetController;
    m_widgets[index]->setViewType("background");
    m_widgets[index]->setObjectName("SeasideDetailBox");

    m_layouts[index] = new QGraphicsLinearLayout(Qt::Vertical, m_widgets[index]);
    m_layouts[index]->setContentsMargins(0, 0, 0, 0);
    m_layouts[index]->setSpacing(0);

    m_mainPolicy->addItem(m_headers[index], 2 * index + 1, 0);
    m_mainPolicy->addItem(m_widgets[index], 2 * index + 2, 0);
    connect(m_headers[index], SIGNAL(clicked(SeasideLabel *)),
            this, SLOT(toggleSection(SeasideLabel *)));
}

void PersonDetailView::addRecent(const SeasideDetail& detail)
{
    if (!m_headers[SectionRecent])
        addSection(SectionRecent, "Recent conversations");  // TODO: i18n

    // TODO: choose different object names when we support recent events
    m_layouts[SectionRecent]->addItem(createImageLabel("CallDialed", detail.text()));
}

void PersonDetailView::addIM(const SeasideDetail& detail)
{

  //   qWarning() << "adding IM Account" << m_headers[SectionIM] << "text: " << detail.text();

    if(detail.text().isEmpty())
        return;

    if (!m_headers[SectionIM])
        addSection(SectionIM, "IM Accounts");  // TODO: i18n

    QString objectName = "imaccount";
    QGraphicsLinearLayout *linear = new QGraphicsLinearLayout(Qt::Horizontal);
    linear->setContentsMargins(0, 0, 0, 0);
    linear->setSpacing(0);
    linear->addItem(createImageLabel(objectName, detail.text()));

    SeasideLabel *button = new SeasideLabel("IM");  // TODO: i18n
        button->setObjectName("SeasideIMButton");
    button->setMouseEnabled(true);
    m_labelToDetail[button] = detail;
    connect(button, SIGNAL(clicked(SeasideLabel*)),
            this, SLOT(composeIM(SeasideLabel*)));
    linear->addItem(button);

    m_layouts[SectionIM]->addItem(linear);
}


void PersonDetailView::addPhone(const SeasideDetail& detail)
{
    if (!m_headers[SectionPhone])
        addSection(SectionPhone, "Telephone numbers");  // TODO: i18n

    QString objectName;
    switch (detail.location()) {
    case Seaside::LocationMobile:
        objectName = "MobilePhone";
        break;
    case Seaside::LocationWork:
        objectName = "WorkPhone";
        break;
    default:
        objectName = "HomePhone";
        break;
    }

    QGraphicsLinearLayout *linear = new QGraphicsLinearLayout(Qt::Horizontal);
    linear->setContentsMargins(0, 0, 0, 0);
    linear->setSpacing(0);
    linear->addItem(createImageLabel(objectName, detail.text()));

    SeasideLabel *button = new SeasideLabel("CALL");  // TODO: i18n
    if (detail.location() == Seaside::LocationMobile)
        button->setObjectName("SeasideCallButton");
    else
        button->setObjectName("SeasideCallButtonWide");
    button->setMouseEnabled(true);
    m_labelToDetail[button] = detail;
    connect(button, SIGNAL(clicked(SeasideLabel*)),
            this, SLOT(callNumber(SeasideLabel*)));
    linear->addItem(button);

    if (detail.location() == Seaside::LocationMobile) {
        button = new SeasideLabel("SMS");  // TODO: i18n
        button->setObjectName("SeasideSmsButton");
        button->setMouseEnabled(true);
        m_labelToDetail[button] = detail;
        connect(button, SIGNAL(clicked(SeasideLabel*)),
                this, SLOT(composeSMS(SeasideLabel*)));
        linear->addItem(button);
    }

    m_layouts[SectionPhone]->addItem(linear);
}

void PersonDetailView::addEmail(const SeasideDetail& detail)
{
    if (!m_headers[SectionEmail])
        addSection(SectionEmail, "Email");  // TODO: i18n

    QGraphicsLinearLayout *linear = new QGraphicsLinearLayout(Qt::Horizontal);
    linear->setContentsMargins(0, 0, 0, 0);
    linear->setSpacing(0);
    linear->addItem(createImageLabel("EmailAddress", detail.text()));

    SeasideLabel *button = new SeasideLabel("EMAIL");  // TODO: i18n
    button->setObjectName("SeasideEmailButton");
    button->setMouseEnabled(true);
    m_labelToDetail[button] = detail;
    connect(button, SIGNAL(clicked(SeasideLabel*)),
            this, SLOT(composeEmail(SeasideLabel*)));
    linear->addItem(button);

    m_layouts[SectionEmail]->addItem(linear);
}

void PersonDetailView::addAddress(const SeasideDetail& detail)
{
    if (!m_headers[SectionAddress])
        addSection(SectionAddress, "Addresses");  // TODO: i18n

    QString objectName;
    switch (detail.location()) {
    case Seaside::LocationWork:
        objectName = "WorkAddress";
        break;
    default:
        objectName = "HomeAddress";
        break;
    }

    QStringList fields = detail.text().split(QChar('\n'));
    while (fields.size() < 5)
        fields << "";
    QString street  = fields[0];
    QString city    = fields[1];
    QString state   = fields[2];
    QString zip     = fields[3];
    QString country = fields[4];

    // city and state with separator (%1 = city, %2 = state/region)
    QString format = "%1, %2";  // TODO: i18n
    QString place;
    if (!city.isEmpty() && !state.isEmpty())
        place = format.arg(city, state);
    else if (!city.isEmpty())
        place = city;
    else
        place = state;

    // city/state and postcode with separator (%1 = city/state, %2 = postcode)
    QString line;
    format = "%1&nbsp;&nbsp;%2";  // TODO: i18n
    if (!place.isEmpty() && !zip.isEmpty())
        line = format.arg(place, zip);
    else if (!place.isEmpty())
        line = place;
    else
        line = zip;

    QString address(street);
    if (!address.isEmpty() && !line.isEmpty())
        address += "<br>";
    address += line;
    if (!address.isEmpty() && !country.isEmpty())
        address += "<br>";
    address += country;

    // set minimum width so initial line wrapping calculations don't yield a
    //   large height
    ImageLabel *label = createImageLabel(objectName, address);
    label->setMinimumWidth(300);
    m_layouts[SectionAddress]->addItem(label);
}

void PersonDetailView::toggleSection(SeasideLabel *label)
{
    int i;

    for (i=0; i<SectionEnd; ++i)
        if (label == m_headers[i])
            break;
    if (i >= SectionEnd)
        return;

    m_visible[i] = !m_visible[i];

    // FIXME: there are some perverse cases here:
    //   1) when there are lots of items in early fields but no address field,
    //      the special case below will not help
    //   2) when a middle section is long enough that it extends below the address
    //      section, animations can look ugly, but also the buffer to avoid physics
    //      issues in scrolling doesn't work

    if (m_visible[i]) {
        if (i == SectionEnd - 1)
            delete m_mainPolicy->itemAt(2 * SectionEnd + 1, 0);
        m_mainPolicy->addItem(m_widgets[i], 2 * i + 2, 0);
        emit viewRequest(m_headers[i]->pos().y(),
                         m_headers[i]->size().height() + m_widgets[i]->size().height());
    }
    else {
        m_mainPolicy->removeItem(m_widgets[i]);
        if (i == SectionEnd - 1) {
            MWidget *widget = new MWidget;
            widget->setPreferredHeight(m_widgets[i]->size().height());
            m_mainPolicy->addItem(widget, 2 * SectionEnd + 1, 0);
        }
    }
}

void PersonDetailView::callNumber(SeasideLabel *label)
{
    const SeasideDetail& detail = m_labelToDetail[label];
    if (detail.isValid())
        emit callNumber(detail.text());
}

void PersonDetailView::composeSMS(SeasideLabel *label)
{
    const SeasideDetail& detail = m_labelToDetail[label];
    if (detail.isValid())
        emit composeSMS(detail.text());
}

void PersonDetailView::composeIM(SeasideLabel *label)
{
    const SeasideDetail& detail = m_labelToDetail[label];
    if (detail.isValid())
        emit composeIM(detail.text());
}

void PersonDetailView::composeEmail(SeasideLabel *label)
{
    const SeasideDetail& detail = m_labelToDetail[label];
    if (detail.isValid())
        emit composeEmail(detail.text());
}

M_REGISTER_VIEW(PersonDetailView, MWidgetController)
