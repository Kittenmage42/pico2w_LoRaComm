/*
 LoRaComm, projekt erstellt von Oswin Joseph Anatolius Ziervogel, aka. Kittenmage42, https://github.com/Kittenmage42
 Dieses Projekt wurde für den Raspberry Pi Pico 2 W geschrieben, unter verwendung des Raspberry Pi Pico C/C++ SDKs und der ssd1306 bibliothek von David Schramm (https://github.com/daschr/pico-ssd1306)
 
 Dieses Projekt benötigt einen Raspberry Pi Pico 2 W, ein ssd1306 oled Display (128x64 pixel), einen Rotary Encoder sowie einen Knopf (optional, aber empfehlenswert wenn man keine Kabel per Hand verbinden will), ein Breadboard ist zu empfehlen, ich habe 13 Jumper-Kabel verwendet (7 MF), 6 MM)
 Link zum Verkabelungsdiagramm: https://wokwi.com/projects/461181496055270401
 Zudem befindet sich eine kopie von diagram.json in diesem ordner

*/

// If this Comment is on github I have succeded in getting it to work in xcode
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include <cstdio>
#include <cstdlib>
#include <array>
extern "C" {
    #include "ssd1306.h"
}
#include "valley.h"
#include "desc.h"
#include "font2.h"

// MARK: - Pin defines
#define PIN_CLK 14 // CLK Pin am Rotary Encoder - wichtig für funktion des Rotary Encoders
#define PIN_DT  15 // DT  Pin am Rotary Encoder - wichtig für funktion des Rotary Encoders
#define PIN_SW  16 // Knopf   am Rotary Encoder - zum auswählen des Zeichens

#define PIN_B   17 // Knopf zum senden des Buffers

volatile int32_t encoder_position = 0;
volatile uint8_t last_state = 0;

char selected_char = '0'; // The currently selected char for typing
bool SW_held = false; // für nicht spam beim knopf-drücken
bool B_held = false; // s.o.

std::array<char, 128> msg_buffer{}; // Nachrichtenbuffer, 128 chars (MeSsaGe_BUFFER)
size_t cur_pos = 0;                // Aktuelle Stelle in msg_buffer zum schreiben

// Zustandsübergangstabelle (Gray Code Decoder)
const int8_t transition_table[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
};

void encoder_callback(uint gpio, uint32_t events) {
    uint8_t clk = gpio_get(PIN_CLK);
    uint8_t dt  = gpio_get(PIN_DT);

    uint8_t current_state = (clk << 1) | dt;
    uint8_t index = (last_state << 2) | current_state;

    encoder_position += transition_table[index];
    last_state = current_state;
}

// MARK: - Main function -
int main() {
    
    // MARK: - Initialisation and setup -
    
    
    stdio_init_all();
    sleep_ms(100);
    printf("Program started");

    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }
    const uint LED_PIN = CYW43_WL_GPIO_LED_PIN;
    cyw43_arch_gpio_put(LED_PIN, 0);
    
    // Rotary Encoder Pins Initialisieren
    gpio_init(PIN_CLK);
    gpio_set_dir(PIN_CLK, GPIO_IN);
    gpio_pull_up(PIN_CLK);

    gpio_init(PIN_DT);
    gpio_set_dir(PIN_DT, GPIO_IN);
    gpio_pull_up(PIN_DT);

    gpio_init(PIN_SW);
    gpio_set_dir(PIN_SW, GPIO_IN);
    gpio_pull_up(PIN_SW);
    
    gpio_init(PIN_B);
    gpio_set_dir(PIN_B, GPIO_IN);
    gpio_pull_up(PIN_B);

    // I2C konfigurieren
    i2c_init(i2c0, 400000);
    gpio_set_function(4, GPIO_FUNC_I2C); // SDA
    gpio_set_function(5, GPIO_FUNC_I2C); // SCL
    gpio_pull_up(4);
    gpio_pull_up(5);

    // Display initialisieren
    ssd1306_t disp;
    disp.external_vcc = false;
    ssd1306_init(&disp, 128, 64, 0x3C, i2c0);
    
    // Anfangszustand setzen
    last_state = (gpio_get(PIN_CLK) << 1) | gpio_get(PIN_DT);
    
    // Interrupts auf beide Encoder-Pins
    gpio_set_irq_enabled_with_callback(PIN_CLK, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoder_callback);
    gpio_set_irq_enabled(PIN_DT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    
    int32_t last_encoder = encoder_position;
    
    
    // MARK: - Programm Start -
    

    
    // MARK: - Anfängliche Nachricht
    ssd1306_clear(&disp);
    ssd1306_bmp_show_image(&disp, valley, valley_len);
    ssd1306_show(&disp);
    sleep_ms(1000);
    ssd1306_draw_square(&disp, 12, 12, 104, 40);
    ssd1306_clear_square(&disp, 13, 13, 102, 38);
    ssd1306_draw_string(&disp, 16, 16, 2, "LoRaComm");
    ssd1306_draw_string(&disp, 16, 32, 1, "by Kittenmage42");
    ssd1306_show(&disp);
    sleep_ms(3000);
    
    while (true) {
        
        
        
        // Encoder auslesen
        int32_t diff = encoder_position - last_encoder;
        if (diff != 0) {
            last_encoder = encoder_position;
        }
        
        selected_char = static_cast<char>((-encoder_position/4) % 256);
        
        // MARK: - Display code (Grafik)
        ssd1306_clear(&disp);
        for (int8_t i = -4; i <= 4; i++) {
            if ( ( (selected_char + i) % 256 + 256) % 256 < 128){
                ssd1306_draw_string_with_font(&disp,
                                              58 + (i * 24)  + 8 * ((i < 0) ? -1 : (i > 0 ? 1 : 0)),
                                              (i==0) ? 42 : 48,
                                              (selected_char < 127 && selected_char > 32) ? ((i == 0) ? 2 : 1) : 1,
                                              font2_8x5,
                                              desc[((selected_char + i) % 256 + 256) % 256]);
            } else {
                ssd1306_draw_char_with_font(&disp,
                                              58 + (i * 24)  + 8 * ((i < 0) ? -1 : (i > 0 ? 1 : 0)),
                                              (i==0) ? 42 : 48,
                                              (selected_char < 127 && selected_char > 32) ? ((i == 0) ? 2 : 1) : 1,
                                              font2_8x5,
                                              ((selected_char + i) % 256 + 256) % 256);
            }
            // Dieses Monster von Code rendert einfach nur 9 Strings mit ein paar Spezialeffekten | вєωαяє, ι αм ƒαη¢у!
        }
        
        char buf[16];
        snprintf(buf, sizeof(buf), "%u", (uint8_t)selected_char);
        
        ssd1306_draw_string(&disp, 0, 0, 1, buf);
        ssd1306_draw_string(&disp, 128 - cur_pos * 6, 24, 1, msg_buffer.data());
        
        
        ssd1306_show(&disp);
        // MARK: Ende Grafik
        
        // MARK: - Zeichen in buffer schreiben
        if (gpio_get(PIN_SW) == 0) {
            if (!SW_held){
                cyw43_arch_gpio_put(LED_PIN, 1);
                
                if (selected_char == 128) { // 128 neues Backspace
                    if (cur_pos != 0) {
                        msg_buffer[cur_pos] = '\0';
                        msg_buffer[--cur_pos] = '\0';
                    }
                } else if (cur_pos + 1 < msg_buffer.size()) {
                    msg_buffer[cur_pos++] = selected_char;
                    msg_buffer[cur_pos] = '\0';
                }
                
                SW_held = true;
            }
        } else {
            cyw43_arch_gpio_put(LED_PIN, 0);
            SW_held = false;
        }
        
        // MARK: - Buffer senden
        if (gpio_get(PIN_B) == 0) {
            if (!B_held){
                
                printf("%s\n", msg_buffer.data());
                msg_buffer.fill(0);
                cur_pos = 0;
                
                B_held = true;
            }
        } else {
            B_held = false;
        }
    }
}
