package main

import (
	"bufio"
	"log"
	"os"
	"regexp"
	"strconv"
)

type relation struct {
	target int
	delta  int
}

type person struct {
	index     int
	name      string
	relations []relation
}

func findOrCreatePerson(people map[string]*person, name string) *person {
	old_person, found := people[name]
	if found {
		return old_person
	}
	new_person := new(person)
	new_person.index = len(people)
	new_person.relations = make([]relation, 0)
	people[name] = new_person
	return new_person
}

type swap struct {
	a int
	b int
}

func permutationSwapsImpl(n int, swaps chan swap) {
	if n > 1 {
		for i := 0; i < n-1; i++ {
			permutationSwapsImpl(n-1, swaps)
			if n%2 == 0 {
				swaps <- swap{a: i, b: n - 1}
			} else {
				swaps <- swap{a: 0, b: n - 1}
			}
		}
		permutationSwapsImpl(n-1, swaps)
	}
}

func permutationSwaps(n int) <-chan swap {
	ch := make(chan swap, 16)
	go func() {
		permutationSwapsImpl(n, ch)
		close(ch)
	}()
	return ch
}

type table struct {
	people    []int
	happiness []int
	relations []int
}

func makeTable(people_map map[string]*person) *table {
	table := new(table)
	num_people := len(people_map)

	people := make([]int, num_people)
	for i := 0; i < num_people; i++ {
		people[i] = i
	}
	table.people = people

	relations := make([]int, num_people*num_people)
	for _, person := range people_map {
		a := person.index
		for _, relation := range person.relations {
			b := relation.target
			relations[a+b*num_people] += relation.delta
			relations[b+a*num_people] += relation.delta
		}
	}
	table.relations = relations

	happiness := make([]int, num_people)
	for i := 0; i < num_people; i++ {
		next := (i + 1) % num_people
		happiness[i] = relations[people[i]+people[next]*num_people]
	}
	table.happiness = happiness

	return table
}

func (t *table) relation(a int, b int) int {
	return t.relations[a*len(t.people)+b]
}

func (t *table) swapPeople(ai int, bi int) int {
	size := len(t.people)

	a, b := t.people[ai], t.people[bi]

	al, ar := (ai+size-1)%size, (ai+1)%size
	bl, br := (bi+size-1)%size, (bi+1)%size

	t.people[ai] = b
	t.people[bi] = a

	old_happiness := t.happiness[al] + t.happiness[ai] + t.happiness[bl] + t.happiness[bi]

	al_happiness := t.relation(b, t.people[al])
	ar_happiness := t.relation(b, t.people[ar])
	bl_happiness := t.relation(a, t.people[bl])
	br_happiness := t.relation(a, t.people[br])

	t.happiness[al] = al_happiness
	t.happiness[ai] = ar_happiness
	t.happiness[bl] = bl_happiness
	t.happiness[bi] = br_happiness

	return al_happiness + ar_happiness + bl_happiness + br_happiness - old_happiness
}

func parsePeople() map[string]*person {
	people := make(map[string]*person)

	regex := regexp.MustCompile("(\\w+) would (lose|gain) (\\d+) happiness units by sitting next to (\\w+)")
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		line := scanner.Text()
		matches := regex.FindStringSubmatch(line)
		if matches == nil {
			continue
		}

		src := findOrCreatePerson(people, matches[1])
		dst := findOrCreatePerson(people, matches[4])

		abs_amount, err := strconv.Atoi(matches[3])
		if err != nil {
			log.Print(err)
			continue
		}

		var amount int

		switch matches[2] {
		case "lose":
			amount = -abs_amount
		case "gain":
			amount = abs_amount
		default:
			log.Print("Unexpected mode %v\n", matches[3])
			continue
		}

		src.relations = append(src.relations, relation{target: dst.index, delta: amount})
	}

	return people
}
