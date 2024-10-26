#ifndef CONTENTBTNCOUNT_H
#define CONTENTBTNCOUNT_H

#include <QDebug>

class contentBtnCount {
public:
    static int getTotalCnt(){ return totalContentBtnCount; }
    static int getMarkedCnt(){ return markedContentBtnCount; }

protected:
    //ctor as protected, to make this class only be usable as base class, but not instantiable
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

private:
    static int totalContentBtnCount;
    static int markedContentBtnCount;
};

#endif // CONTENTBTNCOUNT_H
