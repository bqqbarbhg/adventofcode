#!/usr/bin/env bash

ticker=$(awk '{printf "%s|", $0}' < $1)
sed -En "s/^Sue ([0-9]*): ($ticker, )*\$/\\1/p"

