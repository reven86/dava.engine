//
//  LineEditExt.h
//  UIEditor
//
//  Created by Alexey Strokachuk on 9/20/14.
//
//

#ifndef __UIEditor__LineEditWithButton__
#define __UIEditor__LineEditWithButton__

#include <QLineEdit>

class QToolButton;

class LineEditExt: public QLineEdit
{
    Q_OBJECT
public:
    explicit LineEditExt(QWidget *parent = NULL);
    ~LineEditExt();
    
    bool clearButtonEnabled() const;
    void setClearButtonEnabled(bool enable);
    
protected:
    void resizeEvent(QResizeEvent *event);
    
private slots:
    void updateClearButton(const QString &text);
    
private:
    QString styleSheetForCurrentState() const;
    QString buttonStyleSheetForCurrentState() const;
    
    QToolButton *clearButton;
    bool buttonEnabled;
};

#endif /* defined(__UIEditor__LineEditWithButton__) */
