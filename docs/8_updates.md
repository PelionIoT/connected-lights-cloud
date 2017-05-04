### Applying firmware updates to the device

One of the big features of mbed Cloud is the ability to update devices through a firmware update over the air. This is not applicable when you're developing, but it is important when you have deployed thousands of devices in the field. Through the firmware update process, you can patch bugs and apply security updates.

Currently, your application sends a notification to the cloud every time the PIR sensor is triggered. That is wasteful if someone is standing in front of the sensor. The lights are already on, but the sensor keeps firing, so the networking stack needs to wake up all the time. Modify the code, so it does not send events when the lights are already on.

#### Building with mbed CLI

To enable firmware updates, the device needs to have the [mbed bootloader](https://github.com/armmbed/mbed-bootloader-private). The bootloader verifies the firmware on the device and can swap firmware for other firmware. Unfortunately, you cannot add the bootloader to your application via the mbed Online Compiler. You can only do so with mbed CLI.

##### Exporting your code

If you are not yet using mbed CLI:

1. Install [mbed CLI](https://docs.mbed.com/docs/mbed-os-handbook/en/latest/dev_tools/cli/#installing-mbed-cli) and its dependencies.
1. Install the [GNU ARM Embedded Toolchain 4.9](https://launchpad.net/gcc-arm-embedded/4.9/4.9-2015-q3-update).

<span class="tips">**Tip:** There is also a [Windows installer](https://mbed-media.mbed.com/filer_public/2c/88/2c880c48-2059-40fb-882b-3fa52ac172fc/mbed_install_v032.exe).</span>

Then, to export your code from the mbed Online Compiler and into mbed CLI:

1. Right click on your project in the Online Compiler.
1. Click *Publish*.
1. Click *Fork*.
1. If you're prompted for a commit message, enter one.
1. Enter a description for the project, and mark the project as 'Private'.

    <span class="images">![Publishing the project](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights21.png)</span>

    <span class="notes">**Note:** Mark the project because private as it contains your device certificate.</span>

1. Click *OK*.
1. You're presented with a URL to the published project.

    <span class="images">![Published project](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights22.png)</span>

1. Locally, open a terminal and run:

    ```
    $ mbed import https://path-to-your-project
    ```

#### Update certificates

To enable updates, you need to embed an update certificate into the firmware of your application. This verifies that the update came from a trusted source because all firmware images are signed with a private key. The update certificate also prevents incompatible firmware to be flashed on the device because the certificate contains information about the manufacturer, device class and device ID.

For development, you can use a self-signed certificate, but please note that this is not secure.

<span class="notes">**Note:** If you're deploying devices in the field, always use a certificate from a trusted certificate authority (CA). Instructions on how to use your own certificate are [in the manifest-tool documentation]().</span>

##### Generating an update certificate

To create a new self-signed certificate, run:

```
$ simple-cloud-client/tools/manifest-tool/bin/manifest-tool init -d yourdomain.com -m lighting-system-2000 -r
```

When prompted, answer the questions.

#### Building with the bootloader

Now that the update certificate is in place, you can build the application with the bootloader enabled. This procedure differs by development board.

##### FRDM-K64F

First, apply some linker patches:

```
$ cd mbed-os
$ git apply ../simple-cloud-client/tools/MK64FN1M0xxx12.ld.diff
$ git apply ../simple-cloud-client/tools/gcc_k64f_ram_patch.diff
```

Then build, and add the bootloader to your firmware with:

```
$ mbed compile -m K64F -t GCC_ARM
$ simple-cloud-client/tools/combine_bootloader_with_app.py -b simple-cloud-client/tools/mbed-bootloader.bin -a  BUILD/K64F/GCC_ARM/*.bin --app-offset 0x14100 --header-offset 0x14000 -o combined.bin
```

Flash `combined.bin` to your development board.

##### Ameba RTL8195A

The bootloader is already included when you build for this board. Run:

```
$ mbed compile -m RTL8195A -t GCC_ARM
```

Then flash `BUILD/RTL8195A/GCC_ARM/connected-lights-cloud.bin` to your development board.

##### ST NUCLEO-F429ZI

TBD

##### u-blox EVK-ODIN-W2

TBD

#### Creating the updated firmware

When your board is back online in mbed Cloud, you can then prepare an update. Open `main.cpp`, and change the `pir_rise()` function to:

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

##### Uploading the firmware to mbed Cloud

To schedule an update, you need to upload the firmware to mbed Cloud.

1. Log in to [mbed Cloud Portal](https://portal.mbedcloud.com/login).
1. Go to *Firmware management* > *Images*.
1. Click *Upload new images*.
1. Enter a descriptive name.
1. Select the firmware:
    * On FRDM-K64F, select `BUILD/K64F/GCC_ARM/connected-lights-cloud.bin`.
    * On RTL8195A, select `BUILD/RTL8195A/GCC_ARM/connected-lights-cloud-ota.bin`.
    * Other platforms: TBD.
1. Click *Upload*.

After the upload succeeds, find the URL to your firmware file on the overview page.

##### Creating an update manifest

Every firmware update requires an update manifest. This update contains the cryptographic hash of the firmware, signed with your certificate. It also contains information about which devices this update applies to, so you don't accidentally update devices with incompatible firmware.

To generate a new update manifest, run:

```
$ simple-cloud-client/tools/manifest-tool/bin/manifest-tool create -u http://path-to-your-firmware -o connected-lights.manifest
```

##### Uploading the manifest to mbed Cloud

To upload the manifest to mbed Cloud:

1. Go to *Firmware management* > *Manifests*.
1. Click *Upload new manifest*.
1. Enter a descriptive name.
1. Select the manifest (`connected-lights.manifest`).
1. Click *Upload*.

##### Creating an update campaign

To apply the firmware update, you need to start an update campaign. The campaign holds information on the devices that you need to updated, and what manifest you need to used. To create a campaign, you first need to create a device filter, which holds the list of devices that you need to update.

###### Creating a device filter

In the mbed Cloud Portal:

1. Go to *Device management*.
1. Click *Create new filter*.
1. Click *Add attribute*.
1. Select 'Device ID'.
1. Enter your device ID. (Look under *Developer Tools* > *Connectivity inspector* to find your device ID.)
1. Give the filter a descriptive name, and save the filter.

###### Starting the campaign

With the firmware, the manifest and the device filter in place, you can start the firmware update campaign.

1. Go to *Firmware management* > *Update campaigns*.
1. Click *Create campaign*.
1. Give the campaign a descriptive name.
1. Select the manifest you uploaded.
1. Select the filter you created.
1. Click *Save*.

To start the campaign, click *Start*.

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
