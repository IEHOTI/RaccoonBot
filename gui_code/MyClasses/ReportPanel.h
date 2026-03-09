#ifndef REPORTPANEL_H
#define REPORTPANEL_H

#include <QWidget>

class ReportPanel : public QWidget
{
    Q_OBJECT
protected:
    void keyPressEvent(QKeyEvent *event) override;

public:
    explicit ReportPanel(const QString &accountID = "", QWidget *parent = nullptr);
    void formingReport(const QString &str, bool sendImage);
    ~ReportPanel(); // delete in Release

private:
    void setupUI();

signals:
    void sendReport(const QString &str, const QString &accountID, bool &sendImage); // for client later

private:
    QString accountID;
};

#endif // REPORTPANEL_H
