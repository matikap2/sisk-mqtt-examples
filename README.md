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

After that, use following commands (probably work ONLY on OS X, maybe also on LINUX - sorry):

```bash
cd cpp_example/

# Build application using CMake - trust me
cmake -Bbuild/
cmake --build build/

# Start program
./build/mqtt_example_client
```
