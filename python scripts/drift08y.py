import matplotlib.pyplot as plt
import re
data = """
phase: -2331 new dac val: 1798

 phase: -2350 new dac val: 1796

 phase: -2265 new dac val: 1794

 phase: -2352 new dac val: 1792

 phase: -2306 new dac val: 1790

 phase: -2253 new dac val: 1788

 phase: -2314 new dac val: 1786

 phase: -2327 new dac val: 1784

 phase: -2356 new dac val: 1782

 phase: -2163 new dac val: 1780

 phase: -2356 new dac val: 1778

 phase: -2411 new dac val: 1776

 phase: -2386 new dac val: 1774

 phase: -2296 new dac val: 1772

 phase: -2282 new dac val: 1770

 phase: -2271 new dac val: 1768

 phase: -2381 new dac val: 1766

 phase: -2236 new dac val: 1764

 phase: -2352 new dac val: 1762

 phase: -1579 new dac val: 1760

 phase: -686 new dac val: 1758

 phase: -337 new dac val: 1756

 phase: 916 new dac val: 1758

 phase: -823 new dac val: 1756

 phase: 868 new dac val: 1758

 phase: -2116 new dac val: 1756

 phase: 549 new dac val: 1758

 phase: -1788 new dac val: 1756



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