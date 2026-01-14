#include "MainWindow/MainWindow.h"
#include "MyClasses/GeneralData.h"

#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QMetaObject>
#include <QJsonObject>
#include <QJsonDocument>

#include "Controller/Controller.h"
#include "User/UserProfile.h"

void MainWindow::createGeneralTab(QWidget *tab, int index) {
    QListWidget* taskWidget = new QListWidget(tab);
    taskWidget->setObjectName("taskWidget");
    taskWidget->setGeometry(5,5,160,255);
    taskWidget->setDragEnabled(true);
    taskWidget->setAcceptDrops(true);
    taskWidget->setDropIndicatorShown(true);
    taskWidget->setDragDropMode(QAbstractItemView::InternalMove);

    QLabel* nameFile = new QLabel("Название сценария",tab);
    nameFile->setObjectName("nameFile");
    nameFile->setAlignment(Qt::AlignCenter);
    nameFile->setGeometry(5,265,160,20);

    QLineEdit *setFile = new QLineEdit(tab);
    setFile->setObjectName("setFile");
    setFile->setText("standart");
    setFile->setGeometry(5,290,180,25);
    setFile->setMaxLength(127);
    QString path = QDir::current().filePath("user/taskList/" + setFile->text() + ".stn");
    readTaskList(path,taskWidget);

    QPushButton* addTask = new QPushButton("Добавить\n"
                                           "задание",tab);
    addTask->setObjectName("addTask");
    addTask->setGeometry(175,5,95,40); //*,10,*,*

    QMenu *addMenu = new QMenu(tab);
    addMenu->setObjectName("addMenu");
    for(int i = 0, n = nameTasks.size(); i < n; ++i){
        QAction *soborAct = addMenu->addAction(nameTasks[i]);
        soborAct->setObjectName("addMenuAction");
        connect(soborAct,&QAction::triggered,this,[=](){
            taskWidget->addItem(nameTasks[i]);
        });
    }
    addTask->setMenu(addMenu);

    QPushButton* removeTask = new QPushButton("Удалить\n"
                                              "задание",tab);
    removeTask->setObjectName("removeTask");
    removeTask->setGeometry(280,5,95,40);// *,10,*,*

    connect(removeTask, &QPushButton::clicked,this,[=](){
        int i = taskWidget->currentRow();
        if(i > 0) taskWidget->takeItem(i);
        else taskWidget->takeItem(0);
        taskWidget->setCurrentRow(-1);
    });
    connect(taskWidget, &QListWidget::doubleClicked, removeTask, &QPushButton::click);

    QWidget* subTab = new QWidget(tab);
    subTab->setObjectName("subTab");
    subTab->setGeometry(175,50,200,230);

    QVBoxLayout* layout = new QVBoxLayout(subTab);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QCheckBox* adsTask = new QCheckBox("Просмотр реклам ресурсов",subTab);
    adsTask->setObjectName("adsTask");

    QCheckBox* resourcesTask = new QCheckBox("Сбор ресурсов и эмоций",subTab);
    resourcesTask->setObjectName("resourcesTask");

    QCheckBox* saverHPTask = new QCheckBox("Сохранять Историческую Мощь",subTab);//ИМ ?
    saverHPTask->setObjectName("saverHPTask");

    QCheckBox* giftTask = new QCheckBox("Отправка и получение"
                                        "\nподарков друзей",subTab);
    giftTask->setObjectName("giftTask");

    QCheckBox* dailyTask = new QCheckBox("Забирать сундук\n"
                                         "ежедневных заданий",subTab);
    dailyTask->setObjectName("dailyTask");

    QWidget* taker = new QWidget(subTab);
    taker->setObjectName("taker");
    taker->setFixedSize(210,20);

    QLabel* takeTask = new QLabel("Запрос ресурсов",taker);
    takeTask->setObjectName("takeTask");
    takeTask->setAlignment(Qt::AlignLeft);
    takeTask->setGeometry(0,0,120,20);

    QComboBox* takeBox = new QComboBox(taker);
    takeBox->setObjectName("takeBox");
    QStringList listResources{"Яблоки", "Золото", "Мана", "Дерево", "Металл", "Мифрилл"};
    takeBox->addItems(listResources);
    takeBox->setGeometry(110,0,90,20);

    QCheckBox* tourTask = new QCheckBox("Отправка путешествий",subTab);
    tourTask->setObjectName("tourTask");

    QCheckBox* foodTask = new QCheckBox("Открывать сундуки с едой",subTab);
    foodTask->setObjectName("foodTask");

    layout->addWidget(adsTask);
    layout->addWidget(resourcesTask);
    layout->addWidget(saverHPTask);
    layout->addWidget(giftTask);
    layout->addWidget(dailyTask);
    layout->addWidget(taker);
    layout->addWidget(tourTask);
    layout->addWidget(foodTask);
    subTab->setLayout(layout);

    QPushButton* addFile = new QPushButton("Сохранить",tab);
    addFile->setObjectName("addFile");
    addFile->setGeometry(195,290,85,25);

    connect(addFile, &QPushButton::clicked,this,[=](){
        QString temp = setFile->text();
        QList<QString> taskList;
        for(int i = 0, n = taskWidget->count(); i < n; ++i) {
            QListWidgetItem *temp = taskWidget->item(i);
            taskList.append(temp->text());
        }
        saveTaskList(temp, taskList);
    });

    QPushButton* loadFile = new QPushButton("Загрузить",tab);
    loadFile->setObjectName("loadFile");
    loadFile->setGeometry(290,290,85,25);

    connect(loadFile,&QPushButton::clicked,this,[=](){
        QString path = QFileDialog::getOpenFileName(this, "Выберите изображение", QDir::current().path() + "/user/taskList", "Settings (*.stn)");
        if(path != "") {
            setFile->setText(QFileInfo(path).baseName());
            readTaskList(path,taskWidget);
        }
    });

    connect(listData[index]->controller,&Controller::saveTaskQueue,this,[=](int local_index){
        listData[local_index]->listTaskQueue.clear();
        for(size_t i = 0, n = taskWidget->count(); i < n; ++i)
            listData[local_index]->listTaskQueue.append(taskWidget->item(i)->text());

        getSettings(local_index);
        QMetaObject::invokeMethod(listData[index].data(),"executeTasks",Qt::QueuedConnection);
    });
}

void MainWindow::saveTaskList(const QString &filename, const QList<QString> &taskList) {
    QDir dir("user/taskList");
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            QMessageBox::critical(this, "Ошибка", "Не удалось создать папку " + QDir::current().filePath("user_settings"), QMessageBox::Ok);
            return; // или обработка ошибки
        }
    }
    QString path = QDir::current().filePath("user/taskList/" + filename + ".stn");
    QFile file(path);

    // Открываем файл перед записью
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        //nullptr?
        QMessageBox::critical(nullptr, "Ошибка", "Не удалось открыть файл для записи: " + file.errorString(), QMessageBox::Ok);
        return;
    }

    QTextStream out(&file);
    for(const QString &task : taskList) out << task << "\n"; // Добавляем перевод строки
    file.close(); // Явное закрытие файла
}

void MainWindow::readTaskList(QString &path, QListWidget *widget) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //nullptr?
        QMessageBox::critical(nullptr, "Ошибка", "Не удалось открыть файл для записи: " + file.errorString(), QMessageBox::Ok);
        return;
    }
    QTextStream in(&file);
    widget->clear();
    while (!in.atEnd()) {
        QString line = in.readLine();
        if(nameTasks.contains(line)) widget->addItem(line);
    }
    file.close();
}
