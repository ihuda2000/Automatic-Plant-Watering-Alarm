We persist configuration in EEPROM:
- address 0..1 : uint16_t threshold (0–1023), default 650 (drier side)
- address 2    : uint8_t alarmEnabled (1/0), default 1
EEPROM size used: 16 bytes

