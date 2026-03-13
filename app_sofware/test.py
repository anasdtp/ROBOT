def map(x, in_min, in_max, out_min, out_max):
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

print(hex(int(map(550, 0, 2000, 0x3600, 0x39F0))))   # Should print value in hex