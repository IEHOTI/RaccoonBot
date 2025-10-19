#include "mainwindow.h"

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

#include "Controller/Controller.h"

#include "generaldata.h"

void MainWindow::createGeneralTab(QWidget *tab, int index) {
    QListWidget* taskWidget = new QListWidget(tab);
    taskWidget->setGeometry(5,5,160,255);
    taskWidget->setDragEnabled(true);
    taskWidget->setAcceptDrops(true);
    taskWidget->setDropIndicatorShown(true);
    taskWidget->setDragDropMode(QAbstractItemView::InternalMove);

    QLabel* nameFile = new QLabel("Название сценария",tab);
    nameFile->setAlignment(Qt::AlignCenter);
    nameFile->setGeometry(5,265,160,20);

    QLineEdit *setFile = new QLineEdit(tab);
    setFile->setText("standart");
    setFile->setGeometry(5,290,180,25);
    QString path = QDir::current().filePath("user_settings/" + setFile->text() + ".stn");
    readSettings(path,taskWidget);

    QPushButton* addTask = new QPushButton("Добавить\n"
                                           "задание",tab);
    addTask->setGeometry(175,5,95,40); //*,10,*,*

    QMenu *addMenu = new QMenu(tab);
    for(int i = 0; i < nameTasks.size();i++){
        QAction *soborAct = addMenu->addAction(nameTasks[i]);
        connect(soborAct,&QAction::triggered,this,[=](){
            taskWidget->addItem(nameTasks[i]);
        });
    }
    addTask->setMenu(addMenu);

    QPushButton* removeTask = new QPushButton("Удалить\n"
                                              "задание",tab);
    removeTask->setGeometry(280,5,95,40);// *,10,*,*

    connect(removeTask, &QPushButton::clicked,this,[=](){
        int i = taskWidget->currentRow();
        if(i > 0) taskWidget->takeItem(i);
        else taskWidget->takeItem(0);
        taskWidget->setCurrentRow(-1);
    });
    connect(taskWidget,&QListWidget::doubleClicked,removeTask,&QPushButton::click);

    QWidget* subTab = new QWidget(tab);
    subTab->setGeometry(175,50,200,230);

    QVBoxLayout* layout = new QVBoxLayout(subTab);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QCheckBox* adsTask = new QCheckBox("Просмотр реклам ресурсов",subTab);
    QCheckBox* resourcesTask = new QCheckBox("Сбор ресурсов и эмоций",subTab);
    QCheckBox* invasionTask = new QCheckBox("Прохождение вторжений",subTab);
    QCheckBox* giftTask = new QCheckBox("Отправка и получение"
                                        "\nподарков друзей",subTab);
    QCheckBox* dailyTask = new QCheckBox("Забирать сундук\n"
                                         "ежедневных заданий",subTab);
    QWidget* taker = new QWidget(subTab);
    taker->setFixedSize(210,20);
    QLabel* takeTask = new QLabel("Запрос ресурсов",taker);
    takeTask->setAlignment(Qt::AlignCenter);
    takeTask->setGeometry(0,0,160,20);
    QComboBox* takeBox = new QComboBox(taker);
    takeBox->addItem("0");
    takeBox->setGeometry(150,0,50,20);
    QCheckBox* tourTask = new QCheckBox("Отправка путешествий",subTab);
    QCheckBox* foodTask = new QCheckBox("Открывать сундуки с едой",subTab);

    layout->addWidget(adsTask);
    layout->addWidget(resourcesTask);
    layout->addWidget(invasionTask);
    layout->addWidget(giftTask);
    layout->addWidget(dailyTask);
    layout->addWidget(taker);
    layout->addWidget(tourTask);
    layout->addWidget(foodTask);
    subTab->setLayout(layout);

    QPushButton* addFile = new QPushButton("Сохранить",tab);
    addFile->setGeometry(195,290,85,25);

    connect(addFile,&QPushButton::clicked,this,[=](){
        QDir dir("user_settings");
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                QMessageBox::critical(this, "Ошибка", "Не удалось создать папку " + QDir::current().filePath("user_settings"), QMessageBox::Ok);
                return; // или обработка ошибки
            }
        }
        QString path = QDir::current().filePath("user_settings/" + setFile->text() + ".stn");
        QFile file(path);

        // Открываем файл перед записью
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл для записи: " + file.errorString(), QMessageBox::Ok);
            return;
        }

        QTextStream out(&file);
        for(int i = 0; i < taskWidget->count(); i++) {
            QListWidgetItem *temp = taskWidget->item(i);
            out << temp->text() << "\n"; // Добавляем перевод строки
        }
        file.close(); // Явное закрытие файла
    });

    QPushButton* loadFile = new QPushButton("Загрузить",tab);
    loadFile->setGeometry(290,290,85,25);

    connect(loadFile,&QPushButton::clicked,this,[=](){
        QString path = QFileDialog::getOpenFileName(this, "Выберите изображение", QDir::current().path() + "/user_settings", "Settings (*.stn)");
        if(path != "") {
            setFile->setText(QFileInfo(path).baseName());
            readSettings(path,taskWidget);
        }
    });
    connect(listData[index]->controller,&Controller::saveTaskQueue,this,[=](int local_index){
        listData[local_index]->listTaskQueue.clear();
        for(int i = 0; i < taskWidget->count();i++)
            listData[local_index]->listTaskQueue.append(taskWidget->item(i)->text());

        getSettings(local_index);
        QMetaObject::invokeMethod(listData[index].data(),"executeTasks",Qt::QueuedConnection);
    });
}

void MainWindow::readSettings(QString &path, QListWidget *widget) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл для записи: " + file.errorString(), QMessageBox::Ok);
        return;
    }
    QTextStream in(&file);
    widget->clear();
    while (!in.atEnd()) {
        QString line = in.readLine(); // Чтение строки
        widget->addItem(line);
    }
    file.close();
}
