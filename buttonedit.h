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
    void editButton(contentButton* btn);

private slots:
    void save(bool closeAtEnd);
    void handleSaveOnly();
    void handleSaveAndClose();

private:
    Ui::buttonEdit *ui;
    contentButton* currentlyEditing = nullptr;
};

#endif // BUTTONEDIT_H
