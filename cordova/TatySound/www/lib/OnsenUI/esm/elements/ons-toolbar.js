'use strict';

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _elements = require('../ons/elements');

var _elements2 = _interopRequireDefault(_elements);

var _util = require('../ons/util');

var _util2 = _interopRequireDefault(_util);

var _internal = require('../ons/internal');

var _internal2 = _interopRequireDefault(_internal);

var _autostyle = require('../ons/autostyle');

var _autostyle2 = _interopRequireDefault(_autostyle);

var _modifierUtil = require('../ons/internal/modifier-util');

var _modifierUtil2 = _interopRequireDefault(_modifierUtil);

var _baseElement = require('./base/base-element');

var _baseElement2 = _interopRequireDefault(_baseElement);

var _contentReady = require('../ons/content-ready');

var _contentReady2 = _interopRequireDefault(_contentReady);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; } /*
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

var defaultClassName = 'toolbar';

var scheme = {
  '': 'toolbar--*',
  '.toolbar__left': 'toolbar--*__left',
  '.toolbar__center': 'toolbar--*__center',
  '.toolbar__right': 'toolbar--*__right'
};

/**
 * @element ons-toolbar
 * @category page
 * @modifier material
 *   [en]Material Design toolbar.[/en]
 *   [ja][/ja]
 * @modifier transparent
 *   [en]Transparent toolbar.[/en]
 *   [ja]透明な背景を持つツールバーを表示します。[/ja]
 * @modifier cover-content
 *   [en]Displays the toolbar on top of the page's content. Should be combined with `transparent` modifier.[/en]
 *   [ja][/ja]
 * @modifier noshadow
 *   [en]Toolbar without shadow.[/en]
 *   [ja]ツールバーに影を付けずに表示します。[/ja]
 * @description
 *   [en]
 *     Toolbar component that can be used with navigation.
 *
 *     Left, center and right containers can be specified by class names.
 *
 *     This component will automatically display as a Material Design toolbar when running on Android devices.
 *   [/en]
 *   [ja]ナビゲーションで使用するツールバー用コンポーネントです。クラス名により、左、中央、右のコンテナを指定できます。[/ja]
 * @codepen aHmGL
 * @tutorial vanilla/Reference/toolbar
 * @guide compilation.html#toolbar-compilation [en]Adding a toolbar[/en][ja]ツールバーの追加[/ja]
 * @seealso ons-bottom-toolbar
 *   [en]The `<ons-bottom-toolbar>` displays a toolbar on the bottom of the page.[/en]
 *   [ja]ons-bottom-toolbarコンポーネント[/ja]
 * @seealso ons-back-button
 *   [en]The `<ons-back-button>` component displays a back button inside the toolbar.[/en]
 *   [ja]ons-back-buttonコンポーネント[/ja]
 * @seealso ons-toolbar-button
 *   [en]The `<ons-toolbar-button>` component displays a toolbar button inside the toolbar.[/en]
 *   [ja]ons-toolbar-buttonコンポーネント[/ja]
 * @example
 * <ons-page>
 *   <ons-toolbar>
 *     <div class="left">
 *       <ons-back-button>
 *         Back
 *       </ons-back-button>
 *     </div>
 *     <div class="center">
 *       Title
 *     </div>
 *     <div class="right">
 *       <ons-toolbar-button>
 *         <ons-icon icon="md-menu"></ons-icon>
 *       </ons-toolbar-button>
 *     </div>
 *   </ons-toolbar>
 * </ons-page>
 */

var ToolbarElement = function (_BaseElement) {
  _inherits(ToolbarElement, _BaseElement);

  /**
   * @attribute inline
   * @initonly
   * @description
   *   [en]Display the toolbar as an inline element.[/en]
   *   [ja]ツールバーをインラインに置きます。スクロール領域内にそのまま表示されます。[/ja]
   */

  /**
   * @attribute static
   * @description
   *   [en]Static toolbars are not animated by `ons-navigator` when pushing or popping pages. This can be useful to improve performance in some situations.[/en]
   *   [ja][/ja]
   */

  /**
   * @attribute modifier
   * @description
   *   [en]The appearance of the toolbar.[/en]
   *   [ja]ツールバーの表現を指定します。[/ja]
   */

  function ToolbarElement() {
    _classCallCheck(this, ToolbarElement);

    var _this = _possibleConstructorReturn(this, (ToolbarElement.__proto__ || Object.getPrototypeOf(ToolbarElement)).call(this));

    (0, _contentReady2.default)(_this, function () {
      _this._compile();
    });
    return _this;
  }

  _createClass(ToolbarElement, [{
    key: 'attributeChangedCallback',
    value: function attributeChangedCallback(name, last, current) {
      switch (name) {
        case 'class':
          _util2.default.restoreClass(this, defaultClassName, scheme);
          break;
        case 'modifier':
          _modifierUtil2.default.onModifierChanged(last, current, this, scheme);
          break;
      }
    }

    /**
     * @method setVisibility
     * @signature setVisibility(visible)
     * @param {Boolean} visible
     *   [en]Set to true to show the toolbar, false to hide it[/en]
     *   [ja][/ja]
     * @description
     *   [en]Shows the toolbar if visible is true, otherwise hides it.[/en]
     *   [ja][/ja]
     */

  }, {
    key: 'setVisibility',
    value: function setVisibility(visible) {
      var _this2 = this;

      (0, _contentReady2.default)(this, function () {
        _this2.style.display = visible ? '' : 'none';

        if (_this2.parentNode) {
          var siblingBackground = _util2.default.findChild(_this2.parentNode, '.page__background');
          if (siblingBackground) {
            siblingBackground.style.top = visible ? null : 0;
          }

          var siblingContent = _util2.default.findChild(_this2.parentNode, '.page__content');
          if (siblingContent) {
            siblingContent.style.top = visible ? null : 0;
          }
        }
      });
    }

    /**
     * @method show
     * @signature show()
     * @description
     *   [en]Show the toolbar.[/en]
     *   [ja][/ja]
     */

  }, {
    key: 'show',
    value: function show() {
      this.setVisibility(true);
    }

    /**
     * @method hide
     * @signature hide()
     * @description
     *   [en]Hide the toolbar.[/en]
     *   [ja][/ja]
     */

  }, {
    key: 'hide',
    value: function hide() {
      this.setVisibility(false);
    }

    /**
     * @return {HTMLElement}
     */

  }, {
    key: '_getToolbarLeftItemsElement',
    value: function _getToolbarLeftItemsElement() {
      return this.querySelector('.left') || _internal2.default.nullElement;
    }

    /**
     * @return {HTMLElement}
     */

  }, {
    key: '_getToolbarCenterItemsElement',
    value: function _getToolbarCenterItemsElement() {
      return this.querySelector('.center') || _internal2.default.nullElement;
    }

    /**
     * @return {HTMLElement}
     */

  }, {
    key: '_getToolbarRightItemsElement',
    value: function _getToolbarRightItemsElement() {
      return this.querySelector('.right') || _internal2.default.nullElement;
    }

    /**
     * @return {HTMLElement}
     */

  }, {
    key: '_getToolbarBackButtonLabelElement',
    value: function _getToolbarBackButtonLabelElement() {
      return this.querySelector('ons-back-button .back-button__label') || _internal2.default.nullElement;
    }

    /**
     * @return {HTMLElement}
     */

  }, {
    key: '_getToolbarBackButtonIconElement',
    value: function _getToolbarBackButtonIconElement() {
      return this.querySelector('ons-back-button .back-button__icon') || _internal2.default.nullElement;
    }
  }, {
    key: '_compile',
    value: function _compile() {
      _autostyle2.default.prepare(this);
      this.classList.add(defaultClassName);
      this._ensureToolbarItemElements();
      _modifierUtil2.default.initModifier(this, scheme);
    }
  }, {
    key: '_ensureToolbarItemElements',
    value: function _ensureToolbarItemElements() {
      for (var i = this.childNodes.length - 1; i >= 0; i--) {
        // case of not element
        if (this.childNodes[i].nodeType != 1) {
          this.removeChild(this.childNodes[i]);
        }
      }

      var center = this._ensureToolbarElement('center');
      center.classList.add('toolbar__title');

      if (this.children.length !== 1 || !this.children[0].classList.contains('center')) {
        var left = this._ensureToolbarElement('left');
        var right = this._ensureToolbarElement('right');

        if (this.children[0] !== left || this.children[1] !== center || this.children[2] !== right) {
          this.appendChild(left);
          this.appendChild(center);
          this.appendChild(right);
        }
      }
    }
  }, {
    key: '_ensureToolbarElement',
    value: function _ensureToolbarElement(name) {
      if (_util2.default.findChild(this, '.toolbar__' + name)) {
        var _element = _util2.default.findChild(this, '.toolbar__' + name);
        _element.classList.add(name);
        return _element;
      }

      var element = _util2.default.findChild(this, '.' + name) || _util2.default.create('.' + name);
      element.classList.add('toolbar__' + name);

      return element;
    }
  }], [{
    key: 'observedAttributes',
    get: function get() {
      return ['modifier', 'class'];
    }
  }]);

  return ToolbarElement;
}(_baseElement2.default);

exports.default = ToolbarElement;


_elements2.default.Toolbar = ToolbarElement;
customElements.define('ons-toolbar', ToolbarElement);