define([
  'dojo/_base/declare',
  'dojo/dom-class',
  'dojo/dom-construct',
  'dijit/TitlePane',
  'dijit/layout/ContentPane',
  'dijit/form/Button',
  'codecompass/model',
  'dojox/highlight',
  'dojox/highlight/languages/cpp',],
function (declare, domClass, dom, TitlePane, ContentPane, Button, model,
  highlight) {

  model.addService('gitservice', 'GitService', GitServiceClient);

  /**
   * Handles the diff tables which are shown in toggle panes per file.
   */
  var GitDiffViewer = declare(ContentPane, {
    /**
     * Controlling how to diff text is generated. It tells how many lines must
     * be shown below and above the diff text.
     */
    _contextLines : 3,

    /**
     * It controls the line steps on expand button click.
     */
    _contextLineStep : 5,

    /**
     * Cache title panes to easily reload content of these if we click on an
     * expand button.
     */
    _titlePanelCache : {},

    /**
     * Create an expand button. When the user clicks on it, it will increase the
     * context line size and reload the content of the actual file.
     * @param refNode The DOM node where the node should be placed.
     * If a string, it is assumed to be the ID of the node.
     * @param {String} path
     * @param {Boolean} sideBySide This variable sets whether side-by-side view
     * should be generated or not.
     */
    _createExpanderBtn : function (refNode, path, sideBySide) {
      var that = this;

      var expanderBox = dom.create('th', {
        colspan : sideBySide ? 1 : 2,
        class   : 'diff-expander'
      }, refNode);

      var expandBtn = new Button({
        class     : 'git-expander-btn',
        iconClass : 'icon icon-expand',
        showLabel : false,
        onClick   : function () {
          var titlePanel = that._titlePanelCache[path];
          titlePanel.contextLines += that._contextLineStep;
          titlePanel.set('content', 
            that.getFileDiff(path, titlePanel.contextLines));
        }
      }, dom.create('div', {}, expanderBox));
    },

    /**
     * This function creates a DOM table object which contains the diff view of
     * the file of which the changes start at line 'i'. In the diff file the
     * changes of a file start with a line of which the prefix is 'diff --git'.
     * The function builds only the header of such a table. The real content is
     * filled by parseSingleDiffSection() function.
     * @param {String} path
     * @param {Array} lines The lines of the whole diff file as an array of
     * strings.
     * @param {Number} i Line number where the changes of the modified file
     * begins, i.e. the line which starts with 'diff --git'.
     * @param {Boolean} sideBySide This variable sets whether side-by-side view
     * should be generated or not.
     */
    parseSingleDiffFile : function (path, lines, i, sideBySide) {
      var fileDiffDom = dom.create('table', { class : 'diff-containertable'});

      for (++i; i < lines.length && lines[i][0] !== '@'; ++i)
        if (lines[i].substr(0, 10) === 'diff --git')
          return fileDiffDom;

      for (; i < lines.length; ++i) {
        var line = lines[i];

        if ('@' === line[0]) {
          var lineContainer = dom.create('tr', {
            class : 'diff-linecontainer'
          }, fileDiffDom);

          //--- Create expander button in the diff header ---//

          this._createExpanderBtn(lineContainer, path, sideBySide);

          //--- Create line diff header ---//

          dom.create('td', {
            colspan   : sideBySide ? 3 : 1,
            class     : 'diff-linetext diff-rowheader',
            innerHTML : line
          }, lineContainer);

          var res = line.match(/[0-9]+/g);

          i = this.parseSingleDiffSection(lines, i + 1, fileDiffDom,
            parseInt(res[0]), parseInt(res[2]), sideBySide) - 1;
        } else if ('diff --git' === line.substr(0, 10) || 0 === line.length) {
          return fileDiffDom;
        } else {
          console.log('Error at line ' + i + ': ' + line);
          throw 'Error in _parseSingleDiffFile';
        }
      }

      return fileDiffDom;
    },

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
    parseSingleDiffSection : function (lines, i, fileDiffDom, oldlinenumber,
      newlinenumber, sideBySide) {

      --oldlinenumber;
      --newlinenumber;

      for (; i < lines.length; ++i) {
        var line = lines[i];

        var content = line;
        var mode = 'nomode';

        if (0 === line.length)
          return i;

        switch (line[0]) {
          case '+':
            ++newlinenumber;
            content = content.substr(1);
            mode = 'new';
            break;

          case '-':
            ++oldlinenumber;
            content = content.substr(1);
            mode = 'old';
            break;

          case ' ':
            ++newlinenumber;
            ++oldlinenumber;
            content = content.substr(1);
            mode = 'both';
            break;

          case '\\':
            content = '(' + content.substr(2) + ')';
            mode = 'remark';
            break;

          case '@':
          case 'd':
            return i;

          default:
            console.log(
              'Error at line ' + i + ' of ' + lines.length + ': ' + line);
            throw 'Error in _parseSingleDiffSection';
        }

        content = highlight.processString(content).result;

        var lineContainer = dom.create('tr', {
          class : 'diff-linecontainer'
        }, fileDiffDom);

        dom.create('td', {
          class : 'diff-linenumber ' +
            (mode === 'old' && oldlinenumber ? 'old' :
            !sideBySide && mode === 'new' && newlinenumber ? 'new' : ''),
          innerHTML : ('old' == mode || 'both' == mode
            ? oldlinenumber : '&nbsp;')
        }, lineContainer);

        if (sideBySide)
          dom.create('td', {
            class : 'diff-linetext diff-linetext-sidebyside-side ' + (
               mode === 'old' && oldlinenumber ? mode : ''),
            innerHTML : ('old' == mode || 'both' == mode || 'remark' == mode 
               ? content : '&nbsp;')
          }, lineContainer);

        dom.create('td', {
          class : 'diff-linenumber ' + (mode === 'new' && newlinenumber ? 'new' :
            !sideBySide && mode === 'old' && oldlinenumber ? 'old' : ''),
          innerHTML : ('new' == mode || 'both' == mode
            ? newlinenumber : '&nbsp;')
        }, lineContainer);

        if (sideBySide)
          dom.create('td', {
            class : 'diff-linetext diff-linetext-sidebyside-side ' + (
              mode === 'new' && newlinenumber ? mode : ''),
            innerHTML : ('new' == mode || 'both' == mode || 'remark' == mode
              ? content : '&nbsp;')
          }, lineContainer);
        else
          dom.create('td', {
            class: 'diff-linetext ' + mode,
            innerHTML: content
          }, lineContainer);
      }

      return i;
    },

    /**
     * Get changes between commit.
     * @param {String} path File path.
     * @param {Number} contextLines Context line size.
     */
    getFileDiff : function (path, contextLines) {
      var options = new GitDiffOptions();
      options.contextLines = contextLines;
      options.pathspec = [path];

      var diff = model.gitservice.getCommitDiffAsString(
        this._repoId, this._commitId, options).split(/\r?\n/);

      return this.parseSingleDiffFile(path, diff, 1, this._sideBySide);
    },

    /**
     * @param {Number} repoId Repository id.
     * @param {String} commitId Commit hash.
     */
    load : function (repoId, commitId, sideBySide) {
      this._titlePanelCache = {};

      dom.empty(this.domNode);

      var that = this;

      this._repoId = repoId;
      this._commitId = commitId;
      this._sideBySide = sideBySide;

      var options = new GitDiffOptions();
      options.contextLines = this._contextLines;

      var diff = model.gitservice.getCommitDiffAsString(
        repoId, commitId, options, true);

      var lines = diff.split(/\r?\n/);
      for (var i = 0; i < lines.length; ++i) {
        if (lines[i].indexOf('diff --git') === 0) {

          //--- Gathering file names ---//

          var fileFromTo = lines[i].substr(11).split(' ');
          fileFromTo[0] = fileFromTo[0].substr(2);
          fileFromTo[1] = fileFromTo[1].substr(2);

          var fileNameText = fileFromTo[0];
          if (fileFromTo[0] !== fileFromTo[1])
            fileNameText += ' => ' + fileFromTo[1];

          //--- Construct TitlePane ---//

          this._titlePanelCache[fileNameText] = new TitlePane({
            title        : fileNameText,
            open         : true,
            contextLines : this._contextLines,
            content      : this.getFileDiff(fileNameText, this._contextLines)
          });

          this.addChild(this._titlePanelCache[fileNameText]);
          
        }
      }
    }
  });

  return declare(ContentPane, {
    _sideBySide : false,

    constructor : function () {
      var that = this;

      this._header = new ContentPane();

      this._inlineBtn = new Button({
        label   : 'Inline',
        class   : 'inline-btn',
        onClick : function () {
          if (that._sideBySide) {
            that._sideBySide = false;
            that.changeSplitButtonType();
            that.loadDiffTable(that._repoId, that._commitId);
          }
        }
      });

      this._sideBySideBtn = new Button({
        label   : 'Side-By-Side',
        class   : 'side-by-side-btn',
        onClick : function () {
          if (!that._sideBySide) {
            that._sideBySide = true;
            that.changeSplitButtonType();
            that.loadDiffTable(that._repoId, that._commitId);
          }
        }
      });

      this._diffTable = new GitDiffViewer();
    },

    postCreate : function () {
      this.addChild(this._header);
      this._header.addChild(this._inlineBtn);
      this._header.addChild(this._sideBySideBtn);
      this.addChild(this._diffTable);

      this.changeSplitButtonType();
    },

    changeSplitButtonType : function () {
      if (this._sideBySide) {
        domClass.add(this._sideBySideBtn.domNode, 'selected');
        domClass.remove(this._inlineBtn.domNode, 'selected');
      } else {
        domClass.add(this._inlineBtn.domNode, 'selected');
        domClass.remove(this._sideBySideBtn.domNode, 'selected');
      }
    },

    loadDiffTable : function (repoId, commitId) {
      this._repoId = repoId;
      this._commitId = commitId;

      this._diffTable.load(repoId, commitId, this._sideBySide);
    }
  });
});
