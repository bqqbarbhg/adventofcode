#!/usr/bin/env ruby
print ARGF.count { |l| /(..).*\1/ =~ l && /(.).\1/ =~ l }
