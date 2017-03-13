# CC2640R2 SimpleLink LaunchPad Settings & Resources

The CC2640R2 LaunchPad consists of a LaunchPad board with a CC2640R2EM
(CC2640R2F device).

![](./images/CC2640R2_LAUNCHXL.jpg "CC2640R2 LaunchPad")

## Jumper Settings

* For any examples that use LEDs, make sure the LED’s jumper settings are closed.
* Set __RXD &lt;&lt;__ and __TXD &gt;&gt;__ to provide UART communication via the onboard USB debugger.
* If you use a BoosterPack with this board, the BoosterPack fits over J1 and J2.

## Board resources used in driver examples

The following table lists the Board names of the resources used by
the driver examples, and their corresponding pins.  Note that there may be
other resources configured in the board files that are not used by any
driver example.  Only the example resources are listed in the table.

  |Board Resource|Pin|
  |--------------|:---|
  |`Board_ADC0`|DIO23|
  |`Board_ADC1`|DIO24|
  |`Board_GPIO_BUTTON0`|DIO13 (BTN-1)|
  |`Board_GPIO_BUTTON1`|DIO14 (BTN-2)|
  |`Board_GPIO_LED0`|DIO6  (Red LED must be jumpered)|
  |`Board_GPIO_LED1`|DIO7  (Green LED must be jumpered)|
  |`Board_I2C_TMP`|DIO4, DIO5|
  |`Board_PIN_LED0`|DIO6  (Red LED must be jumpered)|
  |`Board_PIN_LED1`|DIO7  (Green LED must be jumpered)|
  |`Board_PIN_LED2`|DIO6  (Red LED must be jumpered)|
  |`Board_PWM0`|DIO6|
  |`Board_PWM1`|DIO7|
  |`Board_SPI0`|DIO8, DIO9, DIO10|
  |`Board_UART0`|DIO3, DIO2 UART provided through emulation (__RXD &lt;&lt;__ and __TXD &gt;&gt;__ must be jumpered)|

## Booster packs

The following examples require booster packs.

  |Example|Booster Pack|
  |-------|:------------|
  |display|[430BOOST-SHARP96 Card BoosterPack](http://www.ti.com/tool/430boost-sharp96)|
  |i2ctmp007|[Sensors BoosterPack](http://www.ti.com/tool/boostxl-sensors#1)|

## ADC example pin connections

The ADC examples sample input from the `Board_ADC0` and `Board_ADC1` pins.  These pins should
be connected to the analog input. For quick testing, the `Board_ADC0` pin can be
connected to `GND` and `Board_ADC1` to `GND`.

  |ADC Pin (Function)||Analog Input|
  |------------------|-|:-----------|
  |Board_ADC0|-->|GND|
  |Board_ADC1|-->|GND|

>__Important:__ Caution should be used when connecting the pins to analog inputs greater than 3VDC.

## Peripherals Used

The following list shows which CC2640R2_LAUNCHXL peripherals are used by
driver and kernel applications. Driver examples control which peripherals (and which ports) are used.

* __TI-RTOS Kernel (SYS/BIOS)__
    * Uses the CC26xx’s RTC timer and that timer's associated interrupts. The TI-RTOS Kernel manages the Interrupt Vector Table.
* __TI-RTOS Drivers__
    * __ADC:__ A CC13xx/CC26xx specific driver that uses the onboard ADC peripheral.
    * __Crypto:__ A CC13xx/CC26xx specific driver that uses the onboard AES Crypto processor.
    * __GPIO:__ The GPIO driver is used in various examples to drive `Board_GPIO_LED0` and `Board_GPIO_LED1`, and to monitor the `Board_GPIO_BUTTON0` and `Board_GPIO_BUTTON1` buttons.
    * __I<sup>2</sup>C:__ The I<sup>2</sup>C driver is used to manage the`Board_I2C0` I<sup>2</sup>C instance.
    * __PIN:__ A CC13xx/CC26xx specific driver PIN driver that uses 1 output pin for the onboard `Board_PIN_LED0` and `Board_PIN_LED1`, and 2 input pins for buttons `Board_PIN_BUTTON0` and `Board_PIN_BUTTON1`.
    * __RF:__ The RF driver uses the on-chip radio module on CC26xx devices.
    * __SPI:__ The SPI driver uses SPI0 for `Board_SPI0` SPI instance.
    * __UART:__ The UART driver uses UART0, which is attached to the onboard emulator to facilitate serial communications.
    * __Watchdog:__ The Watchdog driver example uses the Watchdog Timer WDT peripheral (`Board_WATCHDOG0`).
