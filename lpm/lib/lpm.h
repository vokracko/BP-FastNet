
#define LPM_BSPL 0
#define LPM_TMB 1
#define LPM_SST 2

#define LPM_HASH_JENKINS 0

uint32_t calculate_hash(char * key, size_t len);
void lpm_init(uint8_t lookup_function, uint8_t hash_function);
