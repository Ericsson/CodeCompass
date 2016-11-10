define([
  'dojo/_base/declare',
  'dojo/dom-construct',
  'dojo/topic',
  'dojo/dom-style',
  'dijit/MenuItem',
  'dijit/form/Button',
  'dijit/form/CheckBox',
  'dijit/form/Select',
  'dijit/form/DropDownButton',
  'dijit/layout/ContentPane',
  'codecompass/view/component/ContextMenu',
  'codecompass/viewHandler',
  'codecompass/urlHandler',
  'codecompass/model'],
function (declare, dom, topic, style, MenuItem, Button, CheckBox, Select,
  DropDownButton, ContentPane, ContextMenu, viewHandler, urlHandler, model) {

  model.addService('metricsservice', 'MetricsService', MetricsServiceClient);

  /**
   * This function reorganizes the structure of the JSON object which is the
   * input of the TreeMap. The original format is an object with exactly one
   * attribute. This is either a directory name to which a subobject belongs or
   * a file name which is a String or a Number describing a given metric to that
   * file.
   * The output format describes the same directory and file hierarchy. In this
   * format a node belonging to a directory has a "name" and a "children"
   * attribute. Name is a string, children is an array of subobjects. A node
   * belonging to a file has a "name" and a "value" attribute. The value is the
   * given metric of that file.
   */
  function preprocessInput(obj) {
    function reorganize(obj, name) {
      if (typeof(obj) === 'object') {
        var children = [];

        for (var child in obj) {
          children.push(reorganize(obj[child], child));
          delete obj[child];
        }

        return { name : name, children : children };
      } else {
        return { name : name, value : parseInt(obj) };
      }
    }

    /**
     * This function collapses the nodes in the directory hierarchy which have
     * only one child. Thus selecting a file which is reachable only on one path
     * can be done in less step.
     */
    function collapse(tree) {
      var newLabel = tree.name;
      var node = tree;

      while (node.children && node.children.length === 1) {
        newLabel += '/' + node.children[0].name;
        node = node.children[0];
      }

      if (node.children) {
        tree.children = node.children;
        tree.name = newLabel;

        for (var child in tree.children)
          collapse(child);
      }
    }

    obj = reorganize(obj, '');
    collapse(obj);
    return obj;
  }

  /**
   * This function returns the metric value from the JSON object based on a
   * given full path. This function takes collapsed directories into account
   * too.
   */
  function getValueByPath(obj, path) {
    function isPathPrefixOf(left, right) {
      left  = left.split('/');
      right = right.split('/');

      if (left[0]  === '') left.shift();
      if (right[0] === '') right.shift();

      if (left.length > right.length)
        return false;

      for (var i = 0; i < left.length; ++i)
        if (left[i] !== right[i])
          return false;

      return true;
    }

    function valueByPath(obj, path) {
      if (path[0] === '/')
        path = path.substr(1);

      if (obj.name && isPathPrefixOf(path, obj.name))
        return obj.value;

      for (var i = 0; i < obj.children.length; ++i) {
        var child = obj.children[i];

        if (isPathPrefixOf(child.name, path))
          return valueByPath(child, path.substr(child.name.length));
      }
    }

    return valueByPath({ children : [obj] }, '/' + path);
  }

  /**
   * This function returns the full path of an intermediate node in the JSON
   * tree.
   */
  function getPath(obj) {
    var path = obj.name;

    while (obj.parent) {
      obj = obj.parent;
      path = obj.name + '/' + path;
    }

    return path;
  }

  var Metrics = declare(ContentPane, {

    fileTypes : [
      {id: 'Unknown'    , name : 'Unknown'},
      {id: 'Dir'        , name : 'Directory'},
      {id: 'CPP'        , name : 'Cxx'},
      {id: 'Java'       , name : 'Java'},
      {id: 'Ruby'       , name : 'Ruby'},
      {id: 'Sql'        , name : 'Sql'},
      {id: 'Python'     , name : 'Python'},
      {id: 'ErlangBash' , name : 'ErlangBash'},
      {id: 'Perl'       , name : 'Perl'}
    ],

    _selectedFileTypes : {},
  
    style : 'padding: 10px',

    constructor : function() {
      var that = this;

      /**
       * This action loads the Metrics center module.
       * @param fileId : 'The id of the file or directory of which we want 
       * to display metrics for.',
       * @param fileTypes : 'Array of FileType values. If not given then it 
       * behaves as all values were listed. Only the files of these types will
       * be included in the metric.',
       * @param metricsTypes : 'Array of MetricsType values. The array has to be
       * of length 2 which specify the size and color dimensions respectively.'
       */

      topic.subscribe('codecompass/metrics', function (message) {
        var fileInfo = message.fileInfo;

        topic.publish('codecompass/setCenterModule', 'metrics');
        urlHandler.setStateValue({
          center : 'metrics'
        });

        that._currentFileId = fileInfo.id;

        var fileTypes = [fileInfo.type].concat(that.getSelectedFileTypes());
        var metricsTypes = message.metricsTypes || [
          that._metricsSelect1.get('value'),
          that._metricsSelect2.get('value')];

        that.loadMetrics(fileInfo.id, fileTypes, metricsTypes);
      });
    },

    postCreate : function () {
      var that = this;

      //--- Create dimension form table ---//

      var dimensionDiv = dom.create('div', {
        id    : 'dimensionPanel',
        class : 'bluePanel'
      });

      var dimensionTable = dom.create('table', { id : 'dimensionTable' });
      dom.place(dimensionTable, dimensionDiv);

      var dimensionRow1 = dom.create('tr');
      var dimensionRow2 = dom.create('tr');
      var dimensionRow3 = dom.create('tr');

      dom.place(dimensionRow1, dimensionTable);
      dom.place(dimensionRow2, dimensionTable);
      dom.place(dimensionRow3, dimensionTable);

      var dimensionCol11 = dom.create('td', { style : 'padding-right: 10px', rowspan : 2 });
      var dimensionCol12 = dom.create('td', { rowspan : 2 });
      var dimensionCol13 = dom.create('td', { style : 'padding-left: 10px' });
      var dimensionCol14 = dom.create('td', { style : 'padding-left: 10px' });
      var dimensionCol23 = dom.create('td', { style : 'padding-left: 10px' });
      var dimensionCol24 = dom.create('td', { style : 'padding-left: 10px' });
      var dimensionCol3  = dom.create('td', {
        colspan : 4,
        style   : 'text-align: center'
      });

      dom.place(dimensionCol11, dimensionRow1);
      dom.place(dimensionCol12, dimensionRow1);
      dom.place(dimensionCol13, dimensionRow1);
      dom.place(dimensionCol14, dimensionRow1);
      dom.place(dimensionCol23, dimensionRow2);
      dom.place(dimensionCol24, dimensionRow2);
      dom.place(dimensionCol3,  dimensionRow3);

      //--- Create dimension input fields ---//

      var fileTypeDiv = dom.create('div', { id : 'fileTypeDiv' });

      for (var fileType in that.fileTypes) {
        var fileTypeId = that.fileTypes[fileType].id;
        var cb = function (fileType) {
          that._selectedFileTypes[fileType] = fileTypeId;
          return new CheckBox({
            checked : true,
            onChange : function (newValue) {
              if (newValue)
                that._selectedFileTypes[fileType] = fileTypeId;
              else
                delete that._selectedFileTypes[fileType];
            }
          });
        }(fileType);

        var label = dom.toDom(that.fileTypes[fileType].name);

        var div = dom.create('div');
        dom.place(cb.domNode, div);
        dom.place(label, div);

        dom.place(div, fileTypeDiv);
      }

      var metricsTypeNames = model.metricsservice.getMetricsTypeNames().map(
      function (metricsTypeName) {
        return { label : metricsTypeName.name, value : metricsTypeName.type };
      });

      this._metricsSelect1 = new Select({ options : metricsTypeNames });
      this._metricsSelect2 = new Select({ options : metricsTypeNames });

      var metricsButton = new Button({
        label : 'Draw metrics',
        style : 'margin-top: 10px',
        onClick : function () {
          that.loadMetrics(
            that._currentFileId,
            that.getSelectedFileTypes(),
            [ that._metricsSelect1.get('value'),
              that._metricsSelect2.get('value')]);
        }
      });

      //--- Place input fields into the table ---//

      dom.place(dom.toDom('File type'), dimensionCol11);
      dom.place(fileTypeDiv, dimensionCol12);
      dom.place(dom.toDom('Size dimension'), dimensionCol14);
      dom.place(this._metricsSelect1.domNode, dimensionCol13);
      dom.place(dom.toDom('Color dimension'), dimensionCol24);
      dom.place(this._metricsSelect2.domNode, dimensionCol23);

      dom.place(metricsButton.domNode, dimensionCol3);

      //--- Add dimension table and metrics chart ---//

      dom.place(dimensionDiv, this.domNode);
      dom.place(dom.create('div', { id : 'metrics_chart' }), this.domNode);
    },

    setState : function (state) {
      if (!state.fid || state.center !== this.id)
        return;

      var fileInfo = model.project.getFileInfo(state.fid);
      topic.publish('codecompass/metrics', {'fileInfo' : fileInfo });
    },
    
    getSelectedFileTypes : function(){
      var that = this;
      return Object.keys(this._selectedFileTypes).map(function (x) {
        return that._selectedFileTypes[x];
      });
    },
    /**
     * This function loads the given metrics for the given file from the server
     * and redraws the TreeMap representation.
     * @param {FileId} fileId Id of the file which is the root of the TreeMap.
     * @param {Array} fileTypes Array of FileType items. The files under
     * fileId will be filtered on these file types.
     * @param {Array} metricsTypes Array of length 2 of MetricsType items.
     * the elements of the array specify the metrics in size and color
     * dimensions respectively.
     */
    loadMetrics : function (fileId, fileTypes, metricsTypes) {  
      var metricsChart = document.getElementById('metrics_chart');

      function removeContent() {
        while (metricsChart.firstChild)
          metricsChart.removeChild(metricsChart.firstChild);
      }

      //--- Remove previous metrics and play animgif ---//

      removeContent();
      dom.place(dom.create('span', { class : 'diagram-loading' }), metricsChart);

      //--- Get data from server ---//

      model.metricsservice.getMetrics(fileId, fileTypes, metricsTypes[0],
      function (root) {
      model.metricsservice.getMetrics(fileId, fileTypes, metricsTypes[1],
      function (colorValues) {
        root = JSON.parse(root);
        colorValues = JSON.parse(colorValues);
        
        //--- Create SVG and TreeMap ---//

        var margin = { top : 20, right : 0, bottom : 0, left : 0 };
        var width  = 960;
        var height = 500 - margin.top - margin.bottom;

        var formatNumber = d3.format(',d');

        var transitioning;

        var x = d3.scale.linear().domain([0, width]).range([0, width]);
        var y = d3.scale.linear().domain([0, height]).range([0, height]);

        var treemap = d3.layout.treemap()
          .children(function (d, depth) { return depth ? null : d._children; })
          .sort(function (a, b) { return a.value - b.value; })
          .ratio(height / width * 0.5 * (1 + Math.sqrt(5)))
          .round(false);

        removeContent();

        d3.select('svg').remove();
        var svg = d3.select('#metrics_chart')
          .append('svg')
          .attr('width', width + margin.left + margin.right)
          .attr('height', height + margin.bottom + margin.top)
          .style('margin-left', -margin.left + 'px')
          .style('margin.right', -margin.right + 'px')
          .append('g')
          .attr('transform', 'translate(' + margin.left + ',' + margin.top + ')')
          .style('shape-rendering', 'crispEdges');

        var grandparent = svg.append('g')
          .attr('class', 'grandparent');

        grandparent.append('rect')
          .attr('y', -margin.top)
          .attr('width', width)
          .attr('height', margin.top);

        grandparent.append('text')
          .attr('x', 6)
          .attr('y', 6 - margin.top)
          .attr('dy', '.75em');

        //--- Init input ---//

        root = preprocessInput(root);
        colorValues = preprocessInput(colorValues);

        initialize(root);
        accumulate(root);

        initialize(colorValues);
        accumulate(colorValues);

        var color = d3.scale.linear()
          .domain([0, colorValues.value])
          .range(['green', 'blue']);

        layout(root);
        display(root);

        //--- Helper functions ---//

        function initialize(root) {
          root.x = root.y = 0;
          root.dx = width;
          root.dy = height;
          root.depth = 0;
        }

        function accumulate(d) {
          return (d._children = d.children)
            ? d.value = d.children.reduce(function (p, v) {
                return p + accumulate(v); }, 0)
            : d.value;
        }

        function layout(d) {
          if (!d._children)
            return;

          treemap.nodes({ _children : d._children });
          d._children.forEach(function (c) {
            c.x = d.x + c.x * d.dx;
            c.y = d.y + c.y * d.dy;
            c.dx *= d.dx;
            c.dy *= d.dy;
            c.parent = d;
            layout(c);
          });
        }

        function display(d) {
          var max = 1;
          d._children.forEach(function (child) {
            var value = getValueByPath(colorValues, getPath(child));
            if (value > max) max = value;
          });

          color = d3.scale.linear()
            .domain([0, max])
            .range(['green', 'blue']);

          grandparent
            .datum(d.parent)
            .on('click', transition)
            .select('text')
            .text(getPath(d));

          var g1 = svg.insert('g', '.grandparent')
            .datum(d)
            .attr('class', 'depth');

          var g = g1.selectAll('g')
            .data(d._children)
            .enter()
            .append('g')
            .on('click', openFile);

          g.filter(function (d) { return d._children; })
            .classed('children', true)
            .on('click', transition);

          g.selectAll('.child')
            .data(function (d) { return d._children || [d]; })
            .enter().append('rect')
            .attr('class', 'child')
            .call(rect);

          g.append('rect')
            .attr('class', 'parent')
            .call(rect)
            .append('title')
            .text(function (d) {
              var colorDim = getValueByPath(colorValues, getPath(d));
              var sizeDim = d.value;

              return d.name + '\n'
               + 'Size dimension: '  + formatNumber(sizeDim) + '\n'
               + 'Color dimension: ' + formatNumber(colorDim || 0);
            });

          g.append('text')
            .attr('dy', '.75em')
            .text(function (d) { return d.name; })
            .call(text);

          function transition(d) {
            if (transitioning || !d) return;
            transitioning = true;

            var g2 = display(d),
                t1 = g1.transition().duration(750),
                t2 = g2.transition().duration(750);

            x.domain([d.x, d.x + d.dx]);
            y.domain([d.y, d.y + d.dy]);

            svg.style('shape-rendering', null);
            svg.selectAll('.depth').sort(
              function (a, b) { return a.depth - b.depth; });

            g2.selectAll('text').style('fill-opacity', 0);

            t1.selectAll('text').call(text).style('fill-opacity', 0);
            t2.selectAll('text').call(text).style('fill-opacity', 1);
            t1.selectAll('rect').call(rect);
            t2.selectAll('rect').call(rect);

            t1.remove().each('end', function () {
              svg.style('shape-rendering', 'crispEdges');
              transitioning = false;
            });
          }

          function openFile(d) {
            var fileInfo = model.project.getFileInfoByPath(getPath(d));
            topic.publish('codecompass/openFile', {
              fileId     : fileInfo.id,
              moduleId   : 'text',
              info       : 'Open file: ' + fileInfo.name,
              newSession : true
            });
          }

          return g;
        }

        function text(text) {
          text
            .attr('x', function (d) { return x(d.x) + 6; })
            .attr('y', function (d) { return y(d.y) + 6; });
        }

        function rect(rect) {
          rect
            .attr('x', function (d) { return x(d.x); })
            .attr('y', function (d) { return y(d.y); })
            .attr('width',  function (d) { return x(d.x + d.dx) - x(d.x); })
            .attr('height', function (d) { return y(d.y + d.dy) - y(d.y); })
            .style('fill', function (d) {
              return color(getValueByPath(colorValues, getPath(d)) || 0);
            })
            .style('cursor', 'pointer');
        }
      });
      });
      
//      var root = JSON.parse(model.metricsservice.getMetrics(
//        fileId, fileTypes, metricsTypes[0]));
//      var colorValues = JSON.parse(model.metricsservice.getMetrics(
//        fileId, fileTypes, metricsTypes[1]));
    }
  });
  
  var metrics = new Metrics({
    id : 'metrics'
  });
  viewHandler.registerModule(metrics, {
    type : viewHandler.moduleType.Center
  });

  //--- Register Menu Points ---//
  
  var metricsMenu = {
    id     : 'cppMetricsMenu',
    render : function(fileInfo){
      return new MenuItem({
        label    : 'Metrics',
        onClick  : function () {
          topic.publish('codecompass/metrics', {
            fileInfo : fileInfo
          })
        }
      });
    }
  };

  viewHandler.registerModule(metricsMenu, {
    type : viewHandler.moduleType.FileManagerContextMenu
  });

  return Metrics;
});
