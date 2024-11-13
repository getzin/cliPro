#ifndef DYNADDRMBUTTON_H
#define DYNADDRMBUTTON_H

#include <QPushButton>

class dynAddRmButton : public QPushButton
{
    Q_OBJECT

public:
    enum btnMode {
        btnModeADD = 0,
        btnModeRM = 1,
    };
    explicit dynAddRmButton(btnMode mode = btnModeADD);
    ~dynAddRmButton();
    btnMode getCurrBtnMode();

signals:
    void mainWindowButtonsNeedSwitch(dynAddRmButton::btnMode mode);
    void keyPressOnDynBtn(int key);

public slots:
    void setMode(dynAddRmButton::btnMode mode = btnModeADD);
    void switchMode();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    btnMode currBtnMode = btnModeADD;
    void setStyleSheetAdd();
    void setStyleSheetRm();

};

#endif // DYNADDRMBUTTON_H
