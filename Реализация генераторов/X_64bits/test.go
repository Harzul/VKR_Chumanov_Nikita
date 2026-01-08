package x64bits

type Test struct {
	S [4]uint64
}

func (x Test) rotl(a uint64, b int) uint64 {
	return (a << b) | (a >> (64 - b))
}

func (x *Test) Next() uint64 {
	var result uint64 = x.rotl(x.S[0]+x.S[3], 23)

	var t uint64 = x.S[1] << 17

	(*x).S[2] ^= x.S[0]
	(*x).S[3] ^= x.S[1]
	(*x).S[1] ^= x.S[2]
	(*x).S[0] ^= x.S[3]

	(*x).S[2] ^= t

	(*x).S[3] = x.rotl(x.S[3], 45)

	return result
}
