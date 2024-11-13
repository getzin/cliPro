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

    Ui::profileDialog *ui;
    dialogMode currMode = dialogModeNew;

    qsizetype currEditIndex = -1;
    QString currEditName = "";
    QListWidget *visibleProfileListPtr; //ToDo check if this can be implemented better or force it being set to something legit? Currently doing nullptr checks on it

    bool checkStringIsAlphanumeric(QString strToCheck);
    bool checkNameIsNotTaken(QString nameTocheck);

    static const int windowWidth = 240;
    static const int windowHeight = 130;
};

#endif // PROFILEDIALOG_H
