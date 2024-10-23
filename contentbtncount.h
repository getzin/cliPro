#ifndef CONTENTBTNCOUNT_H
#define CONTENTBTNCOUNT_H

#include <QDebug>

class contentBtnCount {
public:
    static int getTotalCnt(){ return totalContentBtnCount; }

protected:
    contentBtnCount(){
        totalContentBtnCount++;
        qDebug() << "contentBtnCount (Ctor) | post-creation-button-count: " << totalContentBtnCount;
    }

    ~contentBtnCount(){
        totalContentBtnCount--;
        qDebug() << "contentBtnCount (Dtor) | post-deletion-button-count: " << totalContentBtnCount;
    }

    static int incrMarkedCnt(){ return ++markedContentBtnCount; }
    static int decrMarkedCnt(){ return --markedContentBtnCount; }
    static int getMarkedCnt(){ return markedContentBtnCount; }

private:
    static int totalContentBtnCount;
    static int markedContentBtnCount;
};

#endif // CONTENTBTNCOUNT_H
