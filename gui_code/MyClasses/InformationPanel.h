#ifndef INFORMATIONPANEL_H
#define INFORMATIONPANEL_H

#include "qtabwidget.h"
#include <QWidget>

class InformationPanel : public QWidget
{
    Q_OBJECT
protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

public:
    explicit InformationPanel(QWidget *parent = nullptr);
    void showMe();

private:
    void setupUI();
    QString loadGuidePage(const QString &guideName); //guidename without .txt

signals:

private:
    QTabWidget *tabWidget;
};

#endif // INFORMATIONPANEL_H
