// Wraps the binary addon with JS-side features, like
// field immutability, streaming, events, and more!

// The module
try {
  var mod = require('../build/Debug/gitteh')
} catch (e) {
  var mod = require('../build/Release/gitteh')
}
module.exports = mod;

// TODO: do the work here

