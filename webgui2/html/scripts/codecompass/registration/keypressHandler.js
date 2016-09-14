define([],
function () {
  var registrations = [];
  
  document.onkeydown = function (event) {
    registrations.forEach(function (p) {
      if (p.check(event)) {
        event.preventDefault();
        p.callback();
        return false;
      }
    });
  };
  
  /**
   * By this function one can registrate for key press events on the page
   * (e.g. ctrl-f).
   * @param {Function} check A predicate which gets the key press event as
   * parameter and returns true if the callback function shoud be invoked on
   * the event.
   * @param {Function} callback A callback function to run when the event
   * occurs.
   */
  return function (check, callback) {
    registrations.push({ check : check, callback : callback });
  };
});