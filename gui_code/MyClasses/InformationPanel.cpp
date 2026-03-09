#include "InformationPanel.h"

#include <QVBoxLayout>
#include <QTabWidget>
#include <QTextBrowser>
#include <QResource>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QPushButton>

InformationPanel::InformationPanel(QWidget *parent)
    : QWidget(parent, Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint | Qt::WindowTitleHint |Qt::WindowCloseButtonHint)
{
    this->hide();

    // Полупрозрачный фон с закруглениями
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet(R"(
        InformationPanel {
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

    setWindowTitle("Руководство пользователя");

    setupUI();
    this->setMinimumWidth(600);
    this->setMinimumHeight(400);
}

QString InformationPanel::loadGuidePage(const QString &guideName) {
    QString path = (":pages/guides/" + guideName + ".txt");
    QResource resource(path);
    if (!resource.isValid()) {
        qWarning() << "Resource not found:" << path;
        return QString();
    }

    QByteArray data = resource.uncompressedData();
    return QString::fromUtf8(data);
}

void InformationPanel::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    tabWidget = new QTabWidget(this);

    for (int i = 0; i < 2; ++i) {
        QString tabInformation, tabName;
        QTextBrowser *tab = new QTextBrowser(tabWidget);
        switch(i) {
        case 0: {
            tabInformation = loadGuidePage("mainGuide");
            tabName = "Главная";
            break;
        }
        case 1:{
            tabInformation = loadGuidePage("generalGuide");
            tabName = "Общее";
            break;
        }

        }
        tab->setReadOnly(true);
        tab->setHtml(tabInformation);// try
        tab->setOpenExternalLinks(true);
        tab->setOpenLinks(true); // links can open in browser
        tabWidget->addTab(tab,tabName);
    }
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(tabWidget);
    this->setLayout(layout);
}

void InformationPanel::showMe() {
    //this->resize(600,400);
    QWidget *mainWindow = this->parentWidget();

    QRect mainRect = mainWindow->geometry();
    QScreen* screen = QGuiApplication::screenAt(mainWindow->pos());
    QRect screenRect = screen ? screen->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry();

    int tipWidth = this->width();
    int tipHeight = this->height();

    int x = mainRect.right();
    int y = mainRect.top();

    bool fitsOnRight = (x + tipWidth <= screenRect.right());

    int leftX = mainRect.left() - tipWidth;
    bool fitsOnLeft = (leftX >= screenRect.left());

    // Если справа не помещается, но слева помещается - показываем слева
    if (!fitsOnRight && fitsOnLeft) x = leftX;
    // Если не помещается ни справа, ни слева, прижимаем к краю экрана
    else if (!fitsOnRight) x = screenRect.right() - tipWidth - 10;

    // Проверяем вертикальное положение
    // Если подсказка выходит за нижний край экрана
    if (y + tipHeight > screenRect.bottom()) y = screenRect.bottom() - tipHeight - 10; // Отступ от нижнего края
    // Если подсказка выходит за верхний край экрана
    else if (y < screenRect.top()) y = screenRect.top() + 10; // Отступ от верхнего края

    this->move(x, y);
    this->show();
}

void InformationPanel::closeEvent(QCloseEvent* event)
{
    hide();
    event->ignore(); // Не закрываем, а скрываем с анимацией
}

void InformationPanel::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) hide();
    else QWidget::keyPressEvent(event);
}
