#ifndef DYNBUTTON_H
#define DYNBUTTON_H

#include <QPushButton>

class dynButton : public QPushButton
{
    Q_OBJECT

public:
    enum btnMode {
        btnModeDISABLED = 0,
        btnModeADD = 1,
        btnModeRM = 2,
    };
    ~dynButton();
    explicit dynButton(QWidget *const parent = nullptr);
    dynButton(const dynButton&) = delete;
    dynButton& operator=(const dynButton&) = delete;
    btnMode getCurrBtnMode() const;

signals:
    void mainWindowButtonsNeedSwitch(dynButton::btnMode const mode);
    void keyPressOnDynBtn(int const key);

public slots:
    void setMode(dynButton::btnMode const mode);

protected:
    void keyPressEvent(QKeyEvent *const event) override;

private:
    void init();
    void setStyleSheetAdd();
    void setStyleSheetRm();
    void setStyleSheetDisabled();

    btnMode currBtnMode = btnModeADD;
};

#endif // DYNBUTTON_H
