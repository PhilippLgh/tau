const EventEmitter = require('events').EventEmitter

const app = new EventEmitter()

setTimeout(() => {
    app.emit('ready')
}, 5 * 1000)

module.exports = app