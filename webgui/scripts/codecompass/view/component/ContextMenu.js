define([
  'dojo/_base/declare',
  'dojo/dom-class',
  'dijit/Menu',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'dijit/Tooltip',
  'dijit/_WidgetBase'],
function (declare, domClass, Menu, MenuItem, PopupMenuItem, Tooltip,
  _WidgetBase) {
    
  return declare(Menu, {
    /**
     * This function adds an item to the popup menu.
     * @param {_WidgetBase | Object} item Item to add to the menu. It can be
     * either a _WidgetBase dojo object or a custom object which has to have the
     * following attributes:
     * - label: Label of the menu
     * - path (optional): A string array which defines the submenus under which
     *   the item is placed with the given label.
     * - onClick: Callback function to call when click on the menu item.
     * - helpText (optional): If given then a question mark is placed next to
     *   the label. Hovering the mouse over it, this text will be displayed.
     * - shortcut (optional): A key-combo (like Ctrl-C).
     */
    addChild : function (item) {
      if (item.isInstanceOf && item.isInstanceOf(_WidgetBase)) {
        this.inherited(arguments);
        return;
      }
      
      var path = item.path || [];
      
      var current = this;
      for (var levelIdx = 0; levelIdx < path.length; ++levelIdx) {
        var menuItems = current.getChildren();
        
        for (var itemIdx = 0; itemIdx < menuItems.length; ++itemIdx)
          if (menuItems[itemIdx].label === path[levelIdx])
            break;
        
        if (itemIdx == menuItems.length) {
            var subMenu = new Menu();
            current.addChild(new PopupMenuItem({
              label : path[levelIdx],
              popup : subMenu
            }));
            current = subMenu;
        } else {
          // Here we suppose that a popup type menu item is never clickable i.e.
          // server never sends menu items with name [a, b, c] and [a, b].
          current = menuItems[itemIdx].popup;
        }
      }
      
      current.addChild(new MenuItem({
        label     : item.label,
        onClick   : item.onClick,
        iconClass : item.helpText ? 'menuicon helpicon' : undefined,
        accelKey  : item.shortcut,
        
        onMouseOver : function (node) {
          if (domClass.contains(node.target, 'helpicon'))
            Tooltip.show(item.helpText, node.target, ['above']);
        },
        
        onMouseOut : function (node) {
          Tooltip.hide(node.target);
        }
      }));
    },
    
    /**
     * This function removes all items from the context menu.
     */
    clear : function () {
      this.getChildren().forEach(function (child) {
        this.removeChild(child);
      }, this);
    }
  });
});
