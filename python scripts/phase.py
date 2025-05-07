import re
import matplotlib.pyplot as plt

# Input data as a multiline string
data = '''

 dac: 0 phase shift: 1407

 dac: 5 phase shift: 1711

 dac: 10 phase shift: 1529

 dac: 15 phase shift: 1836

 dac: 20 phase shift: 1746

 dac: 25 phase shift: 1807

 dac: 30 phase shift: 2013

 dac: 35 phase shift: 1795

 dac: 40 phase shift: 2032

 dac: 45 phase shift: 1897

 dac: 50 phase shift: 1880

 dac: 55 phase shift: 1930

 dac: 60 phase shift: 1432

 dac: 65 phase shift: 1050

 dac: 70 phase shift: -129

 dac: 75 phase shift: -1118

 dac: 80 phase shift: -1178

 dac: 85 phase shift: -1145

 dac: 90 phase shift: -1109

 dac: 95 phase shift: -1170

 dac: 100 phase shift: -1195

 dac: 105 phase shift: -1158

 dac: 110 phase shift: -1167

 dac: 115 phase shift: -1165

 dac: 120 phase shift: -1176

 dac: 125 phase shift: -1180

 dac: 130 phase shift: -1190

 dac: 135 phase shift: -1168

 dac: 140 phase shift: -1165

 dac: 145 phase shift: -1197

 dac: 150 phase shift: -1197

 dac: 155 phase shift: -1216

 dac: 160 phase shift: -1215

 dac: 165 phase shift: -1191

 dac: 170 phase shift: -1206

 dac: 175 phase shift: -1002

 dac: 180 phase shift: -798

 dac: 185 phase shift: -340

 dac: 190 phase shift: 628

 dac: 195 phase shift: 2113

 dac: 200 phase shift: 1904

 dac: 205 phase shift: 1957

 dac: 210 phase shift: 1931

 dac: 215 phase shift: 1898

 dac: 220 phase shift: 1907

 dac: 225 phase shift: 1920

 dac: 230 phase shift: 1943

 dac: 235 phase shift: 1917

 dac: 240 phase shift: 1986

 dac: 245 phase shift: 1879

 dac: 260 phase shift: 2005

 dac: 280 phase shift: 1978

 dac: 300 phase shift: 1888

 dac: 320 phase shift: 2117

 dac: 340 phase shift: 2128

 dac: 360 phase shift: 2109

 dac: 380 phase shift: 2149

 dac: 400 phase shift: 1900

 dac: 420 phase shift: 1794

 dac: 440 phase shift: 789

 dac: 460 phase shift: 639

 dac: 480 phase shift: 346

 dac: 500 phase shift: 1092

 dac: 520 phase shift: 1595

 dac: 540 phase shift: 2268

 dac: 560 phase shift: 1929

 dac: 580 phase shift: 2137

 dac: 600 phase shift: 2044

 dac: 620 phase shift: 2242

 dac: 640 phase shift: 2149

 dac: 660 phase shift: 2198

 dac: 680 phase shift: 1971

 dac: 700 phase shift: 2143

 dac: 720 phase shift: 2285

 dac: 740 phase shift: 2181

 dac: 760 phase shift: 2240

 dac: 780 phase shift: 2248

 dac: 800 phase shift: 2120

 dac: 820 phase shift: 2163

 dac: 840 phase shift: 1998

 dac: 860 phase shift: 2034

 dac: 880 phase shift: 1964

 dac: 900 phase shift: 1972

 dac: 920 phase shift: 2138

 dac: 940 phase shift: 1892

 dac: 960 phase shift: 1968

 dac: 980 phase shift: 1756

 dac: 1000 phase shift: 2176

 
 dac: 1000 phase shift: 1810

 dac: 1005 phase shift: 1598

 dac: 1010 phase shift: 1697

 dac: 1015 phase shift: 1786

 dac: 1020 phase shift: 1862

 dac: 1025 phase shift: 2126

 dac: 1030 phase shift: 2253

 dac: 1035 phase shift: 1704

 dac: 1040 phase shift: 2605

 dac: 1045 phase shift: 1282

 dac: 1050 phase shift: 2550

 dac: 1055 phase shift: 2552

 dac: 1060 phase shift: 2203

 dac: 1065 phase shift: 2555

 dac: 1070 phase shift: 1254

 dac: 1075 phase shift: 1189

 dac: 1080 phase shift: 1624

 dac: 1085 phase shift: 911

 dac: 1090 phase shift: 325

 dac: 1095 phase shift: -8

 dac: 1100 phase shift: 141

 dac: 1105 phase shift: -871

 dac: 1110 phase shift: -905

 dac: 1115 phase shift: -797

 dac: 1120 phase shift: -917

 dac: 1125 phase shift: -1043

 dac: 1130 phase shift: -1711

 dac: 1135 phase shift: -1462

 dac: 1140 phase shift: -1498

 dac: 1145 phase shift: -2240

 dac: 1150 phase shift: -1290

 dac: 1155 phase shift: -1178

 dac: 1160 phase shift: -1255

 dac: 1165 phase shift: -1390

 dac: 1170 phase shift: -1462

 dac: 1175 phase shift: -1531

 dac: 1180 phase shift: -1551

 dac: 1185 phase shift: -1503

 dac: 1190 phase shift: -1260

 dac: 1195 phase shift: -1246

 dac: 1220 phase shift: -879

 dac: 1240 phase shift: -1076

 dac: 1260 phase shift: -1069

 dac: 1280 phase shift: -1053

 dac: 1300 phase shift: -1111

 dac: 1320 phase shift: -1143

 dac: 1340 phase shift: -1077

 dac: 1360 phase shift: -1162

 dac: 1380 phase shift: -974

 dac: 1400 phase shift: -1017

 dac: 1420 phase shift: -975

 dac: 1440 phase shift: -981

 dac: 1460 phase shift: -987

 dac: 1480 phase shift: -1058

 dac: 1500 phase shift: -968

 dac: 1520 phase shift: -1002

 dac: 1540 phase shift: -1290

 dac: 1560 phase shift: -1113

 dac: 1580 phase shift: -1135

 dac: 1600 phase shift: -1204

 dac: 1605 phase shift: -1173

 dac: 1610 phase shift: -1534

 dac: 1615 phase shift: -1243

 dac: 1620 phase shift: -1350

 dac: 1625 phase shift: -1106

 dac: 1630 phase shift: -1193

 dac: 1635 phase shift: -1398

 dac: 1640 phase shift: -1389

 dac: 1645 phase shift: -1621

 dac: 1650 phase shift: -1645

 dac: 1655 phase shift: -1879

 dac: 1660 phase shift: -2295

 dac: 1665 phase shift: -2343

 dac: 1670 phase shift: -2870

 dac: 1675 phase shift: -2496

 dac: 1680 phase shift: -3724

 dac: 1685 phase shift: -3071

 dac: 1690 phase shift: -3021

 dac: 1695 phase shift: -3636

 dac: 1700 phase shift: -3565

 dac: 1705 phase shift: -3089

 dac: 1710 phase shift: -2235

 dac: 1715 phase shift: -2654

 dac: 1720 phase shift: -2421

 dac: 1725 phase shift: -2491

 dac: 1730 phase shift: -1991

 dac: 1735 phase shift: -1556

 dac: 1740 phase shift: -1783

 dac: 1745 phase shift: -931

 dac: 1750 phase shift: -1165

 dac: 1755 phase shift: -1390

 dac: 1760 phase shift: -1441

 dac: 1765 phase shift: -1155

 dac: 1770 phase shift: -1063

 dac: 1775 phase shift: -1219

 dac: 1780 phase shift: -1071

 dac: 1785 phase shift: -1410

 dac: 1790 phase shift: -1364

 dac: 1795 phase shift: -1287

 dac: 1800 phase shift: -1127

 dac: 1820 phase shift: -992

 dac: 1840 phase shift: -1324

 dac: 1860 phase shift: -1319

 dac: 1880 phase shift: -987

 dac: 1900 phase shift: -1100

 dac: 1920 phase shift: -1120

 dac: 1940 phase shift: -1564

 dac: 1960 phase shift: -4330

 dac: 1980 phase shift: -4339

 dac: 2000 phase shift: -4340

 dac: 2020 phase shift: -4316

 dac: 2040 phase shift: -4359


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