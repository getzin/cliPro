#ifndef CONTENTBTNCOUNT_H
#define CONTENTBTNCOUNT_H

#include <QDebug>

class contentBtnCount {
public:
    static qsizetype getTotalCnt(){ return totalContentBtnCount; }
    static qsizetype getMarkedForDelCnt(){ return markedForDeletionCount; }

protected:
    //ctor as protected, to make this class only usable as a base class, but not instantiable
    contentBtnCount(){
        ++totalContentBtnCount;
        qDebug() << "contentBtnCount (Ctor) | post-creation-button-count: " << totalContentBtnCount;
    }

    ~contentBtnCount(){
        --totalContentBtnCount;
        qDebug() << "contentBtnCount (Dtor) | post-deletion-button-count: " << totalContentBtnCount;
    }

    static qsizetype incrMarkedForDelCnt(){ return ++markedForDeletionCount; }
    static qsizetype decrMarkedForDelCnt(){ return --markedForDeletionCount; }

private:
    static qsizetype totalContentBtnCount;
    static qsizetype markedForDeletionCount;
};

#endif // CONTENTBTNCOUNT_H
