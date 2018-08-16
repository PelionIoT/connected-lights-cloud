## Bringing it all together

You’ve already built almost all of the pieces required for the lighting system. You now have:

* The circuitry.
* The firmware.
* A connection to Device Management.
* A way of talking from a Node.js or Python app to the device.

You can now combine everything by building a web app that allows you to control the lights from anywhere in the world. For convenience, we built a Node.js mobile web app for you, which is in the `connected-lights-cloud` folder. The web app has the following features:

* Lists all the lights under your account.
* Changes the status of the light (from motion sensing to on/off).
* Quickly changes colors.
* Gets notifications whenever the PIR sensor detects movement.

Here is a screenshot of the app running on an Android phone:

![Screenshot of the light control interface](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights15.png)

*This is what the mobile interface looks like with two lights connected. Tap the color to change it.*

<span class="notes">**Note:** To give the lights a more friendly name (such as "living room"), change the name of the device in the Pelion Portal.</span>

### Running the application

To start the application, first open `connected-lights-cloud/webapp/main.js`, and paste your Device Management API key in the first line.:

1. Open a terminal or command prompt.
1. Navigate to `connected-lights-cloud/webapp`.
1. Install the dependencies:

    ```
    $ npm install
    ```

1. Run the application:

    ```
    $ node main.js
    ```

Open your web browser, and go to [http://localhost:5265](http://localhost:5265) to see the application running.

### Konekuta

The web application is built on top of [Konekuta](https://github.com/armmbed/konekuta), a framework for building dynamic web applications on top of Device Management. Konekuta solves a number of common problems with building connected applications, including state syncing, going offline/online, handling errors and updating the UI when devices connect or disconnect from Device Management. Konekuta is open source and available on [GitHub](https://github.com/armmbed/konekuta).
