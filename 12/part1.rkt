#!/usr/bin/env racket
#lang racket

(require json)

(define (json-sum json)
  (cond
	[(list? json) (for/sum ([elem json]) (json-sum elem))]
	[(hash? json) (for/sum ([(key value) json]) (json-sum value))]
	[(number? json) json]
	[else 0]))

(let ([json (read-json)])
  (write (json-sum json)))

