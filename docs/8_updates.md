## Applying firmware updates to the device

One of the big features of Device Management is the ability to update devices through a firmware update over the air. This is not applicable when you're developing, but it is important when you have deployed thousands of devices in the field. Through the firmware update process, you can patch bugs and apply security updates.

Currently, your application sends a notification to the cloud every time the PIR sensor is triggered. That is wasteful if someone is standing in front of the sensor. The lights are already on, but the sensor keeps firing, so the networking stack needs to wake up all the time. Modify the code, so it does not send events when the lights are already on.

### Building with Mbed CLI

To enable firmware updates, the device needs to have the [Mbed bootloader](https://github.com/armmbed/mbed-bootloader). The bootloader verifies the firmware on the device and can swap firmware for other firmware. To enable the bootloader you need to configure the linker to put your application in a separate part of flash. The bootloader can then run first.

Open `connected-lights-cloud/mbed_app.json` and replace the `target_overrides` section by:

```
    "target_overrides": {
        "*": {
            "target.features_add": ["NANOSTACK", "LOWPAN_ROUTER", "COMMON_PAL"],
            "platform.stdio-baud-rate": 115200,
            "platform.stdio-convert-newlines": true,
            "mbed-client.event-loop-size": 1024,
            "nanostack-hal.event_loop_thread_stack_size": 8192,
            "storage-selector.storage": "SD_CARD",
            "storage-selector.filesystem": "FAT",
            "storage-selector.mount-point": "\"sd\"",
            "update-client.storage-address"  : "(1024*1024*64)",
            "update-client.storage-size"     : "(1024*1024*2)",
            "update-client.storage-locations": "1",
            "mbed-trace.enable": 0
        },
        "K64F": {
            "target.mbed_app_start"            : "0x0000a400",
            "update-client.bootloader-details" : "0x00007188",
            "sotp-section-1-address"           : "(32*1024)",
            "sotp-section-1-size"              : "( 4*1024)",
            "sotp-section-2-address"           : "(36*1024)",
            "sotp-section-2-size"              : "( 4*1024)",
            "update-client.application-details": "(40*1024)"
        },
        "NUCLEO_F429ZI": {
            "target.mbed_app_start"            : "0x08010400",
            "update-client.bootloader-details" : "0x080078CC",
            "sotp-section-1-address"           : "(0x08000000+32*1024)",
            "sotp-section-1-size"              : "(16*1024)",
            "sotp-section-2-address"           : "(0x08000000+48*1024)",
            "sotp-section-2-size"              : "(16*1024)",
            "update-client.application-details": "(0x08000000+64*1024)"
        },
        "UBLOX_EVK_ODIN_W2": {
            "target.device_has_remove": ["EMAC"],
            "target.mbed_app_start"            : "0x08010400",
            "update-client.bootloader-details" : "0x08007300",
            "sotp-section-1-address"           : "(0x08000000+32*1024)",
            "sotp-section-1-size"              : "(16*1024)",
            "sotp-section-2-address"           : "(0x08000000+48*1024)",
            "sotp-section-2-size"              : "(16*1024)",
            "update-client.application-details": "(0x08000000+64*1024)"
        }
    }
```

### Update certificates

To enable updates, you need to embed an update certificate into the firmware of your application. This verifies that the update came from a trusted source because all firmware images are signed with a private key. The update certificate also prevents incompatible firmware to be flashed on the device because the certificate contains information about the manufacturer, device class and device ID.

For development, you can use a self-signed certificate, but please note that this is not secure.

<span class="notes">**Note:** If you're deploying devices in the field, always use a certificate from a trusted certificate authority (CA). Instructions on how to use your own certificate are [in the manifest-tool documentation](/docs/v1.2/mbed-cloud-management/update-manifest-creation.html#quick-start).</span>

#### Installing the manifest tool

To generate update certificates, you need the manifest tool. Install using:

**Windows, Linux**

```bash
$ pip install git+https://github.com/ARMmbed/manifest-tool.git
```

**MacOS**

```bash
$ pip install git+https://github.com/ARMmbed/manifest-tool.git --user python
```

#### Generating an update certificate

To create a new self-signed certificate, run:

```
$ manifest-tool init -a YOUR_MBED_CLOUD_API_KEY -d yourdomain.com -m lighting-system-2000 -q --force
```

**Note:** Make sure to replace `YOUR_MBED_CLOUD_API_KEY` with the API key you created earlier.

### Building with the bootloader

Now that the update certificate is in place, you can build the application with the bootloader enabled. This procedure differs by development board.

#### FRDM-K64F

Build and add the bootloader to your firmware with:

```
$ mbed compile -m K64F -t GCC_ARM
$ tools/combine_bootloader_with_app.py -m K64F -a BUILD/K64F/GCC_ARM/connected-lights-cloud_application.bin -o combined.bin
```

Flash `combined.bin` to your development board.

#### ST NUCLEO-F429ZI

Build and add the bootloader to your firmware with:

```
$ mbed compile -m NUCLEO_F429ZI -t GCC_ARM
$ tools/combine_bootloader_with_app.py -m NUCLEO_F429ZI -a BUILD/NUCLEO_F429ZI/GCC_ARM/connected-lights-cloud_application.bin -o combined.bin
```

Flash `combined.bin` to your development board.

#### u-blox EVK-ODIN-W2

Build and add the bootloader to your firmware with:

```
$ mbed compile -m ublox_evk_odin_w2 -t GCC_ARM
$ tools/combine_bootloader_with_app.py -m ublox_evk_odin_w2 -a BUILD/ublox_evk_odin_w2/GCC_ARM/connected-lights-cloud_application.bin -o combined.bin
```

Flash `combined.bin` to your development board.

### Creating the updated firmware

When your board is back online in Device Management, you can then prepare an update. Open `main.cpp`, and change the `pir_rise()` function to:

```cpp
// When the PIR sensor fires...
void pir_rise() {
    // Update the resource if the light is not on yet (because of the PIR sensor)
    if (!ledOnBecauseOfPir) {
        pirCount->set_value(pirCount->get_value_int() + 1);
    }

    // Permanent off? Don't put the lights on...
    if (ledStatus->get_value_int() == STATUS_OFF) return;

    // Otherwise do it!
    ledOnBecauseOfPir = true;
    putLightsOn();

    // And attach the timeout
    pirTimeout.attach(eventQueue.event(&onPirTimeout), static_cast<float>(ledTimeout->get_value_int()));
}
```

Then rebuild the application, but do not flash the binary to your development board.

### Updating the device

Now we can push this new application to your development through Device Management. The manifest tool can both sign the update - using the private key generated earlier - and upload it to Device Management in a single command.

Run:

```
$ manifest-tool update device -p BUILD/YOUR_BOARD_NAME/GCC_ARM/connected-lights-cloud_application.bin -D YOUR_ENDPOINT_NAME
```

Replace `YOUR_BOARD_NAME` with the name of your development board, and replace `YOUR_ENDPOINT_NAME` with the endpoint name in Device Management.

Inspect the logs on the device (via a serial monitor) to see the firmware update progress. It looks similar to:

```
Firmware download requested
Authorization granted
Downloading: [+++-                                              ] 6 %
```

When the download completes, the firmware is verified. If everything is OK, the firmware update is applied. Your device is now running the latest version of the application, and when you have the web app open, you see that you don't get PIR notifications if the light is already on.
