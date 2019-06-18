(function() {

    var running;
    var app;
    var watchId;

    /**
     * Handles the hardware key event.
     * @private
     * @param {Object} event - The hardware key event object
     */
    function keyEventHandler(event) {
            if (event.keyName === "back") {
                if (pageController.isPageMain() === true) {
                    // Terminate the application if the current page is the main page.
                    try {
                        tizen.application.getCurrentApplication().exit();
                    } catch (ignore) {}
                } else {
                    // Go to the last page if the current page is not the main page.
                    //stopRunAnimation();
                    pageController.moveBackPage();
                }
            }
        }
    
    function new_data(event, data) {
        /* Data from first app must be received here */
        //console.log('Data: ' + JSON.stringify(data));
    		console.log('received event');
    }
        /**
         * Sets default event listeners.
         * @private
         */
    function setDefaultEvents() {
        var btnStartStop = document.querySelector("#btn-startstop");

        // Add hardware key event listener
        window.addEventListener("tizenhwkey", keyEventHandler);
        //document.addEventListener("rotarydetent", rotaryEventHandler);

        btnStartStop.addEventListener("click", function() {
            if (running) {
                var appControl = new tizen.ApplicationControl("http://tizen.org/appcontrol/operation/default", null, null, null, [new tizen.ApplicationControlData("service_action", ["stop"])]);
                tizen.application.launchAppControl(appControl, "arq901aCcl.tatysoundservice", function() {
                    console.log("tatysoundservice stopped");
                    btnStartStop.style.backgroundImage = "url('./image/button_start.png')";
                    running = false;


                }, function(e) {
                    console.log("tatysoundservice stop error " + e.type + " msg:" + e.message);
                });

            } else {

                //        			tizen.application.getAppsInfo(function onListInstalledApps(applications) {
                //        		        console.log("APPS INSTALLED:");
                //        		        for (var i = 0; i < applications.length; i++)
                //        		            console.log(i + " ID: " + applications[i].id);
                //        		        });


                tizen.application.launch("arq901aCcl.tatysoundservice", function() {
                    console.log("tatysoundservice started");
                    btnStartStop.style.backgroundImage = "url('./image/button_stop.png')";
                    running = true;

                    //app.removeEventListener(watchId);
                }, function(e) {
                    console.log("tatysoundservice launch error " + e.type + " msg:" + e.message);
                });
            }

        });
    }

    function onRunningAppsContext(contexts) {
        for (var i = 0; i < contexts.length; i++) {
            console.log("ID: " + contexts[i].id + " - " + contexts[i].appId);
            if (contexts[i].appId === "arq901aCcl.tatysoundservice") {
                running = true;
                console.log("running = true");
            }
        }
        
        if (running) {
        		var btnStartStop = document.querySelector("#btn-startstop");
            btnStartStop.style.backgroundImage = "url('./image/button_stop.png')";
        }
    }

    /**
     * Initializes the application.
     * @private
     */
    function init() {

        app = tizen.application.getCurrentApplication();
        running = false;

        tizen.application.getAppsContext(onRunningAppsContext);

        
        console.log("addEventListener");
        
        watchId = app.addEventListener({
            appId: 'arq901aCcl.tatysoundservice',
            name: 'new_data_event'
        }, new_data);
        
        console.log("watchId: " + watchId);

        setDefaultEvents();

        // Add  pages to the page controller
        pageController.addPage("page-main");
    }

    window.onload = init;
}());