import anedya
import time
import random
import psutil  # Ensure this package is installed

# Set your actual device ID and connection key
deviceID = "cfe58997-c68c-4473-b08f-3b1880262626"
connectionKey = "dd73a8b6458bb637f7521c9ba1f67388"

virtual_sensor = True  # Set to False if using a physical sensor

def get_actual_water_level():
    # Implement this function to fetch actual water level data
    # For example, return a random value for testing
    return random.uniform(0, 30)  # Adjust based on your actual sensor data

def main():
    config = anedya.default_config()
    config.connection_mode = anedya.ConnectionMode.MQTT
    config.set_deviceid(deviceID)
    config.set_connection_key(connectionKey)

    client = anedya.AnedyaClient(config)
    time.sleep(1)
    client.connect()
    time.sleep(2)
    print(client._mqttclient.is_connected())

    while True:
        water_level = get_actual_water_level()
        print("Water Level:", water_level)

        data = anedya.DataPoints()
        dp1 = anedya.FloatData(
            variable="water-level",
            timestamp_milli=int(time.time_ns() / 1000000),
            value=water_level,
        )
        data.append(dp1)

        client.submit_data(data)
        print("Data Pushed!")
        data.reset_datapoints()
        time.sleep(15)

    client.disconnect()

if __name__ == "__main__":
    main()
