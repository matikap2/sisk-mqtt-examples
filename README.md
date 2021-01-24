# sisk-mqtt-examples

## Python examples

Follow commands below:
```bash
cd python_example/

# Install all necessary libraries
pip3 install -r requirements.txt    

# Run first part of example - msg publisher script - temperature/humidity sensor app
python3 mqtt_example_publisher.py 

# Run second part of example - msg subscriber script - heater app
python3 mqtt_example_subscriber.py   
```

## C++ examples

Firstly, init and update git submodules from repo - `paho.mqtt.c` and `paho.mqtt.cpp` libraries and install them using CMake following the instruction from repos:
* [Paho MQTT C library](https://github.com/eclipse/paho.mqtt.c)
* [Paho MQTT C++ library](https://github.com/eclipse/paho.mqtt.cpp)

After that, use following commands (probably work ONLY on OS X, maybe also on LINUX (for Windows you should change CMake config) - sorry):

```bash
cd cpp_example/

# Build application using CMake - trust me
cmake -Bbuild/
cmake --build build/

# Start program
./build/mqtt_example_client
```

## ESP32 C examples

Install [ESP-IDF for VS Code](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html).

Open desired project from `project` folder, then open project workspace file.

Compile using option from `Command Palette/ESP-IDF: Build your project`, then flash using `ESP-IDF Flash device` option. More information on building/flashing in [ESP-IDF VS Code Extension onboarding](https://www.youtube.com/watch?v=Lc6ausiKvQM&feature=youtu.be).

Project `sisk-app-subscriber-esp32` was developed on [AiThinker ESP32 CAM](https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/) devboard.
Project `sisk-app-publisher-esp32` was developed on ESP32-DevKit (ESP-WROOM-32).
