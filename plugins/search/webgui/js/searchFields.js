require([
  'dojo/dom-construct',
  'dojo/_base/declare',
  'dojo/topic',
  'dojo/keys',
  'dojo/store/Memory',
  'dijit/_WidgetBase',
  'dijit/form/RadioButton',
  'dijit/form/CheckBox',
  'dijit/form/ComboBox',
  'dijit/form/DropDownButton',
  'dijit/Fieldset',
  'dijit/Tooltip',
  'dijit/TooltipDialog',
  'dijit/form/Select',
  'dijit/popup',
  'codecompass/viewHandler',
  'codecompass/view/component/IconTextBox',
  'codecompass/model'],
function (dom, declare, topic, keys, Memory, _WidgetBase, RadioButton, CheckBox,
  ComboBox, DropDownButton, Fieldset, Tooltip, TooltipDialog, Select, popup,
  viewHandler, IconTextBox, model) {

  //--- Register service ---//

  model.addService('searchservice', 'SearchService', SearchServiceClient);

  SearchFields = declare(_WidgetBase, {
    _placeholders : {
      text : 'Search expression, like "foo AND bar".',
      file : 'File name regex (.*cpp$).',
      log  : 'Arbitrary log message (e.g. ERROR: foobar.cpp something went \
              wrong at 12:34).',
      fileFilter : 'File name filter regex (.*cpp).',
      dirFilter  : 'Path filter regex (click on a dir below).'
    },

    _fileFilterTooltipLabel : 'This filter is a regular expression of file \
      names. <br> Example: .*cpp',

    _dirFilterTooltipLabel : 'This filter is a regular expression of directory \
      paths. <br> Example: /path/to/source/subdir/.*',

    _searchTooltipLabel :
      'For <b>"Text search"</b> and <b>"Definition search"</b> you can use \
      wildcards, regex, and logical operators: \
      <ul> \
        <li><b>Phrases:</b> a Phrase is a group of words surrounded by \
          double quotes such as "hello dolly". <br> \
          A Phrase query matches if the words are next to each other in the \
          particular order. \
        </li> \
        <li><b>Wildcards:</b> single and multiple character wildcard searches \
          within single terms.<br> \
          Examples: &nbsp;&nbsp;&nbsp; *test &nbsp;&nbsp;&nbsp; te?t \
          &nbsp;&nbsp;&nbsp; test* &nbsp;&nbsp;&nbsp; te*t <br> \
          <b>Note:</b> you cannot use wildcard searches within phrase queries! \
        </li> \
        <li><b>Regular Expressions:</b> matching a pattern between forward \
          slashes.<br> Example: /[mb]oat/ \
        </li> \
        <li><b>Boolean Operators:</b> Boolean operators allow terms to be \
          combined through logic operators. <br> \
          We support AND, "+", OR, NOT and "-" as Boolean operators. \
          (<b>Note</b>: Boolean operators must be ALL CAPS) <br> \
          <ul> \
            <li><b>OR:</b> The OR operator links two terms and finds a \
              matching document if either of the terms exist in a document. \
              <br><b>If there is no Boolean operator between two terms, the OR \
              operator is used.</b><br> \
              Example: "some text" OR other \
            </li> \
            <li><b>AND:</b> The AND operator matches documents where both \
              terms exist anywhere in the text of a single document.<br> \
              Example: "some text" AND other \
            </li> \
          </ul> \
        </li>\
        <li><b>Escaping Special Characters:</b> The current list special \
          characters are:<br> \
          + &nbsp; - &nbsp; && &nbsp; || &nbsp; ! &nbsp; ( &nbsp; ) &nbsp; \
          { &nbsp; } &nbsp; [ &nbsp; ] &nbsp; ^ &nbsp; " &nbsp; ~ &nbsp; * \
          ? &nbsp; : &nbsp; \\ &nbsp; / <br> \
          <b>The escape character is the "\\" (back slash).</b> \
        </li> \
      </ul> \
      In <b>"File name search"</b> you can use a regular expression for \
      matching on full (absolute) path of files in the database. <br> \
      Example: /path/.*/te?t\\.cpp <br><br> \
      For the documentation of <b>"Log search"</b> please see the user manual.',

    _mimeTypes: [
      { id : 'text/x-asm',         name : 'text/x-asm'         },
      { id : 'text/x-bcpl',        name : 'text/x-bcpl'        },
      { id : 'text/x-c',           name : 'text/x-c'           },
      { id : 'text/x-c++',         name : 'text/x-c++'         },
      { id : 'text/x-awk',         name : 'text/x-awk'         },
      { id : 'text/x-gawk',        name : 'text/x-gawk'        },
      { id : 'text/x-nawk',        name : 'text/x-nawk'        },
      { id : 'text/x-php',         name : 'text/x-php'         },
      { id : 'text/x-shellscript', name : 'text/x-shellscript' },
      { id : 'text/x-fortran',     name : 'text/x-fortran'     },
      { id : 'text/x-po',          name : 'text/x-po'          },
      { id : 'text/x-java',        name : 'text/x-java'        },
      { id : 'text/x-java-source', name : 'text/x-java-source' },
      { id : 'text/x-xmcd',        name : 'text/x-xmcd'        },
      { id : 'text/x-lisp',        name : 'text/x-lisp'        },
      { id : 'text/x-lua',         name : 'text/x-lua'         },
      { id : 'text/x-m4',          name : 'text/x-m4'          },
      { id : 'text/x-makefile',    name : 'text/x-makefile'    },
      { id : 'text/x-vcard',       name : 'text/x-vcard'       },
      { id : 'text/x-msdos-batch', name : 'text/x-msdos-batch' },
      { id : 'text/x-pascal',      name : 'text/x-pascal'      },
      { id : 'text/x-perl',        name : 'text/x-perl'        },
      { id : 'text/x-python',      name : 'text/x-python'      },
      { id : 'text/x-ruby',        name : 'text/x-ruby'        },
      { id : 'text/x-tcl',         name : 'text/x-tcl'         },
      { id : 'text/x-info',        name : 'text/x-info'        },
      { id : 'text/x-tex',         name : 'text/x-tex'         },
      { id : 'text/x-texinfo',     name : 'text/x-texinfo'     }
    ],

    _langMap : {
      cpp: {
        name: 'C/C++',
        mimeTypes: ['text/x-c', 'text/x-c++']
      },
      java: {
        name: 'Java',
        mimeTypes: ['text/x-java', 'text/x-java-source']
      },
      js: {
        name: 'JavaScript',
        mimeTypes: ['text/x-javascript', 'application/javascript']
      },
      shell: {
        name: 'Shell script',
        mimeTypes: ['text/x-shellscript', 'application/x-shellscript']
      },
      perl: {
        name: 'Perl',
        mimeTypes: ['text/x-perl']
      },
      python: {
        name: 'Python',
        mimeTypes: ['text/x-python']
      }
    },

    _langRadios : {},

    _searchHistory : [],

    constructor : function () {
      var that = this;

      var mimeTypes = new Memory({
        data : that._mimeTypes
      });

      this._mimeComboBox = new ComboBox({ store : mimeTypes }); 
      
      this._langMap.mime = {
        name : this._mimeComboBox,
        mimeTypes : []
      };

      topic.subscribe('codecompass/openFile', function (message) {
        that._setDirFilterTooltip(
          model.project.getFileInfo(message.fileId).path);
      });

      topic.subscribe('codecompass/searchError', function (message) {
        var exception = message.exception;

        if (exception)
          that._search.set(
            'invalidMessage',
            exception.message || exception.what);

        that._search.validator = function (value) {
          return !exception;
        };

        that._search.validate();
      });
    },

    /**
     * When hovering the directory filter, then a Tooltip contains the path of
     * the opened file. The path items are clickable so filling the directory
     * filter by the regex describing the clicked item can be easier.
     * @param {String} path File path.
     */
    _setDirFilterTooltip : function (path) {
      var that = this;

      var result = dom.create('span');
      var string = '';

      path.split('/').slice(1).forEach(function (file) {
        string += '/' + file;
        var s = string; // This is needed because of closure affairs.

        dom.place(dom.toDom('/'), result);
        dom.place(dom.create('span', {
          innerHTML : file,
          class     : 'pathelement',
          onclick   : function () { that._dirFilter.setValue(s + '.*'); }
        }), result);
      });

      that._dirFilterDialogTooltip.set('content', result);
    },

    /**
     * @returns True if you can use the standard Lucene syntax.
     */
    _canUseLuceneSearchSyntax : function () {
      switch (this._searchType.get('value')) {
        case SearchOptions.SearchInSource:
        case SearchOptions.SearchInDefs:
          return true;
        default:
          return false;
      }
    },

    /**
     * Text search query is established by a query string in a given format.
     * This function assembles this query string based on the check state of
     * checkboxes in "typeCheckboxes" and "langCheckboxes" array.
     */
    _assembleQueryText : function () {
      var queryComponents = [],
          origQuery = this._search.get('displayedValue').trim(),
          finalQuery = "";

      if (!this._canUseLuceneSearchSyntax())
        return origQuery;

      origQuery = origQuery.replace(/:/g, '\\:');

      if (this._searchType.get('value') === SearchOptions.SearchInDefs){
        finalQuery = 'defs:(' + origQuery + ')';

        if (!this._typeCheckboxes.all.get('checked')) {
          finalQuery += ' AND ';
          for (var field in this._typeCheckboxes) {
            if (this._typeCheckboxes[field].get('checked') && field !== 'all') {
              queryComponents.push(field + ':(' + origQuery + ')');
            }
          }

          // Assemble query string. I assume there is at least one checked
          // checkbox.
          finalQuery += '( ' + queryComponents[0];
          for (var i = 1; i < queryComponents.length; ++i) {
            finalQuery += ' OR ' + queryComponents[i];
          }
          finalQuery += ')';
        }
      } else {
        finalQuery = origQuery;
      }

      if (!this._langRadios.all.get('checked')) {
        queryComponents = [];
        for (var lang in this._langMap) {
          if (this._langRadios[lang].get('checked')) {
            this._langMap[lang].mimeTypes.forEach(function (mime) {
              queryComponents.push('mime:("' + mime + '")');
            });
          }
        }

        if (this._langRadios['mime'].get('checked'))
          queryComponents.push(
            'mime:("' + this._mimeComboBox.get('value') + '")');

        if (finalQuery.length > 0){
          finalQuery += ' AND ';
        }
        finalQuery += '( ' + queryComponents[0];
        for (var i = 1; i < queryComponents.length; ++i) {
          finalQuery += ' OR ' + queryComponents[i];
        }
        finalQuery += ')';
      }

      return finalQuery;
    },
    
    /**
     * Updates the search history with the current ComboBox value, then sets
     * a new store for it.
     */
    _updateSearchHistory : function () {
      var userInput = this._search.get('value').trim(),
          historyIndex = -1;
      this._searchHistory.every(function (item, index) {
        if (item.text === userInput) {
          historyIndex = index;
          return false;
        }
        return true;
      });

      if (historyIndex === -1) {
        // New item!
        this._searchHistory.unshift({text : userInput });
      }

      while (this._searchHistory.length > 15) {
        // We have to delete some item
        this._searchHistory.pop();
      }

      var mem = new Memory();
      this._searchHistory.forEach(function (item) {
        mem.put({text: item.text});
      });

      this._search.set('store', mem);
    },

    /**
     * This helper function publishes a search message based on the values
     * of the search text boxes.
     */
    _publishSearch : function() {
      this._updateSearchHistory();

      topic.publish('codecompass/search', {
        fileFilter : this._fileFilter.get('value').trim(),
        dirFilter  : this._dirFilter.get('value').trim(),
        searchType : this._searchType.get('value'),
        text       : this._assembleQueryText()
      });
    },

    /**
     * Suggests a search text based on the current content of the search
     * ComboBox. The request is done via an async callback, so this function
     * returns quickly.
     */
    _suggestSearchText : function () {
      var search = this._search,
          userInput = search.get('displayedValue').trim();

      window.clearTimeout(search.suggestTimer);
      delete search.suggestTimer;

      if (userInput.length < 3 || userInput === search.suggestLastInput) {
        // Just a little optimalization: if we don't have enough user input
        // or the last time we requested suggestions to the same input.
        return;
      }

      var params = new  SearchSuggestionParams();
      params.options = this._searchType.get('value');
      params.userInput = search.suggestLastInput = userInput;
      params.tag = search.suggestTag = Date.now().toString();

      model.searchservice.suggest(params, function (result) {
        if (result.tag !== search.suggestTag) {
          // There is an another call in progress so we just drop this result.
          return;
        }
        
        var sstore = new Memory();
        result.results.forEach(function (item) {
          sstore.put({text: item});
        });

        search.set('store', sstore);
      });
    },

    _addTypeCheckbox : function (text, container, radios) {
      function isBlocked() {
        for (var field in radios)
          if (radios[field].blockAllButton)
            return true;
        return false;
      }

      var newDiv   = dom.create('div');
      var checkbox = new CheckBox({
        checked  : true,
        style    : 'margin-right: 5px; margin-bottom: 3px;',
        disabled : true,
        onChange : function (isOn) {
          var that = this;
          var keys = Object.keys(radios);

          if (text === 'All') {
            if (isBlocked())
              return;

            for (var field in radios)
              radios[field].set('checked', isOn);

            if (!isOn)
              radios[keys[0]].set('checked', true);
          } else {
            var num = 0;
            for (field in radios)
              if (radios[field].get('checked') && field !== 'all')
                ++num;

            this.blockAllButton = true;

            if (num === 0)
              this.set('checked', true);

            radios.all.set('checked', num === keys.length - 1);

            setTimeout(function () { that.blockAllButton = false; }, 500);
          }
        }
      });

      dom.place(newDiv, container.content);
      dom.place(checkbox.domNode, newDiv);
      dom.place(dom.toDom(text), newDiv);

      return checkbox;
    },

    /**
     * Create search settings drop down button.
     */
    _createSettingsButton : function (){
      var that = this;

      var advancedTooltip = new TooltipDialog();

      var optionsDiv = dom.create('div');
      advancedTooltip.set('content', optionsDiv);
      
      var langGroup = new Fieldset({
        title   : 'Languages',
        content : dom.create('div'),
        style   : 'float: left; margin-right: 10px'
      });

      for (var lang in this._langMap) {
        this._langRadios[lang]
          = this._addLangRadio(this._langMap[lang].name, langGroup);
      }
      this._langRadios.cpp.set('checked', true);

      dom.place(dom.toDom('<hr/>'), langGroup.content);

      this._langRadios.all = new CheckBox({
        style    : 'margin-right: 5px; margin-bottom: 3px;',
        checked  : true,
        onChange : function (isOn) {
          for (var field in that._langRadios)
            if (field !== 'all')
              that._langRadios[field].set('disabled', isOn);
        }
      });

      var anyDiv = new dom.create('div');
      dom.place(this._langRadios.all.domNode, anyDiv);
      dom.place(dom.toDom('Any'), anyDiv);
      dom.place(anyDiv, langGroup.content);

      dom.place(dom.toDom(
        '<div style="font-size: 8pt; color: gray; margin: 5px">In case of \
        <i>Any</i> search happens<br/>in every file, not only in listed ones. \
        </div>'),
        langGroup.domNode);

      dom.place(langGroup.domNode, optionsDiv);

      this._typeCheckboxes = {};
      var typeGroup = new Fieldset({
        title   : 'Types',
        content : dom.create('div'),
        style   : 'float: right'
      });

      this._typeCheckboxes['type']   = this._addTypeCheckbox('Type',      typeGroup, this._typeCheckboxes, true);
      this._typeCheckboxes['func']   = this._addTypeCheckbox('Function',  typeGroup, this._typeCheckboxes, true);
      this._typeCheckboxes['const']  = this._addTypeCheckbox('Constant',  typeGroup, this._typeCheckboxes, true);
      this._typeCheckboxes['var']    = this._addTypeCheckbox('Variable',  typeGroup, this._typeCheckboxes, true);
      this._typeCheckboxes['field']  = this._addTypeCheckbox('Field',     typeGroup, this._typeCheckboxes, true);
      this._typeCheckboxes['label']  = this._addTypeCheckbox('Label',     typeGroup, this._typeCheckboxes, true);
      this._typeCheckboxes['macro']  = this._addTypeCheckbox('Macro',     typeGroup, this._typeCheckboxes, true);
      this._typeCheckboxes['module'] = this._addTypeCheckbox('Module',    typeGroup, this._typeCheckboxes, true);

      dom.place(dom.toDom('<hr/>'), typeGroup.content);

      this._typeCheckboxes['all']    = this._addTypeCheckbox('All',       typeGroup, this._typeCheckboxes, true);

      dom.place(typeGroup.domNode, optionsDiv);

      dom.place(dom.create('div', { style : 'clear: both' }), optionsDiv);

      var advancedSettings = new DropDownButton({
        id       : 'cc-search-settings',
        label    : 'Settings',
        dropDown : advancedTooltip
      });
      
      return advancedSettings;
    },

    _addLangRadio: function (text, container) {
      var newDiv = dom.create('div');
      var radio = new RadioButton({
        name     : 'lang',
        style    : 'margin-right: 5px; margin-bottom: 3px;',
        disabled : true
      });

      dom.place(newDiv, container.content);
      dom.place(radio.domNode, newDiv);

      if (typeof text === 'string')
        dom.place(dom.toDom(text), newDiv);
      else
        dom.place(text.domNode, newDiv);

      return radio;
    },

    _createSearchTypes : function (advancedSettings){
      var that = this;

      var searchTypes = [];
      model.searchservice.getSearchTypes().forEach(function (item) {
        searchTypes.push({
          label : item.name,
          value : item.id
        });
      });

      return new Select({
        style    : 'width: 120px;',
        //dropDown : advancedTooltip,
        options  : searchTypes,
        onChange : function (newValue) {
          if (newValue === SearchOptions.SearchInSource ||
              newValue === SearchOptions.SearchInDefs) {
            that._langRadios.all.set('checked', false);
            that._langRadios.all.set('checked', true);
            that._typeCheckboxes.all.set('checked', false);
            that._typeCheckboxes.all.set('checked', true);

            for (var key in that._typeCheckboxes)
              that._typeCheckboxes[key].set(
                'disabled',
                newValue === SearchOptions.SearchInSource);

            that._search.set('placeHolder', that._placeholders.text);
          } else if (newValue === SearchOptions.SearchForFileName) {
            that._search.set('placeHolder', that._placeholders.file);
          } else if (newValue === SearchOptions.FindLogText) {
            that._search.set('placeHolder', that._placeholders.log);
          }
        
          if (newValue === SearchOptions.SearchInSource ||
              newValue === SearchOptions.SearchInDefs){
            advancedSettings.set('disabled', false);
          }else{
            advancedSettings.set('disabled', true);
          }
        }
      });
    },

    /**
     * Create main search field.
     */
    _createMainSearchField : function () {
      var that = this;

      return new ComboBox({
        id              : 'cc-search-filter',
        onInput         : function (event) {
          if (event.keyCode === keys.ENTER) {
            that._publishSearch();
          } else if (this.suggestTimer === undefined) {
            this.suggestTimer = window.setTimeout(function () {
              that._suggestSearchText();
            }, 300);
          }
          return true;
        },
        store           : new Memory({ data : that._searchHistory }),
        searchAttr      : 'text',
        tooltipPosition : ['below'],
        autoComplete    : true,
        ignoreCase      : true,
        placeHolder     : that._placeholders.text,
        tooltip         : that._placeholders.text
      });
    },

    /**
     * Create directory filter.
     */
    _createDirectoryFilter : function () {
      var that = this;

      return new IconTextBox({
        id          : 'cc-search-dirfilter',
        label       : 'Directory Filter:',
        icon        : 'icon icon-search',
        placeHolder : this._placeholders.dirFilter,
        onSubmit    : function () {
          popup.close(that._dirFilterDialogTooltip);
          that._publishSearch();
        }
      });
    },

    /**
     * Set directory filter events.
     */
    _setDirectoryFilterEvents : function (dirFilter) {
      var that = this;

      dirFilter.on('mouseover', function () {
        if (that._dirFilterDialogTooltip.get('content'))
          that._dirFilterDialogTooltipTimeout = setTimeout(function () {
            popup.open({
              popup  : that._dirFilterDialogTooltip,
              around : that._dirFilter.domNode
            });
          }, 200);
      });

      dirFilter.on('mouseout', function () {
        clearTimeout(that._dirFilterDialogTooltipTimeout);
      });
    },

    /**
     * Create file filter.
     */
    _createFileFilter : function () {
      var that = this;

      return new IconTextBox({
        id          : 'cc-search-filefilter',
        label       : 'File Filter:',
        icon        : 'icon icon-search',
        placeHolder : that._placeholders.fileFilter,
        onSubmit    : function () {
          popup.close(that._dirFilterDialogTooltip);
          that._publishSearch();
        }
      });
    },

    buildRendering : function () {
      var that = this;

      //--- Advanced search tooltip dialog ---//

      var advancedSettings = this._createSettingsButton();

      //--- Query search types ---//

      this._searchType = this._createSearchTypes(advancedSettings);

      //--- Search fields ---//

      this._search     = this._createMainSearchField();
      this._dirFilter  = this._createDirectoryFilter();
      this._fileFilter = this._createFileFilter();

      this._dirFilterDialogTooltip = new TooltipDialog({
        onMouseLeave : function () {
          popup.close(this);
        }
      });

      this._setDirectoryFilterEvents(this._dirFilter);

      //--- Create tooltips ---//

      var searchHelp     = dom.create('span', { class : 'cc-search-help' });
      var fileFilterHelp = dom.create('span', { class : 'cc-search-help' });
      var dirFilterHelp  = dom.create('span', { class : 'cc-search-help' });

      var searchTooltip     = new Tooltip({ 
        position: ['below'], 
        label : that._searchTooltipLabel
      });
      var fileFilterTooltip = new Tooltip({
        position: ['below'],
        label : that._fileFilterTooltipLabel
      });
      var dirFilterTooltip  = new Tooltip({
        position: ['below'],
        label : that._dirFilterTooltipLabel
      });

      searchTooltip.addTarget(searchHelp);
      fileFilterTooltip.addTarget(fileFilterHelp);
      dirFilterTooltip.addTarget(dirFilterHelp);

      //--- Search fields  ---//

      var searchFields = dom.create('div', { class : 'cc-search-fields' });

      var searchGroup1 = new dom.create('div', { class : 'cc-search-group'});
      dom.place(this._searchType.domNode, searchGroup1);

      var search = dom.create('div', { id : 'cc-search' });
      dom.place(advancedSettings.domNode, search);
      dom.place(this._search.domNode,     search);
      dom.place(searchHelp, search);
      dom.place(search, searchGroup1);

      dom.place(searchGroup1, searchFields);

      var searchGroup2 = new dom.create('div', { class : 'cc-search-group'});

      var fileFilterBox = dom.create('div', { id : 'cc-filefilter-box' });
      dom.place(this._fileFilter.domNode, fileFilterBox);
      dom.place(fileFilterHelp,  fileFilterBox);

      var dirFilterBox = dom.create('div', { id : 'cc-dirfilter-box' });
      dom.place(this._dirFilter.domNode,  dirFilterBox);
      dom.place(dirFilterHelp,  dirFilterBox);

      dom.place(fileFilterBox, searchGroup2);
      dom.place(dirFilterBox,  searchGroup2);

      dom.place(searchGroup2, searchFields);

      this.domNode = searchFields;
    }
  });

  //--- Search ---//

  var searchFields = new SearchFields({
    id : 'searchfields'
  });

  viewHandler.registerModule(searchFields, {
    type : viewHandler.moduleType.Header,
    priority : 20
  });
});
