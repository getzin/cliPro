#ifndef BUTTONEDIT_H
#define BUTTONEDIT_H

#include <QDialog>
#include "contentbutton.h"

namespace Ui {
class buttonEdit;
}

class buttonEdit : public QDialog
{
    Q_OBJECT

public:
    explicit buttonEdit(QWidget *parent = nullptr);
    ~buttonEdit();

    // void showWithText(QString text);
    void editButton(contentButton* btn);
    // QPushButton* getCancelBtn();
    // QPushButton* getSaveBtn();

private slots:
    void handleSave(bool closeAtEnd);
    void handleSaveOnly();
    void handleSaveAndClose();

private:
    Ui::buttonEdit *ui;
    contentButton* currentlyEditing = nullptr;
};

#endif // BUTTONEDIT_H
