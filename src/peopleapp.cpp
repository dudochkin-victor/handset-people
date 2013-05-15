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
#include <QtDBus/QtDBus>
#include <QDBusConnection>
#include <MAction>
#include <QActionGroup>

#include <MApplicationPage>
#include <MTextEdit>
#include <MButton>
#include <MPannableViewport>
#include <MSceneManager>

#include <seasidesyncmodel.h>
#include <seasidepersonmodel.h>

#include "peopleapp.h"
#include "window.h"
#include "people.h"
#include "person.h"
#include "slider.h"
#include "personcommspage.h"
#include "peopledbusadaptor.h"

PeopleApplication *PeopleApplication::theApp = NULL;

PeopleApplication *PeopleApplication::instance()
{
    return theApp;
}

PeopleApplication::PeopleApplication(int &argc, char**argv):
        MApplication(argc, argv)
{
    theApp = this;

    m_realModel = SeasideSyncModel::instance();

    m_window = new SeasideWindow;
    m_window->show();

    m_mainPage = NULL;
    m_detailPage = NULL;
    m_editPage = NULL;
    m_commPage = NULL;

    m_topSpacer = NULL;
    m_bottomSpacer = NULL;

    m_people = NULL;
    m_sliderH = NULL;
    m_sliderV = NULL;
    m_searchWidget = NULL;
    m_searchEdit = NULL;

    m_currentPerson = NULL;
    m_editModel = NULL;
    m_editModelModified = NULL;


//    m_AccountPath = NULL;

    createPeoplePage();

    connect(m_window, SIGNAL(orientationChanged(M::Orientation)),
            this, SLOT(repositionOverlays()));

     PeopleDBusAdaptor *adpt = new PeopleDBusAdaptor(this);
     if(!adpt)
        qWarning() << "People Application Dbus adaptor failed" << adpt;

     bool err;
     err = QDBusConnection::sessionBus().registerService(DBUSINTFNAME);
     if(!err)
          qDebug() << "People Application Dbus calls error:" << err;
       err = QDBusConnection::sessionBus().registerObject(DBUSOBJPATH, this);
       if(!err)
            qDebug() << "People Application Dbus calls error:" << err;

       m_dbusDialerWatcher = new QDBusServiceWatcher("com.meego.dialer" , QDBusConnection::sessionBus(),
                                           QDBusServiceWatcher::WatchForRegistration|QDBusServiceWatcher::WatchForUnregistration,
                                           this);
       m_dbusEmailWatcher = new QDBusServiceWatcher("com.meego.email" , QDBusConnection::sessionBus(),
                                           QDBusServiceWatcher::WatchForRegistration|QDBusServiceWatcher::WatchForUnregistration,
                                           this);
       m_dbusIMWatcher = new QDBusServiceWatcher("com.meego.meego_handset_chat" , QDBusConnection::sessionBus(),
                                           QDBusServiceWatcher::WatchForRegistration|QDBusServiceWatcher::WatchForUnregistration,
                                           this);
       m_dbusSMSWatcher = new QDBusServiceWatcher("com.meego.sms" , QDBusConnection::sessionBus(),
                                           QDBusServiceWatcher::WatchForRegistration|QDBusServiceWatcher::WatchForUnregistration,
                                           this);

    connect(m_dbusDialerWatcher, SIGNAL(serviceRegistered(QString)),
            this, SLOT(handleDialerRegistered())); 
    connect(m_dbusEmailWatcher, SIGNAL(serviceRegistered(QString)),
            this, SLOT(handleEmailRegistered()));
    connect(m_dbusIMWatcher, SIGNAL(serviceRegistered(QString)),
            this, SLOT(handleIMRegistered()));
    connect(m_dbusSMSWatcher, SIGNAL(serviceRegistered(QString)),
            this, SLOT(handleSMSRegistered()));
}

PeopleApplication::~PeopleApplication()
{
    delete m_mainPage;
    delete m_window;
    m_realModel->releaseInstance();

    if (theApp == this)
        theApp = NULL;
}

void PeopleApplication::createPeoplePage()
{

    //m_AccountManager = AccountManger::getInstance();
    m_mainPage = new MApplicationPage;
    m_mainPage->setTitle(QObject::tr("People","Title of the application"));

    QGraphicsLinearLayout *linear = new QGraphicsLinearLayout(Qt::Vertical);
    linear->setContentsMargins(0, 0, 0, 0);
    linear->setSpacing(0);
    m_mainPage->centralWidget()->setLayout(linear);

    m_topSpacer = new QGraphicsWidget;
    m_topSpacer->setPreferredHeight(0);
    linear->addItem(m_topSpacer);

    m_people = new SeasidePeople;
    linear->addItem(m_people);

    m_bottomSpacer = new QGraphicsWidget;
    m_bottomSpacer->setPreferredHeight(0);
    linear->addItem(m_bottomSpacer); 

    m_actionSearch = new MAction(QObject::tr("Search","Menu action to bring up search for contact list"), this);
    m_actionSearch->setLocation(MAction::ApplicationMenuLocation);
    m_mainPage->addAction(m_actionSearch);
    connect(m_actionSearch, SIGNAL(triggered()), this, SLOT(searchClicked()));

    m_actionAdd = new MAction(QObject::tr("Add Contact","Menu action to add contact"), this);
    m_actionAdd->setLocation(MAction::ApplicationMenuLocation);
    m_mainPage->addAction(m_actionAdd);
    connect(m_actionAdd, SIGNAL(triggered()), this, SLOT(addNewContact()));

    m_actionAll = new MAction(QObject::tr("<b>Show All</b>","Menu filter to show all contacts"), this);  
    m_actionAll->setLocation(MAction::ApplicationMenuLocation);
    m_actionAll->setObjectName("ShowAllFilter");
    m_actionAll->setCheckable(true);
    connect(m_actionAll, SIGNAL(triggered()), m_people, SLOT(filterAll()));

    m_actionFav = new MAction(QObject::tr("Show Favorites", "Menu filter to show all contacts marked as favorites"), this);
    m_actionFav->setLocation(MAction::ApplicationMenuLocation);
    m_actionFav->setObjectName("ShowFavoritesFilter");
    m_actionFav->setCheckable(true);
    connect(m_actionFav, SIGNAL(triggered()), m_people, SLOT(filterFavorites()));

    m_actionRecent = new MAction(QObject::tr("Show Recent","Menu filter to show all contacts with recent communications"), this);
    m_actionRecent->setLocation(MAction::ApplicationMenuLocation);
    m_actionRecent->setCheckable(true);
    m_actionRecent->setObjectName("ShowRecentFilter");
    connect(m_actionRecent, SIGNAL(triggered()), m_people, SLOT(filterRecent()));

    m_actionFilters = new QActionGroup(this);
    m_actionFilters->addAction(m_actionAll);
    m_actionFilters->addAction(m_actionRecent);
    m_actionFilters->addAction(m_actionFav);
    m_actionFilters->setExclusive(true);
    m_mainPage->addActions(m_actionFilters->actions());
    connect(m_actionFilters, SIGNAL(triggered(QAction*)), this, SLOT(menuFilterSelected(QAction*)));

    connect(m_people, SIGNAL(itemClicked(QModelIndex)),
            this, SLOT(createDetailPage(QModelIndex)));
    connect(m_people, SIGNAL(editRequest(QModelIndex)),
            this, SLOT(createEditPage(QModelIndex)));

    connect(m_people, SIGNAL(callNumber(const QString&)),
            this, SLOT(callNumber(const QString&)));
    connect(m_people, SIGNAL(composeSMS(const QString&)),
            this, SLOT(composeSMS(const QString&)));
    connect(m_people, SIGNAL(composeIM(const QString&)),
            this, SLOT(composeIM(const QString&)));
    connect(m_people, SIGNAL(composeEmail(const QString&)),
            this, SLOT(composeEmail(const QString&)));

    m_mainPage->appear();

    initSlider();
    initSearch();
    repositionOverlays();
}

void PeopleApplication::menuFilterSelected(QAction* filter){ 
  filter->setChecked(true);
  /*Hack for "font().setBold" not working BMC#??? */
  (m_actionAll->isChecked())    ? m_actionAll->setText(QObject::tr("<b>Show All</b>","Menu filter to show all contacts"))  : m_actionAll->setText(QObject::tr("Show All","Menu filter to show all contacts"));
  (m_actionRecent->isChecked()) ? m_actionRecent->setText(QObject::tr("<b>Show Recent</b>","Menu filter to show all contacts with recent communications")): m_actionRecent->setText(QObject::tr("Show Recent","Menu filter to show all contacts with recent communications"));
  (m_actionFav->isChecked())    ? m_actionFav->setText(QObject::tr("<b>Show Favorites</b>", "Menu filter to show all contacts marked as favorites"))    : m_actionFav->setText(QObject::tr("Show Favorites", "Menu filter to show all contacts marked as favorites"));
}

void PeopleApplication::createDetailPage(const QModelIndex &index)
{
    if (m_detailPage)
        return;

    if (!index.isValid())
        return;

    searchCancel();

    m_currentIndex = index;

    m_currentPerson = new SeasidePerson(index);
    m_currentPerson->setViewType("personDetail");

    connect(m_currentPerson, SIGNAL(callNumber(const QString&)),
            this, SLOT(callNumber(const QString&)));
    connect(m_currentPerson, SIGNAL(composeIM(const QString&)),
            this, SLOT(composeIM(const QString&)));
    connect(m_currentPerson, SIGNAL(composeSMS(const QString&)),
            this, SLOT(composeSMS(const QString&)));
    connect(m_currentPerson, SIGNAL(composeEmail(const QString&)),
            this, SLOT(composeEmail(const QString&)));
    connect(m_currentPerson, SIGNAL(viewRequest(qreal,qreal)),
            this, SLOT(scrollIntoView(qreal,qreal)));

    m_detailPage = new MApplicationPage;
    m_detailPage->setTitle(QObject::tr("Contact Detail","Title for detail view"));
    m_detailPage->setCentralWidget(m_currentPerson);

    MAction *action = new MAction(QObject::tr("<b>Edit</b>","Edit toolbar button to edit contact details"), this);
    action->setLocation(MAction::ApplicationMenuLocation);
    m_detailPage->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(editCurrent()));

    connect(m_detailPage, SIGNAL(backButtonClicked()), this, SLOT(detailBack()));

    m_detailPage->appear(MApplicationPage::DestroyWhenDismissed);
}

void PeopleApplication::createCommPage(SeasidePersonModel *pm, CommCat type)
{
    if (m_commPage)
        return;

    searchCancel();

    m_commPage = new PersonCommsPage(pm, type);

    connect(m_commPage, SIGNAL(backButtonClicked()),
            this, SLOT(commBack()));

    switch (type) {
    case CatCall:
        connect(m_commPage, SIGNAL(destSelected(QString)),
                this, SLOT(callNumber(QString)));
        break;

    case CatSMS:
        connect(m_commPage, SIGNAL(destSelected(QString)),
                this, SLOT(composeSMS(QString)));
        break;

    case CatIM:
      connect(m_commPage, SIGNAL(destSelectedIM(QString)),
              this, SLOT(composeIM(QString)));
      break;

    case CatEmail:
        connect(m_commPage, SIGNAL(destSelected(QString)),
                this, SLOT(composeEmail(QString)));
        break;
    }
    connect(m_commPage, SIGNAL(destSelected(QString)),
            this, SLOT(commBack()));

    m_commPage->appear(MApplicationPage::DestroyWhenDismissed);
}

void PeopleApplication::initSlider()
{
    QString alphabet = QObject::tr("A B C D E F G H I J K L M N O P Q R S T U V W X Y Z","Capital letter alphabet");

    m_sliderH = new SeasideSlider(M::Landscape);
    m_sliderH->setObjectName("SeasideSliderLandscape");
    m_sliderH->setStops(alphabet.split(QChar(' ')));
    m_sliderH->setParentItem(m_mainPage);
    m_sliderH->setZValue(1);
    m_sliderH->setPreferredWidth(m_window->width());

    m_sliderV = new SeasideSlider(M::Portrait);
    m_sliderV->setObjectName("SeasideSliderPortrait");
    m_sliderV->setStops(alphabet.split(QChar(' ')));
    m_sliderV->setParentItem(m_mainPage);
    m_sliderV->setZValue(1);

    connect(m_sliderH, SIGNAL(stopActivated(int,QString)),
            this, SLOT(sliderActivated(int,QString)));
    connect(m_sliderV, SIGNAL(stopActivated(int,QString)),
            this, SLOT(sliderActivated(int,QString)));
    connect(m_people, SIGNAL(scrollRequest(qreal)),
            this, SLOT(scrollTo(qreal)));
    connect(m_mainPage, SIGNAL(exposedContentRectChanged()),
            this, SLOT(repositionOverlays()));
}

void PeopleApplication::initSearch()
{
    m_searchWidget = new MWidgetController;
    m_searchWidget->setViewType("background");
    m_searchWidget->setObjectName("PeopleSearch");
    m_searchWidget->setParentItem(m_mainPage);
    m_searchWidget->setZValue(1);
    m_searchWidget->hide();

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Horizontal, m_searchWidget);

    m_searchEdit = new MTextEdit;
    m_searchEdit->setObjectName("PeopleSearchEdit");
    m_searchEdit->setPrompt(QObject::tr("Tap to start searching people","Prompt for search bar text edit field"));
    layout->addItem(m_searchEdit);
    layout->setAlignment(m_searchEdit, Qt::AlignVCenter);
    connect(m_searchEdit, SIGNAL(returnPressed()), this, SLOT(searchCommit()));

    // uncomment this line to enable dynamic search
    //connect(m_searchEdit, SIGNAL(textChanged()), this, SLOT(searchChanged()));

    MButton *button = new MButton();
    button->setIconID("icon-m-framework-close-alt");
    button->setObjectName("PeopleSearchButton");
    layout->addItem(button);
    layout->setAlignment(button, Qt::AlignVCenter);
    connect(button, SIGNAL(clicked()), this, SLOT(searchClear()));

    button = new MButton(QObject::tr("<b>Search</b>","Toolbar button to launch search widget"));
    button->setObjectName("PeopleSearchButton");
    layout->addItem(button);
    layout->setAlignment(button, Qt::AlignVCenter);
    connect(button, SIGNAL(clicked()), this, SLOT(searchCommit()));
}

void PeopleApplication::searchClicked()
{
    m_searchWidget->show();
    //force keyboard to pop-up
    m_searchEdit->setFocus(Qt::TabFocusReason);
    emit QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    m_bottomSpacer->setPreferredHeight(m_searchWidget->size().height());
}

void PeopleApplication::searchClear()
{
    if (m_searchEdit) {
        m_searchEdit->clear();
        m_searchEdit->setFocus();
    }
}

void PeopleApplication::searchChanged()
{
    if (m_searchEdit)
        m_people->filterSearch(m_searchEdit->text());
}

void PeopleApplication::searchCancel()
{
    if (m_searchEdit)
        m_searchEdit->clearFocus();

    if (m_searchWidget) {
        m_searchWidget->hide();
        m_bottomSpacer->setPreferredHeight(0);
    }
}

void PeopleApplication::searchCommit()
{
    searchChanged();
    searchCancel();
}

void PeopleApplication::addNewContact()
{
    // there should be no current index or person set, but just in case...
    m_currentIndex = QModelIndex();
    m_currentPerson = NULL;
    createEditPage(m_currentIndex, QObject::tr("Add Contact","Page title for Edit Screen"));
}

void PeopleApplication::detailBack()
{
    m_currentIndex = QModelIndex();
    m_currentPerson = NULL;
    if (m_detailPage) {
        m_detailPage->dismiss();
        m_detailPage = NULL;
    }
}

void PeopleApplication::commBack()
{
    if (m_commPage) {
        m_commPage->dismiss();
        m_commPage = NULL;
    }
}


void PeopleApplication::editCurrent()
{
    createEditPage(m_currentIndex);
}

void PeopleApplication::createEditPage(const QModelIndex &index, const QString& title)
{
    if (m_editPage)
        return;

    QModelIndex useIndex;

    if (index.isValid())
        useIndex = index;
    else
        useIndex = m_currentIndex;

    if (useIndex.isValid())
        m_editModel = SeasideSyncModel::createPersonModel(useIndex);
    else  // create empty edit model to add new contact
        m_editModel = new SeasidePersonModel;
    MWidgetController *person = new MWidgetController(m_editModel);
    person->setViewType("personEdit");

    m_editModelModified = false;
    connect(m_editModel, SIGNAL(modified(QList<const char*>)),
            this, SLOT(editModified()));

    m_editPage = new MApplicationPage;
    if (title.isNull())
        m_editPage->setTitle(QObject::tr("Edit Contact","Title for edit screen"));
    else
        m_editPage->setTitle(title);
    m_editPage->setCentralWidget(person);

    MAction *action = new MAction(QObject::tr("<b>Save</b>","Save toolbar button for edit screen"), this);
    action->setLocation(MAction::ApplicationMenuLocation);
    m_editPage->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(editSave()));

    connect(m_editPage, SIGNAL(backButtonClicked()), this, SLOT(editBack()));

    m_editPage->appear(MApplicationPage::DestroyWhenDismissed);
}

void PeopleApplication::editModified()
{
    m_editModelModified = true;
}

void PeopleApplication::editSave()
{
    if (m_editModel && m_editModelModified && !m_editModel->isEmpty()) {
        m_realModel->updatePerson(m_editModel);
        if (m_currentPerson)
            m_currentPerson->setModel(m_realModel->createPersonModel(m_editModel->uuid()));
        m_editPage->dismiss();
        m_editPage = NULL;
    }
}

void PeopleApplication::editBack()
{
    if (m_editPage) {
        m_editPage->dismiss();
        m_editPage = NULL;
    }
}

void PeopleApplication::showDetailsPage(const QContactLocalId& id){
  qDebug() << "PeopleApplication::showDetailsPage id" << id;
    QModelIndex index = m_realModel->getModelIndex(id);   
  qDebug() << "PeopleApplication::showDetailsPage index" << index;
    createDetailPage(index);
}

void PeopleApplication::setThumbnailPath(const QUuid& id, const QString& path){
    qDebug() << "PeopleApplication::setThumbnailPath id" << id << " path " << path;
    m_realModel->setAvatar(id, path);
}

void PeopleApplication::callNumber(const QString& number)
{
    qDebug() << "PeopleApplication::callNumber attempting call to:" << number;

    if(QDBusConnection::sessionBus().interface()->isServiceRegistered("com.meego.dialer")){
      qDebug() << "PeopleApplication::callNumber dialer is registered";
	callNumberToActiveService(number);	    
	return;
    }else{
      qDebug() << "PeopleApplication::callNumber dialer is NOT registered";
      QDBusReply<void> reply = QDBusConnection::sessionBus().interface()->startService("com.meego.dialer");
      if (!reply.isValid()){
	qWarning() << "Starting Dialer failed:" << reply.error().message();  
      }else{
      qDebug() << "PeopleApplication::callNumber saving number to call on registration";
	m_callPending = true;
	m_number = number;
      }           
    }
}

void PeopleApplication::handleDialerRegistered(){
      qDebug() << "PeopleApplication::handleDialerRegistered dialer is registered";
  if(m_callPending){
    qDebug() << "PeopleApplication::handleDialerRegistered call is pending so make call";
    m_callPending = false;
    callNumberToActiveService(m_number);	    
    m_number = "";
  }
}

void PeopleApplication::callNumberToActiveService(const QString& number){

  // hard-coded details of the MeeGo Dialer application
  QDBusInterface dialer("com.meego.dialer", "com/meego/dialer",
			"com.meego.dialer");
  if (!dialer.isValid()) {
    qWarning() << "Dialing" << number << "- could not find dialer app";
    return;
  }

  QDBusReply<void> reply = dialer.call(QDBus::BlockWithGui, "call", number);
  if (!reply.isValid())
    qWarning() << "Dialing" << number << "failed:" <<
      reply.error().message();        
}

void PeopleApplication::composeSMS(const QString& number)
{
  qDebug() << "Attempting to compose SMS to" << number;

  if(QDBusConnection::sessionBus().interface()->isServiceRegistered("com.meego.sms")){
    qDebug() << "PeopleApplication::composeSMS SMS is registered";
    smsNumberToActiveService(number);	    
    return;
  }else{
    qDebug() << "PeopleApplication::composeSMS SMS is NOT registered";
    QDBusReply<void> reply = QDBusConnection::sessionBus().interface()->startService("com.meego.sms");
    if (!reply.isValid()){
      qWarning() << "Starting SMS failed:" << reply.error().message();  
    }else{
      qDebug() << "PeopleApplication::composeSMS saving number to text on registration";
      m_smsPending = true;
      m_smsNumber = number;
    }           
  }
}

void PeopleApplication::handleSMSRegistered(){
  qDebug() << "PeopleApplication::handleSMSRegistered sms is registered";
  if(m_smsPending){
    qDebug() << "PeopleApplication::handleSMSRegistered text is pending so compose msg";
    m_smsPending = false;
    smsNumberToActiveService(m_smsNumber);	    
    m_smsNumber = "";
  }
}

void PeopleApplication::smsNumberToActiveService(const QString& number){
   // hard-coded details of the MeeGo SMS application
    QDBusInterface sms("com.meego.sms", "/", "com.meego.sms");
    if (!sms.isValid()) {
        qWarning() << "Composing SMS to" << number << "- could not find SMS app";
        return;
    }

    QDBusReply<void> reply = sms.call(QDBus::BlockWithGui, "showdialogpage", number);
    if (!reply.isValid())
        qWarning() << "Composing SMS to" << number << "failed:" <<
                reply.error().message();
}

void PeopleApplication::composeIM(const QString& accountString)
{
    qDebug() << "called composeIM" << accountString;
    QStringList list = accountString.split(":");
    if(list.count() != 3){
        qDebug() << "PeopleApplication::composeIM had invalid accountname";
        return;
    }

    QString accountPath = list.at(2);
    QString accountUri = list.at(0);
    //index=1 is nickname, not used here

    qDebug() << "Attempting to compose IM to accountPath: " << accountPath <<" accountUri: " << accountUri;

  if(QDBusConnection::sessionBus().interface()->isServiceRegistered("com.meego.meego_handset_chat")){
    qDebug() << "PeopleApplication::composeIM IM is registered";
    imToActiveService(accountString);
    return;
  }else{
    qDebug() << "PeopleApplication::composeIM IM is NOT registered";
    QDBusReply<void> reply = QDBusConnection::sessionBus().interface()->startService("com.meego.meego_handset_chat");
    if (!reply.isValid()){
      qWarning() << "Starting IM failed:" << reply.error().message();  
    }else{
      qDebug() << "PeopleApplication::composeIM saving info to IM on registration";
      m_imPending = true;
      m_imAccount = accountString;
    }           
  }
}

void PeopleApplication::handleIMRegistered(){
  qDebug() << "PeopleApplication::handleIMRegistered IM is registered";
  if(m_imPending){
    qDebug() << "PeopleApplication::handleIMRegistered im is pending so compose im";
    m_imPending = false;
    imToActiveService(m_imAccount);	    
    m_imAccount = "";
  }
}

void PeopleApplication::imToActiveService(const QString& account){
  QStringList list = account.split(":");
  if(list.count() != 3)
    return;

  QString accountPath = list.at(2);
  QString accountUri = list.at(0);
  //index=1 is nickname, not used here

  // hard-coded details of the MeeGo IM application
  QDBusInterface im("com.meego.meego_handset_chat", "/com/meego/meego_handset_chat", "com.meego.meego_handset_chat");
  if (!im.isValid()) {
    qWarning() << "Composing IM to path: " <<accountPath << " accountUri: " << accountUri << "- could not find IM app";
    return;
  }

  QDBusReply<void> reply = im.call(QDBus::BlockWithGui, "showDialogPageForContact", accountPath , accountUri);
  if (!reply.isValid())
    qWarning() << "Composing IM to path: " <<accountPath << " accountUri: " << accountUri << "failed:" ;
  reply.error().message();
}

void PeopleApplication::composeEmail(const QString& address) 
{
    qDebug() << "PeopleApplication::composeEmail attempting email:" << address;

    if(QDBusConnection::sessionBus().interface()->isServiceRegistered("com.meego.email")){
      qDebug() << "PeopleApplication::composeEmail email is registered";
	emailToActiveService(address);	    
	return;
    }else{
      qDebug() << "PeopleApplication::composeEmail email is NOT registered";
      QDBusReply<void> reply = QDBusConnection::sessionBus().interface()->startService("com.meego.email");
      if (!reply.isValid()){
	qWarning() << "Starting Email failed:" << reply.error().message();  
      }else{
      qDebug() << "PeopleApplication::composeEmail saving address to email on registration";
	m_emailPending = true;
	m_email = address;
      }           
    }
}

void PeopleApplication::handleEmailRegistered(){
      qDebug() << "PeopleApplication::handleEmailRegistered email is registered";
  if(m_emailPending){
    qDebug() << "PeopleApplication::handleEmailRegistered email is pending so compose msg  ";
    m_emailPending = false;
    emailToActiveService(m_email);	    
    m_email = "";
  }
}

void PeopleApplication::emailToActiveService(const QString& address){
    qDebug() << "Attempting to compose Email to" << address;

    // hard-coded details of the MeeGo Email application
    QDBusInterface email("com.meego.email", "/", "com.meego.email");
    if (!email.isValid()) {
        qWarning() << "Composing Email to" << address << "- could not find Email app";
        return;
    }

    QDBusReply<void> reply = email.call(QDBus::BlockWithGui, "showComposePage", address);
    if (!reply.isValid())
        qWarning() << "Composing Email to" << address << "failed:" <<
                reply.error().message();
}

void PeopleApplication::sliderActivated(int index, const QString& stop)
{
    Q_UNUSED(index)
    m_people->scrollTo(stop);
}

void PeopleApplication::repositionOverlays()
{
    QRectF exposed = m_mainPage->exposedContentRect();

    if (m_window->orientation() == M::Landscape) {
        m_sliderH->setPos(0, exposed.y());
        m_sliderH->show();
        m_sliderV->hide();
        m_topSpacer->setPreferredHeight(m_sliderH->preferredHeight());
        m_mainPage->layout()->setContentsMargins(0, 0, 0, 0);
    }
    else {
       int width = m_sliderV->preferredWidth();
       m_sliderV->setPos(exposed.width() - width, exposed.y());
        m_sliderV->setPreferredHeight(m_sliderH->preferredWidth()-exposed.y());
        m_sliderH->hide();
        m_sliderV->show();
        m_topSpacer->setPreferredHeight(0);
        m_mainPage->layout()->setContentsMargins(0, 0, m_sliderV->preferredWidth(), 0);
    }

    if (m_searchWidget)
        m_searchWidget->setPos(0, exposed.bottom() - m_searchWidget->preferredHeight());
}

void PeopleApplication::scrollTo(qreal pos)
{
    // effects: scrolls to the given vertical position, up to the maximum

    MApplicationPage *page = activeApplicationWindow()->currentPage();
    QGraphicsWidget *widget = page->centralWidget();
    MPannableWidget *viewport = page->pannableViewport();
    if (!widget || !viewport)
        return;

    QRectF exposed = page->exposedContentRect();
    int max = widget->size().height() - exposed.height();
    if (pos > max)
        pos = max;

    viewport->physics()->stop();
    viewport->physics()->setPosition(QPointF(0, pos));
}

void PeopleApplication::scrollIntoView(qreal ypos, qreal height)
{
    // effects: scrolls the defined vertical area into view, or as much as possible

    MApplicationPage *page = activeApplicationWindow()->currentPage();
    QGraphicsWidget *widget = page->centralWidget();
    MPannableWidget *viewport = page->pannableViewport();
    if (!widget || !viewport)
        return;

    QRectF exposed = page->exposedContentRect();
    int viewportHeight = exposed.height();

    qreal scrollpos = viewport->physics()->position().y();

    // check to see if it's already fully visible
    if (scrollpos < ypos && scrollpos + viewportHeight > ypos + height)
        return;

    // if bottom end is off-screen, scroll up until bottom is visible
    qreal scrollto = scrollpos;
    if (scrollpos + viewportHeight < ypos + height)
        scrollto += ypos + height - scrollpos - viewportHeight;

    // if top end is off-screen, scroll down until top is visible
    if (scrollto > ypos)
        scrollto = ypos;

    viewport->physics()->stop();
    viewport->physics()->setPosition(QPointF(0, scrollto));
}
