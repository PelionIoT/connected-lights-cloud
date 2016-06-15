var TOKEN = 'YOUR_ACCESS_TOKEN';

var CloudApi = require('mbed-connector-api');
var express = require('express');
var bodyParser = require('body-parser');
var fs = require('fs');
var Path = require('path');
var promisify = require('es6-promisify');
var app = express();
var server = require('http').Server(app);
var EventEmitter = require('events');
var io = require('socket.io')(server);

app.use(express.static(__dirname + '/public'));
app.set('view engine', 'html');
app.set('views', __dirname + '/views');
app.engine('html', require('hbs').__express);
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));

if (!process.env.TOKEN && TOKEN === 'YOUR_ACCESS_TOKEN') {
  throw 'Please set your access token first!';
}

var api = new CloudApi({
  accessKey: process.env.TOKEN || TOKEN
});

app.get('/', function(req, res, next) {
  api.getEndpoints(function(err, devices) {
    if (err) return next(err);

    var devicePromise = devices.map(function(device) {

      return new Promise(function(res, rej) {
        console.log(device.name, 'reading values from device');
        api.putResourceSubscription(device.name, '/pir/0/count', function(err) {
          if (err) return rej(err);

          api.getResourceValue(device.name, '/led/0/permanent_status', function(err, status) {
            if (err) return rej(err);
            console.log(device.name, 'status is', status);

            api.getResourceValue(device.name, '/led/0/timeout', function(err, timeout) {
              if (err) return rej(err);
              console.log(device.name, 'timeout is', timeout);

              api.getResourceValue(device.name, '/led/0/color', function(err, color) {
                if (err) return rej(err);
                console.log(device.name, 'color is', color);

                api.getResourceValue(device.name, '/pir/0/count', function(err, count) {
                  if (err) return rej(err);
                  console.log(device.name, 'count is', count);

                  res({
                    status: Number(status),
                    timeout: Number(timeout),
                    color: Number(color),
                    count: Number(count)
                  });
                });
              });
            });
          });
        });
      });
    });

    Promise.all(devicePromise).then(function(data) {

      var model = data.map(function(d, ix) {
        d.name = devices[ix].name;
        d.color = (d.color >> 16 & 0xff) + ', ' + (d.color >> 8 & 0xff) + ', ' + (d.color & 0xff);
        d.motionClass = d.status === 0 ? 'selected' : '';
        d.onClass = d.status === 1 ? 'selected' : '';
        d.offClass = d.status === 2 ? 'selected' : '';
        return d;
      });

      console.log('got data...', data);

      res.render('index', { devices: model });

    }).catch(function(err) {
      console.error('argh?!', err);
      return next(err);
    });
  }, { parameters: { type: 'light-system' } });
});

// socket.io
io.on('connection', function(socket) {
  socket.on('change-status', function(endpoint, newStatus) {
    console.log('change-status', endpoint, newStatus);
    api.putResourceValue(endpoint, '/led/0/permanent_status', newStatus);
  });
  socket.on('change-timeout', function(endpoint, newtimeout) {
    console.log('change-timeout', endpoint, newtimeout);
    api.putResourceValue(endpoint, '/led/0/timeout', newtimeout);
  });
  socket.on('change-color', function(endpoint, newcolor) {
    console.log('change-color', endpoint, newcolor);
    api.putResourceValue(endpoint, '/led/0/color', newcolor);
  });
});


// Start notification channel
api.startLongPolling(function(err) {
  if (err) throw 'Connection to mbed Cloud failed ' + err;

  server.listen(process.env.PORT || 5265, process.env.HOST || '0.0.0.0', function() {
    console.log('Web server listening on port %s!', process.env.PORT || 5265);
  });
});

// Notifications
api.on('notification', function(notification) {
  console.log('Got a notification', notification);
});
