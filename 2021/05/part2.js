
// WGSL shader source code
const kernel = `

struct Line {
    a: vec2<i32>;
    b: vec2<i32>;
};

[[block]] struct InputBuffer {
    lines: array<Line>;
};

[[block]] struct ArgBuffer {
    sizeX: u32;
    sizeY: u32;
    numLines: u32;
};

[[block]] struct TempBuffer {
    grid: array<atomic<u32>>;
};

[[block]] struct ResultBuffer {
    intersectionCount: atomic<u32>;
};

[[group(0), binding(0)]] var<uniform> args: ArgBuffer;
[[group(0), binding(1)]] var<storage, read> input: InputBuffer;
[[group(0), binding(2)]] var<storage, read_write> temp: TempBuffer;
[[group(0), binding(3)]] var<storage, read_write> result: ResultBuffer;

// These could be [[override]] but it's not supported yet..
let blockX: u32 = 8u;
let blockY: u32 = 8u;

[[stage(compute), workgroup_size(blockX, blockY)]]
fn main([[builtin(global_invocation_id)]] globalId : vec3<u32>) {
    let lineIndex = globalId.y;
    if (lineIndex >= args.numLines) { return; }

    // Each X-thread handles a sub-portion of a single line
    let subIndex = i32(globalId.x);
    let subCount = i32(blockX);

    // Load the line from the input buffer
    let line = input.lines[lineIndex];
    let dx = line.b.x - line.a.x;
    let dy = line.b.y - line.a.y;

    // The amount of steps depends on the Manhattan distance between the points
    let stepCount = max(abs(dx), abs(dy)) + 1;
    let stepX = clamp(dx, -1, 1);
    let stepY = clamp(dy, -1, 1);

    // Step through the line and evaluate each point
    var intersectionCount = 0u;
    for (var step: i32 = subIndex; step < stepCount; step = step + i32(blockX)) {
        let x = line.a.x + stepX * step;
        let y = line.a.y + stepY * step;

        // Atomically add one to each grid cell, we only count the first
        // intersections so increment 'intersectionCount' if there is only
        // a single line in the cell beforehand.
        let index = y * i32(args.sizeX) + x;
        let count = atomicAdd(&temp.grid[index], 1u);
        if (count == 1u) {
            intersectionCount = intersectionCount + 1u;
        }
    }

    // Accumulate the total intersection count if we hit any
    if (intersectionCount > 0u) {
        ignore(atomicAdd(&result.intersectionCount, intersectionCount));
    }
}
`

// Parse an input line into a 4-tuple [x0, y0, x1, y1]
// Input format: `x0,y0 -> x1,y1`
function parseLine(line) {
    const pattern = /\s*(\d+)\s*,\s*(\d+)\s*->\s*(\d+)\s*,\s*(\d+)\s*/
    const m = pattern.exec(line)
    if (!m) return null
    const int = s => parseInt(s, 10)
    return [int(m[1]), int(m[2]), int(m[3]), int(m[4])]
}

// Parse a list of input lines into 4-tuples [x0, y0, x1, y1]
function parseInput(text) {
    const lines = text
        .split("\n")
        .map(parseLine)
        .filter(a => a !== null)
    return lines
}

// Transfer data from CPU to GPU buffers.
// descs: [{ buffer: gpuBuffer, data: cpuArray, offset?: gpuOffset }...]
function uploadData(device, descs) {

    // Calculate the total required space for all transfers.
    const sum = arr => arr.reduce((a, b) => a + b, 0)
    const stagingSize = sum(descs.map(({ data }) => data.byteLength))

    // Create the GPU-side staging buffer and map it as writable.
    const staging = device.createBuffer({
        mappedAtCreation: true,
        size: stagingSize,
        usage: GPUBufferUsage.MAP_WRITE | GPUBufferUsage.COPY_SRC,
    })
    const stagingMapping = staging.getMappedRange()
    const dst = new Uint8Array(stagingMapping, 0, stagingSize)

    // Copy each descriptor data array to the staging buffer, simultaneously
    // record copy commands to the actual buffers to the encoder.
    let pos = 0
    const encoder = device.createCommandEncoder()
    for (const { data, buffer, offset = 0 } of descs) {
        const size = data.byteLength
        dst.set(new Uint8Array(data.buffer, data.byteOffset, size), pos)
        encoder.copyBufferToBuffer(staging, pos, buffer, offset, size)
        pos += size
    }

    // Unmap the buffer and submit the GPU copy commands.
    staging.unmap()
    const commands = encoder.finish()
    device.queue.submit([commands])

    // It is safe to destroy the buffer here as WebGPU tracks it in `submit()`.
    staging.destroy()
}

// Transfer data from a single GPU to the CPU.
async function downloadData(device, buffer, offset, size) {

    // Create the GPU-side staging buffer.
    const staging = device.createBuffer({
        size,
        usage: GPUBufferUsage.MAP_READ | GPUBufferUsage.COPY_DST,
    })

    // Copy the data to the buffer on the GPU.
    const encoder = device.createCommandEncoder()
    encoder.copyBufferToBuffer(buffer, offset, staging, 0, size)
    const commands = encoder.finish()
    device.queue.submit([commands])

    // Wait for the GPU to be done and map the buffer.
    await staging.mapAsync(GPUMapMode.READ)
    const stagingMapping = staging.getMappedRange()

    // Copy the data to the CPU and unmap/destroy the staging buffer.
    const copy = stagingMapping.slice(0)
    staging.unmap()
    staging.destroy()

    return copy
}

// Count the number of intersecting points between lines
// lines: Array of [x0, y0, x1, y1] tuples
async function countIntersections(lines) {
    const { STORAGE, UNIFORM, COPY_DST, COPY_SRC } = GPUBufferUsage

    // Setup the WebGPU device and adapter
    const adapter = await navigator.gpu.requestAdapter()
    const device = await adapter.requestDevice()

    // Encode the input lines into a flat 32-bit integer array
    const input = new Int32Array(lines.flat())

    // Determine the bounding box of all lines
    const sizeX = Math.max(...input.filter((_, ix) => ix % 2 == 0)) + 1
    const sizeY = Math.max(...input.filter((_, ix) => ix % 2 == 1)) + 1

    // Create the GPU-side buffers
    const argBytes = 3 * 4
    const inputBytes = input.byteLength
    const argBuffer = device.createBuffer({ size: argBytes, usage: UNIFORM | COPY_DST })
    const inputBuffer = device.createBuffer({ size: inputBytes, usage: STORAGE | COPY_DST })
    const tempBuffer = device.createBuffer({ size: sizeX * sizeY * 4, usage: STORAGE })
    const resultBuffer = device.createBuffer({ size: 4, usage: STORAGE | COPY_SRC })

    // Compile the WGSL shader code into a pipeline
    const module = device.createShaderModule({ code: kernel })
    const pipeline = device.createComputePipeline({
        compute: { module: module, entryPoint: "main" },
    })

    // Create a bind group that binds resources like buffers or textures
    // to the shader inputs ie. `[[group(X), binding(Y)]]` definitions.
    const bindGroup = device.createBindGroup({
        layout: pipeline.getBindGroupLayout(0),
        entries: [
            { binding: 0, resource: { buffer: argBuffer } },
            { binding: 1, resource: { buffer: inputBuffer } },
            { binding: 2, resource: { buffer: tempBuffer } },
            { binding: 3, resource: { buffer: resultBuffer } },
        ],
    })

    // Upload arguments and input data. The argument order must match the
    // layout of `ArgBuffer` in the shader.
    const numLines = input.length / 4
    const args = [sizeX, sizeY, numLines]
    uploadData(device, [
        { buffer: argBuffer, data: new Uint32Array(args) },
        { buffer: inputBuffer, data: input },
    ])

    // Start encoding the commands to `pass`.
    const encoder = device.createCommandEncoder()
    const pass = encoder.beginComputePass()

    // Encode the single dispatch we need to do. X count is 1 since the
    // uses that dimension only to loop over line cells. Y is the number
    // of 8 line groups we want to process.
    pass.setPipeline(pipeline)
    pass.setBindGroup(0, bindGroup)
    pass.dispatch(1, Math.ceil(numLines / 8))
    pass.endPass()

    // Submit to actually start processing on the GPU.
    const commands = encoder.finish()
    device.queue.submit([commands])

    // Download the resulting count from the GPU. `downloadData()` uses
    // `GPUBuffer.mapAsync()` internally that waits for the previous
    // commands to complete on the GPU.
    const resultData = await downloadData(device, resultBuffer, 0, 4)
    const resultArray = new Uint32Array(resultData)
    const result = resultArray[0]

    // According to the spec we could use `device.destroy()` to release
    // all the resources but it doesn't seem to be available at the moment?
    const resources = [argBuffer, inputBuffer, tempBuffer, resultBuffer]
    resources.forEach(r => r.destroy())

    return result
}

const input = fetch("input.txt")
    .then(r => r.text())
    .then(parseInput)
    .then(countIntersections)
    .then(console.log)

