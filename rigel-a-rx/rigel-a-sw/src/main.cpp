/*
  RadioLib STM32WLx Blocking Receive Example

  This example listens for LoRa transmissions using STM32WL MCU with
  integrated (SX126x) LoRa radio.

  To successfully receive data, the following settings have to be the same
  on both transmitter and receiver:
  - carrier frequency
  - bandwidth
  - spreading factor
  - coding rate
  - sync word
  - preamble length
   
  This example assumes Nucleo WL55JC1 is used. For other Nucleo boards
  or standalone STM32WL, some configuration such as TCXO voltage and
  RF switch control may have to be adjusted.

  Using blocking receive is not recommended, as it will lead
  to significant amount of timeouts, inefficient use of processor
  time and can some miss packets!
  Instead, interrupt receive is recommended.

  For default module settings, see the wiki page
  https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx126x---lora-modem

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/

// include the library
#include <RadioLib.h>

// no need to configure pins, signals are routed to the radio internally
STM32WLx radio = new STM32WLx_Module();

// set RF switch configuration for Nucleo WL55JC1
// NOTE: other boards may be different!
//       Some boards may not have either LP or HP.
//       For those, do not set the LP/HP entry in the table.
static const uint32_t rfswitch_pins[] =
                         {PC_3,  PC_4,  PC_5};
static const Module::RfSwitchMode_t rfswitch_table[] = {
  {STM32WLx::MODE_IDLE,  {LOW,  LOW,  LOW}},
  {STM32WLx::MODE_RX,    {HIGH, HIGH, LOW}},
  {STM32WLx::MODE_TX_LP, {HIGH, HIGH, HIGH}},
  {STM32WLx::MODE_TX_HP, {HIGH, LOW,  HIGH}},
  END_OF_MODE_TABLE,
};

HardwareSerial Serial2(PA3,PA2);

void setup() {
  Serial2.begin(115200);

  // set RF switch control configuration
  // this has to be done prior to calling begin()
  radio.setRfSwitchTable(rfswitch_pins, rfswitch_table);

  // initialize STM32WL with default settings, except frequency
  Serial2.print(F("[STM32WL] Initializing ... "));
int state = radio.begin(
    915, // center frequency
    10.4, // bandwidth
    12, // spreading factor
    6, // CR?
    18, // syncword
    10, // power dbm
    8,
    1.6,
    false
  );

  if (state == RADIOLIB_ERR_NONE) {
    Serial2.println(F("success!"));
  } else {
    Serial2.print(F("failed, code "));
    Serial2.println(state);
    while (true);
  }

  // set appropriate TCXO voltage for Rigel
  state = radio.setTCXO(3.0);
  radio.forceLDRO(true);
  radio.setRxBoostedGainMode(true);
  radio.implicitHeader(8);

  if (state == RADIOLIB_ERR_NONE) {
    Serial2.println(F("success!"));
  } else {
    Serial2.print(F("failed, code "));
    Serial2.println(state);
    while (true);
  }
}

void loop() {
  Serial2.print(F("[STM32WL] Waiting for incoming transmission ... "));

  // you can receive data as an Arduino String
  String str;
  int state = radio.receive(str);

  // you can also receive data as byte array
  /*
    byte byteArr[8];
    int state = radio.receive(byteArr, 8);
  */

  if (state == RADIOLIB_ERR_NONE) {
    // packet was successfully received
    Serial2.println(F("success!"));

    // print the data of the packet
    Serial2.print(F("[STM32WL] Data:\t\t"));
    Serial2.println(str);

    // print the RSSI (Received Signal Strength Indicator)
    // of the last received packet
    Serial2.print(F("[STM32WL] RSSI:\t\t"));
    Serial2.print(radio.getRSSI());
    Serial2.println(F(" dBm"));

    // print the SNR (Signal-to-Noise Ratio)
    // of the last received packet
    Serial2.print(F("[STM32WL] SNR:\t\t"));
    Serial2.print(radio.getSNR());
    Serial2.println(F(" dB"));

  } else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
    // timeout occurred while waiting for a packet
    Serial2.println(F("timeout!"));

  } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
    // packet was received, but is malformed
    Serial2.println(F("CRC error!"));

  } else {
    // some other error occurred
    Serial2.print(F("failed, code "));
    Serial2.println(state);

  }
}