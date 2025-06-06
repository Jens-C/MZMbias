import matplotlib.pyplot as plt
import re
data = """

 phase: -1893 new dac val: 1798

 phase: -2255 new dac val: 1796

 phase: -2241 new dac val: 1794

 phase: -2286 new dac val: 1792

 phase: -2061 new dac val: 1790

 phase: -2108 new dac val: 1788

 phase: -1994 new dac val: 1786

 phase: -2206 new dac val: 1784

 phase: -1311 new dac val: 1782

 phase: -440 new dac val: 1780

 phase: 913 new dac val: 1782

 phase: -646 new dac val: 1780

 phase: 1429 new dac val: 1782

 phase: 365 new dac val: 1784

 phase: -1690 new dac val: 1782

 phase: 5 new dac val: 1784

 phase: -1387 new dac val: 1782




"""

# Extract DAC values using regex
dac_values = [int(match.group(2)) for match in re.finditer(r"phase: (-?\d+) new dac val: (\d+)", data)]
voltages = [0.180 + (dac / 1800) * (6.0 - 0.180) for dac in dac_values]
# Generate sample numbers as x-axis
num_samples = len(dac_values)
time_values = [i * (180) / (num_samples - 1) for i in range(num_samples)]

# Plotting
plt.figure(figsize=(10, 5))
plt.plot(time_values, voltages, marker='o', linestyle='-', label='Voltage')
plt.xlabel('Time (minutes)')
plt.ylabel('Voltage (V)')
plt.title('Voltage vs Time (0 to 120 hours)')
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()