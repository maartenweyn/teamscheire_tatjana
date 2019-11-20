var noiselevel = {
    ts: 0,
    id: 0,
    sound_level: 0,
    leq_min : 0.0,
    leq_hour: 0.0,
    leq_8hours: 0.0,
    leq_day: 0.0,
    leq: 0.0,
    hours8dose: 0,
    daydose: 0,
    corrected_leq: 0.0,
    prev_length :0,
    prev_ts: {ts:0, sensor_ts:0},
    upload_status: false,
    data_length:0,
    callbackcount : 0,
    //sound_data: [],
    posturl : "",
    ref_date: new Date('1/1/2019'),
    prev_update_date: new Date('1/1/2019'),
    initialize: function () {
        noiselevel.posturl = "http://teamscheire.wesdec.be:8080/api/v1/" + device.uuid + "/telemetry";
        debug.log("UUID " + device.uuid, "success");
        debug.log("posturl " + noiselevel.posturl , "success");
    },
    avgHourCallback: function(value, valuedb) {
        noiselevel.leq_hour = valuedb;
        console.log("leq_hour:" + noiselevel.leq_hour);
    },
    avg8HourCallback: function(value, valuedb) {
        noiselevel.leq_8hours = valuedb;
        noiselevel.hours8dose = Math.round(100*value/100000000);
        console.log("leq_8hours:" + noiselevel.leq_8hours + ", " + noiselevel.hours8dose);
    },
    avgDayCallback: function(value, valuedb) {
        noiselevel.leq_day = valuedb;
        noiselevel.daydose = Math.round(100*value/31622777);
        console.log("leq_day:" + noiselevel.leq_day + ", " + noiselevel.daydose);
        noiselevel.processNoiseData();
    },
    processNoiseData: function() {
        addUploadEntry(noiselevel.ts, 0, noiselevel.leq_min, noiselevel.leq_hour, noiselevel.leq_8hours, noiselevel.leq_day, noiselevel.hours8dose, noiselevel.daydose, noiselevel.id, noiselevel.data_length);
    },
    uploadNoiseData: function() {
        storage.callbackcount = 0;
        var response  = 0;
        var json = {
            //ts: ts,
            ts: noiselevel.ts,
            values: {
                min: noiselevel.leq_min.toFixed(1),
                hour: noiselevel.leq_hour.toFixed(1),
                hours8: noiselevel.leq_8hours.toFixed(1),
                day: noiselevel.leq_day.toFixed(1),
                id: noiselevel.id,
                length: noiselevel.data_length,
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

        noiselevel.showNoiseLevel();
    },
    parseData: function (datastring) {
        var res = datastring.split(',');

        if (res[0] == '1') {
            // if (res.length != 10) {
            //     debug.log("incorrect message length " + res.length + ": " + datastring, "error");
            //     console.log("Array: " + res);
            //     return;
            // }


            var length_of_data_entries      = parseInt(res[3]);
            noiselevel.data_length = length_of_data_entries;


            noiselevel.ts = parseInt(res[1])*1000;
            if (noiselevel.ts < 26265600)
            {
                if (length_of_data_entries == 1)
                {
                    noiselevel.ts = (new Date()).getTime();
                } else {
                    if (noiselevel.prev_length == 0) {
                        noiselevel.ts = (new Date()).getTime();
                        noiselevel.prev_ts.ts = noiselevel.ts;
                        noiselevel.prev_ts.sensor_ts = noiselevel.ts;
                        noiselevel.prev_length = length_of_data_entries
                    } else {
                        noiselevel.ts = noiselevel.prev_ts.ts - (noiselevel.prev_ts.sensor_ts - noiselevel.ts);
                        noiselevel.prev_length = length_of_data_entries;
                    }
                }
            } else {
                noiselevel.ts += (noiselevel.ref_date).getTime();
            }

            //e.g. 0,4300,859,53.8,57.8,54.0,56.9,57.0,0
             //parseInt(res[1]);
            noiselevel.id          = parseInt(res[1]);
            noiselevel.leq_min = parseInt(res[2]) / 100.0;
            noiselevel.sound_level = noiselevel.leq_min;

            storage.addSoundEntry(noiselevel.ts, noiselevel.leq_min, 1);

            storage.callbackcount = 0;
            storage.getAverage(noiselevel.ts - 360000, noiselevel.ts, noiselevel.avgHourCallback);
            storage.getAverage(noiselevel.ts - (8 * 360000), noiselevel.ts, noiselevel.avg8HourCallback);
            storage.getAverage(noiselevel.ts - (24 * 360000), noiselevel.ts, noiselevel.avgDayCallback);

            
        } else if (res[0] == "0") {
            noiselevel.sound_level = parseInt(res[2]) / 100.0;
            //noiselevel.corrected_leq = parseFloat(res[2]);
            //console.log("leq " + noiselevel.leq.toFixed(1) + ", " + "corr " + noiselevel.corrected_leq.toFixed(1) );
            noiselevel.showNoiseLevel();
        } else {
            debug.log("unkown message type " + datastring, "error");
        }
        
    },
    
    showNoiseLevel: function () {

        if (cordova.plugins.backgroundMode.isActive()) 
            return;
            
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
            'Corrected: ' + noiselevel.corrected_leq + ' dBA <br />'+
            'Upload status: ' + noiselevel.upload_status + '<br />');
    }
}