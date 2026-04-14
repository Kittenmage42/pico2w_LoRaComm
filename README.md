# pico2w_LoRaComm
[De] Ein Projekt für den Raspberry Pi Pico 2 W, welches eine Kommunikation über lange strecke mit einem LoRa Modul ermöglichen soll. Aktuell nur schreiben in die Computerkonsole möglich, da das LoRa Modul noch nicht geliefert worden ist. 
Dieses Projekt verwendet ein ssd1306 OLED dieplay, einen Knopf und einen Rotary Encoder (sowie pico, breadboard und kabel). Weitere Infos in main.cpp

## Simulation Über wokwi: https://wokwi.com/projects/461266821758218241

## How to build / wie mache ich das jetzt auf den Pico?:

[De]
    [macOS / vermutlich auch Linux]
    Um dieses Projekt zu Kompilieren: 
        cd build #(im Terminal in den build Ordner gehen)
        cmake ..
        make # -j anhängen für einen schnelleren build (nutzt dann alle Prozessorkerne)
    nachdem diese Schritte abgeschlossen sind, befindet sich im build ordner eine Datei namens "pico2w_LoRaComm.uf2". Diese Datei muss man nur noch auf den pico Kopieren. Ein Pico 2 W sollte vorzugsweise verwendet werden, für einen Normalen Pico müsste die Funktionsweise der Internen LED umgeschrieben werden.
    Um dieses Projekt zu bauen muss zuerst natürlich das Pico C/C++ SDK korrekt eingerichtet werden. Dafür sollte eine andere Anleitung konsultiert werden. 
    Danach können die oben genannten Schritte ausgeführt werden. 
