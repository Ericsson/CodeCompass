define([
    'dojo/_base/declare',
    'dojo/cookie',
    'dojo/dom',
    'dojo/dom-construct',
    'dojo/dom-class',
    'dojo/io-query',
    'dojo/keys',
    'dojo/on',
    'dijit/form/Button',
    'dijit/form/TextBox',
    'dijit/layout/BorderContainer',
    'dijit/layout/ContentPane',
    'dojox/widget/Standby',
    'login/MessagePane'],
function (declare, cookie, dom, domConstruct, domClass, ioQuery, keys, on,
        Button, TextBox, BorderContainer, ContentPane, Standby, MessagePane) {

var LoginPane = declare(ContentPane, {
  _doLogin: function () {
    var that = this;

    var userName = this.txtUser.get('value');
    var pass = this.txtPass.get('value');

    // No username supplied.
    if (!userName || !userName.trim().length) {
      domClass.add(that._mbox.domNode, 'mbox-error');
      that._mbox.show("Failed to log in!", "No username supplied.");
      return;
    }

    this.set('isAlreadyLoggingIn', true);
    this._standBy.show();

    AuthService.loginUsernamePassword(userName, pass,
      function (sessionCookie) {
        domClass.add(that._mbox.domNode, 'mbox-success');
        that._mbox.show("Successfully logged in!", '');

        // Set the cookie in the browser.
        var cookieParts = sessionCookie.split('=');
        cookie(cookieParts[0], cookieParts[1], {path: '/'});

        window.location = window.location.origin + '/index.html';
      }).fail(function (jsReq, status, exc) {
      if (status === "parsererror") {
        that._standBy.hide();
        domClass.add(that._mbox.domNode, 'mbox-error');

        if (exc instanceof IncorrectCredentials) {
          that._mbox.show("Failed to log in!", "Incorrect credentials given!");
        } else if (exc instanceof InvalidAuthMethod) {
          that._mbox.show("Failed to log in!",
            "Username-password authentication not supported by the server!");
        } else {
          that._mbox.show("Failed to log in!", "Other kind of error: " + exc);
        }

        that.txtPass.set('value', '');
        that.txtPass.focus();
        that.set('isAlreadyLoggingIn', false);
      }
    });
  },

  constructor: function () {
    this._mbox = new MessagePane({
      class: 'mbox'
    });

    this.txtUser = new TextBox({
      class: 'form-input',
      name: 'username'
    });

    this.txtPass = new TextBox({
      class: 'form-input',
      name: 'password',
      type: 'password'
    });

    var that = this;
    this.btnSubmit = new Button({
      label: "Login",
      onClick: function () {
        if (!that.get('isAlreadyLoggingIn'))
          that._doLogin();
      }
    });
  },

  postCreate: function () {
    this.set('isAlreadyLoggingIn', false);

    this.addChild(this._mbox);
    this._mbox.hide();

    this._standBy = new Standby({
      color: '#ffffff',
      target: this.domNode,
      duration: 0
    });
    this.addChild(this._standBy);

    var isSessionValid = false;
    try {
      isSessionValid = AuthService.isCurrentSessionValid();
    } catch (ex) {
      console.error(ex);
    }

    if (isSessionValid) {
      domClass.add(this._mbox.domNode, 'mbox-success');
      this._mbox.show("No authentication required",
                      "You are already logged in, or the server does not" +
                      " require authentication.");
      window.location = window.location.origin + '/index.html';
    } else {
      var cntPrompt = domConstruct.create('div', {
        class: 'formElement'
      }, this.containerNode);
      domConstruct.create('span', {
        class: 'login-prompt',
        style: 'width: 100%',
        innerHTML: this.loginPrompt
      }, cntPrompt);

      // Render the login dialog's controls.

      var cntUser = domConstruct.create('div', {
        class: 'formElement'
      }, this.containerNode);
      domConstruct.create('label', {
        class: 'formLabel bold',
        innerHTML: "Username: ",
        for: 'username'
      }, cntUser);
      domConstruct.place(this.txtUser.domNode, cntUser);

      var cntPass = domConstruct.create('div', {
        class: 'formElement'
      }, this.containerNode);
      domConstruct.create('label', {
        class: 'formLabel bold',
        innerHTML: "Password: ",
        for: 'password'
      }, cntPass);
      domConstruct.place(this.txtPass.domNode, cntPass);

      var cntLogin = domConstruct.create('div', {
        class: 'formElement'
      }, this.containerNode);
      domConstruct.place(this.btnSubmit.domNode, cntLogin);

      var that = this;

      function keypressHandler(evt) {
        if (!that.get('isAlreadyLoggingIn') &&
          evt.keyCode === keys.ENTER) {
          that.btnSubmit.focus();
          that._doLogin();
        }
      }

      on(this.txtUser.domNode, 'keypress', keypressHandler);
      on(this.txtPass.domNode, 'keypress', keypressHandler);
      on(this.btnSubmit.domNode, 'keypress', keypressHandler);
    }
  }
});

return function () {

  //---------------------------- Global objects ----------------------------//

  AuthService = new AuthenticationServiceClient(
    new Thrift.Protocol(new Thrift.Transport("/AuthenticationService")));

  //----------------------------- Main layout ------------------------------//

  var layout = new BorderContainer({
    style : 'height: 100%;'
  }, domConstruct.create('div', null, window.document.body));

  //--- Create header ---//

  header = new ContentPane({
    id     : 'cc-mainheader',
    region : 'top'
  });

  var logo = domConstruct.create("div", {id : 'cc-headerlogo'});
  domConstruct.place(logo, header.domNode);

  layout.addChild(header);

  //--- Create center panel ---//

  center = new ContentPane({ region : 'center' });

  var loginPrompt = "Accessing this CodeCompass server requires authentication!";
  try {
    loginPrompt = AuthService.getAuthPrompt();
  } catch (ex) {
    console.warn("Couldn't get login prompt:", ex);
  }

  this.loginPane = new LoginPane({
    region: 'center',
    loginPrompt: loginPrompt
  });

  var that = this;
  var loginContainer = new ContentPane({
    region: 'center',
    postCreate: function () {
      var smallerContainer = domConstruct.create('form', {
        id: 'login-form'
      }, this.containerNode);

      domConstruct.place(that.loginPane.domNode, smallerContainer);
    }
  });

  center.addChild(loginContainer);

  layout.addChild(center);

  //--- Load layout ---//

  layout.startup();
};
});
