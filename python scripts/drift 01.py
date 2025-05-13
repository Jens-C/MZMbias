import matplotlib.pyplot as plt
import re
data = """
phase: -119 new dac val: 1798

 phase: 754 new dac val: 1800

 phase: 814 new dac val: 1802

 phase: 242 new dac val: 1804

 phase: 944 new dac val: 1806

 phase: 607 new dac val: 1808

 phase: -1360 new dac val: 1806

 phase: 654 new dac val: 1808

 phase: -1536 new dac val: 1806

 phase: -380 new dac val: 1804

 phase: 383 new dac val: 1806

 phase: -2383 new dac val: 1804

 phase: -310 new dac val: 1802

 phase: 630 new dac val: 1804

 phase: -914 new dac val: 1802

 phase: 875 new dac val: 1804

 phase: -2110 new dac val: 1802

 phase: -130 new dac val: 1800

 phase: 910 new dac val: 1802

 phase: -2088 new dac val: 1800

 phase: 47 new dac val: 1802

 phase: -2152 new dac val: 1800

 phase: -1582 new dac val: 1798

 phase: 361 new dac val: 1800

 phase: -1687 new dac val: 1798

 phase: 260 new dac val: 1800

 phase: -2042 new dac val: 1798

 phase: -2154 new dac val: 1796

 phase: -323 new dac val: 1794

 phase: 911 new dac val: 1796

 phase: -50 new dac val: 1794

 phase: 740 new dac val: 1796

 phase: -1058 new dac val: 1794

 phase: 341 new dac val: 1796

 phase: -1441 new dac val: 1794

 phase: -581 new dac val: 1792

 phase: 713 new dac val: 1794

 phase: -1409 new dac val: 1792

 phase: 433 new dac val: 1794

 phase: -1319 new dac val: 1792

 phase: -209 new dac val: 1790

 phase: 636 new dac val: 1792

 phase: -879 new dac val: 1790

 phase: 43 new dac val: 1792

 phase: -1189 new dac val: 1790

 phase: -61 new dac val: 1788

 phase: 760 new dac val: 1790

 phase: -1226 new dac val: 1788

 phase: -1696 new dac val: 1786

 phase: -440 new dac val: 1784

 phase: -493 new dac val: 1782

 phase: 544 new dac val: 1784

 phase: 656 new dac val: 1786

 phase: 372 new dac val: 1788

 phase: -1383 new dac val: 1786

 phase: 262 new dac val: 1788

 phase: 568 new dac val: 1790

 phase: -670 new dac val: 1788

 phase: -2260 new dac val: 1786

 phase: -1852 new dac val: 1784

 phase: -1732 new dac val: 1782

 phase: -1124 new dac val: 1780

 phase: -218 new dac val: 1778

 phase: 552 new dac val: 1780

 phase: -556 new dac val: 1778

 phase: -1248 new dac val: 1776

 phase: 546 new dac val: 1778

 phase: 96 new dac val: 1780

 phase: -773 new dac val: 1778

 phase: -274 new dac val: 1776

 phase: 721 new dac val: 1778

 phase: -1922 new dac val: 1776

 phase: -987 new dac val: 1774

 phase: -354 new dac val: 1772

 phase: 802 new dac val: 1774

 phase: -1260 new dac val: 1772

 phase: 47 new dac val: 1774

 phase: -1368 new dac val: 1772

 phase: 127 new dac val: 1774

 phase: -623 new dac val: 1772

 phase: 430 new dac val: 1774

 phase: -1799 new dac val: 1772

 phase: -929 new dac val: 1770

 phase: 609 new dac val: 1772

 phase: -1094 new dac val: 1770

 phase: 425 new dac val: 1772




"""

# Extract DAC values using regex
dac_values = [int(match.group(2)) for match in re.finditer(r"phase: (-?\d+) new dac val: (\d+)", data)]
voltages = [0.180 + (dac / 1800) * (6.0 - 0.180) for dac in dac_values]
# Generate sample numbers as x-axis
num_samples = len(dac_values)
time_values = [i * (180) / (num_samples - 1) for i in range(num_samples)]

# Plotting
plt.figure(figsize=(10, 5))
plt.plot(time_values, dac_values, marker='o', linestyle='-', label='Voltage')
plt.xlabel('Time (minutes)')
plt.ylabel('Voltage (V)')
plt.title('Voltage vs Time (0 to 120 hours)')
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()