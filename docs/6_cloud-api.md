## Using the Device Management API

The Device Management Portal that you used in the previous section is a wrapper around the Device Management API. Through this API, you can connect any app to any device. You can use this API to build an app that allows you to control any of the lighting systems that you deploy in your house or office.

### Obtaining an access key

To talk to the API, you need an API key. This key is used to authenticate with the API. To create a new access key, go to the [Access management](https://portal.mbedcloud.com/access/keys) page in the Device Management Portal.

Click **New API Key** to create a new API key, and name it.

<span class="images">![Creating a new access key in Device Management](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights14.png)</span>

### Testing the API

You can quickly test if the access key works by sending a call to the API to query for all the devices. To retrieve a list of all devices, make a GET request to `https://api.us-east-1.mbedcloud.com/v2/endpoints`. You need to send an authorization header with this request:

```
Authorization: Bearer <your_access_key>
```

You can make this request with any request library, but if you're using curl, use the following command:

```
curl -v -H "Authorization: Bearer <your_access_key>" https://api.us-east-1.mbedcloud.com/v2/endpoints
```

It will return something like this:

```
*   Trying 52.1.229.179...
* Connected to api.us-east-1.mbedcloud.com (52.1.229.179) port 443 (#0)
* TLS 1.2 connection using TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256
> GET /v2/endpoints HTTP/1.1
> Host: api.us-east-1.mbedcloud.com
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

<span class="notes">**Note:** Please see the official [API documentation](https://www.pelion.com/docs/device-management/current/service-api-references/index.html) for the Device Management REST API interface.</span>

### Using the official libraries

Official Device Management SDKs are available for Node.js and Python. These APIs are asynchronous because for many functions, an action (such as writing to a device) might not happen immediately - the device might be in deep sleep or otherwise slow to respond. Therefore, you need to listen to callbacks on a notification channel. The official libraries abstract the notification channels and set up the channels for you, which makes it easier for you to write applications on top of Device Management.

An additional feature of the libraries is that they support subscriptions. You can subscribe to resources and get a notification whenever they change. This is useful for the `3201/0/5700` (PIR count) resource because you can receive a notification whenever someone moves in front of the sensor.

The following sections show an example of changing the color of the light and receiving a notification whenever someone waves in front of the PIR sensor, in both Node.js and Python.

#### Node.js

First, make sure you have installed [Node.js](http://nodejs.org). Then, create a new folder, and install the Device Management Node.js SDK via npm:

```bash
$ npm install mbed-cloud-sdk --save
```

Next, create a new file `main.js` in the same folder where you installed the library, and fill it with the following content (replace `YOUR_ACCESS_KEY` with your access key):

```js
var TOKEN = 'YOUR_ACCESS_KEY';

var mbed = require('mbed-cloud-sdk');
var api = new mbed.ConnectApi({
    apiKey: process.env.TOKEN || TOKEN,
    host: 'https://api.us-east-1.mbedcloud.com'
});

// Start notification channel (to receive data back from the device)
api.startNotifications(function(err) {
    if (err) return console.error(err);

    // Find all the lights
    var filter = { deviceType: { $eq: 'light-system' } };
    api.listConnectedDevices({ filter: filter }, function(err, resp) {
        if (err) return console.error(err);

        var devices = resp.data;
        if (devices.length === 0) return console.error('No lights found...');

        console.log('Found', devices.length, 'lights', devices.map(d => d.id));

        devices.forEach(function(d) {

            // Subscribe to the PIR sensor
            api.addResourceSubscription(
                d.id,
                '/3201/0/5700',
                function(count) {
                    console.log('Motion detected at', d.id, 'new count is', count);
                },
                function(err) {
                    console.log('subscribed to resource', err || 'OK');
                });

            // Set the color of the light
            var orange = 0xff6400;
            api.setResourceValue(d.id, '/3311/0/5706', orange.toString(), function(err) {
                console.log('set color to orange', err || 'OK');
            });

        });
    });
});
```

When you run this program and you wave your hand in front of the PIR sensor, you will see something like this:

```
$ node main.js
Found 1 lights [ '015b58400ce40000000000010010022a' ]
subscribed to resource OK
set color to orange OK
Motion detected at 015b58400ce40000000000010010022a new count is 1
Motion detected at 015b58400ce40000000000010010022a new count is 2
```

See the [full docs](https://cloud.mbed.com/docs/current/mbed-cloud-sdk-javascript/) on how to use the JavaScript SDK.

#### Python

1. Install [Python 2.7](https://www.python.org/downloads/) and [pip](https://pip.pypa.io/en/stable/installing/) if you have not already.
1. Create a new folder.
1. Install the Device Management SDK through pip:

**Windows, Linux**
```bash
$ pip install mbed-cloud-sdk
```
**MacOS**

```bash
$ pip install mbed-cloud-sdk --user python
```
**The remaining steps are the same regardless of which OS you use.**

1. Create a new file - `lights.py` - in the same folder where you installed the library, and fill it with the following content (replace `YOUR_ACCESS_KEY` with your access key):

```python
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
```

When you run this program and you wave your hand in front of the PIR sensor, you see something like this:

```
$ python lights.py
('Found 1 lights', ['015b58400ce40000000000010010022a'])
subscribed to resource
set color to pink
Motion detected at 015b58400ce40000000000010010022a, new count is 7
Motion detected at 015b58400ce40000000000010010022a, new count is 8
```

See the [full docs](https://cloud.mbed.com/docs/current/mbed-cloud-sdk-python/index.html) on how to use the Python library.
