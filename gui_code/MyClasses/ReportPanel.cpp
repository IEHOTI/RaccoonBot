#include "ReportPanel.h"
#include "qdir.h"
#include "qevent.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QMessageBox>

ReportPanel::ReportPanel(const QString &accID, QWidget *parent)
    : QWidget(parent, Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint | Qt::WindowTitleHint |Qt::WindowCloseButtonHint), accountID(accID)
{

    // Полупрозрачный фон с закруглениями
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    setStyleSheet(R"(
        ReportPanel {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #ffffff, stop:1 #f0f0f0);
            border-radius: 12px;
            border: 2px solid #cccccc;
        }
        QPushButton#closeButton {
            background: #ff6b6b;
            border-radius: 10px;
            color: white;
            padding: 5px;
        }
    )");

    setWindowTitle("Отчёт об ошибке");

    setupUI();
    this->setFixedWidth(600);
    this->setFixedHeight(450);
    this->show();
}

ReportPanel::~ReportPanel() {
    qDebug() << "ReportPanel уничтожен, account:" << accountID;
}

void ReportPanel::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *label = new QLabel("Опишите ниже проблемную ситуацию");
    label->setAlignment(Qt::AlignCenter);
    label->setFixedHeight(50);

    QPlainTextEdit *textEdit = new QPlainTextEdit(this);
    textEdit->setPlaceholderText("Введите описание проблемы...");
    textEdit->setWordWrapMode(QTextOption::WordWrap);
    textEdit->setMinimumHeight(300);

    QWidget *buttonWidget = new QWidget(this);
    QHBoxLayout *labelLayout = new QHBoxLayout(buttonWidget);
    QCheckBox *checkBox = new QCheckBox("Прикрепить текущее изображение",this);
    QPushButton *sendButton = new QPushButton("Отправить",this);
    sendButton->setFixedWidth(75);
    connect(sendButton,&QPushButton::clicked,this,[=](){
        formingReport(textEdit->toPlainText(),checkBox->isChecked());
    });
    labelLayout->addWidget(checkBox);
    labelLayout->addWidget(sendButton);
    labelLayout->setSpacing(0);
    buttonWidget->setFixedHeight(50);


    layout->addWidget(label);
    layout->addWidget(textEdit,1);
    layout->addWidget(buttonWidget);
    layout->setSpacing(5);
    layout->setContentsMargins(0,0,0,0); // Минимальные отступы
}

void ReportPanel::formingReport(const QString &str, bool imageSend) {
    bool temp = imageSend;
    //temp
    QString path{"user/" + accountID};
    QDir dir(path);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            QMessageBox::critical(this, "Ошибка", "Не удалось создать папку " + QDir::current().filePath(path), QMessageBox::Ok);
            return; // или обработка ошибки
        }
    }
    QFile file(path + "/rep.mrc");
    // Открываем файл перед записью
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл для записи: " + file.errorString(), QMessageBox::Ok);
        return;
    }
    QTextStream stream(&file);
    stream << str;
    stream.flush(); // Необязательно, но полезно для гарантии записи
    file.close();
    //
    emit sendReport(str, accountID, temp);
    this->close();
}

void ReportPanel::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        this->close();
        return;
    }
    QWidget::keyPressEvent(event);
}
