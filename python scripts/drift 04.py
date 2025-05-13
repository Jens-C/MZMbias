import matplotlib.pyplot as plt
import re
data = """
phase: -2055 new dac val: 1798

 phase: -2395 new dac val: 1796

 phase: -2391 new dac val: 1794

 phase: -2438 new dac val: 1792

 phase: -1309 new dac val: 1790

 phase: -2067 new dac val: 1788

 phase: -2324 new dac val: 1786

 phase: -2290 new dac val: 1784

 phase: -2303 new dac val: 1782

 phase: -2388 new dac val: 1780

 phase: -2276 new dac val: 1778

 phase: -2406 new dac val: 1776

 phase: -1298 new dac val: 1774

 phase: 390 new dac val: 1776

 phase: 51 new dac val: 1778

 phase: -2207 new dac val: 1776

 phase: -1524 new dac val: 1774

 phase: 532 new dac val: 1776

 phase: -2458 new dac val: 1774

 phase: -2239 new dac val: 1772

 phase: -2448 new dac val: 1770

 phase: -2467 new dac val: 1768

 phase: -1848 new dac val: 1766

 phase: 340 new dac val: 1768

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