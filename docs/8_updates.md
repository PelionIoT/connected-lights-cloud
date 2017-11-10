### Applying firmware updates to the device

One of the big features of Mbed Cloud is the ability to update devices through a firmware update over the air. This is not applicable when you're developing, but it is important when you have deployed thousands of devices in the field. Through the firmware update process, you can patch bugs and apply security updates.

Currently, your application sends a notification to the cloud every time the PIR sensor is triggered. That is wasteful if someone is standing in front of the sensor. The lights are already on, but the sensor keeps firing, so the networking stack needs to wake up all the time. Modify the code, so it does not send events when the lights are already on.

#### Building with Mbed CLI

To enable firmware updates, the device needs to have the [Mbed bootloader](https://docs.mbed.com/docs/mbed-os-handbook/en/latest/advanced/bootloader/). The bootloader verifies the firmware on the device and can swap firmware for other firmware. To enable the bootloader you need to configure the linker to put your application in a separate part of flash. The bootloader can then run first.

Open `connected-lights-cloud/mbed_app.json` and replace `target_overrides` by:

```
    "target_overrides": {
        "*": {
            "target.features_add": ["NANOSTACK", "LOWPAN_ROUTER", "COMMON_PAL"],
            "platform.stdio-baud-rate": 115200,
            "platform.stdio-convert-newlines": true,
            "nanostack-hal.event_loop_thread_stack_size": 8192,
            "mbed-trace.enable": 0
        },
        "NUCLEO_F401RE": {
            "esp8266-tx": "D8",
            "esp8266-rx": "D2"
        },
        "NUCLEO_F411RE": {
            "esp8266-tx": "D8",
            "esp8266-rx": "D2"
        },
        "K64F": {
            "target.mbed_app_start": "0x00020400",
            "update-client.application-details": "0x00020000",
            "update-client.bootloader-details": "0x172d4"
        },
        "NUCLEO_F429ZI": {
            "target.mbed_app_start": "0x08020400",
            "update-client.application-details": "0x08020000",
            "update-client.bootloader-details": "0x8018630"
        },
        "UBLOX_EVK_ODIN_W2": {
            "target.device_has_remove": ["EMAC"],
            "target.mbed_app_start": "0x08020400",
            "update-client.application-details": "0x08020000",
            "update-client.bootloader-details": "0x801ae38"
        }
    }
```

There already is an entry for the ODIN-W2 board. Remove this first.

#### Update certificates

To enable updates, you need to embed an update certificate into the firmware of your application. This verifies that the update came from a trusted source because all firmware images are signed with a private key. The update certificate also prevents incompatible firmware to be flashed on the device because the certificate contains information about the manufacturer, device class and device ID.

For development, you can use a self-signed certificate, but please note that this is not secure.

<span class="notes">**Note:** If you're deploying devices in the field, always use a certificate from a trusted certificate authority (CA). Instructions on how to use your own certificate are [in the manifest-tool documentation](/docs/v1.2/mbed-cloud-management/update-manifest-creation.html#quick-start).</span>

##### Installing the manifest tool

To generate update certificates, you need the manifest tool. Install using:

```
$ pip install git+https://github.com/ARMmbed/manifest-tool.git
```

##### Generating an update certificate

To create a new self-signed certificate, run:

```
$ manifest-tool init -d yourdomain.com -m lighting-system-2000 -q --force
```

#### Building with the bootloader

Now that the update certificate is in place, you can build the application with the bootloader enabled. This procedure differs by development board.

##### FRDM-K64F

Build and add the bootloader to your firmware with:

```
$ mbed compile -m K64F -t GCC_ARM
$ simple-cloud-client/tools/combine_bootloader_with_app.py -b simple-cloud-client/tools/mbed-bootloader-k64f.bin -a  BUILD/K64F/GCC_ARM/connected-lights-cloud_application.bin --app-offset 0x20400 --header-offset 0x20000 -o combined.bin
```

Flash `combined.bin` to your development board.

##### ST NUCLEO-F429ZI

Build and add the bootloader to your firmware with:

```
$ mbed compile -m NUCLEO_F429ZI -t GCC_ARM
$ simple-cloud-client/tools/combine_bootloader_with_app.py -b simple-cloud-client/tools/mbed-bootloader-nucleo-f429zi.bin -a  BUILD/NUCLEO_F429ZI/GCC_ARM/connected-lights-cloud_application.bin --app-offset 0x20400 --header-offset 0x20000 -o combined.bin
```

Flash `combined.bin` to your development board.

##### u-blox EVK-ODIN-W2

Build and add the bootloader to your firmware with:

```
$ mbed compile -m ublox_evk_odin_w2 -t GCC_ARM
$ simple-cloud-client/tools/combine_bootloader_with_app.py -b simple-cloud-client/tools/mbed-bootloader-ublox-evk-odin-w2.bin -a  BUILD/ublox_evk_odin_w2/GCC_ARM/connected-lights-cloud_application.bin --app-offset 0x20400 --header-offset 0x20000 -o combined.bin
```

Flash `combined.bin` to your development board.

#### Creating the updated firmware

When your board is back online in Mbed Cloud, you can then prepare an update. Open `main.cpp`, and change the `pir_rise()` function to:

```cpp
// When the PIR sensor fires...
void pir_rise() {
    // Update the resource if the light is not on yet (because of the PIR sensor)
    if (!ledOnBecauseOfPir) {
        pirCount = pirCount + 1;
    }

    // Permanent off? Don't put the lights on...
    if (ledStatus == STATUS_OFF) return;

    // Otherwise do it!
    ledOnBecauseOfPir = true;
    putLightsOn();

    // And attach the timeout
    pirTimeout.attach(eventQueue.event(&onPirTimeout), static_cast<float>(ledTimeout));
}
```

Then rebuild the application, but do not flash the binary to your development board.

##### Uploading the firmware to Mbed Cloud

To schedule an update, you need to upload the firmware to Mbed Cloud.

1. Log in to [Mbed Cloud Portal](https://portal.us-east-1.mbedcloud.com).
1. Select **Update firmware** > **Images**.
1. Click **Upload new images**.
1. Enter a descriptive name.
1. Select the firmware:
    * On FRDM-K64F, select `BUILD/K64F/GCC_ARM/connected-lights-cloud_application.bin`.
    * On NUCLEO-F429ZI, select `BUILD/NUCLEO_F429ZI/GCC_ARM/connected-lights-cloud_application.bin`.
    * On ODIN-W2, select `BUILD/ublox_evk_odin_w2/GCC_ARM/connected-lights-cloud_application.bin`.
1. Click **Upload**.

After the upload succeeds, find the URL to your firmware file on the overview page.

##### Creating an update manifest

Every firmware update requires an update manifest. This update contains the cryptographic hash of the firmware, signed with your certificate. It also contains information about which devices this update applies to, so you don't accidentally update devices with incompatible firmware.

To generate a new update manifest, run:

```
$ manifest-tool create -p BUILD/YOUR_BOARD_NAME/GCC_ARM/connected-lights-cloud_application.bin -u http://path-to-your-firmware -o connected-lights.manifest
```

Replace `YOUR_BOARD_NAME` with the name of your development board, and replace `http://path-to-your-firmware` with the location of the firmware.

##### Uploading the manifest to Mbed Cloud

To upload the manifest to Mbed Cloud:

1. Select **Update firmware** > **Manifests**.
1. Click **Upload new manifest**.
1. Enter a descriptive name.
1. Select the manifest (`connected-lights.manifest`).
1. Click **Upload firmware manifest**.

##### Creating an update campaign

To apply the firmware update, you need to start an update campaign. The campaign holds information on the devices that you need to updated, and what manifest you need to used. To create a campaign, you first need to create a device filter, which holds the list of devices that you need to update.

###### Creating a device filter

In the Mbed Cloud Portal:

1. Select **Device directory**.
1. Click **Create new filter**.
1. Click **Add attribute**.
1. Select **Device ID**.
1. Enter your device ID. (Look in serial output to find your device ID.)
1. Give the filter a descriptive name, and save the filter.

###### Starting the campaign

With the firmware, the manifest and the device filter in place, you can start the firmware update campaign.

1. Select **Update firmware** > **Update campaigns**.
1. Click **Create campaign**.
1. Give the campaign a descriptive name.
1. Select the manifest you uploaded.
1. Select the filter you created.
1. Click **Save**.

To start the campaign, click **Start**.

Inspect the logs on the device (via a serial monitor) to see the firmware update progress. It looks similar to:

```
Firmware download requested
Authorization granted
Manifest timestamp: 1491912366
Firmware URL http://firmware-catalog-media-98...
Firmware size: 392384
Firmware hash (32): 00c3224b10728028ef239d3d283a11669f4235ebfbe9333e790932558d13c000
Downloading: 0 %
```

When the download completes, the firmware is verified. If everything is OK, the firmware update is applied. Your device is now running the latest version of the application, and when you have the web app open, you see that you don't get PIR notifications if the light is already on.
