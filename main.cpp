/*
 LoRaComm, projekt erstellt von Oswin Joseph Anatolius Ziervogel, aka. Kittenmage42, https://github.com/Kittenmage42
 Dieses Projekt wurde für den Raspberry Pi Pico 2 W geschrieben, unter verwendung des Raspberry Pi Pico C/C++ SDKs und der ssd1306 bibliothek von David Schramm (https://github.com/daschr/pico-ssd1306)
 
 Dieses Projekt benötigt einen Raspberry Pi Pico 2 W, ein ssd1306 oled Display (128x64 pixel), einen Rotary Encoder sowie einen Knopf (optional, aber empfehlenswert wenn man keine Kabel per Hand verbinden will), ein Breadboard ist zu empfehlen, ich habe 13 Jumper-Kabel verwendet (7 MF), 6 MM)
 Link zum Verkabelungsdiagramm: https://wokwi.com/projects/461266821758218241
 Zudem befindet sich eine kopie von diagram.json in diesem ordner

*/

// If this Comment is on github I have succeded in getting it to work in xcode

#include <stdio.h>
#include <cmath>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include <cstdio>
#include <cstdlib>
#include <array>
#include <string>
extern "C" {
    #include "ssd1306.h"
}
#include "valley.h"
#include "desc.h"
#include "font2.h"

// MARK: - Pin defines
#define LED_PIN CYW43_WL_GPIO_LED_PIN // statt: const uint LED_PIN = CYW43_WL_GPIO_LED_PIN; das weiter unten wäre
//#define PIN_CLK 16 // CLK Pin am Rotary Encoder - wichtig für funktion des Rotary Encoders
//#define PIN_DT  17 // DT  Pin am Rotary Encoder - wichtig für funktion des Rotary Encoders
//#define PIN_SW  15 // Knopf   am Rotary Encoder - zum auswählen des Zeichens

#define PIN_B   14 // Knopf zum senden des Buffers

#define JOYSTICK_X_CHANNEL 0 // GP26
#define JOYSTICK_Y_CHANNEL 1 // GP27
#define JOYSTICK_BTN_PIN 15

//constexpr bool joystickmode = false; // unwichtig da rotary encoder rausgeschmissen wird (in favor of joysticks and potentiometers), daher default true

//// Rotary Encoder Funktionen und code, [deprecated] (wird vieleicht noch mal eingebaut und dann wechselbar gemacht, verschwendet aktuell aber unnötig Ressourcen)
//volatile int32_t encoder_position = 0;
//volatile uint8_t last_state = 0;
//
//// Zustandsübergangstabelle (Gray Code Decoder)
//const int8_t transition_table[16] = {
//     0, -1,  1,  0,
//     1,  0,  0, -1,
//    -1,  0,  0,  1,
//     0,  1, -1,  0
//};
//
//void encoder_callback(uint gpio, uint32_t events) {
//    uint8_t clk = gpio_get(PIN_CLK);
//    uint8_t dt  = gpio_get(PIN_DT);
//    
//    uint8_t current_state = (clk << 1) | dt;
//    uint8_t index = (last_state << 2) | current_state;
//    
//    encoder_position += transition_table[index];
//    last_state = current_state;
//}

char selected_char = '0'; // The currently selected char for typing
bool BTN_held = false; // für nicht spam beim knopf-drücken
bool B_held = false; // s.o.

float joystick_deadzone = 0.08f;

std::array<char, 128> msg_buffer{}; // Nachrichtenbuffer, 128 chars (MeSsaGe_BUFFER)
size_t cur_pos = 0;                // Aktuelle Stelle in msg_buffer zum schreiben

static float acc = 0.0f;

// MARK: - Main function -
int main() {
    
    // MARK: - Initialisation and setup -
    
    stdio_init_all();
    
    printf("Program pico2w_LoRaComm started");
    
    adc_init();
    
    // GPIO-Pins als ADC-Eingänge konfigurieren
    adc_gpio_init(26); // GP26 = ADC0
    adc_gpio_init(27); // GP27 = ADC1
    
    gpio_init(JOYSTICK_BTN_PIN);
    gpio_set_dir(JOYSTICK_BTN_PIN, GPIO_IN);
    gpio_pull_up(JOYSTICK_BTN_PIN);

    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }
    
    cyw43_arch_gpio_put(LED_PIN, 0);
    
    // UART Init
    uart_init(uart0, 9600);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);
    
    // Rotary Encoder Pins Initialisieren
//    gpio_init(PIN_CLK);
//    gpio_set_dir(PIN_CLK, GPIO_IN);
//    gpio_pull_up(PIN_CLK);
//
//    gpio_init(PIN_DT);
//    gpio_set_dir(PIN_DT, GPIO_IN);
//    gpio_pull_up(PIN_DT);
//
//    gpio_init(PIN_SW);
//    gpio_set_dir(PIN_SW, GPIO_IN);
//    gpio_pull_up(PIN_SW);
//    
//    gpio_init(PIN_B);
//    gpio_set_dir(PIN_B, GPIO_IN);
//    gpio_pull_up(PIN_B);

    // I2C konfigurieren
    i2c_init(i2c0, 400000);
    gpio_set_function(4, GPIO_FUNC_I2C); // SDA Pin
    gpio_set_function(5, GPIO_FUNC_I2C); // SCL Pin
    gpio_pull_up(4);
    gpio_pull_up(5);
    
    // 2nd Display
    i2c_init(i2c1, 400000);
    gpio_set_function(2, GPIO_FUNC_I2C); // SDA Pin
    gpio_set_function(3, GPIO_FUNC_I2C); // SCL Pin
    gpio_pull_up(2);
    gpio_pull_up(3);

    
    // Displays deklarieren
    ssd1306_t disp1;
    ssd1306_t disp2;
    
    // Display1 initialisieren
    disp1.external_vcc = false;
    ssd1306_init(&disp1, 128, 64, 0x3C, i2c0);
    
    // Display 2 initialisieren
    disp2.external_vcc = false;
    ssd1306_init(&disp2, 128, 64, 0x3C, i2c1);
    
    ssd1306_t* disps[2] = { &disp1, &disp2 }; // Pointer zu Displays
    
    // Anfangszustand setzen
//    last_state = (gpio_get(PIN_CLK) << 1) | gpio_get(PIN_DT);
    
    // Interrupts auf beide Encoder-Pins
//    gpio_set_irq_enabled_with_callback(PIN_CLK, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoder_callback);
//    gpio_set_irq_enabled(PIN_DT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    
//    int32_t last_encoder = encoder_position;
    
    std::string buffer;
    
    // MARK: - Programm Start -
    
    // MARK: - Anfängliche Nachricht
    for (size_t i = 0; i < 2; i++){
        ssd1306_clear(disps[i]);
        ssd1306_bmp_show_image(disps[i], valley, valley_len);
        ssd1306_show(disps[i]);
    }
    sleep_ms(1000);
    ssd1306_draw_square(disps[0], 12, 12, 104, 40);
    ssd1306_clear_square(disps[0], 13, 13, 102, 38);
    ssd1306_draw_string(disps[0], 16, 16, 2, "LoRaComm");
    ssd1306_draw_string(disps[0], 16, 32, 1, "by Kittenmage42");
    ssd1306_show(disps[0]);
    sleep_ms(3000);
    
    while (true) {
        
        // MARK: - INPUT
        
        // Encoder auslesen
//        int32_t diff = encoder_position - last_encoder;
//        if (diff != 0) {
//            last_encoder = encoder_position;
//        }
        
        // X-Achse lesen
        adc_select_input(JOYSTICK_X_CHANNEL);
        uint16_t x_raw = adc_read(); // 0–4095

        // Y-Achse lesen
        adc_select_input(JOYSTICK_Y_CHANNEL);
        uint16_t y_raw = adc_read(); // 0–4095

        // Button lesen (active LOW wegen Pull-Up)
        bool btn_pressed = !gpio_get(JOYSTICK_BTN_PIN);
        
        // Optional: auf -1.0 bis +1.0 normalisieren
        float x_norm = (x_raw / 2047.5f) - 1.0f;
        float y_norm = (y_raw / 2047.5f) - 1.0f;
        
        float x = std::abs(x_norm) >= joystick_deadzone ? x_norm : 0.0f;
        float y = std::abs(y_norm) >= joystick_deadzone ? y_norm : 0.0f;
        
        // MARK: - ausgewählten char bestimmen
//        selected_char = static_cast<char>((-encoder_position/4) % 256); // TODO: change to joystick input
//        selected_char += std::floor(x * 2);
//        selected_char = ((selected_char + (int)std::floor(x * 2)) % 256 + 256) % 256;
        
        acc += x * 0.6f; // 0.3f = Geschwindigkeitsfaktor, anpassen nach Gefühl

        if (std::abs(acc) >= 1.0f) {
            selected_char = ((selected_char + (int)acc) % 256 + 256) % 256;
            acc -= (int)acc; // Nachkommastellen behalten
        }
        
        // MARK: - Display code (Grafik)
        ssd1306_clear(disps[0]);
        for (int8_t i = -4; i <= 4; i++) {
            if ( ( (selected_char + i) % 256 + 256) % 256 < 128){
                ssd1306_draw_string_with_font(disps[0],
                                              58 + (i * 24)  + 8 * ((i < 0) ? -1 : (i > 0 ? 1 : 0)),
                                              (i==0) ? 42 : 48,
                                              (selected_char < 127 && selected_char > 32) ? ((i == 0) ? 2 : 1) : 1,
                                              font2_8x5,
                                              desc[((selected_char + i) % 256 + 256) % 256]);
            } else {
                ssd1306_draw_char_with_font(disps[0],
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
        
        ssd1306_draw_string(disps[0], 0, 0, 1, buf);
        ssd1306_draw_string(disps[0], 128 - cur_pos * 6, 24, 1, msg_buffer.data());
        
        
        ssd1306_show(disps[0]);
        // MARK: Ende Grafik
        
        // MARK: - Zeichen in buffer schreiben
        if (gpio_get(JOYSTICK_BTN_PIN) == 0) {
            if (!BTN_held){
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
                
                BTN_held = true;
            }
        } else {
            cyw43_arch_gpio_put(LED_PIN, 0);
            BTN_held = false;
        }
        
//        // MARK: - Buffer senden
        if (gpio_get(PIN_B) == 0) {
            if (!B_held){
                
                printf("%s\n", msg_buffer.data()); // Übertragung per USB
                uart_puts(uart0, msg_buffer.data()); // Übertragung per UART
                uart_puts(uart0, "\n");
                msg_buffer.fill(0);
                cur_pos = 0;
                
                B_held = true;
            }
        } else {
            B_held = false;
        }
        
        // MARK: - Buffer Empfangen und Ausgeben
        
        if (uart_is_readable(uart0)) {
            char c = uart_getc(uart0);

            if (c == '\n') {
                // komplette Nachricht erhalten
                printf("Empfangen: %s\n", buffer.c_str());
                ssd1306_clear(disps[1]);
                ssd1306_draw_string(disps[1], 0, 0, 1, buffer.c_str());
                ssd1306_show(disps[1]);
                buffer.clear();
            } else {
                buffer += c;
            }
        }
        
        // MARK: - kleine Pause damit es auch nicht zu schnell läuft (wegen dem joystick)
        sleep_ms(50);
    }
}
