# Noted Errata in various documents

## Game Boy Programming Manual
- `LD r, r'` command lists register D code as 101, which is the same as register L code. Correct code for D is 010 per Intel 8080 manual (and common sense)
- `LD dd, nn` command lists dd code 01 as pointing to the register pair DD. They must mean DE because E is the second in the pair. Intel8080 manual calls this command `LXI D` which implies that it fills DE given consistency with the other 3 (`LXI B`, `LXI H`, `LXI SP`)
- `SET b, r` and `SET b, (HL)` commands seem to have an off-by-one on the examples for the 3rd bit (Note that `RES 3, (HL)` example is correct)
- `CALL cc` command example seems to have a math error, should start at instruction 0x7FFD
- `RLCA` instruction describes that bit 7 of A is copied to bit 0, but the example doesn't follow that rule. Per Intel 8080 manual, the description is correct, example is wrong
- `SRA` instruction example seems to mix up registers
