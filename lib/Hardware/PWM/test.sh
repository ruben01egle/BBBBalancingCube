#!/bin/bash

sudo python3 -c 'import mmap, os, time; fd=os.open("/dev/mem", os.O_RDWR|os.O_SYNC); mem=mmap.mmap(fd, 4096, offset=0x48304000);
while True:
    tbprd = int.from_bytes(mem[0x20A:0x20C], "little")
    cmpa  = int.from_bytes(mem[0x212:0x214], "little")
    cmpb  = int.from_bytes(mem[0x214:0x216], "little")
    duty_b = (cmpb / tbprd * 100) if tbprd > 0 else 0
    print(f"TBPRD: {tbprd} | CMPA: {cmpa} | CMPB: {cmpb} (Duty B: {duty_b:.1f}%)  ", end="\r")
    time.sleep(0.5)'
