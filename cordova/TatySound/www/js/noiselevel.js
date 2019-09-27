var noiselevel = {
    ts: 0,
    id: 0,
    sound_level: 0,
    leq_min : 0.0,
    leq_hour: 0.0,
    leq_8hours: 0.0,
    leq_day: 0.0,
    leq: 0.0,
    corrected_leq: 0.0,
    upload_status: false,
    posturl : "",
    initialize: function () {
        noiselevel.posturl = "http://teamscheire.wesdec.be:8080/api/v1/" + device.uuid + "/telemetry";
        debug.log("UUID " + device.uuid, "success");
        debug.log("posturl " + noiselevel.posturl , "success");
    },

    parseData: function (datastring) {
        var res = datastring.split(',');

        if (res[0] == '0') {
            if (res.length != 10) {
                debug.log("incorrect message length " + res.length + ": " + datastring, "error");
                console.log("Array: " + res);
                return;
            }

            //e.g. 0,4300,859,53.8,57.8,54.0,56.9,57.0,0
            noiselevel.ts          = (new Date()).getTime(); //parseInt(res[1]);
            noiselevel.id          = parseInt(res[2]);
            noiselevel.sound_level = parseFloat(res[3]);
            noiselevel.leq_min     = parseFloat(res[4]);
            noiselevel.leq_hour    = parseFloat(res[5]);
            noiselevel.leq_8hours  = parseFloat(res[6]);
            noiselevel.leq_day     = parseFloat(res[7]);
            var response    = parseInt(res[8]);
            var length      = parseInt(res[9]);

            var json = {
                //ts: ts,
                ts: noiselevel.ts,
                values: {
                    sound_level: noiselevel.sound_level.toFixed(1),
                    min: noiselevel.leq_min.toFixed(1),
                    hour: noiselevel.leq_hour.toFixed(1),
                    hours8: noiselevel.leq_8hours.toFixed(1),
                    day: noiselevel.leq_day.toFixed(1),
                    id: noiselevel.id,
                    length: length,
                    resp: response
                }
            };
            console.log(JSON.stringify(json));

            //curl  -X POST -H "Content-Type: application/json" -d '{"ts":1569580418000,"values":{"sound_level":"21.6","min":"31.4","hour":"36.4","hours8":"36.6","day":"36.6","id":390,"length":0,"resp":0}}' http://teamscheire.wesdec.be:8080/api/v1/9MoLAEg7QEqU5TDjCH2k/telemetry -v

            const options = {
                method: 'post',
                data: json,
                headers: {}
              };
            cordova.plugin.http.setDataSerializer('json');
            cordova.plugin.http.sendRequest(noiselevel.posturl, options, function(response) {
                // prints 200
                debug.log("POST RESPONSE: " + response.status, "succes");
                noiselevel.upload_status = true;
                }, function(response) {
                // prints 403
                debug.log("POST RESPONSE: " + response.status, "error");
                noiselevel.upload_status = false;
            });
        } else if (res[0] == "1") {
            noiselevel.leq = parseFloat(res[1]);
            noiselevel.corrected_leq = parseFloat(res[2]);
            console.log("leq " + noiselevel.leq.toFixed(1) + ", " + "corr " + noiselevel.corrected_leq.toFixed(1) );
            noiselevel.showNoiseLevel();
        } else {
            debug.log("unkown message type " + datastring, "error");
        }
        
    },
    
    showNoiseLevel: function () {
        var date = new Date(noiselevel.ts);
        var hours = date.getHours();
        var minutes = "0" + date.getMinutes();
        var seconds = "0" + date.getSeconds();
        var formattedTime = hours + ':' + minutes.substr(-2) + ':' + seconds.substr(-2);


        $('#soundlevelinfo').html('Timestamp: ' + formattedTime + '<br />' +
            'Last ID: ' + noiselevel.id + '<br />' +
            'Last leq: ' + noiselevel.sound_level + ' dBA <br />' +
            'Last minute: ' + noiselevel.leq_min + ' dBA <br />' +
            'Last hour: ' + noiselevel.leq_hour + ' dBA <br />' +
            'Last 8 Hours: ' + noiselevel.leq_8hours + ' dBA <br />' +
            'Last Day: ' + noiselevel.leq_day + ' dBA <br />'+
            'leq: ' + noiselevel.leq + ' dBA <br />'+
            'Corrected: ' + noiselevel.corrected_leq + ' dBA <br />'+
            'Upload status: ' + noiselevel.upload_status + '<br />');
    }
}