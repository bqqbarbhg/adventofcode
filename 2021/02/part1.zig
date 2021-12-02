const std = @import("std");

const Op = enum {
    forward,
    up,
    down,
};

const Cmd = struct {
    op: Op,
    arg: i32,
};

const Point = struct {
    x: i32,
    y: i32,
};

pub fn parseOperation(name: []const u8) ?Op {
    inline for (@typeInfo(Op).Enum.fields) |field, i| {
        if (std.mem.eql(u8, name, field.name)) {
            return @intToEnum(Op, i);
        }
    }
    return null;
}

pub fn parseCommand(line: []const u8) !Cmd {
    const mid = std.mem.indexOfScalar(u8, line, ' ') orelse return error.BadSyntax;
    return Cmd{
        .op = parseOperation(line[0..mid]) orelse return error.UnknownOperation,
        .arg = try std.fmt.parseInt(i32, line[mid+1..], 10),
    };
}

pub fn main() !void {
    const stdin = std.io.getStdIn().reader();
    const stdout = std.io.getStdOut().writer();
    const stderr = std.io.getStdErr().writer();

    var pos = Point{ .x = 0, .y = 0 };

    var buf: [256]u8 = undefined;
    var lineno: u32 = 0;
    while (try stdin.readUntilDelimiterOrEof(buf[0..], '\n')) |rawLine| {
        lineno += 1;
        const line = std.mem.trim(u8, rawLine, std.ascii.spaces[0..]);

        const cmd = parseCommand(line) catch |err| {
            try stderr.print("Error on line {}: {s}\n", .{ lineno, @errorName(err) });
            continue;
        };

        const arg = cmd.arg;
        switch (cmd.op) {
            .up => pos.y -= arg,
            .down => pos.y += arg,
            .forward => pos.x += arg,
        }
    }

    const result = pos.x * pos.y;
    try stdout.print("{}\n", .{result});
}
