require("io")

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

local H = #map
local W = #map[1]

local nbs = {
    { x=-1, y= 0 },
    { x= 1, y= 0 },
    { x= 0, y=-1 },
    { x= 0, y= 1 },
}

local totalSum = 0
for y = 1, H do
    for x = 1, W do
        local height = map[y][x]
        local nbMin = 10
        for _, nb in ipairs(nbs) do
            local nbx = x + nb.x
            local nby = y + nb.y
            if nbx >= 1 and nby >= 1 and nbx <= W and nby <= H then
                local nbHeight = map[nby][nbx]
                if nbHeight < nbMin then
                    nbMin = nbHeight
                end
            end
        end
        if height < nbMin then
            local risk = height + 1
            totalSum = totalSum + risk
        end
    end
end

print(totalSum)
