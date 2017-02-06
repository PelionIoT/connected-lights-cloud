# Using the mbed Device Connector API

The mbed Device Connector web interface that we used in the previous section is a wrapper around the mbed Device Connector API. Through this API we can connect any app to any device. We can use this API to build an app that allows us to control any of the lighting systems that we deploy in our house or office.

## Obtaining an access key

To talk to the API we need to obtain an access key. This key is used to authenticate with the API. To create a new access key, go to the [Access Keys](https://connector.mbed.com/#accesskeys) page in the mbed Device Connector web interface.

Click *CREATE NEW ACCESS KEY* to create a new access key, and give it a descriptive name.


![Creating a new access key in mbed Device Connector](assets/lights14.png)

## Testing the API

We can quickly test out if the access key works by doing a call to the API to query for all our devices. To retrieve a list of all devices, make a GET request to https://api.connector.mbed.com/endpoints/. You'll need to send an authorization header with this request:

```
Authorization: Bearer <your_access_key>
```

You can make this request with any request library, but if you're using curl, use the following command:



```
curl -v -H "Authorization: Bearer <your_access_key>" https://api.connector.mbed.com/endpoints/
```

This will return something like this:

```
< HTTP/1.1 200 OK
< Date: Tue, 14 Jun 2016 10:37:01 GMT
< Server: mbed Device Server
< Content-Type: application/json
< Content-Length: 169
<
* Connection #0 to host api.connector.mbed.com left intact

[
  {
    "name": "9e3a37ae-e74f-465f-b04c-f105c8f69f1c",
    "type": "test",
    "status": "ACTIVE"
  },
  {
    "name": "8874e0eb-96ef-4799-b948-e91d05147bfe",
    "type": "light-system",
    "status": "ACTIVE"
  }
]
```

<span class="notes">**Note:** The official API documentation for the mbed Device Connector REST interface is [located here](https://docs.mbed.com/docs/mbed-device-connector-web-interfaces/en/latest/).</span>

## Using the official libraries

You don't need to write raw HTTP requests to deal with the mbed Device Connector REST interface, as there are official libraries available for node.js, Python and Java. This is especially nice because the APIs to interact with resources are [asynchronous](https://docs.mbed.com/docs/mbed-device-connector-web-interfaces/en/latest/#asynchronous-requests), because for many functions it's not guaranteed that an action (such as writing to a device) will happen straight away, as the device might be in deep sleep or otherwise slow to respond. Therefore, you need to listen to callbacks on a [notification channel](https://docs.mbed.com/docs/mbed-device-connector-web-interfaces/en/latest/api-reference/#notifications). If you're using any of the official libraries, notification channels are abstracted away, making it easier to write applications on top of mbed Device Connector.

An additional feature in the libraries is that they support subscriptions. We can subscribe to resources and get a notification whenever they change. This is useful for our `/pir/0/count` resource, as we can get notified whenever someone moves in front of the sensor.

The following sections show an example of changing the color of the light, and receiving a notification whenever someone waves in front of the PIR sensor, in both node.js and Python.

### node.js

First, make sure you have installed [node.js](http://nodejs.org). Then create a new folder, and install the mbed Device Connector node.js library via npm:

```bash
$ npm install mbed-connector-api
```

Next create a new file ``main.js`` in the same folder where you installed the library, and fill it with the following content (replace `YOUR_ACCESS_KEY` with your access key):

```js
var CloudApi = require('mbed-connector-api');
var api = new CloudApi({
  accessKey: 'YOUR_ACCESS_KEY'
});

// Start notification channel
api.startLongPolling(function(err) {
  if (err) throw err;

  // Find all lights
  api.getEndpoints(function(err, devices) {
    if (err) throw err;

    console.log('Found', devices.length, 'lights', devices);

    devices.forEach(function(d) {
      // For every light, we will request notifications on the PIR resource
      api.putResourceSubscription(d.name, '/pir/0/count');

      // and set the color to orange, as your's truly is Dutch
      var orange = 0xff6400;
      api.putResourceValue(d.name, '/led/0/color', orange, function(err) {
        if (err) console.error('Setting led/0/color for', d.name, 'failed', err);
        console.log('Set color of', d.name, 'to orange!');
      });

    });

  }, { parameters: { type: 'light-system' } });
});

// Notifications
api.on('notification', function(notification) {
  console.log('Got a notification', notification);
});
```

When you run this program, and you wave your hand in front of the PIR sensor, you'll see something like this:

```
$ node main.js
Found 1 lights [ { name: '8874e0eb-96ef-4799-b948-e91d05147bfe',
    type: 'light-system',
    status: 'ACTIVE' } ]
Set color of 8874e0eb-96ef-4799-b948-e91d05147bfe to orange!
Got a notification { ep: '8874e0eb-96ef-4799-b948-e91d05147bfe',
  path: '/pir/0/count',
  ct: 'text/plain',
  payload: '7',
  'max-age': 0 }
```

See here for the [full docs](https://github.com/ARMmbed/mbed-connector-api-node) on how to use the node.js library.

### Python

First make sure that you have installed [Python 2.7 or Python 3](https://www.python.org/downloads/) and [pip](https://pip.pypa.io/en/stable/installing/). Then create a new folder, and install the mbed Device Connector library through pip:

```bash
$ pip install -U mbed-connector-api
```

Next create a new file - ``lights.py`` - in the same folder where you installed the library, and fill it with the following content (replace `YOUR_ACCESS_KEY` with your access key):

```python
import mbed_connector_api
import time
import base64

connector = mbed_connector_api.connector("YOUR_ACCESS_KEY")

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
    pink = 0xff69b4
    x = connector.putResourceValue(endpoint['name'], "/led/0/color", pink)
    while not x.isDone():
        None
    if (x.error):
        print("Setting pink color for %s failed: %s" % (endpoint['name'], x.error.error))
    else:
        print("Set color of %s to pink!" % endpoint['name'])

while 1:
    time.sleep(1.0)
```

When we run this program, and you wave your hand in front of the PIR sensor, we'll see something like this:

```
$ python lights.py
Found 1 lights: [{u'status': u'ACTIVE', u'type': u'light-system', u'name': u'8874e0eb-96ef-4799-b948-e91d05147bfe'}]
Set color of 8874e0eb-96ef-4799-b948-e91d05147bfe to pink!
Got a notification for 8874e0eb-96ef-4799-b948-e91d05147bfe: /pir/0/count -> new value 49
```

See here for the [full docs](https://github.com/ARMmbed/mbed-connector-api-python) on how to use the Python library.
