define([
  'dijit/Dialog',
  'dojo/_base/array',
  'dojo/_base/lang',
  'dojo/date/locale',
  'dojo/dom-style',
  'dojo/data/ItemFileWriteStore',
  'dojox/grid/DataGrid',
  'codecompass/model'],
function (Dialog, array, lang, locale, style, ItemFileWriteStore, DataGrid,
  model) {
  return {
    /**
     * This function returns the file name of a path string i.e. the part after
     * the last '/' character.
     * @param {String} path Path
     */
    getFilename : function (path) {
      return path.substr(path.lastIndexOf('/') + 1);
    },

    /**
     * This function returns the file directory of a path string i.e. the part
     * before the last '/' character.
     * @param {String} path Path
     */
    getDirectory : function (path) {
      return path.substr(0, path.lastIndexOf('/'));
    },

    /**
     * This function returns the extenstion part of a file. The file can be
     * given with relative or absolute path. By file name we mean the part of
     * the file after the last '/' character or the given string if no '/'
     * character. The extension is the part of the file name after the last '.'
     * character. The extension is empty if there is no '.' character in the
     * file name.
     */
    getExtension : function (path) {
      var filename = path.slice(path.lastIndexOf('/') + 1);
      var pos = filename.lastIndexOf('.');
      return pos === -1 ? '' : filename.slice(pos + 1);
    },

    /**
     * This function returns a css class based on the given path or file name.
     * @param {String} path File path or file name
     */
    getIconClass : function (path) {
      var extension = this.getExtension(path);
      var fileContentClass = 'icon-empty-file icon-file-content';
      switch (extension) {
        case 'cc':
        case 'cpp':    return fileContentClass + ' icon-ext-cpp';
        case 'c':      return fileContentClass + ' icon-ext-c';
        case 'hpp':
        case 'h':      return fileContentClass + ' icon-ext-h';
        case 'cs':     return fileContentClass + ' icon-ext-cs';
        case 'o':      return fileContentClass + ' icon-ext-o';
        case 'css':    return fileContentClass + ' icon-ext-css';
        case 'html':   return fileContentClass + ' icon-ext-html';
        case 'java':   return fileContentClass + ' icon-ext-java';
        case 'class':  return fileContentClass + ' icon-ext-class';
        case 'js':     return fileContentClass + ' icon-ext-js';
        case 'json':   return fileContentClass + ' icon-ext-json';
        case 'py':     return fileContentClass + ' icon-ext-py';
        case 'sqlite': return fileContentClass + ' icon-ext-sqlite';
        case 'out':
        case 'exe':    return fileContentClass + ' icon-ext-exe';
        case 'git':    return fileContentClass + ' icon-ext-git';
      }
    },

    /**
     * This function creates a hexadecimal color from a string.
     */
    strToColor : function (str) {
      var hash = 0;
      for (var i = 0; i < str.length; i++)
         hash = str.charCodeAt(i) + ((hash << 5) - hash);

      var c = (hash & 0x00FFFFFF).toString(16).toUpperCase();

      return '#' + '00000'.substring(0, 6 - c.length) + c;
    },

    /**
     * Creates a human friendly relative time ago on the date.
     */
    timeAgo : function (date) {
      var delta = Math.round((+new Date - date) / 1000);

      var minute = 60,
          hour   = minute * 60,
          day    = hour * 24,
          week   = day * 7,
          month  = day * 30
          year   = day * 365;

      var fuzzy;

      if (delta < 30) {
        fuzzy = 'just then';
      } else if (delta < minute) {
        fuzzy = delta + ' seconds ago';
      } else if (delta < 2 * minute) {
        fuzzy = 'a minute ago'
      } else if (delta < hour) {
        fuzzy = Math.floor(delta / minute) + ' minutes ago';
      } else if (Math.floor(delta / hour) == 1) {
        fuzzy = '1 hour ago'
      } else if (delta < day) {
        fuzzy = Math.floor(delta / hour) + ' hours ago';
      } else if (delta < day * 2) {
        fuzzy = 'yesterday';
      } else if (delta < week) {
        fuzzy = Math.floor(delta / day) + ' days ago';
      } else if (delta < 2 * week) {
        fuzzy = '1 week ago';
      } else if (delta < month) {
        fuzzy = Math.floor(delta / week) + ' weeks ago';
      } else {
        fuzzy = 'on ' + locale.format(date, {
          datePattern: 'yyyy. MM. dd', selector: 'date'});
      }

      return fuzzy;
    },

    /**
     * This function returns the full height of a DOM element which means the
     * sum of its height, top and bottom margin, padding and border.
     */
    getFullHeight : function (element) {
      var computedStyle = style.getComputedStyle(element);

      var height = parseInt(computedStyle.height);
      var paddingTop = parseInt(computedStyle.paddingTop);
      var paddingBottom = parseInt(computedStyle.paddingBottom);
      var borderTop = parseInt(computedStyle.borderTopWidth);
      var borderBottom = parseInt(computedStyle.borderBottomWidth);
      var marginTop = parseInt(computedStyle.marginTop);
      var marginBottom = parseInt(computedStyle.marginBottom);

      return height + paddingTop + paddingBottom + borderTop + borderBottom +
        marginTop + marginBottom;
    },

    /**
     * This function returns the full width of a DOM element which means the
     * sum of its width, left and right margin, padding and border.
     */
    getFullWidth : function (element) {
      var width = parseFloat(style.get(element, 'width'));
      var paddingLeft = parseFloat(style.get(element, 'paddingLeft'));
      var paddingRight = parseFloat(style.get(element, 'paddingRight'));
      var borderLeft = parseFloat(style.get(element, 'borderLeft'));
      var borderRight = parseFloat(style.get(element, 'borderRight'));
      var marginLeft = parseFloat(style.get(element, 'marginLeft'));
      var marginRight = parseFloat(style.get(element, 'marginRight'));

      return width + paddingLeft + paddingRight + borderLeft + borderRight +
        marginLeft + marginRight;
    },

    /**
     * This function returns the index of the element in the array to which the
     * "f" predicate returns true. If no element is found then the function
     * returns -1.
     * @param {Array} array
     * @param {Function} f
     */
    findIf : function (array, f) {
      for (var i = 0, max = array.length; i < max; ++i)
        if (f(array[i]))
          return i;
      return -1;
    },

    /**
     * This function changes <, > and & characters respectively to &lt;, &gt and
     * &amp;.
     * @param {String} string Input string
     */
    escapeTags : function (string) {
      return string.replace(/&/g, '&amp;')
                   .replace(/</g, '&lt;')
                   .replace(/>/g, '&gt;');
    },

    /**
     *
     *  MD5 (Message-Digest Algorithm)
     *  http://www.webtoolkit.info/
     *
     **/
    MD5 : function (string) {
      function RotateLeft(lValue, iShiftBits) {
        return (lValue<<iShiftBits) | (lValue>>>(32-iShiftBits));
      }

      function AddUnsigned(lX,lY) {
        var lX4,lY4,lX8,lY8,lResult;
        lX8 = (lX & 0x80000000);
        lY8 = (lY & 0x80000000);
        lX4 = (lX & 0x40000000);
        lY4 = (lY & 0x40000000);
        lResult = (lX & 0x3FFFFFFF)+(lY & 0x3FFFFFFF);
        if (lX4 & lY4) {
          return (lResult ^ 0x80000000 ^ lX8 ^ lY8);
        }
        if (lX4 | lY4) {
          if (lResult & 0x40000000) {
            return (lResult ^ 0xC0000000 ^ lX8 ^ lY8);
          } else {
            return (lResult ^ 0x40000000 ^ lX8 ^ lY8);
          }
        } else {
          return (lResult ^ lX8 ^ lY8);
        }
      }

      function F(x,y,z) { return (x & y) | ((~x) & z); }
      function G(x,y,z) { return (x & z) | (y & (~z)); }
      function H(x,y,z) { return (x ^ y ^ z); }
      function I(x,y,z) { return (y ^ (x | (~z))); }

      function FF(a,b,c,d,x,s,ac) {
        a = AddUnsigned(a, AddUnsigned(AddUnsigned(F(b, c, d), x), ac));
        return AddUnsigned(RotateLeft(a, s), b);
      };

      function GG(a,b,c,d,x,s,ac) {
        a = AddUnsigned(a, AddUnsigned(AddUnsigned(G(b, c, d), x), ac));
        return AddUnsigned(RotateLeft(a, s), b);
      };

      function HH(a,b,c,d,x,s,ac) {
        a = AddUnsigned(a, AddUnsigned(AddUnsigned(H(b, c, d), x), ac));
        return AddUnsigned(RotateLeft(a, s), b);
      };

      function II(a,b,c,d,x,s,ac) {
        a = AddUnsigned(a, AddUnsigned(AddUnsigned(I(b, c, d), x), ac));
        return AddUnsigned(RotateLeft(a, s), b);
      };

      function ConvertToWordArray(string) {
        var lWordCount;
        var lMessageLength = string.length;
        var lNumberOfWords_temp1=lMessageLength + 8;
        var lNumberOfWords_temp2=(lNumberOfWords_temp1-(lNumberOfWords_temp1 % 64))/64;
        var lNumberOfWords = (lNumberOfWords_temp2+1)*16;
        var lWordArray=Array(lNumberOfWords-1);
        var lBytePosition = 0;
        var lByteCount = 0;
        while ( lByteCount < lMessageLength ) {
          lWordCount = (lByteCount-(lByteCount % 4))/4;
          lBytePosition = (lByteCount % 4)*8;
          lWordArray[lWordCount] = (lWordArray[lWordCount] | (string.charCodeAt(lByteCount)<<lBytePosition));
          lByteCount++;
        }
        lWordCount = (lByteCount-(lByteCount % 4))/4;
        lBytePosition = (lByteCount % 4)*8;
        lWordArray[lWordCount] = lWordArray[lWordCount] | (0x80<<lBytePosition);
        lWordArray[lNumberOfWords-2] = lMessageLength<<3;
        lWordArray[lNumberOfWords-1] = lMessageLength>>>29;
        return lWordArray;
      };

      function WordToHex(lValue) {
        var WordToHexValue="",WordToHexValue_temp="",lByte,lCount;
        for (lCount = 0;lCount<=3;lCount++) {
          lByte = (lValue>>>(lCount*8)) & 255;
          WordToHexValue_temp = "0" + lByte.toString(16);
          WordToHexValue = WordToHexValue + WordToHexValue_temp.substr(WordToHexValue_temp.length-2,2);
        }
        return WordToHexValue;
      };

      function Utf8Encode(string) {
        string = string.replace(/\r\n/g,"\n");
        var utftext = "";

        for (var n = 0; n < string.length; n++) {

          var c = string.charCodeAt(n);

          if (c < 128) {
            utftext += String.fromCharCode(c);
          }
          else if((c > 127) && (c < 2048)) {
            utftext += String.fromCharCode((c >> 6) | 192);
            utftext += String.fromCharCode((c & 63) | 128);
          }
          else {
            utftext += String.fromCharCode((c >> 12) | 224);
            utftext += String.fromCharCode(((c >> 6) & 63) | 128);
            utftext += String.fromCharCode((c & 63) | 128);
          }

        }

        return utftext;
      };

      var x=Array();
      var k,AA,BB,CC,DD,a,b,c,d;
      var S11=7, S12=12, S13=17, S14=22;
      var S21=5, S22=9 , S23=14, S24=20;
      var S31=4, S32=11, S33=16, S34=23;
      var S41=6, S42=10, S43=15, S44=21;

      string = Utf8Encode(string);

      x = ConvertToWordArray(string);

      a = 0x67452301; b = 0xEFCDAB89; c = 0x98BADCFE; d = 0x10325476;

      for (k=0;k<x.length;k+=16) {
        AA=a; BB=b; CC=c; DD=d;
        a=FF(a,b,c,d,x[k+0], S11,0xD76AA478);
        d=FF(d,a,b,c,x[k+1], S12,0xE8C7B756);
        c=FF(c,d,a,b,x[k+2], S13,0x242070DB);
        b=FF(b,c,d,a,x[k+3], S14,0xC1BDCEEE);
        a=FF(a,b,c,d,x[k+4], S11,0xF57C0FAF);
        d=FF(d,a,b,c,x[k+5], S12,0x4787C62A);
        c=FF(c,d,a,b,x[k+6], S13,0xA8304613);
        b=FF(b,c,d,a,x[k+7], S14,0xFD469501);
        a=FF(a,b,c,d,x[k+8], S11,0x698098D8);
        d=FF(d,a,b,c,x[k+9], S12,0x8B44F7AF);
        c=FF(c,d,a,b,x[k+10],S13,0xFFFF5BB1);
        b=FF(b,c,d,a,x[k+11],S14,0x895CD7BE);
        a=FF(a,b,c,d,x[k+12],S11,0x6B901122);
        d=FF(d,a,b,c,x[k+13],S12,0xFD987193);
        c=FF(c,d,a,b,x[k+14],S13,0xA679438E);
        b=FF(b,c,d,a,x[k+15],S14,0x49B40821);
        a=GG(a,b,c,d,x[k+1], S21,0xF61E2562);
        d=GG(d,a,b,c,x[k+6], S22,0xC040B340);
        c=GG(c,d,a,b,x[k+11],S23,0x265E5A51);
        b=GG(b,c,d,a,x[k+0], S24,0xE9B6C7AA);
        a=GG(a,b,c,d,x[k+5], S21,0xD62F105D);
        d=GG(d,a,b,c,x[k+10],S22,0x2441453);
        c=GG(c,d,a,b,x[k+15],S23,0xD8A1E681);
        b=GG(b,c,d,a,x[k+4], S24,0xE7D3FBC8);
        a=GG(a,b,c,d,x[k+9], S21,0x21E1CDE6);
        d=GG(d,a,b,c,x[k+14],S22,0xC33707D6);
        c=GG(c,d,a,b,x[k+3], S23,0xF4D50D87);
        b=GG(b,c,d,a,x[k+8], S24,0x455A14ED);
        a=GG(a,b,c,d,x[k+13],S21,0xA9E3E905);
        d=GG(d,a,b,c,x[k+2], S22,0xFCEFA3F8);
        c=GG(c,d,a,b,x[k+7], S23,0x676F02D9);
        b=GG(b,c,d,a,x[k+12],S24,0x8D2A4C8A);
        a=HH(a,b,c,d,x[k+5], S31,0xFFFA3942);
        d=HH(d,a,b,c,x[k+8], S32,0x8771F681);
        c=HH(c,d,a,b,x[k+11],S33,0x6D9D6122);
        b=HH(b,c,d,a,x[k+14],S34,0xFDE5380C);
        a=HH(a,b,c,d,x[k+1], S31,0xA4BEEA44);
        d=HH(d,a,b,c,x[k+4], S32,0x4BDECFA9);
        c=HH(c,d,a,b,x[k+7], S33,0xF6BB4B60);
        b=HH(b,c,d,a,x[k+10],S34,0xBEBFBC70);
        a=HH(a,b,c,d,x[k+13],S31,0x289B7EC6);
        d=HH(d,a,b,c,x[k+0], S32,0xEAA127FA);
        c=HH(c,d,a,b,x[k+3], S33,0xD4EF3085);
        b=HH(b,c,d,a,x[k+6], S34,0x4881D05);
        a=HH(a,b,c,d,x[k+9], S31,0xD9D4D039);
        d=HH(d,a,b,c,x[k+12],S32,0xE6DB99E5);
        c=HH(c,d,a,b,x[k+15],S33,0x1FA27CF8);
        b=HH(b,c,d,a,x[k+2], S34,0xC4AC5665);
        a=II(a,b,c,d,x[k+0], S41,0xF4292244);
        d=II(d,a,b,c,x[k+7], S42,0x432AFF97);
        c=II(c,d,a,b,x[k+14],S43,0xAB9423A7);
        b=II(b,c,d,a,x[k+5], S44,0xFC93A039);
        a=II(a,b,c,d,x[k+12],S41,0x655B59C3);
        d=II(d,a,b,c,x[k+3], S42,0x8F0CCC92);
        c=II(c,d,a,b,x[k+10],S43,0xFFEFF47D);
        b=II(b,c,d,a,x[k+1], S44,0x85845DD1);
        a=II(a,b,c,d,x[k+8], S41,0x6FA87E4F);
        d=II(d,a,b,c,x[k+15],S42,0xFE2CE6E0);
        c=II(c,d,a,b,x[k+6], S43,0xA3014314);
        b=II(b,c,d,a,x[k+13],S44,0x4E0811A1);
        a=II(a,b,c,d,x[k+4], S41,0xF7537E82);
        d=II(d,a,b,c,x[k+11],S42,0xBD3AF235);
        c=II(c,d,a,b,x[k+2], S43,0x2AD7D2BB);
        b=II(b,c,d,a,x[k+9], S44,0xEB86D391);
        a=AddUnsigned(a,AA);
        b=AddUnsigned(b,BB);
        c=AddUnsigned(c,CC);
        d=AddUnsigned(d,DD);
      }

      var temp = WordToHex(a)+WordToHex(b)+WordToHex(c)+WordToHex(d);

      return temp.toLowerCase();
    }
  };
});
