cordova.define("cordova-background-timer.BackgroundTimer", function(require, exports, module) {
var exec = require("cordova/exec");
module.exports = {
    onTimerEvent: function(success) {
        exec(success || function() {},
             function() {},
             'BackgroundTimer',
             'onTimerEvent',
             []);
    },
	start: function(success, failure, config) {
        exec(success || function() {},
             failure || function() {},
             'BackgroundTimer',
             'start',
             [config]);
    },
    stop: function(success, failure) {
        exec(success || function() {},
            failure || function() {},
            'BackgroundTimer',
            'stop',
            []);
    }
};

});
