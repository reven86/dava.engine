CMake projects
==============

На данный момент отлажена сборка проектов под следующие среды разработки:

-   XCode

-   Visual Studio 12 2013 (x32 и x64 архитектуры, для генерации x64 используем "Visual Studio 12 2013 Win64")

-   Eclipse + ADT Plugin

Доступные cmake проекты во фреймворке - 

| N   | Имя             | путь                                    | win | mac | ios | android |
|-----|-----------------|-----------------------------------------|-----|-----|-----|---------|
| 1   | ResourceEditor  | dava.framework/Tools/ResourceEditor     | +   | +   |     |         |
| 2   | QuickEd         | dava.framework/Tools/QuickEd            | +   | +   |     |         |
| 3   | Launcher        | dava.framework/Projects/Launcher        | +   | +   |     |         |
| 4   | UnitTests       | dava.framework/Projects/UnitTests       | +   | +   | +   | +       |
| 5   | PerfomanceTests | dava.framework/Projects/PerfomanceTests | +   | +   | +   | +       |
| 6   | SceneViewer     | dava.framework/Projects/SceneViewer     | +   | +   | +   | +       |
| 7   | Magasolution    | dava.framework/Tools                    | +   | +   |     |         |

 

 

Для успешной генерации  проектов с помощь cmake необходимо следующее

**Для windows:**

-   CMake **3.3 **версии

-   если в терминале не работает команда cmake то нужно установить путь к  &lt;install path&gt;/CMake/bin в переменную окружения PATH

-   Visual Studio 12 (2013)

-   Qt 5.4.2 (для сборки эдиторов )

-   Eclipse + ADT Plugin (для сборки юнит тестов)

-   Android-NDK-r10e

-   Android-SDK API Level: 21

**Для windows 10:**

-   CMake любой версии

-   В Path дописать путь к git/bin

-   Visual Studio 2015

-   Windows 10 SDK версии **не ниже** 10.0.240.0

**Для mac:**

-   CMake  3.3 версии

-   Установить cmake command line tools одним из способов:

    -   выполнить команду: sudo /Applications/CMake.app/Contents/bin/cmake-gui" --install
        Тогда cmake проинсталлится в /usr/bin и попадет в path

    -   установить путь к  /Applications/CMake.app/Contents/bin в переменную окружения PATH
        Для этого требуется в консоли набрать *sudo nano /etc/paths* и в открывшемся редакторе дописать путь в конец с новой строки, после чего сохранить документ (*ctrl*+*X* -&gt; *Y* -&gt; *Enter*)

-   Xcode

-   Eclipse + ADT Plugin

-   Qt 5.4.2 (для сборки эдиторов )

-   Eclipse + ADT Plugin (для сборки юнит тестов)

-   Android-NDK-r9 -  Android-NDK-r10

-   Android-SDK API Level: 21

### Установка CMAKE

CMake можно скачать на сайте - <http://www.cmake.org/download/>; (гарантированно работает с версией 3.0.2 )

Для windows 10 потребуется так же:

-   Открыть dava.framework\\Tools\\WinStoreCMake в command line tool (для этого и нужен git в path. Из git bash скрипт не выполнится. Без git в path скрипт не выполнится).

-   Запустить rebuild\_cmake.bat. Он выкачает и соберет последнюю версию CMake для Windows 10.

Для MacOS:

-   в терминале выполнить *sudo /Application/CMake.app/Contents/bin/cmake-gui --install*

 

### Установка Qt

Qt 5 можно установить через online installer (рекомендуется):

-   Ссылка для [Windows](http://download.qt.io/archive/qt/5.4/5.4.2/qt-opensource-windows-x86-msvc2013-5.4.2.exe)

-   Ссылка для [OS X](http://download.qt.io/archive/qt/5.4/5.4.2/qt-opensource-mac-x64-clang-5.4.2.dmg)

Для установки под Windows нужно выбрать следующие компоненты:

-   Qt - Qt 5.4 - **msvc2013 32-bit ~~OpenGL ~~** (опционально, чтобы заранее подготовиться к переходу на новую Visual Studio)

-   Qt - Qt 5.4 - **msvc2013 64-bit ~~OpenGL ~~** (опционально, чтобы заранее подготовиться к переходу на новую Visual Studio)

-   Qt - Qt 5.4 - **Source Components** (выбрать всё)

Для установки под OS X нужно выбрать следующие компоненты:

-   Qt - Qt 5.4 - **clang 64-bit **

-   Qt - Qt 5.4 - **Source Components** (выбрать всё)

 

Так же рекомендуется установить под Windows: [Qt Visual Studio Add-in](http://download.qt-project.org/official_releases/vsaddin/qt-vs-addin-1.2.4-opensource.exe). Он позволяет при отладке просматривать содержимое Qt-шных типов данных (строки, контейнеры и т.п.).

После его установки нужно открыть Visual Studio, зайти в меню Qt5 -&gt; Qt Options и добавить туда путь к установленному Qt. Например: c:\\Qt\\5.4\\msvc2013

### Настройка проекта

 Перед началом сборки нужно открыть файл "..\\dava.framework\\DavaConfig.in" и указать там пути к установленным инструментам. Например (при установке в пути по-умолчанию):

Если файл DavaConfig.in отсутствует его можно создать ручками по нижеуказанному шаблону.  Так же если его нет, он автоматически сгенерится при создание любого DAVA проекта цмейком )

 

> QT4\_PATH\_WIN = c:\\Qt4\\4.8.6
> QT5\_PATH\_WIN = c:\\Qt\\5.4\\msvc2013 
>
> QT5\_PATH\_WIN64 = c:\\Qt\\5.4\\msvc2013\_64\_opengl
>
> QT4\_PATH\_MAC =
> QT5\_PATH\_MAC = $ENV{HOME}/Qt/5.4/clang\_64
>
> ANDROID\_ANT  = d:\\apache-ant-1.9.4  
> ANDROID\_NDK  = d:\\android-ndk-r10
> ANDROID\_SDK  = d:\\android\_sdk
> ANDROID\_ABI   = armeabi-v7a
> ANDROID\_NATIVE\_API\_LEVEL = 17
> ANDROID\_TARGET\_API\_LEVEL = 21
>
> Файл "DavaConfig.in" не коммитить.

### Пример генерации проектов

Используйте графический интерфейс **cmake-gui** так вы уменьшите вероятность ошибок при генерации т.к.

увидите дополнительные переменные влияющие на сборку и сам процесс генерации разделен на конфигурацию и саму генерацию, а

это влияет на сборку андроида, так же вы можете положить сбоку куда угодно и когда нужно быстро переключиться снова именно на ее.

**Если нужно собрать из терминала**: заходим в корневую папку проекта( где живет файл CMakeLists.txt),** **вызываем следующие команды

Для **Windows**:

> mkdir \_build
>
> cd \_build
>
> cmake  -G "Visual Studio 12" .. (либо cmake  -G "Visual Studio 12 Win64" .. для генерации x64 архитектуры)

Для **OS X**:

> mkdir \_build
>
> cd \_build
>
> cmake -G "Xcode" ..

Для** IOS** :

> mkdir \_build
>
> cd \_build
>
> cmake -G"Xcode" -DCMAKE\_TOOLCHAIN\_FILE=../../../Sources/CMake/Toolchains/ios.toolchain.cmake ..
>
>  
>
> для BLITZ
>
> cmake -G"Xcode" -DCMAKE\_TOOLCHAIN\_FILE=../dava.framework/Sources/CMake/Toolchains/ios.toolchain.cmake ..
>
>  

Для ** MAC+ANDROID **:

> mkdir \_build
>
> cd \_build
>
> cmake  -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE\_TOOLCHAIN\_FILE=../../../Sources/CMake/Toolchains/android.toolchain.cmake ..
>
> (повторить еще раз)cmake  -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE\_TOOLCHAIN\_FILE=../../../Sources/CMake/Toolchains/android.toolchain.cmake ..
>
>  
>
> для BLITZ
>
> cmake -G"Unix Makefiles" -DCMAKE\_TOOLCHAIN\_FILE=../dava.framework/Sources/CMake/Toolchains/android.toolchain.cmake ..
>
> (повторить еще раз) cmake ...
>
>  

Для ** WINDOWS+ANDROID **:

> mkdir \_build
>
> cd \_build
>
> cmake  -G "Eclipse CDT4 - NMake Makefiles" -DCMAKE\_TOOLCHAIN\_FILE=../../../Sources/CMake/Toolchains/android.toolchain.cmake ..
>
> (повторить еще раз)cmake  -G "Eclipse CDT4 - NMake Makefiles" -DCMAKE\_TOOLCHAIN\_FILE=../../../Sources/CMake/Toolchains/android.toolchain.cmake ..
>
>  
>
> для BLITZ
>
> cmake  -G "Eclipse CDT4 - NMake Makefiles" -DCMAKE\_TOOLCHAIN\_FILE=../dava.framework/Sources/CMake/Toolchains/android.toolchain.cmake ..
>
> (повторить еще раз) cmake ..

**Для WINDOWS 10**

Для генерации и сборки проекта под Win10 (UAP, UWP) требуется специфичный CMake - форк от форка Microsoft (предварительно собран).

Для пересборки cmake потребуются Git и cmake в PATH (cmake собирается cmake'ом, который собирается cmake'ом :)

Перед генерацией следует удостовериться, что в системе установлен Windows 10 SDK версии **не ниже** 10.0.240.0.

Скрипты для генерации лежат в директории dava.framework\\Tools\\WinStoreCMake

rebuild\_cmake.bat выполняет update\_cmake.bat и сборку MS CMake

update\_cmake.bat выкачивает репозиторий MS CMake'a или обновляет его

mk\_project.bat выполняет непосредственную генерацию проекта. В случае отсутствия MS CMake, вызывается rebuild\_cmake.bat

при желании / необходимости добавляем mk\_project.bat или собранный (c поддержкой win10) cmake.exe в PATH

       mkdir \_build

       cd \_build

       dava.framework\\Tools\\WinStoreCMake\\mk\_project.bat %1 %2 %3, где

                  %1 путь до CMakeList.txt (обязательный аргумент)

                  %2 архитектура проекта: ARM, Win32 или Win64 (необязательный аргумент, **если его не задавать, проект будет мультиплатформенным**)

                  %3 тэг Unibuild (необязательный аргумент, если он присутствует, проект будет сгенерирован с использованием Unity build)

Если есть необходимость собрать проект из командной строки то в папке со сгенерированным проектом (\_build) вызываем следующую команду:

cmake --build .

 
