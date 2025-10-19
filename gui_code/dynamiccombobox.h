#ifndef DYNAMICCOMBOBOX_H
#define DYNAMICCOMBOBOX_H

#include <QComboBox>

class DynamicComboBox : public QComboBox {
    Q_OBJECT
public:
    explicit DynamicComboBox(QWidget* parent = nullptr);

protected:
    void showPopup() override;

private:
    void updateEmulatorList();
};
#endif // DYNAMICCOMBOBOX_H
