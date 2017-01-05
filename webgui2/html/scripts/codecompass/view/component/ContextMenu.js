define([
  'dojo/_base/declare',
  'dojo/_base/array',
  'dojo/dom-class',
  'dijit/Menu',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'dijit/Tooltip',
  'dijit/_WidgetBase'],
function (declare, array, domClass, Menu, MenuItem, PopupMenuItem, Tooltip,
  _WidgetBase) {
    
  return declare(Menu, {
    /**
     * This function adds an item to the popup menu.
     * @param {_WidgetBase (dojo) | Object} item Item to add to the menu. It can
     * be either a _WidgetBase dojo object or a custom object which has to have
     * the following attributes:
     * - menuType: MenuType thrift object
     * - onClick: Callback function to call when click on the menu item.
     */
    addChild : function (item) {
      if (item.isInstanceOf && item.isInstanceOf(_WidgetBase)) {
        this.inherited(arguments);
        return;
      }
      
      var menuType = item.menuType;
      
      var current = this;
      for (var levelIdx = 0, levelMax = menuType.name.length;
           levelIdx < levelMax;
           ++levelIdx) {

        var menuItems = current.getChildren();
        
        for (var itemIdx = 0, itemMax = menuItems.length;
             itemIdx < itemMax;
             ++itemIdx)
          if (menuItems[itemIdx].label  === menuType.name[levelIdx])
            break;
        
        if (itemIdx === menuItems.length) {
          if (levelIdx === levelMax - 1) {
            current.addChild(new MenuItem({
              label     : menuType.name[levelIdx],
              onClick   : item.onClick,
              iconClass : menuType.helpText ? 'menuicon helpicon' : undefined,
              accelKey  : menuType.shortcut,
              
              onMouseOver : function (node) {
                if (domClass.contains(node.target, 'helpicon'))
                  Tooltip.show(menuType.helpText, node.target, ['above']);
              },
              
              onMouseOut : function (node) {
                Tooltip.hide(node.target);
              }
            }));
          } else {
            var subMenu = new Menu();
            current.addChild(new PopupMenuItem({
              label : menuType.name[levelIdx],
              popup : subMenu
            }));
            current = subMenu;
          }
        } else {
          // Here we suppose that a popup type menu item is never clickable i.e.
          // server never sends menu items with name [a, b, c] and [a, b].
          current = menuItems[itemIdx].popup;
        }
      }
    },
    
    /**
     * This function removes all items from the context menu.
     */
    clear : function () {
      array.forEach(this.getChildren(), function (child) {
        this.removeChild(child);
      }, this);
    }
  });
});