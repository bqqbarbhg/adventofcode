#!/usr/bin/env bash

ticker=$(\
	cat $1 | \
	awk 'm=/cats|trees/{ printf "%s (%d",$1,$2+1; for (i=$2+2; i<=10; i++) printf "|%d", i; printf ")\n"} !m' | \
	awk 'm=/pomeranians|goldfish/{ printf "%s (%d",$1,0; for (i=1; i<$2; i++) printf "|%d", i; printf ")\n"} !m' | \
	awk '{printf "%s|", $0}' \
)

sed -En "s/^Sue ([0-9]*): ($ticker, )*\$/\\1/p"

