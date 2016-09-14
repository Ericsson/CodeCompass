define([
  'dojo/_base/declare',
  'dojo/_base/lang',
  'dojo/dom-construct',
  'dijit/layout/ContentPane',
  'dijit/form/NumberSpinner',
  'dijit/form/ToggleButton',
  'dijit/TitlePane',
  'dojox/html/entities',
  'codecompass/registration/model'],
function (declare, lang, domConstruct, ContentPane, NumberSpinner, ToggleButton,
  TitlePane, entities, model) {

  /**
   * This function creates the lines of the diff table.
   * @param {Array} lines The lines of the whole diff file as an array of
   * strings.
   * @param {Number} i A line number where the changes begin, i.e. the line
   * number after a line which starts with '@'.
   * @param {Object} fileDiffDom A table DOM object in which the result is
   * added.
   * @param {Number} oldlinenumber The start line number of the change in the
   * original file. This can be read from the diff file, because this line
   * number is written in a line which begins with '@'.
   * @param {Number} newlinenumber The start line number of the change in the
   * modified file. This can be read from the diff file, because this line
   * number is written in a line which begins with '@'.
   * @param {Boolean} sideBySide This variable sets whether side-by-side view
   * should be generated or not.
   */
  function parseSingleDiffSection(
      lines, i, fileDiffDom, oldlinenumber, newlinenumber, sideBySide) {
      
      --oldlinenumber;
      --newlinenumber;

      for (; i < lines.length; ++i) {
        var line = lines[i];

        var linetextcssclass = "";
        var content = line;
        var mode = "nomode";
        
        if (0 === line.length)
          return i;
        
        switch (line[0]) {
          case "+":
            ++newlinenumber;
            linetextcssclass = "diff-linetext-new";
            content = content.substr(1);
            mode = "new";
            break;
          case "-":
            ++oldlinenumber;
            linetextcssclass = "diff-linetext-old";
            content = content.substr(1);
            mode = "old";
            break;
          case " ":
            ++newlinenumber;
            ++oldlinenumber;
            linetextcssclass = "diff-linetext-both";
            content = content.substr(1);
            mode = "both";
            break;
          case "\\":
            linetextcssclass = "diff-linetext-remark";
            content = "(" + content.substr(2) + ")";
            mode = "remark";
            break;
          case "@":
            return i;
          case "d":
            return i;
          default:
            console.log("Error at line " + i + " of " + lines.length + ": " + line);
            throw "Error in _parseSingleDiffSection";
        }
        
        content = entities.encode(content);
        
        var lineContainer = domConstruct.create("tr", {
          class: "diff-linecontainer"
        }, fileDiffDom);

        if (!sideBySide) {
          
          domConstruct.create("th", {
            class: "diff-linenumber diff-linenumber-old",
            innerHTML: ("old" == mode || "both" == mode ? oldlinenumber : "&nbsp;")
          }, lineContainer);
          domConstruct.create("th", {
            class: "diff-linenumber diff-linenumber-new",
            innerHTML: ("new" == mode || "both" == mode ? newlinenumber : "&nbsp;")
          }, lineContainer);
          domConstruct.create("td", {
            class: "diff-linetext " + linetextcssclass,
            innerHTML: content
          }, lineContainer);
          
        }
        else
        {

          domConstruct.create("th", {
            class: "diff-linenumber diff-linenumber-old",
            innerHTML: ("old" == mode || "both" == mode ? oldlinenumber : "&nbsp;")
          }, lineContainer);
          domConstruct.create("td", {
            class: "diff-linetext diff-linetext-sidebyside-side " + linetextcssclass,
            innerHTML: ("old" == mode || "both" == mode || "remark" == mode ? content : "&nbsp;")
          }, lineContainer);
          domConstruct.create("th", {
            class: "diff-linenumber diff-linenumber-new",
            innerHTML: ("new" == mode || "both" == mode ? newlinenumber : "&nbsp;")
          }, lineContainer);
          domConstruct.create("td", {
            class: "diff-linetext diff-linetext-sidebyside-side " + linetextcssclass,
            innerHTML: ("new" == mode || "both" == mode || "remark" == mode ? content : "&nbsp;")
          }, lineContainer);
          
        }
      }
      
      return i;
    }

    /**
     * This function creates a DOM table object which contains the diff view of
     * the file of which the changes start at line "i". In the diff file the
     * changes of a file start with a line of which the prefix is "diff --git".
     * The function builds only the header of such a table. The real content is
     * filled by parseSingleDiffSection() function.
     * @param {Array} lines The lines of the whole diff file as an array of
     * strings.
     * @param {Number} i Line number where the changes of the modified file
     * begins, i.e. the line which starts with "diff --git".
     * @param {Boolean} sideBySide This variable sets whether side-by-side view
     * should be generated or not.
     */
    function parseSingleDiffFile(lines, i, sideBySide) {
      var fileDiffDom = domConstruct.create('table', {
        class : "diff-containertable"
      });
      
      for (++i; i < lines.length && lines[i][0] !== '@'; ++i)
        if (lines[i].substr(0, 10) === "diff --git")
          return fileDiffDom;
      
      for (; i < lines.length; ++i) {
        var line = lines[i];

        if ('@' === line[0]) {
          var lineContainer = domConstruct.create("tr", {
            class : "diff-linecontainer"
          }, fileDiffDom);

          if (!sideBySide) {
          
            domConstruct.create("th", {
              class     : "diff-linenumber diff-linenumber-old",
              innerHTML : "..."
            }, lineContainer);
            domConstruct.create("th", {
              class     : "diff-linenumber diff-linenumber-new",
              innerHTML : "..."
            }, lineContainer);
            domConstruct.create("td", {
              class     : "diff-linetext diff-rowheader",
              innerHTML : line
            }, lineContainer);
            
          } else {
            
            domConstruct.create("th", {
              class     : "diff-linenumber diff-linenumber-old",
              innerHTML : "..."
            }, lineContainer);
            domConstruct.create("td", {
              colspan   : 3,
              class     : "diff-linetext diff-rowheader",
              innerHTML : line
            }, lineContainer);
            
          }

          var res = line.match(/[0-9]+/g);

          i = parseSingleDiffSection(
            lines,
            i + 1,
            fileDiffDom,
            parseInt(res[0]),
            parseInt(res[2]),
            sideBySide) - 1;
        } else if ("diff --git" === line.substr(0, 10)) {
          return fileDiffDom;
        } else if (0 === line.length) {
          return fileDiffDom;
        } else {
          console.log("Error at line " + i + ": " + line);
          throw "Error in _parseSingleDiffFile";
        }
      }
      
      return fileDiffDom;
    }

    /**
     * VersionDiffViewer handles the diff tables which are shown in toggle panes
     * per file.
     */
    var VersionDiffViewer = declare(ContentPane, {

      /**
       * This function reloads all toggle panes based on the diff file given as
       * parameter in simple string format. The content of such a toggle pane is
       * loaded lazyly when opening.
       */
      setContent : function (diff) {
        var that = this;
        
        this.destroyDescendants();
        
        var lines = diff.split(/\r?\n/);
        
        for (var i = 0; i < lines.length; ++i)
          if (lines[i].indexOf('diff --git') === 0) {
            
            //--- Gathering file names ---//
            
            var fileFromTo = lines[i].substr(11).split(' ');
            fileFromTo[0] = fileFromTo[0].substr(2);
            fileFromTo[1] = fileFromTo[1].substr(2);
            
            var fileNameText = fileFromTo[0];
            if (fileFromTo[0] !== fileFromTo[1])
              fileNameText += ' => ' + fileFromTo[1];
            
            //--- Calc outlineDom icon ---//
            
            var outlineDomIcon = 'modified';
            if (lines[i + 1].indexOf('new file') === 0)
              outlineDomIcon = 'new';
            else if (lines[i + 1].indexOf('deleted file') === 0)
              outlineDomIcon = 'deleted';
            
            var outlineDomIcon
              = '<div class="diff-outline-icon diff-outline-icon-'
              + outlineDomIcon
              + '"></div>';
            
            //--- Construct TitlePane ---//
            
            (function (i) { // This is needed because of closure affairs.
              var fnt = fileNameText;
              
              var titlePane = new TitlePane({
                title  : outlineDomIcon + fileNameText,
                open   : false,
                onShow : function () {
                  that._openPanes[fnt] = true;
                  if (!this.get('isLoaded')) {
                    
                    console.log(that._cache);
                    
                    //check cache for file diff
                    if (that._cache[fnt]) {
                      //load diff from cache
                      detailedDiff = that._cache[fnt];
                    } else {
                      //load diff from server
                      var vdo = lang.clone(that._vdo);
                      vdo.contextLines = that._contextLines.value;
                      vdo.pathspec = [ fnt ];
                      
                      var detailedDiff = model.versionservice.getCommitDiffAsString(
                        that._commitDetails.repoId,
                        that._commitDetails.oid,
                        vdo);
                      detailedDiff = detailedDiff.split(/\r?\n/);
                      
                      that._cache[fnt] = detailedDiff;
                    }

                    this.set('content',
                      parseSingleDiffFile(detailedDiff, 1, that.sideBySide));
                  }
                },
                onHide : function () {
                  that._openPanes[fnt] = false;
                }
              });
              
              that.addChild(titlePane);
              
              if (that._openPanes[fnt])
                titlePane.set('open', true);
            })(i);
          }
      },
      
      sideBySide : false,
      _openPanes : {},

      /**
       * Caches diffs of individual files
       * 
       * Currently this is used to avoid reloading diffs from the server
       * when switching side-by-side view
       */
      _cache : {}
    });
  
  return declare(ContentPane, {
    constructor : function () {
      var that = this;
      
      this._diff = "";
      
      this._topGroup = new ContentPane({
        style : 'height: 25px; margin: 10px 0px 10px 0px'
      });

      this._contextLines = new NumberSpinner({
        intermediateChanges : true,
        constraints         : { min : 0, max : 10000, places : 0 },
        onChange            : function (val) { that.reload(); },
        value               : 3
      });

      this._sideBySide = new ToggleButton({
        label    : 'Side-by-side',
        onChange : function (val) {
          that.redraw(false);
          this.set('label', this.get('checked') ? 'One sided' : 'Side-by-side');
        }
      });

      this._vdiffActual = new VersionDiffViewer();
      
      this._commitDetails = {};
    },
    
    postCreate : function () {
      this._topGroup.addChild(this._contextLines);
      this._topGroup.addChild(this._sideBySide);

      this.addChild(this._topGroup);
      this.addChild(this._vdiffActual);
    },
    
    setContent : function(diff, commitDetails, vdo) {
      //store commit details
      this._commitDetails = commitDetails;
      this._diff = diff;
      this._vdo = vdo;
      this._vdiffActual._vdo = vdo;
      this._vdiffActual._openPanes = {};
      this.redraw(true);
    },
    
    /**
     * Reloads the content from the model and redraws.
     */
    reload : function () {
      
      var vdo = this._vdo;
      vdo.contextLines = this._contextLines.value;

      this._diff = model.versionservice.getCommitDiffAsStringCompact(
        this._commitDetails.repoId,
        this._commitDetails.oid,
        vdo);

      this.redraw(true);
    },
    
    /**
     * Redraws without reloading from the server.
     * 
     * @param resetCache bool if true, the cache is reset
     */
    redraw : function (resetCache) {
      if (resetCache) {
        this._vdiffActual._cache = {};
      }
      this._vdiffActual.set('sideBySide', this._sideBySide.checked);
      this._vdiffActual.setContent(this._diff);
      this._vdiffActual._contextLines = this._contextLines;
      this._vdiffActual._commitDetails = this._commitDetails;
    }
    
  });


});

