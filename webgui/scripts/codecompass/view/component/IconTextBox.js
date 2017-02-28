define([
  'dojo/dom-construct',
  'dojo/dom-style',
  'dojo/_base/declare',
  'dojo/query!css2',
  'dojo/on',
  'dojo/keys',
  'dijit/form/ValidationTextBox'],
function (dom, style, declare, query, on, keys, ValidationTextBox) {

  return declare(ValidationTextBox, {
    constructor : function (params) {
      this._button = dom.create('span', { class : params.icon });
    },

    postCreate : function () {
      var that = this;

      var inputContainer = query('.dijitInputContainer', this.domNode)[0];
      var inputField = query('input', inputContainer)[0];

      //--- Create and style button ---//

      // TODO: button size should match in all themes
      style.set(this._button, 'position', 'absolute');
      style.set(this._button, 'right', '2px');
      style.set(this._button, 'cursor', 'pointer');

      on(this._button, 'click', function () {
        that.onSubmit(that.get('value'));
      });
      
      on(inputField, 'keypress', function (event) {
        if (event.keyCode === keys.ENTER)
          that.onSubmit(that.get('value'));
      });

      //--- Add button ---//

      dom.place(this._button, inputContainer);
    },

    /**
     * The onSubmit callback function is called when Enter key is pressed or
     * icon is clicked.
     * @param {String} value The callback function is given the value of the
     * text box as parameter.
     */
    onSubmit : function (value) {},

    /**
     * This function sets the icon image.
     * @param {type} icon Icon image file.
     */
    _setIconAttr : function (icon) {
      this._button.setAttribute('src', icon);
    },

    /**
     * This function returns the icon file name.
     */
    _getIconAttr : function () {
      return this._button.getAttribute('src');
    },

    /**
     * This function sets the placeholder attribute directly to the input html
     * tag.
     * // TODO: this hack is needed, because the dojo's own placeholder doesn't
     * work well. Dojo doesn't use the native placeholder tag of the input tag
     * but places a layer over it. In this layer the placeholder text is not
     * hidden while typing. 
     */
    _setPlaceHolderAttr : function (text) {
      var that = this;
      require(['dojo/query', 'dojo/dom-attr'], function (query, attr) {
        var input = query('input', that.domNode)[1];
        attr.set(input, 'placeholder', text);
      });
    }
  });
});