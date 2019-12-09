var app = {
    user: {},
    debug: false,
    initialize: function () {
        console.log('app initialize');
        document.addEventListener('deviceready', this.onDeviceReady.bind(this), false);
        /*console.log(window.location.hostname);
        if (window.location.hostname == 'bicyclelight.localhost') {
            app.debug = true;
            ons.ready(function () {
                app.onDeviceReady();
            });
        }*/
    },

    databaseInitializedCallback: function() {
        console.log('databaseInitializedCallback');
        noiselevel.fillChart();
    },

    onDeviceReady: function () {
        debug.log('device ready', 'success');

        // $('#ble_config').hide();
        // $('#log_info').hide();
        app.bindEvents();
        //app.checkIfUserLoggedIn();
        //mqttclient.initialize();
        //gps.initialize();
        //gps.getLocation();
        storage.openDatabase(app.databaseInitializedCallback);
        noiselevel.initialize();
        bluetooth.initialize();

        cordova.plugins.backgroundMode.enable();
        cordova.plugins.backgroundMode.overrideBackButton();
        //cordova.plugins.backgroundMode.setDefaults({ silent: true })

        cordova.plugins.backgroundMode.setDefaults({
            title: "TatySound",
            text: "TatySound communication",
            icon: 'ic_launcher.png', // this will look for icon.png in platforms/android/res/drawable|mipmap
            color: 'F14F4D', // hex format like 'F14F4D'
            resume: true,
            hidden: true,
            bigText: false
        });

        
    },

    bindEvents: function () {
      // setTimeout(function () {
      //     mqttclient.addMessage('app,1');
      // }, 3000);
      $(document).on('click', '#ble_button', function (e) {
        console.log("ble_button click");
        $('#ble_config').toggle();
      });
      $(document).on('click', '#refreshDeviceList', function (e) {
        console.log("refreshDeviceList click");
        bluetooth.refreshDeviceList(false);
      });
      $('#ble-found-devices').on('tap', 'ons-list-item', function (e) {
          console.log("ble-found-devices click");
        bluetooth.connectDevice($(this).attr("data-device-id"), $(this).attr("data-device-name"));
      });
      $(document).on('click', '#disconnectDevice', function (e) {
        bluetooth.disconnectDevice(e);
      });

      $(document).on('click', '#log_button', function (e) {
        $('#log_info').toggle();
      });

      $(document).on('click', '#settings_button', function (e) {
        $('#settingscard').toggle();
      });

      $('#home').on('click', '.savebutton', function (e) {
        var token = $('#settings-token').val();
        noiselevel.settoken(token);
        });

      document.addEventListener("pause", app.onDevicePause, false);
      document.addEventListener("resume", app.onDeviceResume, false);
    //   document.addEventListener("menubutton", app.onMenuKeyDown, false);
    },

    onDevicePause: function () {
        debug.log('in pause');
        // mqttclient.addMessage('app,2');
    },
    onDeviceResume: function () {
        debug.log('out of pause');
        bluetooth.refreshDeviceList();
        noiselevel.showNoiseLevel();
        if (!noiselevel.is_uploading) {
          noiselevel.checkUploadData();
        }
        // mqttclient.addMessage('app,3');
    },
    onMenuKeyDown: function () {
        debug.log('menubuttonpressed');
        // mqttclient.addMessage('app,4');
    },
    onError: function (error) {
        debug.log(JSON.stringify(error), 'error');
    },

    // loadUser: function () {
    //     app.user = storage.getItem('user');
    //     console.log('logged in as: ' + JSON.stringify(app.user));
    // },
    // validateUser: function (newUser) {
    //     newUser.userId = newUser.userName.toLowerCase().replace(/ /g,'');
    //     return newUser;
    // },
    // saveUser: function (newUser) {
    //     storage.setItem('user', newUser);
    // }
    // },
    // checkIfUserLoggedIn() {
    //     app.loadUser();

    //     if (app.user) {
    //         $('.logged-in').show();
    //         $('.logged-out').hide();
    //         $('.username').html(app.user.userName);
    //     } else {
    //         $('.logged-out').show();
    //         $('.logged-in').hide();
    //     }
    // }
};

app.initialize();