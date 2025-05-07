import re
import matplotlib.pyplot as plt

# Input data as a multiline string
data = '''


dac: 1500 phase shift: 818



dac: 1520 phase shift: 355



dac: 1540 phase shift: -696



dac: 1560 phase shift: -1619



dac: 1580 phase shift: -1538



dac: 1600 phase shift: 243



dac: 1620 phase shift: 5



dac: 1640 phase shift: 616



dac: 1660 phase shift: 1073



dac: 1680 phase shift: 827



dac: 1700 phase shift: 407



dac: 1720 phase shift: 583



dac: 1740 phase shift: -309



dac: 1760 phase shift: 245



dac: 1780 phase shift: 988



dac: 1800 phase shift: 884



dac: 1820 phase shift: 857



dac: 1840 phase shift: 155



dac: 1860 phase shift: -2282



dac: 1880 phase shift: -2389



dac: 1900 phase shift: -2373



dac: 1920 phase shift: -2409



dac: 1940 phase shift: -2363



dac: 1960 phase shift: 617



dac: 1980 phase shift: 902



dac: 2000 phase shift: 959

 
'''


dac_values = []
phase_shifts = []
#0 is 0.180
#1800 is 6v
# Use regular expressions to extract DAC and phase shift values
for match in re.finditer(r'dac:\s*(-?\d+)\s+phase shift:\s*(-?\d+)', data):
    dac = int(match.group(1))
    phase = int(match.group(2))
    dac_values.append(dac)
    phase_shifts.append(phase)

# Convert DAC values to voltage
# Mapping: DAC = 0 -> 0.180 V, DAC = 1800 -> 6 V
voltages = [0.180 + (dac / 1800) * (6.0 - 0.180) for dac in dac_values]

# Plotting
plt.figure(figsize=(8, 6))
plt.plot(voltages, phase_shifts,  marker='o')
plt.xlabel('Phase Shift')
plt.ylabel('Voltage (V)')
plt.title('Voltage vs. Phase Shift')
plt.grid(True)
plt.tight_layout()
plt.show()