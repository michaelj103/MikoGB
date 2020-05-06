# Noted Errata in various documents

## Game Boy Programming Manual
- `LD r, r'` command lists register D code as 101, which is the same as register L code. Correct code for D is 010 per Intel 8080 manual (and common sense)
- `LD dd, nn` command lists dd code 01 as pointing to the register pair DD. They must mean DE because E is the second in the pair. Intel8080 manual calls this command `LXI D` which implies that it fills DE given consistency with the other 3 (`LXI B`, `LXI H`, `LXI SP`)
