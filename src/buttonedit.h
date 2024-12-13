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
    buttonEdit(const buttonEdit&) = delete;
    buttonEdit& operator=(const buttonEdit&) = delete;
    void editButton(contentButton *const btn);

private slots:
    void save(bool const closeAtEnd);
    void handleSaveOnly();
    void handleSaveAndClose();

private:
    Ui::buttonEdit *ui;
    contentButton *currentlyEditing = nullptr;
    void init();
};

#endif // BUTTONEDIT_H
