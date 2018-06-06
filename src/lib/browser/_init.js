'use strict'

const fs = require('fs')
const path = require('path')
const Module = require('module')

// We modified the original process.argv to let node.js load the entrypoint.js,
// we need to restore it here.
process.argv.splice(1, 1)

// Clear / patch search paths.
require('../common/reset-search-paths')

// Don't quit on fatal error.
process.on('uncaughtException', function (error) {
    // Do nothing if the user has a custom uncaught exception handler.
    var dialog, message, ref, stack
    /*
    if (process.listeners('uncaughtException').length > 1) {
        return
    }
    */

    // Show error in GUI.
    // dialog = require('electron').dialog
    stack = (ref = error.stack) != null ? ref : error.name + ': ' + error.message
    message = 'Uncaught Exception:\n' + stack
    // dialog.showErrorBox('A JavaScript error occurred in the main process', message)
    console.log('A JavaScript error occurred in the main process', message)

})

// FIXME if everything works correctly this path is set in c++
process.resourcesPath = path.join(__dirname,"../../../Resources")
console.log('resource path is: ', process.resourcesPath)
// Now we try to load app's package.json.
let packagePath = null
let packageJson = null
const searchPaths = ['app', 'app.asar', 'default_app.asar']
for (packagePath of searchPaths) {
    try {
        packagePath = path.join(process.resourcesPath, packagePath)
        packageJson = require(path.join(packagePath, 'package.json'))
        break
    } catch (error) {
        continue
    }
}

if (packageJson == null) {
    process.nextTick(function () {
        return process.exit(1)
    })
    throw new Error('Unable to find a valid app')
}

// Set main startup script of the app.
const mainStartupScript = packageJson.main || 'index.js'

// Finally load app's main.js and transfer control to C++.
Module._load(path.join(packagePath, mainStartupScript), Module, true)
