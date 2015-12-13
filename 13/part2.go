package main

import (
	"fmt"
)

func main() {
	people := parsePeople()
	findOrCreatePerson(people, "bqqbarbhg")

	table := makeTable(people)

	happiness := 0
	for _, local_happiness := range table.happiness {
		happiness += local_happiness
	}

	max_happiness := happiness
	for swap := range permutationSwaps(len(table.people)) {
		happiness += table.swapPeople(swap.a, swap.b)
		if happiness > max_happiness {
			max_happiness = happiness
		}
	}

	fmt.Printf("%v\n", max_happiness)
}
