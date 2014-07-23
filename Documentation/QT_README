Как добавлять объекты, пронаследованные от QObjects

1. Наследуем класс от QObject
2. Для MacOS:
	a. Show Package Content на файле проекта
	b. Редактируем qt_preprocess.mak
	c. Находим строки, представленные ниже и по аналогии с MainMenu добавляем свой объект

###=================================
compilers: ./moc_mainwindow.cpp ./moc_davaglwidget.cpp ./Classes/Qt/moc_MainMenu.cpp ./ui_mainwindow.h ./ui_davaglwidget.h 
compiler_objective_c_make_all:
compiler_objective_c_clean:
compiler_moc_header_make_all: moc_mainwindow.cpp moc_davaglwidget.cpp Classes/Qt/moc_MainMenu.cpp
compiler_moc_header_clean:
	-$(DEL_FILE) moc_mainwindow.cpp moc_davaglwidget.cpp Classes/Qt/moc_MainMenu.cpp
moc_mainwindow.cpp: mainwindow.h
	~/QtSDK/Desktop/Qt/4.8.1/gcc/bin/moc $(DEFINES) $(INCPATH) -D__APPLE__ -D__GNUC__ mainwindow.h -o moc_mainwindow.cpp

moc_davaglwidget.cpp: Classes/davaglwidget.h
	~/QtSDK/Desktop/Qt/4.8.1/gcc/bin/moc $(DEFINES) $(INCPATH) -D__APPLE__ -D__GNUC__ Classes/davaglwidget.h -o moc_davaglwidget.cpp

Classes/Qt/moc_MainMenu.cpp: Classes/Qt/MainMenu.h
	~/QtSDK/Desktop/Qt/4.8.1/gcc/bin/moc $(DEFINES) $(INCPATH) -D__APPLE__ -D__GNUC__ Classes/Qt/MainMenu.h -o Classes/Qt/moc_MainMenu.cpp
###=================================


3. Для Win32:
	a. добавляем файл [Filename].build в папку QtBuildTool
	b. кликаем правой кнопкой, выбираем свойства
	с. выбираем настройку для всех конфигураций
	d. C/C++ compiler меняем на Custom Build Tool
	e. настройки делаем аналогично уже добавленным (меняется только Command line)
	f. после первого успешного билда добавляем сгенерированный moc_[Filename].cpp в проект


 




