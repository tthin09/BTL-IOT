## Introduction
This is an IOT project for Smart Waste Sorting, named EcoSort. The product uses an ESP32 with a camera to detect waste on the conveyor belt and send it to an AI model. The AI model will classify and return waste type, then servo will push it to it's corresponding bin. Result will be saved and display on CoreIoT for visualization, and it will help on updating device's firmware.

## Installation


## How to run project
First, you Upload the code in /src folder to ESP32. Then you open a new Terminal, cd to /gateway folder, and then you run main.py.
```bash
cd gateway
python main.py
```
