This is a workbench for testing LibSWD on 32F0308DISCOVERY Development Board. Written by [Tomek CEDRO](http://www.tomek.cedro.info).

# 32F0308DISCOVERY Setup #

## Bypass ST-LINK ##

In order to test LibSWD with external Interface on 32F0308DISCOVERY:
* Solder SB13 to keep ST-LINK in reset state.
* Set CN2 to ON/Short to route connector to Target CPU.
* Connect your Interface to CN3/SWD connector signals.
* Connect Interface buffers reference voltage to VDD pin (NOT CN3 connector).

Remember to:
* provide Vref to the Interface buffers (to get correct voltage levels) - CN3 does NOT provide a reference VDD voltage.
* power-down devices before rewire.
* connect GND signal first.
* power Target Board with MiniUSB connection.

## ST-LINK Setup ##

DevelKit comes with on-board debug interface (ST-LINK/V2) working in SWD only mode. This interface may be routed to the Target CPU or external Target via CN3 connector:
 * CN2 ON: CN3 connects to CPU and ST-LINK.
 * CN2 OFF: CN3 connects to ST-LINK only.

## CN3 SWD Connector ##

* 1: VDD_TARGET (VDD **INPUT** from application)
* 2: SWCLK (SWD clock)
* 3: GND (Ground)
* 4: SWDIO (SWD data input/output)
* 5: NRST (RESET of target MCU)
* 6: SWO (Reserved)

## JP2 (Idd) ##

JP2 can be used to measure current consumption by the Target CPU. If you do not intend to measure this current, leave JP2 ON (short-circuit).

## Solder Bridges ##

Solder Bridges are used to change default behavior of the board, but consume less space than jumpers.

* SB16,17 (X2 crystal)
 * OFF (Default): X2, C13, C14, R22 and R23 provide a clock. PF0, PF1 are disconnected from P1.
 * ON: PF0, PF1 are connected to P1 (R22, R23 and SB18 must not be fitted).
* SB6,8,10,12 (Default) 
 * ON (Default): Reserved, do not modify.
* SB5,7,9,11 (Reserved) 
 * OFF (Default): Reserved, do not modify.
* SB20,21 (X3 crystal)
 * OFF (Default): X3, C15, C16, R24 and R25 deliver a 32 KHz clock. PC14, PC15 are not connected to P1.
 * ON: PC14, PC15 are only connected to P1 (R24, R25 must not be fitted).
* SB4 (B2-RESET) 
 * ON (Default): B2 push button is connected to the NRST pin of the STM32F030R8T6 MCU.
 * OFF: B2 push button is not connected the NRST pin of the STM32F030R8T6 MCU.
* SB3(B1-USER) 
 * ON (Default): B1 push button is connected to PA0.
 * OFF: B1 push button is not connected to PA0.
* SB1(VDD_3) 
 * ON (Default): VDD_3 must be permanently connected to VDD for normal use.
 * OFF: Reserved, do not modify.
* SB14,15 (RX,TX) 
 * OFF (Default): Reserved, do not modify.
 * ON: Reserved, do not modify.
* SB19 (NRST) 
 * ON (Default): NRST signal of the CN3 connector is connected to the NRST pin of the STM32F030R8T6 MCU.
 * OFF: NRST signal of the CN3 connector is not connected to the NRST pin of the STM32F030R8T6 MCU.
* SB22(T_SWO) 
 * ON (Default): SWO signal of the CN3 connector is connected to PB3. 
 * OFF: SWO signal is not connected. 
* SB13(STM_RST) 
 * OFF (Default): No incidence on STM32F103C8T6 (ST-LINK/V2) NRST signal. 
 * ON: STM32F103C8T6 (ST-LINK/V2) NRST signal is connected to GND. 
* SB2(BOOT0) 
 * ON (Default): BOOT0 signal of the STM32F030R8T6 MCU is held low through a 510 Ohm pull-down resistor. 
 * OFF: BOOT0 signal of the STM32F030R8T6 MCU can be set high through a 10 KOhm pull-up resistor R27 to solder.
* SB18 (MCO)
 * ON (Default): Provides the 8 MHz for OSC_IN from MCO of STM32F030R8T6.
* OFF: See SB16, SB17 description


# Resources #

* [Product Website](http://www.st.com/web/en/catalog/tools/PF259100)
* [Product User Manual / Technical Specification](http://www.st.com/st-web-ui/static/active/en/resource/technical/document/user_manual/DM00092306.pdf)

