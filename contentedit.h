#ifndef CONTENTEDIT_H
#define CONTENTEDIT_H

#include <QDialog>
#include "contentbutton.h"

namespace Ui {
class contentEdit;
}

class contentEdit : public QDialog
{
    Q_OBJECT

public:
    explicit contentEdit(QWidget *parent = nullptr);
    ~contentEdit();

    // void showWithText(QString text);
    void editButton(contentButton* btn);
    // QPushButton* getCancelBtn();
    // QPushButton* getSaveBtn();

private slots:
    void handleSave(bool closeAtEnd);
    void handleSaveOnly();
    void handleSaveAndClose();

private:
    Ui::contentEdit *ui;
    contentButton* currentlyEditing = nullptr;
};

#endif // CONTENTEDIT_H
