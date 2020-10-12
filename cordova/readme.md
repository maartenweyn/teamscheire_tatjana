cordova platform add android
cordova platform add ios

brew install imagemagick
sudo npm install -g cordova-splash cordova-icon
sudo npm install -g ios-deploy
cordova-icon
cordova-splash

cordova build ios


### iOS 10

For iOS 10, apps will crash unless they include usage description keys for the types of data they access. For Bluetooth, [NSBluetoothPeripheralUsageDescription](https://developer.apple.com/library/prerelease/content/documentation/General/Reference/InfoPlistKeyReference/Articles/CocoaKeys.html#//apple_ref/doc/uid/TP40009251-SW20) must be defined.

This can be done when the plugin is installed using the BLUETOOTH_USAGE_DESCRIPTION variable.

    $ cordova plugin add cordova-plugin-ble-central --variable BLUETOOTH_USAGE_DESCRIPTION="Your description here"


### Android

logcat: 

    $ adb logcat -e "CONSOLE"