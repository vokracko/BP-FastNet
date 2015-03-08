#include "lpm.h"

typedef struct _sst_node_
{
	uint32_t shape;
	uint32_t internal;
	uint32_t external;

	_sst_node_ * child;
	_LPM_RULE * rule;

} _sst_node;

/*
N_i = počet uzlů ve vzdálenosti i od konkrétního uzlu (i s dummy nodes)
F_i = pozice prního uzlu ve vzdálenosti i

N_1 = 2;
F_1 = 0;

F_i = F_(i-1) + N_(i-1)

ones(i, j) = počet jedniček v shape bitmap od i po j
N_i = 2 * ones(F_(i-1). F_i - 1)

A_i = ítý bit IP adresy související s aktuálním uzlem
P_i = index do shape bitmapy vztahující se k uzlu specifikovaného IP adresou ve vzdálenosti i od konkrétního uzlu

P_1 = A_1

P_i = F_i + 2 * ones(F_(i-1), P_(i-1) - 1) + A_i pro i > 1

pokud je i nejmenší číslo pro které platí že shape bitmapa pro P_i je nula => odchází z podstromu (!! nekončí vyhledávání)

pokračovat do dalšího podstromu:
	zkontrolovat externí bitmapu na indexu:
		zeros(0, P_i - 1) v shape bitmapě

	pokud je index validní a bit na této pozici v externí bitmapě je nastaven na 1 tak vyhledávání pokračuje v potomkovy tohoto uzlu
	pro najití potomka se použije child[ones(0, index - 1)]
*/
//TODO podívat se do bakalářek na fitu, tohle moc nechápu
