class HeartRate {
  private const byte _interruptPin;
  private volatile int interruptCounter = 0;
  private int numberOfInterrupts = 0;
  public HeartRate(byte interruptPin)
  {
    _interruptPin = interruptPin;
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
  }
  public void begin()
  {
    pinMode(interruptPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);
  }
  void IRAM_ATTR handleInterrupt() {
    portENTER_CRITICAL_ISR(&mux);
    interruptCounter++;
    portEXIT_CRITICAL_ISR(&mux);
  }

  public void loop() {
  }
  
  if(interruptCounter>0){

      portENTER_CRITICAL(&mux);
      interruptCounter--;
      portEXIT_CRITICAL(&mux);

      numberOfInterrupts++;
      Serial.print("An interrupt has occurred. Total: ");
      Serial.println(numberOfInterrupts);
  }
}


}

