import re
import matplotlib.pyplot as plt

# Input data as a multiline string
data = '''

 
 dac: 0 phase shift: 684

 dac: 10 phase shift: -1177

 dac: 20 phase shift: -1950

 dac: 30 phase shift: -2603

 dac: 40 phase shift: -2644

 dac: 50 phase shift: -2708

 dac: 60 phase shift: -2240

 dac: 70 phase shift: -536

 dac: 80 phase shift: 573

 dac: 90 phase shift: 659

 dac: 100 phase shift: 784

 dac: 110 phase shift: 709

 dac: 120 phase shift: 753

 dac: 130 phase shift: 730

 dac: 140 phase shift: 773

 dac: 150 phase shift: 692

 dac: 160 phase shift: -202

 dac: 170 phase shift: 148

 dac: 180 phase shift: -246

 dac: 190 phase shift: -599

 dac: 200 phase shift: 327

 dac: 210 phase shift: 1025

 dac: 220 phase shift: -512

 dac: 230 phase shift: 951

 dac: 240 phase shift: 599

 dac: 250 phase shift: 753

 dac: 260 phase shift: 701

 dac: 270 phase shift: 710

 dac: 280 phase shift: 957

 dac: 290 phase shift: 421

 dac: 300 phase shift: 749

 dac: 310 phase shift: 836

 dac: 320 phase shift: 275

 dac: 330 phase shift: -282

 dac: 340 phase shift: -990

 dac: 350 phase shift: -1219

 dac: 360 phase shift: -434

 dac: 370 phase shift: -1506

 dac: 380 phase shift: -664

 dac: 390 phase shift: -97

 dac: 400 phase shift: -543

 dac: 410 phase shift: -323

 dac: 420 phase shift: -58

 dac: 430 phase shift: 558

 dac: 440 phase shift: 804

 dac: 450 phase shift: 1008

 dac: 460 phase shift: 650

 dac: 470 phase shift: 676

 dac: 480 phase shift: 1071

 dac: 490 phase shift: 453

 dac: 500 phase shift: -147

 dac: 510 phase shift: 924

 dac: 520 phase shift: 428

 dac: 530 phase shift: 776

 dac: 540 phase shift: 827

 dac: 550 phase shift: 872

 dac: 560 phase shift: 830

 dac: 570 phase shift: 608

 dac: 580 phase shift: 360

 dac: 590 phase shift: 1056

 dac: 600 phase shift: 680

 dac: 610 phase shift: 745

 dac: 620 phase shift: 548

 dac: 630 phase shift: 965

 dac: 640 phase shift: 579

 dac: 650 phase shift: 920

 dac: 660 phase shift: 814

 dac: 670 phase shift: 799

 dac: 680 phase shift: 861

 dac: 690 phase shift: 782

 dac: 700 phase shift: 933

 dac: 710 phase shift: 801

 dac: 720 phase shift: 859

 dac: 730 phase shift: 720

 dac: 740 phase shift: 798

 dac: 750 phase shift: 694

 dac: 760 phase shift: 638

 dac: 770 phase shift: 761

 dac: 780 phase shift: 623

 dac: 790 phase shift: 771

 dac: 800 phase shift: 811

 dac: 810 phase shift: 702

 dac: 820 phase shift: 816

 dac: 830 phase shift: 775

 dac: 840 phase shift: 721

 dac: 850 phase shift: 621

 dac: 860 phase shift: 707

 dac: 870 phase shift: 648

 dac: 880 phase shift: 543

 dac: 890 phase shift: 428

 dac: 900 phase shift: 350

 dac: 910 phase shift: 146

 dac: 920 phase shift: 804

 dac: 930 phase shift: 564

 dac: 940 phase shift: 520

 dac: 950 phase shift: 573

 dac: 960 phase shift: -88

 dac: 970 phase shift: 1120

 dac: 980 phase shift: 1134

 dac: 990 phase shift: 618

 dac: 1000 phase shift: 948

 dac: 1010 phase shift: 1201

 dac: 1020 phase shift: -167

 dac: 1030 phase shift: 1176

 dac: 1040 phase shift: 234

 dac: 1050 phase shift: 739

 dac: 1060 phase shift: 879

 dac: 1070 phase shift: 66

 dac: 1080 phase shift: 879

 dac: 1090 phase shift: 881

 dac: 1100 phase shift: 704

 dac: 1110 phase shift: 302

 dac: 1120 phase shift: 1029

 dac: 1130 phase shift: 841

 dac: 1140 phase shift: 831

 dac: 1150 phase shift: 1012

 dac: 1160 phase shift: 1089

 dac: 1170 phase shift: 1001

 dac: 1180 phase shift: 995

 dac: 1190 phase shift: 929

 dac: 1200 phase shift: 1083

 dac: 1210 phase shift: 1058

 dac: 1220 phase shift: 1046

 dac: 1230 phase shift: 898

 dac: 1240 phase shift: 911

 dac: 1250 phase shift: 1008

 dac: 1260 phase shift: 918

 dac: 1270 phase shift: 797

 dac: 1280 phase shift: 952

 dac: 1290 phase shift: 818

 dac: 1300 phase shift: 859

 dac: 1310 phase shift: 964

 dac: 1320 phase shift: 1252

 dac: 1330 phase shift: 471

 dac: 1340 phase shift: 658

 dac: 1350 phase shift: 709

 dac: 1360 phase shift: 364

 dac: 1370 phase shift: 795

 dac: 1380 phase shift: 696

 dac: 1390 phase shift: 952

 dac: 1400 phase shift: 910

 dac: 1410 phase shift: 889

 dac: 1420 phase shift: 845

 dac: 1430 phase shift: 799

 dac: 1440 phase shift: 697

 dac: 1450 phase shift: 881

 dac: 1460 phase shift: 730

 dac: 1470 phase shift: 691

 dac: 1480 phase shift: 1045

 dac: 1490 phase shift: 1108

 dac: 1500 phase shift: 745

 dac: 1510 phase shift: 584

 dac: 1520 phase shift: 501

 dac: 1530 phase shift: 1299

 dac: 1540 phase shift: 568

 dac: 1550 phase shift: 613

 dac: 1560 phase shift: 935

 dac: 1570 phase shift: 1259

 dac: 1580 phase shift: -48

 dac: 1590 phase shift: -1256

 dac: 1600 phase shift: -921

 dac: 1610 phase shift: -313

 dac: 1620 phase shift: -1471

 dac: 1630 phase shift: -1071

 dac: 1640 phase shift: -714

 dac: 1650 phase shift: -1456

 dac: 1660 phase shift: -717

 dac: 1670 phase shift: -200

 dac: 1680 phase shift: 188

 dac: 1690 phase shift: 1209

 dac: 1700 phase shift: 389

 dac: 1710 phase shift: 851

 dac: 1720 phase shift: 1096

 dac: 1730 phase shift: 886

 dac: 1740 phase shift: 689

 dac: 1750 phase shift: 619

 dac: 1760 phase shift: 1332

 dac: 1770 phase shift: 362

 dac: 1780 phase shift: -43

 dac: 1790 phase shift: 410

 dac: 1800 phase shift: 294

 dac: 1810 phase shift: -517

 dac: 1820 phase shift: 388

 dac: 1830 phase shift: 1170

 dac: 1840 phase shift: 863

 dac: 1850 phase shift: 853

 dac: 1860 phase shift: 845

 dac: 1870 phase shift: 925

 dac: 1880 phase shift: 819

 dac: 1890 phase shift: 106

 dac: 1900 phase shift: -2428

 dac: 1910 phase shift: -2585

 dac: 1920 phase shift: -2609

 dac: 1930 phase shift: -2531

 dac: 1940 phase shift: -2613

 dac: 1950 phase shift: -2578

 dac: 1960 phase shift: -2566

 dac: 1970 phase shift: -2540

 dac: 1980 phase shift: -2605

 dac: 1990 phase shift: -2525

 dac: 2000 phase shift: -2425

 dac: 2010 phase shift: 77

 dac: 2020 phase shift: 767

 dac: 2030 phase shift: 863

 dac: 2040 phase shift: 832

 
 dac: 2040 phase shift: 1021

 dac: 2060 phase shift: 1072

 dac: 2080 phase shift: -1024

 dac: 2100 phase shift: -2246

 dac: 2120 phase shift: -2105

 dac: 2140 phase shift: -1956

 dac: 2160 phase shift: -2713

 dac: 2180 phase shift: -2214

 dac: 2200 phase shift: -2417

 dac: 2220 phase shift: -2932

 dac: 2240 phase shift: -2816

 dac: 2260 phase shift: -2767

 dac: 2280 phase shift: -1969

 dac: 2300 phase shift: -2771

 dac: 2320 phase shift: -2919

 dac: 2340 phase shift: -2899

 dac: 2360 phase shift: -2623

 dac: 2380 phase shift: -3149

 dac: 2400 phase shift: -2748

 dac: 2420 phase shift: -1926

 dac: 2440 phase shift: -2739

 dac: 2460 phase shift: -2639

 dac: 2480 phase shift: -1906

 dac: 2500 phase shift: -2628

 dac: 2520 phase shift: -2577

 dac: 2540 phase shift: -2945

 dac: 2560 phase shift: -2968

 dac: 2580 phase shift: -2884

 dac: 2600 phase shift: -2762

 dac: 2620 phase shift: -3101

 dac: 2640 phase shift: -3039

 dac: 2660 phase shift: -1896

 dac: 2680 phase shift: -2539

 dac: 2700 phase shift: -2478

 dac: 2720 phase shift: -2769

 dac: 2740 phase shift: -2902

 dac: 2760 phase shift: -2775

 dac: 2780 phase shift: -2776

 dac: 2800 phase shift: -1955

 dac: 2820 phase shift: -2700

 dac: 2840 phase shift: -1650

 dac: 2860 phase shift: -2818

 dac: 2880 phase shift: -2927

 dac: 2900 phase shift: -2707

 dac: 2920 phase shift: -2944

 dac: 2940 phase shift: -2569

 dac: 2960 phase shift: -2590

 dac: 2980 phase shift: -1376

 dac: 3000 phase shift: -1323

 dac: 3020 phase shift: -1791

 dac: 3040 phase shift: -2554

 dac: 3060 phase shift: -1711

 dac: 3080 phase shift: -1405

 dac: 3100 phase shift: -542

 dac: 3120 phase shift: 568

 dac: 3140 phase shift: 836

 dac: 3160 phase shift: 950

 dac: 3180 phase shift: -1958

 dac: 3200 phase shift: -2736

 dac: 3220 phase shift: -2871

 dac: 3240 phase shift: -2927

 dac: 3260 phase shift: -3042

 dac: 3280 phase shift: -960

 dac: 3300 phase shift: 530

 dac: 3320 phase shift: 883

 dac: 3340 phase shift: 822

 dac: 3360 phase shift: 169

 dac: 3380 phase shift: 1045

 dac: 3400 phase shift: 321

 dac: 3420 phase shift: 294

 dac: 3440 phase shift: 1065

 dac: 3460 phase shift: 975

 dac: 3480 phase shift: 2150

 dac: 3500 phase shift: 865

 dac: 3520 phase shift: -890

 dac: 3540 phase shift: -1830

 dac: 3560 phase shift: -1292

 dac: 3580 phase shift: 756

 dac: 3600 phase shift: 670

 dac: 3620 phase shift: 1230

 dac: 3640 phase shift: 1480

 dac: 3660 phase shift: 1169

 dac: 3680 phase shift: 1516

 dac: 3700 phase shift: 1380

 dac: 3720 phase shift: 948

 dac: 3740 phase shift: 708

 dac: 3760 phase shift: 1492

 dac: 3780 phase shift: 262

 dac: 3800 phase shift: 1011



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