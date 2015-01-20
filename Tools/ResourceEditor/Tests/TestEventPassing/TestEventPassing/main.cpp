#include <QApplication>

#include <QPushButton>
#include <QMessageBox>
#include <QVBoxLayout>


#include "Container.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QWidget root;
    root.setLayout( new QVBoxLayout() );
    root.setMinimumSize( 640, 480 );

    Container *w = new Container(&root);
    QPushButton *btn = new QPushButton( "LOL", w );

    QObject::connect( btn, &QPushButton::clicked, []{ QMessageBox::information( nullptr, "", "OTAKE!!!" ); } );

    root.layout()->addWidget( w );
    root.show();

    return a.exec();
}
