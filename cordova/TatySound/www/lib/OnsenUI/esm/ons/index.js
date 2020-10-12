'use strict';

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _extends = Object.assign || function (target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i]; for (var key in source) { if (Object.prototype.hasOwnProperty.call(source, key)) { target[key] = source[key]; } } } return target; };

var _typeof = typeof Symbol === "function" && typeof Symbol.iterator === "symbol" ? function (obj) { return typeof obj; } : function (obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; /*
                                                                                                                                                                                                                                                                              Copyright 2013-2015 ASIAL CORPORATION
                                                                                                                                                                                                                                                                              
                                                                                                                                                                                                                                                                              Licensed under the Apache License, Version 2.0 (the "License");
                                                                                                                                                                                                                                                                              you may not use this file except in compliance with the License.
                                                                                                                                                                                                                                                                              You may obtain a copy of the License at
                                                                                                                                                                                                                                                                              
                                                                                                                                                                                                                                                                                 http://www.apache.org/licenses/LICENSE-2.0
                                                                                                                                                                                                                                                                              
                                                                                                                                                                                                                                                                              Unless required by applicable law or agreed to in writing, software
                                                                                                                                                                                                                                                                              distributed under the License is distributed on an "AS IS" BASIS,
                                                                                                                                                                                                                                                                              WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
                                                                                                                                                                                                                                                                              See the License for the specific language governing permissions and
                                                                                                                                                                                                                                                                              limitations under the License.
                                                                                                                                                                                                                                                                              
                                                                                                                                                                                                                                                                              */

var _util = require('./util');

var _util2 = _interopRequireDefault(_util);

var _elements = require('./elements');

var _elements2 = _interopRequireDefault(_elements);

var _animit = require('./animit');

var _animit2 = _interopRequireDefault(_animit);

var _gestureDetector = require('./gesture-detector');

var _gestureDetector2 = _interopRequireDefault(_gestureDetector);

var _platform = require('./platform');

var _platform2 = _interopRequireDefault(_platform);

var _notification = require('./notification');

var _notification2 = _interopRequireDefault(_notification);

var _actionSheet = require('./action-sheet');

var _actionSheet2 = _interopRequireDefault(_actionSheet);

var _internal = require('./internal');

var _internal2 = _interopRequireDefault(_internal);

var _orientation = require('./orientation');

var _orientation2 = _interopRequireDefault(_orientation);

var _modifier = require('./modifier');

var _modifier2 = _interopRequireDefault(_modifier);

var _softwareKeyboard = require('./software-keyboard');

var _softwareKeyboard2 = _interopRequireDefault(_softwareKeyboard);

var _pageAttributeExpression = require('./page-attribute-expression');

var _pageAttributeExpression2 = _interopRequireDefault(_pageAttributeExpression);

var _autostyle = require('./autostyle');

var _autostyle2 = _interopRequireDefault(_autostyle);

var _doorlock = require('./doorlock');

var _doorlock2 = _interopRequireDefault(_doorlock);

var _pageLoader = require('./page-loader');

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

/**
 * @object ons
 * @category util
 * @description
 *   [ja]Onsen UIで利用できるグローバルなオブジェクトです。[/ja]
 *   [en]A global object that's used in Onsen UI. [/en]
 */
var ons = {
  animit: _animit2.default,
  defaultPageLoader: _pageLoader.defaultPageLoader,
  elements: _elements2.default,
  GestureDetector: _gestureDetector2.default,
  modifier: _modifier2.default,
  notification: _notification2.default,
  orientation: _orientation2.default,
  pageAttributeExpression: _pageAttributeExpression2.default,
  PageLoader: _pageLoader.PageLoader,
  platform: _platform2.default,
  softwareKeyboard: _softwareKeyboard2.default,
  _autoStyle: _autostyle2.default,
  _internal: _internal2.default,
  _readyLock: new _doorlock2.default(),
  _util: _util2.default
};

ons.platform.select((window.location.search.match(/platform=([\w-]+)/) || [])[1]);

waitDeviceReady();

var readyError = function readyError(after) {
  return _util2.default.throw('This method must be called ' + (after ? 'after' : 'before') + ' ons.isReady() is true');
};

/**
 * @method isReady
 * @signature isReady()
 * @return {Boolean}
 *   [en]Will be true if Onsen UI is initialized.[/en]
 *   [ja]初期化されているかどうかを返します。[/ja]
 * @description
 *   [en]Returns true if Onsen UI is initialized.[/en]
 *   [ja]Onsen UIがすでに初期化されているかどうかを返すメソッドです。[/ja]
 */
ons.isReady = function () {
  return !ons._readyLock.isLocked();
};

/**
 * @method isWebView
 * @signature isWebView()
 * @return {Boolean}
 *   [en]Will be true if the app is running in Cordova.[/en]
 *   [ja]Cordovaで実行されている場合にtrueになります。[/ja]
 * @description
 *   [en]Returns true if running inside Cordova.[/en]
 *   [ja]Cordovaで実行されているかどうかを返すメソッドです。[/ja]
 */
ons.isWebView = ons.platform.isWebView;

/**
 * @method ready
 * @signature ready(callback)
 * @description
 *   [ja]アプリの初期化に利用するメソッドです。渡された関数は、Onsen UIの初期化が終了している時点で必ず呼ばれます。[/ja]
 *   [en]Method used to wait for app initialization. Waits for `DOMContentLoaded` and `deviceready`, when necessary, before executing the callback.[/en]
 * @param {Function} callback
 *   [en]Function that executes after Onsen UI has been initialized.[/en]
 *   [ja]Onsen UIが初期化が完了した後に呼び出される関数オブジェクトを指定します。[/ja]
 */
ons.ready = function (callback) {
  if (ons.isReady()) {
    callback();
  } else {
    ons._readyLock.waitUnlock(callback);
  }
};

/**
 * @method setDefaultDeviceBackButtonListener
 * @signature setDefaultDeviceBackButtonListener(listener)
 * @param {Function} listener
 *   [en]Function that executes when device back button is pressed. Must be called on `ons.ready`.[/en]
 *   [ja]デバイスのバックボタンが押された時に実行される関数オブジェクトを指定します。[/ja]
 * @description
 *   [en]Set default handler for device back button.[/en]
 *   [ja]デバイスのバックボタンのためのデフォルトのハンドラを設定します。[/ja]
 */
ons.setDefaultDeviceBackButtonListener = function (listener) {
  if (!ons.isReady()) {
    readyError(true);
  }
  ons._defaultDeviceBackButtonHandler.setListener(listener);
};

/**
 * @method disableDeviceBackButtonHandler
 * @signature disableDeviceBackButtonHandler()
 * @description
 * [en]Disable device back button event handler. Must be called on `ons.ready`.[/en]
 * [ja]デバイスのバックボタンのイベントを受け付けないようにします。[/ja]
 */
ons.disableDeviceBackButtonHandler = function () {
  if (!ons.isReady()) {
    readyError(true);
  }
  _internal2.default.dbbDispatcher.disable();
};

/**
 * @method enableDeviceBackButtonHandler
 * @signature enableDeviceBackButtonHandler()
 * @description
 * [en]Enable device back button event handler. Must be called on `ons.ready`.[/en]
 * [ja]デバイスのバックボタンのイベントを受け付けるようにします。[/ja]
 */
ons.enableDeviceBackButtonHandler = function () {
  if (!ons.isReady()) {
    readyError(true);
  }
  _internal2.default.dbbDispatcher.enable();
};

ons.fireDeviceBackButtonEvent = function () {
  _internal2.default.dbbDispatcher.fireDeviceBackButtonEvent();
};

/**
 * @method enableAutoStatusBarFill
 * @signature enableAutoStatusBarFill()
 * @description
 *   [en]Enable status bar fill feature on iOS7 and above (except for iPhone X). Must be called before `ons.ready`.[/en]
 *   [ja]iOS7以上（iPhone Xは除く）で、ステータスバー部分の高さを自動的に埋める処理を有効にします。[/ja]
 */
ons.enableAutoStatusBarFill = function () {
  if (ons.isReady()) {
    readyError(false);
  }
  _internal2.default.config.autoStatusBarFill = true;
};

/**
 * @method disableAutoStatusBarFill
 * @signature disableAutoStatusBarFill()
 * @description
 *   [en]Disable status bar fill feature on iOS7 and above (except for iPhone X). Must be called before `ons.ready`.[/en]
 *   [ja]iOS7以上（iPhone Xは除く）で、ステータスバー部分の高さを自動的に埋める処理を無効にします。[/ja]
 */
ons.disableAutoStatusBarFill = function () {
  if (ons.isReady()) {
    readyError(false);
  }
  _internal2.default.config.autoStatusBarFill = false;
};

/**
 * @method mockStatusBar
 * @signature mockStatusBar()
 * @description
 *   [en]Creates a static element similar to iOS status bar. Only useful for browser testing. Must be called before `ons.ready`.[/en]
 *   [ja][/ja]
 */
ons.mockStatusBar = function () {
  if (ons.isReady()) {
    readyError(false);
  }

  var mock = function mock() {
    if (!document.body.children[0] || !document.body.children[0].classList.contains('ons-status-bar-mock')) {
      var android = _platform2.default.isAndroid(),
          i = function i(_i) {
        return '<i class="' + _i.split('-')[0] + ' ' + _i + '"></i>';
      };
      var left = android ? i('zmdi-twitter') + ' ' + i('zmdi-google-play') : 'No SIM ' + i('fa-wifi'),
          center = android ? '' : '12:28 PM',
          right = android ? i('zmdi-network') + ' ' + i('zmdi-wifi') + ' ' + i('zmdi-battery') + ' 12:28 PM' : '80% ' + i('fa-battery-three-quarters');

      document.body.insertBefore(_util2.default.createElement('<div class="ons-status-bar-mock ' + (android ? 'android' : 'ios') + '">' + ('<div>' + left + '</div><div>' + center + '</div><div>' + right + '</div>') + '</div>'), document.body.firstChild);
    }
  };

  document.body ? mock() : _internal2.default.waitDOMContentLoaded(mock);
};

/**
 * @method disableAnimations
 * @signature disableAnimations()
 * @description
 *   [en]Disable all animations. Could be handy for testing and older devices.[/en]
 *   [ja]アニメーションを全て無効にします。テストの際に便利です。[/ja]
 */
ons.disableAnimations = function () {
  _internal2.default.config.animationsDisabled = true;
};

/**
 * @method enableAnimations
 * @signature enableAnimations()
 * @description
 *   [en]Enable animations (default).[/en]
 *   [ja]アニメーションを有効にします。[/ja]
 */
ons.enableAnimations = function () {
  _internal2.default.config.animationsDisabled = false;
};

ons._disableWarnings = function () {
  _internal2.default.config.warningsDisabled = true;
};

ons._enableWarnings = function () {
  _internal2.default.config.warningsDisabled = false;
};

/**
 * @method disableAutoStyling
 * @signature disableAutoStyling()
 * @description
 *   [en]Disable automatic styling.[/en]
 *   [ja][/ja]
 */
ons.disableAutoStyling = _autostyle2.default.disable;

/**
 * @method enableAutoStyling
 * @signature enableAutoStyling()
 * @description
 *   [en]Enable automatic styling based on OS (default).[/en]
 *   [ja][/ja]
 */
ons.enableAutoStyling = _autostyle2.default.enable;

/**
 * @method disableIconAutoPrefix
 * @signature disableIconAutoPrefix()
 * @description
 *   [en]Disable adding `fa-` prefix automatically to `ons-icon` classes. Useful when including custom icon packs.[/en]
 *   [ja][/ja]
 */
ons.disableIconAutoPrefix = function () {
  _util2.default.checkMissingImport('Icon');
  _elements2.default.Icon.setAutoPrefix(false);
};

/**
 * @method forcePlatformStyling
 * @signature forcePlatformStyling(platform)
 * @description
 *   [en]Refresh styling for the given platform. Only useful for demos. Use `ons.platform.select(...)` instead for development and production.[/en]
 *   [ja][/ja]
 * @param {string} platform New platform to style the elements.
 */
ons.forcePlatformStyling = function (newPlatform) {
  ons.enableAutoStyling();
  ons.platform.select(newPlatform || 'ios');

  ons._util.arrayFrom(document.querySelectorAll('*')).forEach(function (element) {
    if (element.tagName.toLowerCase() === 'ons-if') {
      element._platformUpdate();
    } else if (element.tagName.match(/^ons-/i)) {
      _autostyle2.default.prepare(element, true);
      if (element.tagName.toLowerCase() === 'ons-tabbar') {
        element._updatePosition();
      }
    }
  });
};

/**
 * @method preload
 * @signature preload(templatePaths)
 * @param {String|Array} templatePaths
 *   [en]Set of HTML file paths containing 'ons-page' elements.[/en]
 *   [ja][/ja]
 * @return {Promise}
 *   [en]Promise that resolves when all the templates are cached.[/en]
 *   [ja][/ja]
 * @description
 *   [en]Separated files need to be requested on demand and this can slightly delay pushing new pages. This method requests and caches templates for later use.[/en]
 *   [ja][/ja]
 */
ons.preload = function () {
  var templates = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : [];

  return Promise.all((templates instanceof Array ? templates : [templates]).map(function (template) {
    if (typeof template !== 'string') {
      _util2.default.throw('Expected string arguments but got ' + (typeof template === 'undefined' ? 'undefined' : _typeof(template)));
    }
    return _internal2.default.getTemplateHTMLAsync(template);
  }));
};

/**
 * @method createElement
 * @signature createElement(template, options)
 * @param {String} template
 *   [en]Either an HTML file path, a `<template>` id or an HTML string such as `'<div id="foo">hoge</div>'`.[/en]
 *   [ja][/ja]
 * @param {Object} [options]
 *   [en]Parameter object.[/en]
 *   [ja]オプションを指定するオブジェクト。[/ja]
 * @param {Boolean|HTMLElement} [options.append]
 *   [en]Whether or not the element should be automatically appended to the DOM.  Defaults to `false`. If `true` value is given, `document.body` will be used as the target.[/en]
 *   [ja][/ja]
 * @param {HTMLElement} [options.insertBefore]
 *   [en]Reference node that becomes the next sibling of the new node (`options.append` element).[/en]
 *   [ja][/ja]
 * @return {HTMLElement|Promise}
 *   [en]If the provided template was an inline HTML string, it returns the new element. Otherwise, it returns a promise that resolves to the new element.[/en]
 *   [ja][/ja]
 * @description
 *   [en]Create a new element from a template. Both inline HTML and external files are supported although the return value differs.[/en]
 *   [ja][/ja]
 */
ons.createElement = function (template) {
  var options = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};

  template = template.trim();

  var create = function create(html) {
    var element = ons._util.createElement(html);
    element.remove();

    if (options.append) {
      var target = options.append instanceof HTMLElement ? options.append : document.body;
      target.insertBefore(element, options.insertBefore || null);
      options.link instanceof Function && options.link(element);
    }

    return element;
  };

  return template.charAt(0) === '<' ? create(template) : _internal2.default.getPageHTMLAsync(template).then(create);
};

/**
 * @method createPopover
 * @signature createPopover(page, [options])
 * @param {String} page
 *   [en]Page name. Can be either an HTML file or a <template> containing a <ons-dialog> component.[/en]
 *   [ja]pageのURLか、もしくは`<template>`で宣言したテンプレートのid属性の値を指定できます。[/ja]
 * @param {Object} [options]
 *   [en]Parameter object.[/en]
 *   [ja]オプションを指定するオブジェクト。[/ja]
 * @param {Object} [options.parentScope]
 *   [en]Parent scope of the dialog. Used to bind models and access scope methods from the dialog.[/en]
 *   [ja]ダイアログ内で利用する親スコープを指定します。ダイアログからモデルやスコープのメソッドにアクセスするのに使います。このパラメータはAngularJSバインディングでのみ利用できます。[/ja]
 * @return {Promise}
 *   [en]Promise object that resolves to the popover component object.[/en]
 *   [ja]ポップオーバーのコンポーネントオブジェクトを解決するPromiseオブジェクトを返します。[/ja]
 * @description
 *   [en]Create a popover instance from a template.[/en]
 *   [ja]テンプレートからポップオーバーのインスタンスを生成します。[/ja]
 */
/**
 * @method createDialog
 * @signature createDialog(page, [options])
 * @param {String} page
 *   [en]Page name. Can be either an HTML file or an `<template>` containing a <ons-dialog> component.[/en]
 *   [ja]pageのURLか、もしくは`<template>`で宣言したテンプレートのid属性の値を指定できます。[/ja]
 * @param {Object} [options]
 *   [en]Parameter object.[/en]
 *   [ja]オプションを指定するオブジェクト。[/ja]
 * @return {Promise}
 *   [en]Promise object that resolves to the dialog component object.[/en]
 *   [ja]ダイアログのコンポーネントオブジェクトを解決するPromiseオブジェクトを返します。[/ja]
 * @description
 *   [en]Create a dialog instance from a template.[/en]
 *   [ja]テンプレートからダイアログのインスタンスを生成します。[/ja]
 */
/**
 * @method createAlertDialog
 * @signature createAlertDialog(page, [options])
 * @param {String} page
 *   [en]Page name. Can be either an HTML file or an `<template>` containing a <ons-alert-dialog> component.[/en]
 *   [ja]pageのURLか、もしくは`<template>`で宣言したテンプレートのid属性の値を指定できます。[/ja]
 * @param {Object} [options]
 *   [en]Parameter object.[/en]
 *   [ja]オプションを指定するオブジェクト。[/ja]
 * @return {Promise}
 *   [en]Promise object that resolves to the alert dialog component object.[/en]
 *   [ja]ダイアログのコンポーネントオブジェクトを解決するPromiseオブジェクトを返します。[/ja]
 * @description
 *   [en]Create a alert dialog instance from a template.[/en]
 *   [ja]テンプレートからアラートダイアログのインスタンスを生成します。[/ja]
 */
ons.createPopover = ons.createDialog = ons.createAlertDialog = function (template) {
  var options = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};
  return ons.createElement(template, _extends({ append: true }, options));
};

/**
 * @method openActionSheet
 * @signature openActionSheet(options)
 * @description
 *   [en]Shows an instant Action Sheet and lets the user choose an action.[/en]
 *   [ja][/ja]
 * @param {Object} [options]
 *   [en]Parameter object.[/en]
 *   [ja]オプションを指定するオブジェクト。[/ja]
 * @param {Array} [options.buttons]
 *   [en]Represent each button of the action sheet following the specified order. Every item can be either a string label or an object containing `label`, `icon` and `modifier` properties.[/en]
 *   [ja][/ja]
 * @param {String} [options.title]
 *   [en]Optional title for the action sheet.[/en]
 *   [ja][/ja]
 * @param {Number} [options.destructive]
 *   [en]Optional index of the "destructive" button (only for iOS). It can be specified in the button array as well.[/en]
 *   [ja][/ja]
 * @param {Boolean} [options.cancelable]
 *   [en]Whether the action sheet can be canceled by tapping on the background mask or not.[/en]
 *   [ja][/ja]
 * @param {String} [options.modifier]
 *   [en]Modifier attribute of the action sheet. E.g. `'destructive'`.[/en]
 *   [ja][/ja]
 * @param {String} [options.maskColor]
 *   [en]Optionally change the background mask color.[/en]
 *   [ja][/ja]
 * @param {String} [options.id]
 *   [en]The element's id attribute.[/en]
 *   [ja][/ja]
 * @param {String} [options.class]
 *   [en]The element's class attribute.[/en]
 *   [ja][/ja]
 * @return {Promise}
 *   [en]Will resolve when the action sheet is closed. The resolve value is either the index of the tapped button or -1 when canceled.[/en]
 *   [ja][/ja]
 */
ons.openActionSheet = _actionSheet2.default;

/**
 * @method resolveLoadingPlaceholder
 * @signature resolveLoadingPlaceholder(page)
 * @param {String} page
 *   [en]Page name. Can be either an HTML file or a `<template>` id.[/en]
 *   [ja]pageのURLか、もしくは`<template>`で宣言したテンプレートのid属性の値を指定できます。[/ja]
 * @description
 *   [en]If no page is defined for the `ons-loading-placeholder` attribute it will wait for this method being called before loading the page.[/en]
 *   [ja]ons-loading-placeholderの属性値としてページが指定されていない場合は、ページロード前に呼ばれるons.resolveLoadingPlaceholder処理が行われるまで表示されません。[/ja]
 */
ons.resolveLoadingPlaceholder = function (page, link) {
  var elements = ons._util.arrayFrom(window.document.querySelectorAll('[ons-loading-placeholder]'));
  if (elements.length === 0) {
    _util2.default.throw('No ons-loading-placeholder exists');
  }

  elements.filter(function (element) {
    return !element.getAttribute('page');
  }).forEach(function (element) {
    element.setAttribute('ons-loading-placeholder', page);
    ons._resolveLoadingPlaceholder(element, page, link);
  });
};

ons._setupLoadingPlaceHolders = function () {
  ons.ready(function () {
    var elements = ons._util.arrayFrom(window.document.querySelectorAll('[ons-loading-placeholder]'));

    elements.forEach(function (element) {
      var page = element.getAttribute('ons-loading-placeholder');
      if (typeof page === 'string') {
        ons._resolveLoadingPlaceholder(element, page);
      }
    });
  });
};

ons._resolveLoadingPlaceholder = function (parent, page) {
  var link = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : function (el, done) {
    return done();
  };

  page && ons.createElement(page).then(function (element) {
    element.style.display = 'none';
    parent.appendChild(element);
    link(element, function () {
      while (parent.firstChild && parent.firstChild !== element) {
        parent.removeChild(parent.firstChild);
      }
      element.style.display = '';
    });
  }).catch(function (error) {
    return Promise.reject('Unabled to resolve placeholder: ' + error);
  });
};

function waitDeviceReady() {
  var unlockDeviceReady = ons._readyLock.lock();
  window.addEventListener('DOMContentLoaded', function () {
    if (ons.isWebView()) {
      window.document.addEventListener('deviceready', unlockDeviceReady, false);
    } else {
      unlockDeviceReady();
    }
  }, false);
}

/**
 * @method getScriptPage
 * @signature getScriptPage()
 * @description
 *   [en]Access the last created page from the current `script` scope. Only works inside `<script></script>` tags that are direct children of `ons-page` element. Use this to add lifecycle hooks to a page.[/en]
 *   [ja][/ja]
 * @return {HTMLElement}
 *   [en]Returns the corresponding page element.[/en]
 *   [ja][/ja]
 */
var getCS = 'currentScript' in document ? function () {
  return document.currentScript;
} : function () {
  return document.scripts[document.scripts.length - 1];
};
ons.getScriptPage = function () {
  return getCS() && /ons-page/i.test(getCS().parentElement.tagName) && getCS().parentElement || null;
};

exports.default = ons;