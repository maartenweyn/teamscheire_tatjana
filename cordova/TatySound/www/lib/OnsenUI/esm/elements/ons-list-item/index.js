'use strict';

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _elements = require('../../ons/elements');

var _elements2 = _interopRequireDefault(_elements);

var _animit = require('../../ons/animit');

var _animit2 = _interopRequireDefault(_animit);

var _util = require('../../ons/util');

var _util2 = _interopRequireDefault(_util);

var _styler = require('../../ons/styler');

var _styler2 = _interopRequireDefault(_styler);

var _autostyle = require('../../ons/autostyle');

var _autostyle2 = _interopRequireDefault(_autostyle);

var _modifierUtil = require('../../ons/internal/modifier-util');

var _modifierUtil2 = _interopRequireDefault(_modifierUtil);

var _animatorFactory = require('../../ons/internal/animator-factory');

var _animatorFactory2 = _interopRequireDefault(_animatorFactory);

var _animator = require('./animator');

var _baseElement = require('../base/base-element');

var _baseElement2 = _interopRequireDefault(_baseElement);

var _contentReady = require('../../ons/content-ready');

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

var defaultClassName = 'list-item';
var scheme = {
  '.list-item': 'list-item--*',
  '.list-item__left': 'list-item--*__left',
  '.list-item__center': 'list-item--*__center',
  '.list-item__right': 'list-item--*__right',
  '.list-item__label': 'list-item--*__label',
  '.list-item__title': 'list-item--*__title',
  '.list-item__subtitle': 'list-item--*__subtitle',
  '.list-item__thumbnail': 'list-item--*__thumbnail',
  '.list-item__icon': 'list-item--*__icon'
};

var _animatorDict = {
  'default': _animator.SlideListItemAnimator,
  'none': _animator.ListItemAnimator
};

/**
 * @element ons-list-item
 * @category list
 * @modifier tappable
 *   [en]Make the list item change appearance when it's tapped. On iOS it is better to use the "tappable" and "tap-background-color" attribute for better behavior when scrolling.[/en]
 *   [ja]タップやクリックした時に効果が表示されるようになります。[/ja]
 * @modifier chevron
 *   [en]Display a chevron at the right end of the list item and make it change appearance when tapped.[/en]
 *   [ja][/ja]
 * @modifier longdivider
 *   [en]Displays a long horizontal divider between items.[/en]
 *   [ja][/ja]
 * @modifier nodivider
 *   [en]Removes the divider between list items.[/en]
 *   [ja][/ja]
 * @modifier material
 *   [en]Display a Material Design list item.[/en]
 *   [ja][/ja]
 * @description
 *   [en]
 *     Component that represents each item in a list. The list item is composed of four parts that are represented with the `left`, `center`, `right` and `expandable-content` classes. These classes can be used to ensure that the content of the list items is properly aligned.
 *
 *     ```
 *     <ons-list-item>
 *       <div class="left">Left</div>
 *       <div class="center">Center</div>
 *       <div class="right">Right</div>
 *       <div class="expandable-content">Expandable content</div>
 *     </ons-list-item>
 *     ```
 *
 *     There are also a number of classes (prefixed with `list-item__*`) that help when putting things like icons and thumbnails into the list items.
 *   [/en]
 *   [ja][/ja]
 * @seealso ons-list
 *   [en]ons-list component[/en]
 *   [ja]ons-listコンポーネント[/ja]
 * @seealso ons-list-header
 *   [en]ons-list-header component[/en]
 *   [ja]ons-list-headerコンポーネント[/ja]
 * @codepen yxcCt
 * @tutorial vanilla/Reference/list
 * @example
 * <ons-list-item>
 *   <div class="left">
 *     <ons-icon icon="md-face" class="list-item__icon"></ons-icon>
 *   </div>
 *   <div class="center">
 *     <div class="list-item__title">Title</div>
 *     <div class="list-item__subtitle">Subtitle</div>
 *   </div>
 *   <div class="right">
 *     <ons-switch></ons-switch>
 *   </div>
 * </ons-list-item>
 */

var ListItemElement = function (_BaseElement) {
  _inherits(ListItemElement, _BaseElement);

  /**
   * @attribute modifier
   * @type {String}
   * @description
   *   [en]The appearance of the list item.[/en]
   *   [ja]各要素の表現を指定します。[/ja]
   */

  /**
   * @attribute lock-on-drag
   * @type {String}
   * @description
   *   [en]Prevent vertical scrolling when the user drags horizontally.[/en]
   *   [ja]この属性があると、ユーザーがこの要素を横方向にドラッグしている時に、縦方向のスクロールが起きないようになります。[/ja]
   */

  /**
   * @attribute tappable
   * @type {Boolean}
   * @description
   *   [en]Makes the element react to taps. `prevent-tap` attribute can be added to child elements like buttons or inputs to prevent this effect. `ons-*` elements are ignored by default.[/en]
   *   [ja][/ja]
   */

  /**
   * @attribute tap-background-color
   * @type {Color}
   * @description
   *   [en] Changes the background color when tapped. For this to work, the attribute "tappable" needs to be set. The default color is "#d9d9d9". It will display as a ripple effect on Android.[/en]
   *   [ja][/ja]
   */

  /**
   * @attribute expandable
   * @type {Boolean}
   * @description
   *   [en]Makes the element able to be expanded to reveal extra content. For this to work, the expandable content must be defined in `div.expandable-content`.[/en]
   *   [ja][/ja]
   */

  /**
   * @attribute animation
   * @type {String}
   * @default default
   * @description
   *  [en]The animation used when showing and hiding the expandable content. Can be either `"default"` or `"none"`.[/en]
   *  [ja][/ja]
   */

  function ListItemElement() {
    _classCallCheck(this, ListItemElement);

    var _this = _possibleConstructorReturn(this, (ListItemElement.__proto__ || Object.getPrototypeOf(ListItemElement)).call(this));

    _this._animatorFactory = _this._updateAnimatorFactory();
    _this.toggleExpansion = _this.toggleExpansion.bind(_this);

    // Elements ignored when tapping
    var re = /^ons-(?!col$|row$|if$)/i;
    _this._shouldIgnoreTap = function (e) {
      return e.hasAttribute('prevent-tap') || re.test(e.tagName);
    };

    // show and hide functions for Vue hidable mixin
    _this.show = _this.showExpansion;
    _this.hide = _this.hideExpansion;

    (0, _contentReady2.default)(_this, function () {
      _this._compile();
    });
    return _this;
  }

  /**
   * Compiles the list item.
   *
   * Various elements are allowed in the body of a list item:
   *
   *  - div.left, div.right, and div.center are allowed as direct children
   *  - if div.center is not defined, anything that isn't div.left, div.right or div.expandable-content will be put in a div.center
   *  - if div.center is defined, anything that isn't div.left, div.right or div.expandable-content will be ignored
   *  - if list item has expandable attribute:
   *      - div.expandable-content is allowed as a direct child
   *      - div.top is allowed as direct child
   *      - if div.top is defined, anything that isn't div.expandable-content should be inside div.top - anything else will be ignored
   *      - if div.right is not defined, a div.right will be created with a drop-down chevron
   *
   * See the tests for examples.
   */


  _createClass(ListItemElement, [{
    key: '_compile',
    value: function _compile() {
      _autostyle2.default.prepare(this);
      this.classList.add(defaultClassName);

      var top = void 0,
          expandableContent = void 0;
      var topContent = [];
      Array.from(this.childNodes).forEach(function (node) {
        if (node.nodeType !== Node.ELEMENT_NODE) {
          topContent.push(node);
        } else if (node.classList.contains('top')) {
          top = node;
        } else if (node.classList.contains('expandable-content')) {
          expandableContent = node;
        } else {
          topContent.push(node);
        }

        if (node.nodeName !== 'ONS-RIPPLE') {
          node.remove();
        }
      });
      topContent = top ? Array.from(top.childNodes) : topContent;

      var left = void 0,
          right = void 0,
          center = void 0;
      var centerContent = [];
      topContent.forEach(function (node) {
        if (node.nodeType !== Node.ELEMENT_NODE) {
          centerContent.push(node);
        } else if (node.classList.contains('left')) {
          left = node;
        } else if (node.classList.contains('right')) {
          right = node;
        } else if (node.classList.contains('center')) {
          center = node;
        } else {
          centerContent.push(node);
        }
      });

      if (this.hasAttribute('expandable')) {
        this.classList.add('list-item--expandable');

        if (!top) {
          top = document.createElement('div');
          top.classList.add('top');
        }
        top.classList.add('list-item__top');
        this.appendChild(top);
        this._top = top;

        if (expandableContent) {
          expandableContent.classList.add('list-item__expandable-content');
          this.appendChild(expandableContent);
        }

        if (!right) {
          right = document.createElement('div');
          right.classList.add('list-item__right', 'right');

          // We cannot use a pseudo-element for this chevron, as we cannot animate it using
          // JS. So, we make a chevron span instead.
          var chevron = document.createElement('span');
          chevron.classList.add('list-item__expand-chevron');
          right.appendChild(chevron);
        }
      } else {
        top = this;
      }

      if (!center) {
        center = document.createElement('div');
        center.classList.add('center');
        centerContent.forEach(function (node) {
          return center.appendChild(node);
        });
      }
      center.classList.add('list-item__center');
      top.appendChild(center);

      if (left) {
        left.classList.add('list-item__left');
        top.appendChild(left);
      }
      if (right) {
        right.classList.add('list-item__right');
        top.appendChild(right);
      }

      _util2.default.updateRipple(this);
      _modifierUtil2.default.initModifier(this, scheme);
    }

    /**
     * @method showExpansion
     * @signature showExpansion()
     * @description
     *   [en]Show the expandable content if the element is expandable.[/en]
     *   [ja][/ja]
     */

  }, {
    key: 'showExpansion',
    value: function showExpansion() {
      var _this2 = this;

      if (this.hasAttribute('expandable') && !this._expanding) {
        this.expanded = true;
        this._expanding = true;

        var animator = this._animatorFactory.newAnimator();
        animator.showExpansion(this, function () {
          _this2.classList.add('expanded');
          _this2._expanding = false;
        });
      }
    }

    /**
     * @method hideExpansion
     * @signature hideExpansion()
     * @description
     *   [en]Hide the expandable content if the element expandable.[/en]
     *   [ja][/ja]
     */

  }, {
    key: 'hideExpansion',
    value: function hideExpansion() {
      var _this3 = this;

      if (this.hasAttribute('expandable') && !this._expanding) {
        this.expanded = false;
        this._expanding = true;

        var animator = this._animatorFactory.newAnimator();
        animator.hideExpansion(this, function () {
          _this3.classList.remove('expanded');
          _this3._expanding = false;
        });
      }
    }
  }, {
    key: 'toggleExpansion',
    value: function toggleExpansion() {
      this.classList.contains('expanded') ? this.hideExpansion() : this.showExpansion();
      this.dispatchEvent(new Event('expansion'));
    }
  }, {
    key: '_updateAnimatorFactory',
    value: function _updateAnimatorFactory() {
      return new _animatorFactory2.default({
        animators: _animatorDict,
        baseClass: _animator.ListItemAnimator,
        baseClassName: 'ListItemAnimator',
        defaultAnimation: this.getAttribute('animation') || 'default'
      });
    }
  }, {
    key: 'attributeChangedCallback',
    value: function attributeChangedCallback(name, last, current) {
      switch (name) {
        case 'class':
          _util2.default.restoreClass(this, defaultClassName, scheme);
          break;
        case 'modifier':
          _modifierUtil2.default.onModifierChanged(last, current, this, scheme);
          break;
        case 'ripple':
          _util2.default.updateRipple(this);
          break;
        case 'animation':
          this._animatorFactory = this._updateAnimatorFactory();
          break;
      }
    }
  }, {
    key: 'connectedCallback',
    value: function connectedCallback() {
      var _this4 = this;

      (0, _contentReady2.default)(this, function () {
        _this4._setupListeners(true);
        _this4._originalBackgroundColor = _this4.style.backgroundColor;
        _this4.tapped = false;
      });
    }
  }, {
    key: 'disconnectedCallback',
    value: function disconnectedCallback() {
      this._setupListeners(false);
    }
  }, {
    key: '_setupListeners',
    value: function _setupListeners(add) {
      var action = (add ? 'add' : 'remove') + 'EventListener';
      _util2.default[action](this, 'touchstart', this._onTouch, { passive: true });
      _util2.default[action](this, 'touchmove', this._onRelease, { passive: true });
      this[action]('touchcancel', this._onRelease);
      this[action]('touchend', this._onRelease);
      this[action]('touchleave', this._onRelease);
      this[action]('drag', this._onDrag);
      this[action]('mousedown', this._onTouch);
      this[action]('mouseup', this._onRelease);
      this[action]('mouseout', this._onRelease);

      if (this._top) {
        this._top[action]('click', this.toggleExpansion);
      }
    }
  }, {
    key: '_onDrag',
    value: function _onDrag(event) {
      var gesture = event.gesture;
      // Prevent vertical scrolling if the users pans left or right.
      if (this.hasAttribute('lock-on-drag') && ['left', 'right'].indexOf(gesture.direction) > -1) {
        gesture.preventDefault();
      }
    }
  }, {
    key: '_onTouch',
    value: function _onTouch(e) {
      var _this5 = this;

      if (this.tapped || this !== e.target && (this._shouldIgnoreTap(e.target) || _util2.default.findParent(e.target, this._shouldIgnoreTap, function (p) {
        return p === _this5;
      }))) {
        return; // Ignore tap
      }

      this.tapped = true;
      var touchStyle = { transition: 'background-color 0.0s linear 0.02s, box-shadow 0.0s linear 0.02s' };

      if (this.hasAttribute('tappable')) {
        if (this.style.backgroundColor) {
          this._originalBackgroundColor = this.style.backgroundColor;
        }

        touchStyle.backgroundColor = this.getAttribute('tap-background-color') || '#d9d9d9';
        touchStyle.boxShadow = '0px -1px 0px 0px ' + touchStyle.backgroundColor;
      }

      (0, _styler2.default)(this, touchStyle);
    }
  }, {
    key: '_onRelease',
    value: function _onRelease() {
      this.tapped = false;
      this.style.backgroundColor = this._originalBackgroundColor || '';
      _styler2.default.clear(this, 'transition boxShadow');
    }
  }, {
    key: 'expandableContent',
    get: function get() {
      return this.querySelector('.list-item__expandable-content');
    }
  }, {
    key: 'expandChevron',
    get: function get() {
      return this.querySelector('.list-item__expand-chevron');
    }
  }], [{
    key: 'observedAttributes',
    get: function get() {
      return ['modifier', 'class', 'ripple', 'animation'];
    }
  }]);

  return ListItemElement;
}(_baseElement2.default);

exports.default = ListItemElement;


_elements2.default.ListItem = ListItemElement;
customElements.define('ons-list-item', ListItemElement);