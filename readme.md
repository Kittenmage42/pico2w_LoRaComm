LoRaComm

LoRaComm (Long Range Communication deviceoderso) ist ein Projekt für den Raspberry Pi Pico (sollte auf allen Modellen laufen). 
Verwendung: 	Es wird ein ssd1306 OLED display benötigt (128X64), sowie ein Rotary Encoder mit eingebautem Knopf. 
		Verabelung: 	Display: SDA -> GP4, SCL -> GP5
				Rotary Encoder: CLK -> GP14, DT -> GP15, SW -> GP16
		Das Programm wird beim Start seinen Namen anzeigen, daraufhin werden einige sachen auf dem Display angezeigt: 
			oben links: aktuelles ASCII Zeichen (als Zahl)
			darunter: BEschreibung des zeichens (von Wikipedia)
			unten am Bildschirm: Graphische visualisierung des Zeichens (falls vorhanden) und der Zeichen die Davor / Danach kommen. (GUI oder so)
		Mit dem Rotary Encoder kann das aktuell ausgewählte Zeichen geändert werden. 
		Durch drücken des eingebauten knopfs des Rotary Encoders wird das Zeichen über den USB port des pico an den Computer (oder was auch immer am anderen Ende Hängt) gesendet. 

ToDo (dev): 
	LoRa modul einbinden
	Nachrichtenbuffer einbauen (erst schreiben, dann komplett senden)
	nicht diese Dokumentation vergessen
	support für Joystick hinzufügen
