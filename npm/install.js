#!/usr/bin/env node

var version = require('./package').version

var fs = require('fs')
var os = require('os')
var path = require('path')
var extract = require('extract-zip')
var download = require('./tau-download')

function onerror (err) {
  throw err
}

// downloads if not cached
download()
.then(filepath => {
  console.log('extract zip', filepath)
  extractFile(filepath)
})
.catch(onerror)

// unzips and makes path.txt point at the correct executable
function extractFile (zipPath) {
  extract(zipPath, {dir: path.join(__dirname, 'dist')}, function (err) {
    if (err) return onerror(err)
    /*
    fs.writeFile(path.join(__dirname, 'path.txt'), platformPath, function (err) {
      if (err) return onerror(err)
    })
    */
  })
}

