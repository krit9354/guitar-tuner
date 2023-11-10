const int ADC_PIN = 34;
// ADC1_CH6 / GPI034
// A FreeRTOS queue used to store ADC values
QueueHandle_t adc_queue;
// Callback function of the hardware timer.
void IRAM_ATTR timer_callback() {
  uint32_t value = analogReadMilliVolts(ADC_PIN);  //
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  // Send the ADC value to the FreeRTOS queue
  xQueueSendFromISR(adc_queue, &value, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken == pdTRUE) {
    portYIELD_FROM_ISR();
  }
}
// Initialize the ADC input channel.
void initADC() {
  // Set ADC resolution to 12 bits
  analogSetWidth(12);
  // set attenuation level to 11 dB.
  analogSetPinAttenuation(ADC_PIN, ADC_11db);
}

// Initialize the hardware timer.
void initTimer(uint32_t hw_timer_unit = 0) {      
  static hw_timer_t *timer = NULL;
  timer = timerBegin(1000000UL);  // 1MHz (1us tick)
  timerWrite(timer, 0);
  //Attach the callback function (ISR) to the timer
  timerAttachInterrupt(timer, &timer_callback);
  timerAlarm(timer, 500 /* period in usec*/, true,0);
  timerRestart(timer);
}
void setup() {
  // Initialize the Serial (use baudrate of 921600)
  Serial.begin(921600);
  Serial.print("\n\n\n");
  Serial.flush();
  // Create a FreeRTOS queue object.
  adc_queue = xQueueCreate(16, sizeof(uint32_t));
  initADC();  // Initialize the ADC.
  // Initialize the hardware timer.
  initTimer();
}
void loop() {
  uint32_t adc_reading;
  if (xQueueReceive(adc_queue, &adc_reading, portMAX_DELAY)) {
    Serial.printf("%lu\n", adc_reading);
  }
}