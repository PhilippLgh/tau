#!/usr/bin/env node

var tauon = require('./')

var proc = require('child_process')

var child = proc.spawn(tauon, process.argv.slice(2), {stdio: 'inherit'})
child.on('close', function (code) {
  process.exit(code)
})