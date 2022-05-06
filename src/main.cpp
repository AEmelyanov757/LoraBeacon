#include <Arduino.h>
#include <HardwareSerial.h>

// Глобальные переменные для taskBlink, индикатор состояния изделия 
const int LED_PIN = LED_BUILTIN; // Pin of the LED. P/s: реально пин на GPIO_NUM_2 
enum ledmode_t : uint8_t {LED_OFF, LED_ON, LED_1HZ, LED_2HZ, LED_4HZ}; // LED_OFF - выключено/сон изделия
                                                                       // LED_ON  - запуск изделия
                                                                       // LED_1HZ - нормальная работа изделия
                                                                       // LED_2HZ - поиск спутников GPS
                                                                       // LED_4HZ - признак критической ошибки изделия
TaskHandle_t handleTaskBlink; // хэндл задачи, для управления

// Глобальные переменные для GPS module
const int RX_NEO = GPIO_NUM_17; // uart_2
const int TX_NEO = GPIO_NUM_16; // uart_2
HardwareSerial SerialGPS(2);
TaskHandle_t handleTaskGPS; // хэндл задачи, для управления

// Глобальные переменные для Lora module
const int AUX_E220 = GPIO_NUM_18;
const int M0_E220 = GPIO_NUM_21;
const int M1_E220 = GPIO_NUM_19;
const int RX_E220 = GPIO_NUM_10; // uart_1
const int TX_E220 = GPIO_NUM_9;  // uart_1
HardwareSerial SerialLora(1);
TaskHandle_t handleTaskLora; // хэндл задачи, для управления

/*
** Задача индикация состояния изделия
*/
void taskBlink(void * parameter){
  // инициализация работы задачи
  const uint32_t LED_PULSE = 25; // Длина одного "блинка"
  ledmode_t ledmode = LED_ON;   // статус: "запуск изделия"

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);   // сразу зажигаем диод

  // основной цикл задачи
  while(true){
    // переменная управления состоянием диода через xTaskNotify()
    uint32_t notifyValue;

    // получаем текущие состояние
    if(xTaskNotifyWait(0, 0, &notifyValue, (ledmode < LED_1HZ ? portMAX_DELAY : 0)) == pdTRUE){
      ledmode = (ledmode_t)notifyValue; // Сохранили новое значение
    }

    // применяем состояние диода
    if(ledmode == LED_OFF){
      digitalWrite(LED_PIN, LOW);      // режим отключен диод
    }else if(ledmode == LED_ON){
      // режим включен диод
      digitalWrite(LED_PIN, HIGH);
    }else{
      // режимы мигания
      digitalWrite(LED_PIN, HIGH);
      vTaskDelay(pdMS_TO_TICKS(LED_PULSE)); // время горения диода
      digitalWrite(LED_PIN, LOW);
      vTaskDelay(pdMS_TO_TICKS((ledmode == LED_1HZ ? 1000 : ledmode == LED_2HZ ? 500 : 250) - LED_PULSE));
    }

  }
}

/*
** Задача индикация состояния изделия
*/
void taskGPS(void * parameter){
  // инициализация работы задачи
  SerialGPS.begin(9600,SERIAL_8N1,TX_NEO,RX_NEO);

  // основной цикл задачи
  while(true){
    //
  }
}

/*
** Задача индикация состояния изделия
*/
void taskLora(void * parameter){
  SerialLora.begin(9600,SERIAL_8N1,TX_E220,RX_E220);

  // основной цикл задачи
  while(true){
    //
  }
}

/*
** Функция инициализации МК
*/
void setup() {

  if(xTaskCreate(
    taskBlink,       // Function that should be called
    "Toggle LED",    // Name of the task (for debugging)
    1024,            // Stack size (bytes)
    NULL,            // Parameter to pass
    2,               // Task priority
    &handleTaskBlink // Task handle
  ) != pdPASS){
    Serial.println("ERROR! Task Blink not start!");
    Serial.flush();
    esp_deep_sleep_start();
  }

}

void loop() {
  // статичная переменная для теста состояний
  static ledmode_t ledmodes = LED_OFF;

  if(ledmodes < LED_4HZ){
    ledmodes = (ledmode_t)((uint8_t)ledmodes + 1);
  }else ledmodes = LED_OFF;

  if(xTaskNotify(handleTaskBlink, ledmodes, eSetValueWithoutOverwrite) != pdPASS){
    Serial.println("ERROR! Seting LED mode is error!");
  }
  vTaskDelay(pdMS_TO_TICKS(5000));

}
  // функция loop() не используется, поэтому уничтожаем задачу
  //vTaskDelete(NULL); // удаление задачи

  // if(eTaskGetState(handleTaskBlink) == eSuspended){
  //   vTaskResume(handleTaskBlink);      // возвращаем из сна задачу
  // } else {
  //   vTaskSuspend(handleTaskBlink);     // отправили заду в сон
  //   vTaskDelay(pdMS_TO_TICKS(500));    // чтобы не горел диод во время сна
  //   digitalWrite(LED_PIN, LOW);        // выключаем его через 500 мс.
  // }
