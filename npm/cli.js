#!/usr/bin/env node

var tau = require('./')

var proc = require('child_process')
let args = process.argv.slice(2)
console.log("start tau with ", process.cwd(), tau, args, process.argv)
var child = proc.spawn(tau, args, {stdio: 'inherit'})
child.on('close', function (code) {
  process.exit(code)
})

child.on('error', function(err){
  console.error(err)
})