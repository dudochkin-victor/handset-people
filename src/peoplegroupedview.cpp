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
#include <QAbstractItemModel>
#include <QTimer>

#include <seasidepersonmodel.h>
#include <seasidelabel.h>

#include <seasidesyncmodel.h>
#include "peoplegroupedview.h"
#include "people.h"
#include "personcard.h"

PeopleGroupedView::PeopleGroupedView(SeasidePeople *controller): MWidgetView(controller)
{
    m_controller = controller;
    m_mainLayout = NULL;
    m_itemModel = NULL;
    m_numSections = 0;

    connect(m_controller, SIGNAL(scrollRequest(QString)),
            this, SLOT(scrollTo(QString)));
}

PeopleGroupedView::~PeopleGroupedView()
{
}

static void clearLayout(QGraphicsGridLayout *grid)
{
    int rows = grid->rowCount();
    for (int i=0; i<rows; i++) {
        QGraphicsLayout *child = static_cast<QGraphicsLayout *>(grid->itemAt(i, 0));
        if (child) {
            int childCount = child->count();
            for (int j=0; j<childCount; j++) {
                QGraphicsLayoutItem *item = child->itemAt(0);
                child->removeAt(0);
                delete item;
            }
            delete child;
        }
    }
}

static SeasideLabel *createHeader(const QString& text)
{
    SeasideLabel *label = new SeasideLabel(text);
    label->setObjectName("AlphabetHeader");
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    return label;
}

static int findSection(const QString& name, const QStringList& list) {
    // returns a number between 0 and the number of items in the list, indicating
    //   at which point in that list
    int section = 0;
    foreach (const QString& heading, list) {
        if (heading.localeAwareCompare(name) > 0)
            break;
        section++;
    }
    return section;
}

void PeopleGroupedView::insertCard(const QModelIndex& index) {
    // requires: model is sorted in alphabetical order
    QGraphicsLinearLayout *linear;

    SEASIDE_SHORTCUTS
    SEASIDE_SET_MODEL_AND_ROW(m_itemModel, index.row());
    QString name = SEASIDE_FIELD(FirstName, String);
    bool isSelf = SEASIDE_FIELD(isSelf, Bool);

    int section;

    //if MeCard section 0, else shift other sections down one
    if(isSelf)
        section = 0;
    else
        section = findSection(name, m_headings)+1;

    // if this is the first item in the section, add the heading
    if (m_sectionCounts[section] == 0) {
        // items from before the first heading go in an "Other" section at the beginning
        QString heading;
        if (section > 1)
            heading = m_displayHeadings[section-2];
        else if(section == 1)
            heading = "Other";  // TODO: i18n
        else
            heading = "Me";
        linear = new QGraphicsLinearLayout(Qt::Vertical);
        linear->setContentsMargins(0, 0, 0, 0);
        linear->setSpacing(0);

        m_mainLayout->addItem(linear, section, 0);
        linear->addItem(createHeader(heading));
    }

    m_sectionCounts[section]++;

    qDebug() << "section " << section << " sectionCount " << m_sectionCounts[section];

    linear = static_cast<QGraphicsLinearLayout *>(m_mainLayout->itemAt(section, 0));
    SeasidePersonCard *card = new SeasidePersonCard(index);

    int i;
    int count = linear->count();
    for (i=1; i<count; i++) {
        SeasidePersonCard *existing = static_cast<SeasidePersonCard *>(linear->itemAt(i));
        if (existing->name().localeAwareCompare(name) > 0)
            break;
    }
     qDebug() << "item at " << i;
    linear->insertItem(i, card);

    QObject::connect(card, SIGNAL(requestDetails(QModelIndex)),
                     m_controller, SIGNAL(itemClicked(QModelIndex)),
                     Qt::UniqueConnection);

    /*QObject::connect(card, SIGNAL(requestEdit(QModelIndex)),
                     m_controller, SIGNAL(editRequest(QModelIndex)),
                     Qt::UniqueConnection);*/

    QObject::connect(card, SIGNAL(requestSetFavorite(const QUuid&,bool)),
                     m_controller, SLOT(setFavorite(const QUuid&,bool)),
                     Qt::UniqueConnection);

    QObject::connect(card, SIGNAL(requestDelete(const QUuid&)),
                     m_controller, SLOT(deletePerson(const QUuid&)),
                     Qt::UniqueConnection);

    QObject::connect(card, SIGNAL(callNumber(const QString&)),
                     m_controller, SIGNAL(callNumber(const QString&)),
                     Qt::UniqueConnection);

    QObject::connect(card, SIGNAL(composeSMS(const QString&)),
                     m_controller, SIGNAL(composeSMS(const QString&)),
                     Qt::UniqueConnection);

    QObject::connect(card, SIGNAL(composeIM(const QString&)),
                     m_controller, SIGNAL(composeIM(const QString&)),
                     Qt::UniqueConnection);

    QObject::connect(card, SIGNAL(composeEmail(const QString&)),
                     m_controller, SIGNAL(composeEmail(const QString&)),
                     Qt::UniqueConnection);
}

static void removeAndDelete(QGraphicsLayout *layout, int index)
{
    qDebug() << "removeAndDelete" << index;
    QGraphicsLayoutItem *item = layout->itemAt(index);
    layout->removeAt(index);
    delete item;
}

void PeopleGroupedView::removeCard(int row) {
    qDebug() << "removeCard" << row;
    // NOTE: this is relying on the model sort order matching our order

    // count the items that appear before this one
    int items = 0;
    for (int i=0; i<=m_numSections; i++) {
        if (row - items < m_sectionCounts[i]) {
            QGraphicsLayout *layout = static_cast<QGraphicsLayout *>(m_mainLayout->itemAt(i, 0));
            qDebug() << "layout for " << i;
            removeAndDelete(layout, row - items +1);
            layout->updateGeometry();

            m_sectionCounts[i]--;

            // if empty, remove the heading as well
            if (m_sectionCounts[i] == 0) {
                removeAndDelete(layout, 0);
                delete layout;
            }
            return;
        }
        items += m_sectionCounts[i];
    }
}

void PeopleGroupedView::setupModel()
{
    MWidgetView::setupModel();

    if (m_mainLayout){
        qDebug() << "MODEL CLEARED";
        clearLayout(m_mainLayout);
    }
    m_itemModel = m_controller->itemModel();

    m_mainLayout = new QGraphicsGridLayout(m_controller);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    connect(m_itemModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(rowsInserted(QModelIndex,int,int)), Qt::UniqueConnection);
    connect(m_itemModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(rowsRemoved(QModelIndex,int,int)), Qt::UniqueConnection);
    connect(m_itemModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(dataChanged(QModelIndex,QModelIndex)), Qt::UniqueConnection);
    connect(m_itemModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(rowsMoved(QModelIndex,int,int,QModelIndex,int)), Qt::UniqueConnection);
    connect(m_itemModel, SIGNAL(layoutChanged()),
            this, SLOT(layoutChanged()), Qt::UniqueConnection);
    connect(m_itemModel, SIGNAL(modelReset()),
            this, SLOT(modelReset()), Qt::UniqueConnection);

    QString alphabet(QObject::tr("a b c d e f g h i j k l m n o p q r s t u v w x y z", "Alphabet for contact index"));
    m_headings = alphabet.split(" ", QString::SkipEmptyParts);

    QString capitals(QObject::tr("A B C D E F G H I J K L M N O P Q R S T U V W X Y Z","Alphabet for contact index"));
    m_displayHeadings = capitals.split(" ", QString::SkipEmptyParts);

    m_numSections = m_headings.count();
    qDebug() << "m_numSections : " << m_numSections;

    // initialize vector elements to zero
    m_sectionCounts.clear();
    m_sectionCounts.resize(m_numSections + 2);

    int rows = m_itemModel->rowCount();
    for (int i=0; i<rows; ++i){
        qDebug() << "PeopleGroupedView::setupModel() insertCard at" << i;
        insertCard(m_itemModel->index(i, 0));
    }
}

void PeopleGroupedView::orientationChangeEvent(MOrientationChangeEvent *event)
{
    // workaround because layout doesn't update on rotate in libmeegotouch 0.20
    Q_UNUSED(event);
    QTimer::singleShot(0, this, SLOT(adjustGeometry()));
}

void PeopleGroupedView::adjustGeometry()
{
    m_mainLayout->updateGeometry();
}

void PeopleGroupedView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent);
    qDebug() << "[PeopleGroupedView] Rows Inserted" << start << end;
    for (int i=start; i<=end; i++){
        insertCard(m_itemModel->index(i, 0, parent));      
    }
}

void PeopleGroupedView::rowsRemoved(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent);
    qDebug() << "[PeopleGroupedView] Rows Removed" << start << end;
    for (int i=start; i<=end; i++)
        removeCard(i);
}

void PeopleGroupedView::dataChanged(const QModelIndex& topLeft,
                                    const QModelIndex& bottomRight)
{
    qDebug() << "[PeopleGroupedView] Data Changed" << topLeft.row() << bottomRight.row();

    int start = topLeft.row();
    int end = bottomRight.row();

    for (int i=start; i<=end; i++) {
        removeCard(i);
        insertCard(m_itemModel->index(i, 0));
    }
}

void PeopleGroupedView::rowsMoved(const QModelIndex& sourceParent,
                                  int sourceStart, int sourceEnd,
                                  const QModelIndex& destParent, int destRow)
{
    Q_UNUSED(sourceParent);
    Q_UNUSED(sourceStart);
    Q_UNUSED(sourceEnd);
    Q_UNUSED(destParent);
    Q_UNUSED(destRow);
    qDebug() << "[PeopleGroupedView] Rows Moved" << sourceStart << " to " << sourceEnd;
}

void PeopleGroupedView::layoutChanged()
{
    qDebug() << "[PeopleGroupedView] Layout Changed";
    setupModel();
}

void PeopleGroupedView::modelReset()
{
    qDebug() << "[PeopleGroupedView] Model Reset";
    setupModel();
}

void PeopleGroupedView::scrollTo(const QString& name)
{
    Q_UNUSED(name);
    int i;
    for (i=0; i<m_numSections; i++) {
        if (!name.compare(m_headings[i], Qt::CaseInsensitive))
            break;
    }

    int section=0;
    //if Other section exists shift increase i by 1
    if(m_sectionCounts[1] > 0)
     section = (i == m_numSections) ? 1 : i + 2;
    else //account for mecard +1
     section = (i == m_numSections) ? 0 : i + 1;
    m_mainLayout->rowCount();

    // prevent looking in non-existent rows in the grid layout
    int numSections = m_numSections;
    if (m_mainLayout->rowCount() < m_numSections)
        numSections = m_mainLayout->rowCount() - 1;

    QGraphicsLayoutItem *item = NULL;
    int diff = 1;
    while (section <= numSections || (section - diff) >= 0) {
        // first check this section for an item
        if (section <= numSections) {
            item = m_mainLayout->itemAt(section, 0);
            if (item)
                break;
        }

        int alternate = section - diff;
        if (alternate >= 0 && alternate <= numSections) {
            item = m_mainLayout->itemAt(alternate, 0);
            if (item)
                break;
        }

        section++;
        diff += 2;
    }

    if (item)
        m_controller->scrollTo(item->geometry().top());
}

M_REGISTER_VIEW(PeopleGroupedView, SeasidePeople)
