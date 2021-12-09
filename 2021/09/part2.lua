require("io")

local vec2_meta = { }
vec2_meta.__index = vec2_meta
function vec2(x, y)
    local v = { x=x, y=y }
    return setmetatable(v, vec2_meta)
end
vec2_meta.__add = function(a, b) return vec2(a.x+b.x, a.y+b.y) end
vec2_meta.__sub = function(a, b) return vec2(a.x-b.x, a.y-b.y) end

local Grid = { }

function Grid:new(size, value)
    value = value or 0
    local tab = { }
    for y = 1, size.y do
        tab[y] = { }
        for x = 1, size.x do
            tab[y][x] = value
        end
    end
    self.__index = self
    tab.size = size
    return setmetatable(tab, self)
end

function Grid:get(pos, def)
    if pos.x < 1 or pos.y < 1 or pos.x > self.size.x or pos.y > self.size.y then
        assert(def ~= nil)
        return def
    end
    return self[pos.y][pos.x]
end

function Grid:set(pos, value)
    assert(pos.x >= 1)
    assert(pos.x <= self.size.x)
    assert(pos.y >= 1)
    assert(pos.y <= self.size.y)
    self[pos.y][pos.x] = value
end

function readMap()
    local map = { }
    local y = 1
    for line in io.lines() do
        map[y] = { }
        local x = 1
        for height in line:gmatch("%d") do
            map[y][x] = tonumber(height)
            x = x + 1
        end
        y = y + 1
    end
    map.size = vec2(#map[1], #map)
    return setmetatable(map, Grid)
end

function iter2D(size)
    local x, y = 0, 1
    return function()
        x = x + 1
        if x > size.x then
            x = 1
            y = y + 1
            if y > size.y then
                return nil
            end
        end
        return vec2(x, y)
    end
end

local nbDirs = { vec2(-1, 0), vec2( 1, 0), vec2( 0,-1), vec2( 0, 1) }
local map = readMap()

local basins = { }
local basinMap = Grid:new(map.size)

function updateBasin(pos)
    local memo = basinMap:get(pos)
    if memo ~= 0 then return memo end
    local minHeight = map:get(pos)
    if minHeight >= 9 then return 0 end

    local minNb = nil
    for _, dir in ipairs(nbDirs) do
        local nb = pos + dir
        local nbHeight = map:get(nb, 10)
        if nbHeight < minHeight then
            minHeight = nbHeight
            minNb = nb
        end
    end

    local index = nil
    if minNb == nil then
        table.insert(basins, 1)
        index = #basins
    else
        index = updateBasin(minNb)
        basins[index] = basins[index] + 1
    end
    basinMap:set(pos, index)
    return index
end

for pos in iter2D(map.size) do
    updateBasin(pos)
end

table.sort(basins, function(a, b) return a > b end)
print(basins[1] * basins[2] * basins[3])
