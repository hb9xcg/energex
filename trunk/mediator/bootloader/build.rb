#!/usr/bin/ruby

# Bootloader must be configured with less than 1kiB

`avr-gcc -mmcu=atmega644p -Wall -gdwarf-2  -Os -ffunction-sections --param inline-call-cost=2 -finline-limit=3 -fno-inline-small-functions -funsigned-bitfields -fpack-struct -fshort-enums -fsigned-char -MD -MP  -c  bootldr.c`
# was working but looks wrong: `avr-gcc -mmcu=atmega644p -Wl,-Map,bootldr.map,-section-start=.text=0xe000 bootldr.o -o bootldr.elf`
`avr-gcc -mmcu=atmega644p -Wl,-Map,bootldr.map,-section-start=.text=0xfc00 bootldr.o -o bootldr.elf`
`avr-objcopy -O ihex -R .eeprom  bootldr.elf bootldr.hex`

# avr-gcc bootldr.o -mmcu=atmega644p -Wl,-Ttext=0x1c00 -o project.out -Wl,-Map,project.map
