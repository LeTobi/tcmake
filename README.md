# TCMAKE
Kompiliert Ganze C++ Projekte automatisch nach Ordnersystem.

## Grundfunktion
Im Projektverzeichnis ```build/``` wird folgende Ausgabe erzeugt:
* **build/:** enthält Makefile. Kann optional .a Bibliotheken und Anwendungen enthalten.
* **build/bin/:** hier werden binärdaten zwischengespeichert
* **build/include/:** hierhin werden gewünschte headerdateien kopiert

## Projektstruktur
Die einzige Anforderung ist, dass header- und source-dateien den gleichen namen tragen. Es muss jedoch nicht unbedingt beides enthalten sein. Unterstützte Dateiformate: .h / .hpp / .cpp / .c

## tcmakefile
Das tcmakefile gehört in das oberste Projektverzeichnis. Es besteht aus einer Auswahl an Outputs. Jeder Output kann mehrere dateien wie bibliotheken oder Anwendungen Erzeugen.

Dateiformat:
```
selection mylib
lib libmyname.a
add ./

exe test.exe test.cpp
link extern-library1
link extern-library2
```
* ```selection```: beginnt eine neue Ausgabe (wird am Anfang der Datei vorausgesetzt)
* ```lib```: erzeugt eine Bibliothek
* ```add```: definiert einen Pfad, der der Bibliothek hinzugefügt werden soll
* ```exe```: erzeugt eine Anwendung
* ```link```: definiert zusätzliche Abhängigkeiten der Anwendung

## Anwendung

```tcmake [SELECTION] [OPTIONS]```

Wird tcmake ohne argumente aufgerufen, werden die verfügbaren Selections angezeigt.  
Wenn SELECTION angegeben ist, wird das entsprechende makefile erzeugt.  
Die einzige verfügbare Option ist "debug".

Beispiel: ```tcmake mylib debug```  
Oder einfach: ```tcmake mylib```