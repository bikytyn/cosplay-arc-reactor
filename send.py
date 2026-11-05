# Pošle příkazy do ESP přes sériovou linku: python send.py "n 40" "a 255 0 0"
import sys, time, serial
s = serial.Serial(); s.port = "COM9"; s.baudrate = 115200; s.timeout = 0.5
s.dtr = False; s.rts = False   # ať se ESP při otevření neresetuje
s.open(); time.sleep(0.2); s.reset_input_buffer()
for cmd in sys.argv[1:]:
    s.write((cmd + "\n").encode()); time.sleep(0.15)
    print(s.read_all().decode(errors="replace").strip())
s.close()
