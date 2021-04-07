#include "gendereditor.h"
#include "stringtable.h"

#include <QtWidgets>

GenderEditor::GenderEditor(QWidget *parent) :
    QComboBox(parent)
{
    addItem(StringTable::Str_Male);
    addItem(StringTable::Str_Female);
}

E_GENDER GenderEditor::gender() const
{
    return (E_GENDER)currentIndex();
}

void GenderEditor::setGender(E_GENDER gender)
{
    setCurrentIndex((int)gender);
}
