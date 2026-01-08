package x32bits

type Xoroshiro64_MM struct {
	S [2]uint32
}

func (x Xoroshiro64_MM) rotl(a uint32, b int) uint32 {
	return (a << b) | (a >> (32 - b))
}

func (x *Xoroshiro64_MM) Next() uint32 {
	var (
		s0     uint32 = x.S[0]
		s1     uint32 = x.S[1]
		result uint32 = x.rotl(s0*0x9E3779BB, 5) * 5
	)

	s1 ^= s0
	(*x).S[0] = x.rotl(s0, 26) ^ s1 ^ (s1 << 9) // a, b
	(*x).S[1] = x.rotl(s1, 13)                  // c

	return result
}
