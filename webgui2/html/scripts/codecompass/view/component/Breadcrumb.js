define([
  'dojo/dom-construct',
  'dojo/dom-style',
  'dojo/on',
  'dojo/topic',
  'dojo/_base/declare',
  'dijit/_WidgetBase',
  'codecompass/viewHandler'],
function (dom, style, on, topic,declare, _WidgetBase, viewHandler) {
  
  function buildElems(breadcrumb) {
    dom.empty(breadcrumb._containerDiv);

    var browsingHistory = viewHandler.getModule('browsinghistory');

    breadcrumb._sumWidth = 0;
    breadcrumb._divs = [];
    var div, triangle;
    browsingHistory._getBranchItems().forEach(function (item) {
      div = dom.create('div', {
        innerHTML : item.get('label'),
        class     : 'elem'
      });

      triangle = dom.create('div', {
        class : 'triangle elem'
      });

      breadcrumb._divs.push(div);
      breadcrumb._divs.push(triangle);
      dom.place(div, breadcrumb._containerDiv);
      dom.place(triangle, breadcrumb._containerDiv);

      // 5 = margin-right
      breadcrumb._sumWidth += parseInt(style.getComputedStyle(div).width) + 5;
      // Arrow width + margin-right
      breadcrumb._sumWidth += 5 + 5;

      on(div, 'click', function (event) {
        on.emit(item.contentNode, 'click', event);
      });
    });

    breadcrumb._sumWidth -= 10;
    dom.destroy(triangle);
  }
  
  /**
   * This function returns the minimum and maximum possible values of the "left"
   * attribute of the _containerDiv which contains the breadcrumb elements.
   * @param {Breadcrumb} breadcrumb A breadcrumb object.
   */
  function minmaxLeft(breadcrumb) {
    // 30 = width of pager
    return {
      min : -breadcrumb._sumWidth + breadcrumb._width - 30,
      max : 30
    };
  }
  
  /**
   * This function computes the current size of some divs and stores the result
   * in private variables.
   * @param {Breadcrumb} breadcrumb A breadcrumb object.
   */
  function computeSizes(breadcrumb) {
    breadcrumb._width
      = parseInt(style.getComputedStyle(breadcrumb.domNode).width);
    breadcrumb._padding
      = parseInt(style.getComputedStyle(breadcrumb.domNode)['padding-left']);
  }
  
  return declare(_WidgetBase, {
    constructor : function () {
      var that = this;
      this._browsingHistory = viewHandler.getModule('browsinghistory');
      
      function refresh() {
        buildElems(that);
        computeSizes(that);
        
        if (that._sumWidth > that._width - 2 * 30) {
          var minmax = minmaxLeft(that);
          style.set(that._containerDiv, 'left', minmax.min + 'px');
        }
      }
      
      topic.subscribe('codecompass/openFile', refresh);
      topic.subscribe('codecompass/setstate', refresh);
      topic.subscribe('/dojo/hashchange',     refresh);
    },
    
    buildRendering : function () {
      var that = this;
      
      var maindiv = dom.create('div', { class : 'breadcrumb' });
      
      this._containerDiv = dom.create('div', {
        style : 'position: relative; left: 30px'
      });
      
      dom.place(this._containerDiv, maindiv);
      
      //--- Pager buttons ---//
      
      this._pagerLeft  = dom.create('div', { class : 'pager left'  });
      this._pagerRight = dom.create('div', { class : 'pager right' });
      
      dom.place(this._pagerLeft,  maindiv);
      dom.place(this._pagerRight, maindiv);
      
      function scroll(pixel) {
        computeSizes(that);
  
        that._scroll = setInterval(function () {
          var left = parseInt(style.getComputedStyle(that._containerDiv).left);
          var minmax = minmaxLeft(that);
          
          if (left + pixel <= minmax.max && pixel > 0 ||
              left + pixel >= minmax.min && pixel < 0)
            style.set(that._containerDiv, 'left', left + pixel + 'px');
        }, 0);
      }
      
      on(this._pagerLeft,  'mousedown', function () { scroll(+2); });
      on(this._pagerRight, 'mousedown', function () { scroll(-2); });
      
      on(this._pagerLeft, 'mouseup', function () {
        clearTimeout(that._scroll);
      });
      
      on(this._pagerRight, 'mouseup', function () {
        clearTimeout(that._scroll);
      });
      
      on(this._pagerLeft, 'mouseout', function () {
        clearTimeout(that._scroll);
      });
      
      on(this._pagerRight, 'mouseout', function () {
        clearTimeout(that._scroll);
      });
      
      this.domNode = maindiv;
    }
  });
});