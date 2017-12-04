
define i32 @checksum(i32* %data, i32 %count) {
Entry:
	br label %LoopStart

LoopStart:
	; Outer loop setup
	; if (ix == count) %LoopDone else %LoopBody
	%ix = phi i32 [0, %Entry], [%ix2, %LoopEnd]
	%loopcmp = icmp eq i32 %ix, %count
	br i1 %loopcmp, label %LoopDone, label %LoopBody
LoopBody:
	; Increment index and load value
	; val = data[ix]
	%ptr = getelementptr i32, i32* %data, i32 %ix
	%val = load i32, i32* %ptr
	br label %InnerStart

InnerStart:
	; Inner loop setup 
	; if (iy == count) %LoopEnd else %InnerBody
	%iy = phi i32 [0, %LoopBody], [%iy2, %InnerEnd]
	%innercmp = icmp eq i32 %iy, %count
	br i1 %innercmp, label %LoopEnd, label %InnerCheck
InnerCheck:
	; if (iy == ix) %InnerEnd else %InnerBody
	%checkcmp = icmp eq i32 %iy, %ix
	br i1 %checkcmp, label %InnerEnd, label %InnerBody
InnerBody:
	; Increment index and load inner value
	; ref = data[iy]
	%refptr = getelementptr i32, i32* %data, i32 %iy
	%ref = load i32, i32* %refptr

	; Check the remainder
	; if (val % ref == 0) %Found else %InnerEnd
	%rem = urem i32 %val, %ref
	%remcmp = icmp eq i32 %rem, 0
	br i1 %remcmp, label %Found, label %InnerEnd

InnerEnd:
	; Increment inner index
	; iy++
	%iy2 = add i32 %iy, 1
	br label %InnerStart

LoopEnd:
	; Increment outer index
	; ix++
	%ix2 = add i32 %ix, 1
	br label %LoopStart

Found:
	; Return division of values
	; return val / ref
	%div = udiv i32 %val, %ref
	ret i32 %div

LoopDone:
	; No divisible pairs found, return 0
	; return 0
	ret i32 0

}

