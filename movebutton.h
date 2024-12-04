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
    ~moveButton();
    explicit moveButton(QWidget *const parent = nullptr);
    moveButton(const moveButton&) = delete;
    moveButton& operator=(const moveButton&) = delete;
    void openMenu(qsizetype const index, qsizetype const maxIndex);

signals:
    void updateButtonPosition(qsizetype const oldIndex, qsizetype const newIndex);

private slots:
    void save();

private:
    Ui::moveButton *ui;
    qsizetype oldIndex;
    void init();
};

#endif // MOVEBUTTON_H
