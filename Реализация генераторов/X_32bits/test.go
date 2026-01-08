package x32bits

type Test struct {
	S [4]uint32
}

func (x Test) rotl(a uint32, b int) uint32 {
	return (a << b) | (a >> (32 - b))
}

func (x *Test) Next() uint32 {
	var result uint32 = x.rotl(x.S[0]^x.S[3], 7) ^ x.S[0]

	var t uint32 = x.S[1] << 9

	(*x).S[2] ^= x.S[0]
	(*x).S[3] ^= x.S[1]
	(*x).S[1] ^= x.S[2]
	(*x).S[0] ^= x.S[3]

	(*x).S[2] ^= t

	(*x).S[3] = x.rotl(x.S[3], 11)

	return result
}
