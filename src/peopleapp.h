/*
 * meego-handset-people - Contacts application
 * Copyright © 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef PEOPLEAPP_H
#define PEOPLEAPP_H

#include <QPersistentModelIndex>

#include <MApplication>
#include <QUuid>

class QGraphicsWidget;
class MApplicationPage;
class MWidgetController;
class MTextEdit;
class SeasideSyncModel;
class SeasidePersonModel;
class SeasideWindow;
class SeasidePeople;
class SeasidePerson;
class SeasideSlider;
class PersonCommsPage;
class MAction;
class QActionGroup;
class QDBusServiceWatcher;


#include <QContactManager>
using namespace QtMobility;

class PeopleApplication: public MApplication
{
    Q_OBJECT

public:
    PeopleApplication(int& argc, char **argv);
    virtual ~PeopleApplication();

    static PeopleApplication *instance();

    enum CommCat {
        CatCall,
        CatSMS,
        CatEmail,
        CatIM
    };

public slots:
    void createCommPage(SeasidePersonModel *pm, CommCat type);
    void showDetailsPage(const QContactLocalId& id);
    void setThumbnailPath(const QUuid &id, const QString &path);

protected:
    void createPeoplePage();
    void initSlider();
    void initSearch();

protected slots:
    void createDetailPage(const QModelIndex& index);
    void searchClicked();
    void searchChanged();
    void searchClear();
    void searchCancel();
    void searchCommit();
    void addNewContact();
    void detailBack();
    void commBack();
    void editCurrent();
    void createEditPage(const QModelIndex& index = QModelIndex(),
                        const QString& title = QString());
    void editModified();
    void editSave();
    void editBack();
    void menuFilterSelected(QAction* filter);
    void callNumber(const QString& number);
    void composeSMS(const QString& number);
    void composeEmail(const QString& address);
    void composeIM(const QString& accountString);
    void sliderActivated(int index, const QString& stop);
    void repositionOverlays();
    void scrollTo(qreal pos);
    void scrollIntoView(qreal ypos, qreal height);

    void callNumberToActiveService(const QString& number);
    void smsNumberToActiveService(const QString& number);
    void imToActiveService(const QString& account);
    void emailToActiveService(const QString& email);
    void handleDialerRegistered();
    void handleSMSRegistered();
    void handleIMRegistered();
    void handleEmailRegistered();

private:
    static PeopleApplication *theApp;

    SeasideSyncModel *m_realModel;

    SeasideWindow *m_window;
    MApplicationPage *m_mainPage;
    MApplicationPage *m_detailPage;
    MApplicationPage *m_editPage;
    PersonCommsPage *m_commPage;

    QGraphicsWidget *m_topSpacer;
    QGraphicsWidget *m_bottomSpacer;

    SeasidePeople *m_people;
    QPersistentModelIndex m_currentIndex;

    SeasideSlider *m_sliderH;
    SeasideSlider *m_sliderV;
    MWidgetController *m_searchWidget;
    MTextEdit *m_searchEdit;

    SeasidePerson *m_currentPerson;
    SeasidePersonModel *m_editModel;
    bool m_editModelModified;

    MAction *m_actionAll;
    MAction *m_actionRecent;
    MAction *m_actionFav;
    MAction *m_actionSearch;
    MAction *m_actionAdd;

    QActionGroup *m_actionFilters;
    QDBusServiceWatcher *m_dbusDialerWatcher;
    QDBusServiceWatcher *m_dbusEmailWatcher;
    QDBusServiceWatcher *m_dbusIMWatcher;
    QDBusServiceWatcher *m_dbusSMSWatcher;

    bool m_callPending;
    QString m_number;

    bool m_smsPending;
    QString m_smsNumber;

    bool m_imPending;
    QString m_imAccount;

    bool m_emailPending;
    QString m_email;

     //AccountManager * m_AccountManager;
    //QString m_AccountPath;
};

#endif // PEOPLEAPP_H
