### Using the mbed Cloud API

The mbed Cloud Portal that we used in the previous section is a wrapper around the mbed Cloud API. Through this API we can connect any app to any device. We can use this API to build an app that allows us to control any of the lighting systems that we deploy in our house or office.

#### Obtaining an access key

To talk to the API we need to get an API key. This key is used to authenticate with the API. To create a new access key, go to the [Key management](https://portal.mbedcloud.com/access/keys) page in the mbed Cloud Portal.

Click *Create API Key* to create a new API key, and give it a descriptive name.

<span class="images">![Creating a new access key in mbed Cloud](assets/lights14.png)</span>

#### Testing the API

We can quickly test if the access key works by doing a call to the API to query for all our devices. To retrieve a list of all devices, make a GET request to https://api.mbedcloud.com/v2/endpoints. You'll need to send an authorization header with this request:

```
Authorization: Bearer <your_access_key>
```

You can make this request with any request library, but if you're using curl, use the following command:

```
curl -v -H "Authorization: Bearer <your_access_key>" https://api.mbedcloud.com/v2/endpoints
```

It will return something like this:

```
*   Trying 52.1.229.179...
* Connected to api.mbedcloud.com (52.1.229.179) port 443 (#0)
* TLS 1.2 connection using TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256
> GET /v2/endpoints HTTP/1.1
> Host: api.mbedcloud.com
> User-Agent: curl/7.43.0
> Accept: */*
> Authorization: Bearer ak_...
>
< HTTP/1.1 200 OK
< Content-Type: application/json; charset=utf-8
< Server: nginx
< Content-Length: 85
< Connection: keep-alive
<
[
    {
        "name": "015b58400ce40000000000010010022a",
        "type": "light-system",
        "status": "ACTIVE"
    }
]
```

<span class="notes">**Note:** The official API documentation for the mbed Cloud REST API interface is [located here](/docs/v1.2/api-references/index.html).</span>

#### Using the official libraries

There are official mbed Cloud SDKs available for node.js and Python. These APIs are asynchronous, because for many functions it is not guaranteed that an action (such as writing to a device) will happen straight away -  the device might be in deep sleep or otherwise slow to respond. Therefore, you need to listen to callbacks on a notification channel. If you're using any of the official libraries, notification channels are abstracted away, making it easier to write applications on top of mbed Cloud.

An additional feature in the libraries is that they support subscriptions. We can subscribe to resources and get a notification whenever they change. This is useful for our `/pir/0/count` resource, as we can get notified whenever someone moves in front of the sensor.

The following sections show an example of changing the color of the light, and receiving a notification whenever someone waves in front of the PIR sensor, in both node.js and Python.

##### node.js

First, make sure you have installed [node.js](http://nodejs.org). Then create a new folder, and install the mbed Cloud node.js SDK via npm:

```bash
$ npm install mbed-cloud-sdk
```

Next create a new file ``main.js`` in the same folder where you installed the library, and fill it with the following content (replace `YOUR_ACCESS_KEY` with your access key):

```js
var TOKEN = 'YOUR_ACCESS_TOKEN';

var mbed = require('mbed-cloud-sdk');
var api = new mbed.DevicesApi({
    apiKey: process.env.TOKEN || TOKEN
});

// Start notification channel (to receive data back from the device)
api.startNotifications(function(err) {
    if (err) return console.error(err);

    // Find all the lights
    api.listConnectedDevices({ type: 'light-system' }, function(err, resp) {
        if (err) return console.error(err);

        var devices = resp.data;
        if (devices.length === 0) return console.error('No lights found...');

        console.log('Found', devices.length, 'lights', devices.map(d => d.id));

        devices.forEach(function(d) {

            // Subscribe to the PIR sensor
            api.addResourceSubscription({
                id: d.id,
                path: '/pir/0/count',
                fn: function(count) {
                    console.log('Motion detected at', d.id, 'new count is', count);
                }
            }, function(err) {
                console.log('subscribed to resource', err || 'OK');
            });

            // Set the color of the light
            var orange = 0xff6400;
            api.setResourceValue({
                id: d.id,
                path: '/led/0/color',
                value: orange
            }, function(err) {
                console.log('set color to orange', err || 'OK');
            });

        });
    });
});

```

When you run this program, and you wave your hand in front of the PIR sensor, you will see something like this:

```
$ node main.js
Found 1 lights [ '015b58400ce40000000000010010022a' ]
subscribed to resource OK
set color to orange OK
Motion detected at 015b58400ce40000000000010010022a new count is 1
Motion detected at 015b58400ce40000000000010010022a new count is 2
```

See here for the [full docs](https://github.com/ARMmbed/mbed-cloud-sdk-javascript) on how to use the JavaScript SDK.

##### Python

First make sure that you have installed [Python 2.7](https://www.python.org/downloads/) and [pip](https://pip.pypa.io/en/stable/installing/). Then create a new folder, and install the mbed Cloud SDK through pip:

```bash
$ pip install -U mbed-cloud-sdk
```

Next create a new file - ``lights.py`` - in the same folder where you installed the library, and fill it with the following content (replace `YOUR_ACCESS_KEY` with your access key):

```python
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
```

When we run this program, and you wave your hand in front of the PIR sensor, we'll see something like this:

```
$ python lights.py
('Found 1 lights', ['015b58400ce40000000000010010022a'])
subscribed to resource
set color to pink
Motion detected at 015b58400ce40000000000010010022a, new count is 7
Motion detected at 015b58400ce40000000000010010022a, new count is 8
```

See here for the [full docs](https://github.com/ARMmbed/mbed-cloud-sdk-python) on how to use the Python library.
