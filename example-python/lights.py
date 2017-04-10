import os
import pprint
from mbed_cloud.devices import DeviceAPI

TOKEN = "YOUR_ACCESS_TOKEN"

# set up the Python SDK
config = {}
config['api_key'] = os.environ['TOKEN'] or TOKEN
api = DeviceAPI(config)
api.start_long_polling()

# todo, filter by endpoint type, see https://github.com/ARMmbed/mbed-cloud-sdk-python/issues/88
devices = list(api.list_connected_devices())

print("Found %d lights" % (len(devices)), [ c.id for c in devices ])

for device in devices:
    def pir_callback(count):
        print("Motion detected at %s, new count is %s" % (device.id, count))

    api.add_subscription_with_callback(device.id, '/pir/0/count', pir_callback)
    print("subscribed to resource")

    pink = 0xff69b4
    api.set_resource_value(device.id, '/led/0/color', pink)
    print("set color to pink")

# Run forever
while True:
    pass
