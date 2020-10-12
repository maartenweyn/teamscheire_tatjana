var storage = {
    db: null,
    average: 0.0,
    openDatabase: function(callback) {
        storage.db = window.sqlitePlugin.openDatabase({
            name: 'tatysound.db',
            location: 'default',
          });

        //storage.dropTable();
        storage.initdb(callback);
    },
    initdb: function(callback)  {
      storage.db.transaction(function(tx) {
        tx.executeSql('CREATE TABLE IF NOT EXISTS Measurements (timestamp, dbA, minutes, processed, queuelength)');
        tx.executeSql('CREATE TABLE IF NOT EXISTS Sound (timestamp, dbA, leq, minutes, processed, queuelength, protection)');
        tx.executeSql('CREATE TABLE IF NOT EXISTS Status (timestamp, action, action_value)');
        tx.executeSql('CREATE TABLE IF NOT EXISTS Upload (timestamp, status, min, hour, hours8, day, hours8dose, daydose, length, protection)');
        tx.executeSql('CREATE UNIQUE INDEX IF NOT EXISTS timestamp_indx ON Measurements (timestamp)');
        tx.executeSql('CREATE UNIQUE INDEX IF NOT EXISTS timestamp_indx ON Sound (timestamp)');
        tx.executeSql('CREATE UNIQUE INDEX IF NOT EXISTS timestamp_indx ON Status (timestamp)');
        tx.executeSql('CREATE UNIQUE INDEX IF NOT EXISTS timestamp_indx ON Upload (timestamp)');
      }, function(error) {
          console.log('Transaction ERROR: ' + error.message);
      }, function() {
          console.log('Created database OK');
          if (callback)
            callback();
      });
    },
    dropTable: function() {
        storage.db.transaction(function(tx) {
          tx.executeSql('DROP TABLE IF EXISTS Measurements');
          tx.executeSql('DROP TABLE IF EXISTS Sound');
          tx.executeSql('DROP TABLE IF EXISTS Status');
          tx.executeSql('DROP TABLE IF EXISTS Upload');
        }, function(error) {
            console.log('Transaction ERROR: ' + error.message);
        }, function() {
            console.log('Table removed');
            storage.initdb(null);
        });
    },
    addSoundMeasurementEntry(timestamp, dbValue, nrofMinutes, queuelength) {
      storage.db.transaction(function(tx) {
          tx.executeSql('INSERT INTO Measurements VALUES (?,?,?,0,?)', [timestamp, dbValue, nrofMinutes, queuelength]);
        }, function(error) {
          console.log('Transaction ERROR: ' + error.message);
        }, function() {
          console.log('Inserted values: ' + timestamp + ", " + dbValue  + ", " + nrofMinutes + ",0," + queuelength);
        });
    },
    getUnprocessedMeasurementEntry(callback) {
        storage.db.transaction(function(tx) {
            tx.executeSql('SELECT * FROM Measurements WHERE processed == 0 ORDER BY timestamp ASC', [], function(tx, rs) {
              console.log('SELECT getUnprocessedMeasurementEntry: ' + rs.rows.length);
              callback(rs.rows, rs.rows.length);
            }, function(tx, error) {
              console.log('SELECT error: ' + error.message);
              callback(0, 0);
            });
          });
    },
    setProcessedMeasurement(timestamp) {
        storage.db.transaction(function(tx) {
            tx.executeSql('UPDATE Measurements SET processed=1 WHERE timestamp==?', [timestamp]);
          }, function(error) {
            console.log('setProcessedMeasurement Transaction ERROR: ' + error.message);
          }, function() {
            console.log('setProcessedMeasurement set ' + timestamp);
          });
    },
    addSoundEntry(timestamp, dbValue, leqValue, nrofMinutes, queuelength, protection) {
        storage.db.transaction(function(tx) {
            tx.executeSql('INSERT INTO Sound VALUES (?,?,?,?,0,?,?)', [timestamp, dbValue, leqValue, nrofMinutes, queuelength, protection]);
          }, function(error) {
            console.log('Transaction ERROR: ' + error.message);
          }, function() {
            console.log('Inserted values: ' + timestamp + ", " + dbValue + ", " + leqValue + ", " + nrofMinutes + ",0," + queuelength + ", " + protection );
          });
    },
    getUnprocessedSoundEntry(callback) {
        storage.db.transaction(function(tx) {
            tx.executeSql('SELECT * FROM Sound WHERE processed == 0 ORDER BY timestamp ASC', [], function(tx, rs) {
              console.log('SELECT getUnprocessedSoundEntry: ' + rs.rows.length);
              callback(rs.rows, rs.rows.length);
            }, function(tx, error) {
              console.log('SELECT error: ' + error.message);
              callback(0, 0);
            });
          });
    },
    setProcessed(timestamp) {
        storage.db.transaction(function(tx) {
            tx.executeSql('UPDATE Sound SET processed=1 WHERE timestamp==?', [timestamp]);
          }, function(error) {
            console.log('setProcessed Transaction ERROR: ' + error.message);
          }, function() {
            console.log('setProcessed set ' + timestamp);
          });
    },
    getAverage(noise_data, from, to, callback) {
      storage.db.transaction(function(tx) {
        tx.executeSql('SELECT SUM(leq * minutes) / SUM(minutes) as average, COUNT(*) as nrofitems FROM Sound WHERE timestamp >= ? and timestamp <= ?', [from, to], function(tx, rs) {
            average = Math.round(1000 * Math.log10(rs.rows.item(0).average)) / 100;
            console.log('Average leq is : ' + average + ' from ' + from + ' to ' + to + ' of ' + rs.rows.item(0).nrofitems + ' items');
            callback(noise_data, rs.rows.item(0).average, average);
        }, function(tx, error) {
          console.log('SELECT error: ' + error.message);
          callback(0);
        });
      });
    },
    addUploadEntry(timestamp, status, min, hour, hours8, day, hours8dose, daydose, length, protection) {
        storage.db.transaction(function(tx) {
            tx.executeSql('INSERT INTO Upload VALUES (?,?,?,?,?,?,?,?,?,?)', [timestamp, status, min, hour, hours8, day, hours8dose, daydose, length, protection]);
          }, function(error) {
            console.log('Transaction ERROR: ' + error.message);
          }, function() {
            console.log('Inserted values: ' + timestamp + ", " + status + ", " + min + ", " + hour +  ", " + hours8 + ", " + day + ", " + hours8dose + ", " + daydose + ", " + length + ", " + protection);
          });
    },
    setUploadStatus(timestamp, status) {
        storage.db.transaction(function(tx) {
            tx.executeSql('UPDATE Upload SET status==? WHERE timestamp==?', [status, timestamp]);
          }, function(error) {
            console.log('setUploadStatus Transaction ERROR: ' + error.message);
          }, function() {
            console.log('setUploadStatus changed ' + timestamp + ", "  + status);
          });
    },
    getProcessedDataEntries(from, callback) {
      var to = from + 900000; //3600000;
      console.log("getProcessedDataEntries from " + from + " to " + to);
      storage.db.transaction(function(tx) {
        tx.executeSql('SELECT * FROM Upload WHERE timestamp >= ? and timestamp <= ? ORDER BY timestamp DESC LIMIT 15', [from, to], function(tx, rs) {
            callback(rs.rows.length, rs.rows, to);
        }, function(tx, error) {
          console.log('SELECT error: ' + error + ' / ' + error.message);
          callback(0, 0, 0);
        });
      });
    },
    getUnploadedEntries(limit, callback) {
        storage.db.transaction(function(tx) {
            tx.executeSql('SELECT * FROM Upload WHERE status == 0 LIMIT ?', [limit], function(tx, rs) {
                callback(rs.rows);
            }, function(tx, error) {
              console.log('SELECT error: ' + error.message);
              callback(0);
            });
          });
    },
    addStatusEntry(timestamp, action, action_value) {
      storage.db.transaction(function(tx) {
          tx.executeSql('INSERT INTO Status VALUES (?,?,?)', [timestamp, action, action_value]);
        }, function(error) {
          console.log('Transaction ERROR: ' + error.message);
        }, function() {
          console.log('Inserted Status: ' + timestamp + ", " + action + ", " + action_value);
        });
    },
    getStatus(ts, action, callback, data) {
      console.log("getStatus of " + action + " before " + ts);
      storage.db.transaction(function(tx) {
        tx.executeSql('SELECT action_value FROM Status WHERE action == ? and timestamp <= ? ORDER BY timestamp DESC LIMIT 1', [action, ts], function(tx, rs) {
          if (rs.rows.length == 1) {
            callback(1, rs.rows.item(0).action_value, data);
          } else {
            callback(0, 0, data);
          }
        }, function(tx, error) {
          console.log('SELECT error: ' + error + ' / ' + error.message);
          callback(0, 0, data);
        });
      });
    },

    getLatestStatus(action, callback) {
      console.log("getLatestStatus of " + action);
      storage.db.transaction(function(tx) {
        tx.executeSql('SELECT * FROM Status WHERE action == "?" ORDER BY timestamp DESC LIMIT 1', [action], function(tx, rs) {
            callback(rs.rows.length, rs.rows);
        }, function(tx, error) {
          console.log('SELECT error: ' + error + ' / ' + error.message);
          callback(0, 0);
        });
      });
    },
    setItem: function (referenceName, object) {
        try {
            var objectAsString = JSON.stringify(object);
            window.localStorage.setItem(referenceName, objectAsString);
            debug.log('item stored: ' + referenceName + ': ' + objectAsString, 'success');
            return true;
        } catch (error) {
            console.log(error);
        }
    },
    getItem: function (referenceName, defaultValue) {
        try {
            var objectAsString = window.localStorage.getItem(referenceName);
            var object = JSON.parse(objectAsString);
            return (object) ? object : defaultValue;
        } catch (error) {
            console.log(error);
        }
    },
    removeItem: function (referenceName) {
        try {
            window.localStorage.removeItem(referenceName);
            debug.log('stored item removed: ' + referenceName, 'success');
            return true;
        } catch (error) {
            console.log(error);
        }
    }
}