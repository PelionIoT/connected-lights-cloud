import os
import pprint
from mbed_cloud.connect import ConnectAPI

TOKEN = "YOUR_ACCESS_TOKEN"

# set up the Python SDK
config = {}
config['api_key'] = os.environ['TOKEN'] or TOKEN
config['host'] = 'https://api.us-east-1.mbedcloud.com'
api = ConnectAPI(config)
api.start_notifications()

# todo, filter by endpoint type, see https://github.com/ARMmbed/mbed-cloud-sdk-python/issues/88
devices = list(api.list_connected_devices())

print("Found %d lights" % (len(devices)), [ c.id for c in devices ])

for device in devices:
    def pir_callback(device_id, path, count):
        print("Motion detected at %s, new count is %s" % (device_id, count))

    api.add_resource_subscription_async(device.id, '/pir/0/count', pir_callback)
    print("subscribed to resource")

    pink = 0xff69b4
    api.set_resource_value(device.id, '/led/0/color', pink)
    print("set color to pink")

# Run forever
while True:
    pass
