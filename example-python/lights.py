import mbed_connector_api
import time
import base64
import os

connector = mbed_connector_api.connector(os.environ['TOKEN'])

def notificationHandler(data):
    for n in data['notifications']:
        print "Got a notification for %s: %s -> new value %s" % (n['ep'], n['path'], base64.b64decode(n['payload']))

connector.startLongPolling()
connector.setHandler('notifications', notificationHandler)

e = connector.getEndpoints("light-system")
while not e.isDone():
    None
if e.error:
    raise(e.error.error)
print("Found %d lights: %s" % (len(e.result), str(e.result)))

for endpoint in e.result:
    # Get a notification whenever the PIR count changes
    connector.putResourceSubscription(endpoint['name'], "/pir/0/count")

    # And change the color to pink, because that's nice
    pink = (255 << 16) + (100 << 8) + 15
    x = connector.putResourceValue(endpoint['name'], "/led/0/color", pink)
    while not x.isDone():
        None
    if (x.error):
        print("Setting pink color for %s failed: %s" % (endpoint['name'], x.error.error))
    else:
        print("Set color of %s to pink!" % endpoint['name'])

while 1:
    time.sleep(1.0)
