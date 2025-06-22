import re
import matplotlib.pyplot as plt

# Input data as a multiline string
data = '''

 dac: 0 phase shift: -1177

 dac: 20 phase shift: -1405

 dac: 40 phase shift: -1445

 dac: 60 phase shift: -1450

 dac: 80 phase shift: -1458

 dac: 100 phase shift: -1454

 dac: 120 phase shift: -1454

 dac: 140 phase shift: -1457

 dac: 160 phase shift: -1457

 dac: 180 phase shift: -1461

 dac: 200 phase shift: -1461

 dac: 220 phase shift: -1455

 dac: 240 phase shift: -1465

 dac: 260 phase shift: -1462

 dac: 280 phase shift: -1474

 dac: 300 phase shift: -1472

 dac: 320 phase shift: -1463

 dac: 340 phase shift: -1474

 dac: 360 phase shift: -1467

 dac: 380 phase shift: -1473

 dac: 400 phase shift: -1468

 dac: 420 phase shift: -1475

 dac: 440 phase shift: -1458

 dac: 460 phase shift: -1461

 dac: 480 phase shift: -1466

 dac: 500 phase shift: -1453

 dac: 520 phase shift: -1462

 dac: 540 phase shift: -1474

 dac: 560 phase shift: -1474

 dac: 580 phase shift: -1483

 dac: 600 phase shift: -1470

 dac: 620 phase shift: -1479

 dac: 640 phase shift: -1476

 dac: 660 phase shift: -1491

 dac: 680 phase shift: -1486

 dac: 700 phase shift: -1468

 dac: 720 phase shift: -1480

 dac: 740 phase shift: -1487

 dac: 760 phase shift: -1502

 dac: 780 phase shift: -1533

 dac: 800 phase shift: -1530

 dac: 820 phase shift: -1453

 dac: 840 phase shift: -1519

 dac: 860 phase shift: -1524

 dac: 880 phase shift: -1562

 dac: 900 phase shift: -1644

 dac: 920 phase shift: -1763

 dac: 940 phase shift: -1871

 dac: 960 phase shift: -2022

 dac: 980 phase shift: -2368

 dac: 1000 phase shift: -1889

 dac: 1020 phase shift: -1768

 dac: 1040 phase shift: -120

 dac: 1060 phase shift: 475

 dac: 1080 phase shift: 2529

 dac: 1100 phase shift: 2944

 dac: 1120 phase shift: 2915

 dac: 1140 phase shift: 2625

 dac: 1160 phase shift: 2733

 dac: 1180 phase shift: 2696

 dac: 1200 phase shift: 2667

 dac: 1220 phase shift: 2625

 dac: 1240 phase shift: 2574

 dac: 1260 phase shift: 2579

 dac: 1280 phase shift: 2550

 dac: 1300 phase shift: 2535

 dac: 1320 phase shift: 2546

 dac: 1340 phase shift: 2571

 dac: 1360 phase shift: 2567

 dac: 1380 phase shift: 2559

 dac: 1400 phase shift: 2554

 dac: 1420 phase shift: 2529

 dac: 1440 phase shift: 2543

 dac: 1460 phase shift: 2524

 dac: 1480 phase shift: 2535

 dac: 1500 phase shift: 2537

 dac: 1520 phase shift: 2527

 dac: 1540 phase shift: 2548

 dac: 1560 phase shift: 2546

 dac: 1580 phase shift: 2540

 dac: 1600 phase shift: 2547

 dac: 1620 phase shift: 2531

 dac: 1640 phase shift: 2533

 dac: 1660 phase shift: 2533

 dac: 1680 phase shift: 2540

 dac: 1700 phase shift: 2532

 dac: 1720 phase shift: 2525

 dac: 1740 phase shift: 2517

 dac: 1760 phase shift: 2537

 dac: 1780 phase shift: 2531

 dac: 1800 phase shift: 2523

 dac: 1820 phase shift: 2541

 dac: 1840 phase shift: 2521

 dac: 1860 phase shift: 2519

 dac: 1880 phase shift: 2545

 dac: 1900 phase shift: 2529

 dac: 1920 phase shift: 2541

 dac: 1940 phase shift: 2540

 dac: 1960 phase shift: 2534

 dac: 1980 phase shift: 2532

 dac: 2000 phase shift: 2538

 dac: 2020 phase shift: 2546

 dac: 2040 phase shift: 2537

 dac: 2060 phase shift: 2430

 dac: 2080 phase shift: 2527

 dac: 2100 phase shift: 2536

 dac: 2120 phase shift: 2542

 dac: 2140 phase shift: 2549

 dac: 2160 phase shift: 2520

 dac: 2180 phase shift: 2541

 dac: 2200 phase shift: 2544

 dac: 2220 phase shift: 2535

 dac: 2240 phase shift: 2533

 dac: 2260 phase shift: 2559

 dac: 2280 phase shift: 2539

 dac: 2300 phase shift: 2558

 dac: 2320 phase shift: 2572

 dac: 2340 phase shift: 2558

 dac: 2360 phase shift: 2569

 dac: 2380 phase shift: 2559

 dac: 2400 phase shift: 2560

 dac: 2420 phase shift: 2591

 dac: 2440 phase shift: 2591

 dac: 2460 phase shift: 2602

 dac: 2480 phase shift: 2651

 dac: 2500 phase shift: 2649

 dac: 2520 phase shift: 2738

 dac: 2540 phase shift: 2818

 dac: 2560 phase shift: 2879

 dac: 2580 phase shift: 3418

 dac: 2600 phase shift: -1597

 dac: 2620 phase shift: -1849

 dac: 2640 phase shift: -1821

 dac: 2660 phase shift: -1721

 dac: 2680 phase shift: -1519

 dac: 2700 phase shift: -1544

 dac: 2720 phase shift: -1526

 dac: 2740 phase shift: -1587

 dac: 2760 phase shift: -1558

 dac: 2780 phase shift: -1523

 dac: 2800 phase shift: -1504

 dac: 2820 phase shift: -1469

 dac: 2840 phase shift: -1464

 dac: 2860 phase shift: -1468

 dac: 2880 phase shift: -1498

 dac: 2900 phase shift: -1463

 dac: 2920 phase shift: -1467

 dac: 2940 phase shift: -1494

 dac: 2960 phase shift: -1487

 dac: 2980 phase shift: -1484

 dac: 3000 phase shift: -1488

 dac: 3020 phase shift: -1475

 dac: 3040 phase shift: -1480

 dac: 3060 phase shift: -1463

 dac: 3080 phase shift: -1473

 dac: 3100 phase shift: -1458

 dac: 3120 phase shift: -1466

 dac: 3140 phase shift: -1458

 dac: 3160 phase shift: -1465

 dac: 3180 phase shift: -1462

 dac: 3200 phase shift: -1439

 dac: 3220 phase shift: -1462

 dac: 3240 phase shift: -1478

 dac: 3260 phase shift: -1456

 dac: 3280 phase shift: -1461

 dac: 3300 phase shift: -1472

 dac: 3320 phase shift: -1450

 dac: 3340 phase shift: -1460

 dac: 3360 phase shift: -1451

 dac: 3380 phase shift: -1448

 dac: 3400 phase shift: -1445

 dac: 3420 phase shift: -1451

 dac: 3440 phase shift: -1456

 dac: 3460 phase shift: -1450

 dac: 3480 phase shift: -1439

 dac: 3500 phase shift: -1447

 dac: 3520 phase shift: -1452

 dac: 3540 phase shift: -1454

 dac: 3560 phase shift: -1454

 dac: 3580 phase shift: -1458

 dac: 3600 phase shift: -1481

 dac: 3620 phase shift: -1455

 dac: 3640 phase shift: -1450

 dac: 3660 phase shift: -1456

 dac: 3680 phase shift: -1477

 dac: 3700 phase shift: -1463

 dac: 3720 phase shift: -1448

 dac: 3740 phase shift: -1464

 dac: 3760 phase shift: -1453

 dac: 3780 phase shift: -1460

 dac: 3800 phase shift: -1460

 dac: 3820 phase shift: -1456

 dac: 3840 phase shift: -1453




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
plt.plot(dac_values, phase_shifts,  marker='o')
plt.xlabel('Phase Shift')
plt.ylabel('Voltage (V)')
plt.title('Voltage vs. Phase Shift')
plt.grid(True)
plt.tight_layout()
plt.show()