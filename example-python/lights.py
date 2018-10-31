import os
import pprint
from mbed_cloud import ConnectAPI

TOKEN = "YOUR_ACCESS_KEY"

# set up the Python SDK
config = {}
config['api_key'] = os.getenv('TOKEN') or TOKEN
config['host'] = 'https://api.us-east-1.mbedcloud.com'
api = ConnectAPI(config)
api.start_notifications()

devices = list(api.list_connected_devices(filters={'device_type': 'light-system'}))

print("Found %d lights" % (len(devices)), [ c.id for c in devices ])

for device in devices:
    def pir_callback(device_id, path, count):
        print("Motion detected at %s, new count is %s" % (device_id, count))

    api.add_resource_subscription_async(device.id, '/3201/0/5700', pir_callback)
    print("subscribed to resource")

    pink = 0xff69b4
    api.set_resource_value(device.id, '/3311/0/5706', pink)
    print("set color to pink")

# Run forever
while True:
    pass
