# MZM bias
This repo contains the software code for the master thesis "Adaptive Bias and Polarization Control in Mach-Zehnder
Modulators for Radio-Frequency-over-Fiber". The code is developed for the Nucleo-L476RG board and [acompanying shield](https://github.com/Jens-C/Nucleo-shield) using the stm32CubeIDE 1.18.0. All of the techinical details can be read in the master thesis itself.

# structure of the repo
```
├───phase-shift-meas     ← Code used for doing sweeps of the DC bias of the MZM used in the measurment campaign
├───Pol-controller       ← Code for only polarization control (PC)
├───pol_bias_control     ← Code for both MZM and PC
├───pol_bias_enable      ← Code for both MZM and PC but with functionality to periodicaly turn off dither
└───python scripts       ← Python scripts used for creating graphs in the measurment campaign
```
The main source code is every time under:
 .\Core\Src\main.c

