var TOKEN = 'YOUR_ACCESS_TOKEN';

var konekuta = require('konekuta');
var express = require('express');
var app = express();
var server = require('http').Server(app);
var io = require('socket.io')(server);
var hbs = require('hbs');

hbs.registerPartials(__dirname + '/views/partials');
app.use(express.static(__dirname + '/public'));
app.set('view engine', 'html');
app.set('views', __dirname + '/views');
app.engine('html', hbs.__express);

if (!process.env.TOKEN && TOKEN === 'YOUR_ACCESS_TOKEN') {
  throw 'Please set your access token first in main.js!';
}

function mapToView(d) {
  var hex = Number(d.color).toString(16);
  var model = {
    name: d.endpoint,
    rawColor: d.color,
    color: '#' + '000000'.substring(0, 6 - hex.length) + hex,
    motionClass: d.status == 0 ? 'selected' : '',
    onClass: d.status == 1 ? 'selected' : '',
    offClass: d.status == 2 ? 'selected' : '',
    timeout: d.timeout,
    status: d.status
  };

  var html = hbs.handlebars.compile('{{> device}}')(model);

  return { model: model, html: html };
}

var options = {
  endpointType: 'light-system',
  token: TOKEN,
  io: io,
  retrieve: {
    status: 'led/0/permanent_status',
    timeout: 'led/0/timeout',
    color: 'led/0/color',
    count: 'pir/0/count'
  },
  subscribe: {
    count: 'pir/0/count'
  },
  updates: {
    status: {
      method: 'put',
      path: 'led/0/permanent_status'
    },
    timeout: {
      method: 'put',
      path: 'led/0/timeout'
    },
    color: {
      method: 'put',
      path: 'led/0/color'
    }
  },
  mapToView: mapToView,
  verbose: true
};

konekuta(options, (err, devices, ee, connector) => {
  if (err) {
    throw err;
  }

  server.listen(process.env.PORT || 5265, process.env.HOST || '0.0.0.0', function() {
    console.log('Web server listening on port %s!', process.env.PORT || 5265);
  });

  app.get('/', function(req, res, next) {
    res.render('index', { devices: devices.map(d => mapToView(d).model) });
  });
});
