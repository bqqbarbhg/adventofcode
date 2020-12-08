require("fs")
    .readFileSync(process.argv[2], { encoding: "utf-8" })
    .replace(/\r\n/g, "\n")
    .split("\n\n")
    .map(g => g
        .split("\n")
        .map(s => new Set(s.replace(/[^a-z]/g, "")))
        .filter(s => s.size > 0)
        .reduce((a, b) => new Set([...a].filter(l => b.has(l))))
    )
    .map(g => g.size)
    .reduce((a, b) => a + b)
    .toString()
    .split(" ")
    .forEach(s => console.log(s))