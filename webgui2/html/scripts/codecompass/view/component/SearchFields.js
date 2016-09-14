define([
  'dojo/dom-construct',
  'dojo/_base/declare',
  'dojo/_base/array',
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
  'codecompass/view/component/IconTextBox',
  'codecompass/registration/model'],
function (dom, declare, array, topic, keys, Memory, _WidgetBase, RadioButton,
  CheckBox, ComboBox, DropDownButton, Fieldset, Tooltip, TooltipDialog, Select,
  popup, IconTextBox, model) {
  
  function buildDirFilter(searchFields, path) {
    var result = dom.create('span');
    var string = '';

    array.forEach(path.split('/').slice(1), function (file) {
      string += '/' + file;
      var s = string; // This is needed because of closure affairs.

      dom.place(dom.toDom('/'), result);
      dom.place(dom.create('span', {
        innerHTML : file,
        class     : 'pathelement',
        onclick   : function () { searchFields._dirFilter.setValue(s + '.*'); }
      }), result);
    });

    searchFields._dirFilterDialogTooltip.set('content', result);
  }
  
  return declare(_WidgetBase, {
    constructor : function () {
      var that = this;
      
      topic.subscribe('codecompass/fileClick', function (message) {
        buildDirFilter(that, message.path);
        
        if (message.display)
          that._dirFilter.setValue(message.display);
      });
      
      topic.subscribe('codecompass/openFile', function (message) {
        buildDirFilter(that, model.project.getFileInfo(message.fileId).path);
      });
      
      topic.subscribe('codecompass/searchError', function (message) {
        var exception = message.exception;
        
        if (exception)
          that._search.set('invalidMessage', exception.message || exception.what);
        
        that._search.validator = function (value) {
          return !exception;
        };
        
        that._search.validate();
      });
    },
    
    buildRendering : function () {
      var that = this;
      
      /**
       * @returns {Boolean} true if you can use the standard Lucene syntax.
       */
      function canUseLuceneSearchSyntax() {
        switch (that._searchType.get('value')) {
          case model.SearchOptions.SearchInSource:
          case model.SearchOptions.SearchInDefs:
            return true;
          default:
            return false;
        }
      } 
      
      /**
       * Text search query is established by a query string in a given format. This function
       * assembes this query string based on the check state of checkboxes in "typeCheckboxes" and
       * "langCheckboxes" array.
       */
      function assembleQueryText() {
        var queryComponents = [],
            origQuery = that._search.get('displayedValue').trim(),
            finalQuery = "";
        
        if (!canUseLuceneSearchSyntax())
          return origQuery;
        
        origQuery = origQuery.replace(/:/g, '\\:');
        
        if (that._searchType.get('value') === model.SearchOptions.SearchInDefs)
        {
          finalQuery = 'defs:(' + origQuery + ')';
          
          if (!typeCheckboxes.all.get('checked')) {
            finalQuery += ' AND ';
            for (var field in typeCheckboxes) {
              if (typeCheckboxes[field].get('checked') && field !== 'all') {
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
        }
        else
        {
          finalQuery = origQuery;
        }

        if (!langRadios.all.get('checked'))
        {
          queryComponents = [];
          for (var lang in langMap) {
            if (langRadios[lang].get('checked')) {
              langMap[lang].mimeTypes.forEach(function (mime) {
                queryComponents.push('mime:("' + mime + '")');
              });
            }
          }
          
          if (langRadios['mime'].get('checked'))
            queryComponents.push('mime:("' + mimeComboBox.get('value') + '")');

          if (finalQuery.length > 0)
          {
            finalQuery += ' AND ';
          }
          finalQuery += '( ' + queryComponents[0];
          for (var i = 1; i < queryComponents.length; ++i) {
            finalQuery += ' OR ' + queryComponents[i];
          }
          finalQuery += ')';
        }

        return finalQuery;
      }

      /**
       * Updates the search history with the current ComboBox value, then sets
       * a new store for it.
       *
       * @return nothig
       */
      function updateSearchHistory() {
        var userInput = that._search.get('value').trim(),
            historyIndex = -1;
        that._searchHistory.every(function (item, index) {
          if (item.text === userInput) {
            historyIndex = index;
            return false;
          }
          return true;
        });

        if (historyIndex === -1) {
          // New item!
          that._searchHistory.unshift({text : userInput });
        }

        while (that._searchHistory.length > 15) {
          // We have to delete some item
          that._searchHistory.pop();
        }

        var mem = new Memory();
        that._searchHistory.forEach(function (item) {
          mem.put({text: item.text});
        });

        that._search.set('store', mem);
      }
      
      /**
       * This helper function publishes a search message based on the values of the search text
       * boxes.
       */
      function publishSearch() {
        updateSearchHistory();
        
        topic.publish('codecompass/search', {
          fileFilter : that._fileFilter.get('value').trim(),
          dirFilter  : that._dirFilter.get('value').trim(),
          searchType : that._searchType.get('value'),
          text       : assembleQueryText()
        });
      }

      /**
       * Suggests a search text based on the current content of the search
       * ComboBox. The request is done via an async callback, so this function
       * returns quickly.
       *
       * @return nothing
       */
      function suggestSearchText() {
        var search = that._search,
            userInput = search.get('displayedValue').trim();

        window.clearTimeout(search.suggestTimer);
        delete search.suggestTimer;

        if (userInput.length < 3 || userInput === search.suggestLastInput) {
          // Just a little optimalization: if we don't have enough user input
          // or the last time we requested suggestions to the same input.
          return;
        }

        var params = new  model.SearchSuggestionParams();
        params.options = that._searchType.get('value');
        params.userInput = search.suggestLastInput = userInput;
        params.tag = search.suggestTag = Date.now().toString();
 
        model.searchservice.suggest(params, function (result) {
          if (result.tag !== search.suggestTag) {
            // There is an another call in progress so we just drop this result
            return;
          }
          
          var sstore = new Memory();
          result.results.forEach(function (item) {
            sstore.put({text: item});
          });

          search.set('store', sstore);
        });
      }

      //--- Advanced search tooltip dialog ---//
      
      var advancedTooltip = new TooltipDialog();
      
      var optionsDiv = dom.create('div');
      advancedTooltip.set('content', optionsDiv);
      
      function addTypeCheckbox(text, container, radios) {
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
      }
      
      function addLangRadio(text, container) {
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
      }
      
      //--- Create checkboxes ---//
      
      var mimeTypes = new Memory({
        data : [
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
        ]
      });
      
      var mimeComboBox = new ComboBox({ store : mimeTypes }); 
      
      var langMap = {
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
        },
        mime: {
          name: mimeComboBox,
          mimeTypes: []
        }
      };
      
      var langRadios = {};
      var langGroup = new Fieldset({
        title   : 'Languages',
        content : dom.create('div'),
        style   : 'float: left; margin-right: 10px'
      });
      
      for (var lang in langMap) {
        langRadios[lang] = addLangRadio(langMap[lang].name, langGroup);
      }
      langRadios.cpp.set('checked', true);
      
      dom.place(dom.toDom('<hr/>'), langGroup.content);
      
      langRadios.all = new CheckBox({
        style    : 'margin-right: 5px; margin-bottom: 3px;',
        checked  : true,
        onChange : function (isOn) {
          for (var field in langRadios)
            if (field !== 'all')
              langRadios[field].set('disabled', isOn);
        }
      });
      
      var anyDiv = new dom.create('div');
      dom.place(langRadios.all.domNode, anyDiv);
      dom.place(dom.toDom('Any'), anyDiv);
      dom.place(anyDiv, langGroup.content);
      
      dom.place(dom.toDom('<div style="font-size: 8pt; color: gray; margin: 5px">In case of\
        <i>Any</i> search happens<br/>in every file, not only in listed ones.</div>'),
        langGroup.domNode);
      
      dom.place(langGroup.domNode, optionsDiv);
      
      var typeCheckboxes = {};
      var typeGroup = new Fieldset({
        title   : 'Types',
        content : dom.create('div'),
        style   : 'float: right'
      });
      
      typeCheckboxes.type     = addTypeCheckbox('Type',      typeGroup, typeCheckboxes, true);
      typeCheckboxes.func     = addTypeCheckbox('Function',  typeGroup, typeCheckboxes, true);
      typeCheckboxes['const'] = addTypeCheckbox('Constant',  typeGroup, typeCheckboxes, true);
      typeCheckboxes['var']   = addTypeCheckbox('Variable',  typeGroup, typeCheckboxes, true);
      typeCheckboxes.field    = addTypeCheckbox('Field',     typeGroup, typeCheckboxes, true);
      typeCheckboxes.label    = addTypeCheckbox('Label',     typeGroup, typeCheckboxes, true);
      typeCheckboxes.macro    = addTypeCheckbox('Macro',     typeGroup, typeCheckboxes, true);
      typeCheckboxes.module   = addTypeCheckbox('Module',    typeGroup, typeCheckboxes, true);
      dom.place(dom.toDom('<hr/>'), typeGroup.content);
      typeCheckboxes.all      = addTypeCheckbox('All',       typeGroup, typeCheckboxes, true);
      
      dom.place(typeGroup.domNode, optionsDiv);
      
      dom.place(dom.create('div', { style : 'clear: both' }), optionsDiv);
      
      var advancedSettings = new DropDownButton({
        label    : 'Settings',
        dropDown : advancedTooltip,
        style    : 'height: 20px'
      });

      //--- Query search types ---//
      
      var searchTypes = [];
      model.searchservice.getSearchTypes().forEach(function (item) {
        searchTypes.push({
          label : item.name,
          value : item.id
        });
      });

      //--- Create text boxes ---//

      var placeholders = {
        text       : 'Search expression, like "foo AND bar" (see question mark icon for help)',
        file       : 'File name regex (.*cpp$)',
        log        : 'Arbitrary log message (e.g. ERROR: foobar.cpp something went wrong at 12:34)',
        fileFilter : 'File name filter regex (.*cpp)',
        dirFilter  : 'Path filter regex (click on a dir below)'
      };
      
      this._searchType = new Select({
        style    : 'width: 130px',
        dropDown : advancedTooltip,
        options  : searchTypes,
        onChange : function (newValue) {
          if (newValue === model.SearchOptions.SearchInSource ||
              newValue === model.SearchOptions.SearchInDefs) {
            
            advancedSettings.set('style', 'display: inline-block');
            
            langRadios.all.set('checked', false);
            langRadios.all.set('checked', true);
            typeCheckboxes.all.set('checked', false);
            typeCheckboxes.all.set('checked', true);
            
            for (var key in typeCheckboxes)
              typeCheckboxes[key].set('disabled', newValue === model.SearchOptions.SearchInSource);
            
            that._search.set('placeHolder', placeholders.text);
          } else if (newValue === model.SearchOptions.SearchForFileName) {
            advancedSettings.set('style', 'display: none');
            that._search.set('placeHolder', placeholders.file);
          } else if (newValue === model.SearchOptions.FindLogText) {
            advancedSettings.set('style', 'display: none');
            that._search.set('placeHolder', placeholders.log);
          } else {
            advancedSettings.set('style', 'display: none');
          }
        }
      });

      this._dirFilterDialogTooltip = new TooltipDialog({
        onMouseLeave : function () {
          popup.close(this);
        }
      });

      this._searchHistory = [];
      this._search = new ComboBox({
        title           : 'Search:',
        onInput         : function (event) {
          if (event.keyCode === keys.ENTER) {
            publishSearch();
          } else if (this.suggestTimer === undefined) {
            this.suggestTimer = window.setTimeout(suggestSearchText, 300);
          }
          return true;
        },
        style           : 'width: 100%',
        store           : new Memory({data: that._searchHistory }),
        searchAttr      : 'text',
        tooltipPosition : ['below'],
        autoComplete    : true,
        ignoreCase      : true,
        placeHolder     : placeholders.text
      });

      this._fileFilter = new IconTextBox({
        label       : 'File Filter:',
        icon        : '/images/search-icon.png',
        style       : 'width: 100%',
        placeHolder : placeholders.fileFilter,
        onSubmit    : publishSearch
      });

      this._dirFilter = new IconTextBox({
        label       : 'Directory Filter:',
        icon        : '/images/search-icon.png',
        style       : 'width: 100%',
        placeHolder : placeholders.dirFilter,
        onSubmit    : function () {
          popup.close(that._dirFilterDialogTooltip);
          publishSearch();
        }
      });
      
      this._dirFilter.on('mouseover', function () {
        if (that._dirFilterDialogTooltip.get('content'))
          that._dirFilterDialogTooltipTimeout = setTimeout(function () {
            popup.open({
              popup  : that._dirFilterDialogTooltip,
              around : that._dirFilter.domNode
            });
          }, 200);
      });
      
      this._dirFilter.on('mouseout', function () {
        clearTimeout(that._dirFilterDialogTooltipTimeout);
      });
      
      //--- Tooltip texts ---//

      var fileFilterTooltipLabel =
        'This filter is a regular expression of file names. <br> \
        Example: .*cpp';
      var dirFilterTooltipLabel  =
        'This filter is a regular expression of directory paths. <br> \
        Example: /path/to/source/subdir/.*';
      var searchTooltipLabel     =
        'For <b>"Text search"</b> and <b>"Definition search"</b> you can use wildcards, \
        regex, and logical operators: \
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
            We support AND, "+", OR, NOT and "-" as Boolean operators \
            (<b>Note</b>: Boolean operators must be ALL CAPS) <br> \
            <ul> \
              <li><b>OR:</b> The OR operator links two terms and finds a \
                matching document if either of the terms exist in a document.<br> \
                <b>If there is no Boolean operator between two terms, the OR \
                operator is used</b><br> \
                Example: "some text" OR other \
              </li> \
              <li><b>AND:</b> The AND operator matches documents where both \
                terms exist anywhere in the text of a single document<br> \
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
        In <b>"File name search"</b> you can use a regular expression for matching \
        on full (absolute) path of files in the database. <br> \
        Example: /path/.*/te?t\\.cpp <br><br> \
        For the documentation of <b>"Log search"</b> please see the user manual.';

      //--- Create DOM ---//

      var searchHelp     = dom.create('img', { src : '/images/question.png', width : 18 });
      var fileFilterHelp = dom.create('img', { src : '/images/question.png', width : 18 });
      var dirFilterHelp  = dom.create('img', { src : '/images/question.png', width : 18 });

      var searchTooltip     = new Tooltip({ position: ['below'], label : searchTooltipLabel     });
      var fileFilterTooltip = new Tooltip({ position: ['below'], label : fileFilterTooltipLabel });
      var dirFilterTooltip  = new Tooltip({ position: ['below'], label : dirFilterTooltipLabel  });
      
      searchTooltip.addTarget(searchHelp);
      fileFilterTooltip.addTarget(fileFilterHelp);
      dirFilterTooltip.addTarget(dirFilterHelp);

      //--- Build table ---//

      var table = dom.create('table', { class : 'searchtable' });
      
      var line1 = new dom.create('tr');
      dom.place(line1, table);
      
      var col11 = new dom.create('td', { innerHTML : 'Search:', class : 'searchlabel', style : 'width: 65px' });
      dom.place(col11, line1);
      
      var aligntable = dom.create('table', { style : 'width : 100%' });
      
      var aline = new dom.create('tr');
      dom.place(aline, aligntable);
      
      var acol1 = new dom.create('td', { style : 'width: 100px' });
      dom.place(acol1, aline);
      dom.place(this._searchType.domNode, acol1);
      
      var acol2 = new dom.create('td');
      dom.place(acol2, aline);
      dom.place(advancedSettings.domNode, acol2);
      
      var acol3 = new dom.create('td', { style : 'width: 100%' });
      dom.place(acol3, aline);
      dom.place(this._search.domNode, acol3);
      
      var col13 = new dom.create('td', { colspan : 4 });
      dom.place(col13, line1);
      dom.place(aligntable, col13);
      
      var col14 = new dom.create('td', { class : 'searchhelpcell' });
      dom.place(col14, line1);
      dom.place(searchHelp, col14);
      
      var line2 = new dom.create('tr');
      dom.place(line2, table);
      
      var col21 = new dom.create('td', { innerHTML : 'File Filter:', class : 'searchlabel' });
      dom.place(col21, line2);
      
      var col22 = new dom.create('td');
      dom.place(col22, line2);
      dom.place(this._fileFilter.domNode, col22);
      dom.place(fileFilterHelp, col22);
      
      var col23 = new dom.create('td', { class : 'searchhelpcell' });
      dom.place(col23, line2);
      dom.place(fileFilterHelp, col23);
      
      var col24 = new dom.create('td', { innerHTML : 'Directory Filter:', class : 'searchlabel', style : 'width: 90px' });
      dom.place(col24, line2);
      
      var col25 = new dom.create('td');
      dom.place(col25, line2);
      dom.place(this._dirFilter.domNode, col25);
      
      var col26 = new dom.create('td', { class : 'searchhelpcell' });
      dom.place(col26, line2);
      dom.place(dirFilterHelp, col26);
      
      this.domNode = table;
    }
  });
});
