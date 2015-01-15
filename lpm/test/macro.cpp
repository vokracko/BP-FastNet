#include "../lib/bspl.h"
#include <iostream>
#include <bitset>

int main(void)
{
	std::bitset<32> set = 0b11101000000000000000000000000010;

	std::cout << set << ".at(32) " << get_bit( set.to_ulong(), 32) << std::endl;
	std::cout << set << ".at(31) " << get_bit( set.to_ulong(), 31) << std::endl;
	std::cout << set << ".at(30) " << get_bit( set.to_ulong(), 30) << std::endl;
	std::cout << set << ".at(29) " << get_bit( set.to_ulong(), 29) << std::endl;
	std::cout << set << ".at(28) " << get_bit( set.to_ulong(), 28) << std::endl;
	std::cout << set << ".at(27) " << get_bit( set.to_ulong(), 27) << std::endl;
	std::cout << set << ".at(2) " << get_bit( set.to_ulong(), 2) << std::endl;
	std::cout << set << ".at(1) " << get_bit( set.to_ulong(), 1) << std::endl;
	std::cout << set << ".at(0) "  << get_bit( set.to_ulong(), 0) << std::endl;

	std::cout << set << " - 32 - "<< std::bitset<32>(get_bits( set.to_ulong(), 32)) << std::endl;
	std::cout << set << " - 31 - "<< std::bitset<32>(get_bits( set.to_ulong(), 31)) << std::endl;
	std::cout << set << " - 30 - "<< std::bitset<32>(get_bits( set.to_ulong(), 30)) << std::endl;
	std::cout << set << " - 29 - "<< std::bitset<32>(get_bits( set.to_ulong(), 29)) << std::endl;
	std::cout << set << " - 28 - "<< std::bitset<32>(get_bits( set.to_ulong(), 28)) << std::endl;
	std::cout << set << " - 5 - "<< std::bitset<32>(get_bits( set.to_ulong(), 5)) << std::endl;
	std::cout << set << " - 4 - "<< std::bitset<32>(get_bits( set.to_ulong(), 4)) << std::endl;
	std::cout << set << " - 3 - "<< std::bitset<32>(get_bits( set.to_ulong(), 3)) << std::endl;
	std::cout << set << " - 2 - "<< std::bitset<32>(get_bits( set.to_ulong(), 2)) << std::endl;
	std::cout << set << " - 1 - "<< std::bitset<32>(get_bits( set.to_ulong(), 1)) << std::endl;
	std::cout << set << " - 0 - "<< std::bitset<32>(get_bits( set.to_ulong(), 0)) << std::endl;

}
