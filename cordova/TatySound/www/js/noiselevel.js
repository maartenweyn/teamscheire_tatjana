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
    sound_data: [],
    posturl : "",
    chart_data_points: 8 * 60,
    is_uploading: false,
    ref_date: new Date('1/1/2019'),
    prev_update_date: new Date('1/1/2019'),
    token: "",
    initialize: function () {
      noiselevel.token = device.uuid.substr(0, 20);
      noiselevel.posturl = "http://teamscheire.wesdec.be:8080/api/v1/" + noiselevel.token + "/telemetry";
      debug.log("UUID " + device.uuid, "success");
      debug.log("posturl " + noiselevel.posturl , "success");
    },
    avgHourCallback: function(noise_data, value, valuedb) {
      noise_data.hour = valuedb;
      console.log("leq_hour:" + noise_data.hour);
        
      storage.getAverage(noise_data, noise_data.ts - (8 * 360000), noise_data.ts, noiselevel.avg8HourCallback);
    },
    avg8HourCallback: function(noise_data, value, valuedb) {
      noise_data.hours8 = valuedb;
      noise_data.hours8dose = Math.round(100*value/100000000);
      console.log("leq_8hours:" + noise_data.hours8 + ", " + noise_data.hours8dose);
        
      storage.getAverage(noise_data, noise_data.ts - (24 * 360000), noise_data.ts, noiselevel.avgDayCallback);
    },
    avgDayCallback: function(noise_data, value, valuedb) {
      noise_data.day = valuedb;
      noise_data.daydose = Math.round(100*value/31622777);
      console.log("leq_day:" + noise_data.day + ", " + noise_data.daydose);
      noiselevel.processNoiseData(noise_data);
      //for testing
      if (!noiselevel.is_uploading) {
        noiselevel.checkUploadData();
      }
    },
    unuploadedDataCallback: function(rows) {
        if (rows.length != 0) {
            console.log("unuploadedDataCallback nr of rows " + rows.length + " " + rows.item(0).timestamp);
            var json = {
                //ts: ts,
                ts: rows.item(0).timestamp,
                values: {
                    min: rows.item(0).min,
                    hour: rows.item(0).hour,
                    hours8: rows.item(0).hours8,
                    day: rows.item(0).day,
                    hours8dose: rows.item(0).hours8dose,
                    daydose: rows.item(0).daydose,
                    id: rows.item(0).id,
                    length: rows.item(0).length,
                    upload: rows.length
                }
            };
            noiselevel.uploadNoiseData(json);
        } else {
          noiselevel.is_uploading = false;
          console.log("unuploadedDataCallback no rows");
        }
    },
    processNoiseData: function(noise_data) {
      noiselevel.sound_data = noise_data;
      storage.addUploadEntry(noise_data.ts, 0, noise_data.min, noise_data.hour, noise_data.hours8, noise_data.day, noise_data.hours8dose, noise_data.daydose, noise_data.id, noise_data.length);
    
      noiselevel.addSoundToChart(noise_data.ts, noise_data.min, noise_data.hour, noise_data.hours8, noise_data.day, noise_data.hours8dose, noise_data.daydose);
      sound_chart.redraw();
    },
    addSoundToChart: function(ts, min, hour, hours8, day, hours8dose, daydose) {  
      var shift = sound_chart.series[0].data.length > noiselevel.chart_data_points;
      sound_chart.series[0].addPoint([ts, min], false, shift);
      sound_chart.series[1].addPoint([ts, hour], false, shift);
      sound_chart.series[2].addPoint([ts, hours8], false, shift);
      sound_chart.series[3].addPoint([ts, day], false, shift);
      sound_chart.series[4].addPoint([ts, hours8dose], false, shift);
      sound_chart.series[5].addPoint([ts, daydose], false, shift);
    },
    soundDataCallback: function(rows) {
      if (rows.length != 0) {
        console.log("soundDataCallback nr of rows " + rows.length);
        var i;
        for (i = 0; i < rows.length; i++) {  
          console.log("soundDataCallback row " + i + ": "+ rows.item(i).timestamp);
          noiselevel.addSoundToChart(rows.item(i).timestamp, rows.item(i).min, rows.item(i).hour, rows.item(i).hours8, rows.item(i).day, rows.item(i).hours8dose, rows.item(i).daydose);
        } 
        sound_chart.redraw();
      } else {
        console.log("unuploadedDataCallback no rows");
      }
    },
    fillChart: function() {
      storage.getProcessedDataEntries(noiselevel.soundDataCallback, new Date() - (24 * 360000), new Date());
    },
    checkUploadData: function() {
        noiselevel.is_uploading = true;
        storage.getUnploadedEntries(noiselevel.unuploadedDataCallback);
    },
    uploadNoiseData: function(json) {
        storage.callbackcount = 0;
        console.log(JSON.stringify(json));

        //curl  -X POST -H "Content-Type: application/json" -d '{"ts":1569580418000,"values":{"sound_level":"21.6","min":"31.4","hour":"36.4","hours8":"36.6","day":"36.6","id":390,"length":0,"resp":0}}' http://teamscheire.wesdec.be:8080/api/v1/9MoLAEg7QEqU5TDjCH2k/telemetry -v

        const options = {
            method: 'post',
            data: json,
            headers: {}
            };
        cordova.plugin.http.setDataSerializer('json');
        debug.log("sendRequest: "+ noiselevel.posturl + " with " + options);
        cordova.plugin.http.sendRequest(noiselevel.posturl, options, function(response) {
            // prints 200
            debug.log("POST RESPONSE: " + response.status + " for id " + options.data.values.id + " on ts " + options.data.ts, "succes");
            noiselevel.upload_status = true;
            storage.setUploadStatus(options.data.ts, options.data.values.id, 1);
            if (options.data.values.upload > 1) 
            {
                noiselevel.checkUploadData();
            } else {
              noiselevel.is_uploading = false;
            }
            }, function(response) {
            // prints 403
            debug.log("POST RESPONSE: " + response.status  + " " + response.error, "error");
            noiselevel.upload_status = false;
            noiselevel.is_uploading = false;
        });

        //noiselevel.showNoiseLevel();
        console.log("uploadNoiseData done");
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
              noiselevel.prev_update_date = new Date('1/1/2019');
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

            var noise_data = {
              ts: noiselevel.ts,
              min: noiselevel.leq_min,
              hour: 0,
              hours8: 0,
              day: 0,
              hours8dose: 0,
              daydose: 0,
              id: noiselevel.id,
              length: length_of_data_entries
          };

            storage.callbackcount = 0;
            storage.getAverage(noise_data, noiselevel.ts - 360000, noiselevel.ts, noiselevel.avgHourCallback);

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

        $('#soundlevelinfo').html('Device ID: ' + noiselevel.token  + '<br />' +
            'Timestamp: ' + formattedTime + '<br />' +
            'Last ID: ' + noiselevel.sound_data.id + '<br />' +
            'Last leq: ' + noiselevel.sound_level + ' dBA <br />' +
            'Last minute: ' + noiselevel.sound_data.min + ' dBA <br />' +
            'Last hour: ' + noiselevel.sound_data.hour + ' dBA <br />' +
            'Last 8 Hours: ' + noiselevel.sound_data.hours8 + ' dBA <br />' +
            'Last Day: ' + noiselevel.sound_data.day + ' dBA <br />'+
            'Noise Dose 8 Hours: ' + noiselevel.sound_data.hours8dose + '%<br />'+
            'Noise dose Last Day: ' + noiselevel.sound_data.daydose + '% <br />'+
            'Upload status: ' + noiselevel.upload_status + '<br />');
    }
}