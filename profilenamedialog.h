#ifndef PROFILENAMEDIALOG_H
#define PROFILENAMEDIALOG_H

#include <QDialog>
#include <QListWidget>

namespace Ui {
class profileNameDialog;
}

class profileNameDialog : public QDialog
{
    Q_OBJECT

public:
    enum dialogMode {
        dialogModeEdit = 0,
        dialogModeNew = 1,
    };
    explicit profileNameDialog(QWidget *parent);
    ~profileNameDialog();
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

    Ui::profileNameDialog *ui;
    dialogMode currMode = dialogModeNew;

    qsizetype currEditIndex = -1;
    QString currEditName = "";
    QListWidget *visibleProfileListPtr = nullptr; //ToDo check if this can be implemented better or force it being set to something legit? Currently doing nullptr checks on it

    static int constexpr windowSize_w = 240;
    static int constexpr WindowSize_h = 130;
};

#endif // PROFILENAMEDIALOG_H
