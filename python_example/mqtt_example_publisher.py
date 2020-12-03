import paho.mqtt.client as mqtt
import kpn_senml as senml
import time
import random

# Constants
DEVICE_SN_PUBLISHER = "0xDEADBEEF"
BASE_NAME_PUBLISHER = "urn:dev:" + DEVICE_SN_PUBLISHER + ":sensors:"
BROKER_HOST = "rpi.mateuszkapala.eu"
BROKER_PORT = 1883
MQTT_TOPIC = "example/living_room/sensors"
SLEEP_TIME = 10


# Data section
def get_current_temperature():
    return random.uniform(25.0, 35.0)


def get_current_humidity():
    return random.uniform(0.0, 100.0)


def build_sensor_senml_object(temperature: float, humidity: float):
    pack = senml.SenmlPack(BASE_NAME_PUBLISHER)                  # set base name
    pack.base_time = time.time()                                 # set base time
    temp = senml.SenmlRecord("temperature",
                             unit=senml.SenmlUnits.SENML_UNIT_DEGREES_CELSIUS,
                             value=temperature)             # add room temperature record
    humidity = senml.SenmlRecord("humidity",
                                 unit=senml.SenmlUnits.SENML_UNIT_RELATIVE_HUMIDITY,
                                 value=humidity)            # add room relative humidity record
    pack.add(temp)
    pack.add(humidity)
    return pack.to_json()


# MQTT section
def on_publish(mqttc, obj, mid):
    print("=====> Published! Message ID: " + str(mid))


# Main script execution
if __name__ == "__main__":
    # Setup MQTT Paho client
    client = mqtt.Client(DEVICE_SN_PUBLISHER)
    client.on_publish = on_publish

    while True:
        # Connect
        client.connect(BROKER_HOST, BROKER_PORT)

        # Prepare sensor data and SenML object
        print("============================================================")
        msg = build_sensor_senml_object(get_current_temperature(),
                                        get_current_humidity())
        print(f"Data to be published: {msg}")

        # Publish to MQTT_TOPIC
        client.publish(MQTT_TOPIC, msg, qos=0)

        # Disconnect
        client.disconnect()

        # Wait for next iteration
        time.sleep(SLEEP_TIME)
