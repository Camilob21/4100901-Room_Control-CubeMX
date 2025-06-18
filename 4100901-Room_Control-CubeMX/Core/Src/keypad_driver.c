#include "keypad_driver.h"
#include "stm32l4xx_hal.h"
#include <stdio.h> // Para printf (solo para depuración)

// Mapa de teclas para un keypad 4x4
static const char keymap[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// Inicializa los pines del keypad

/**
 * @brief Inicializa el keypad configurando los pines de filas y columnas.
 * @param kpd Puntero a la estructura del manejador del keypad. 
 */
void keypad_init(keypad_handle_t *kpd) {
    // Configura las filas como salidas
   // for (int r = 0; r < 4; r++) {
      //  GPIO_InitTypeDef GPIO_InitStruct = {0};
       // GPIO_InitStruct.Pin = kpd->row_pins[r];
      //  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      //  GPIO_InitStruct.Pull = GPIO_NOPULL;
      //  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
     //   HAL_GPIO_Init(kpd->row_ports[r], &GPIO_InitStruct);
     //   HAL_GPIO_WritePin(kpd->row_ports[r], kpd->row_pins[r], GPIO_PIN_RESET);
 //  }

    // Configura las columnas como entradas con pull-down (o pull-up según tu hardware)
   // for (int c = 0; c < 4; c++) {
       // GPIO_InitTypeDef GPIO_InitStruct = {0};
      //  GPIO_InitStruct.Pin = kpd->col_pins[c];
       // GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
     //   GPIO_InitStruct.Pull = GPIO_PULLDOWN; // O GPIO_PULLUP según tu circuito
   //     HAL_GPIO_Init(kpd->col_ports[c], &GPIO_InitStruct);
 //   }
}

// Escanea el keypad cuando ocurre una interrupción en una columna
/**
 * @brief Escanea el keypad para detectar la tecla presionada.
 * @param kpd Puntero a la estructura del manejador del keypad.
 */
char keypad_scan(keypad_handle_t *kpd, uint16_t triggered_col_pin) {
    static uint32_t last_key_press_time = 0;
    uint32_t current_time = HAL_GetTick();

    // Debounce: verifica si ha pasado suficiente tiempo desde la última pulsación
    if (current_time - last_key_press_time < 150) { // Ajusta este valor (en ms)
        return '\0'; // Ignora la pulsación
    }

    // Identifica la columna que generó la interrupción
    int col_idx = -1;
    for (int c = 0; c < 4; c++) {
        if (kpd->col_pins[c] == triggered_col_pin) {
            col_idx = c;
            break;
        }
    }
    if (col_idx == -1) return '\0'; // Pin no corresponde a ninguna columna

    // Escaneo: activa cada fila una por una y lee la columna
    for (int row = 0; row < 4; row++) {
        // Pone la fila actual en ALTO, las demás en BAJO
        for (int r = 0; r < 4; r++) {
            if (r == row) {
                HAL_GPIO_WritePin(kpd->row_ports[r], kpd->row_pins[r], GPIO_PIN_SET); // Activa la fila actual
            } else {
                HAL_GPIO_WritePin(kpd->row_ports[r], kpd->row_pins[r], GPIO_PIN_RESET); // Desactiva las otras filas
            }
        }
        // Pequeño retardo para estabilizar
        for (volatile int d = 0; d < 100; d++);

        // Lee el estado de la columna
        GPIO_PinState col_state = HAL_GPIO_ReadPin(kpd->col_ports[col_idx], kpd->col_pins[col_idx]);
        if (col_state == GPIO_PIN_SET) {
            // Devuelve el carácter correspondiente
            // Limpia filas antes de salir
            for (int r = 0; r < 4; r++) {
                HAL_GPIO_WritePin(kpd->row_ports[r], kpd->row_pins[r], GPIO_PIN_RESET);
            }
            last_key_press_time = current_time; // Actualiza el tiempo de la última pulsación
            return keymap[row][col_idx];
        }
    }
    // Limpia filas al terminar
    for (int r = 0; r < 4; r++) {
        HAL_GPIO_WritePin(kpd->row_ports[r], kpd->row_pins[r], GPIO_PIN_RESET);
    }
    return '\0'; // No se detectó tecla
}