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
    ~profileNameDialog();
    explicit profileNameDialog(QWidget *const parent = nullptr);
    profileNameDialog (const profileNameDialog&) = delete;
    profileNameDialog& operator= (const profileNameDialog&) = delete;
    void setPtrToVisibleProfileList(QListWidget const * const listWidget);
    static int constexpr maxProfileNameLength = 50;

signals:
    void profileNameEdited(QString const &oldName, QString const &newName);
    void createNewProfile(QString const &newName);

public slots:
    void startNewProfileCreation();
    void startEditOfProfileName();

protected:
    void keyPressEvent(QKeyEvent *const event) override;

private slots:
    void adjustOKButton();
    void processOKbuttonPressed();

private:
    void init();
    void start();
    bool checkStringIsAlphanumeric(QString const &strToCheck) const;
    bool checkNameIsNotTaken(QString const &nameTocheck) const;

    Ui::profileNameDialog *ui;
    dialogMode currMode = dialogModeNew;

    qsizetype currEditIndex = -1;
    QString currEditName = "";
    QListWidget const *PtrToVisibleProfileList = nullptr;

    static int constexpr windowSize_w = 240;
    static int constexpr WindowSize_h = 130;
};

#endif // PROFILENAMEDIALOG_H
