import paho.mqtt.client as mqtt
import kpn_senml as senml
import time
import random

# Constants
DEVICE_SN_RECEIVER = "0xBAADF00D"
DEVICE_SN_PUBLISHER = "0xDEADBEEF"
BASE_NAME_PUBLISHER = "urn:dev:" + DEVICE_SN_PUBLISHER + ":sensors:"
BROKER_HOST = "rpi.mateuszkapala.eu"
BROKER_PORT = 1883
MQTT_TOPIC = "example/living_room/sensors"
TEMP_LOW = 27.0
TEMP_HIGH = 30.0


# Utilities
def static_vars(**kwargs):
    def decorate(func):
        for k in kwargs:
            setattr(func, k, kwargs[k])
        return func
    return decorate


# Data section
@static_vars(heater_on=False)
def change_heater_state(current_temperature: float):
    if change_heater_state.heater_on and current_temperature > TEMP_HIGH:
        change_heater_state.heater_on = False
        print("=====> Turning off!")
    elif not change_heater_state.heater_on and current_temperature < TEMP_LOW:
        change_heater_state.heater_on = True
        print("=====> Turning on!")
    else:
        print(f"=====> Current heater status: {change_heater_state.heater_on}")


def handle_temp_value(record):
    print(f"Current temperature value: {record.value}")
    change_heater_state(record.value)


# MQTT section
def on_message(mqttc, obj, msg):
    print("============================================================")
    recv_msg = msg.payload.decode('ASCII')
    print(f"Topic: {msg.topic}")
    print(f"Recived message: {recv_msg}")

    # Setup SenML decoder
    pack = senml.SenmlPack(BASE_NAME_PUBLISHER)
    temp = senml.SenmlRecord("temperature", callback=handle_temp_value)
    pack.add(temp)

    # Parse
    try:
        pack.from_json(recv_msg)
    except:
        print("Payload is not usable!")


def on_subscribe(mqttc, obj, mid, granted_qos):
    print(f"Subscribed! ID: {mid}")


# Main script execution
if __name__ == "__main__":
    # Setup MQTT Paho client
    client = mqtt.Client(DEVICE_SN_RECEIVER)
    client.on_message = on_message
    client.on_subscribe = on_subscribe

    # Connect
    client.connect(BROKER_HOST, BROKER_PORT, 60)

    # Subscribe
    client.subscribe(MQTT_TOPIC, 0)

    # Wait for messages
    client.loop_forever()
