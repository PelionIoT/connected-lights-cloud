var TOKEN = 'YOUR_ACCESS_TOKEN';

var CloudApi = require('mbed-connector-api');
var api = new CloudApi({
  accessKey: process.env.TOKEN || TOKEN
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
      api.putResourceSubscription(d.name, '/pir/0/count', function(err) {
        console.log('subscribed to resource', err);
      });

      // and set the color to orange, as your's truly is Dutch
      var orange = (255 << 16) + (100 << 8) + 0;
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
