var storage = {
    db: null,
    average: 0.0,
    openDatabase: function() {
        storage.db = window.sqlitePlugin.openDatabase({
            name: 'tatysound.db',
            location: 'default',
          });

        //storage.dropTable();

        storage.db.transaction(function(tx) {
            tx.executeSql('CREATE TABLE IF NOT EXISTS Sound (timestamp, dbA, leq, minutes)');
            tx.executeSql('CREATE TABLE IF NOT EXISTS Status (timestamp, action)');
            tx.executeSql('CREATE TABLE IF NOT EXISTS Upload (timestamp, status, min, hour, hours8, day, hours8dose, daydose, id, length)');
        }, function(error) {
            console.log('Transaction ERROR: ' + error.message);
        }, function() {
            console.log('Created database OK');
        });
    },
    dropTable: function() {
        storage.db.transaction(function(tx) {
            tx.executeSql('DROP TABLE IF EXISTS Sound');
        }, function(error) {
            console.log('Transaction ERROR: ' + error.message);
        }, function() {
            console.log('Table removed');
        });
    },
    addSoundEntry(timestamp, dbValue, nrofMinutes) {
        leqValue = Math.pow(10, dbValue/10);
        storage.db.transaction(function(tx) {
            tx.executeSql('INSERT INTO Sound VALUES (?,?,?,?)', [timestamp, dbValue, leqValue, nrofMinutes]);
          }, function(error) {
            console.log('Transaction ERROR: ' + error.message);
          }, function() {
            console.log('Inserted values: ' + timestamp + ", " + dbValue + ", " + leqValue + ", " + nrofMinutes);
          });
    },
    getAverage(from, to, callback) {
        storage.db.transaction(function(tx) {
            tx.executeSql('SELECT SUM(leq * minutes) / SUM(minutes) as average  FROM Sound WHERE timestamp >= ? and timestamp <= ?', [from, to], function(tx, rs) {
                average = Math.round(1000 * Math.log10(rs.rows.item(0).average)) / 100;
                console.log('Average leq is : ' + average);
                callback(rs.rows.item(0).average, average);
            }, function(tx, error) {
              console.log('SELECT error: ' + error.message);
              callback(0);
            });
          });
    },
    addUploadEntry(timestamp, status, min, hour, hours8, day, hours8dose, daydose, id, length) {
        storage.db.transaction(function(tx) {
            tx.executeSql('INSERT INTO Upload VALUES (?,?,?,?,?,?,?,?,?,?)', [timestamp, status, min, hour, hours8, day, hours8dose, daydose, id, length]);
          }, function(error) {
            console.log('Transaction ERROR: ' + error.message);
          }, function() {
            console.log('Inserted values: ' + timestamp + ", " + status + ", " + min + ", " + hour +  ", " + hours8 + ", " + day + ", " + hours8dose + ", " + daydose + ", " + id + ", " + length);
          });
    },
    setUploadStatus(timestamp, id, status) {
        storage.db.transaction(function(tx) {
            tx.executeSql('UPDATE Upload SET status==? WHERE timestamp==? and id==?', [status, timestamp, id]);
          }, function(error) {
            console.log('setUploadStatus Transaction ERROR: ' + error.message);
          }, function() {
            console.log('setUploadStatus changed ' + timestamp + ", " + id + ", " + status);
          });
    },
    getUnploadedEntries(callback) {
        storage.db.transaction(function(tx) {
            tx.executeSql('SELECT *  FROM Upload WHERE status == 0', [], function(tx, rs) {
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