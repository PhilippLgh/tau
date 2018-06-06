const BrowserWindow = process._linkedBinding('BrowserWindow').BrowserWindow

/*
const EventEmitter = require('events').EventEmitter

class BrowserWindow extends EventEmitter {
    constructor() {
        super()
        console.log('hello window')
    }

    loadFile(filePath) {
        console.log('load file', filePath)
    }
}
*/

module.exports = BrowserWindow