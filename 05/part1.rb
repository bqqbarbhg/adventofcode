#!/usr/bin/env ruby
print ARGF.count { |l| /([aeiou].*){3}/ =~ l && /(.)\1/ =~ l && /ab|cd|pq|xy/ !~ l }
