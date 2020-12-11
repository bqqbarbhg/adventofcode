const std = @import("std");
const ator = std.heap.page_allocator;

const Grid = struct {
    buffer: []u8,
    data: []u8,
    width: i32,
    height: i32,

    fn get(self: *const Grid, x: i32, y: i32) u8 {
        if (x >= 0 and y >= 0 and x < self.width and y < self.height) {
            return self.data[@intCast(usize, y * self.width + x)];
        } else {
            return '@';
        }
    }

    fn set(self: *Grid, x: i32, y: i32, val: u8) void {
        if (x >= 0 and y >= 0 and x < self.width and y < self.height) {
            self.data[@intCast(usize, y * self.width + x)] = val;
        }
    }

    fn equal(a: *const Grid, b: *const Grid) bool {
        if (a.width != b.width or a.height != b.height) return false;
        const size = @intCast(usize, a.width * a.height);
        return std.mem.eql(u8, a.data[0..size], b.data[0..size]);
    }

};

fn readGrid(reader: *std.fs.File.Reader) !Grid {
    const buffer = try reader.readAllAlloc(ator, std.math.maxInt(i32));

    var width: i32 = 0;
    var height: i32 = 1;
    var lo: u32 = 0;
    var hi: u32 = 0;
    while (hi < buffer.len) {
        const c = buffer[hi];
        if (c == '\r' or c == '\n') {
            if (width == 0) {
                width = @intCast(i32, hi);
            }
            if (c == '\n') {
                height += 1;
            }
        } else {
            buffer[lo] = buffer[hi];
            lo += 1;
        }
        hi += 1;
    }

    const size = @intCast(usize, width * height);
    return Grid{
        .buffer = buffer,
        .data = buffer[0..size],
        .width = width,
        .height = height,
    };
}

fn allocateGrid(width: i32, height: i32) !Grid {
    const buffer = try ator.alloc(u8, @intCast(usize, width * height));
    return Grid{
        .buffer = buffer,
        .data = buffer,
        .width = width,
        .height = height,
    };
}

fn hitscan(grid: *const Grid, x: i32, y: i32, dx: i32, dy: i32) u8 {
    var cx = x;
    var cy = y;
    while (true) {
        cx += dx;
        cy += dy;
        const nb = grid.get(cx, cy);
        if (nb != '.') return nb;
    }
}

fn countNeighbors(grid: *const Grid, x: i32, y: i32, kind: u8) i32 {
    const neighbors = .{
        .{ -1, -1 }, .{ 0, -1 }, .{ 1, -1 },
        .{ -1,  0 },             .{ 1,  0 },
        .{ -1,  1 }, .{ 0,  1 }, .{ 1 , 1 },
    };

    var num: i32 = 0;
    inline for (neighbors) |dnb| {
        if (hitscan(grid, x, y, dnb[0], dnb[1]) == '#') num += 1;
    }
    return num;
}

fn simulate(dst: *Grid, src: *const Grid) void {
    var y: i32 = 0;
    while (y < dst.height) : (y += 1) {
        var x: i32 = 0;
        while (x < dst.width) : (x += 1) {
            const nbs = countNeighbors(src, x, y, '#');
            dst.set(x, y, switch (src.get(x, y)) {
                'L' => if (nbs == 0) @intCast(u8, '#') else 'L',
                '#' => if (nbs >= 5) @intCast(u8, 'L') else '#',
                else => |other| other,
            });
        }
    }
}

pub fn main() !void {
    const writer = &std.io.getStdOut().writer();
    const reader = &std.io.getStdIn().reader();

    var grids: [2] Grid = undefined;

    grids[0] = try readGrid(reader);
    defer ator.free(grids[0].buffer);

    grids[1] = try allocateGrid(grids[0].width, grids[0].height);
    defer ator.free(grids[1].buffer);

    var write: u1 = 0;
    while (!Grid.equal(&grids[0], &grids[1])) {
        write ^= 1;
        simulate(&grids[write], &grids[write ^ 1]);
    }

    const num = std.mem.count(u8, grids[write].data, ([_]u8 {'#'})[0..]);
    try writer.print("{}", .{num});
}

