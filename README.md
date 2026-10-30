<div align="center">

# The Thermometer
### *An analog temperature & dew point gauge — part of the AQI Board*

<img src="https://RolandThsive.github.io/FeaturedImages/1688470081229896581.jpg" alt="The Thermometer (AQI Board)" width="720"/>

[![Blog](https://img.shields.io/badge/Build%20Guide-ro--h--bots-FF5722?style=for-the-badge&logo=blogger&logoColor=white)](https://ro-h-bots.blogspot.com/2026/07/the-thermometer-aqi-board.html)
[![YouTube](https://img.shields.io/badge/Watch%20it%20move-YouTube-FF0000?style=for-the-badge&logo=youtube&logoColor=white)](https://www.youtube.com/watch?v=eJ1ZuOeHxdU)
[![Onshape](https://img.shields.io/badge/Open%20in-Onshape-0A6EB4?style=for-the-badge&logo=onshape&logoColor=white)](https://cad.onshape.com/documents/023499ad2580b325269edb09/w/a4a7c260365f00878d46eb10/e/65b66c7796726d7d7edb32fc?renderMode=0&uiState=6add18dae21be264acb28c49)
[![Instagram](https://img.shields.io/badge/@ro__h__bots-Instagram-E4405F?style=for-the-badge&logo=instagram&logoColor=white)](https://www.instagram.com/ro_h_bots)

![ESP32-C3](https://img.shields.io/badge/MCU-ESP32--C3-E7352C?style=flat-square&logo=espressif&logoColor=white)
![PlatformIO](https://img.shields.io/badge/Firmware-PlatformIO-F5822A?style=flat-square&logo=platformio&logoColor=white)
![KiCad](https://img.shields.io/badge/PCB-KiCad-314CB0?style=flat-square&logo=kicad&logoColor=white)
![Blender](https://img.shields.io/badge/CAD-Blender-F5792A?style=flat-square&logo=blender&logoColor=white)
[![License: CC BY-NC-SA 4.0](https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg?style=flat-square)](http://creativecommons.org/licenses/by-nc-sa/4.0/)

</div>

---

## Overview

This repository is a part of the **AQI Board** project by [@RolandThsive](https://ro-h-bots.blogspot.com/p/about-me.html) — a wall of analog-style instruments (hygrometer, thermometer, AQI meter and VOC meter) that are as much a showpiece as they are useful.

The Thermometer shows **room temperature** and **dew point** with two independent mechanical ticks, each driven by its own N20 motor and tracked by its own absolute encoder.

| Tick | Shows | Drive train |
| :--- | :--- | :--- |
| **Tick 1** — brass rod | Room temperature | N20 → crossed helical gear → two 22T spur gears → tick |
| **Tick 2** — planetary set | Dew point | N20 → crossed helical gear → helical ring gear → planetary set |

Both ticks are read **1 : 1** by AS5600 magnetic encoders on the main board, so the gauge always knows exactly where its needles are. Gear visibility and a symmetrical face were the top design priorities.

---

## Features

- **Absolute positioning** on both ticks with AS5600 magnetic encoders
- **Planetary gear set** as the visual centrepiece of the face
- **Custom main board** built around a Seeed Studio XIAO ESP32-C3
- **PCA9546A I²C multiplexer** to talk to both encoders and the SHT40 sensor
- **DRV8833** dual motor driver, with reverse-polarity and ESD protection on the connectors
- **USART link** (115200 baud) so the gauge can be polled and commanded by a master board
- **125 mm glass** front and a two-part 3D-printed housing with embedded nuts

---

## Gallery

<div align="center">

| Full Assembly | Both Ticks Engaged | Main Board |
| :---: | :---: | :---: |
| <img src="CAD/GLB-GLTF%20FIles/Whole%20Assembly%20Animated/Model%20Viewer%20-%20Whole%20Assembly%20Animated/WholeAssembly_AnimatedCompressed.png" width="250"/> | <img src="CAD/GLB-GLTF%20FIles/Mechanism%20-%20Both%20Ticks%20Engaged/Model%20Viewer%20-%20Mechanism_Both_Ticks_Engaged/Mechanism_Both_Ticks_Engaged.png" width="250"/> | <img src="CAD/GLB-GLTF%20FIles/TheMainBoard_Thermometer/Model%20Viewer%20-%20TheMainBoard_Thermometer/TheMainBoard_Thermometer.png" width="250"/> |
| **Top Assembly** | **Bottom Assembly** | **Top Housing** |
| <img src="CAD/GLB-GLTF%20FIles/Top%20Assembly/Model%20Viewer%20-%20Top_Assembly/Top_Assembly.png" width="250"/> | <img src="CAD/GLB-GLTF%20FIles/Bottom%20Assembly/Model%20Viewer%20-%20Bottom%20Assembly/Bottom_Assembly.png" width="250"/> | <img src="CAD/GLB-GLTF%20FIles/Top%20Housing/Model%20Viewer%20-%20Top_Housing/Top_Housing.png" width="250"/> |

*Want to spin these around? The blog post has interactive 3D viewers for every assembly.*

</div>

---

## Instructions

For complete printing and assembly instructions, scroll all the way down to the **'Manufacturing'** section on the official blog post:
[The Thermometer (AQI Board) Guide](https://ro-h-bots.blogspot.com/2026/07/the-thermometer-aqi-board.html)

> [!TIP]
> Every part prints normally **except the top housing**, which has nuts embedded mid-print. The layer they sit on must be perfectly flat — use the [Sharpie technique](https://www.youtube.com/shorts/kZ-3rjznSyk) on the supports.

---

## Bill of Materials (BOM)

| Quantity | Item Description |
| :---: | :--- |
| **8x** | M2 x 16 mm Hex Head Allen Type Screws |
| **8x** | M2 Nut |
| **2x** | N20 motor |
| **1x** | Set of 3D printed parts *(Found in the `CAD/Printables` folder)* |
| **1x** | Fully Assembled Circuit *(Found in the `Electronics` folder)*|
| **1x** | 2mm Brass rod *(Cut to size)* |
| **6x** | 15mm x 5mm N52 Neodymium magnets |
| **1x** | 120mm x 120mm PC fan |
| **1x** | 0.6mm PVC mesh *(Cut to size)* |
| **1x** | 1 set of 5-pin pogo pin connector |
| **1x** | SKF 688 Bearing |
| **2x** | NMB MR74 Bearing |
| **2x** | NMB MR63 Bearing |
| **1x** | NMB MR126 Bearing |
| **4x** | SKF 696 Bearing |

---

## Electronics

| Component | Role |
| :--- | :--- |
| Seeed Studio XIAO ESP32-C3 | Main controller |
| SHT40 | Temperature & humidity sensor (dew point is calculated from these) |
| AS5600 ×2 | Absolute magnetic encoders, one per tick |
| PCA9546A | 4-channel I²C multiplexer |
| DRV8833 | Dual H-bridge motor driver |
| DMP3007LK3 | P-MOSFET for reverse-polarity protection |
| USBLC6-2 | ESD protection |

Schematic PDF: [`Electronics/TheMainBoard_Schematic.pdf`](Electronics/TheMainBoard_Schematic.pdf) · KiCad project, custom symbol/footprint libraries, Gerbers and datasheets are all in [`Electronics/`](Electronics).

---

## Firmware

Built with **PlatformIO** on the Arduino framework for `seeed_xiao_esp32c3`.

```bash
cd Firmware/temperature_level_device
pio run -t upload
pio device monitor -b 115200
```

| Command (over USART) | Response |
| :---: | :--- |
| `t` | Reads the SHT40 and returns temperature and dew point as binary floats |
| `w` | Moves the ticks to the latest temperature and dew point |

Between commands the gauge refreshes itself on its own, so it also works standalone.

`Firmware/Testing Programs/` holds the bring-up sketches used along the way: motor silence tests (plain and PID), USART master communication, and a PCA9546A device scanner.

---

## Repository Structure

```
Thermometer/
├── CAD/
│   ├── Blender Projects/        # Source .blend files for every assembly
│   ├── GLB-GLTF FIles/          # Web-ready 3D models + model-viewer pages
│   ├── Printables/              # STL files to print
│   └── Step Files/              # STEP assemblies (with and without electronics)
├── Electronics/
│   ├── The Main Board/          # KiCad project + Gerber files
│   ├── Libraries/               # Custom KiCad symbols, footprints & 3D models
│   ├── Documentations/          # Component datasheets
│   └── TheMainBoard_Schematic.pdf
├── Firmware/
│   ├── temperature_level_device/     # Main firmware
│   ├── temperature_level_device_2/   # Alternate firmware revision
│   └── Testing Programs/             # Bring-up & test sketches
├── LICENSE.txt
└── README.md
```

---

## The AQI Board Family

| Project | Description |
| :--- | :--- |
| [**Hygrometer**](https://github.com/RolandThsive/Hygrometer) | Analog humidity gauge with a globoid worm drive |
| **Thermometer** *(you are here)* | Temperature & dew point gauge with a planetary gear set |
| AQI Meter · VOC Meter | Coming soon |

---

## Connect With Me
* **Blog:** [ro-h-bots.blogspot.com](https://ro-h-bots.blogspot.com/)
* **Insta:** [@ro_h_bots](https://www.instagram.com/ro_h_bots?igsh=cDNkcnNpYnB5aWR1)

---

## License

[![CC BY-NC-SA 4.0][cc-by-nc-sa-shield]][cc-by-nc-sa]

This work is licensed under a [Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License][cc-by-nc-sa].

[![CC BY-NC-SA 4.0][cc-by-nc-sa-image]][cc-by-nc-sa]

[cc-by-nc-sa]: http://creativecommons.org/licenses/by-nc-sa/4.0/
[cc-by-nc-sa-image]: https://licensebuttons.net/l/by-nc-sa/4.0/88x31.png
[cc-by-nc-sa-shield]: https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg

<div align="center">

Made with love by **RolandThsive ([@RolandThsive](https://github.com/RolandThsive))**

</div>
