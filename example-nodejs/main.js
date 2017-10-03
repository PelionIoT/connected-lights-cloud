var TOKEN = 'YOUR_ACCESS_TOKEN';

var mbed = require('mbed-cloud-sdk');
var api = new mbed.DevicesApi({
    apiKey: process.env.TOKEN || TOKEN,
    host: 'https://api.us-east-1.mbedcloud.com'
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
