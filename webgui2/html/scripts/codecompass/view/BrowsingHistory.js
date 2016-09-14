define([
  'dojo/_base/array',
  'dojo/_base/declare',
  'dojo/_base/lang',
  'dojo/dom-class',
  'dojo/dom-construct',
  'dojo/store/Memory',
  'dojo/store/Observable',
  'dojo/topic',
  'dijit/Tree',
  'dijit/tree/ObjectStoreModel',
  'codecompass/urlHandler',
  'codecompass/registration/model',
  'codecompass/util'],
function (array, declare, lang, domClass, dom, Memory, Observable, topic, Tree,
  ObjectStoreModel, urlHandler, model, util) {
  
  var HtmlTreeNode = declare(Tree._TreeNode, {
    _setLabelAttr : { node : "labelNode", type : "innerHTML" }
  });
  
  var HtmlTree = declare(Tree, {
    _createTreeNode : function (args) {
      return new HtmlTreeNode(args);
    }
  });
  
  var IterableStack = declare(null, {
    constructor : function () {
      this._stack = [];
      this._index = -1;
    },
    
    /**
     * This function removes the items from the stack after the 'index', pushes
     * the new item and sets 'index' to sign the index of last element.
     * @param {type} item Item to be pushed.
     */
    push : function (item) {
      ++this._index;
      this._stack.splice(this._index, this._stack.length - this._index, item);
    },
    
    /**
     * This function removes the topmost element of the stack and sets the
     * 'index' to sign the index of the last element.
     */
    pop : function () {
      var item = this._stack.pop();
      this._index = this._stack.length - 1;
      return item;
    },
    
    /**
     * This function returns the topmost element of the stack. Note that it is
     * not necessarily equal to the current item. See: getCurrent().
     */
    top : function () {
      return this._stack[this._stack.length - 1];
    },
    
    /**
     * This function decrements the 'index' variable if its value still points
     * to an element of the stack i.e. its new value is not negative.
     */
    prev : function () {
      if (this._index > 0)
        --this._index;
    },
    
    /**
     * This function increments the 'index' variable if its value still points
     * to an element of the stack i.e. its new value is less or equal of the
     * index of the last element.
     */
    next : function () {
      if (this._index < this._stack.length - 1)
        ++this._index;
    },
    
    /**
     * This function returns the current element in the stack. Note that it is
     * not necessarily equal to the topmost item. See: top().
     */
    getCurrent : function () {
      return this._stack[this._index];
    },
    
    /**
     * This function returns the item before the current one;
     */
    getPrev : function () {
      return this._stack[this._index - 1];
    },
    
    /**
     * This function returns the item next to the current one;
     */
    getNext : function () {
      return this._stack[this._index + 1];
    },
    
    /**
     * This function returns true if the stack is empty, false otherwise.
     */
    isEmpty : function () {
      return this._index === -1;
    }
  });
  
  /**
   * This function checks if the string and object representation of an URL
   * matches.
   * @param {String} url String representation of an URL's last part.
   * @param {Object} obj Object representation of an URL's last part.
   */
  function isSameUrl(url, obj) {
    var parts = url.split('&');
    
    if (parts.length !== Object.keys(obj).length)
      return false;
    
    return array.every(parts, function (item) {
      item = item.split('=');
      return obj[item[0]] + "" === item[1];
    });
  }
  
  function addFromToLinks(node, fromFileRange, toFileRange) {
    var fromFileInfo = model.project.getFileInfo(fromFileRange.file);
    var toFileInfo   = model.project.getFileInfo(toFileRange.file);
    
    var fromLink = 'From: <span class="jumpfrom">'
                 + fromFileInfo.name
                 + ', line: '
                 + fromFileRange.range.startpos.line
                 + '</span>';
    
    node.labelNode.innerHTML += ' ('
                             +  toFileInfo.name
                             +  ', line: '
                             +  toFileRange.range.startpos.line
                             +  ')<br/>'
                             +  fromLink;

    node.labelNode.innerHTML = '<span style="display: inline-table">'
                             + node.labelNode.innerHTML
                             + '</span>';
  }
  
  /**
   * This function returns true if the given node in browsing history tree is
   * not member of the current branch between browsinghistory._currentLeaf and
   * the root.
   * @param {BrowsingHistory} browsinghistory BrowsingHistory object.
   * @param {} node Tree node.
   */
  function isOtherBranch(browsinghistory, node) {
    return array.every(browsinghistory._getBranchItems(), function (item) {
      return item !== node;
    });
  }
  
  return declare(HtmlTree, {
    constructor : function () {
      var that = this;
      
      this._initialUrl = urlHandler.getState();
      
      this._data  = [];
      this._stack = new IterableStack();
      
      this._store = new Observable(new Memory({
        data : this._data,
        getChildren : function (node) {
          return that._store.query({ parent : node.id });
        }
      }));
      
      this._store.add({ id : 'index', name : 'index', parent : null });
      
      this.model = new ObjectStoreModel({
        store           : this._store,
        query           : { id : 'index' },
        mayHaveChildren : function (node) { return true; }
      });
      
      this.onClick = function (item, node, event) {
        that._blockHashchangeEvent = true;
        
        that._stack.getCurrent().setSelected(false);
        that._stack.push(node);
        
        if (isOtherBranch(that, node))
          that._currentLeaf = node;
        
        topic.publish('codecompass/setstate',
          domClass.contains(event.target, 'jumpfrom')
            ? item.fromUrl
            : item.toUrl);
      };
      
      topic.subscribe('codecompass/openFile', function (message) {
        if (message.info) {
          // When a file is opened then hash change event also runs since url
          // changes. By this indicator variable we can check whether hash
          // change event is triggered because of browser back or because of an
          // open file event.
          that._blockHashchangeEvent = true;
          
          if (message.jumpFrom) {
            var fromUrl = that._stack.isEmpty()
                        ? that._initialUrl
                        : lang.clone(that._stack.getCurrent().item.toUrl);
            
            fromUrl.select = util.rangeToString(
              message.jumpFrom.range.startpos,
              message.jumpFrom.range.endpos);
          }
          
          that._store.add({
            name    : message.info,
            fromUrl : fromUrl,
            toUrl   : urlHandler.getState(),
            parent  : message.newSession || that._stack.isEmpty()
                    ? 'index'
                    : that._stack.getCurrent().item.id
          });
          
          if (!that._stack.isEmpty())
            that._stack.getCurrent().setSelected(false);
          
          if (message.newSession || that._stack.isEmpty()) {
            // Jump over hidden root node
            var children = that.getChildren()[0].getChildren();
            that._stack.push(children[children.length - 1]);
          } else {
            that._stack.push(
              that._stack.getCurrent().getChildren().slice(-1)[0]);
          }
          
          if (message.jumpFrom)
            addFromToLinks(
              that._stack.getCurrent(), message.jumpFrom, message.jumpTo);
          
          that._stack.getCurrent().setSelected();
          that.expandAll(); // TODO: expand only parent node
          
          that._currentLeaf = that._stack.getCurrent();
        }
      });
      
      topic.subscribe('/dojo/hashchange', function (url) {
        if (that._blockHashchangeEvent) {
          that._blockHashchangeEvent = false;
          return;
        }
        
        if (that._stack.isEmpty())
          return;
        
        that._stack.getCurrent().setSelected(false);
        
        var prev = that._stack.getPrev();
        var next = that._stack.getNext();
        
        // TODO: Maybe it worth searching in the full stack if better
        // syncronization is needed.
        if (prev && (isSameUrl(url, prev.item.fromUrl || {}) ||
                     isSameUrl(url, prev.item.toUrl)))
          that._stack.prev();
        else if (next && (isSameUrl(url, next.item.toUrl) ||
                          isSameUrl(url, next.item.fromUrl || {})))
          that._stack.next();
        
        that._stack.getCurrent().setSelected();
        
        if (isOtherBranch(that, that._stack.getCurrent()))
          that._currentLeaf = that._stack.getCurrent();
      });
    },
    
    _getBranchItems : function () {
      if (!this._currentLeaf)
        return [];
      
      var rootNode = this.get('rootNode');
      var current = this._currentLeaf;
      var result = [];
      
      while (current !== rootNode) {
        result.unshift(current);
        current = current.getParent();
      }
      
      return result;
    }
  });
});
