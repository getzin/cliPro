#ifndef MOVEBUTTON_H
#define MOVEBUTTON_H

#include <QDialog>

namespace Ui {
class moveButton;
}

class moveButton : public QDialog
{
    Q_OBJECT

public:
    explicit moveButton(QWidget *parent = nullptr);
    ~moveButton();
    void openMenu(qsizetype index, qsizetype maxIndex);

signals:
    void updateButtonPosition(qsizetype oldIndex, qsizetype newIndex);

private slots:
    void save();

private:
    Ui::moveButton *ui;
    qsizetype oldIndex;
};

#endif // MOVEBUTTON_H
