import re
import matplotlib.pyplot as plt

# Input data as a multiline string
data = '''


dac: 0 phase shift: -2240



dac: 20 phase shift: -2254



dac: 40 phase shift: -2255



dac: 60 phase shift: -2247



dac: 80 phase shift: -2263



dac: 100 phase shift: -2262



dac: 120 phase shift: -2244



dac: 140 phase shift: -2232



dac: 160 phase shift: -2265



dac: 180 phase shift: -2220



dac: 200 phase shift: -2265



dac: 220 phase shift: -2247



dac: 240 phase shift: -342



dac: 260 phase shift: 1052



dac: 280 phase shift: 974



dac: 300 phase shift: 898



dac: 320 phase shift: 931



dac: 340 phase shift: 902



dac: 360 phase shift: 883



dac: 380 phase shift: 914



dac: 400 phase shift: 912



dac: 420 phase shift: 935



dac: 440 phase shift: 908



dac: 460 phase shift: 918



dac: 480 phase shift: 907



dac: 500 phase shift: 904



dac: 520 phase shift: 894



dac: 540 phase shift: 895



dac: 560 phase shift: 906



dac: 580 phase shift: 906



dac: 600 phase shift: 898



dac: 620 phase shift: 909



dac: 640 phase shift: 904



dac: 660 phase shift: 904



dac: 680 phase shift: 898



dac: 700 phase shift: 898



dac: 720 phase shift: 906



dac: 740 phase shift: 912



dac: 760 phase shift: 914



dac: 780 phase shift: 910



dac: 800 phase shift: 898



dac: 820 phase shift: 906



dac: 840 phase shift: 899



dac: 860 phase shift: 905



dac: 880 phase shift: 900



dac: 900 phase shift: 880



dac: 920 phase shift: 894



dac: 940 phase shift: 921



dac: 960 phase shift: 930



dac: 980 phase shift: 959



dac: 1000 phase shift: 998



dac: 1020 phase shift: 948



dac: 1040 phase shift: 911



dac: 1060 phase shift: 872



dac: 1080 phase shift: 844



dac: 1100 phase shift: 881



dac: 1120 phase shift: 895



dac: 1140 phase shift: 890



dac: 1160 phase shift: 907



dac: 1180 phase shift: 910



dac: 1200 phase shift: 897



dac: 1220 phase shift: 908



dac: 1240 phase shift: 901



dac: 1260 phase shift: 904



dac: 1280 phase shift: 898



dac: 1300 phase shift: 899



dac: 1320 phase shift: 904



dac: 1340 phase shift: 909



dac: 1360 phase shift: 901



dac: 1380 phase shift: 905



dac: 1400 phase shift: 900



dac: 1420 phase shift: 914



dac: 1440 phase shift: 905



dac: 1460 phase shift: 898



dac: 1480 phase shift: 913



dac: 1500 phase shift: 917



dac: 1520 phase shift: 902



dac: 1540 phase shift: 898



dac: 1560 phase shift: 899



dac: 1580 phase shift: 918



dac: 1600 phase shift: 913



dac: 1620 phase shift: 918



dac: 1640 phase shift: 915



dac: 1660 phase shift: 910



dac: 1680 phase shift: 895



dac: 1700 phase shift: 924



dac: 1720 phase shift: 915



dac: 1750 phase shift: 964



dac: 1752 phase shift: 938



dac: 1754 phase shift: 889



dac: 1756 phase shift: 917



dac: 1758 phase shift: 972



dac: 1760 phase shift: 871



dac: 1762 phase shift: 804



dac: 1764 phase shift: 889



dac: 1766 phase shift: 921



dac: 1768 phase shift: 837



dac: 1770 phase shift: 910



dac: 1772 phase shift: 819



dac: 1774 phase shift: 943



dac: 1776 phase shift: 1194



dac: 1778 phase shift: 1043



dac: 1780 phase shift: 1004



dac: 1782 phase shift: 932



dac: 1784 phase shift: 764



dac: 1786 phase shift: 775



dac: 1788 phase shift: 1419



dac: 1790 phase shift: 1065



dac: 1792 phase shift: 924



dac: 1794 phase shift: 1081



dac: 1796 phase shift: 1654



dac: 1798 phase shift: 602



dac: 1800 phase shift: -1751



dac: 1802 phase shift: -2685



dac: 1804 phase shift: -2480



dac: 1806 phase shift: -2412



dac: 1808 phase shift: -2271



dac: 1810 phase shift: -2323



dac: 1812 phase shift: -2357



dac: 1814 phase shift: -2301



dac: 1816 phase shift: -2077



dac: 1818 phase shift: -2083



dac: 1820 phase shift: -2393



dac: 1822 phase shift: -2292



dac: 1824 phase shift: -2304



dac: 1826 phase shift: -2343



dac: 1828 phase shift: -2418



dac: 1830 phase shift: -2343



dac: 1832 phase shift: -2302



dac: 1834 phase shift: -2294



dac: 1836 phase shift: -2371



dac: 1838 phase shift: -2384



dac: 1840 phase shift: -2306



dac: 1842 phase shift: -2396



dac: 1844 phase shift: -2349



dac: 1846 phase shift: -2410



dac: 1848 phase shift: -2333



dac: 1850 phase shift: -2319



dac: 1852 phase shift: -2311



dac: 1854 phase shift: -2348



dac: 1856 phase shift: -2311



dac: 1858 phase shift: -2240



dac: 1860 phase shift: -2300



dac: 1862 phase shift: -2299



dac: 1864 phase shift: -2272



dac: 1866 phase shift: -2220



dac: 1868 phase shift: -2328



dac: 1870 phase shift: -2324




dac: 1880 phase shift: -2263



dac: 1900 phase shift: -2237



dac: 1920 phase shift: -2253



dac: 1940 phase shift: -2233



dac: 1960 phase shift: -2249



dac: 1980 phase shift: -2225



dac: 2000 phase shift: -2254



dac: 2020 phase shift: -2250



dac: 2040 phase shift: -2226



dac: 2060 phase shift: -2243



dac: 2080 phase shift: -2228



dac: 2100 phase shift: -2231



dac: 2120 phase shift: -2234



dac: 2140 phase shift: -2231



dac: 2160 phase shift: -2241



dac: 2180 phase shift: -2249



dac: 2200 phase shift: -2228



dac: 2220 phase shift: -2233



dac: 2240 phase shift: -2219



dac: 2260 phase shift: -2240



dac: 2280 phase shift: -2224



dac: 2300 phase shift: -2237



dac: 2320 phase shift: -2235



dac: 2340 phase shift: -2239



dac: 2360 phase shift: -2229



dac: 2380 phase shift: -2229



dac: 2400 phase shift: -2235



dac: 2420 phase shift: -2231



dac: 2440 phase shift: -2232



dac: 2460 phase shift: -2232



dac: 2480 phase shift: -2237



dac: 2500 phase shift: -2234



dac: 2520 phase shift: -2232



dac: 2540 phase shift: -2230



dac: 2560 phase shift: -2225



dac: 2580 phase shift: -2233



dac: 2600 phase shift: -2223



dac: 2620 phase shift: -2229



dac: 2640 phase shift: -2233



dac: 2660 phase shift: -2243



dac: 2680 phase shift: -2235



dac: 2700 phase shift: -2237



dac: 2720 phase shift: -2232



dac: 2740 phase shift: -2230



dac: 2760 phase shift: -2227



dac: 2780 phase shift: -2217



dac: 2800 phase shift: -2232



dac: 2820 phase shift: -2226



dac: 2840 phase shift: -2224



dac: 2860 phase shift: -2231



dac: 2880 phase shift: -2233



dac: 2900 phase shift: -2233



dac: 2920 phase shift: -2228



dac: 2940 phase shift: -2228



dac: 2960 phase shift: -2227



dac: 2980 phase shift: -2231



dac: 3000 phase shift: -2233



dac: 3020 phase shift: -2221



dac: 3040 phase shift: -2226



dac: 3060 phase shift: -2234



dac: 3080 phase shift: -2230



dac: 3100 phase shift: -2234



dac: 3120 phase shift: -2232



dac: 3140 phase shift: -2228



dac: 3160 phase shift: -2222



dac: 3180 phase shift: -2230



dac: 3200 phase shift: -2220



dac: 3220 phase shift: -2229



dac: 3240 phase shift: -2225



dac: 3260 phase shift: -2234



dac: 3280 phase shift: -2246



dac: 3300 phase shift: -2229



dac: 3320 phase shift: -2235



dac: 3340 phase shift: -2269



dac: 3360 phase shift: -2238



dac: 3380 phase shift: -2255



dac: 3400 phase shift: -2257



dac: 3420 phase shift: 1387



dac: 3440 phase shift: 949



dac: 3460 phase shift: 928



dac: 3480 phase shift: 919



dac: 3500 phase shift: 925



dac: 3520 phase shift: 926



dac: 3540 phase shift: 925



dac: 3560 phase shift: 966



dac: 3580 phase shift: 940



dac: 3600 phase shift: 939



dac: 3620 phase shift: 936



dac: 3640 phase shift: 935



dac: 3660 phase shift: 930



dac: 3680 phase shift: 921



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
plt.ylabel('Phase Shift')
plt.xlabel('Voltage (V)')
plt.title('Voltage vs. Phase Shift')
plt.grid(True)
plt.tight_layout()
plt.show()