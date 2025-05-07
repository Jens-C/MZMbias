import re
import matplotlib.pyplot as plt

# Input data as a multiline string
data = '''

 dac: 0 phase shift: 3113

 dac: 100 phase shift: 3139

 dac: 200 phase shift: -3108

 dac: 300 phase shift: -2803

 dac: 400 phase shift: -3075

 dac: 500 phase shift: 2949

 dac: 600 phase shift: 2825

 dac: 700 phase shift: -3119

 dac: 800 phase shift: -3074

 dac: 900 phase shift: -3087

 dac: 1000 phase shift: 2991

 dac: 1100 phase shift: 3015
'''

# Use regular expressions to extract DAC and phase shift values
dac_values = []
phase_shifts = []

for match in re.finditer(r'dac:\s*(-?\d+)\s+phase shift:\s*(-?\d+)', data):
    dac = int(match.group(1))
    phase = int(match.group(2))
    dac_values.append(dac)
    phase_shifts.append(phase)

# Plotting
plt.figure(figsize=(8, 6))
plt.plot(dac_values, phase_shifts, marker='o')
plt.ylabel('Phase Shift')
plt.xlabel('DAC Value')
plt.title('DAC Value vs. Phase Shift')
plt.grid(True)
plt.tight_layout()
plt.show()