#ifndef GENDEREDITOR_H
#define GENDEREDITOR_H

#include "clientbase.h"

#include <QComboBox>

class GenderEditor : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY(E_GENDER gender READ gender WRITE setGender USER true)
public:
    explicit GenderEditor(QWidget *parent = 0);

    E_GENDER gender() const;
    void setGender(E_GENDER gender);


signals:

public slots:

};

#endif // GENDEREDITOR_H
