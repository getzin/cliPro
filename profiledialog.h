#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QDialog>
#include <QListWidget>

namespace Ui {
class profileDialog;
}

class profileDialog : public QDialog
{
    Q_OBJECT

public:
    enum dialogMode {
        dialogModeEdit = 0,
        dialogModeNew = 1,
    };
    explicit profileDialog(QWidget *parent = nullptr);
    ~profileDialog();
    void setPtrToVisibleProfileList(QListWidget *listWidget);

signals:
    void profileNameEdited(QString oldName, QString newName);
    void createNewProfile(QString newName);

public slots:
    void startNewProfileCreation();
    void startEditOfProfileName();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void adjustOKButton();
    void processOKbuttonPressed();

private:
    void start();
    bool checkStringIsAlphanumeric(QString strToCheck) const;
    bool checkNameIsNotTaken(QString nameTocheck) const;

    Ui::profileDialog *ui;
    dialogMode currMode = dialogModeNew;

    qsizetype currEditIndex = -1;
    QString currEditName = "";
    QListWidget *visibleProfileListPtr; //ToDo check if this can be implemented better or force it being set to something legit? Currently doing nullptr checks on it

    static int const windowWidth = 240;
    static int const windowHeight = 130;
};

#endif // PROFILEDIALOG_H
