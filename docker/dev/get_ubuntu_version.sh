#!/usr/bin/env bash
grep VERSION_ID /etc/os-release | cut -d= -f2 | tr -d \"
