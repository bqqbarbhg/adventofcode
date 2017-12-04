
define i32 @checksum(i32* %data, i32 %count) {
Entry:
	br label %LoopStart
LoopStart:
	%ix = phi i32 [0, %Entry], [%ix2, %LoopBody]
	%min = phi i32 [4294967295, %Entry], [%min2, %LoopBody]
	%max = phi i32 [0, %Entry], [%max2, %LoopBody]

	%cmp = icmp eq i32 %ix, %count
	br i1 %cmp, label %LoopDone, label %LoopBody
LoopBody:

	%ix2 = add i32 %ix, 1
	%ptr = getelementptr i32, i32* %data, i32 %ix
	%val = load i32, i32* %ptr

	%mincmp = icmp ult i32 %val, %min
	%min2 = select i1 %mincmp, i32 %val, i32 %min
	%maxcmp = icmp ugt i32 %val, %max
	%max2 = select i1 %maxcmp, i32 %val, i32 %max

	br label %LoopStart
LoopDone:
	%result = sub i32 %max, %min
	ret i32 %result
}
