#include "MainWindow.h"

#include <QSettings>
#include <QString>
#include <QTabWidget>
#include <QMetaProperty>
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
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
#include <QButtonGroup>
#include <QVariantMap>
#include <QWidgetAction>

void MainWindow::serializeAllPage(int id, QTabWidget *taskTabWidget) {
    QVariantMap allSettings;
    QString accountPrefix = QString("%1/").arg(id);

    // 1. Сохраняем индекс текущей вкладки
    allSettings[accountPrefix + "currentTab"] = taskTabWidget->currentIndex();

    // 2. Сохраняем ВСЕ вкладки по порядку (только с именем)
    for (int i = 0, n = taskTabWidget->count(); i < n; ++i) {
        QWidget *widget = taskTabWidget->widget(i);
        if (!widget || widget->objectName().isEmpty()) continue;
        saveWidget(allSettings, (accountPrefix + QString("Tab%1/").arg(i)), widget);
    }

    // 3. Сохраняем в бинарный файл
    QByteArray binaryData;
    QDataStream stream(&binaryData, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_8);
    stream << allSettings;

    // 4. Записываем в файл
    QFile file("user/" + QString::number(id) + "/data.mrc");
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(nullptr, "Ошибка",
                              "Не удалось открыть файл для записи: " + file.errorString(), QMessageBox::Ok);
        return;
    }

    file.write(binaryData);
    file.close();
}

void MainWindow::saveWidget(QVariantMap &settings, const QString &prefix, QWidget *widget) {
    if (!widget || widget->objectName().isEmpty()) return;

    QString widgetPrefix = prefix + widget->objectName() + "/";

    // Сохраняем пользовательские свойства виджета через QMetaObject
    const QMetaObject *meta = widget->metaObject();
    for (int i = 0, n = meta->propertyCount(); i < n; ++i) {
        QMetaProperty prop = meta->property(i);
        if (prop.isUser() && prop.isReadable()) {
            QVariant value = prop.read(widget);
            if (value.isValid()) settings[widgetPrefix + prop.name()] = value;
        }
    }

    if (QCheckBox *cb = qobject_cast<QCheckBox*>(widget)) settings[widgetPrefix + "checked"] = cb->isChecked();
    else if (QComboBox *combo = qobject_cast<QComboBox*>(widget)) {
        settings[widgetPrefix + "currentIndex"] = combo->currentIndex();
        settings[widgetPrefix + "currentText"] = combo->currentText();
    }
    else if (QLineEdit *edit = qobject_cast<QLineEdit*>(widget)) settings[widgetPrefix + "text"] = edit->text();
    else if (QRadioButton *radio = qobject_cast<QRadioButton*>(widget)) settings[widgetPrefix + "checked"] = radio->isChecked();
    else if (QListWidget *list = qobject_cast<QListWidget*>(widget)) {
        QStringList items;
        for (int i = 0, n = list->count(); i < n; ++i) items << list->item(i)->text();
        settings[widgetPrefix + "items"] = items;
    }
    else if (QButtonGroup *group = qobject_cast<QButtonGroup*>(widget)) settings[widgetPrefix + "checkedId"] = group->checkedId();
    else if (QTabWidget *tabWidget = qobject_cast<QTabWidget*>(widget)) settings[widgetPrefix + "currentIndex"] = tabWidget->currentIndex();
    else if (QPushButton *button = qobject_cast<QPushButton*>(widget)) {if (button->menu()) saveWidget(settings, widgetPrefix, button->menu());}
    else if (QMenu *menu = qobject_cast<QMenu*>(widget)) {
        QList<QAction*> actions = menu->actions();
        for (int i = 0, n = actions.size(); i < n; ++i) {
            QAction *action = actions.at(i);
            if (!action->objectName().isEmpty()) {
                QString actionPrefix = widgetPrefix + "action_" + QString::number(i) + "_" + action->objectName() + "/";
                settings[actionPrefix + "text"] = action->text();
                settings[actionPrefix + "checked"] = action->isChecked();
                settings[actionPrefix + "enabled"] = action->isEnabled();

                if (QWidgetAction *widgetAction = qobject_cast<QWidgetAction*>(action)){
                    if (widgetAction->defaultWidget()) {
                        saveWidget(settings, actionPrefix, widgetAction->defaultWidget());
                    }
                }
            }
        }
    }

    // Рекурсивно сохраняем детей
    for (QObject *child : widget->children()) {
        if (QWidget *childWidget = qobject_cast<QWidget*>(child)) saveWidget(settings, widgetPrefix, childWidget);
        else if (QAction *action = qobject_cast<QAction*>(child)) {
            if (!action->objectName().isEmpty()) {
                QString actionPrefix = widgetPrefix + "action_" + action->objectName() + "/";
                settings[actionPrefix + "text"] = action->text();
                settings[actionPrefix + "checked"] = action->isChecked();
                settings[actionPrefix + "enabled"] = action->isEnabled();
            }
        }
    }
}

void MainWindow::unSerializeAllPage(int id, QTabWidget *taskTabWidget) {
    QString filePath = "user/" + QString::number(id) + "/data.mrc";
    QFile file(filePath);

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(nullptr, "Ошибка",
                              "Не удалось открыть файл для чтения: " + file.errorString(), QMessageBox::Ok);
        return;
    }

    QByteArray binaryData = file.readAll();
    file.close();

    if (binaryData.isEmpty()) return;

    QDataStream stream(&binaryData, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_6_8);

    QVariantMap allSettings;
    stream >> allSettings;

    if (stream.status() != QDataStream::Ok) return;

    QString accountPrefix = QString("%1/").arg(id);

    // 1. Восстанавливаем текущую вкладку
    if (allSettings.contains(accountPrefix + "currentTab")) {
        int savedTabIndex = allSettings[accountPrefix + "currentTab"].toInt();
        if (savedTabIndex >= 0 && savedTabIndex < taskTabWidget->count()) taskTabWidget->setCurrentIndex(savedTabIndex);
    }

    // 2. Загружаем ВСЕ вкладки по порядку
    for (int i = 0, n = taskTabWidget->count(); i < n; ++i) {
        QWidget *tab = taskTabWidget->widget(i);
        if (!tab || tab->objectName().isEmpty()) continue;
        loadWidget(allSettings, (accountPrefix + QString("Tab%1/").arg(i)), tab);
    }
}

void MainWindow::loadWidget(const QVariantMap &settings, const QString &prefix, QWidget *widget) {
    if (!widget || widget->objectName().isEmpty()) return;

    QString widgetPrefix = prefix + widget->objectName() + "/";

    // Загружаем пользовательские свойства виджета через QMetaObject
    const QMetaObject *meta = widget->metaObject();
    for (int i = 0, n = meta->propertyCount(); i < n; ++i) {
        QMetaProperty prop = meta->property(i);
        QString key = widgetPrefix + prop.name();
        if (settings.contains(key) && prop.isWritable()) {
            QVariant value = settings[key];
            if (value.isValid()) prop.write(widget, value);
        }
    }

    // Загружаем спец. свойства для разных типов виджетов
    if (QCheckBox *cb = qobject_cast<QCheckBox*>(widget)) {
        QString key = widgetPrefix + "checked";
        if (settings.contains(key)) {
            bool checked = settings[key].toBool();
            cb->setChecked(checked);
        }
    }
    else if (QComboBox *combo = qobject_cast<QComboBox*>(widget)) {
        QString keyIndex = widgetPrefix + "currentIndex";
        if (settings.contains(keyIndex)) {
            int idx = settings[keyIndex].toInt();
            if (idx >= 0 && idx < combo->count()) combo->setCurrentIndex(idx);
        }
    }
    else if (QLineEdit *edit = qobject_cast<QLineEdit*>(widget)) {
        QString key = widgetPrefix + "text";
        if (settings.contains(key)) {
            QString text = settings[key].toString();
            edit->setText(text);
        }
    }
    else if (QRadioButton *radio = qobject_cast<QRadioButton*>(widget)) {
        QString key = widgetPrefix + "checked";
        if (settings.contains(key)) {
            bool checked = settings[key].toBool();
            radio->setChecked(checked);
        }
    }
    else if (QListWidget *list = qobject_cast<QListWidget*>(widget)) {
        QString key = widgetPrefix + "items";
        if (settings.contains(key)) {
            QStringList items = settings[key].toStringList();
            list->clear();
            list->addItems(items);
        }
    }
    else if (QButtonGroup *group = qobject_cast<QButtonGroup*>(widget)) {
        QString key = widgetPrefix + "checkedId";
        if (settings.contains(key)) {
            int id = settings[key].toInt();
            if (QAbstractButton *btn = group->button(id)) btn->setChecked(true);
        }
    }
    else if (QTabWidget *tabWidget = qobject_cast<QTabWidget*>(widget)) {
        QString key = widgetPrefix + "currentIndex";
        if (settings.contains(key)) {
            int idx = settings[key].toInt();
            if (idx >= 0 && idx < tabWidget->count()) tabWidget->setCurrentIndex(idx);
        }
    }
    else if (QPushButton *button = qobject_cast<QPushButton*>(widget)) {
        if (button->menu())
            loadWidget(settings, widgetPrefix, button->menu());
    }
    else if (QMenu *menu = qobject_cast<QMenu*>(widget)) {
        QList<QAction*> actions = menu->actions();
        for (int i = 0, n = actions.size(); i < n; ++i) {
            QAction *action = actions.at(i);
            if (!action->objectName().isEmpty()) {
                QString actionPrefix = widgetPrefix + "action_" + QString::number(i) + "_" + action->objectName() + "/";

                QString keyText = actionPrefix + "text";
                if (settings.contains(keyText)) action->setText(settings[keyText].toString());

                QString keyChecked = actionPrefix + "checked";
                if (settings.contains(keyChecked)) action->setChecked(settings[keyChecked].toBool());

                QString keyEnabled = actionPrefix + "enabled";
                if (settings.contains(keyEnabled)) action->setEnabled(settings[keyEnabled].toBool());

                if (QWidgetAction *widgetAction = qobject_cast<QWidgetAction*>(action)) {
                    if (widgetAction->defaultWidget()) {
                        loadWidget(settings, actionPrefix, widgetAction->defaultWidget());
                    }
                }
            }
        }
    }

    // Рекурсивно загружаем детей
    for(QObject *child : widget->children()){
        if (QWidget *childWidget = qobject_cast<QWidget*>(child)) loadWidget(settings, widgetPrefix, childWidget);
        else if (QAction *action = qobject_cast<QAction*>(child)) {
            if (!action->objectName().isEmpty()) {
                QString actionPrefix = widgetPrefix + "action_" + action->objectName() + "/";

                QString keyText = actionPrefix + "text";
                if (settings.contains(keyText)) action->setText(settings[keyText].toString());

                QString keyChecked = actionPrefix + "checked";
                if (settings.contains(keyChecked)) action->setChecked(settings[keyChecked].toBool());

                QString keyEnabled = actionPrefix + "enabled";
                if (settings.contains(keyEnabled)) {
                    action->setEnabled(settings[keyEnabled].toBool());
                }
            }
        }
    }
}
