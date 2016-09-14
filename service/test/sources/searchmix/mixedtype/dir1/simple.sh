#!/bin/bash

function toCelcius {
  bc <<< "scale=2;(5/9)*(${1}-32)";
}

function sayHello {
  echo "Hello!";
}
