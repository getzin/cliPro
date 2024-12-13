#ifndef PROFILEBUTTON_H
#define PROFILEBUTTON_H

#include <QPushButton>
#include "profilemenu.h"

class profileButton : public QPushButton
{
public:
    profileButton(QWidget* &, profileMenu* menu);
    ~profileButton();

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    profileMenu* profilesMenuRef = nullptr;
};

#endif // PROFILEBUTTON_H
