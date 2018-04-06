## Controlling the device from Mbed Cloud

Now, the device is connected through Mbed Cloud. In the code sample in the previous section, you defined resources using calls to `add_resource()`. These resources are automatically exposed to Mbed Cloud, from where you can read and write resources, and changes automatically sync with the device. That means that you already have a remote management interface for this device.

### Seeing the status of a device

Each device that you connect to Mbed Cloud has an endpoint name. This is a long string, which is the unique identifier of your device. If you don't know the endpoint name of your device, check the [serial output](https://os.mbed.com/docs/latest/tutorials/serial-comm.html) on your device for a line starting with 'Device Identity'.

You need to know the endpoint's name to check the device's status in the Mbed Cloud Portal. The [Device directory](https://portal.us-east-1.mbedcloud.com/devices) page lists all devices associated with your account and their current status. Click the **Connected only** toggle to only see connected devices.

<span class="tips">**Tip:** The Mbed Cloud interface lists your devices by type. You can categorize devices by setting the device type in the application running on the device. See the `endpoint-type` property in `mbed_app.json`.</span>

<span class="images">![Two connected devices](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights11.png)<span>The Mbed Cloud Portal connectivity inspector page, showing two connected devices: our light-system and another device.</span></span>

### Controlling the device

You created four resources before (see `main.cpp`):

* `3311/0/5706` - the color of the LED, encoded as three bytes.
* `3311/0/5853` - the timeout (in seconds) after detection; lights are disabled when this period ends.
* `3311/0/5850` - whether we should have the lights permanently on (status 1) or off (status 2), or just let the PIR sensor figure it out (status 0).
* `3201/0/5700` - the number of times the PIR sensor was triggered. This is read only and shows notifications.

You can control these resources through the Mbed Cloud Portal. For instance, when you write the value `1` to `3311/0/5850`, the lights stay on indefinitely.

#### Turning the lights on

To test this, click on your Device ID in the device directory in Mbed Cloud Portal. This gives you access to a management console where you can quickly test interactions with resources.

<span class="images">![Viewing resources on the device](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights19.png)<span>These tables show the available resources on this device.</span></span>

To enable the lights:

1. Click **/3311/0/5850**.
1. Click **Edit**.
1. Enter `1`.
1. Click **Save**.

    <span class="images">![Updating the value of a resource](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights20.png)</span>

Now, your lights stay on until you change the status of this resource to 0 (listen to PIR sensor) or 2 (always off).

#### Setting the color

You can control the color of the lights the same way. The color is encoded in an integer that stores three channels: red, green and blue. Each of the channels can have a value between 0 (off) and 255 (completely on).

To encode the value of a color:

```js
red = 0;
green = 255;
blue = 255;

// alternatively: encode the color as a hex value, via encoded = 0x00ffff

encoded = (red << 16) + (green << 8) + blue;
// 65380
```

Use the API Console to write this value to resource `/3311/0/5706` and change the color of the LED to turquoise.

#### Other variables

You can also change the value of the timeout (in a real light system, you probably want at least 30 seconds) and read the number of times the PIR sensor triggered.
