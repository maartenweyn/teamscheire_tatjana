var storage = {
    db: null,
    average: 0.0,
    openDatabase: function(callback) {
        storage.db = window.sqlitePlugin.openDatabase({
            name: 'tatysound.db',
            location: 'default',
          });

        //storage.dropTable();

        storage.db.transaction(function(tx) {
            tx.executeSql('CREATE TABLE IF NOT EXISTS Sound (timestamp, dbA, leq, minutes, processed, queuelength)');
            tx.executeSql('CREATE TABLE IF NOT EXISTS Status (timestamp, action)');
            tx.executeSql('CREATE TABLE IF NOT EXISTS Upload (timestamp, status, min, hour, hours8, day, hours8dose, daydose, length)');
        }, function(error) {
            console.log('Transaction ERROR: ' + error.message);
        }, function() {
            console.log('Created database OK');
            callback();
        });
    },
    dropTable: function() {
        storage.db.transaction(function(tx) {
            tx.executeSql('DROP TABLE IF EXISTS Sound');
            tx.executeSql('DROP TABLE IF EXISTS Status');
            tx.executeSql('DROP TABLE IF EXISTS Upload');
        }, function(error) {
            console.log('Transaction ERROR: ' + error.message);
        }, function() {
            console.log('Table removed');
        });
    },
    addSoundEntry(timestamp, dbValue, leqValue, nrofMinutes, queuelength) {
        storage.db.transaction(function(tx) {
            tx.executeSql('INSERT INTO Sound VALUES (?,?,?,?,0,?)', [timestamp, dbValue, leqValue, nrofMinutes, queuelength]);
          }, function(error) {
            console.log('Transaction ERROR: ' + error.message);
          }, function() {
            console.log('Inserted values: ' + timestamp + ", " + dbValue + ", " + leqValue + ", " + nrofMinutes + ",0," + queuelength);
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
        tx.executeSql('SELECT SUM(leq * minutes) / SUM(minutes) as average, COUNT(*) as nrofitems  FROM Sound WHERE timestamp >= ? and timestamp <= ?', [from, to], function(tx, rs) {
            average = Math.round(1000 * Math.log10(rs.rows.item(0).average)) / 100;
            console.log('Average leq is : ' + average + ' from ' + from + ' to ' + to + ' of ' + rs.rows.item(0).nrofitems + ' items');
            callback(noise_data, rs.rows.item(0).average, average);
        }, function(tx, error) {
          console.log('SELECT error: ' + error.message);
          callback(0);
        });
      });
    },
    addUploadEntry(timestamp, status, min, hour, hours8, day, hours8dose, daydose, length) {
        storage.db.transaction(function(tx) {
            tx.executeSql('INSERT INTO Upload VALUES (?,?,?,?,?,?,?,?,?)', [timestamp, status, min, hour, hours8, day, hours8dose, daydose, length]);
          }, function(error) {
            console.log('Transaction ERROR: ' + error.message);
          }, function() {
            console.log('Inserted values: ' + timestamp + ", " + status + ", " + min + ", " + hour +  ", " + hours8 + ", " + day + ", " + hours8dose + ", " + daydose + ", " + length);
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
    getProcessedDataEntries(callback) {
      console.log("getProcessedDataEntries");
      storage.db.transaction(function(tx) {
        tx.executeSql('SELECT * FROM Upload ORDER BY timestamp DESC LIMIT 300', [], function(tx, rs) {
            callback(rs.rows.length, rs.rows);
        }, function(tx, error) {
          console.log('SELECT error: ' + error + ' / ' + error.message);
          callback(0, 0);
        });
      });
    },
    getUnploadedEntries(callback) {
        storage.db.transaction(function(tx) {
            tx.executeSql('SELECT * FROM Upload WHERE status == 0', [], function(tx, rs) {
                callback(rs.rows);
            }, function(tx, error) {
              console.log('SELECT error: ' + error.message);
              callback(0);
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