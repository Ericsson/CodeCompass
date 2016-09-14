define([
  'dojo/_base/array',
  'dojo/_base/declare',
  'dojo/dom-construct',
  'dojo/dom-style',
  'dojo/keys',
  'dijit/_WidgetBase',
  'dijit/Toolbar',
  'dijit/ToolbarSeparator',
  'dijit/form/Button',
  'dijit/form/NumberTextBox',
  'dijit/form/Select'],
function (array, declare, dom, style, keys, _WidgetBase, Toolbar,
  ToolbarSeparator, Button, NumberTextBox, Select) {
    
  var Total = declare(_WidgetBase, {
    buildRendering : function () {
      this.domNode = dom.create('span', {
        innerHTML : 'of 1',
        style     : 'margin-left: 5px;'
      });
    },
    
    _setValueAttr : function (value) {
      this.value = value;
      this.domNode.innerHTML = 'of ' + value;
    },
    
    value : 1
  });

  /**
   * This function sets a new value to the page number text box if the given
   * value is valid i.e. is in the range determined by the constraints of the
   * field.
   * @param {Pager} pager The modified pager object.
   * @param {Number} number The new value.
   */
  function setPageNumberIfValid(pager, number) {
    if (pager._pageNumber.rangeCheck(number, pager._pageNumber.constraints))
      pager._pageNumber.set('value', number);
  }

  /**
   * This function sets a new value to the page size select item if the given
   * value is valid i.e. the value is among the selectable options.
   * @param {Pager} pager The modified pager object.
   * @param {Number} number The new value.
   */
  function setPageSizeIfValid(pager, number) {
    if (array.some(pager._pageSize.get('options'), function (option) {
          return option.value === number;
        }))
      pager._pageSize.set('value', number);
  }

  return declare(Toolbar, {
    constructor : function (params) {
      var that = this;

      /**
       * This function is used for sake of brevity, so that its body not be
       * written many times in the code.
       */
      function valueChangeCallback() {
        // This check is needed because this callback function is called twice
        // on page size change, since that sets page number to 1 which also runs
        // this callback.
        if (that._prevSize === that._pageSize.get('value') &&
            that._prevPage === that._pageNumber.get('value'))
          return;
        
        that._prevSize = that._pageSize.get('value');
        that._prevPage = that._pageNumber.get('value');
        
        if (that._pageNumber.isInRange())
          that.onChange(
            that._pageNumber.get('value'),
            that._pageSize.get('value'));
      };

      //--- Page select combo box ---//

      this._pageSize = new Select({
        options : [
          { label : '10', value : 10 },
          { label : '20', value : 20 },
          { label : '30', value : 30 },
          { label : '40', value : 40 },
          { label : '50', value : 50 }
        ],
        value    : 10,
        onChange : function () {
          that._pageNumber.set('value', 1);
          valueChangeCallback();
        }
      });

      //--- Page number text box ---//

      this._pageNumber = new NumberTextBox({
        constraints : params.constraints || { min : 1, max : 1 },
        value       : params.constraints ? params.constraints.min : 1,
        onChange    : valueChangeCallback,
        onKeyDown   : function (event) {
          if (event.keyCode === keys.ENTER)
            valueChangeCallback();
        }
      });

      //--- Total items ---//

      this._total = new Total();

      style.set(this._pageNumber.domNode, 'width', '50px');
    },

    postCreate : function () {
      var that = this;

      this.addChild(this._pageSize);

      this.addChild(new ToolbarSeparator());

      this.addChild(new Button({
        showLabel : false,
        iconClass : 'dijitCalendarIncrementControl dijitCalendarDecrease',
        onClick   : function () {
          that.set('pageNumber', that.get('pageNumber') - 1);
        }
      }));

      this.addChild(this._pageNumber);

      this.addChild(this._total);

      this.addChild(new Button({
        showLabel : false,
        iconClass : 'dijitCalendarIncrementControl dijitCalendarIncrease',
        onClick   : function () {
          that.set('pageNumber', that.get('pageNumber') + 1);
        }
      }));
    },

    _getPageNumberAttr : function () { return this._pageNumber.get('value'); },
    _getPageSizeAttr   : function () { return this._pageSize.get('value');   },

    _setPageNumberAttr : function (value) { setPageNumberIfValid(this, value); },
    _setPageSizeAttr   : function (value) { setPageSizeIfValid(this, value);   },
    _setTotalAttr      : function (value) {
      value = Math.ceil(value / this._pageSize.get('value'));
      this._pageNumber.get('constraints').max = value;
      this._total.set('value', value);
    },

    /**
     * This callback function can be set. The callback is called when the page
     * number or page size value is changed or refresh button is clicked.
     * // TODO: there is no refresh button yet.
     * @param {Number} pageNumber The function is passed the new page number.
     * @param {Number} pageSize The function is passed the new page size.
     */
    onChange : function (pageNumber, pageSize) {}
  });
});