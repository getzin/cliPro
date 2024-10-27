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

    void setPtrToListWidget(QListWidget *listWidget);

signals:
    void editName(QString oldName, QString newName);
    void newName(QString newName);

public slots:
    void newProfile();
    // void editProfile(int indexOfEditedProfile, QString previousName);
    void editProfile();

// signals:
//     void verifyInput();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void adjustOKButton();
    void processOK();

private:
    Ui::profileDialog *ui;

    dialogMode currMode = dialogModeNew;
    QString currEditName = "";
    int currEditIndex = -1;

    void start();
    // void processOK();
    QListWidget *listWidgetPtr;

    bool checkStringIsAlphanumeric(QString strToCheck);
    bool nameCanBeUsed(QString userInput);
    // void addEditNameActionToUnsavedActions(QString userInput);
    // void addNewNameActionToUnsavedActions(QString userInput);
    // void processUserInput(bool isEditOperation, QString windowName, QString promptText, QString defaultTxtForInput);
    // void processUserInput(QString userInput);
};

#endif // PROFILEDIALOG_H
